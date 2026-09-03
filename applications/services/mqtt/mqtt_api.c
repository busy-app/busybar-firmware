#include "mqtt_i.h"

#include <mongoose_dns.h>

// =========  API message passing (public) =========

static void mqtt_send_message(Mqtt* instance, MqttApiMessage* message) {
    message->lock = api_lock_alloc_locked();
    mg_wakeup(&instance->mgr, instance->api_connection_id, message, sizeof(MqttApiMessage));
    api_lock_wait_unlock_and_free(message->lock);
}

MqttStatus mqtt_get_status(Mqtt* instance) {
    furi_check(instance);

    MqttStatus status;

    MqttApiMessage message = {
        .type = MqttApiMessageTypeGetStatus,
        .data.get_status =
            {
                .status = &status,
            },
    };

    mqtt_send_message(instance, &message);
    return status;
}

bool mqtt_request_link_pin(Mqtt* instance) {
    furi_check(instance);

    bool success = false;

    MqttApiMessage message = {
        .type = MqttApiMessageTypeRequestPin,
        .data.request_pin =
            {
                .is_success = &success,
            },
    };

    mqtt_send_message(instance, &message);
    return success;
}

void mqtt_unlink(Mqtt* instance) {
    furi_check(instance);

    MqttApiMessage message = {
        .type = MqttApiMessageTypeUnlink,
    };

    mqtt_send_message(instance, &message);
}

void mqtt_get_session_info(Mqtt* instance, MqttSessionInfo* info) {
    furi_check(instance);

    MqttApiMessage message = {
        .type = MqttApiMessageTypeGetSessionInfo,
        .data.get_session_info =
            {
                .session_id = info->session_id,
                .user_id = info->user_id,
                .email = info->email,
                .is_valid = &info->is_valid,
            },
    };

    mqtt_send_message(instance, &message);
}

void mqtt_get_config(Mqtt* instance, MqttConfig* config) {
    furi_check(instance);
    furi_check(config);

    MqttApiMessage message = {
        .type = MqttApiMessageTypeGetConfig,
        .data.get_config =
            {
                .config = config,
            },
    };

    mqtt_send_message(instance, &message);
}

bool mqtt_set_config(Mqtt* instance, const MqttConfig* config) {
    furi_check(instance);
    furi_check(config);

    bool is_success = false;

    MqttApiMessage message = {
        .type = MqttApiMessageTypeSetConfig,
        .data.set_config =
            {
                .config = config,
                .is_success = &is_success,
            },
    };

    mqtt_send_message(instance, &message);
    return is_success;
}

bool mqtt_publish(
    Mqtt* instance,
    MqttQos qos,
    const char* topic,
    const void* data,
    size_t data_size) {
    furi_check(instance);
    furi_check(topic);
    furi_check(data);
    furi_check(data_size);
    furi_check(qos < MqttQosMax);

    bool is_success = false;

    MqttApiMessage message = {
        .type = MqttApiMessageTypePublish,
        .data.publish =
            {
                .topic = topic,
                .data = data,
                .data_size = data_size,
                .qos = qos,
                .is_success = &is_success,
            },
    };

    mqtt_send_message(instance, &message);
    return is_success;
}

bool mqtt_publish_ex(
    Mqtt* instance,
    MqttQos qos,
    const char* topic,
    const void* data,
    size_t data_size,
    const MqttProperty* props,
    uint32_t props_count) {
    furi_check(instance);
    furi_check(topic);
    furi_check(data);
    furi_check(data_size);
    furi_check(qos < MqttQosMax);

    bool is_success = false;

    MqttApiMessage message = {
        .type = MqttApiMessageTypePublish,
        .data.publish =
            {
                .topic = topic,
                .data = data,
                .data_size = data_size,
                .props = props,
                .props_count = props_count,
                .qos = qos,
                .is_success = &is_success,
            },
    };

    mqtt_send_message(instance, &message);
    return is_success;
}

MqttSubscription* mqtt_subscribe(
    Mqtt* instance,
    MqttQos qos,
    const char* topic,
    MqttSubscriptionCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(topic);
    furi_check(callback);
    furi_check(qos < MqttQosMax);

    MqttSubscription* subscription;

    MqttApiMessage message = {
        .type = MqttApiMessageTypeSubscribe,
        .data.subscribe =
            {
                .topic = topic,
                .callback = callback,
                .callback_context = context,
                .subscription = &subscription,
                .qos = qos,
            },
    };

    mqtt_send_message(instance, &message);
    return subscription;
}

void mqtt_unsubscribe(Mqtt* instance, MqttSubscription* subscription) {
    furi_check(instance);
    furi_check(subscription);

    MqttApiMessage message = {
        .type = MqttApiMessageTypeUnsubscribe,
        .data.unsubscribe =
            {
                .subscription = subscription,
            },
    };

    mqtt_send_message(instance, &message);
}

// ========= Direct access API (public) =========

FuriPubSub* mqtt_get_pubsub(Mqtt* instance) {
    furi_check(instance);
    return instance->event_pubsub;
}

// ========= API message handling (private) =========

static void mqtt_get_status_api_message_handler(Mqtt* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageGetStatus* get_status = &data->get_status;
    *(get_status->status) = instance->status;
}

static void mqtt_unlink_api_message_handler(Mqtt* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    UNUSED(data);

    FURI_LOG_I(TAG, "Received unlink message from user");

    if(instance->status == MqttStatusConnectedLinked) {
        const char* empty = "{}";

        bool is_success = mqtt_publish_internal(
            instance,
            MqttScopeSession,
            MqttQosAtMostOnce,
            MQTT_TOPIC_UNLINK_FROM_DEVICE,
            empty,
            strlen(empty),
            NULL,
            0);
        if(!is_success) {
            FURI_LOG_W(TAG, "Failed to send unlink message to cloud");
        }
    }

    mqtt_account_unlink(instance);
}

