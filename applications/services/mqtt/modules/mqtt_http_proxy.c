#include "mqtt_http_proxy_i.h"

#include <network/network.h>

#define TAG "MqttHttpProxy"

#define POLL_PERIOD_MS (1000)

#define HTTP_HOST            "http://127.0.0.1"
#define HTTP_URI_API_PREFIX  "/api/"
#define HTTP_CONN_TIMEOUT_MS (5000)

#define SUB_QOS (MqttQosExactlyOnce)
#define PUB_QOS (MqttQosExactlyOnce)

#define SUB_TOPIC "http-request"

static const char* mqtt_http_proxy_method_names[MqttHttpProxyMethodIdMax] = {
    [MqttHttpProxyMethodIdGet] = "GET",
    [MqttHttpProxyMethodIdPost] = "POST",
    [MqttHttpProxyMethodIdPut] = "PUT",
    [MqttHttpProxyMethodIdDelete] = "DELETE",
};

static const MqttHttpProxyBlocklistEntry mqtt_http_proxy_blocklist[] = {
    {
        .name = "update",
        .id = MqttHttpProxyMethodIdPost,
    },
    {
        .name = "account",
        .id = MqttHttpProxyMethodIdDelete,
    },
    {
        .name = "account/link",
        .id = MqttHttpProxyMethodIdPost,
    },
    {
        .name = "account/backend",
        .id = MqttHttpProxyMethodIdPut,
    },
    {
        .name = "wifi/connect",
        .id = MqttHttpProxyMethodIdPost,
    },
    {
        .name = "wifi/disconnect",
        .id = MqttHttpProxyMethodIdPost,
    },
    {
        .name = "wifi/networks",
        .id = MqttHttpProxyMethodIdGet,
    },
};

static MqttHttpProxyMethodId mqtt_http_proxy_get_method_id_by_name(const struct mg_str name) {
    MqttHttpProxyMethodId found_id;

    for(found_id = 0; found_id < MqttHttpProxyMethodIdMax; ++found_id) {
        if(mg_strcmp(name, mg_str(mqtt_http_proxy_method_names[found_id])) == 0) {
            break;
        }
    }

    return found_id;
}

static MqttHttpProxyRequest*
    mqtt_http_proxy_request_alloc(Mqtt* mqtt, const MqttMessage* message) {
    MqttHttpProxyRequest* request = malloc(sizeof(MqttHttpProxyRequest));

    request->mqtt = mqtt;
    request->response_topic = furi_string_alloc();
    request->correlation_data = furi_string_alloc();
    request->start_tick = furi_get_tick();

    size_t data_size;
    const void* data = mqtt_message_get_data(message, &data_size);

    if(data_size) {
        request->data = malloc(data_size);
        request->data_size = data_size;
        memcpy(request->data, data, data_size);
    }

    mqtt_message_get_string_property(
        message, MqttPropertyTypeResponseTopic, request->response_topic);
    mqtt_message_get_string_property(
        message, MqttPropertyTypeCorrelationData, request->correlation_data);

    return request;
}

static void mqtt_http_proxy_request_free(MqttHttpProxyRequest* request) {
    furi_string_free(request->response_topic);
    furi_string_free(request->correlation_data);

    if(request->data) {
        free(request->data);
    }

    free(request);
}

static bool mqtt_http_proxy_request_requires_response(const MqttHttpProxyRequest* request) {
    bool requires_response = false;

    do {
        if(furi_string_empty(request->response_topic)) {
            break;
        }
        if(furi_string_empty(request->correlation_data)) {
            break;
        }
        requires_response = true;
    } while(false);

    return requires_response;
}

static bool mqtt_http_proxy_request_is_expired(const MqttHttpProxyRequest* request) {
    return (furi_get_tick() - request->start_tick) >= HTTP_CONN_TIMEOUT_MS;
}

static bool mqtt_http_proxy_trim_uri(const struct mg_http_message* http_msg, struct mg_str* uri) {
    bool is_valid = false;

    do {
        struct mg_str captures[2];
        if(!mg_match(http_msg->uri, mg_str(HTTP_URI_API_PREFIX "#"), captures)) {
            break;
        }
        // Remove URI prefix
        struct mg_str tmp = captures[0];
        if(tmp.len == 0) {
            break;
        }
        // Remove possible trailing slash
        if(tmp.buf[tmp.len - 1] == '/') {
            tmp.len -= 1;
        }

        *uri = tmp;
        is_valid = true;

    } while(false);

    return is_valid;
}

static bool mqtt_http_proxy_method_is_blocked(
    const struct mg_http_message* http_msg,
    const struct mg_str* uri_mask) {
    bool is_blocked = false;

    for(uint32_t i = 0; i < COUNT_OF(mqtt_http_proxy_blocklist); i++) {
        const MqttHttpProxyBlocklistEntry* const block_entry = &mqtt_http_proxy_blocklist[i];

        if(mg_strcmp(*uri_mask, mg_str(block_entry->name)) == 0) {
            const MqttHttpProxyMethodId id =
                mqtt_http_proxy_get_method_id_by_name(http_msg->method);

            if(id != MqttHttpProxyMethodIdMax) {
                if(id == block_entry->id) {
                    is_blocked = true;
                    break;
                }
            }
        }
    }

    return is_blocked;
}

static bool mqtt_http_proxy_is_websocket_upgrade(const struct mg_http_message* http_msg) {
    bool is_websocket = false;

    const struct mg_str* hdr = mg_http_get_header((struct mg_http_message*)http_msg, "Connection");

    if(mg_match(http_msg->method, mg_str("GET"), NULL) && (hdr != NULL)) {
        if(mg_strcasecmp(*hdr, mg_str("upgrade")) == 0) {
            is_websocket = true;
        }
    }

    return is_websocket;
}

