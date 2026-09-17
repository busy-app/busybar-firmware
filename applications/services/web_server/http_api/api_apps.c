#include "http_api.h"

#include <furi.h>
#include <toolbox/timers.h>

#include <storage_utils/temp_file.h>

#include <js_app_installer/js_app_installer_paths.h>
#include <js_app_installer/js_app_installer.h>
#include <js_app/js_app_registry.h>
#include <js_app/js_app_common.h>

#include <cjson/cJSON.h>

#define TAG "HttpApiApps"

#define MAX_UPLOAD_FILE_SIZE (100 * 1024 * 1024)

#define APP_UPLOAD_IDLE_TIMEOUT_MS 5000

#define INSTALL_KEY_LEN_MAX 11
#define APP_ID_LEN_MAX      (JS_APP_ID_LEN_MAX + 2)

typedef struct {
    Storage* storage;
    TempFile* update_file;

    FuriThreadPriority original_thread_priority;

    size_t total_file_size; // Expected total size from Content-Length
    size_t received_file_size; // Bytes received so far

    CoarseTimer timeout_timer;
} HttpInstallHandlerCtx;

static void api_apps_on_data_cb(struct mg_connection* conn, struct mg_iobuf* io);
static void api_apps_on_close_cb(struct mg_connection* conn);

static HttpInstallHandlerCtx* alloc_install_context() {
    HttpInstallHandlerCtx* ctx = malloc(sizeof(HttpInstallHandlerCtx));
    ctx->storage = furi_record_open(RECORD_STORAGE);
    ctx->update_file = temp_file_alloc(ctx->storage);

    ctx->original_thread_priority = furi_thread_get_current_priority();

    ctx->total_file_size = 0;
    ctx->received_file_size = 0;
    return ctx;
}

static void free_install_context(HttpInstallHandlerCtx* ctx) {
    if(!ctx) return;

    furi_thread_set_current_priority(ctx->original_thread_priority);

    if(ctx->update_file) {
        temp_file_remove(ctx->update_file);
        temp_file_free(ctx->update_file);
        ctx->update_file = NULL;
    }

    if(ctx->storage) {
        furi_record_close(RECORD_STORAGE);
        ctx->storage = NULL;
    }

    free(ctx);
}

static const char* const installer_errors[] = {
    [JsAppInstallerErrorNone] = "OK",
    [JsAppInstallerErrorUnpack] = "Cannot unpack",
    [JsAppInstallerErrorManifest] = "Wrong app manifest",
    [JsAppInstallerErrorInstall] = "Installation failed",
};

static const char* const installer_error_codes[] = {
    [JsAppInstallerErrorNone] = "ok",
    [JsAppInstallerErrorUnpack] = "unpack_error",
    [JsAppInstallerErrorManifest] = "manifest_error",
    [JsAppInstallerErrorInstall] = "installation_failed",
};

static_assert(COUNT_OF(installer_errors) == JsAppInstallerErrorMax);
static_assert(COUNT_OF(installer_error_codes) == JsAppInstallerErrorMax);

static cJSON* serialize_app_info(const JsAppInfo* info);

static bool
    handle_completed_upload(HttpInstallHandlerCtx* install_ctx, struct mg_connection* conn) {
    FURI_LOG_I(TAG, "upload completed");
    UNUSED(install_ctx);

    JsAppInstaller* installer = furi_record_open(RECORD_JS_APP_INSTALLER);

    JsAppInstallerStageResult result = js_app_installer_stage(installer);

    cJSON* root = cJSON_CreateObject();
    int status_code = 200;
    if(result.error == JsAppInstallerErrorNone) {
        cJSON_AddStringToObject(root, "result", "OK");
        cJSON_AddNumberToObject(root, "install_key", result.install_key);

        JsAppInfo info;

        furi_check(js_app_get_info(result.staged_app, &info));
        cJSON* staged = serialize_app_info(&info);
        cJSON_AddItemToObject(root, "staged", staged);
        js_app_free(result.staged_app);

        if(result.installed_app) {
            if(js_app_get_info(result.installed_app, &info)) {
                cJSON* installed = serialize_app_info(&info);
                cJSON_AddItemToObject(root, "installed", installed);
            }
            js_app_free(result.installed_app);
        }
        status_code = 200;
    } else {
        cJSON_AddStringToObject(root, "error", installer_errors[result.error]);
        cJSON_AddStringToObject(root, "error_code", installer_error_codes[result.error]);
        status_code = 400;
    }

    char* json = cJSON_PrintUnformatted(root);

    mg_http_reply(conn, status_code, DEFAULT_JSON_HEADERS "Connection: close\r\n", "%s", json);

    free(json);
    cJSON_Delete(root);

    conn->is_draining = 1;

    furi_record_close(RECORD_JS_APP_INSTALLER);
    return true;
}

