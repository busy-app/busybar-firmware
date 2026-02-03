#include "mqtt_i.h"

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

    if(instance->status == MqttStatusConnectedLinked && scope == MqttScopeSession) {
        is_valid = true;
    } else if(instance->status == MqttStatusConnectedNotLinked && scope == MqttScopeDevice) {
        is_valid = true;
    }

    return is_valid;
}

static void mqtt_make_topic_path(
    Mqtt* instance,
    MqttScope scope,
    const char* dir,
    const char* topic,
    FuriString* out) {
    const char* root;
    const char* id;

    if(scope == MqttScopeDevice) {
        root = MQTT_DEVICE_ROOT_TOPIC;
        id = furi_string_get_cstr(instance->device_serial);

    } else if(scope == MqttScopeSession) {
        root = MQTT_SESSION_ROOT_TOPIC;
        id = furi_string_get_cstr(instance->saved_state.session_id);

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

    // TODO: Implement proper QoS handling
    furi_check(instance->conn);
    mg_mqtt_pub(instance->conn, &opts);

    if(raw_props) {
        free(raw_props);
    }

    furi_string_free(path);

    return true;
}
