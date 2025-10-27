#include "http_api.h"

#define TAG "HttpBusy"

typedef struct {
    HttpHandlersList_t handlers;
} ApiBusyCtx;

static bool api_busy_get_snapshot_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(conn);
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FURI_LOG_I(TAG, "GetSnapshot!");

    MG_REPLY_OK(conn);

    return true;
}

static bool api_busy_set_snapshot_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(conn);
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FURI_LOG_I(TAG, "SetSnapshot!");

    MG_REPLY_OK(conn);

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
        .method = "POST",
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
