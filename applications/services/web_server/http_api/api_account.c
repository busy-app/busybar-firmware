#include "http_api.h"
#include <mqtt_client/mqtt_client.h>

#define TAG "HttpAccount"

#define CUSTOM_URL_LEN_MAX 64
#define LINK_TIMEOUT       3000

static bool http_api_account_get_info(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FuriString* json_str = furi_string_alloc();

    MqttClient* mqtt = furi_record_open(RECORD_MQTT);

    FuriString* id_str = furi_string_alloc();
    FuriString* email_str = furi_string_alloc();
    FuriString* user_id_str = furi_string_alloc();

    mqtt_client_get_session_info(mqtt, id_str, email_str, user_id_str);

    bool linked = !furi_string_empty(id_str);
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
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FuriString* json_str = furi_string_alloc();

    MqttClient* mqtt = furi_record_open(RECORD_MQTT);
    MqttClientStatus status = mqtt_client_get_status(mqtt);

    if(status == MqttClientStatusError) {
        furi_string_printf(json_str, "\"%s\":\"%s\"", "status", "error");
    } else if(status == MqttClientStatusNotConnected) {
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
    MqttClient* mqtt;
    FuriPubSubSubscription* mqtt_event_sub;
    struct mg_timer timeout_timer;
    char pin[MQTT_LINK_PIN_LEN + 1];
    uint32_t pin_expires_at;
} MqttLinkContext;

static void mqtt_link_events_callback(const void* message, void* context) {
    MqttLinkContext* link_ctx = context;
    furi_assert(link_ctx);

    MqttClientEvent* mqtt_event = (MqttClientEvent*)message;
    furi_assert(mqtt_event);

    if(mqtt_event->type == MqttClientEventLinkPin) {
        strncpy(link_ctx->pin, mqtt_event->link.pin, MQTT_LINK_PIN_LEN);
        link_ctx->pin[MQTT_LINK_PIN_LEN] = '\0';
        link_ctx->pin_expires_at = mqtt_event->link.expires_at;
        mg_wakeup(web_srv_get_mgr(), link_ctx->conn->id, NULL, 0);
    }
}

static void mqtt_link_timeout(void* data) {
    furi_assert(data);
    MqttLinkContext* link_ctx = data;

    MG_REPLY_INTERNAL_ERROR(link_ctx->conn, "PIN request timeout");
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
}

static void mqtt_link_close_callback(struct mg_connection* conn) {
    furi_assert(conn);
    ConnectionContext* conn_ctx = (void*)conn->data;
    MqttLinkContext* link_ctx = conn_ctx->context;
    furi_assert(link_ctx);

    mg_timer_free(&web_srv_get_mgr()->timers, &link_ctx->timeout_timer);

    furi_pubsub_unsubscribe(mqtt_client_get_pubsub(link_ctx->mqtt), link_ctx->mqtt_event_sub);
    furi_record_close(RECORD_MQTT);
    conn_ctx->on_wakeup = NULL;
    conn_ctx->on_close = NULL;
    conn_ctx->context = NULL;
    free(link_ctx);
}

static bool http_api_account_link(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    MqttClient* mqtt = furi_record_open(RECORD_MQTT);
    MqttClientStatus status = mqtt_client_get_status(mqtt);

    if(status != MqttClientStatusConnectedNotLinked) {
        MG_REPLY_ERROR(
            conn,
            400,
            (status == MqttClientStatusConnectedLinked) ? "Already linked" : "Not connected");
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
        furi_pubsub_subscribe(mqtt_client_get_pubsub(mqtt), mqtt_link_events_callback, link_ctx);

    // Setup timeout timer
    mg_timer_init(
        &web_srv_get_mgr()->timers,
        &link_ctx->timeout_timer,
        LINK_TIMEOUT,
        MG_TIMER_ONCE,
        mqtt_link_timeout,
        link_ctx);

    // Send request
    mqtt_client_request_link_pin(mqtt);

    // Hold connection untill link pin response or timeout
    return true;
}

static bool http_api_account_unlink(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    MqttClient* mqtt = furi_record_open(RECORD_MQTT);
    mqtt_client_unlink(mqtt);
    furi_record_close(RECORD_MQTT);

    MG_REPLY_OK(conn);

    return true;
}

static bool http_api_account_mqtt_profile(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    if(mg_match(msg->method, mg_str("GET"), NULL)) {
        FuriString* url = furi_string_alloc();
        MqttClient* mqtt = furi_record_open(RECORD_MQTT);
        const MqttProfileId profile_id = mqtt_client_get_profile(mqtt, url);
        furi_record_close(RECORD_MQTT);

        if(profile_id == MqttProfileIdProduction) {
            MG_REPLY_OK_BODY(conn, "{\"profile\":\"%s\"}\n", "prod");
        } else if(profile_id == MqttProfileIdDevelopment) {
            MG_REPLY_OK_BODY(conn, "{\"profile\":\"%s\"}\n", "dev");
        } else if(profile_id == MqttProfileIdLocal) {
            MG_REPLY_OK_BODY(conn, "{\"profile\":\"%s\"}\n", "local");
        } else {
            MG_REPLY_OK_BODY(
                conn,
                "{\"profile\":\"%s\",\"custom_url\":\"%s\"}\n",
                "custom",
                furi_string_get_cstr(url));
        }

        furi_string_free(url);

    } else if(mg_match(msg->method, mg_str("POST"), NULL)) {
        bool success = false;
        do {
            if(msg->query.len == 0) break;

            char temp_str[CUSTOM_URL_LEN_MAX];
            int var_len = mg_http_get_var(&msg->query, "profile", temp_str, sizeof(temp_str));
            if(var_len <= 0) break;

            MqttProfileId profile_id;
            if(strncmp("prod", temp_str, var_len) == 0) {
                profile_id = MqttProfileIdProduction;
            } else if(strncmp("dev", temp_str, var_len) == 0) {
                profile_id = MqttProfileIdDevelopment;
            } else if(strncmp("local", temp_str, var_len) == 0) {
                profile_id = MqttProfileIdLocal;
            } else if(strncmp("custom", temp_str, var_len) == 0) {
                profile_id = MqttProfileIdCustom;
                var_len = mg_http_get_var(&msg->query, "custom_url", temp_str, sizeof(temp_str));
                if(var_len <= 0) break;
            } else
                break;

            MqttClient* mqtt = furi_record_open(RECORD_MQTT);
            mqtt_client_set_profile(mqtt, profile_id, temp_str);
            furi_record_close(RECORD_MQTT);

            success = true;
        } while(0);

        if(success)
            MG_REPLY_OK(conn);
        else
            MG_REPLY_BAD_REQUEST(conn);
    } else
        MG_REPLY_METHOD_NOT_ALLOWED(conn);

    return true;
}

static const HttpHandler api_account_handlers[] = {
    {
        .uri = "",
        .method = "DELETE",
        .type = HttpHandlerCustom,
        .on_request = http_api_account_unlink,
    },
    {
        .uri = "link",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = http_api_account_link,
    },
    {
        .uri = "info",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = http_api_account_get_info,
    },
    {
        .uri = "status",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = http_api_account_get_status,
    },
    {
        .uri = "profile",
        .method = "*",
        .type = HttpHandlerCustom,
        .on_request = http_api_account_mqtt_profile,
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
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiAccountCtx* context = ctx;

    return http_handle_request(path, context->handlers, conn, msg);
}
