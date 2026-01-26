#include "mqtt_client_i.h"

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

static void mqtt_publish_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
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

static void mqtt_subscribe_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
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
    mqtt_unsubscribe_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageUnsubscribe* unsubscribe = &data->unsubscribe;
    UNUSED(unsubscribe);
    // TODO: Implementation
}

static void mqtt_wifi_state_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageWifiState* wifi_state = &data->wifi_state;

    if(wifi_state->state == WifiStateConnected) {
        if((!instance->is_wifi_up) && (instance->conn == NULL)) {
            mqtt_connect_callback(instance);
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
    [MqttApiMessageTypePublish] = mqtt_publish_message_handler,
    [MqttApiMessageTypeSubscribe] = mqtt_subscribe_message_handler,
    [MqttApiMessageTypeUnsubscribe] = mqtt_unsubscribe_message_handler,
    [MqttApiMessageTypeWifiState] = mqtt_wifi_state_message_handler,
};

void mqtt_handle_api_message(MqttClient* instance, const MqttApiMessage* message) {
    const MqttApiMessageType message_type = message->type;
    furi_assert(message_type < MqttApiMessageTypeMax);

    mqtt_api_message_handlers[message_type](instance, &message->data);

    if(message->lock) {
        api_lock_unlock(message->lock);
    }
}
