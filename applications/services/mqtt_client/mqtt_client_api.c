#include "mqtt_client_i.h"

static void mqtt_client_send_message(MqttClient* mqtt, MqttClientMessage* msg) {
    msg->lock = api_lock_alloc_locked();
    mg_wakeup(&mqtt->mgr, mqtt->wakeup_conn_id, msg, sizeof(MqttClientMessage));
    api_lock_wait_unlock_and_free(msg->lock);
}

MqttClientStatus mqtt_client_get_status(MqttClient* mqtt) {
    furi_assert(mqtt);

    MqttClientStatus status;

    MqttClientMessage msg = {.type = MqttClientMessageGetStatus, .status = &status};
    mqtt_client_send_message(mqtt, &msg);

    return status;
}

bool mqtt_client_request_link_pin(MqttClient* mqtt) {
    furi_assert(mqtt);

    bool success = false;

    MqttClientMessage msg = {.type = MqttClientMessageRequestPin, .bool_param = &success};
    mqtt_client_send_message(mqtt, &msg);

    return success;
}

void mqtt_client_unlink(MqttClient* mqtt) {
    furi_assert(mqtt);

    MqttClientMessage msg = {.type = MqttClientMessageUnlink};
    mqtt_client_send_message(mqtt, &msg);
}

void mqtt_client_get_session_info(
    MqttClient* mqtt,
    FuriString* id,
    FuriString* email,
    FuriString* user_id) {
    furi_assert(mqtt);

    MqttClientMessage msg = {
        .type = MqttClientMessageGetSessionInfo,
        .session_info = {.id = id, .email = email, .user_id = user_id},
    };
    mqtt_client_send_message(mqtt, &msg);
}

MqttClientProfile mqtt_client_get_profile(MqttClient* mqtt, FuriString* custom_url) {
    furi_assert(mqtt);
    MqttClientProfile profile;

    MqttClientMessage msg = {
        .type = MqttClientMessageGetProfile,
        .profile = {.id = &profile, .custom_url = custom_url},
    };
    mqtt_client_send_message(mqtt, &msg);

    return profile;
}

void mqtt_client_set_profile(MqttClient* mqtt, MqttClientProfile profile, char* custom_url) {
    furi_assert(mqtt);

    FuriString* url = furi_string_alloc_set(custom_url);
    MqttClientMessage msg = {
        .type = MqttClientMessageSetProfile,
        .profile = {.id = &profile, .custom_url = url},
    };
    mqtt_client_send_message(mqtt, &msg);
    furi_string_free(url);
}

FuriPubSub* mqtt_client_get_pubsub(MqttClient* mqtt) {
    furi_assert(mqtt);
    return mqtt->event_pubsub;
}
