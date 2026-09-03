#include "http_api.h"
#include <loader/loader.h>
#include <desktop/desktop.h>
#include <gui/gui.h>
#include <toolbox/path.h>
#include <cjson/cJSON.h>

#define TAG "HttpStorage"

#define FILE_PATH_LEN_MAX 64

static bool api_storage_parse_path_parameter(
    struct mg_str* params_str,
    const char* name,
    FuriString* path) {
    if(params_str->len == 0) {
        return false;
    }

    char temp_str[FILE_PATH_LEN_MAX];

    int var_len = mg_http_get_var(params_str, name, temp_str, sizeof(temp_str));
    if(var_len <= 0) {
        return false;
    }
    furi_string_printf(path, "%.*s", var_len, temp_str);
    if(furi_string_end_with(path, "/")) {
        furi_string_left(path, furi_string_size(path) - 1);
    }

    if(!mg_path_is_sane(mg_str(furi_string_get_cstr(path)))) {
        return false;
    }

    return furi_string_start_with(path, STORAGE_EXT_PATH_PREFIX);
}

static bool api_storage_parse_append_parameter(struct mg_str* params_str, bool* append) {
    char value_str[2];

    int value_len = mg_http_get_var(params_str, "append", value_str, sizeof(value_str));
    if(value_len == 1) {
        if(value_str[0] == '1') {
            *append = true;
        } else if(value_str[0] == '0') {
            *append = false;
        } else {
            return false;
        }
    } else if(value_len != -4) { /* -4 = var not present, see mg_http_get_var */
        return false;
    }

    return true;
}

static bool api_storage_write_headers_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FuriString* file_path = furi_string_alloc();
    bool append = false;
    if(api_storage_parse_path_parameter(&msg->query, "path", file_path) &&
       api_storage_parse_append_parameter(&msg->query, &append)) {
        http_upload_start(conn, msg, furi_string_get_cstr(file_path), append);
    } else {
        MG_REPLY_ERROR_CLOSE(conn, 400, "Bad Request");
    }

    furi_string_free(file_path);

    return true;
}

static bool api_storage_write_request_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(path);
    UNUSED(msg);
    UNUSED(ctx);

    if(method == HttpMethodOptions) {
        http_reply_cors_preflight(conn, HttpMethodPost);
        return true;
    }

    MG_REPLY_BAD_REQUEST(conn);

    return true;
}

static bool api_storage_filename_is_header_safe(const FuriString* filename) {
    return furi_string_search_char(filename, '"', 0) == FURI_STRING_FAILURE &&
           furi_string_search_char(filename, '\r', 0) == FURI_STRING_FAILURE &&
           furi_string_search_char(filename, '\n', 0) == FURI_STRING_FAILURE;
}

