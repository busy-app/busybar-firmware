#include "http_api.h"
#include <cjson/cJSON.h>
#include <ble/ble.h>

#define TAG "HttpBle"

typedef struct {
    HttpHandlersList_t handlers;
} ApiBleCtx;

const char* ble_state_names[] = {
    [BleServiceStateReset] = "reset",
    [BleServiceStateInitialization] = "initialization",
    [BleServiceStateReady] = "enabled",
    [BleServiceStateAdvertising] = "advertising",
    [BleServiceStateConnected] = "connected",
    [BleServiceStateError] = "internal error",
};

static bool api_ble_enable_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    Ble* ble = furi_record_open(RECORD_BLE);
    bool result = ble_start(ble);
    furi_record_close(RECORD_BLE);

    int code = 404;
    const char* message = "Unable to start BLE";
    if(result)
        MG_REPLY_OK(conn);
    else
        MG_REPLY_ERROR(conn, code, message);

    return true;
}

static bool api_ble_disable_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    Ble* ble = furi_record_open(RECORD_BLE);
    bool result = ble_stop(ble);
    furi_record_close(RECORD_BLE);

    int code = 404;
    const char* message = "Unable to stop BLE";
    if(result)
        MG_REPLY_OK(conn);
    else
        MG_REPLY_ERROR(conn, code, message);

    return true;
}

static bool api_ble_get_state_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    Ble* ble = furi_record_open(RECORD_BLE);
    BleServiceState state = ble_get_state(ble);
    furi_record_close(RECORD_BLE);

    ///TODO:rework logic to return enabled/disabled/connected
    if(state != BleServiceStateError)
        MG_REPLY_OK_BODY(conn, "{\"state\":\"%s\"}\n", ble_state_names[state]);
    else
        MG_REPLY_ERROR(conn, 400, ble_state_names[state]);

    return true;
}

static bool api_ble_delete_pairing_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    Ble* ble = furi_record_open(RECORD_BLE);
    bool result = ble_forget(ble);
    furi_record_close(RECORD_BLE);

    int code = 503;
    const char* message = "Unable to remove pairing";
    if(result)
        MG_REPLY_OK(conn);
    else
        MG_REPLY_ERROR(conn, code, message);

    return true;
}

static const HttpHandler handlers_ble[] = {
    {
        .uri = "enable",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_ble_enable_callback,
    },
    {
        .uri = "disable",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_ble_disable_callback,
    },
    {
        .uri = "status",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = api_ble_get_state_callback,
    },
    {
        .uri = "pairing",
        .method = "DELETE",
        .type = HttpHandlerCustom,
        .on_request = api_ble_delete_pairing_callback,
    },
};

void* http_api_ble_alloc(void) {
    ApiBleCtx* context = malloc(sizeof(ApiBleCtx));
    HttpHandlersList_init(context->handlers);
    for(size_t i = COUNT_OF(handlers_ble); i > 0; i--) {
        http_handler_add(context->handlers, &handlers_ble[i - 1]);
    }

    return context;
}

void http_api_ble_free(void* ctx) {
    furi_assert(ctx);
    ApiBleCtx* context = ctx;
    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_ble_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiBleCtx* context = ctx;
    return http_handle_request(path, context->handlers, conn, msg);
}
