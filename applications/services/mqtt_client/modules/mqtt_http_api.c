#include "../mqtt_client_i.h"

#define TAG "MqttApi"

#define HTTP_HOST           "http://127.0.0.1"
#define HTTP_URI_API_PREFIX "/api/"
#define HTTP_CONN_TIMEOUT   (5000) // In ms

typedef enum {
    MqttHttpApiMethodIdGet,
    MqttHttpApiMethodIdPost,
    MqttHttpApiMethodIdDelete,
    MqttHttpApiMethodIdMax,
} MqttHttpApiMethodId;

typedef struct {
    const char* name;
} MqttHttpApiAllowListEntry;

typedef struct {
    const char* name;
    MqttHttpApiMethodId id;
} MqttHttpApiBlocklistEntry;

typedef struct {
    MqttClient* mqtt;
    FuriString* response_topic;
    FuriString* correlation_data;
    uint8_t* data;
    size_t data_size;
    uint32_t poll_cnt;
} MqttHttpContext;

static const MqttHttpApiAllowListEntry mqtt_api_allowlist[MqttHttpApiMethodIdMax] = {
    [MqttHttpApiMethodIdGet] =
        {
            .name = "GET",
        },
    [MqttHttpApiMethodIdPost] =
        {
            .name = "POST",
        },
    [MqttHttpApiMethodIdDelete] =
        {
            .name = "DELETE",
        },
};

static const MqttHttpApiBlocklistEntry mqtt_api_blocklist[] = {
    {
        .name = "update",
        .id = MqttHttpApiMethodIdPost,
    },
};

static MqttHttpContext*
    mqtt_http_context_alloc(MqttClient* mqtt, const void* data, size_t data_size) {
    MqttHttpContext* context = malloc(sizeof(MqttHttpContext));

    context->mqtt = mqtt;
    context->response_topic = furi_string_alloc();
    context->correlation_data = furi_string_alloc();
    context->poll_cnt = HTTP_CONN_TIMEOUT / MQTT_POLL_PERIOD;

    if(data_size) {
        context->data = malloc(data_size);
        context->data_size = data_size;
        memcpy(context->data, data, data_size);
    }

    return context;
}

static void mqtt_http_context_free(MqttHttpContext* context) {
    furi_string_free(context->response_topic);
    furi_string_free(context->correlation_data);

    if(context->data) {
        free(context->data);
    }

    free(context);
}

static bool mqtt_http_context_has_valid_props(const MqttHttpContext* context) {
    bool is_valid = false;

    do {
        if(furi_string_empty(context->response_topic)) {
            break;
        }
        if(furi_string_empty(context->correlation_data)) {
            break;
        }
        is_valid = true;
    } while(false);

    return is_valid;
}

static bool mqtt_http_api_is_valid_uri(const struct mg_http_message* http_msg) {
    bool is_valid = false;

    do {
        const struct mg_str uri = http_msg->uri;
        const size_t prefix_len = strlen(HTTP_URI_API_PREFIX);

        if(uri.len < prefix_len) {
            break;
        }

        if(strncmp(uri.buf, HTTP_URI_API_PREFIX, prefix_len) != 0) {
            break;
        };

        is_valid = true;
    } while(false);

    return is_valid;
}

static bool mqtt_http_api_method_is_blocked(const struct mg_http_message* http_msg) {
    bool is_blocked = false;

    struct mg_str uri_mask = http_msg->uri;
    const size_t prefix_len = strlen(HTTP_URI_API_PREFIX);
    // Remove URI prefix
    uri_mask.buf += prefix_len;
    uri_mask.len -= prefix_len;
    // Remove possible trailing slash
    if(uri_mask.buf[uri_mask.len - 1] == '/') {
        uri_mask.len -= 1;
    }

    for(uint32_t i = 0; i < COUNT_OF(mqtt_api_blocklist); i++) {
        const MqttHttpApiBlocklistEntry* const block_entry = &mqtt_api_blocklist[i];

        if(mg_strcmp(uri_mask, mg_str(block_entry->name)) == 0) {
            MqttHttpApiMethodId found_id;
            for(found_id = 0; found_id < MqttHttpApiMethodIdMax; ++found_id) {
                const MqttHttpApiAllowListEntry* const allow_entry = &mqtt_api_allowlist[found_id];
                if(mg_strcmp(http_msg->method, mg_str(allow_entry->name)) == 0) {
                    break;
                }
            }

            if(found_id == MqttHttpApiMethodIdMax) {
                continue;
            }

            if(found_id == block_entry->id) {
                is_blocked = true;
                break;
            }
        }
    }

    return is_blocked;
}