static void api_apps_on_data_cb(struct mg_connection* conn, struct mg_iobuf* io) {
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    HttpInstallHandlerCtx* install_ctx = (HttpInstallHandlerCtx*)conn_ctx->context;

    if(!install_ctx || !install_ctx->update_file) {
        FURI_LOG_E(TAG, "on_data: Context or file saver invalid/closed. Draining.");
        MG_REPLY_ERROR_CLOSE(conn, 409, "Update context invalid");
        mg_iobuf_del(io, 0, io->len); // Consume data to prevent further calls
        conn->is_draining = 1; // Mark connection to be closed
        return;
    }

    install_ctx->timeout_timer = coarse_timer_create(APP_UPLOAD_IDLE_TIMEOUT_MS);

    size_t data_len = io->len;
    FURI_LOG_T(
        TAG,
        "on_data: Received %zu bytes. Total received: %zu / %zu",
        data_len,
        install_ctx->received_file_size,
        install_ctx->total_file_size);

    if(data_len > 0) {
        if(install_ctx->received_file_size + data_len > install_ctx->total_file_size) {
            FURI_LOG_E(
                TAG,
                "on_data: Received more data than expected. Expected %zu, got %zu more.",
                install_ctx->total_file_size,
                (install_ctx->received_file_size + data_len) - install_ctx->total_file_size);
            MG_REPLY_ERROR_CLOSE(conn, 413, "Payload Too Large");
            conn->is_draining = 1;
            mg_iobuf_del(io, 0, io->len);
            return;
        }

        if(!temp_file_write(install_ctx->update_file, io->buf, data_len)) {
            FURI_LOG_E(
                TAG, "on_data: Failed to write data to temp TAR file. Wrote %zu bytes.", data_len);
            MG_REPLY_ERROR_CLOSE(conn, 508, "Failed to save update package (write error).");
            conn->is_draining = 1;
            mg_iobuf_del(io, 0, io->len);
            return;
        }

        install_ctx->received_file_size += data_len;
    }

    mg_iobuf_del(io, 0, io->len); // Consume all data from buffer

    if(install_ctx->received_file_size >= install_ctx->total_file_size) {
        FURI_LOG_I(TAG, "on_data: All data received (%zu bytes)", install_ctx->received_file_size);

        temp_file_free(install_ctx->update_file);
        install_ctx->update_file = NULL;

        if(!handle_completed_upload(install_ctx, conn)) {
            // Error response already sent by handle_completed_upload
            FURI_LOG_E(TAG, "on_data: package handling failed.");
        }

        Storage* storage = furi_record_open(RECORD_STORAGE);
        storage_simply_remove(storage, JS_APP_DOWNLOAD_PATH);
        furi_record_close(RECORD_STORAGE);
    }
}

static void api_apps_on_close_cb(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    HttpInstallHandlerCtx* install_ctx = (HttpInstallHandlerCtx*)conn_ctx->context;

    FURI_LOG_D(TAG, "on_close");

    if(install_ctx) {
        free_install_context(install_ctx);
        conn_ctx->context = NULL;
    }

    // Clear callbacks
    conn_ctx->raw.on_data = NULL;
    conn_ctx->raw.on_poll = NULL;
    conn_ctx->on_close = NULL;
}

