#include "http_api.h"
#include <loader/loader.h>
#include <desktop/desktop.h>
#include <gui/gui.h>
#include <toolbox/path.h>

#define TAG "HttpAssets"

#define ASSETS_UPLOAD_DIR EXT_PATH("user_assets")
#define FILE_NAME_LEN_MAX 32

static bool api_assets_upload_parse_parameters(struct mg_str* params_str, FuriString* file_path) {
    if(params_str->len == 0) {
        return false;
    }

    char temp_str[FILE_NAME_LEN_MAX];

    int var_len = mg_http_get_var(params_str, "application_name", temp_str, sizeof(temp_str));
    if(var_len <= 0) {
        return false;
    }
    furi_string_printf(file_path, "%s/%.*s/", ASSETS_UPLOAD_DIR, var_len, temp_str);

    var_len = mg_http_get_var(params_str, "file", temp_str, sizeof(temp_str));
    if(var_len <= 0) {
        return false;
    }
    furi_string_cat_printf(file_path, "%.*s", var_len, temp_str);

    return mg_path_is_sane(mg_str(furi_string_get_cstr(file_path)));
}

static bool api_assets_upload_headers_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;
    if(method != HttpMethodPost) return false;

    FuriString* file_path = furi_string_alloc();
    if(api_assets_upload_parse_parameters(&msg->query, file_path)) {
        http_upload_start(conn, msg, furi_string_get_cstr(file_path));
    } else {
        MG_REPLY_BAD_REQUEST(conn);
        MG_CLOSE_AFTER_HEADERS(conn, msg);
    }

    furi_string_free(file_path);

    return true;
}

static bool api_assets_delete_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;
    if(method != HttpMethodDelete) return false;

    FuriString* dir_path = furi_string_alloc();
    bool success = false;
    do {
        if(msg->query.len == 0) {
            MG_REPLY_BAD_REQUEST(conn);
            break;
        }

        char app_name_str[FILE_NAME_LEN_MAX];

        int var_len =
            mg_http_get_var(&msg->query, "application_name", app_name_str, sizeof(app_name_str));
        if(var_len <= 0) {
            MG_REPLY_BAD_REQUEST(conn);
            break;
        }
        furi_string_printf(dir_path, "%s/%.*s", ASSETS_UPLOAD_DIR, var_len, app_name_str);

        Storage* fs_api = furi_record_open(RECORD_STORAGE);
        if(!storage_dir_exists(fs_api, furi_string_get_cstr(dir_path))) {
            MG_REPLY_ERROR(conn, 404, "Assets not found");
            furi_record_close(RECORD_STORAGE);
            break;
        }

        success = storage_simply_remove_recursive(fs_api, furi_string_get_cstr(dir_path));
        furi_record_close(RECORD_STORAGE);

        if(success) {
            MG_REPLY_OK(conn);
        } else {
            MG_REPLY_INTERNAL_ERROR(conn, "File delete failed");
        }
    } while(0);

    furi_string_free(dir_path);

    return true;
}

static const HttpHandler handlers_assets[] = {
    {
        .uri = "upload",
        .method = HttpMethodPost | HttpMethodDelete,
        .type = HttpHandlerCustom,
        .on_headers = api_assets_upload_headers_callback,
        .on_request = api_assets_delete_callback,
    },
};

typedef struct {
    HttpHandlersList_t handlers;
} ApiAssetsCtx;

void* http_api_assets_alloc(void) {
    ApiAssetsCtx* context = malloc(sizeof(ApiAssetsCtx));
    HttpHandlersList_init(context->handlers);

    for(size_t i = COUNT_OF(handlers_assets); i > 0; i--) {
        http_handler_add(context->handlers, &handlers_assets[i - 1]);
    }

    // Create assets directory
    http_fs_get()->mkd(ASSETS_UPLOAD_DIR);
    return context;
}

void http_api_assets_free(void* ctx) {
    furi_assert(ctx);
    ApiAssetsCtx* context = ctx;
    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_assets_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiAssetsCtx* context = ctx;
    return http_handle_request(path, method, context->handlers, conn, msg);
}

bool http_api_assets_hdr_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiAssetsCtx* context = ctx;
    return http_handle_headers(path, method, context->handlers, conn, msg);
}
