#include "mqtt_client_i.h"

#define TAG "MqttTopics"

void mqtt_topics_on_message(
    MqttClient* mqtt,
    const FuriString* topic_str,
    const struct mg_mqtt_message* msg) {
    if(furi_string_end_with(topic_str, "http-request")) {
        mqtt_http_api_on_message(mqtt, topic_str, msg);
    } else if(furi_string_end_with(topic_str, "stream-request")) {
        mqtt_screen_streaming_on_message(mqtt, topic_str, msg);
    } else if(furi_string_end_with(topic_str, MQTT_BUSY_TIMER_SNAPSHOT_TOPIC)) {
        mqtt_busy_timer_on_message(mqtt, topic_str, msg);
    } else {
        FURI_LOG_W(TAG, "Unknown topic %s", furi_string_get_cstr(topic_str));
    }
}

void mqtt_topics_on_close(MqttClient* mqtt) {
    mqtt_screen_streaming_on_close(mqtt);
}
