#include "http_api.h" // Should contain ConnectionContext and other common defs

#include <furi.h>
#include <toolbox/timers.h>

#include <storage_utils/temp_file.h>

#include <applications/system/updater/updater.h>
#include <applications/system/updater/updater_paths.h>
#include <applications/system/updater/settings/settings.h>
#include <cjson/cJSON.h>

#define TAG "HttpApiUpdate"

#define MAX_UPLOAD_FILE_SIZE (100 * 1024 * 1024)
#define MAX_VERSION_LENGTH   64

#define UPDATE_JSON_KEY_INSTALL           "install"
#define UPDATE_JSON_KEY_CHECK             "check"
#define UPDATE_JSON_KEY_IS_ALLOWED        "is_allowed"
#define UPDATE_JSON_KEY_EVENT             "event"
#define UPDATE_JSON_KEY_ACTION            "action"
#define UPDATE_JSON_KEY_STATUS            "status"
#define UPDATE_JSON_KEY_DETAIL            "detail"
#define UPDATE_JSON_KEY_DOWNLOAD          "download"
#define UPDATE_JSON_KEY_SPEED_BPS         "speed_bytes_per_sec"
#define UPDATE_JSON_KEY_RECEIVED_BYTES    "received_bytes"
#define UPDATE_JSON_KEY_TOTAL_BYTES       "total_bytes"
#define UPDATE_JSON_KEY_AVAILABLE_VERSION "available_version"

#define UPDATE_JSON_KEY_AUTOUPDATE_ENABLED "is_enabled"
#define UPDATE_JSON_KEY_AUTOUPDATE_START   "interval_start"
#define UPDATE_JSON_KEY_AUTOUPDATE_END     "interval_end"

#define UPDATE_UPLOAD_IDLE_TIMEOUT_MS 5000

// Context for the update handler (raw upload)
typedef struct {
    Storage* storage;
    Updater* updater;
    TempFile* update_file;

    FuriThreadPriority original_thread_priority;

    size_t total_file_size; // Expected total size from Content-Length
    size_t received_file_size; // Bytes received so far

    bool file_fully_received; // Flag: true if all bytes received and temp file closed

    CoarseTimer timeout_timer;
} HttpUpdateHandlerCtx;

static const char* const update_status_strings[] = {
    [UpdaterStatusOk] = "ok",
    [UpdaterStatusBatteryLow] = "battery_low",
    [UpdaterStatusBusy] = "busy",
    [UpdaterStatusDownloadFailure] = "download_failure",
    [UpdaterStatusDownloadAbort] = "download_abort",
    [UpdaterStatusShaMismatch] = "sha_mismatch",
    [UpdaterStatusUnpackCreateStagingDirectoryFailure] = "unpack_staging_dir_failure",
    [UpdaterStatusUnpackArchiveOpenFailure] = "unpack_archive_open_failure",
    [UpdaterStatusUnpackArchiveUnpackFailure] = "unpack_archive_unpack_failure",
    [UpdaterStatusInstallationPrepareManifestNotFound] = "install_manifest_not_found",
    [UpdaterStatusInstallationPrepareManifestInvalid] = "install_manifest_invalid",
    [UpdaterStatusInstallationPrepareSessionConfigSetupFailure] = "install_session_config_failure",
    [UpdaterStatusInstallationPreparePointerSetupFailure] = "install_pointer_setup_failure",
    [UpdaterStatusUnknownFailure] = "unknown_failure",
};

static_assert(COUNT_OF(update_status_strings) == UpdaterStatusesCount);

static const char* const update_action_strings[] = {
    [UpdaterUpdateActionDownload] = "download",
    [UpdaterUpdateActionShaVerification] = "sha_verification",
    [UpdaterUpdateActionUnpack] = "unpack",
    [UpdaterUpdateActionInstallationPrepare] = "prepare",
    [UpdaterUpdateActionInstallationApply] = "apply",
    [UpdaterUpdateActionNone] = "none",
};

static_assert(COUNT_OF(update_action_strings) == UpdaterUpdateActionsCount);

