#include "http_api.h"

#include <storage_utils/temp_file.h>
#include <toolbox/timers.h>
#include <toolbox/url.h>

#include <js_app_installer/js_app_installer_paths.h>
#include <js_app_installer/js_app_installer.h>
#include <js_app_launcher/js_app_launcher.h>
#include <js_app/js_app_registry.h>
#include <js_app/js_app_settings_storage.h>
#include <js_app/js_app_common.h>

#include "../web_server.h"

#define TAG "HttpApiApps"

#define MAX_UPLOAD_FILE_SIZE (100 * 1024 * 1024)

#define APP_UPLOAD_IDLE_TIMEOUT_MS 5000

#define INSTALL_KEY_LEN_MAX 11
#define APP_ID_LEN_MAX      (JS_APP_ID_LEN_MAX + 2)

#define RESULT_KEY     "result"
#define OPERATIONS_KEY "operations"
#define CANCEL_ALL_KEY "all"

#define STATUS_KEY               "status"
#define STATUS_VALUE_PENDING     "pending"
#define STATUS_VALUE_IN_PROGRESS "in_progress"

#define DETAIL_KEY          "detail"
#define DETAIL_STEP_KEY     "step"
#define DETAIL_PROGRESS_KEY "progress"

#define APP_ID_KEY            "app_id"
#define ITEM_ID_KEY           "item_id"
#define ERROR_KEY             "error"
#define OPERATION_KEY         "operation"
#define OPERATION_TYPE_KEY    "type"
#define OPERATION_URL_KEY     "url"
#define OPERATION_VERSION_KEY "version"

#define ERROR_SKIPPED_VALUE "skipped"

#define OPERATION_TYPE_INSTALL_VALUE "install"
#define OPERATION_TYPE_REMOVE_VALUE  "remove"

typedef struct {
    Storage* storage;
    TempFile* update_file;

    FuriThreadPriority original_thread_priority;

    size_t total_file_size; // Expected total size from Content-Length
    size_t received_file_size; // Bytes received so far

    CoarseTimer timeout_timer;
} HttpInstallHandlerCtx;

typedef struct {
    JsAppInstallerOperationType operation_type;
    JsAppInstallerSourceInfo source;
} ApiAppsQueueOperationInfo;

typedef struct {
    const char* app_id;
    JsAppInstallerOperationResult result;
    ApiAppsQueueOperationInfo info;
} ApiAppsQueueOperation;

typedef struct {
    ApiAppsQueueOperation* operations;
    size_t operations_count;
} ApiAppsQueueAddRequest;

typedef struct {
    uint32_t item_id;
    bool is_clear_all;
} ApiAppsQueueCancelRequest;

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
    [JsAppInstallerErrorApiVersion] = "API version mismatch",
    [JsAppInstallerErrorInstall] = "Installation failed",
    [JsAppInstallerErrorTimeout] = "Operation timed out",
    [JsAppInstallerErrorNotFound] = "Operation not found",
    [JsAppInstallerErrorQueueFull] = "Too many operations",
};

static const char* const installer_error_codes[] = {
    [JsAppInstallerErrorNone] = "ok",
    [JsAppInstallerErrorUnpack] = "unpack_error",
    [JsAppInstallerErrorManifest] = "manifest_error",
    [JsAppInstallerErrorApiVersion] = "api_mismatch",
    [JsAppInstallerErrorInstall] = "installation_failed",
    [JsAppInstallerErrorTimeout] = "timeout",
    [JsAppInstallerErrorNotFound] = "not_found",
    [JsAppInstallerErrorQueueFull] = "queue_full",
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
        JsAppInstallerError error = js_app_installer_install_staged(installer, install_key);
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

static bool api_apps_launch_request_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) {
        return false;
    }

    char app_id[APP_ID_LEN_MAX];
    if(mg_http_get_var(&msg->query, "app_id", app_id, APP_ID_LEN_MAX) <= 0) {
        MG_REPLY_BAD_REQUEST(conn);
        return true;
    }

    if(!js_app_is_valid_id(app_id)) {
        MG_REPLY_BAD_REQUEST(conn);
        return true;
    }

    JsApp* app = js_app_registry_get_app(app_id);
    if(app == NULL) {
        MG_REPLY_NOT_FOUND(conn);
        return true;
    }

    JsAppInfo app_info;
    if(!js_app_get_info(app, &app_info)) {
        MG_REPLY_SERVICE_UNAVAILABLE(conn, "Failed to get application info");
        js_app_free(app);
        return true;
    }

    const bool is_compatible =
        web_server_is_api_version_compatible(&app_info.manifest.api_version);

    js_app_free(app);

    if(!is_compatible) {
        MG_REPLY_CONFLICT(conn, "API version mismatch, please update firmware");
        return true;
    }

    const JsAppLauncherStatus status = js_app_launcher_start(app_id, JsAppLauncherStartModeResume);

    if(status == JsAppLauncherStatusOk) {
        MG_REPLY_OK(conn);
    } else if(status == JsAppLauncherStatusLowPriority) {
        MG_REPLY_CONFLICT(conn, "not started due to low priority");
    } else {
        MG_REPLY_SERVICE_UNAVAILABLE(conn, "failed to launch application");
    }

    return true;
}

