#include "mqtt_i.h"

#define MQTT_MAX_UNSENT_DATA_SIZE_BYTES (50 * 1024)

static MqttSubscription* mqtt_subscription_alloc(void) {
    MqttSubscription* subscription = malloc(sizeof(MqttSubscription));

    subscription->topic = furi_string_alloc();
    MqttSubscriptionList_init_field(subscription);

    return subscription;
}

static void mqtt_subscription_free(MqttSubscription* subscription) {
    furi_string_free(subscription->topic);
    free(subscription);
}

static bool mqtt_is_valid_scope_for_current_status(Mqtt* instance, MqttScope scope) {
    bool is_valid = false;

    if(scope == MqttScopeSession) {
        if(instance->status == MqttStatusConnectedLinked) {
            is_valid = true;
        }
    } else if(scope == MqttScopeDevice) {
        if(instance->status == MqttStatusConnectedLinked ||
           instance->status == MqttStatusConnectedNotLinked) {
            is_valid = true;
        }
    }

    return is_valid;
}

void mqtt_make_topic_path(
    Mqtt* instance,
    MqttScope scope,
    const char* dir,
    const char* topic,
    FuriString* out) {
    const char* root;
    const char* id;

    if(scope == MqttScopeDevice) {
        root = MQTT_ROOT_TOPIC_DEVICE;
        id = furi_string_get_cstr(instance->device_serial);

    } else if(scope == MqttScopeSession) {
        root = MQTT_ROOT_TOPIC_SESSION;
        id = instance->saved_state.session_id;

    } else {
        furi_crash("Invalid MqttScope value");
    }

    furi_string_printf(out, "%s/%s/%s/%s/%s", root, id, dir, MQTT_API_VERSION, topic);
}

MqttSubscription* mqtt_subscribe_internal(
    Mqtt* instance,
    MqttScope scope,
    MqttQos qos,
    const char* topic,
    MqttSubscriptionCallback callback,
    void* context) {
    MqttSubscription* subscription = mqtt_subscription_alloc();

    furi_string_set(subscription->topic, topic);
    subscription->scope = scope;
    subscription->qos = qos;
    subscription->callback = callback;
    subscription->callback_context = context;

    MqttSubscriptionList_push_back(instance->subscriptions, subscription);
    mqtt_subscription_activate(instance, subscription);

    return subscription;
}

void mqtt_unsubscribe_internal(Mqtt* instance, MqttSubscription* subscription) {
    MqttSubscriptionList_unlink(subscription);
    mqtt_subscription_free(subscription);
    // NOTE: Current Mongoose version does not support unsubscription
    mqtt_connection_close(instance, true);
}

void mqtt_subscription_activate(Mqtt* instance, const MqttSubscription* subscription) {
    if(!mqtt_is_valid_scope_for_current_status(instance, subscription->scope)) {
        return;
    }

    FuriString* topic_path = furi_string_alloc();

    mqtt_make_topic_path(
        instance,
        subscription->scope,
        MQTT_DIRECTION_DOWN,
        furi_string_get_cstr(subscription->topic),
        topic_path);

    FURI_LOG_D(TAG, "Subscribing to %s", furi_string_get_cstr(topic_path));

    const struct mg_mqtt_opts sub_opts = {
        .topic = mg_str(furi_string_get_cstr(topic_path)),
        .qos = subscription->qos,
    };

    furi_check(instance->conn);
    mg_mqtt_sub(instance->conn, &sub_opts);

    furi_string_free(topic_path);
}

