#include "../mqtt_client_i.h"

#include <network/network.h>

#define TAG "MqttHttpProxySrv"

#define POLL_PERIOD_MS (500)

#define HTTP_HOST           "http://127.0.0.1"
#define HTTP_URI_API_PREFIX "/api/"
#define HTTP_CONN_TIMEOUT   (5000) // In ms

#define SUB_QOS (MqttQosExactlyOnce)
#define PUB_QOS (MqttQosExactlyOnce)

#define SUB_TOPIC "http-request"

typedef enum {
    MqttHttpApiMethodIdGet,
    MqttHttpApiMethodIdPost,
    MqttHttpApiMethodIdDelete,
    MqttHttpApiMethodIdMax,
} MqttHttpApiMethodId;

typedef struct {
    const char* name;
    MqttHttpApiMethodId id;
} MqttHttpApiBlocklistEntry;

typedef struct {
    MqttClient* mqtt;
    struct mg_mgr mgr;
    struct mg_timer timer;
    unsigned long api_connection_id;
} MqttHttpApi;

typedef struct {
    MqttClient* mqtt;
    FuriString* response_topic;
    FuriString* correlation_data;
    uint8_t* data;
    size_t data_size;
    uint32_t poll_cnt;
} MqttHttpApiRequest;

static const char* mqtt_http_method_names[MqttHttpApiMethodIdMax] = {
    [MqttHttpApiMethodIdGet] = "GET",
    [MqttHttpApiMethodIdPost] = "POST",
    [MqttHttpApiMethodIdDelete] = "DELETE",
};

static const MqttHttpApiBlocklistEntry mqtt_api_blocklist[] = {
    {
        .name = "update",
        .id = MqttHttpApiMethodIdPost,
    },
};

static MqttHttpApiMethodId mqtt_http_api_get_method_id_by_name(const struct mg_str name) {
    MqttHttpApiMethodId found_id;

    for(found_id = 0; found_id < MqttHttpApiMethodIdMax; ++found_id) {
        if(mg_strcmp(name, mg_str(mqtt_http_method_names[found_id])) == 0) {
            break;
        }
    }

    return found_id;
}

static MqttHttpApiRequest*
    mqtt_http_api_request_alloc(MqttClient* mqtt, const MqttMessage* message) {
    MqttHttpApiRequest* context = malloc(sizeof(MqttHttpApiRequest));

    context->mqtt = mqtt;
    context->response_topic = furi_string_alloc();
    context->correlation_data = furi_string_alloc();
    context->poll_cnt = HTTP_CONN_TIMEOUT / MQTT_POLL_PERIOD;

    size_t data_size;
    const void* data = mqtt_message_get_data(message, &data_size);

    if(data_size) {
        context->data = malloc(data_size);
        context->data_size = data_size;
        memcpy(context->data, data, data_size);
    }

    mqtt_message_get_string_property(
        message, MqttPropertyTypeResponseTopic, context->response_topic);
    mqtt_message_get_string_property(
        message, MqttPropertyTypeCorrelationData, context->correlation_data);

    return context;
}

static void mqtt_http_api_request_free(MqttHttpApiRequest* context) {
    furi_string_free(context->response_topic);
    furi_string_free(context->correlation_data);

    if(context->data) {
        free(context->data);
    }

    free(context);
}