static bool api_apps_quit_request_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) {
        return false;
    }

    const JsAppLauncherStatus status = js_app_launcher_stop(JsAppLauncherStopModeForget);

    if(status == JsAppLauncherStatusOk) {
        MG_REPLY_OK(conn);
    } else if(status == JsAppLauncherStatusNotRunning) {
        MG_REPLY_CONFLICT(conn, "application is not running");
    } else {
        MG_REPLY_SERVICE_UNAVAILABLE(conn, "failed to quit from application");
    }

    return true;
}

static cJSON* serialize_app_info(const JsAppInfo* info) {
    FuriString* tmp = furi_string_alloc();
    cJSON* entry = cJSON_CreateObject();
    cJSON_AddStringToObject(entry, "id", info->manifest.id);
    cJSON_AddStringToObject(entry, "name", info->manifest.name);
    semver_to_string(&info->manifest.version, tmp);
    cJSON_AddStringToObject(entry, "version", furi_string_get_cstr(tmp));
    semver_to_string(&info->manifest.api_version, tmp);
    cJSON_AddStringToObject(entry, "api_version", furi_string_get_cstr(tmp));
    cJSON_AddStringToObject(entry, "author", info->manifest.author);
    cJSON_AddStringToObject(entry, "description", info->manifest.description);
    cJSON_AddBoolToObject(entry, "is_debug", info->manifest.is_debug);
    cJSON_AddStringToObject(entry, "icon_path", info->path.icon.front);
    furi_string_free(tmp);
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

static bool api_apps_stop_running_app(const char* app_id) {
    bool success = false;
    FuriString* running_app_id = furi_string_alloc();

    do {
        JsAppLauncherStatus status;

        status = js_app_launcher_get_running_app_id(running_app_id);
        if(status != JsAppLauncherStatusOk) {
            success = (status == JsAppLauncherStatusNotRunning);
            break;
        }

        if(!furi_string_equal(running_app_id, app_id)) {
            success = true;
            break;
        }

        status = js_app_launcher_stop(JsAppLauncherStopModeForget);
        if((status != JsAppLauncherStatusOk) && (status != JsAppLauncherStatusNotRunning)) {
            break;
        }

        success = true;
    } while(false);

    furi_string_free(running_app_id);
    return success;
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
        return true;
    }

    if(!api_apps_stop_running_app(app_id)) {
        MG_REPLY_SERVICE_UNAVAILABLE(
            conn, "Failed to quit from application before uninstalling, try again");
        return true;
    }

    JsAppRegistryAppUninstallResult uninstall_result = js_app_registry_uninstall_app(app_id);
    switch(uninstall_result) {
    case JsAppRegistryAppUninstallResultOk:
        MG_REPLY_OK(conn);
        break;
    case JsAppRegistryAppUninstallResultNotFound:
        MG_REPLY_NOT_FOUND(conn);
        break;
    case JsAppRegistryAppUninstallResultStorageError:
        MG_REPLY_ERROR(conn, 508, "Filesystem error");
        break;
    default:
        furi_crash();
    }

    return true;
}

static void api_apps_settings_get(struct mg_connection* conn, JsAppSettingsStorage* storage) {
    FuriString* buffer = furi_string_alloc();

    do {
        if(!js_app_settings_storage_load(storage)) {
            MG_REPLY_ERROR(conn, 508, "Failed to load settings");
            break;
        }

        if(!js_app_settings_storage_export(storage, buffer)) {
            MG_REPLY_SERVICE_UNAVAILABLE(conn, "Failed to build settings document");
            break;
        }

        MG_REPLY_OK_BODY(conn, "%s\n", furi_string_get_cstr(buffer));
    } while(false);

    furi_string_free(buffer);
}