static const char* const update_event_strings[] = {
    [UpdaterUpdateEventSessionStart] = "session_start",
    [UpdaterUpdateEventSessionStop] = "session_stop",
    [UpdaterUpdateEventActionBegin] = "action_begin",
    [UpdaterUpdateEventActionDone] = "action_done",
    [UpdaterUpdateEventDetailChange] = "detail_change",
    [UpdaterUpdateEventActionProgress] = "action_progress",
    [UpdaterUpdateEventNone] = "none",
};

static_assert(COUNT_OF(update_event_strings) == UpdaterUpdateEventsCount);

static const char* const check_result_strings[] = {
    [UpdaterCheckResultAvailable] = "available",
    [UpdaterCheckResultNotAvailable] = "not_available",
    [UpdaterCheckResultFailure] = "failure",
    [UpdaterCheckResultNone] = "none",
};

static_assert(COUNT_OF(check_result_strings) == UpdaterCheckResultsCount);

static const char* const check_event_strings[] = {
    [UpdaterCheckEventStart] = "start",
    [UpdaterCheckEventStop] = "stop",
    [UpdaterCheckEventNone] = "none",
};

static_assert(COUNT_OF(check_event_strings) == UpdaterCheckEventsCount);

// Forward declarations
static bool
    handle_completed_upload_and_reboot(HttpUpdateHandlerCtx* ctx, struct mg_connection* conn);
static void api_update_on_data_cb(struct mg_connection* conn, struct mg_iobuf* io);
static void api_update_on_close_cb(struct mg_connection* conn);

static HttpUpdateHandlerCtx* alloc_raw_update_context() {
    HttpUpdateHandlerCtx* ctx = malloc(sizeof(HttpUpdateHandlerCtx));
    ctx->storage = furi_record_open(RECORD_STORAGE);
    ctx->updater = furi_record_open(RECORD_UPDATER);
    ctx->update_file = temp_file_alloc(ctx->storage);

    ctx->original_thread_priority = furi_thread_get_current_priority();

    ctx->total_file_size = 0;
    ctx->received_file_size = 0;
    ctx->file_fully_received = false;
    return ctx;
}

static void free_raw_update_context(HttpUpdateHandlerCtx* ctx) {
    if(!ctx) return;

    furi_thread_set_current_priority(ctx->original_thread_priority);

    if(ctx->update_file) {
        temp_file_remove(ctx->update_file);
        temp_file_free(ctx->update_file);
        ctx->update_file = NULL;
    }

    if(ctx->updater) {
        furi_record_close(RECORD_UPDATER);
        ctx->updater = NULL;
    }

    if(ctx->storage) {
        furi_record_close(RECORD_STORAGE);
        ctx->storage = NULL;
    }

    free(ctx);
}

static bool
    handle_completed_upload_and_reboot(HttpUpdateHandlerCtx* ctx, struct mg_connection* conn) {
    FURI_LOG_I(TAG, "File upload complete. Processing update package.");

    bool is_success = false;
    bool update_started = false;

    do {
        /* Start update session */
        UpdaterStatus session_start_status = updater_session_start(ctx->updater);
        if(session_start_status != UpdaterStatusOk) {
            FuriString* error_string = furi_string_alloc_printf(
                "Update not allowed: %s", updater_get_status_string(session_start_status));

            FURI_LOG_E(TAG, furi_string_get_cstr(error_string));
            MG_REPLY_ERROR_CLOSE(conn, 400, furi_string_get_cstr(error_string));

            furi_string_free(error_string);
            break;
        }

        update_started = true;

        UpdaterStatus unpack_tar_status = updater_unpack(ctx->updater, NULL, NULL, NULL, true);
        if(unpack_tar_status != UpdaterStatusOk) {
            FuriString* error_string = furi_string_alloc_printf(
                "Update bundle unpack failed: %s", updater_get_status_string(unpack_tar_status));

            FURI_LOG_E(TAG, furi_string_get_cstr(error_string));
            MG_REPLY_ERROR_CLOSE(conn, 400, furi_string_get_cstr(error_string));

            furi_string_free(error_string);
            break;
        }

        UpdaterStatus installation_prepare_status =
            updater_installation_prepare(ctx->updater, NULL, true);
        if(installation_prepare_status != UpdaterStatusOk) {
            FuriString* error_string = furi_string_alloc_printf(
                "Update installation preparation failed: %s",
                updater_get_status_string(installation_prepare_status));

            FURI_LOG_E(TAG, furi_string_get_cstr(error_string));
            MG_REPLY_ERROR_CLOSE(conn, 400, furi_string_get_cstr(error_string));

            furi_string_free(error_string);
            break;
        }

        FURI_LOG_I(TAG, "Device will reboot...");

        mg_http_reply(
            conn,
            200,
            DEFAULT_JSON_HEADERS "Connection: close\r\n",
            "{\"result\":\"OK\",\"message\":\"Update accepted. System will reboot.\"}\n");

        conn->is_draining = 1;

        updater_installation_apply(ctx->updater, false);

        is_success = true;
    } while(false);

    if(update_started && !is_success) {
        updater_session_stop(ctx->updater);
    }

    return is_success;
}

