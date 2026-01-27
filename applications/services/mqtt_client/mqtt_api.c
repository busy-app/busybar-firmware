#include "mqtt_client_i.h"

// =========  API message passing =========

static void mqtt_client_send_message(MqttClient* instance, MqttApiMessage* message) {
    message->lock = api_lock_alloc_locked();
    mg_wakeup(&instance->mgr, instance->api_connection_id, message, sizeof(MqttApiMessage));
    api_lock_wait_unlock_and_free(message->lock);
}

MqttClientStatus mqtt_client_get_status(MqttClient* instance) {
    furi_check(instance);

    MqttClientStatus status;

    MqttApiMessage message = {
        .type = MqttApiMessageTypeGetStatus,
        .data.get_status =
            {
                .status = &status,
            },
    };

    mqtt_client_send_message(instance, &message);
    return status;
}

bool mqtt_client_request_link_pin(MqttClient* instance) {
    furi_check(instance);

    bool success = false;

    MqttApiMessage message = {
        .type = MqttApiMessageTypeRequestPin,
        .data.request_pin =
            {
                .is_success = &success,
            },
    };

    mqtt_client_send_message(instance, &message);
    return success;
}

void mqtt_client_unlink(MqttClient* instance) {
    furi_check(instance);

    MqttApiMessage message = {
        .type = MqttApiMessageTypeUnlink,
    };

    mqtt_client_send_message(instance, &message);
}

void mqtt_client_get_session_info(
    MqttClient* instance,
    FuriString* session_id,
    FuriString* email,
    FuriString* user_id) {
    furi_check(instance);

    MqttApiMessage message = {
        .type = MqttApiMessageTypeGetSessionInfo,
        .data.get_session_info =
            {
                .session_id = session_id,
                .user_id = user_id,
                .email = email,
            },
    };

    mqtt_client_send_message(instance, &message);
}

MqttProfileId mqtt_client_get_profile(MqttClient* instance, FuriString* custom_url) {
    furi_check(instance);

    MqttProfileId profile_id;

    MqttApiMessage message = {
        .type = MqttApiMessageTypeGetProfile,
        .data.get_profile =
            {
                .profile_id = &profile_id,
                .custom_url = custom_url,
            },
    };

    mqtt_client_send_message(instance, &message);
    return profile_id;
}

void mqtt_client_set_profile(
    MqttClient* instance,
    MqttProfileId profile_id,
    const char* custom_url) {
    furi_check(instance);
    furi_check(profile_id < MqttProfileIdMax);

    MqttApiMessage message = {
        .type = MqttApiMessageTypeSetProfile,
        .data.set_profile =
            {
                .profile_id = profile_id,
                .custom_url = custom_url,
            },
    };

    mqtt_client_send_message(instance, &message);
}

void mqtt_client_publish(
    MqttClient* instance,
    MqttQos qos,
    const char* topic,
    const void* data,
    size_t data_size) {
    furi_check(instance);
    furi_check(topic);
    furi_check(data);
    furi_check(data_size);
    furi_check(qos < MqttQosMax);

    MqttApiMessage message = {
        .type = MqttApiMessageTypePublish,
        .data.publish =
            {
                .topic = topic,
                .data = data,
                .data_size = data_size,
                .qos = qos,
            },
    };

    mqtt_client_send_message(instance, &message);
}

MqttSubscription* mqtt_subscribe(
    MqttClient* instance,
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

    mqtt_client_send_message(instance, &message);
    return subscription;
}

void mqtt_unsubscribe(MqttClient* instance, MqttSubscription* subscription) {
    furi_check(instance);
    furi_check(subscription);

    MqttApiMessage message = {
        .type = MqttApiMessageTypeUnsubscribe,
        .data.unsubscribe =
            {
                .subscription = subscription,
            },
    };

    mqtt_client_send_message(instance, &message);
}

FuriPubSub* mqtt_client_get_pubsub(MqttClient* instance) {
    furi_check(instance);
    return instance->event_pubsub;
}

const void* mqtt_message_get_data(const MqttMessage* message, size_t* data_size) {
    furi_check(message);
    const struct mg_str data = TO_RAW_MESSAGE(message)->data;

    if(data_size) {
        *data_size = data.len;
    }

    return data.buf;
}

// ========= API message handling =========

static void
    mqtt_get_status_api_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageGetStatus* get_status = &data->get_status;
    *(get_status->status) = instance->status;
}

