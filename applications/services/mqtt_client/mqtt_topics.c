#include "mqtt_client_i.h"

#define TAG "MqttTopics"

void mqtt_topics_subscribe(MqttClient* mqtt) {
    furi_assert(mqtt->conn);

    FuriString* topic = furi_string_alloc_printf(
        "%s/%s/down/%s/#",
        MQTT_API_ROOT_TOPIC,
        furi_string_get_cstr(mqtt->session_id),
        MQTT_API_VERSION);
    const struct mg_mqtt_opts opts = {
        .topic = mg_str(furi_string_get_cstr(topic)), .qos = MQTT_QOS};
    mg_mqtt_sub(mqtt->conn, &opts);

    furi_string_free(topic);
}

void mqtt_topics_on_message(MqttClient* mqtt, FuriString* topic_str, struct mg_mqtt_message* msg) {
    if(furi_string_end_with(topic_str, "http-request")) {
        mqtt_http_api_on_message(mqtt, topic_str, msg);
    } else if(furi_string_end_with(topic_str, "stream-request")) {
        mqtt_screen_streaming_on_message(mqtt, topic_str, msg);
    } else {
        FURI_LOG_W(TAG, "Unknown topic %s", furi_string_get_cstr(topic_str));
        return;
    }
}