static void api_apps_settings_put(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    JsAppSettingsStorage* storage) {
    do {
        if(!js_app_settings_storage_import(storage, msg->body.buf, msg->body.len)) {
            MG_REPLY_BAD_REQUEST(conn);
            break;
        }

        if(!js_app_settings_storage_save(storage)) {
            MG_REPLY_ERROR(conn, 508, "Failed to save settings");
            break;
        }

        MG_REPLY_OK(conn);
    } while(false);
}

static void api_apps_settings_delete(struct mg_connection* conn, JsAppSettingsStorage* storage) {
    if(js_app_settings_storage_reset(storage)) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_ERROR(conn, 508, "Failed to reset settings");
    }
}

static bool api_apps_settings_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) {
        return false;
    }

    char app_id[JS_APP_ID_LEN_MAX + 1];
    if(mg_http_get_var(&msg->query, "app_id", app_id, COUNT_OF(app_id)) <= 0) {
        MG_REPLY_BAD_REQUEST(conn);
        return true;
    }

    if(!js_app_is_valid_id(app_id)) {
        MG_REPLY_BAD_REQUEST(conn);
        return true;
    }

    JsApp* app = js_app_registry_get_app(app_id);
    if(!app) {
        MG_REPLY_NOT_FOUND(conn);
        return true;
    }
    js_app_free(app);

    if(http_api_is_request_from_js(conn)) {
        bool is_allowed = false;

        FuriString* running_app_id = furi_string_alloc();

        if(js_app_launcher_get_running_app_id(running_app_id) == JsAppLauncherStatusOk) {
            is_allowed = furi_string_equal(running_app_id, app_id);
        }

        furi_string_free(running_app_id);

        if(!is_allowed) {
            MG_REPLY_FORBIDDEN(conn);
            return true;
        }
    }

    JsAppSettingsStorageStatus status;
    JsAppSettingsStorage* storage = js_app_settings_storage_alloc(app_id, &status);

    if(!storage) {
        if(status == JsAppSettingsStorageStatusSchemaMissing) {
            MG_REPLY_ERROR(conn, 404, "Application has no settings schema");
        } else if(status == JsAppSettingsStorageStatusSchemaInvalid) {
            MG_REPLY_SERVICE_UNAVAILABLE(conn, "Application settings schema is invalid");
        } else if(status == JsAppSettingsStorageStatusStorageFailure) {
            MG_REPLY_ERROR(conn, 508, "Failed to read settings schema");
        }
    } else {
        if(method == HttpMethodGet) {
            FURI_LOG_D(TAG, "Loading \"%s\" application settings", app_id);
            api_apps_settings_get(conn, storage);
        } else if(method == HttpMethodPut) {
            FURI_LOG_D(TAG, "Saving \"%s\" application settings", app_id);
            api_apps_settings_put(conn, msg, storage);
        } else if(method == HttpMethodDelete) {
            FURI_LOG_D(TAG, "Resetting \"%s\" application settings", app_id);
            api_apps_settings_delete(conn, storage);
        }

        js_app_settings_storage_free(storage);
    }

    return true;
}

static const char* api_apps_queue_get_step_type_string(JsAppInstallerOperationStepType step_type) {
    furi_assert(step_type < JsAppInstallerOperationStepTypeMax);

    static const char* const step_types_table[] = {
        [JsAppInstallerOperationStepTypeDownloading] = "downloading",
        [JsAppInstallerOperationStepTypeUnpacking] = "unpacking",
        [JsAppInstallerOperationStepTypeRemoving] = "removing",
    };

    return step_types_table[step_type];
}

static void api_apps_queue_item_operation_format(
    cJSON* item_json,
    JsAppInstallerOperationType operation_type,
    const JsAppInstallerAppVersion* app_version) {
    cJSON* operation_json = cJSON_AddObjectToObject(item_json, OPERATION_KEY);

    if(operation_type == JsAppInstallerOperationTypeInstall) {
        cJSON_AddStringToObject(operation_json, OPERATION_TYPE_KEY, OPERATION_TYPE_INSTALL_VALUE);

        if(app_version->is_present) {
            FuriString* version_string = furi_string_alloc();
            semver_to_string(&app_version->value, version_string);
            cJSON_AddStringToObject(
                operation_json, OPERATION_VERSION_KEY, furi_string_get_cstr(version_string));
            furi_string_free(version_string);
        }

    } else if(operation_type == JsAppInstallerOperationTypeRemove) {
        cJSON_AddStringToObject(operation_json, OPERATION_TYPE_KEY, OPERATION_TYPE_REMOVE_VALUE);
    } else {
        furi_crash("Invalid JsAppInstallerOperationType value");
    }
}

