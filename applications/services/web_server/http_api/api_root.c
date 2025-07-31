#include "http_api.h"
#include <version.h>

bool http_api_version_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FuriString* ver_str = furi_string_alloc();
    const uint8_t api_ver[] = API_VERSION;

    furi_string_printf(ver_str, "\"api_semver\":\"%u.%u.%u\"", api_ver[0], api_ver[1], api_ver[2]);

    MG_REPLY_OK_BODY(conn, "{%s}\n", furi_string_get_cstr(ver_str));
    furi_string_free(ver_str);

    return true;
}

static const HttpHandler handlers_api_root[] = {
    {
        .uri = "version",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = http_api_version_callback,
    },
    {
        .uri = "assets",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_assets_alloc,
        .ctx_free = http_api_assets_free,
        .on_request = http_api_assets_callback,
        .on_headers = http_api_assets_hdr_callback,
    },
    {
        .uri = "storage",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_storage_alloc,
        .ctx_free = http_api_storage_free,
        .on_request = http_api_storage_callback,
        .on_headers = http_api_storage_hdr_callback,
    },
    {
        .uri = "display",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_display_alloc,
        .ctx_free = http_api_display_free,
        .on_request = http_api_display_callback,
    },
    {
        .uri = "audio",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_audio_alloc,
        .ctx_free = http_api_audio_free,
        .on_request = http_api_audio_callback,
    },
    {
        .uri = "input",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_input_alloc,
        .ctx_free = http_api_input_free,
        .on_request = http_api_input_callback,
    },
    {
        .uri = "status",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = http_api_status_callback,
    },
    {
        .uri = "wifi",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_wifi_alloc,
        .ctx_free = http_api_wifi_free,
        .on_request = http_api_wifi_callback,
    },
    {
        .uri = "update",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_headers = http_api_update_hdr_callback,
    },
    {
        .uri = "screen",
        .method = "*",
        .type = HttpHandlerCustom,
        .on_request = http_api_streaming_single_frame_callback,
    },
    {
        .uri = "screen/ws",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_streaming_ws_alloc,
        .ctx_free = http_api_streaming_ws_free,
        .on_request = http_api_streaming_ws_callback,
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

bool http_api_root_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiRootCtx* context = ctx;
    FURI_LOG_I(
        "HTTP API", "%.*s %.*s", msg->method.len, msg->method.buf, msg->uri.len, msg->uri.buf);
    if(msg->query.len > 0) {
        FURI_LOG_I("HTTP API", "Query %.*s", msg->query.len, msg->query.buf);
    }
    struct mg_str* header_semver = mg_http_get_header(msg, "X-API-Sem-Ver");
    if(header_semver) {
        uint8_t major_ver;
        const uint8_t api_ver[] = API_VERSION;

        struct mg_str major_ver_str;
        if(!mg_span(*header_semver, &major_ver_str, NULL, '.')) {
            MG_REPLY_BAD_REQUEST(conn);
            return true;
        }

        if(!mg_str_to_num(major_ver_str, 10, &major_ver, sizeof(major_ver))) {
            MG_REPLY_BAD_REQUEST(conn);
            return true;
        }

        if(major_ver != api_ver[0]) {
            MG_REPLY_INVALID_VERSION(conn);
            return true;
        }
    }
    return http_handle_request(path, context->handlers, conn, msg);
}

bool http_api_options_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(path);
    UNUSED(msg);
    UNUSED(ctx);
    MG_REPLY_OPTIONS(conn);
    return true;
}

bool http_api_root_hdr_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiRootCtx* context = ctx;
    return http_handle_headers(path, context->handlers, conn, msg);
}