static void api_apps_on_poll_cb(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (void*)conn->data;
    HttpInstallHandlerCtx* install_ctx = conn_ctx->context;
    furi_assert(install_ctx);

    if(coarse_timer_is_expired(install_ctx->timeout_timer)) {
        FURI_LOG_E(TAG, "Connection data timeout (%lu)", conn->id);
        MG_REPLY_TIMEOUT(conn, "Upload timeout");
        conn->is_draining = 1; // Force close hanging connection
    }
}

static bool api_apps_stage_hdr_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* http_handler_ctx) {
    UNUSED(http_handler_ctx);
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    HttpInstallHandlerCtx* install_ctx = NULL;

    if(!IS_HTTP_ENDPOINT(path)) return false;

    if(method == HttpMethodOptions) return false; // let MG_EV_HTTP_MSG respond with preflight
    if(method != HttpMethodPost) {
        http_reply_405_method_not_allowed(conn, HttpMethodPost, true);
        conn->is_draining = 1;
        return true;
    }

    if(msg->body.len == 0) {
        FURI_LOG_W(TAG, "on_headers: Content-Length is 0 or missing/invalid. No file to upload?");
        MG_REPLY_ERROR_CLOSE(conn, 400, "Bad Request");
        conn->is_draining = 1;
        return true;
    }

    install_ctx = alloc_install_context();
    conn_ctx->raw.on_data = api_apps_on_data_cb;
    conn_ctx->raw.on_poll = api_apps_on_poll_cb;
    conn_ctx->on_close = api_apps_on_close_cb;
    conn_ctx->context = install_ctx;

    install_ctx->timeout_timer = coarse_timer_create(APP_UPLOAD_IDLE_TIMEOUT_MS);
    install_ctx->total_file_size = msg->body.len;
    if(install_ctx->total_file_size > MAX_UPLOAD_FILE_SIZE) {
        FURI_LOG_E(
            TAG,
            "on_headers: File size %zu exceeds max %u.",
            install_ctx->total_file_size,
            MAX_UPLOAD_FILE_SIZE);
        MG_REPLY_ERROR_CLOSE(conn, 413, "Payload Too Large");
        conn->is_draining = 1;
        return true;
    }
    FURI_LOG_I(TAG, "on_headers: Expecting file of size: %zu bytes", install_ctx->total_file_size);

    furi_thread_set_current_priority(FuriThreadPriorityLow);

    if(!temp_file_create(install_ctx->update_file, JS_APP_DOWNLOAD_PATH)) {
        FURI_LOG_E(
            TAG, "on_headers: Failed to initialize file saver for: %s", JS_APP_DOWNLOAD_PATH);
        MG_REPLY_ERROR_CLOSE(conn, 508, "Failed to save update package (file init error).");
        conn->is_draining = 1;
        return true;
    }

    FURI_LOG_I(TAG, "on_headers: Initialized file saver for: %s", JS_APP_DOWNLOAD_PATH);

    mg_iobuf_del(&conn->recv, 0, msg->head.len); // Delete HTTP headers
    conn->pfn = NULL; // Silence HTTP protocol handler, we'll use MG_EV_READ

    // Also handle possible data in the buffer
    api_apps_on_data_cb(conn, &conn->recv);

    return true;
}

static bool api_apps_stage_request_callback(
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

static bool api_apps_install_request_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    bool success = false;
    do {
        char install_key_str[INSTALL_KEY_LEN_MAX];
        int install_key_len =
            mg_http_get_var(&msg->query, "install_key", install_key_str, INSTALL_KEY_LEN_MAX);
        if(install_key_len <= 0) {
            break;
        }
        char* endptr = NULL;
        uint32_t install_key = strtoul(install_key_str, &endptr, 10);
        if(*endptr) {
            FURI_LOG_E(TAG, "Invalid install_key: '%s'", install_key_str);
            break;
        }

        JsAppInstaller* installer = furi_record_open(RECORD_JS_APP_INSTALLER);
        JsAppInstallerError error = js_app_installer_install(installer, install_key);
        success = error == JsAppInstallerErrorNone;
        furi_record_close(RECORD_JS_APP_INSTALLER);
    } while(false);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    return true;
}

