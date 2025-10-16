#include "mqtt_client_i.h"

#define TAG "MqttApi"

#define HTTP_HOST           "http://127.0.0.1"
#define HTTP_URI_API_PREFIX "/api/"
#define HTTP_CONN_TIMEOUT   (5000) // In ms

typedef enum {
    MethodGet = 0,
    MethodPost,
    MethodDelete,
    MethodUnknown,
} MqttApiMethod;

static const struct {
    char* http_name;
} mqtt_api_methods[] = {
    [MethodGet] = {"GET"},
    [MethodPost] = {"POST"},
    [MethodDelete] = {"DELETE"},
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

static bool mqtt_api_http_check_request(struct mg_str* msg) {
    struct mg_http_message http_msg;
    if(mg_http_parse(msg->buf, msg->len, &http_msg) < 0) {
        return false;
    }
    struct mg_str uri_mask = http_msg.uri;
    size_t prefix_len = strlen(HTTP_URI_API_PREFIX);

    if(uri_mask.len < prefix_len) return false;
    if(strncmp(uri_mask.buf, HTTP_URI_API_PREFIX, prefix_len) != 0) return false;

    uri_mask.buf += prefix_len;
    uri_mask.len -= prefix_len;

    if(uri_mask.buf[uri_mask.len - 1] == '/') {
        uri_mask.len -= 1;
    }

    bool success = false;
    for(size_t i = 0; i < COUNT_OF(mqtt_api_table); i++) {
        if(mg_strcmp(uri_mask, mg_str(mqtt_api_table[i].name)) == 0) {
            MqttApiMethod method = MethodUnknown;
            for(size_t method_id = 0; method_id < COUNT_OF(mqtt_api_methods); method_id++) {
                if(mg_strcmp(http_msg.method, mg_str(mqtt_api_methods[method_id].http_name)) ==
                   0) {
                    method = method_id;
                    break;
                }
            }
            if((method == MethodUnknown) || (method != mqtt_api_table[i].method)) {
                continue;
            }
            success = true;
        }
    }

    return success;
}

typedef struct {
    MqttClient* mqtt;
    FuriString* response_topic;
    FuriString* cor_data;
    char* request_data;
    size_t request_len;
    uint32_t poll_cnt;
} MqttHttpContext;

static void mqtt_api_http_handler(struct mg_connection* conn, int ev, void* ev_data) {
    MqttHttpContext* http_ctx = conn->fn_data;
    furi_assert(http_ctx);

    if(ev == MG_EV_CONNECT) {
        mg_send(conn, http_ctx->request_data, http_ctx->request_len);
    } else if(ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* msg = (struct mg_http_message*)ev_data;
        FURI_LOG_T(TAG, "HTTP resp: %.*s", (int)msg->body.len, msg->body.buf);

        if(http_ctx->mqtt->conn) {
            struct mg_mqtt_prop props[] = {
                {
                    .id = MQTT_PROP_CORRELATION_DATA,
                    .val = mg_str(furi_string_get_cstr(http_ctx->cor_data)),
                },
            };
            struct mg_mqtt_opts pub_opts = {
                .topic = mg_str(furi_string_get_cstr(http_ctx->response_topic)),
                .message = msg->message,
                .qos = MQTT_QOS,
                .retain = false,
                .props = props,
                .num_props = COUNT_OF(props),
            };
            mg_mqtt_pub(http_ctx->mqtt->conn, &pub_opts);
        } else {
            FURI_LOG_W(TAG, "MQTT died before response is sent");
        }
        conn->is_draining = 1;
    } else if(ev == MG_EV_CLOSE) {
        FURI_LOG_T(TAG, "HTTP close");

        furi_string_free(http_ctx->response_topic);
        furi_string_free(http_ctx->cor_data);
        if(http_ctx->request_data) {
            free(http_ctx->request_data);
        }
        free(http_ctx);
        conn->fn_data = NULL;
    } else if(ev == MG_EV_POLL) {
        if(http_ctx->poll_cnt > 0) {
            http_ctx->poll_cnt--;
            if(http_ctx->poll_cnt == 0) {
                // Should never happen with correct HTTP request
                FURI_LOG_E(TAG, "HTTP timeout");
                conn->is_draining = 1;
            }
        }
    }
}

void mqtt_http_api_on_message(MqttClient* mqtt, FuriString* topic_str, struct mg_mqtt_message* msg) {
    UNUSED(topic_str);

    if(!mqtt_api_http_check_request(&msg->data)) {
        FURI_LOG_W(TAG, "Bad request");
        return;
    }

    FuriString* cor_data = furi_string_alloc();
    FuriString* resp_topic = furi_string_alloc();
    struct mg_mqtt_prop prop;
    size_t prop_ofs = 0;
    do {
        memset(&prop, 0, sizeof(struct mg_mqtt_prop));
        prop_ofs = mg_mqtt_next_prop(msg, &prop, prop_ofs);
        if(prop_ofs <= 0) break;

        if(prop.id == MQTT_PROP_RESPONSE_TOPIC) {
            if(prop.val.len > 0) {
                furi_string_printf(resp_topic, "%.*s", prop.val.len, prop.val.buf);
            }
        } else if(prop.id == MQTT_PROP_CORRELATION_DATA) {
            if(prop.val.len > 0) {
                furi_string_printf(cor_data, "%.*s", prop.val.len, prop.val.buf);
            }
        }
    } while(prop_ofs > 0);

    if((furi_string_empty(cor_data)) || (furi_string_empty(resp_topic))) {
        FURI_LOG_W(TAG, "Missing msg properties");
        furi_string_free(cor_data);
        furi_string_free(resp_topic);
        return;
    }

    MqttHttpContext* http_ctx = malloc(sizeof(MqttHttpContext));
    http_ctx->mqtt = mqtt;
    http_ctx->poll_cnt = HTTP_CONN_TIMEOUT / MQTT_POLL_PERIOD;
    http_ctx->response_topic = resp_topic;
    http_ctx->cor_data = cor_data;
    http_ctx->request_len = msg->data.len;
    http_ctx->request_data = malloc(http_ctx->request_len);
    memcpy(http_ctx->request_data, msg->data.buf, http_ctx->request_len);

    mg_http_connect(&mqtt->mgr, HTTP_HOST, mqtt_api_http_handler, http_ctx);
}