static void api_update_on_data_cb(struct mg_connection* conn, struct mg_iobuf* io) {
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    HttpUpdateHandlerCtx* update_ctx = (HttpUpdateHandlerCtx*)conn_ctx->context;

    if(!update_ctx || !update_ctx->update_file) {
        FURI_LOG_E(TAG, "on_data: Context or file saver invalid/closed. Draining.");
        MG_REPLY_ERROR_CLOSE(conn, 409, "Update context invalid");
        mg_iobuf_del(io, 0, io->len); // Consume data to prevent further calls
        conn->is_draining = 1; // Mark connection to be closed
        return;
    }

    update_ctx->timeout_timer = coarse_timer_create(UPDATE_UPLOAD_IDLE_TIMEOUT_MS);

    size_t data_len = io->len;
    FURI_LOG_T(
        TAG,
        "on_data: Received %zu bytes. Total received: %zu / %zu",
        data_len,
        update_ctx->received_file_size,
        update_ctx->total_file_size);

    if(data_len > 0) {
        if(update_ctx->received_file_size + data_len > update_ctx->total_file_size) {
            FURI_LOG_E(
                TAG,
                "on_data: Received more data than expected. Expected %zu, got %zu more.",
                update_ctx->total_file_size,
                (update_ctx->received_file_size + data_len) - update_ctx->total_file_size);
            MG_REPLY_ERROR_CLOSE(conn, 413, "Payload Too Large");
            conn->is_draining = 1;
            mg_iobuf_del(io, 0, io->len);
            return;
        }

        if(!temp_file_write(update_ctx->update_file, io->buf, data_len)) {
            FURI_LOG_E(
                TAG, "on_data: Failed to write data to temp TAR file. Wrote %zu bytes.", data_len);
            MG_REPLY_ERROR_CLOSE(conn, 508, "Failed to save update package (write error).");
            conn->is_draining = 1;
            mg_iobuf_del(io, 0, io->len);
            return;
        }

        update_ctx->received_file_size += data_len;
    }

    mg_iobuf_del(io, 0, io->len); // Consume all data from buffer

    if(update_ctx->received_file_size >= update_ctx->total_file_size) {
        FURI_LOG_I(TAG, "on_data: All data received (%zu bytes)", update_ctx->received_file_size);
        update_ctx->file_fully_received = true;

        temp_file_free(update_ctx->update_file);
        update_ctx->update_file = NULL;

        if(!handle_completed_upload_and_reboot(update_ctx, conn)) {
            // Error response already sent by handle_completed_upload_and_reboot
            FURI_LOG_E(TAG, "on_data: package handling failed.");
            conn->is_draining = 1;
        }
    }
}

static void api_update_on_close_cb(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    HttpUpdateHandlerCtx* update_ctx = (HttpUpdateHandlerCtx*)conn_ctx->context;

    FURI_LOG_D(TAG, "on_close");

    if(update_ctx) {
        free_raw_update_context(update_ctx);
        conn_ctx->context = NULL;
    }

    // Clear callbacks
    conn_ctx->raw.on_data = NULL;
    conn_ctx->raw.on_poll = NULL;
    conn_ctx->on_close = NULL;
}

static void api_update_on_poll_cb(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (void*)conn->data;
    HttpUpdateHandlerCtx* update_ctx = conn_ctx->context;
    furi_assert(update_ctx);

    if(coarse_timer_is_expired(update_ctx->timeout_timer)) {
        FURI_LOG_E(TAG, "Connection data timeout (%lu)", conn->id);
        MG_REPLY_TIMEOUT(conn, "Upload timeout");
        conn->is_draining = 1; // Force close hanging connection
    }
}

