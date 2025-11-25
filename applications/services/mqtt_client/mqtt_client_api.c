#include "mqtt_client_i.h"

MqttClientStatus mqtt_client_get_status(MqttClient* mqtt) {
    furi_assert(mqtt);

    MqttClientStatus status;

    MqttClientMessage msg = {
        .type = MqttClientMessageGetStatus,
        .status = &status,
        .lock = api_lock_alloc_locked(),
    };

    mg_wakeup(&mqtt->mgr, mqtt->wakeup_conn_id, &msg, sizeof(MqttClientMessage));
    api_lock_wait_unlock_and_free(msg.lock);

    return status;
}

bool mqtt_client_request_link_pin(MqttClient* mqtt) {
    furi_assert(mqtt);

    bool success = false;

    MqttClientMessage msg = {
        .type = MqttClientMessageRequestPin,
        .bool_param = &success,
        .lock = api_lock_alloc_locked(),
    };

    mg_wakeup(&mqtt->mgr, mqtt->wakeup_conn_id, &msg, sizeof(MqttClientMessage));
    api_lock_wait_unlock_and_free(msg.lock);

    return success;
}

void mqtt_client_unlink(MqttClient* mqtt) {
    furi_assert(mqtt);

    MqttClientMessage msg = {
        .type = MqttClientMessageUnlink,
        .lock = api_lock_alloc_locked(),
    };

    mg_wakeup(&mqtt->mgr, mqtt->wakeup_conn_id, &msg, sizeof(MqttClientMessage));
    api_lock_wait_unlock_and_free(msg.lock);
}

void mqtt_client_get_session_id(MqttClient* mqtt, FuriString* id) {
    furi_assert(mqtt);
    furi_assert(id);

    MqttClientMessage msg = {
        .type = MqttClientMessageGetSessionId,
        .str_param = id,
        .lock = api_lock_alloc_locked(),
    };

    mg_wakeup(&mqtt->mgr, mqtt->wakeup_conn_id, &msg, sizeof(MqttClientMessage));
    api_lock_wait_unlock_and_free(msg.lock);
}

void mqtt_client_get_session_email(MqttClient* mqtt, FuriString* id) {
    furi_assert(mqtt);
    furi_assert(id);

    MqttClientMessage msg = {
        .type = MqttClientMessageGetSessionEmail,
        .str_param = id,
        .lock = api_lock_alloc_locked(),
    };

    mg_wakeup(&mqtt->mgr, mqtt->wakeup_conn_id, &msg, sizeof(MqttClientMessage));
    api_lock_wait_unlock_and_free(msg.lock);
}

void mqtt_client_publish(MqttClient* mqtt, const char* topic, const void* data, size_t data_size) {
    furi_check(mqtt);
    furi_check(topic);
    furi_check(data);
    furi_check(data_size);

    const MqttClientMessage msg = {
        .type = MqttClientMessagePublish,
        .publish =
            {
                .topic = topic,
                .data = data,
                .data_size = data_size,
            },
        .lock = api_lock_alloc_locked(),
    };

    mg_wakeup(&mqtt->mgr, mqtt->wakeup_conn_id, &msg, sizeof(MqttClientMessage));
    api_lock_wait_unlock_and_free(msg.lock);
}

FuriPubSub* mqtt_client_get_pubsub(MqttClient* mqtt) {
    furi_assert(mqtt);
    return mqtt->event_pubsub;
}
