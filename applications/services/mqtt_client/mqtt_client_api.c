#include "mqtt_client_i.h"

static void mqtt_client_send_message(MqttClient* instance, MqttApiMessage* message) {
    message->lock = api_lock_alloc_locked();
    mg_wakeup(&instance->mgr, instance->wakeup_conn_id, message, sizeof(MqttApiMessage));
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

FuriPubSub* mqtt_client_get_pubsub(MqttClient* instance) {
    furi_check(instance);
    return instance->event_pubsub;
}
