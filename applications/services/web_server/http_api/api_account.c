#include "http_api.h"

#include <mqtt/mqtt.h>

#define TAG "HttpAccount"

#define LINK_TIMEOUT_MS 3000

static bool http_api_account_get_info(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FuriString* json_str = furi_string_alloc();

    Mqtt* mqtt = furi_record_open(RECORD_MQTT);

    FuriString* id_str = furi_string_alloc();
    FuriString* email_str = furi_string_alloc();
    FuriString* user_id_str = furi_string_alloc();

    MqttSessionInfo info = {.session_id = id_str, .email = email_str, .user_id = user_id_str};
    mqtt_get_session_info(mqtt, &info);

    bool linked = info.is_valid;
    furi_string_printf(json_str, "\"%s\":%s", "linked", linked ? "true" : "false");

    if(linked) {
        furi_string_cat_printf(json_str, ",\"%s\":\"%s\"", "id", furi_string_get_cstr(id_str));
        furi_string_cat_printf(
            json_str, ",\"%s\":\"%s\"", "email", furi_string_get_cstr(email_str));
        furi_string_cat_printf(
            json_str, ",\"%s\":\"%s\"", "user_id", furi_string_get_cstr(user_id_str));
    }

    furi_string_free(id_str);
    furi_string_free(email_str);
    furi_string_free(user_id_str);

    furi_record_close(RECORD_MQTT);
    MG_REPLY_OK_BODY(conn, "{%s}\n", furi_string_get_cstr(json_str));
    furi_string_free(json_str);

    return true;
}

static bool http_api_account_get_status(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FuriString* json_str = furi_string_alloc();

    Mqtt* mqtt = furi_record_open(RECORD_MQTT);
    MqttStatus status = mqtt_get_status(mqtt);

    if(status == MqttStatusError) {
        furi_string_printf(json_str, "\"%s\":\"%s\"", "status", "error");
    } else if(status == MqttStatusNotConnected) {
        furi_string_printf(json_str, "\"%s\":\"%s\"", "status", "disconnected");
    } else {
        furi_string_printf(json_str, "\"%s\":\"%s\"", "status", "connected");
    }

    furi_record_close(RECORD_MQTT);
    MG_REPLY_OK_BODY(conn, "{%s}\n", furi_string_get_cstr(json_str));
    furi_string_free(json_str);

    return true;
}

typedef struct {
    struct mg_connection* conn;
    Mqtt* mqtt;
    FuriPubSubSubscription* mqtt_event_sub;
    struct mg_timer timeout_timer;
    char pin[MQTT_LINK_PIN_LEN + 1];
    uint32_t pin_expires_at;
} MqttLinkContext;

static void mqtt_link_events_callback(const void* message, void* context) {
    MqttLinkContext* link_ctx = context;
    furi_assert(link_ctx);

    const MqttEvent* mqtt_event = (const MqttEvent*)message;
    furi_assert(mqtt_event);

    if(mqtt_event->type == MqttEventTypeLinkPinReceived) {
        const MqttEventLinkPinReceived* event = &mqtt_event->link_pin_received;
        strlcpy(link_ctx->pin, event->pin, sizeof(link_ctx->pin));
        link_ctx->pin_expires_at = event->expires_at;
        mg_wakeup(web_srv_get_mgr(), link_ctx->conn->id, NULL, 0);
    }
}

static void mqtt_link_timeout(void* data) {
    furi_assert(data);
    MqttLinkContext* link_ctx = data;

    MG_REPLY_INTERNAL_ERROR(link_ctx->conn, "PIN request timeout");
    link_ctx->conn->is_draining = true;
}

static void mqtt_link_wakeup_callback(struct mg_connection* conn, void* data, size_t len) {
    UNUSED(data);
    UNUSED(len);
    furi_assert(conn);
    ConnectionContext* conn_ctx = (void*)conn->data;
    MqttLinkContext* link_ctx = conn_ctx->context;
    furi_assert(link_ctx);

    FuriString* json_str = furi_string_alloc();

    furi_string_cat_printf(json_str, "\"%s\":\"%s\",", "code", link_ctx->pin);
    furi_string_cat_printf(json_str, "\"%s\":%lu", "expires_at", link_ctx->pin_expires_at);

    MG_REPLY_OK_BODY(conn, "{%s}\n", furi_string_get_cstr(json_str));
    furi_string_free(json_str);
    conn->is_draining = true;
}

static void mqtt_link_close_callback(struct mg_connection* conn) {
    furi_assert(conn);
    ConnectionContext* conn_ctx = (void*)conn->data;
    MqttLinkContext* link_ctx = conn_ctx->context;
    furi_assert(link_ctx);

    mg_timer_free(&web_srv_get_mgr()->timers, &link_ctx->timeout_timer);

    furi_pubsub_unsubscribe(mqtt_get_pubsub(link_ctx->mqtt), link_ctx->mqtt_event_sub);
    furi_record_close(RECORD_MQTT);
    conn_ctx->on_wakeup = NULL;
    conn_ctx->on_close = NULL;
    conn_ctx->context = NULL;
    free(link_ctx);
}