static bool mqtt_http_api_is_websocket_upgrade(const struct mg_http_message* http_msg) {
    bool is_websocket = false;

    const struct mg_str* hdr = mg_http_get_header((struct mg_http_message*)http_msg, "Connection");

    if(mg_match(http_msg->method, mg_str("GET"), NULL) && (hdr != NULL)) {
        if(mg_strcasecmp(*hdr, mg_str("upgrade")) == 0) {
            is_websocket = true;
        }
    }

    return is_websocket;
}

static bool mqtt_http_context_has_valid_request(const MqttHttpContext* context) {
    bool is_valid = false;

    do {
        struct mg_http_message http_msg;

        const int req_len =
            mg_http_parse((const char*)context->data, context->data_size, &http_msg);
        if(req_len <= 0) {
            break;
        }
        if(!mqtt_http_api_is_valid_uri(&http_msg)) {
            break;
        }
        if(mqtt_http_api_method_is_blocked(&http_msg)) {
            break;
        }
        if(mqtt_http_api_is_websocket_upgrade(&http_msg)) {
            break;
        }

        is_valid = true;
    } while(false);

    return is_valid;
}

static void mqtt_api_http_handler(struct mg_connection* conn, int ev, void* ev_data) {
    MqttHttpContext* http_ctx = conn->fn_data;
    furi_assert(http_ctx);

    if(ev == MG_EV_CONNECT) {
        mg_send(conn, http_ctx->data, http_ctx->data_size);

    } else if(ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* msg = (struct mg_http_message*)ev_data;
        FURI_LOG_T(TAG, "HTTP resp: %.*s", (int)msg->body.len, msg->body.buf);

        if(http_ctx->mqtt->conn) {
            struct mg_mqtt_prop props[] = {
                {
                    .id = MQTT_PROP_CORRELATION_DATA,
                    .val = mg_str(furi_string_get_cstr(http_ctx->correlation_data)),
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

        mqtt_http_context_free(http_ctx);
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

static void mqtt_http_api_respond_error(const MqttHttpContext* http_ctx) {
    const struct mg_mqtt_prop props[] = {
        {
            .id = MQTT_PROP_CORRELATION_DATA,
            .val = mg_str(furi_string_get_cstr(http_ctx->correlation_data)),
        },
    };

    const struct mg_mqtt_opts pub_opts = {
        .topic = mg_str(furi_string_get_cstr(http_ctx->response_topic)),
        .message = mg_str("HTTP/1.1 422 Unprocessable Entity\r\n\r\n"),
        .qos = MQTT_QOS,
        .retain = false,
        .props = (struct mg_mqtt_prop*)props,
        .num_props = COUNT_OF(props),
    };

    if(http_ctx->mqtt->conn) {
        mg_mqtt_pub(http_ctx->mqtt->conn, &pub_opts);
    }
}

void mqtt_http_api_on_message(
    MqttClient* mqtt,
    const FuriString* topic_str,
    const struct mg_mqtt_message* msg) {
    UNUSED(topic_str);

    bool success = false;

    MqttHttpContext* http_ctx = mqtt_http_context_alloc(mqtt, msg->data.buf, msg->data.len);

    do {
        for(size_t prop_offs = 0;;) {
            struct mg_mqtt_prop prop = {};
            // NOTE: mg_mqtt_next_prop() does NOT mutate data pointed to by *msg
            prop_offs = mg_mqtt_next_prop((struct mg_mqtt_message*)msg, &prop, prop_offs);

            if(prop_offs <= 0) {
                break;

            } else if(prop.id == MQTT_PROP_RESPONSE_TOPIC) {
                if(prop.val.len > 0) {
                    furi_string_printf(
                        http_ctx->response_topic, "%.*s", prop.val.len, prop.val.buf);
                }

            } else if(prop.id == MQTT_PROP_CORRELATION_DATA) {
                if(prop.val.len > 0) {
                    furi_string_printf(
                        http_ctx->correlation_data, "%.*s", prop.val.len, prop.val.buf);
                }
            }
        }

        if(!mqtt_http_context_has_valid_props(http_ctx)) {
            FURI_LOG_W(TAG, "Missing msg properties");
            break;
        }

        if(!mqtt_http_context_has_valid_request(http_ctx)) {
            FURI_LOG_W(TAG, "Bad request");
            mqtt_http_api_respond_error(http_ctx);
            break;
        }

        success = true;
    } while(false);

    if(success) {
        mg_http_connect(&mqtt->mgr, HTTP_HOST, mqtt_api_http_handler, http_ctx);
    } else {
        mqtt_http_context_free(http_ctx);
    }
}
