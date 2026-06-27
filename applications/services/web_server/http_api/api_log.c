#include "http_api.h"
#include <log_storage/log_storage.h>

#define HTTP_API_LOG_DUMP_PATH_MAX 128

bool http_api_log_dump_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    char file_path[HTTP_API_LOG_DUMP_PATH_MAX];
    const char* dump_path =
        (mg_http_get_var(&msg->query, "path", file_path, sizeof(file_path)) > 0) ? file_path :
                                                                                   NULL;

    LogStorage* log_storage = furi_record_open(RECORD_LOG_STORAGE);
    bool is_successful = log_storage_dump(log_storage, dump_path);
    furi_record_close(RECORD_LOG_STORAGE);

    if(is_successful) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_INTERNAL_ERROR(conn, "Failed to dump logs.");
    }

    return true;
}