static void mqtt_request_pin_api_message_handler(Mqtt* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    bool is_success = false;

    const MqttApiMessageRequestPin* request_pin = &data->request_pin;

    if(instance->status == MqttStatusConnectedNotLinked) {
        const char* empty = "{}";

        is_success = mqtt_publish_internal(
            instance,
            MqttScopeDevice,
            MqttQosExactlyOnce,
            MQTT_TOPIC_LINK_REQUEST,
            empty,
            strlen(empty),
            NULL,
            0);
    }

    *(request_pin->is_success) = is_success;
}

static void
    mqtt_get_session_info_api_message_handler(Mqtt* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageGetSessionInfo* get_session_info = &data->get_session_info;
    const MqttSavedState* saved_state = &instance->saved_state;

    if(get_session_info->session_id) {
        furi_string_set(get_session_info->session_id, saved_state->session_id);
    }
    if(get_session_info->user_id) {
        furi_string_set(get_session_info->user_id, saved_state->user_id);
    }
    if(get_session_info->email) {
        furi_string_set(get_session_info->email, saved_state->email);
    }

    furi_assert(get_session_info->is_valid);
    *get_session_info->is_valid = mqtt_saved_state_is_valid(saved_state);
}

static void mqtt_get_config_api_message_handler(Mqtt* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageGetConfig* get_config = &data->get_config;
    const MqttSettings* settings = &instance->settings;
    *get_config->config = settings->config;
}

static void mqtt_set_config_api_message_handler(Mqtt* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageSetConfig* set_config = &data->set_config;
    const MqttConfig* config = set_config->config;

    bool is_success = false;

    if(mqtt_config_is_valid(config)) {
        MqttSettings* settings = &instance->settings;
        settings->config = *set_config->config;

        mqtt_settings_save(settings);
        mqtt_connection_close(instance, true);

        is_success = true;
    }

    *set_config->is_success = is_success;
}

static void mqtt_publish_api_message_handler(Mqtt* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessagePublish* publish = &data->publish;
    furi_assert(publish->is_success);

    *publish->is_success = mqtt_publish_internal(
        instance,
        MqttScopeSession,
        publish->qos,
        publish->topic,
        publish->data,
        publish->data_size,
        publish->props,
        publish->props_count);
}

static void mqtt_subscribe_api_message_handler(Mqtt* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageSubscribe* subscribe = &data->subscribe;
    *subscribe->subscription = mqtt_subscribe_internal(
        instance,
        MqttScopeSession,
        subscribe->qos,
        subscribe->topic,
        subscribe->callback,
        subscribe->callback_context);
}

static void mqtt_unsubscribe_api_message_handler(Mqtt* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageUnsubscribe* unsubscribe = &data->unsubscribe;
    mqtt_unsubscribe_internal(instance, unsubscribe->subscription);
}

static void mqtt_wifi_state_api_message_handler(Mqtt* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageWifiState* wifi_state = &data->wifi_state;

    if(wifi_state->state == WifiStateConnected) {
        if((!instance->is_wifi_up) && (instance->conn == NULL)) {
            mongoose_dns_init(&instance->mgr);
            mqtt_connection_open(instance);
        }

        instance->is_wifi_up = true;

    } else {
        instance->is_wifi_up = false;
    }
}

typedef void (*MqttApiMessageHandler)(Mqtt* instance, const MqttApiMessageData* data);

static const MqttApiMessageHandler mqtt_api_message_handlers[MqttApiMessageTypeMax] = {
    [MqttApiMessageTypeGetStatus] = mqtt_get_status_api_message_handler,
    [MqttApiMessageTypeUnlink] = mqtt_unlink_api_message_handler,
    [MqttApiMessageTypeRequestPin] = mqtt_request_pin_api_message_handler,
    [MqttApiMessageTypeGetSessionInfo] = mqtt_get_session_info_api_message_handler,
    [MqttApiMessageTypeGetConfig] = mqtt_get_config_api_message_handler,
    [MqttApiMessageTypeSetConfig] = mqtt_set_config_api_message_handler,
    [MqttApiMessageTypePublish] = mqtt_publish_api_message_handler,
    [MqttApiMessageTypeSubscribe] = mqtt_subscribe_api_message_handler,
    [MqttApiMessageTypeUnsubscribe] = mqtt_unsubscribe_api_message_handler,
    [MqttApiMessageTypeWifiState] = mqtt_wifi_state_api_message_handler,
};

static void
    mqtt_api_event_callback(struct mg_connection* connection, int event, void* event_data) {
    if(event == MG_EV_WAKEUP) {
        Mqtt* instance = connection->fn_data;
        furi_assert(instance);

        const struct mg_str* data = event_data;
        furi_assert(data);
        furi_assert(data->buf);
        furi_assert(data->len == sizeof(MqttApiMessage));

        const MqttApiMessage* message = (const MqttApiMessage*)data->buf;
        const MqttApiMessageType message_type = message->type;
        furi_assert(message_type < MqttApiMessageTypeMax);

        mqtt_api_message_handlers[message_type](instance, &message->data);

        if(message->lock) {
            api_lock_unlock(message->lock);
        }
    }
}

void mqtt_api_init(Mqtt* instance) {
    // Create a dummy connection only for wakeup event
    const struct mg_connection* api_connection =
        mg_wrapfd(&instance->mgr, MG_INVALID_SOCKET, mqtt_api_event_callback, instance);
    instance->api_connection_id = api_connection->id;
}
