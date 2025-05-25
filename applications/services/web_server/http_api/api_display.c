#include "http_api.h"
#include <loader/loader.h>
#include <desktop/desktop.h>
#include <gui/gui.h>
#include <toolbox/path.h>

#define TAG "HTTP Display"

// #define ASSETS_DIR        "/ext/canvas"
#define ASSETS_DIR        "/ext/www/canvas"
#define FILE_NAME_LEN_MAX 32

typedef struct {
    size_t len_remain;
    void* file;
} UploadClientCtx;

static void api_display_upload_data_callback(struct mg_connection* conn, struct mg_iobuf* data) {
    ConnectionContext* conn_ctx = (void*)conn->data;
    UploadClientCtx* upload_ctx = conn_ctx->context;

    if((data->len > 0) && (upload_ctx->file)) {
        // Write file chunk
        http_fs_get()->wr(upload_ctx->file, data->buf, data->len);
    }

    if(data->len >= upload_ctx->len_remain) {
        // End of transfer - close connection
        mg_http_reply(conn, 200, "", "OK");
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

static void api_display_upload_close_callback(struct mg_connection* conn) {
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

static bool api_display_upload_parse_parameters(struct mg_str* params_str, FuriString* file_path) {
    if(params_str->len == 0) {
        return false;
    }

    char temp_str[FILE_NAME_LEN_MAX];

    int var_len = mg_http_get_var(params_str, "app_id", temp_str, sizeof(temp_str));
    if(var_len <= 0) {
        return false;
    }
    furi_string_printf(file_path, "%s/%.*s/", ASSETS_DIR, var_len, temp_str);

    var_len = mg_http_get_var(params_str, "file", temp_str, sizeof(temp_str));
    if(var_len <= 0) {
        return false;
    }
    // TODO: check file extension
    furi_string_cat_printf(file_path, "%.*s", var_len, temp_str);

    return true;
}

static bool api_display_upload_headers_callback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    FuriString* file_path = furi_string_alloc();
    if(api_display_upload_parse_parameters(&msg->query, file_path)) {
        FURI_LOG_I(
            TAG, "Upload len = %u path = %s", msg->body.len, furi_string_get_cstr(file_path));
        // Create upload context
        UploadClientCtx* upload_ctx = malloc(sizeof(UploadClientCtx));
        upload_ctx->len_remain = msg->body.len;
        upload_ctx->file = NULL;

        // Assign callbacks
        ConnectionContext* conn_ctx = (void*)conn->data;
        conn_ctx->on_close = api_display_upload_close_callback;
        conn_ctx->raw.on_data = api_display_upload_data_callback;
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
            mg_http_reply(conn, 500, "", "FS error");
            conn->is_draining = 1;
        }
    } else {
        mg_http_reply(conn, 400, "", "Bad Request");
        conn->is_draining = 1;
    }

    furi_string_free(file_path);

    mg_iobuf_del(&conn->recv, 0, msg->head.len); // Delete HTTP headers
    conn->pfn = NULL; // Silence HTTP protocol handler, we'll use MG_EV_READ

    return true;
}

static bool
    api_display_draw_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(ctx);
    char file_path[32];
    GuiDisplayId display_id;
    bool success = false;
    UNUSED(display_id);

    do {
        if(msg->query.len == 0) {
            break;
        }

        int var_len = mg_http_get_var(&msg->query, "path", file_path, sizeof(file_path));
        if(var_len <= 0) {
            break;
        }

        // TODO: place some file saving logic in here

        char display_id_str[8];
        var_len = mg_http_get_var(&msg->query, "display", display_id_str, sizeof(display_id_str));
        if(var_len <= 0) {
            break;
        }

        if(strncmp(display_id_str, "front", 5) == 0) {
            display_id = GuiDisplayIdFront;
        } else if(strncmp(display_id_str, "back", 4) == 0) {
            display_id = GuiDisplayIdBack;
        } else {
            break;
        }
        success = true;
    } while(0);

    if(!success) {
        mg_http_reply(conn, 400, "", "Bad Request");
        return true;
    }

    Desktop* desktop = furi_record_open(RECORD_DESKTOP);
    if(!desktop_replace_current_app(desktop, "http_viewer", "")) {
        mg_http_reply(conn, 400, "", "Failed to load app");
        success = false;
    }

    if(success)
        mg_http_reply(conn, 200, "Content-Type: application/json\r\n", "{\"result\":\"OK\"}\n");

    furi_record_close(RECORD_DESKTOP);

    return true;
}

static bool api_display_delete_callback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(conn);
    UNUSED(msg);
    UNUSED(ctx);
    FURI_LOG_I(TAG, "DELETE");

    Loader* loader = furi_record_open(RECORD_LOADER);
    loader_stop(loader);
    furi_record_close(RECORD_LOADER);
    mg_http_reply(conn, 200, "", "OK");
    return true;
}

static const HttpHandler handlers_display[] = {
    {
        .uri = "/*/*/upload",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_headers = api_display_upload_headers_callback,
    },
    {
        .uri = "/*/*/draw",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_display_draw_callback,
    },
    {
        .uri = "/*/*/draw",
        .method = "DELETE",
        .type = HttpHandlerCustom,
        .on_request = api_display_delete_callback,
    },
};

typedef struct {
    HttpHandlersList_t handlers;
} ApiDisplayCtx;

void* http_api_display_alloc(void) {
    ApiDisplayCtx* context = malloc(sizeof(ApiDisplayCtx));
    HttpHandlersList_init(context->handlers);

    for(size_t i = COUNT_OF(handlers_display); i > 0; i--) {
        http_handler_add(context->handlers, &handlers_display[i - 1]);
    }

    // Create assets directory
    http_fs_get()->mkd(ASSETS_DIR);
    return context;
}

void http_api_display_free(void* ctx) {
    furi_assert(ctx);
    ApiDisplayCtx* context = ctx;
    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_display_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    ApiDisplayCtx* context = ctx;
    return http_handle_request(context->handlers, conn, msg);
}

bool http_api_display_hdr_callback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiDisplayCtx* context = ctx;
    return http_handle_headers(context->handlers, conn, msg);
}