bool mqtt_publish_internal(
    Mqtt* instance,
    MqttScope scope,
    MqttQos qos,
    const char* topic,
    const void* data,
    size_t data_size,
    const MqttProperty* props,
    uint32_t props_count) {
    if(!mqtt_is_valid_scope_for_current_status(instance, scope)) {
        FURI_LOG_E(TAG, "Unable to publish: scope: %d, status: %d", scope, instance->status);
        return false;
    }

    // Drop messages if we have huge backlog of unsent messages
    if(instance->conn->send.size >= MQTT_MAX_UNSENT_DATA_SIZE_BYTES) {
        FURI_LOG_W(TAG, "Dropping %u bytes from topic '%s'", data_size, topic);
        return false;
    }

    FuriString* path = furi_string_alloc();
    mqtt_make_topic_path(instance, scope, MQTT_DIRECTION_UP, topic, path);

    struct mg_mqtt_prop* raw_props = NULL;

    if(props && props_count) {
        raw_props = malloc(props_count * sizeof(struct mg_mqtt_prop));
        for(uint32_t i = 0; i < props_count; ++i) {
            mqtt_property_to_raw(&props[i], &raw_props[i]);
        }
    }

    const struct mg_mqtt_opts opts = {
        .topic = mg_str(furi_string_get_cstr(path)),
        .message = mg_str_n(data, data_size),
        .qos = qos,
        .props = raw_props,
        .num_props = props_count,
    };

    furi_check(instance->conn);
    const uint16_t message_id = mg_mqtt_pub(instance->conn, &opts);

    if(raw_props) {
        free(raw_props);
    }

    // Only QoS 1 messages without properties are covered by the in-flight tracking
    if(qos == MqttQosAtLeastOnce && props_count == 0) {
        mqtt_inflight_track(instance, message_id, scope, qos, path, data, data_size);
    }

    furi_string_free(path);

    return true;
}

static void mqtt_inflight_message_free(MqttInflightMessage* message) {
    if(message->topic_path) {
        furi_string_free(message->topic_path);
    }

    if(message->data) {
        free(message->data);
    }

    memset(message, 0, sizeof(MqttInflightMessage));
}

static void mqtt_inflight_message_discard(Mqtt* instance, size_t index) {
    instance->inflight_data_size -= instance->inflight[index].data_size;
    mqtt_inflight_message_free(&instance->inflight[index]);

    for(size_t i = index + 1; i < instance->inflight_count; ++i) {
        instance->inflight[i - 1] = instance->inflight[i];
    }

    instance->inflight_count--;
    memset(&instance->inflight[instance->inflight_count], 0, sizeof(MqttInflightMessage));
}

void mqtt_inflight_track(
    Mqtt* instance,
    uint16_t id,
    MqttScope scope,
    MqttQos qos,
    const FuriString* topic_path,
    const void* data,
    size_t data_size) {
    furi_assert(instance);
    furi_assert(id != 0);
    furi_assert(topic_path);

    while(instance->inflight_count >= MQTT_INFLIGHT_MESSAGES_MAX ||
          (instance->inflight_count > 0 &&
           instance->inflight_data_size + data_size > MQTT_INFLIGHT_DATA_MAX_BYTES)) {
        FURI_LOG_W(TAG, "In-flight message limit exceeded, dropping the oldest one");
        mqtt_inflight_message_discard(instance, 0);
    }

    MqttInflightMessage* message = &instance->inflight[instance->inflight_count++];
    message->id = id;
    message->scope = scope;
    message->qos = qos;
    message->topic_path = furi_string_alloc_set(topic_path);
    message->data_size = data_size;

    if(data_size > 0) {
        message->data = malloc(data_size);
        memcpy(message->data, data, data_size);
    }

    instance->inflight_data_size += data_size;
}

void mqtt_inflight_ack(Mqtt* instance, uint16_t id) {
    furi_assert(instance);

    for(size_t i = 0; i < instance->inflight_count; ++i) {
        if(instance->inflight[i].id == id) {
            mqtt_inflight_message_discard(instance, i);
            return;
        }
    }
}

void mqtt_inflight_retransmit(Mqtt* instance) {
    furi_assert(instance);
    furi_check(instance->conn);

    for(size_t i = 0; i < instance->inflight_count;) {
        const MqttInflightMessage* message = &instance->inflight[i];

        if(!mqtt_is_valid_scope_for_current_status(instance, message->scope)) {
            FURI_LOG_W(TAG, "Dropping in-flight message for an invalid scope");
            mqtt_inflight_message_discard(instance, i);
            continue;
        }

        const struct mg_mqtt_opts opts = {
            .topic = mg_str(furi_string_get_cstr(message->topic_path)),
            .message = mg_str_n((const char*)message->data, message->data_size),
            .qos = message->qos,
            .retransmit_id = message->id,
        };

        FURI_LOG_D(TAG, "Retransmitting message %u", message->id);
        mg_mqtt_pub(instance->conn, &opts);
        ++i;
    }
}
