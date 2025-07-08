#include "http_api.h"
#include <loader/loader.h>
#include <desktop/desktop.h>
#include <gui/gui.h>
#include <toolbox/path.h>

#define TAG "HttpStorage"

#define FILE_PATH_LEN_MAX 64

typedef struct {
    size_t len_remain;
    void* file;
} WriteClientCtx;

static bool api_storage_parse_parameters(struct mg_str* params_str, FuriString* file_path) {
    if(params_str->len == 0) {
        return false;
    }

    char temp_str[FILE_PATH_LEN_MAX];

    int var_len = mg_http_get_var(params_str, "path", temp_str, sizeof(temp_str));
    if(var_len <= 0) {
        return false;
    }
    furi_string_printf(file_path, "%.*s", var_len, temp_str);
    if(furi_string_end_with(file_path, "/")) {
        furi_string_left(file_path, furi_string_size(file_path) - 1);
    }

    return furi_string_start_with(file_path, STORAGE_EXT_PATH_PREFIX);
}

static void api_storage_write_data_callback(struct mg_connection* conn, struct mg_iobuf* data) {
    ConnectionContext* conn_ctx = (void*)conn->data;
    WriteClientCtx* write_ctx = conn_ctx->context;

    bool do_close_file = false;

    if((data->len > 0) && (write_ctx->file)) {
        // Write file chunk
        if(http_fs_get()->wr(write_ctx->file, data->buf, data->len) != data->len) {
            FURI_LOG_E(TAG, "Failed to write file chunk");
            MG_REPLY_INTERNAL_ERROR(conn, "Failed to write file chunk");
            do_close_file = true;
        } else {
            write_ctx->len_remain -= data->len;
            FURI_LOG_T(
                TAG,
                "Wrote %zu bytes to file, remaining %zu bytes",
                data->len,
                write_ctx->len_remain);
        }
    }

    if(write_ctx->len_remain == 0) {
        MG_REPLY_OK(conn);
        do_close_file = true; // End of write
    }

    if(do_close_file) { // error or end of write
        FURI_LOG_I(TAG, "Closing file after write data");
        if(write_ctx->file) {
            http_fs_get()->cl(write_ctx->file);
            write_ctx->file = NULL;
        }
        conn->is_draining = 1; // Drain connection
    }
    data->len = 0;
}

static void api_storage_write_close_callback(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (void*)conn->data;
    WriteClientCtx* write_ctx = conn_ctx->context;
    if(write_ctx->file) {
        http_fs_get()->cl(write_ctx->file);
        write_ctx->file = NULL;
    }
    free(write_ctx);
    conn_ctx->on_close = NULL;
    conn_ctx->raw.on_data = NULL;
    conn_ctx->context = NULL;
}

static bool api_storage_write_headers_callback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    FuriString* file_path = furi_string_alloc();
    if(api_storage_parse_parameters(&msg->query, file_path)) {
        FURI_LOG_I(
            TAG, "Write len = %u path = %s", msg->body.len, furi_string_get_cstr(file_path));
        // Create write context
        WriteClientCtx* write_ctx = malloc(sizeof(WriteClientCtx));
        write_ctx->len_remain = msg->body.len;
        write_ctx->file = NULL;

        // Assign callbacks
        ConnectionContext* conn_ctx = (void*)conn->data;
        conn_ctx->on_close = api_storage_write_close_callback;
        conn_ctx->raw.on_data = api_storage_write_data_callback;
        conn_ctx->context = write_ctx;

        const char* path = furi_string_get_cstr(file_path);
        http_fs_get()->rm(path); // Delete file if it exists
        FuriString* dir_path = furi_string_alloc();
        path_extract_dirname(path, dir_path);
        http_fs_get()->mkd(furi_string_get_cstr(dir_path));
        furi_string_free(dir_path);
        write_ctx->file = http_fs_get()->op(path, MG_FS_WRITE); // Open file for writing

        if(write_ctx->file == NULL) {
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

    // Also handle possible data in the buffer
    api_storage_write_data_callback(conn, &conn->recv);

    return true;
}

static bool
    api_storage_read_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(ctx);

    FuriString* file_path = furi_string_alloc();
    bool success = false;
    if(api_storage_parse_parameters(&msg->query, file_path)) {
        Storage* fs_api = furi_record_open(RECORD_STORAGE);
        success = storage_file_exists(fs_api, furi_string_get_cstr(file_path));
        furi_record_close(RECORD_STORAGE);
    }

    if(success) {
        FuriString* filename = furi_string_alloc();
        path_extract_filename(file_path, filename, false);
        FuriString* content_header = furi_string_alloc_printf(
            "Content-Disposition: attachment; filename=\"%s\"\r\n",
            furi_string_get_cstr(filename));
        furi_string_free(filename);

        struct mg_http_serve_opts opts = {
            .ssi_pattern = NULL,
            .extra_headers = furi_string_get_cstr(content_header),
            .mime_types = "*=application/octet-stream",
            .page404 = NULL,
            .fs = http_fs_get(),
        };
        mg_http_serve_file(conn, msg, furi_string_get_cstr(file_path), &opts);
        furi_string_free(content_header);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    furi_string_free(file_path);

    return true;
}

static bool api_storage_delete_callback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    FuriString* file_path = furi_string_alloc();
    bool success = false;
    if(api_storage_parse_parameters(&msg->query, file_path)) {
        Storage* fs_api = furi_record_open(RECORD_STORAGE);
        success = storage_simply_remove_recursive(fs_api, furi_string_get_cstr(file_path));
        furi_record_close(RECORD_STORAGE);
    }
    furi_string_free(file_path);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    return true;
}

