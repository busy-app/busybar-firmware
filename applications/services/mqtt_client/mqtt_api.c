#include "mqtt_i.h"

#define TAG                 "MqttApi"
#define MQTT_TOPIC_PREFIX   "api/"
#define HTTP_URI_API_PREFIX "/api/"

typedef enum {
    MethodGet = 0,
    MethodPost,
    MethodDelete,
    MethodUnknown,
} MqttApiMethod;

static const struct {
    char* mqtt_name;
    char* http_name;
} mqtt_api_methods[] = {
    [MethodGet] = {"/get/", "GET"},
    [MethodPost] = {"/post/", "POST"},
    [MethodDelete] = {"/delete/", "DELETE"},
};

static const struct {
    char* name;
    MqttApiMethod method;
} mqtt_api_table[] = {
    {"version", MethodGet},
    {"status", MethodGet},
    {"status/system", MethodGet},
    {"status/power", MethodGet},

    {"display/draw", MethodPost},
    {"display/draw", MethodDelete},
    {"audio/play", MethodPost},
    {"audio/play", MethodDelete},

    {"display/brightness", MethodGet},
    {"display/brightness", MethodPost},
    {"audio/volume", MethodGet},
    {"audio/volume", MethodPost},

    {"input", MethodPost},
};

bool mqtt_parse_topic(struct mg_str* topic, FuriString* http_req, FuriString* response_topic) {
    furi_assert(topic);
    furi_assert(http_req);
    furi_assert(response_topic);
    bool success = false;

    FuriString* topic_str = furi_string_alloc_printf("%.*s", topic->len, topic->buf);

    if((!furi_string_start_with(topic_str, MQTT_TOPIC_PREFIX)) ||
       (furi_string_end_with(topic_str, "/response"))) {
        furi_string_free(topic_str);
        return false;
    }
    furi_string_right(topic_str, strlen(MQTT_TOPIC_PREFIX));

    for(size_t i = 0; i < COUNT_OF(mqtt_api_table); i++) {
        if(furi_string_start_with(topic_str, mqtt_api_table[i].name)) {
            size_t method_offset = strlen(mqtt_api_table[i].name);
            const char* method_str = furi_string_get_cstr(topic_str) + method_offset;
            MqttApiMethod method = MethodUnknown;
            size_t method_len = 0;
            for(size_t i = 0; i < COUNT_OF(mqtt_api_methods); i++) {
                method_len = strlen(mqtt_api_methods[i].mqtt_name);
                if(strncmp(mqtt_api_methods[i].mqtt_name, method_str, method_len) == 0) {
                    method = i;
                    break;
                }
            }
            if((method == MethodUnknown) || (method != mqtt_api_table[i].method)) {
                continue;
            }
            furi_string_right(topic_str, method_offset + method_len);
            // TODO: ID ?
            if(!furi_string_start_with(topic_str, "request")) {
                break;
            }
            furi_string_right(topic_str, strlen("request"));

            if(furi_string_size(topic_str) > 1) {
                furi_string_right(topic_str, 1);
                furi_string_printf(
                    http_req,
                    "%s %s%s?%s",
                    mqtt_api_methods[method].http_name,
                    HTTP_URI_API_PREFIX,
                    mqtt_api_table[i].name,
                    furi_string_get_cstr(topic_str));
            } else {
                furi_string_printf(
                    http_req,
                    "%s %s%s",
                    mqtt_api_methods[method].http_name,
                    HTTP_URI_API_PREFIX,
                    mqtt_api_table[i].name);
            }

            furi_string_printf(
                response_topic,
                "%s%s%sresponse",
                MQTT_TOPIC_PREFIX,
                mqtt_api_table[i].name,
                mqtt_api_methods[method].mqtt_name);

            success = true;
        }
    }

    furi_string_free(topic_str);
    return success;
}
