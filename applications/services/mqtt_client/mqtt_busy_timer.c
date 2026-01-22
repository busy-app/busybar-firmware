#include "mqtt_client_i.h"

#include <busy_timer/busy_timer.h>

#define TAG "MqttBusyTimer"

static void mqtt_busy_timer_pubsub_callback(const void* data, void* context) {
    furi_assert(data);
    furi_assert(context);

    MqttClient* mqtt = context;
    const BusyTimerEvent* event = data;

    if(event->type == BusyTimerEventTypeUserInteracted) {
        char* snapshot_str = busy_timer_snapshot_serialize(&event->user_interacted.snapshot);
        furi_check(snapshot_str);

        mqtt_client_publish(
            mqtt,
            MqttQosAtLeastOnce,
            MQTT_BUSY_TIMER_SNAPSHOT_TOPIC,
            snapshot_str,
            strlen(snapshot_str));

        free(snapshot_str);
    }
}

void mqtt_busy_timer_init(MqttClient* mqtt) {
    BusyTimer* busy_timer = furi_record_open(RECORD_BUSY_TIMER);
    FuriPubSub* pubsub = busy_timer_get_pubsub(busy_timer);
    furi_pubsub_subscribe(pubsub, mqtt_busy_timer_pubsub_callback, mqtt);
}

void mqtt_busy_timer_on_message(
    MqttClient* mqtt,
    const FuriString* topic_str,
    const struct mg_mqtt_message* msg) {
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