static void api_apps_queue_item_detail_format(
    cJSON* item_json,
    const JsAppInstallerOperationDetail* detail) {
    cJSON* detail_json = cJSON_AddObjectToObject(item_json, DETAIL_KEY);
    cJSON_AddStringToObject(
        detail_json, DETAIL_STEP_KEY, api_apps_queue_get_step_type_string(detail->step));

    if(detail->has_progress) {
        cJSON_AddNumberToObject(detail_json, DETAIL_PROGRESS_KEY, detail->progress);
    }
}

static void api_apps_queue_item_status_format(
    cJSON* item_json,
    JsAppInstallerOperationStatus operation_status,
    const JsAppInstallerOperationDetail* detail) {
    if(operation_status == JsAppInstallerOperationStatusInProgress) {
        cJSON_AddStringToObject(item_json, STATUS_KEY, STATUS_VALUE_IN_PROGRESS);
        api_apps_queue_item_detail_format(item_json, detail);
    } else if(operation_status == JsAppInstallerOperationStatusPending) {
        cJSON_AddStringToObject(item_json, STATUS_KEY, STATUS_VALUE_PENDING);
    } else {
        furi_crash("Invalid JsAppInstallerOperationStatus value");
    }
}

static void api_apps_queue_item_callback(const JsAppInstallerOperationInfo* info, void* context) {
    furi_assert(info);
    furi_assert(context);

    cJSON* items_json = context;
    cJSON* item_json = cJSON_CreateObject();
    cJSON_AddItemToArray(items_json, item_json);

    const JsAppInstallerAppInfo* app_info = &info->app_info;
    cJSON_AddNumberToObject(item_json, ITEM_ID_KEY, info->item_id);
    cJSON_AddStringToObject(item_json, APP_ID_KEY, app_info->app_id);

    api_apps_queue_item_operation_format(item_json, info->type, &app_info->version);
    api_apps_queue_item_status_format(item_json, info->status, &info->detail);
}

static void api_apps_queue_get(struct mg_connection* conn, struct mg_http_message* msg) {
    UNUSED(msg);

    cJSON* response_json = cJSON_CreateObject();
    cJSON* result_json = cJSON_AddArrayToObject(response_json, RESULT_KEY);

    JsAppInstaller* installer = furi_record_open(RECORD_JS_APP_INSTALLER);
    const JsAppInstallerError error =
        js_app_installer_get_operation_queue(installer, api_apps_queue_item_callback, result_json);
    furi_record_close(RECORD_JS_APP_INSTALLER);

    if(error == JsAppInstallerErrorNone) {
        char* response_body = cJSON_PrintUnformatted(response_json);
        MG_REPLY_OK_BODY(conn, "%s", response_body);
        free(response_body);

    } else if(error == JsAppInstallerErrorTimeout) {
        MG_REPLY_TIMEOUT(conn);
    } else {
        MG_REPLY_SERVICE_UNAVAILABLE(conn);
    }

    cJSON_Delete(response_json);
}

static bool api_apps_queue_operation_source_info_parse(
    JsAppInstallerSourceInfo* source,
    const cJSON* json) {
    bool success = false;

    do {
        const cJSON* item;

        item = cJSON_GetObjectItem(json, OPERATION_URL_KEY);
        if(!cJSON_IsString(item)) {
            break;
        }

        const char* url_str = cJSON_GetStringValue(item);
        Url* url = url_alloc();
        const bool is_url_valid = url_parse(url, url_str);
        url_free(url);

        if(!is_url_valid) {
            break;
        }

        source->url = url_str;

        JsAppInstallerAppVersion* version = &source->version;

        if(cJSON_HasObjectItem(json, OPERATION_VERSION_KEY)) {
            item = cJSON_GetObjectItem(json, OPERATION_VERSION_KEY);
            if(!cJSON_IsString(item)) {
                break;
            }

            if(!semver_parse(&version->value, cJSON_GetStringValue(item))) {
                break;
            }

            version->is_present = true;
        } else {
            version->is_present = false;
        }

        success = true;
    } while(false);

    return success;
}

