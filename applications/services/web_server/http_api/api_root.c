#include "http_api.h"
#include <version.h>

bool http_api_version_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);
    FuriString* ver_str = furi_string_alloc();
    const Version* firmware_version = version_get();
    furi_string_printf(
        ver_str,
        "\"branch\":\"%s\",\"version\":\"%s\",\"build_date\":\"%s\",",
        version_get_gitbranch(firmware_version),
        version_get_version(firmware_version),
        version_get_builddate(firmware_version));
    furi_string_cat_printf(
        ver_str,
        "\"commit_hash\":\"%s%s\"",
        version_get_githash(firmware_version),
        version_get_dirty_flag(firmware_version) ? "-dirty" : "");

    MG_REPLY_OK_BODY(conn, "{%s}\n", furi_string_get_cstr(ver_str));
    furi_string_free(ver_str);

    return true;
}

static const HttpHandler handlers_api_root[] = {
    {
        .uri = "/api/v0/version",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = http_api_version_callback,
    },
    {
        .uri = "/api/v0/assets/*",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_assets_alloc,
        .ctx_free = http_api_assets_free,
        .on_request = http_api_assets_callback,
        .on_headers = http_api_assets_hdr_callback,
    },
    {
        .uri = "/api/v0/storage/*",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_storage_alloc,
        .ctx_free = http_api_storage_free,
        .on_request = http_api_storage_callback,
        .on_headers = http_api_storage_hdr_callback,
    },
    {
        .uri = "/api/v0/display/*",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_display_alloc,
        .ctx_free = http_api_display_free,
        .on_request = http_api_display_callback,
    },
    {
        .uri = "/api/v0/audio/*",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_audio_alloc,
        .ctx_free = http_api_audio_free,
        .on_request = http_api_audio_callback,
    },
    {
        .uri = "/api/v0/wifi/*",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_wifi_alloc,
        .ctx_free = http_api_wifi_free,
        .on_request = http_api_wifi_callback,
    },
    {
        .uri = "/api/v0/update",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_headers = http_api_update_callback,
    },
};

typedef struct {
    HttpHandlersList_t handlers;
} ApiRootCtx;

void* http_api_root_alloc(void) {
    ApiRootCtx* context = malloc(sizeof(ApiRootCtx));
    HttpHandlersList_init(context->handlers);

    for(size_t i = COUNT_OF(handlers_api_root); i > 0; i--) {
        http_handler_add(context->handlers, &handlers_api_root[i - 1]);
    }
    return context;
}

void http_api_root_free(void* ctx) {
    furi_assert(ctx);
    ApiRootCtx* context = ctx;
    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_root_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    ApiRootCtx* context = ctx;
    FURI_LOG_I(
        "HTTP API", "%.*s %.*s", msg->method.len, msg->method.buf, msg->uri.len, msg->uri.buf);
    if(msg->query.len > 0) {
        FURI_LOG_I("HTTP API", "Query %.*s", msg->query.len, msg->query.buf);
    }
    return http_handle_request(context->handlers, conn, msg);
}

bool http_api_root_hdr_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    ApiRootCtx* context = ctx;
    return http_handle_headers(context->handlers, conn, msg);
}