static bool api_storage_read_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FuriString* file_path = furi_string_alloc();
    bool success = false;
    if(api_storage_parse_path_parameter(&msg->query, "path", file_path)) {
        Storage* fs_api = furi_record_open(RECORD_STORAGE);
        success = storage_file_exists(fs_api, furi_string_get_cstr(file_path));
        furi_record_close(RECORD_STORAGE);
    }

    if(success) {
        FuriString* filename = furi_string_alloc();
        path_extract_filename(file_path, filename, false);
        if(!api_storage_filename_is_header_safe(filename)) {
            furi_string_free(filename);
            furi_string_free(file_path);
            MG_REPLY_BAD_REQUEST(conn);
            return true;
        }
        FuriString* content_header = furi_string_alloc_printf(
            "Content-Disposition: attachment; filename=\"%s\"\r\n",
            furi_string_get_cstr(filename));
        furi_string_free(filename);

        furi_string_cat(content_header, HEADER_CORS);

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
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FuriString* file_path = furi_string_alloc();
    bool success = false;
    if(api_storage_parse_path_parameter(&msg->query, "path", file_path)) {
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

static bool api_storage_mkdir_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FuriString* dir_path = furi_string_alloc();
    bool success = false;
    if(api_storage_parse_path_parameter(&msg->query, "path", dir_path)) {
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

static bool api_storage_rename_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FuriString* old_path = furi_string_alloc();
    FuriString* new_path = furi_string_alloc();
    bool success = api_storage_parse_path_parameter(&msg->query, "path", old_path) &&
                   api_storage_parse_path_parameter(&msg->query, "new_path", new_path);

    if(success) {
        success =
            http_fs_get()->mv(furi_string_get_cstr(old_path), furi_string_get_cstr(new_path));
    }

    furi_string_free(old_path);
    furi_string_free(new_path);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    return true;
}

static bool api_storage_list_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FuriString* dir_path = furi_string_alloc();
    bool success = false;

    cJSON* list_array = cJSON_CreateArray();

    if(api_storage_parse_path_parameter(&msg->query, "path", dir_path)) {
        Storage* api = furi_record_open(RECORD_STORAGE);
        File* file = storage_file_alloc(api);
        if(storage_dir_open(file, furi_string_get_cstr(dir_path))) {
            success = true;
            FileInfo fileinfo;
            char name[FILE_PATH_LEN_MAX];

            while(storage_dir_read(file, &fileinfo, name, FILE_PATH_LEN_MAX)) {
                cJSON* entry = cJSON_CreateObject();
                if(file_info_is_dir(&fileinfo)) {
                    cJSON_AddStringToObject(entry, "type", "dir");
                    cJSON_AddStringToObject(entry, "name", name);
                } else {
                    cJSON_AddStringToObject(entry, "type", "file");
                    cJSON_AddStringToObject(entry, "name", name);
                    cJSON_AddNumberToObject(entry, "size", (double)(fileinfo.size));
                }
                cJSON_AddItemToArray(list_array, entry);
            }
        }
        storage_dir_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
    }

    if(success) {
        cJSON* root = cJSON_CreateObject();
        cJSON_AddItemToObject(root, "list", list_array);
        char* json_str = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
        MG_REPLY_OK_BODY(conn, "%s\n", json_str);
        free(json_str);
    } else {
        cJSON_Delete(list_array);
        MG_REPLY_BAD_REQUEST(conn);
    }

    furi_string_free(dir_path);

    return true;
}

static bool api_storage_status_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    uint64_t used_bytes, free_bytes, total_bytes;
    bool read_only;
    Storage* api = furi_record_open(RECORD_STORAGE);
    FS_Error error = storage_common_fs_info(
        api, STORAGE_EXT_PATH_PREFIX, &total_bytes, &free_bytes, &read_only);

    if(error == FSE_OK) {
        used_bytes = total_bytes - free_bytes;
        MG_REPLY_OK_BODY(
            conn,
            "{\"used_bytes\":%llu,\"free_bytes\":%llu,\"total_bytes\":%llu}\n",
            used_bytes,
            free_bytes,
            total_bytes);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    furi_record_close(RECORD_STORAGE);

    return true;
}

static const HttpHandler handlers_storage[] = {
    {
        .uri = "write",
        .method = HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_headers = api_storage_write_headers_callback,
        .on_request = api_storage_write_request_callback,
    },
    {
        .uri = "read",
        .method = HttpMethodGet,
        .type = HttpHandlerCustom,
        .on_request = api_storage_read_callback,
    },
    {
        .uri = "remove",
        .method = HttpMethodDelete,
        .type = HttpHandlerCustom,
        .on_request = api_storage_delete_callback,
    },
    {
        .uri = "mkdir",
        .method = HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_storage_mkdir_callback,
    },
    {
        .uri = "rename",
        .method = HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_storage_rename_callback,
    },
    {
        .uri = "list",
        .method = HttpMethodGet,
        .type = HttpHandlerCustom,
        .on_request = api_storage_list_callback,
    },
    {
        .uri = "status",
        .method = HttpMethodGet,
        .type = HttpHandlerCustom,
        .on_request = api_storage_status_callback,
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

bool http_api_storage_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApistorageCtx* context = ctx;
    return http_handle_request(path, method, context->handlers, conn, msg);
}

bool http_api_storage_hdr_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApistorageCtx* context = ctx;
    return http_handle_headers(path, method, context->handlers, conn, msg);
}