static bool api_apps_queue_operation_info_parse(
    ApiAppsQueueOperationInfo* operation_info,
    const cJSON* json) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        const cJSON* item;

        item = cJSON_GetObjectItem(json, OPERATION_TYPE_KEY);
        if(!cJSON_IsString(item)) {
            break;
        }

        const char* type_str = cJSON_GetStringValue(item);
        if(strcmp(type_str, OPERATION_TYPE_INSTALL_VALUE) == 0) {
            if(!api_apps_queue_operation_source_info_parse(&operation_info->source, json)) {
                break;
            }
            operation_info->operation_type = JsAppInstallerOperationTypeInstall;
        } else if(strcmp(type_str, OPERATION_TYPE_REMOVE_VALUE) == 0) {
            operation_info->operation_type = JsAppInstallerOperationTypeRemove;
        } else {
            break;
        }

        success = true;
    } while(false);

    return success;
}

static bool api_apps_queue_operation_parse(ApiAppsQueueOperation* operation, const cJSON* json) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        const cJSON* item;

        item = cJSON_GetObjectItem(json, APP_ID_KEY);
        if(!cJSON_IsString(item)) {
            break;
        }

        const char* app_id_str = cJSON_GetStringValue(item);
        if(!js_app_is_valid_id(app_id_str)) {
            break;
        }

        operation->app_id = app_id_str;

        item = cJSON_GetObjectItem(json, OPERATION_KEY);
        if(!api_apps_queue_operation_info_parse(&operation->info, item)) {
            break;
        }

        success = true;
    } while(false);

    return success;
}

static bool api_apps_queue_add_request_parse(ApiAppsQueueAddRequest* request, const cJSON* json) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        const cJSON* operations = cJSON_GetObjectItem(json, OPERATIONS_KEY);
        if(!cJSON_IsArray(operations)) {
            break;
        }

        const uint32_t count = cJSON_GetArraySize(operations);
        if(count == 0) {
            break;
        }

        ApiAppsQueueOperation* data = calloc(count, sizeof(ApiAppsQueueOperation));

        uint32_t parsed_count;
        for(parsed_count = 0; parsed_count < count; ++parsed_count) {
            const cJSON* item = cJSON_GetArrayItem(operations, parsed_count);
            if(!api_apps_queue_operation_parse(&data[parsed_count], item)) {
                break;
            }
        }

        if(parsed_count != count) {
            free(data);
            break;
        }

        request->operations = data;
        request->operations_count = count;

        success = true;
    } while(false);

    return success;
}

static char* api_apps_queue_add_response_format(const ApiAppsQueueAddRequest* request) {
    char* response_body = NULL;
    cJSON* response_body_json = cJSON_CreateObject();

    cJSON* result_json = cJSON_AddArrayToObject(response_body_json, RESULT_KEY);

    bool has_errors = false;
    for(size_t i = 0; i < request->operations_count; ++i) {
        const ApiAppsQueueOperation* operation = &request->operations[i];
        cJSON* item_json = cJSON_CreateObject();

        cJSON_AddStringToObject(item_json, APP_ID_KEY, operation->app_id);

        if(!has_errors) {
            const JsAppInstallerOperationResult* result = &operation->result;
            const JsAppInstallerError error = result->error;

            if(error == JsAppInstallerErrorNone) {
                cJSON_AddNumberToObject(item_json, ITEM_ID_KEY, result->operation_id);
            } else {
                cJSON_AddStringToObject(item_json, ERROR_KEY, installer_error_codes[error]);
                has_errors = true;
            }

        } else {
            cJSON_AddStringToObject(item_json, ERROR_KEY, ERROR_SKIPPED_VALUE);
        }

        const ApiAppsQueueOperationInfo* info = &operation->info;
        api_apps_queue_item_operation_format(
            item_json, info->operation_type, &info->source.version);

        cJSON_AddItemToArray(result_json, item_json);
    }

    response_body = cJSON_PrintUnformatted(response_body_json);
    cJSON_Delete(response_body_json);

    return response_body;
}