static cJSON* serialize_app_info(const JsAppInfo* info) {
    cJSON* entry = cJSON_CreateObject();
    cJSON_AddStringToObject(entry, "id", info->manifest.id);
    cJSON_AddStringToObject(entry, "name", info->manifest.name);
    cJSON_AddStringToObject(entry, "version", info->manifest.version);
    cJSON_AddStringToObject(entry, "author", info->manifest.author);
    cJSON_AddStringToObject(entry, "description", info->manifest.description);
    cJSON_AddBoolToObject(entry, "is_debug", info->manifest.is_debug);
    cJSON_AddStringToObject(entry, "icon_path", info->path.icon.front);
    return entry;
}

static void app_list_callback(const JsAppInfo* info, void* context) {
    cJSON* list = context;

    cJSON* entry = serialize_app_info(info);

    cJSON_AddItemToArray(list, entry);
}

static bool api_apps_list_request_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    cJSON* root = cJSON_CreateObject();
    cJSON* list = cJSON_AddArrayToObject(root, "apps");
    js_app_registry_list_apps(app_list_callback, list);

    char* json = cJSON_PrintUnformatted(root);

    mg_http_reply(conn, 200, DEFAULT_JSON_HEADERS "Connection: close\r\n", "%s", json);

    free(json);
    cJSON_Delete(root);

    return true;
}

static bool api_apps_delete_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;
    if(method == HttpMethodOptions) {
        http_reply_cors_preflight(conn, HttpMethodDelete);
        return true;
    } else if(method != HttpMethodDelete) {
        http_reply_405_method_not_allowed(conn, HttpMethodDelete, false);
        return true;
    }

    char app_id[APP_ID_LEN_MAX];
    int app_id_len = mg_http_get_var(&msg->query, "app_id", app_id, APP_ID_LEN_MAX);
    if(app_id_len <= 0) {
        MG_REPLY_BAD_REQUEST(conn);
    } else {
        JsAppRegistryAppUninstallResult uninstall_result = js_app_registry_uninstall_app(app_id);
        switch(uninstall_result) {
        case JsAppRegistryAppUninstallResultOk:
            MG_REPLY_OK(conn);
            break;
        case JsAppRegistryAppUninstallResultNotFound:
            MG_REPLY_NOT_FOUND(conn);
            break;
        case JsAppRegistryAppUninstallResultStorageError:
            MG_REPLY_ERROR(conn, 508, "filesystem error");
            break;
        default:
            furi_check(false);
        }
    }

    return true;
}

static const HttpHandler api_apps_handlers[] = {
    {
        .uri = "",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .on_request = api_apps_delete_callback,
    },
    {
        .uri = "stage",
        .method = HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_apps_stage_request_callback,
        .on_headers = api_apps_stage_hdr_callback,
    },
    {
        .uri = "install",
        .method = HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_apps_install_request_callback,
    },
    {
        .uri = "list",
        .method = HttpMethodGet,
        .type = HttpHandlerCustom,
        .on_request = api_apps_list_request_callback,
    },
};

typedef struct {
    HttpHandlersList_t handlers;
} ApiAppsCtx;

void* http_api_apps_alloc(void) {
    ApiAppsCtx* context = malloc(sizeof(*context));
    HttpHandlersList_init(context->handlers);

    for(size_t i = COUNT_OF(api_apps_handlers); i > 0; i--) {
        http_handler_add(context->handlers, &api_apps_handlers[i - 1]);
    }

    return context;
}

void http_api_apps_free(void* ctx) {
    furi_assert(ctx);

    ApiAppsCtx* context = ctx;

    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_apps_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiAppsCtx* context = ctx;

    return http_handle_request(path, method, context->handlers, conn, msg);
}

bool http_api_apps_hdr_callback_root(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiAppsCtx* context = ctx;

    return http_handle_headers(path, method, context->handlers, conn, msg);
}