static bool api_update_raw_hdr_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* http_handler_ctx) {
    UNUSED(http_handler_ctx);
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    HttpUpdateHandlerCtx* update_ctx = NULL;

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FURI_LOG_I(
        TAG, "on_headers: Received update request for URI: %.*s", (int)msg->uri.len, msg->uri.buf);

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

    update_ctx = alloc_raw_update_context();
    conn_ctx->raw.on_data = api_update_on_data_cb;
    conn_ctx->raw.on_poll = api_update_on_poll_cb;
    conn_ctx->on_close = api_update_on_close_cb;
    conn_ctx->context = update_ctx;

    update_ctx->timeout_timer = coarse_timer_create(UPDATE_UPLOAD_IDLE_TIMEOUT_MS);
    update_ctx->total_file_size = msg->body.len;
    if(update_ctx->total_file_size > MAX_UPLOAD_FILE_SIZE) {
        FURI_LOG_E(
            TAG,
            "on_headers: File size %zu exceeds max %u.",
            update_ctx->total_file_size,
            MAX_UPLOAD_FILE_SIZE);
        MG_REPLY_ERROR_CLOSE(conn, 413, "Payload Too Large");
        conn->is_draining = 1;
        return true;
    }
    FURI_LOG_I(TAG, "on_headers: Expecting file of size: %zu bytes", update_ctx->total_file_size);

    furi_thread_set_current_priority(FuriThreadPriorityLow);

    if(!temp_file_create(update_ctx->update_file, UPDATER_DEFAULT_DOWNLOAD_PATH)) {
        FURI_LOG_E(
            TAG,
            "on_headers: Failed to initialize file saver for: %s",
            UPDATER_DEFAULT_DOWNLOAD_PATH);
        MG_REPLY_ERROR_CLOSE(conn, 508, "Failed to save update package (file init error).");
        conn->is_draining = 1;
        return true;
    }

    FURI_LOG_I(TAG, "on_headers: Initialized file saver for: %s", UPDATER_DEFAULT_DOWNLOAD_PATH);

    mg_iobuf_del(&conn->recv, 0, msg->head.len); // Delete HTTP headers
    conn->pfn = NULL; // Silence HTTP protocol handler, we'll use MG_EV_READ

    // Also handle possible data in the buffer
    api_update_on_data_cb(conn, &conn->recv);

    return true;
}

static bool api_update_raw_request_callback(
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

static bool api_update_check_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FURI_LOG_I(TAG, "Received update check request");

    Updater* updater = furi_record_open(RECORD_UPDATER);
    UpdaterStatus status = updater_check_for_update(updater);
    furi_record_close(RECORD_UPDATER);

    int error_code;
    bool is_success = false;
    switch(status) {
    case UpdaterStatusOk:
        is_success = true;
        break;

    case UpdaterStatusBusy:
        error_code = 409;
        break;

    default:
        error_code = 503;
        break;
    }

    if(is_success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_ERROR(conn, error_code, updater_get_status_string(status));
    }

    return true;
}

static bool api_update_changelog_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    const char* error_text;
    bool is_error = true;
    do {
        char version[MAX_VERSION_LENGTH];
        int version_length = mg_http_get_var(&msg->query, "version", version, sizeof(version));
        if(version_length <= 0) {
            error_text = "Version parameter missing";
            break;
        }

        FURI_LOG_I(TAG, "Received update changelog request for version: %s", version);

        Updater* updater = furi_record_open(RECORD_UPDATER);

        UpdaterCheckState check_state;
        furi_state_get(updater_get_check_state(updater), &check_state);

        FuriString* check_changelog = furi_string_alloc();
        FuriString* check_version = furi_string_alloc();
        do {
            if(check_state.result != UpdaterCheckResultAvailable) {
                error_text = "Changelog not available";
                break;
            }

            updater_get_check_info(
                updater,
                &(UpdateCheckInfo){
                    .version = check_version,
                    .url = NULL,
                    .id = NULL,
                    .sha256 = NULL,
                    .changelog = check_changelog,
                });

            if(strcmp(furi_string_get_cstr(check_version), version) != 0) {
                error_text = "Version mismatch";
                break;
            }

            cJSON* response = cJSON_CreateObject();
            cJSON_AddStringToObject(response, "changelog", furi_string_get_cstr(check_changelog));
            char* json_str = cJSON_Print(response);
            MG_REPLY_OK_BODY(conn, "%s\n", json_str);
            cJSON_free(json_str);
            cJSON_Delete(response);

            is_error = false;
        } while(false);

        furi_string_free(check_version);
        furi_string_free(check_changelog);
        furi_record_close(RECORD_UPDATER);
    } while(false);

    if(is_error) {
        MG_REPLY_ERROR(conn, 400, error_text);
    }

    return true;
}