static bool http_api_account_link(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    Mqtt* mqtt = furi_record_open(RECORD_MQTT);
    MqttStatus status = mqtt_get_status(mqtt);

    if(status != MqttStatusConnectedNotLinked) {
        MG_REPLY_ERROR(
            conn, 400, (status == MqttStatusConnectedLinked) ? "Already linked" : "Not connected");
        furi_record_close(RECORD_MQTT);
        return true;
    }

    MqttLinkContext* link_ctx = malloc(sizeof(MqttLinkContext));
    link_ctx->mqtt = mqtt;
    link_ctx->conn = conn;

    // Setup response callbacks
    ConnectionContext* conn_ctx = (void*)conn->data;
    conn_ctx->on_close = mqtt_link_close_callback;
    conn_ctx->on_wakeup = mqtt_link_wakeup_callback;
    conn_ctx->context = link_ctx;

    link_ctx->mqtt_event_sub =
        furi_pubsub_subscribe(mqtt_get_pubsub(mqtt), mqtt_link_events_callback, link_ctx);

    // Setup timeout timer
    mg_timer_init(
        &web_srv_get_mgr()->timers,
        &link_ctx->timeout_timer,
        LINK_TIMEOUT_MS,
        MG_TIMER_ONCE,
        mqtt_link_timeout,
        link_ctx);

    // Send request
    mqtt_request_link_pin(mqtt);

    // Hold connection untill link pin response or timeout
    return true;
}

static bool http_api_account_unlink(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;
    if(method != HttpMethodDelete) {
        http_reply_405_method_not_allowed(conn, HttpMethodDelete);
        return true;
    }

    Mqtt* mqtt = furi_record_open(RECORD_MQTT);
    mqtt_unlink(mqtt);
    furi_record_close(RECORD_MQTT);

    MG_REPLY_OK(conn);

    return true;
}

static void http_api_account_mqtt_backend_get(struct mg_connection* conn) {
    Mqtt* mqtt = furi_record_open(RECORD_MQTT);

    MqttConfig config;
    mqtt_get_config(mqtt, &config);

    furi_record_close(RECORD_MQTT);

    char* json_text = mqtt_config_serialize(&config);

    if(json_text) {
        MG_REPLY_OK_BODY(conn, "%s\n", json_text);
        free(json_text);
    } else {
        MG_REPLY_INTERNAL_ERROR(conn);
    }
}

static void
    http_api_account_mqtt_backend_put(struct mg_connection* conn, struct mg_http_message* msg) {
    bool success = false;
    const char* error_msg = NULL;

    Mqtt* mqtt = furi_record_open(RECORD_MQTT);

    do {
        MqttConfig config;

        if(!mqtt_config_deserialize(&config, msg->body.buf, msg->body.len)) {
            error_msg = "Malformed request";
            break;
        }

        if(!mqtt_set_config(mqtt, &config)) {
            error_msg = "Invalid value";
            break;
        }

        success = true;
    } while(false);

    furi_record_close(RECORD_MQTT);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_ERROR(conn, 400, error_msg);
    }
}

static bool http_api_account_mqtt_backend(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    if(method == HttpMethodGet) {
        http_api_account_mqtt_backend_get(conn);
    } else if(method == HttpMethodPut) {
        http_api_account_mqtt_backend_put(conn, msg);
    }

    return true;
}

static const HttpHandler api_account_handlers[] = {
    {
        .uri = "",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .on_request = http_api_account_unlink,
    },
    {
        .uri = "link",
        .method = HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = http_api_account_link,
    },
    {
        .uri = "info",
        .method = HttpMethodGet,
        .type = HttpHandlerCustom,
        .on_request = http_api_account_get_info,
    },
    {
        .uri = "status",
        .method = HttpMethodGet,
        .type = HttpHandlerCustom,
        .on_request = http_api_account_get_status,
    },
    {
        .uri = "backend",
        .method = HttpMethodGet | HttpMethodPut,
        .type = HttpHandlerCustom,
        .on_request = http_api_account_mqtt_backend,
    },
};

typedef struct {
    HttpHandlersList_t handlers;
} ApiAccountCtx;

void* http_api_account_alloc(void) {
    ApiAccountCtx* context = malloc(sizeof(ApiAccountCtx));
    HttpHandlersList_init(context->handlers);

    for(size_t i = COUNT_OF(api_account_handlers); i > 0; i--) {
        http_handler_add(context->handlers, &api_account_handlers[i - 1]);
    }

    return context;
}

void http_api_account_free(void* ctx) {
    furi_assert(ctx);

    ApiAccountCtx* context = ctx;
    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_account_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiAccountCtx* context = ctx;

    return http_handle_request(path, method, context->handlers, conn, msg);
}
