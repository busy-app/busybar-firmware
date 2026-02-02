#include "http_api.h"

#include <busy_timer/busy_timer.h>

#define TAG "HttpBusy"

typedef struct {
    HttpHandlersList_t handlers;
} ApiBusyCtx;

static bool api_busy_get_snapshot_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) {
        return false;
    }

    BusyTimer* timer = furi_record_open(RECORD_BUSY_TIMER);
    BusyTimerSnapshot snapshot;
    busy_timer_get_snapshot(timer, &snapshot);
    furi_record_close(RECORD_BUSY_TIMER);

    char* json_text = busy_timer_snapshot_serialize(&snapshot);
    furi_check(json_text);

    MG_REPLY_OK_BODY(conn, json_text);
    free(json_text);

    return true;
}

static bool api_busy_set_snapshot_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) {
        return false;
    }

    bool success = false;
    const char* error_msg;

    do {
        BusyTimerSnapshot snapshot;
        if(!busy_timer_snapshot_deserialize(&snapshot, msg->body.buf)) {
            error_msg = "Failed to parse snapshot";
            break;
        }

        BusyTimer* timer = furi_record_open(RECORD_BUSY_TIMER);
        busy_timer_set_snapshot(timer, &snapshot);
        furi_record_close(RECORD_BUSY_TIMER);

        success = true;
    } while(false);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_ERROR(conn, 400, error_msg);
    }

    return true;
}

static const HttpHandler handlers_busy[] = {
    {
        .uri = "snapshot",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = api_busy_get_snapshot_callback,
    },
    {
        .uri = "snapshot",
        .method = "PUT",
        .type = HttpHandlerCustom,
        .on_request = api_busy_set_snapshot_callback,
    },
};

void* http_api_busy_alloc(void) {
    ApiBusyCtx* context = malloc(sizeof(ApiBusyCtx));

    HttpHandlersList_init(context->handlers);
    for(size_t i = 0; i < COUNT_OF(handlers_busy); ++i) {
        http_handler_add(context->handlers, &handlers_busy[i]);
    }

    return context;
}

void http_api_busy_free(void* ctx) {
    furi_assert(ctx);
    ApiBusyCtx* context = ctx;

    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_busy_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiBusyCtx* context = ctx;
    return http_handle_request(path, context->handlers, conn, msg);
}
