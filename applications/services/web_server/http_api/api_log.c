#include "http_api.h"
#include <log_storage/log_storage.h>

#include <toolbox/path.h>

#define HTTP_API_LOG_DUMP_FILENAME_MAX 64

static bool http_api_log_filename_is_valid(const char* chars, size_t size) {
    while(size-- > 0) {
        char character = *chars++;

        if((character < 'a' || character > 'z') && (character < 'A' || character > 'Z') &&
           (character < '0' || character > '9') && (character != '_') && (character != '-')) {
            return false;
        }
    }

    return true;
}

bool http_api_log_dump_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    char filename[HTTP_API_LOG_DUMP_FILENAME_MAX];
    int filename_length = mg_http_get_var(&msg->query, "filename", filename, sizeof(filename));

    const char* full_path = NULL;
    FuriString* full_path_builder = NULL;
    if(filename_length >= 0) {
        if(filename_length == 0 || !http_api_log_filename_is_valid(filename, filename_length)) {
            MG_REPLY_BAD_REQUEST(conn);
            return true;
        }

        full_path_builder = furi_string_alloc();
        path_concat(STORAGE_EXT_PATH_PREFIX, filename, full_path_builder);
        furi_string_cat_printf(full_path_builder, ".txt");
        full_path = furi_string_get_cstr(full_path_builder);
    }

    LogStorage* log_storage = furi_record_open(RECORD_LOG_STORAGE);
    bool is_successful = log_storage_dump(log_storage, full_path);
    furi_record_close(RECORD_LOG_STORAGE);

    if(is_successful) {
        const char* result_path = full_path ? full_path : LOG_STORAGE_DUMP_DEFAULT_FILE_PATH;
        MG_REPLY_OK_BODY(conn, "{\"result\":\"OK\",\"path\":\"%s\"}\n", result_path);
    } else {
        MG_REPLY_ERROR(conn, 508, "Failed to dump logs.");
    }

    if(full_path_builder) furi_string_free(full_path_builder);

    return true;
}
