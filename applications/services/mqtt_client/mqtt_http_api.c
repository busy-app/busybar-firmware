#include "mqtt_client_i.h"

#define TAG "MqttApi"

#define HTTP_HOST           "http://127.0.0.1"
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
    [MethodGet] = {"/get", "GET"},
    [MethodPost] = {"/post", "POST"},
    [MethodDelete] = {"/delete", "DELETE"},
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

    {"assets/upload", MethodPost},
    {"assets/upload", MethodDelete},

    {"storage/write", MethodPost},
    {"storage/read", MethodGet},
    {"storage/list", MethodGet},
    {"storage/delete", MethodDelete},
    {"storage/mkdir", MethodPost},
};

static bool mqtt_api_parse_topic(
    MqttClient* mqtt,
    FuriString* topic_str,
    FuriString* http_req,
    FuriString* response_topic) {
    furi_assert(topic_str);
    furi_assert(http_req);
    furi_assert(response_topic);
    bool success = false;

    // sessions/<session_id>/down/v1/#

    FuriString* topic = furi_string_alloc_set(topic_str);
    size_t prefix_len =
        strlen(MQTT_API_ROOT_TOPIC) + 1 + furi_string_size(mqtt->session_id) + strlen("/down/v1/");
    furi_string_right(topic, prefix_len);

    for(size_t i = 0; i < COUNT_OF(mqtt_api_table); i++) {
        if(furi_string_start_with(topic, mqtt_api_table[i].name)) {
            size_t method_offset = strlen(mqtt_api_table[i].name);
            const char* method_str = furi_string_get_cstr(topic) + method_offset;
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
            furi_string_right(topic, method_offset + method_len);

            if(furi_string_size(topic) > 1) {
                furi_string_right(topic, 1);
                furi_string_printf(
                    http_req,
                    "%s %s%s?%s",
                    mqtt_api_methods[method].http_name,
                    HTTP_URI_API_PREFIX,
                    mqtt_api_table[i].name,
                    furi_string_get_cstr(topic));
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
                "%s/%s/up/%s/%s%s",
                MQTT_API_ROOT_TOPIC,
                furi_string_get_cstr(mqtt->session_id),
                MQTT_API_VERSION,
                mqtt_api_table[i].name,
                mqtt_api_methods[method].mqtt_name);

            success = true;
        }
    }

    furi_string_free(topic);
    return success;
}

typedef struct {
    MqttClient* mqtt;
    FuriString* response_topic;
    FuriString* http_request;
    char* request_data;
    size_t request_len;
} MqttHttpContext;

static void mqtt_api_http_handler(struct mg_connection* conn, int ev, void* ev_data) {
    MqttHttpContext* http_ctx = conn->fn_data;
    furi_assert(http_ctx);

    if(ev == MG_EV_CONNECT) {
        mg_printf(
            conn,
            "%s HTTP/1.1\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %ld\r\n"
            "\r\n",
            furi_string_get_cstr(http_ctx->http_request),
            http_ctx->request_len);
        if(http_ctx->request_len > 0) {
            mg_send(conn, http_ctx->request_data, http_ctx->request_len);
        }
    } else if(ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* msg = (struct mg_http_message*)ev_data;
        FURI_LOG_T(TAG, "HTTP resp: %.*s", (int)msg->body.len, msg->body.buf);

        if(http_ctx->mqtt->conn) {
            struct mg_mqtt_opts pub_opts = {
                .topic = mg_str(furi_string_get_cstr(http_ctx->response_topic)),
                .message = msg->body,
                .qos = MQTT_QOS,
                .retain = false,
            };
            mg_mqtt_pub(http_ctx->mqtt->conn, &pub_opts);
        } else {
            FURI_LOG_W(TAG, "MQTT died before response is sent");
        }
        conn->is_draining = 1;
    } else if(ev == MG_EV_CLOSE) {
        FURI_LOG_T(TAG, "HTTP close");

        furi_string_free(http_ctx->response_topic);
        furi_string_free(http_ctx->http_request);
        if(http_ctx->request_data) {
            free(http_ctx->request_data);
        }
        free(http_ctx);
        conn->fn_data = NULL;
    }
}

void mqtt_api_subscribe(MqttClient* mqtt) {
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

void mqtt_api_on_message(MqttClient* mqtt, FuriString* topic_str, struct mg_str* message) {
    FuriString* http_req = furi_string_alloc();
    FuriString* resp_topic = furi_string_alloc();
    if(!mqtt_api_parse_topic(mqtt, topic_str, http_req, resp_topic)) {
        FURI_LOG_W(TAG, "Unknown topic %s", furi_string_get_cstr(topic_str));
        furi_string_free(http_req);
        furi_string_free(resp_topic);
        return;
    }

    MqttHttpContext* http_ctx = malloc(sizeof(MqttHttpContext));
    http_ctx->mqtt = mqtt;
    http_ctx->response_topic = resp_topic;
    http_ctx->http_request = http_req;
    http_ctx->request_len = message->len;
    if(http_ctx->request_len > 0) {
        http_ctx->request_data = malloc(message->len);
        memcpy(http_ctx->request_data, message->buf, message->len);
    }

    mg_http_connect(&mqtt->mgr, HTTP_HOST, mqtt_api_http_handler, http_ctx);
}
