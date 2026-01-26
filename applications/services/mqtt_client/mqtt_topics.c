#include "mqtt_client_i.h"

#define TAG "MqttTopics"

bool mqtt_topics_on_message(
    MqttClient* mqtt,
    const FuriString* topic_str,
    const struct mg_mqtt_message* msg) {
    bool consumed = false;

    if(furi_string_end_with(topic_str, "http-request")) {
        mqtt_http_api_on_message(mqtt, topic_str, msg);
        consumed = true;

    } else if(furi_string_end_with(topic_str, "stream-request")) {
        mqtt_screen_streaming_on_message(mqtt, topic_str, msg);
        consumed = true;
    }

    return consumed;
}

void mqtt_topics_on_close(MqttClient* mqtt) {
    mqtt_screen_streaming_on_close(mqtt);
}