static bool mqtt_http_proxy_request_is_valid(const MqttHttpProxyRequest* request) {
    bool is_valid = false;

    do {
        struct mg_http_message http_msg;

        const int req_len =
            mg_http_parse((const char*)request->data, request->data_size, &http_msg);
        if(req_len <= 0) {
            break;
        }

        struct mg_str uri_mask;
        if(!mqtt_http_proxy_trim_uri(&http_msg, &uri_mask)) {
            break;
        }
        if(mqtt_http_proxy_method_is_blocked(&http_msg, &uri_mask)) {
            break;
        }
        if(mqtt_http_proxy_is_websocket_upgrade(&http_msg)) {
            break;
        }

        is_valid = true;
    } while(false);

    return is_valid;
}

static void mqtt_api_http_handler(struct mg_connection* connection, int event, void* event_data) {
    MqttHttpProxyRequest* request = connection->fn_data;
    furi_assert(request);

    if(event == MG_EV_CONNECT) {
        mg_send(connection, request->data, request->data_size);

    } else if(event == MG_EV_HTTP_MSG) {
        const struct mg_http_message* msg = (const struct mg_http_message*)event_data;
        FURI_LOG_T(TAG, "HTTP resp: %.*s", msg->body.len, msg->body.buf);

        if(mqtt_http_proxy_request_requires_response(request)) {
            const MqttProperty props[] = {
                {
                    .type = MqttPropertyTypeCorrelationData,
                    .value.string = furi_string_get_cstr(request->correlation_data),
                },
            };

            mqtt_publish_ex(
                request->mqtt,
                MqttQosAtLeastOnce,
                furi_string_get_cstr(request->response_topic),
                msg->message.buf,
                msg->message.len,
                props,
                COUNT_OF(props));
        }

        connection->is_draining = 1;

    } else if(event == MG_EV_CLOSE) {
        FURI_LOG_T(TAG, "HTTP connection closed");

        mqtt_http_proxy_request_free(request);
        connection->fn_data = NULL;

    } else if(event == MG_EV_POLL) {
        if(mqtt_http_proxy_request_is_expired(request)) {
            FURI_LOG_E(TAG, "HTTP timeout");
            connection->is_draining = 1;
        }
    }
}

static void mqtt_http_proxy_respond_error(const MqttHttpProxyRequest* request) {
    const char* message = "HTTP/1.1 422 Unprocessable Entity\r\n\r\n";

    const MqttProperty props[] = {
        {
            .type = MqttPropertyTypeCorrelationData,
            .value.string = furi_string_get_cstr(request->correlation_data),
        },
    };

    mqtt_publish_ex(
        request->mqtt,
        MqttQosAtLeastOnce,
        furi_string_get_cstr(request->response_topic),
        message,
        strlen(message),
        props,
        COUNT_OF(props));
}

static void mqtt_http_proxy_message_callback(const MqttMessage* message, void* context) {
    furi_assert(message);
    furi_assert(context);
    MqttHttpProxySrv* instance = context;

    MqttHttpProxyRequest* request = mqtt_http_proxy_request_alloc(instance->mqtt, message);
    mg_wakeup(
        &instance->mgr, instance->api_connection_id, &request, sizeof(MqttHttpProxyRequest*));
}

static void
    mqtt_http_proxy_process_request(MqttHttpProxySrv* instance, MqttHttpProxyRequest* request) {
    bool success = false;

    do {
        if(!mqtt_http_proxy_request_is_valid(request)) {
            FURI_LOG_E(TAG, "Bad request");
            break;
        }

        if(mg_http_connect(&instance->mgr, HTTP_HOST, mqtt_api_http_handler, request) == NULL) {
            FURI_LOG_E(TAG, "Failed to process request");
            break;
        }

        success = true;
    } while(false);

    if(!success) {
        if(mqtt_http_proxy_request_requires_response(request)) {
            mqtt_http_proxy_respond_error(request);
        }

        mqtt_http_proxy_request_free(request);
    }
}

static void
    mqtt_http_proxy_event_callback(struct mg_connection* connection, int event, void* event_data) {
    if(event == MG_EV_WAKEUP) {
        MqttHttpProxySrv* instance = connection->fn_data;
        furi_assert(instance);

        const struct mg_str* data = event_data;
        furi_assert(data);
        furi_assert(data->buf);
        furi_assert(data->len == sizeof(MqttHttpProxyRequest*));

        MqttHttpProxyRequest* request = *(MqttHttpProxyRequest**)data->buf;
        mqtt_http_proxy_process_request(instance, request);
    }
}

static MqttHttpProxySrv* mqtt_http_proxy_alloc(void) {
    MqttHttpProxySrv* instance = malloc(sizeof(MqttHttpProxySrv));
    instance->mqtt = furi_record_open(RECORD_MQTT);

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);

    mg_mgr_init(&instance->mgr);
    mg_wakeup_init(&instance->mgr);

    const struct mg_connection* api_connection =
        mg_wrapfd(&instance->mgr, MG_INVALID_SOCKET, mqtt_http_proxy_event_callback, instance);
    instance->api_connection_id = api_connection->id;

    mqtt_subscribe(instance->mqtt, SUB_QOS, SUB_TOPIC, mqtt_http_proxy_message_callback, instance);
    return instance;
}

int32_t mqtt_http_proxy_srv(void* arg) {
    UNUSED(arg);

    MqttHttpProxySrv* instance = mqtt_http_proxy_alloc();

    for(;;) {
        mg_mgr_poll(&instance->mgr, POLL_PERIOD_MS);
    }

    return 0;
}
