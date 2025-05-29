#include "http_api.h"
#include <loader/loader.h>
#include <desktop/desktop.h>
#include <gui/gui.h>
#include <toolbox/path.h>

#define TAG "HttpAssets"

#define ASSETS_UPLOAD_DIR EXT_PATH("assets")
#define FILE_NAME_LEN_MAX 32

typedef struct {
    size_t len_remain;
    void* file;
} UploadClientCtx;

static void api_assets_upload_data_callback(struct mg_connection* conn, struct mg_iobuf* data) {
    ConnectionContext* conn_ctx = (void*)conn->data;
    UploadClientCtx* upload_ctx = conn_ctx->context;

    if((data->len > 0) && (upload_ctx->file)) {
        // Write file chunk
        http_fs_get()->wr(upload_ctx->file, data->buf, data->len);
    }

    if(data->len >= upload_ctx->len_remain) {
        // End of transfer - close connection
        MG_REPLY_OK(conn);
        conn->is_draining = 1;
        if(upload_ctx->file) {
            http_fs_get()->cl(upload_ctx->file);
            upload_ctx->file = NULL;
        }
        upload_ctx->len_remain = 0;
    } else {
        upload_ctx->len_remain -= data->len;
    }
    data->len = 0;
}

static void api_assets_upload_close_callback(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (void*)conn->data;
    UploadClientCtx* upload_ctx = conn_ctx->context;
    if(upload_ctx->file) {
        http_fs_get()->cl(upload_ctx->file);
        upload_ctx->file = NULL;
    }
    free(upload_ctx);
    conn_ctx->on_close = NULL;
    conn_ctx->raw.on_data = NULL;
    conn_ctx->context = NULL;
}

static bool api_assets_upload_parse_parameters(struct mg_str* params_str, FuriString* file_path) {
    if(params_str->len == 0) {
        return false;
    }

    char temp_str[FILE_NAME_LEN_MAX];

    int var_len = mg_http_get_var(params_str, "app_id", temp_str, sizeof(temp_str));
    if(var_len <= 0) {
        return false;
    }
    furi_string_printf(file_path, "%s/%.*s/", ASSETS_UPLOAD_DIR, var_len, temp_str);

    var_len = mg_http_get_var(params_str, "file", temp_str, sizeof(temp_str));
    if(var_len <= 0) {
        return false;
    }
    furi_string_cat_printf(file_path, "%.*s", var_len, temp_str);

    return true;
}

static bool api_assets_upload_headers_callback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    FuriString* file_path = furi_string_alloc();
    if(api_assets_upload_parse_parameters(&msg->query, file_path)) {
        FURI_LOG_I(
            TAG, "Upload len = %u path = %s", msg->body.len, furi_string_get_cstr(file_path));
        // Create upload context
        UploadClientCtx* upload_ctx = malloc(sizeof(UploadClientCtx));
        upload_ctx->len_remain = msg->body.len;
        upload_ctx->file = NULL;

        // Assign callbacks
        ConnectionContext* conn_ctx = (void*)conn->data;
        conn_ctx->on_close = api_assets_upload_close_callback;
        conn_ctx->raw.on_data = api_assets_upload_data_callback;
        conn_ctx->context = upload_ctx;

        const char* path = furi_string_get_cstr(file_path);
        if(mg_path_is_sane(mg_str(path))) {
            http_fs_get()->rm(path); // Delete file if it exists
            FuriString* dir_path = furi_string_alloc();
            path_extract_dirname(path, dir_path);
            http_fs_get()->mkd(furi_string_get_cstr(dir_path));
            furi_string_free(dir_path);
            upload_ctx->file = http_fs_get()->op(path, MG_FS_WRITE); // Open file for writing
        }

        if(upload_ctx->file == NULL) {
            MG_REPLY_INTERNAL_ERROR(conn, "Failed to open file for writing");
            conn->is_draining = 1;
        }
    } else {
        MG_REPLY_BAD_REQUEST(conn);
        conn->is_draining = 1;
    }

    furi_string_free(file_path);

    mg_iobuf_del(&conn->recv, 0, msg->head.len); // Delete HTTP headers
    conn->pfn = NULL; // Silence HTTP protocol handler, we'll use MG_EV_READ

    return true;
}

static bool
    api_assets_delete_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(ctx);

    FuriString* dir_path = furi_string_alloc();
    bool success = false;
    do {
        if(msg->query.len == 0) {
            break;
        }

        char app_id_str[FILE_NAME_LEN_MAX];

        int var_len = mg_http_get_var(&msg->query, "app_id", app_id_str, sizeof(app_id_str));
        if(var_len <= 0) {
            break;
        }
        furi_string_printf(dir_path, "%s/%.*s", ASSETS_UPLOAD_DIR, var_len, app_id_str);

        Storage* fs_api = furi_record_open(RECORD_STORAGE);
        success = storage_simply_remove_recursive(fs_api, furi_string_get_cstr(dir_path));
        furi_record_close(RECORD_STORAGE);
    } while(0);

    furi_string_free(dir_path);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    return true;
}

static const HttpHandler handlers_assets[] = {
    {
        .uri = "/api/v0/assets/upload",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_headers = api_assets_upload_headers_callback,
    },
    {
        .uri = "/api/v0/assets/upload",
        .method = "DELETE",
        .type = HttpHandlerCustom,
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

bool http_api_assets_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    ApiAssetsCtx* context = ctx;
    return http_handle_request(context->handlers, conn, msg);
}

bool http_api_assets_hdr_callback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiAssetsCtx* context = ctx;
    return http_handle_headers(context->handlers, conn, msg);
}