static bool api_update_install_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    int error_code;
    const char *error_text, *error_code_text;
    bool is_success = false;
    do {
        char version[MAX_VERSION_LENGTH];
        int version_length = mg_http_get_var(&msg->query, "version", version, sizeof(version));
        if(version_length <= 0) {
            error_code = 400;
            error_text = "Version parameter missing";
            error_code_text = "version_missing";
            break;
        }

        FURI_LOG_I(TAG, "Received update install request for version: %s", version);

        Updater* updater = furi_record_open(RECORD_UPDATER);
        FuriState* update_check_state = updater_get_check_state(updater);
        UpdaterCheckState check_state;
        furi_state_get(update_check_state, &check_state);

        FuriString* check_version = furi_string_alloc();
        FuriString* check_url = furi_string_alloc();
        FuriString* check_sha256 = furi_string_alloc();
        do {
            if(check_state.result != UpdaterCheckResultAvailable) {
                error_code = 400;
                error_text = "Update not available";
                error_code_text = "not_available";
                break;
            }

            updater_get_check_info(
                updater,
                &(UpdateCheckInfo){
                    .version = check_version,
                    .url = check_url,
                    .id = NULL,
                    .sha256 = check_sha256,
                    .changelog = NULL,
                });

            if(strcmp(furi_string_get_cstr(check_version), version) != 0) {
                error_code = 400;
                error_text = "Version mismatch";
                error_code_text = "version_mismatch";
                break;
            }

            UpdaterStatus session_status = updater_session_start(updater);
            if(session_status != UpdaterStatusOk) {
                switch(session_status) {
                case UpdaterStatusBatteryLow:
                    error_code = 503;
                    break;

                case UpdaterStatusBusy:
                    error_code = 409;
                    break;

                default:
                    error_code = 503;
                    break;
                }

                error_text = updater_get_status_string(session_status);
                error_code_text = update_status_strings[session_status];
                break;
            }

            updater_install_from_url(
                updater, furi_string_get_cstr(check_url), furi_string_get_cstr(check_sha256));

            is_success = true;
        } while(false);

        furi_string_free(check_version);
        furi_string_free(check_url);
        furi_string_free(check_sha256);
        furi_record_close(RECORD_UPDATER);
    } while(false);

    if(is_success) {
        MG_REPLY_OK(conn);
    } else {
        _MG_JSON_RESULT(
            conn,
            error_code,
            "{\"error\":\"%M\", \"error_code\":\"%s\"}\n",
            MG_ESC(error_text),
            error_code_text);
    }

    return true;
}