static bool
    api_storage_mkdir_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(ctx);

    FuriString* dir_path = furi_string_alloc();
    bool success = false;
    if(api_storage_parse_parameters(&msg->query, dir_path)) {
        success = http_fs_get()->mkd(furi_string_get_cstr(dir_path));
    }
    furi_string_free(dir_path);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    return true;
}

static bool
    api_storage_list_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(ctx);

    FuriString* dir_path = furi_string_alloc();
    bool success = false;

    FuriString* json_list = furi_string_alloc();

    if(api_storage_parse_parameters(&msg->query, dir_path)) {
        Storage* api = furi_record_open(RECORD_STORAGE);
        File* file = storage_file_alloc(api);
        if(storage_dir_open(file, furi_string_get_cstr(dir_path))) {
            success = true;
            FileInfo fileinfo;
            char name[FILE_PATH_LEN_MAX];
            bool is_first = true;

            while(storage_dir_read(file, &fileinfo, name, FILE_PATH_LEN_MAX)) {
                if(!is_first) {
                    furi_string_cat(json_list, ",");
                }
                is_first = false;
                if(file_info_is_dir(&fileinfo)) {
                    furi_string_cat_printf(json_list, "{\"type\":\"dir\",\"name\":\"%s\"}", name);
                } else {
                    furi_string_cat_printf(
                        json_list,
                        "{\"type\":\"file\",\"name\":\"%s\",\"size\":%lu}",
                        name,
                        (uint32_t)(fileinfo.size));
                }
            }
        }
        storage_dir_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
    }

    if(success) {
        MG_REPLY_OK_BODY(conn, "{\"list\":[%s]}\n", furi_string_get_cstr(json_list));
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    furi_string_free(dir_path);
    furi_string_free(json_list);

    return true;
}

static const HttpHandler handlers_storage[] = {
    {
        .uri = "/api/v0/storage/write",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_headers = api_storage_write_headers_callback,
    },
    {
        .uri = "/api/v0/storage/read",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = api_storage_read_callback,
    },
    {
        .uri = "/api/v0/storage/remove",
        .method = "DELETE",
        .type = HttpHandlerCustom,
        .on_request = api_storage_delete_callback,
    },
    {
        .uri = "/api/v0/storage/mkdir",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_storage_mkdir_callback,
    },
    {
        .uri = "/api/v0/storage/list",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = api_storage_list_callback,
    },
};

typedef struct {
    HttpHandlersList_t handlers;
} ApistorageCtx;

void* http_api_storage_alloc(void) {
    ApistorageCtx* context = malloc(sizeof(ApistorageCtx));
    HttpHandlersList_init(context->handlers);

    for(size_t i = COUNT_OF(handlers_storage); i > 0; i--) {
        http_handler_add(context->handlers, &handlers_storage[i - 1]);
    }
    return context;
}

void http_api_storage_free(void* ctx) {
    furi_assert(ctx);
    ApistorageCtx* context = ctx;
    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_storage_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    ApistorageCtx* context = ctx;
    return http_handle_request(context->handlers, conn, msg);
}

bool http_api_storage_hdr_callback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApistorageCtx* context = ctx;
    return http_handle_headers(context->handlers, conn, msg);
}