static void mqtt_unlink_api_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    if(instance->conn) {
        instance->conn->is_draining = 1;
        instance->fast_reconnect = true;
    }

    mqtt_reset_saved_state(instance);
}

static void
    mqtt_request_pin_api_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    bool is_success = false;

    const MqttApiMessageRequestPin* request_pin = &data->request_pin;

    if(instance->status == MqttClientStatusConnectedNotLinked) {
        const char* empty = "{}";
        mqtt_publish_internal(
            instance, MqttScopeDevice, MqttQosExactlyOnce, "link/request", empty, strlen(empty));
        is_success = true;
    }

    *(request_pin->is_success) = is_success;
}

static void mqtt_get_session_info_api_message_handler(
    MqttClient* instance,
    const MqttApiMessageData* data) {
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
}

static void
    mqtt_get_profile_api_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageGetProfile* get_profile = &data->get_profile;

    MqttSettings* settings = &instance->settings;
    const MqttProfileId profile_id = settings->profile_id;

    *(get_profile->profile_id) = profile_id;

    if(profile_id == MqttProfileIdCustom) {
        if(get_profile->custom_url) {
            furi_string_set(get_profile->custom_url, settings->custom_url);
        }
    }
}

static void
    mqtt_set_profile_api_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageSetProfile* set_profile = &data->set_profile;
    const MqttProfileId profile_id = set_profile->profile_id;

    MqttSettings* settings = &instance->settings;
    settings->profile_id = profile_id;

    if(profile_id == MqttProfileIdCustom) {
        // TODO: Better checks for custom url
        if(set_profile->custom_url) {
            furi_string_set(settings->custom_url, set_profile->custom_url);
        }
    }

    mqtt_settings_save(settings);

    if(instance->conn) {
        instance->conn->is_draining = 1;
    }
}

static void
    mqtt_publish_api_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessagePublish* publish = &data->publish;

    mqtt_publish_internal(
        instance,
        MqttScopeSession,
        publish->qos,
        publish->topic,
        publish->data,
        publish->data_size);
}

static void
    mqtt_subscribe_api_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
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

static void
    mqtt_unsubscribe_api_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageUnsubscribe* unsubscribe = &data->unsubscribe;
    UNUSED(unsubscribe);
    // TODO: Implementation
}

static void
    mqtt_wifi_state_api_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageWifiState* wifi_state = &data->wifi_state;

    if(wifi_state->state == WifiStateConnected) {
        if((!instance->is_wifi_up) && (instance->conn == NULL)) {
            mqtt_connection_open(instance);
        }

        instance->is_wifi_up = true;

    } else {
        instance->is_wifi_up = false;
    }
}

typedef void (*MqttApiMessageHandler)(MqttClient* instance, const MqttApiMessageData* data);

static const MqttApiMessageHandler mqtt_api_message_handlers[MqttApiMessageTypeMax] = {
    [MqttApiMessageTypeGetStatus] = mqtt_get_status_api_message_handler,
    [MqttApiMessageTypeUnlink] = mqtt_unlink_api_message_handler,
    [MqttApiMessageTypeRequestPin] = mqtt_request_pin_api_message_handler,
    [MqttApiMessageTypeGetSessionInfo] = mqtt_get_session_info_api_message_handler,
    [MqttApiMessageTypeGetProfile] = mqtt_get_profile_api_message_handler,
    [MqttApiMessageTypeSetProfile] = mqtt_set_profile_api_message_handler,
    [MqttApiMessageTypePublish] = mqtt_publish_api_message_handler,
    [MqttApiMessageTypeSubscribe] = mqtt_subscribe_api_message_handler,
    [MqttApiMessageTypeUnsubscribe] = mqtt_unsubscribe_api_message_handler,
    [MqttApiMessageTypeWifiState] = mqtt_wifi_state_api_message_handler,
};

static void
    mqtt_api_event_callback(struct mg_connection* connection, int event, void* event_data) {
    if(event == MG_EV_WAKEUP) {
        MqttClient* instance = connection->fn_data;
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

void mqtt_api_init(MqttClient* instance) {
    // Create a dummy connection only for wakeup event
    const struct mg_connection* api_connnection =
        mg_wrapfd(&instance->mgr, MG_INVALID_SOCKET, mqtt_api_event_callback, instance);
    instance->api_connection_id = api_connnection->id;
}