static bool api_update_status_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FURI_LOG_I(TAG, "Received update status request");

    Updater* updater = furi_record_open(RECORD_UPDATER);

    UpdaterStatus allowance_status = updater_get_allowance_status(updater);
    bool is_allowed = (allowance_status == UpdaterStatusOk);

    UpdaterUpdateState update_state;
    furi_state_get(updater_get_update_state(updater), &update_state);

    UpdaterCheckState check_state;
    furi_state_get(updater_get_check_state(updater), &check_state);

    FuriString* check_version = furi_string_alloc();
    updater_get_check_info(
        updater,
        &(UpdateCheckInfo){
            .version = check_version,
            .url = NULL,
            .id = NULL,
            .sha256 = NULL,
            .changelog = NULL,
        });

    cJSON* response = cJSON_CreateObject();

    cJSON* install = cJSON_AddObjectToObject(response, UPDATE_JSON_KEY_INSTALL);
    cJSON_AddBoolToObject(install, UPDATE_JSON_KEY_IS_ALLOWED, is_allowed);

    cJSON_AddStringToObject(
        install,
        UPDATE_JSON_KEY_EVENT,
        (update_state.event < COUNT_OF(update_event_strings)) ?
            update_event_strings[update_state.event] :
            "unknown");

    cJSON_AddStringToObject(
        install,
        UPDATE_JSON_KEY_ACTION,
        (update_state.action < COUNT_OF(update_action_strings)) ?
            update_action_strings[update_state.action] :
            "unknown");

    cJSON_AddStringToObject(
        install,
        UPDATE_JSON_KEY_STATUS,
        (update_state.status < COUNT_OF(update_status_strings)) ?
            update_status_strings[update_state.status] :
            "unknown");

    cJSON_AddStringToObject(install, UPDATE_JSON_KEY_DETAIL, update_state.detail);

    cJSON* download = cJSON_AddObjectToObject(install, UPDATE_JSON_KEY_DOWNLOAD);
    cJSON_AddNumberToObject(
        download, UPDATE_JSON_KEY_SPEED_BPS, update_state.as_download.speed_bytes_per_sec);
    cJSON_AddNumberToObject(
        download, UPDATE_JSON_KEY_RECEIVED_BYTES, update_state.as_download.received_size);
    cJSON_AddNumberToObject(
        download, UPDATE_JSON_KEY_TOTAL_BYTES, update_state.as_download.total_size);

    cJSON* check = cJSON_AddObjectToObject(response, UPDATE_JSON_KEY_CHECK);

    cJSON_AddStringToObject(
        check, UPDATE_JSON_KEY_AVAILABLE_VERSION, furi_string_get_cstr(check_version));

    cJSON_AddStringToObject(
        check,
        UPDATE_JSON_KEY_EVENT,
        (check_state.event < COUNT_OF(check_event_strings)) ?
            check_event_strings[check_state.event] :
            "unknown");

    cJSON_AddStringToObject(
        check,
        UPDATE_JSON_KEY_STATUS,
        (check_state.result < COUNT_OF(check_result_strings)) ?
            check_result_strings[check_state.result] :
            "unknown");

    furi_record_close(RECORD_UPDATER);

    char* json_str = cJSON_Print(response);
    MG_REPLY_OK_BODY(conn, "%s\n", json_str);
    cJSON_free(json_str);
    cJSON_Delete(response);

    furi_string_free(check_version);

    return true;
}

static bool api_update_abort_download_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FURI_LOG_I(TAG, "Received download abort request");

    Updater* updater = furi_record_open(RECORD_UPDATER);
    updater_abort_download(updater);
    furi_record_close(RECORD_UPDATER);

    MG_REPLY_OK(conn);

    return true;
}

static void format_time_minutes(uint32_t minutes_total, FuriString* buffer) {
    int hours = minutes_total / 60;
    int minutes = minutes_total % 60;

    furi_string_printf(buffer, "%02d:%02d", hours, minutes);
}

static bool parse_time_minutes(const char* time_string, uint32_t* minutes_total) {
    int hours, minutes;

    if(sscanf(time_string, "%d:%d", &hours, &minutes) != 2) return false;
    if(hours < 0 || hours > 23 || minutes < 0 || minutes > 59) return false;

    *minutes_total = hours * 60 + minutes;
    return true;
}

static void api_update_autoupdate_get(struct mg_connection* conn, struct mg_http_message* msg) {
    UNUSED(msg);

    FURI_LOG_I(TAG, "Received autoupdate settings get request");

    Updater* updater = furi_record_open(RECORD_UPDATER);

    UpdaterSettings settings;
    updater_get_settings(updater, &settings);

    cJSON* response = cJSON_CreateObject();
    cJSON_AddBoolToObject(
        response, UPDATE_JSON_KEY_AUTOUPDATE_ENABLED, settings.autoupdate_enabled);

    FuriString* time_buffer = furi_string_alloc();
    format_time_minutes(settings.autoupdate_interval_start, time_buffer);
    cJSON_AddStringToObject(
        response, UPDATE_JSON_KEY_AUTOUPDATE_START, furi_string_get_cstr(time_buffer));

    format_time_minutes(settings.autoupdate_interval_end, time_buffer);
    cJSON_AddStringToObject(
        response, UPDATE_JSON_KEY_AUTOUPDATE_END, furi_string_get_cstr(time_buffer));

    char* json_string = cJSON_Print(response);
    MG_REPLY_OK_BODY(conn, "%s\n", json_string);

    cJSON_free(json_string);
    cJSON_Delete(response);

    furi_string_free(time_buffer);
    furi_record_close(RECORD_UPDATER);
}

