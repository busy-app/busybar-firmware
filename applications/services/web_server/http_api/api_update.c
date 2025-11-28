#include "http_api.h" // Should contain ConnectionContext and other common defs

#include <furi.h>
#include <furi_hal_power.h>
#include <toolbox/path.h>

#include <storage/storage.h>
#include <toolbox/fetch/fetch_file_save.h>
#include <applications/system/updater/updater.h>

#define TAG "HttpApiUpdate"

#define MAX_UPLOAD_FILE_SIZE (100 * 1024 * 1024) /* user-set: 100MB */

// Context for the update handler (raw upload)
typedef struct {
    Storage* storage;
    Updater* updater;
    FetchFileSave* file_save;

    size_t total_file_size; // Expected total size from Content-Length
    size_t received_file_size; // Bytes received so far

    bool file_fully_received; // Flag: true if all bytes received and temp file closed
} HttpUpdateHandlerCtx;

// Forward declarations
static bool
    handle_completed_upload_and_reboot(HttpUpdateHandlerCtx* ctx, struct mg_connection* conn);
static void http_api_update_on_data_cb(struct mg_connection* conn, struct mg_iobuf* io);
static void http_api_update_on_close_cb(struct mg_connection* conn);

static HttpUpdateHandlerCtx* alloc_raw_update_context() {
    HttpUpdateHandlerCtx* ctx = malloc(sizeof(HttpUpdateHandlerCtx));
    ctx->storage = furi_record_open(RECORD_STORAGE);
    ctx->updater = furi_record_open(RECORD_UPDATER);
    ctx->file_save = NULL; // Will be allocated in header callback after validation

    ctx->total_file_size = 0;
    ctx->received_file_size = 0;
    ctx->file_fully_received = false;
    return ctx;
}

static void free_raw_update_context(HttpUpdateHandlerCtx* ctx) {
    if(!ctx) return;

    if(ctx->file_save) {
        fetch_file_save_remove(ctx->file_save);
        fetch_file_save_free(ctx->file_save);
        ctx->file_save = NULL;
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
        UpdaterStatus start_status = updater_start_update(ctx->updater);
        if(start_status != UpdaterStatusOk) {
            FuriString* error_string = furi_string_alloc_printf(
                "Update not allowed: %s", updater_get_status_string(start_status));

            FURI_LOG_E(TAG, furi_string_get_cstr(error_string));
            MG_REPLY_ERROR(conn, 400, furi_string_get_cstr(error_string));

            furi_string_free(error_string);
            break;
        }

        update_started = true;

        UpdaterStatus unpack_tar_status = updater_unpack(ctx->updater, NULL, NULL, NULL, true);
        if(unpack_tar_status != UpdaterStatusOk) {
            FuriString* error_string = furi_string_alloc_printf(
                "Update bundle unpack failed: %s", updater_get_status_string(unpack_tar_status));

            FURI_LOG_E(TAG, furi_string_get_cstr(error_string));
            MG_REPLY_ERROR(conn, 400, furi_string_get_cstr(error_string));

            furi_string_free(error_string);
            break;
        }

        UpdaterStatus prepare_install_status = updater_prepare_install(ctx->updater, NULL, true);
        if(prepare_install_status != UpdaterStatusOk) {
            FuriString* error_string = furi_string_alloc_printf(
                "Update prepare install failed: %s",
                updater_get_status_string(prepare_install_status));

            FURI_LOG_E(TAG, furi_string_get_cstr(error_string));
            MG_REPLY_ERROR(conn, 400, furi_string_get_cstr(error_string));

            furi_string_free(error_string);
            break;
        }

        FURI_LOG_I(TAG, "Device will reboot...");

        MG_REPLY_OK_BODY(
            conn, "{\"result\":\"OK\",\"message\":\"Update accepted. System will reboot.\"}\n");

        conn->is_draining = 1;

        updater_reboot_install(ctx->updater, false);

        is_success = true;
    } while(false);

    if(update_started && !is_success) {
        updater_stop_update(ctx->updater);
    }

    return is_success;
}

