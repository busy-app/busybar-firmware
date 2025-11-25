#include "mqtt_client_i.h"

#include <busy_timer/busy_timer.h>

#define TAG "MqttBusyTimer"

void mqtt_busy_timer_on_message(
    MqttClient* mqtt,
    FuriString* topic_str,
    struct mg_mqtt_message* msg) {
    UNUSED(mqtt);
    UNUSED(topic_str);

    BusyTimerSnapshot snapshot;

    if(busy_timer_snapshot_deserialize(&snapshot, msg->data.buf)) {
        BusyTimer* busy_timer = furi_record_open(RECORD_BUSY_TIMER);
        busy_timer_set_snapshot(busy_timer, &snapshot);
        furi_record_close(RECORD_BUSY_TIMER);
    } else {
        FURI_LOG_E(TAG, "Failed to parse timer snapshot");
    }
}