static bool mqtt_http_api_request_has_valid_props(const MqttHttpApiRequest* context) {
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
            const MqttHttpApiMethodId id = mqtt_http_api_get_method_id_by_name(http_msg->method);

            if(id != MqttHttpApiMethodIdMax) {
                if(id == block_entry->id) {
                    is_blocked = true;
                    break;
                }
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

static bool mqtt_http_api_request_has_valid_request(const MqttHttpApiRequest* context) {
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
    MqttHttpApiRequest* request = conn->fn_data;
    furi_assert(request);

    if(ev == MG_EV_CONNECT) {
        mg_send(conn, request->data, request->data_size);

    } else if(ev == MG_EV_HTTP_MSG) {
        const struct mg_http_message* msg = (const struct mg_http_message*)ev_data;
        FURI_LOG_T(TAG, "HTTP resp: %.*s", msg->body.len, msg->body.buf);

        const MqttProperty props[] = {
            {
                .type = MqttPropertyTypeCorrelationData,
                .value.string = furi_string_get_cstr(request->correlation_data),
            },
        };

        mqtt_client_publish_ex(
            request->mqtt,
            MqttQosAtLeastOnce,
            furi_string_get_cstr(request->response_topic),
            msg->message.buf,
            msg->message.len,
            props,
            COUNT_OF(props));

        conn->is_draining = 1;

    } else if(ev == MG_EV_CLOSE) {
        FURI_LOG_T(TAG, "HTTP connection closed");

        mqtt_http_api_request_free(request);
        conn->fn_data = NULL;

    } else if(ev == MG_EV_POLL) {
        if(request->poll_cnt > 0) {
            request->poll_cnt--;
            if(request->poll_cnt == 0) {
                // Should never happen with correct HTTP request
                FURI_LOG_E(TAG, "HTTP timeout");
                conn->is_draining = 1;
            }
        }
    }
}

static void mqtt_http_api_respond_error(const MqttHttpApiRequest* request) {
    const char* message = "HTTP/1.1 422 Unprocessable Entity\r\n\r\n";

    const MqttProperty props[] = {
        {
            .type = MqttPropertyTypeCorrelationData,
            .value.string = furi_string_get_cstr(request->correlation_data),
        },
    };

    mqtt_client_publish_ex(
        request->mqtt,
        MqttQosAtLeastOnce,
        furi_string_get_cstr(request->response_topic),
        message,
        strlen(message),
        props,
        COUNT_OF(props));
}

static void mqtt_http_api_message_callback(const MqttMessage* message, void* context) {
    furi_assert(message);
    furi_assert(context);
    MqttHttpApi* instance = context;

    MqttHttpApiRequest* request = mqtt_http_api_request_alloc(instance->mqtt, message);
    mg_wakeup(&instance->mgr, instance->api_connection_id, &request, sizeof(MqttHttpApiRequest*));
}

static bool mqtt_http_api_process_request(MqttHttpApi* instance, MqttHttpApiRequest* request) {
    bool success = false;

    do {
        if(!mqtt_http_api_request_has_valid_props(request)) {
            FURI_LOG_W(TAG, "Missing msg properties");
            break;
        }

        if(!mqtt_http_api_request_has_valid_request(request)) {
            FURI_LOG_W(TAG, "Bad request");
            mqtt_http_api_respond_error(request);
            break;
        }

        mg_http_connect(&instance->mgr, HTTP_HOST, mqtt_api_http_handler, request);

        success = true;
    } while(false);

    return success;
}

static void
    mqtt_http_api_event_callback(struct mg_connection* connection, int event, void* event_data) {
    if(event == MG_EV_WAKEUP) {
        MqttHttpApi* instance = connection->fn_data;
        furi_assert(instance);

        const struct mg_str* data = event_data;
        furi_assert(data);
        furi_assert(data->buf);
        furi_assert(data->len == sizeof(MqttHttpApiRequest*));

        MqttHttpApiRequest* request = *(MqttHttpApiRequest**)data->buf;

        if(!mqtt_http_api_process_request(instance, request)) {
            mqtt_http_api_request_free(request);
        }
    }
}

static MqttHttpApi* mqtt_http_api_alloc(void) {
    MqttHttpApi* instance = malloc(sizeof(MqttHttpApi));
    instance->mqtt = furi_record_open(RECORD_MQTT);

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);

    mg_mgr_init(&instance->mgr);
    mg_wakeup_init(&instance->mgr);

    const struct mg_connection* api_connnection =
        mg_wrapfd(&instance->mgr, MG_INVALID_SOCKET, mqtt_http_api_event_callback, instance);
    instance->api_connection_id = api_connnection->id;

    mqtt_subscribe(instance->mqtt, SUB_QOS, SUB_TOPIC, mqtt_http_api_message_callback, instance);
    return instance;
}

int32_t mqtt_http_proxy_srv(void* arg) {
    UNUSED(arg);

    MqttHttpApi* instance = mqtt_http_api_alloc();

    for(;;) {
        mg_mgr_poll(&instance->mgr, POLL_PERIOD_MS);
    }

    return 0;
}
