#include "http_api.h"
#include <loader/loader.h>
#include <desktop/desktop.h>
#include <gui/gui.h>
#include <toolbox/path.h>

#define TAG "HttpAssets"

#define ASSETS_UPLOAD_DIR EXT_PATH("user_assets")

#define APP_NAME_LEN_MAX  32
#define FILE_NAME_LEN_MAX 64

#define APP_NAME_PARAM_KEY  "application_name"
#define FILE_NAME_PARAM_KEY "file"

static bool api_assets_get_app_directory_path(const struct mg_str* params, FuriString* out_path) {
    bool success = false;

    char app_name[APP_NAME_LEN_MAX];
    const int app_name_len =
        mg_http_get_var(params, APP_NAME_PARAM_KEY, app_name, sizeof(app_name));

    if(app_name_len > 0) {
        furi_string_printf(out_path, "%s/%.*s", ASSETS_UPLOAD_DIR, app_name_len, app_name);
        success = true;
    }

    return success;
}

static bool
    api_assets_append_target_file_subpath(const struct mg_str* params, FuriString* out_path) {
    bool success = false;

    char file_name[FILE_NAME_LEN_MAX];
    const int file_name_len =
        mg_http_get_var(params, FILE_NAME_PARAM_KEY, file_name, sizeof(file_name));

    if(file_name_len > 0) {
        furi_string_cat_printf(out_path, "/%.*s", file_name_len, file_name);
        success = true;
    }

    return success;
}

static bool api_assets_get_target_file_path(const struct mg_str* params, FuriString* out_path) {
    bool success = false;

    do {
        if(params->len == 0) {
            break;
        }
        if(!api_assets_get_app_directory_path(params, out_path)) {
            break;
        }
        if(!api_assets_append_target_file_subpath(params, out_path)) {
            break;
        }
        if(!mg_path_is_sane(mg_str(furi_string_get_cstr(out_path)))) {
            break;
        }

        success = true;

    } while(false);

    return success;
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

    if(api_assets_get_target_file_path(&msg->query, file_path)) {
        http_upload_start(conn, msg, furi_string_get_cstr(file_path));
    } else {
        MG_REPLY_ERROR_CLOSE(conn, 400, "Bad Request");
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
    Storage* storage = furi_record_open(RECORD_STORAGE);

    do {
        const struct mg_str* params = &msg->query;

        if(params->len == 0) {
            MG_REPLY_BAD_REQUEST(conn);
            break;
        }

        if(!api_assets_get_app_directory_path(params, dir_path)) {
            MG_REPLY_BAD_REQUEST(conn);
            break;
        }

        const char* dir_path_str = furi_string_get_cstr(dir_path);

        FURI_LOG_W(TAG, "!!!!!!!!!!!! %s", dir_path_str);

        if(!storage_dir_exists(storage, dir_path_str)) {
            MG_REPLY_ERROR(conn, 404, "Assets not found");
            break;
        }

        if(!storage_simply_remove_recursive(storage, dir_path_str)) {
            MG_REPLY_SERVICE_UNAVAILABLE(conn, "File delete failed");
            break;
        }

        MG_REPLY_OK(conn);

    } while(0);

    furi_record_close(RECORD_STORAGE);
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

static void api_assets_create_base_directory(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    if(!storage_simply_mkdir(storage, ASSETS_UPLOAD_DIR)) {
        FURI_LOG_E(TAG, "Failed to create base directory");
    }

    furi_record_close(RECORD_STORAGE);
}

void* http_api_assets_alloc(void) {
    ApiAssetsCtx* context = malloc(sizeof(ApiAssetsCtx));
    HttpHandlersList_init(context->handlers);

    for(size_t i = COUNT_OF(handlers_assets); i > 0; i--) {
        http_handler_add(context->handlers, &handlers_assets[i - 1]);
    }

    api_assets_create_base_directory();
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