static void api_apps_queue_post(struct mg_connection* conn, struct mg_http_message* msg) {
    ApiAppsQueueAddRequest request = {0};

    cJSON* json = cJSON_ParseWithLength(msg->body.buf, msg->body.len);

    do {
        if(json == NULL) {
            MG_REPLY_BAD_REQUEST(conn);
            break;
        }

        if(!api_apps_queue_add_request_parse(&request, json)) {
            MG_REPLY_BAD_REQUEST(conn);
            break;
        }

        JsAppInstaller* installer = furi_record_open(RECORD_JS_APP_INSTALLER);

        for(size_t i = 0; i < request.operations_count; ++i) {
            ApiAppsQueueOperation* operation = &request.operations[i];

            const ApiAppsQueueOperationInfo* info = &operation->info;
            const JsAppInstallerOperationType operation_type = info->operation_type;

            if(operation_type == JsAppInstallerOperationTypeInstall) {
                operation->result =
                    js_app_installer_install(installer, operation->app_id, &info->source);
            } else if(operation_type == JsAppInstallerOperationTypeRemove) {
                operation->result = js_app_installer_remove(installer, operation->app_id);
            } else {
                furi_crash("Invalid JsAppInstallerOperationType value");
            }

            if(operation->result.error != JsAppInstallerErrorNone) {
                break;
            }
        }

        furi_record_close(RECORD_JS_APP_INSTALLER);

        char* response_body = api_apps_queue_add_response_format(&request);
        MG_REPLY_OK_ASYNC_BODY(conn, "%s", response_body);
        free(response_body);

    } while(false);

    if(request.operations != NULL) {
        free(request.operations);
    }

    cJSON_Delete(json);
}

static bool api_apps_queue_request_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) {
        return false;
    }

    if(method == HttpMethodGet) {
        api_apps_queue_get(conn, msg);
    } else if(method == HttpMethodPost) {
        api_apps_queue_post(conn, msg);
    }

    return true;
}

static bool api_apps_queue_cancel_request_parse(
    ApiAppsQueueCancelRequest* request,
    const struct mg_str source) {
    bool success = false;

    cJSON* request_json = cJSON_ParseWithLength(source.buf, source.len);

    do {
        if(!cJSON_IsObject(request_json)) {
            break;
        }

        const cJSON* item_id_json = cJSON_GetObjectItem(request_json, ITEM_ID_KEY);
        const cJSON* all_json = cJSON_GetObjectItem(request_json, CANCEL_ALL_KEY);

        if(cJSON_IsNumber(item_id_json) && all_json == NULL) {
            request->is_clear_all = false;
            request->item_id = cJSON_GetNumberValue(item_id_json);
        } else if(cJSON_IsTrue(all_json) && item_id_json == NULL) {
            request->is_clear_all = true;
        } else {
            break;
        }

        success = true;
    } while(false);

    cJSON_Delete(request_json);

    return success;
}

static bool api_apps_queue_cancel_request_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    UNUSED(method);

    if(!IS_HTTP_ENDPOINT(path)) {
        return false;
    }

    ApiAppsQueueCancelRequest request;
    if(!api_apps_queue_cancel_request_parse(&request, msg->body)) {
        MG_REPLY_BAD_REQUEST(conn);
        return true;
    }

    JsAppInstaller* installer = furi_record_open(RECORD_JS_APP_INSTALLER);
    JsAppInstallerError error;

    if(request.is_clear_all) {
        error = js_app_installer_cancel_all(installer);
    } else {
        error = js_app_installer_cancel_operation(installer, request.item_id);
    }

    furi_record_close(RECORD_JS_APP_INSTALLER);

    if(error == JsAppInstallerErrorNone) {
        MG_REPLY_OK_ASYNC(conn);
    } else if(error == JsAppInstallerErrorTimeout) {
        MG_REPLY_TIMEOUT(conn);
    } else if(error == JsAppInstallerErrorNotFound) {
        MG_REPLY_NOT_FOUND(conn);
    } else {
        MG_REPLY_SERVICE_UNAVAILABLE(conn);
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
        .uri = "launch",
        .method = HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_apps_launch_request_callback,
    },
    {
        .uri = "quit",
        .method = HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_apps_quit_request_callback,
    },
    {
        .uri = "list",
        .method = HttpMethodGet,
        .type = HttpHandlerCustom,
        .on_request = api_apps_list_request_callback,
    },
    {
        .uri = "settings",
        .method = HttpMethodGet | HttpMethodPut | HttpMethodDelete,
        .type = HttpHandlerCustom,
        .on_request = api_apps_settings_callback,
    },
    {
        .uri = "queue/cancel",
        .method = HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_apps_queue_cancel_request_callback,
    },
    {
        .uri = "queue",
        .method = HttpMethodGet | HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_apps_queue_request_callback,
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