static void http_api_update_on_data_cb(struct mg_connection* conn, struct mg_iobuf* io) {
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    HttpUpdateHandlerCtx* update_ctx = (HttpUpdateHandlerCtx*)conn_ctx->context;

    if(!update_ctx || !update_ctx->file_save) {
        FURI_LOG_E(TAG, "on_data: Context or file saver invalid/closed. Draining.");
        mg_iobuf_del(io, 0, io->len); // Consume data to prevent further calls
        conn->is_draining = 1; // Mark connection to be closed
        return;
    }

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
            MG_REPLY_PAYLOAD_TOO_LARGE(conn);
            conn->is_draining = 1;
            mg_iobuf_del(io, 0, io->len);
            return;
        }

        if(!fetch_file_save_write(update_ctx->file_save, io->buf, data_len)) {
            FURI_LOG_E(
                TAG, "on_data: Failed to write data to temp TAR file. Wrote %zu bytes.", data_len);
            MG_REPLY_INTERNAL_ERROR(conn, "Failed to save update package (write error).");
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

        fetch_file_save_free(update_ctx->file_save);
        update_ctx->file_save = NULL;

        if(!handle_completed_upload_and_reboot(update_ctx, conn)) {
            // Error response already sent by handle_completed_upload_and_reboot
            FURI_LOG_E(TAG, "on_data: package handling failed.");
            conn->is_draining = 1;
        }
    }
}

static void http_api_update_on_close_cb(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    HttpUpdateHandlerCtx* update_ctx = (HttpUpdateHandlerCtx*)conn_ctx->context;

    FURI_LOG_D(TAG, "on_close");

    if(update_ctx) {
        free_raw_update_context(update_ctx);
        conn_ctx->context = NULL;
    }

    // Clear callbacks
    conn_ctx->raw.on_data = NULL;
    conn_ctx->on_close = NULL;
}

bool http_api_update_hdr_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* http_handler_ctx) {
    UNUSED(http_handler_ctx);
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    HttpUpdateHandlerCtx* update_ctx = NULL;

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FURI_LOG_I(
        TAG, "on_headers: Received update request for URI: %.*s", (int)msg->uri.len, msg->uri.buf);

    if(!mg_match(msg->method, mg_str("POST"), NULL)) {
        MG_REPLY_METHOD_NOT_ALLOWED(conn);
        conn->is_draining = 1;
        return true;
    }

    update_ctx = alloc_raw_update_context();
    conn_ctx->context = update_ctx;

    update_ctx->total_file_size = msg->body.len;
    if(update_ctx->total_file_size == 0) {
        FURI_LOG_W(TAG, "on_headers: Content-Length is 0 or missing/invalid. No file to upload?");
        MG_REPLY_BAD_REQUEST(conn);
        conn->is_draining = 1;
        return true;
    }
    if(update_ctx->total_file_size > MAX_UPLOAD_FILE_SIZE) {
        FURI_LOG_E(
            TAG,
            "on_headers: File size %zu exceeds max %u.",
            update_ctx->total_file_size,
            MAX_UPLOAD_FILE_SIZE);
        MG_REPLY_PAYLOAD_TOO_LARGE(conn);
        conn->is_draining = 1;
        return true;
    }
    FURI_LOG_I(TAG, "on_headers: Expecting file of size: %zu bytes", update_ctx->total_file_size);

    // Allocate file saver (creates directory, removes existing file, opens for writing)
    FuriString* temp_path = furi_string_alloc_set(UPDATER_DEFAULT_DOWNLOAD_PATH);
    update_ctx->file_save = fetch_file_save_alloc(temp_path);
    furi_string_free(temp_path);

    if(!update_ctx->file_save) {
        FURI_LOG_E(
            TAG,
            "on_headers: Failed to initialize file saver for: %s",
            UPDATER_DEFAULT_DOWNLOAD_PATH);
        MG_REPLY_INTERNAL_ERROR(conn, "Failed to save update package (file init error).");
        conn->is_draining = 1;
        return true;
    }

    FURI_LOG_I(TAG, "on_headers: Initialized file saver for: %s", UPDATER_DEFAULT_DOWNLOAD_PATH);

    // Set up raw data handlers
    conn_ctx->raw.on_data = http_api_update_on_data_cb;
    conn_ctx->on_close = http_api_update_on_close_cb;

    mg_iobuf_del(&conn->recv, 0, msg->head.len); // Delete HTTP headers
    conn->pfn = NULL; // Silence HTTP protocol handler, we'll use MG_EV_READ

    // Also handle possible data in the buffer
    http_api_update_on_data_cb(conn, &conn->recv);

    return true;
}