static void api_update_autoupdate_set(struct mg_connection* conn, struct mg_http_message* msg) {
    FURI_LOG_I(TAG, "Received autoupdate settings set request");

    Updater* updater = furi_record_open(RECORD_UPDATER);
    UpdaterSettings settings;
    updater_get_settings(updater, &settings);

    bool is_success;
    do {
        bool value;
        if(mg_json_get_bool(msg->body, "$." UPDATE_JSON_KEY_AUTOUPDATE_ENABLED, &value)) {
            settings.autoupdate_enabled = value;
        }

        char* time_string;
        uint32_t minutes_total;
        time_string = mg_json_get_str(msg->body, "$." UPDATE_JSON_KEY_AUTOUPDATE_START);
        if(time_string) {
            is_success = parse_time_minutes(time_string, &minutes_total);
            free(time_string);

            if(is_success) {
                settings.autoupdate_interval_start = minutes_total;
            } else {
                MG_REPLY_BAD_REQUEST(conn);
                break;
            }
        }

        time_string = mg_json_get_str(msg->body, "$." UPDATE_JSON_KEY_AUTOUPDATE_END);
        if(time_string) {
            is_success = parse_time_minutes(time_string, &minutes_total);
            free(time_string);

            if(is_success) {
                settings.autoupdate_interval_end = minutes_total;
            } else {
                MG_REPLY_BAD_REQUEST(conn);
                break;
            }
        }

        is_success = updater_set_settings(updater, &settings);
        if(is_success) {
            MG_REPLY_OK(conn);
        } else {
            MG_REPLY_SERVICE_UNAVAILABLE(conn, "Failed to apply updater settings");
        }
    } while(false);

    furi_record_close(RECORD_UPDATER);
}

static bool api_update_autoupdate_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    if(method == HttpMethodGet) {
        api_update_autoupdate_get(conn, msg);
    } else if(method == HttpMethodPost) {
        api_update_autoupdate_set(conn, msg);
    }

    return true;
}

static const HttpHandler api_update_handlers[] = {
    {
        .uri = "check",
        .method = HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_update_check_callback,
    },
    {
        .uri = "status",
        .method = HttpMethodGet,
        .type = HttpHandlerCustom,
        .on_request = api_update_status_callback,
    },
    {
        .uri = "changelog",
        .method = HttpMethodGet,
        .type = HttpHandlerCustom,
        .on_request = api_update_changelog_callback,
    },
    {
        .uri = "install",
        .method = HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_update_install_callback,
    },
    {
        .uri = "abort_download",
        .method = HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_update_abort_download_callback,
    },
    {
        .uri = "autoupdate",
        .method = HttpMethodGet | HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_update_autoupdate_callback,
    },
    {
        .uri = "",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .on_request = api_update_raw_request_callback,
        .on_headers = api_update_raw_hdr_callback,
    },
};

typedef struct {
    HttpHandlersList_t handlers;
} ApiUpdateCtx;

void* http_api_update_alloc(void) {
    ApiUpdateCtx* context = malloc(sizeof(*context));
    HttpHandlersList_init(context->handlers);

    for(size_t i = COUNT_OF(api_update_handlers); i > 0; i--) {
        http_handler_add(context->handlers, &api_update_handlers[i - 1]);
    }

    return context;
}

void http_api_update_free(void* ctx) {
    furi_assert(ctx);

    ApiUpdateCtx* context = ctx;

    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_update_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiUpdateCtx* context = ctx;

    return http_handle_request(path, method, context->handlers, conn, msg);
}

bool http_api_update_hdr_callback_root(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiUpdateCtx* context = ctx;

    return http_handle_headers(path, method, context->handlers, conn, msg);
}
