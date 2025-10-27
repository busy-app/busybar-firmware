#include "http_api.h"
#include <mqtt_client/mqtt_client.h>

#define TAG "HttpAccount"

#define LINK_TIMEOUT 3000

static const struct {
    char* name;
    MqttClientStatus status;
} mqtt_statuses[] = {
    {"error", MqttClientStatusError},
    {"disconnected", MqttClientStatusNotConnected},
    {"not_linked", MqttClientStatusConnectedNotLinked},
    {"linked", MqttClientStatusConnectedLinked},
};

static void http_api_account_get_status(struct mg_connection* conn) {
    FuriString* json_str = furi_string_alloc();

    MqttClient* mqtt = furi_record_open(RECORD_MQTT);
    MqttClientStatus status = mqtt_client_get_status(mqtt);

    for(size_t i = 0; i < COUNT_OF(mqtt_statuses); i++) {
        if(status == mqtt_statuses[i].status) {
            furi_string_cat_printf(json_str, "\"%s\":\"%s\"", "state", mqtt_statuses[i].name);
        }
    }

    if(status == MqttClientStatusConnectedLinked) {
        FuriString* id_str = furi_string_alloc();
        mqtt_client_get_session_id(mqtt, id_str);
        furi_string_cat_printf(json_str, ",\"%s\":\"%s\"", "id", furi_string_get_cstr(id_str));

        mqtt_client_get_session_email(mqtt, id_str);
        furi_string_cat_printf(json_str, ",\"%s\":\"%s\"", "email", furi_string_get_cstr(id_str));
        furi_string_free(id_str);
    }

    furi_record_close(RECORD_MQTT);
    MG_REPLY_OK_BODY(conn, "{%s}\n", furi_string_get_cstr(json_str));
    furi_string_free(json_str);
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

static void http_api_account_link(struct mg_connection* conn) {
    MqttClient* mqtt = furi_record_open(RECORD_MQTT);
    MqttClientStatus status = mqtt_client_get_status(mqtt);

    if(status != MqttClientStatusConnectedNotLinked) {
        MG_REPLY_ERROR(
            conn,
            400,
            (status == MqttClientStatusConnectedLinked) ? "Already linked" : "Not connected");
        return;
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
}

static void http_api_account_unlink(struct mg_connection* conn) {
    MqttClient* mqtt = furi_record_open(RECORD_MQTT);
    mqtt_client_unlink(mqtt);
    furi_record_close(RECORD_MQTT);

    MG_REPLY_OK(conn);
}

bool http_api_account_link_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    if(mg_match(msg->method, mg_str("POST"), NULL)) {
        http_api_account_link(conn);
    } else {
        MG_REPLY_METHOD_NOT_ALLOWED(conn);
    }

    return true;
}

bool http_api_account_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    if(mg_match(msg->method, mg_str("GET"), NULL)) {
        http_api_account_get_status(conn);
    } else if(mg_match(msg->method, mg_str("DELETE"), NULL)) {
        http_api_account_unlink(conn);
    } else {
        MG_REPLY_METHOD_NOT_ALLOWED(conn);
    }

    return true;
}
