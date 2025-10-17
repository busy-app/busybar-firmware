#include "http_api.h" // Should contain ConnectionContext and other common defs

#include <furi.h>
#include <furi_hal_power.h>
#include <toolbox/path.h>

#include <storage/storage.h>
#include <applications/system/updater/update_tar.h>

#define TAG "HttpApiUpdate"

#define DEFAULT_UPDATE_PACKAGE_NAME "firmware" // Define a default package name
#define MAX_UPDATE_NAME_LEN         (32)
#define MAX_UPLOAD_FILE_SIZE        (40 * 1024 * 1024) // User-set: 40MB
#define UPDATE_STAGING_ROOT         EXT_PATH("update")

// Context for the update handler (raw upload)
typedef struct {
    Storage* storage;
    FuriString* package_name_param; // Parsed from query string
    FuriString* temp_tar_path; // Path to the temporary TAR file being saved
    FuriString* final_staging_path; // Path to /ext/update/<package_name>
    File* temp_tar_file_handle; // File handle for the temp TAR

    size_t total_file_size; // Expected total size from Content-Length
    size_t received_file_size; // Bytes received so far

    bool file_fully_received; // Flag: true if all bytes received and temp file closed
    bool reboot_initiated; // Flag: true if reboot sequence was successfully started
} HttpUpdateHandlerCtx;

// Forward declarations
static bool
    handle_completed_upload_and_reboot(HttpUpdateHandlerCtx* ctx, struct mg_connection* conn);
static void http_api_update_on_data_cb(struct mg_connection* conn, struct mg_iobuf* io);
static void http_api_update_on_close_cb(struct mg_connection* conn);

static HttpUpdateHandlerCtx* alloc_raw_update_context() {
    HttpUpdateHandlerCtx* ctx = malloc(sizeof(HttpUpdateHandlerCtx));
    ctx->storage = furi_record_open(RECORD_STORAGE);
    ctx->package_name_param = furi_string_alloc(); // Will be set from query
    ctx->temp_tar_path = furi_string_alloc_printf("%s/upload.tar", UPDATE_STAGING_ROOT);
    ctx->final_staging_path = furi_string_alloc(); // Will be /ext/update/<name>
    ctx->temp_tar_file_handle = storage_file_alloc(ctx->storage);

    ctx->total_file_size = 0;
    ctx->received_file_size = 0;
    ctx->file_fully_received = false;
    ctx->reboot_initiated = false;
    return ctx;
}

static void free_raw_update_context(HttpUpdateHandlerCtx* ctx) {
    if(!ctx) return;

    if(ctx->temp_tar_file_handle) {
        if(storage_file_is_open(ctx->temp_tar_file_handle)) {
            storage_file_close(ctx->temp_tar_file_handle);
        }
        storage_file_free(ctx->temp_tar_file_handle);
        ctx->temp_tar_file_handle = NULL; // Nullify after free
    }

    // Clean up temp TAR file if it exists
    if(furi_string_size(ctx->temp_tar_path) > 0 &&
       storage_file_exists(ctx->storage, furi_string_get_cstr(ctx->temp_tar_path))) {
        FURI_LOG_I(TAG, "Cleaning up temp file: %s", furi_string_get_cstr(ctx->temp_tar_path));
        storage_simply_remove(ctx->storage, furi_string_get_cstr(ctx->temp_tar_path));
    }

    furi_string_free(ctx->package_name_param);
    furi_string_free(ctx->temp_tar_path);
    furi_string_free(ctx->final_staging_path);
    if(ctx->storage) {
        furi_record_close(RECORD_STORAGE);
        ctx->storage = NULL;
    }
    free(ctx);
}

static bool
    handle_completed_upload_and_reboot(HttpUpdateHandlerCtx* ctx, struct mg_connection* conn) {
    FURI_LOG_I(TAG, "File upload complete. Processing update package.");

    /* construct final staging path */
    if(furi_string_empty(ctx->package_name_param)) {
        FURI_LOG_E(TAG, "Package name is empty, cannot proceed.");
        MG_REPLY_BAD_REQUEST(conn);
        return false;
    }
    path_concat(
        UPDATE_STAGING_ROOT,
        furi_string_get_cstr(ctx->package_name_param),
        ctx->final_staging_path);
    FURI_LOG_I(TAG, "Final staging path: %s", furi_string_get_cstr(ctx->final_staging_path));

    UpdaterTarStatus status = updater_tar_install(
        furi_string_get_cstr(ctx->temp_tar_path),
        furi_string_get_cstr(ctx->final_staging_path),
        false);

    if(status != UpdaterTarStatusSuccess) {
        const char* error_str = updater_tar_install_get_error_str(status);
        FURI_LOG_E(TAG, "Update installation failed: %s", error_str);
        MG_REPLY_ERROR(conn, 400, "Update installation failed: %s", error_str);
        return false;
    }

    /* update succeeded - mark for reboot */
    ctx->reboot_initiated = true;
    FURI_LOG_I(TAG, "Reboot pending");

    MG_REPLY_OK_BODY(
        conn, "{\"result\":\"OK\",\"message\":\"Update accepted. System will reboot.\"}\n");

    conn->is_draining = 1;

    // Reboot will now occur in on_close_cb after response is sent.
    return true;
}

static void http_api_update_on_data_cb(struct mg_connection* conn, struct mg_iobuf* io) {
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    HttpUpdateHandlerCtx* update_ctx = (HttpUpdateHandlerCtx*)conn_ctx->context;

    if(!update_ctx || !update_ctx->temp_tar_file_handle ||
       !storage_file_is_open(update_ctx->temp_tar_file_handle)) {
        FURI_LOG_E(TAG, "on_data: Context or file handle invalid/closed. Draining.");
        mg_iobuf_del(io, 0, io->len); // Consume data to prevent further calls
        conn->is_draining = 1; // Mark connection to be closed
        return;
    }

    size_t data_len = io->len;
    FURI_LOG_D(
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
            storage_file_close(update_ctx->temp_tar_file_handle); // Close file on error
            conn->is_draining = 1;
            mg_iobuf_del(io, 0, io->len);
            return;
        }

        size_t written = storage_file_write(update_ctx->temp_tar_file_handle, io->buf, data_len);
        if(written != data_len) {
            FURI_LOG_E(
                TAG,
                "on_data: Failed to write data to temp TAR file. Wrote %zu of %zu.",
                written,
                data_len);
            MG_REPLY_INTERNAL_ERROR(conn, "Failed to save update package (write error).");
            storage_file_close(update_ctx->temp_tar_file_handle); // Close file on error
            conn->is_draining = 1;
            mg_iobuf_del(io, 0, io->len);
            return;
        }
        update_ctx->received_file_size += written;
    }

    mg_iobuf_del(io, 0, io->len); // Consume all data from buffer

    if(update_ctx->received_file_size >= update_ctx->total_file_size) {
        FURI_LOG_I(TAG, "on_data: All data received (%zu bytes)", update_ctx->received_file_size);
        if(storage_file_is_open(update_ctx->temp_tar_file_handle)) {
            storage_file_close(update_ctx->temp_tar_file_handle);
        }
        update_ctx->file_fully_received = true;

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

    bool reboot_was_initiated = false;
    if(update_ctx) {
        reboot_was_initiated = update_ctx->reboot_initiated;
        free_raw_update_context(update_ctx);
        conn_ctx->context = NULL;
    }
    // Clear callbacks
    conn_ctx->raw.on_data = NULL;
    conn_ctx->on_close = NULL;

    if(reboot_was_initiated) {
        FURI_LOG_I(TAG, "Rebooting device now after response sent and connection closed.");
        furi_delay_ms(100); // Brief delay for safety before reset
        furi_hal_power_reset();
    }
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

    // Parse 'name' from query string
    char name_buf[MAX_UPDATE_NAME_LEN + 1] = {0};
    int name_len = mg_http_get_var(&msg->query, "name", name_buf, sizeof(name_buf) - 1);
    if(name_len > 0) {
        name_buf[name_len] = '\0';
        FURI_LOG_I(TAG, "on_headers: Update package name from query: '%s'", name_buf);
        furi_string_set_str(update_ctx->package_name_param, name_buf);
    } else {
        FURI_LOG_I(
            TAG,
            "on_headers: Update package name not provided, using default: '%s'",
            DEFAULT_UPDATE_PACKAGE_NAME);
        furi_string_set_str(update_ctx->package_name_param, DEFAULT_UPDATE_PACKAGE_NAME);
    }

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

    // Ensure staging root /ext/update exists
    if(!storage_dir_exists(update_ctx->storage, UPDATE_STAGING_ROOT)) {
        if(storage_common_mkdir(update_ctx->storage, UPDATE_STAGING_ROOT) != FSE_OK) {
            FURI_LOG_E(
                TAG,
                "on_headers: Failed to create staging root directory: %s",
                UPDATE_STAGING_ROOT);
            MG_REPLY_INTERNAL_ERROR(conn, "Failed to create update staging directory.");
            conn->is_draining = 1;
            return true;
        }
    }

    if(storage_file_exists(update_ctx->storage, furi_string_get_cstr(update_ctx->temp_tar_path))) {
        storage_simply_remove(
            update_ctx->storage, furi_string_get_cstr(update_ctx->temp_tar_path));
    }

    if(!storage_file_open(
           update_ctx->temp_tar_file_handle,
           furi_string_get_cstr(update_ctx->temp_tar_path),
           FSAM_WRITE,
           FSOM_CREATE_ALWAYS)) {
        FURI_LOG_E(
            TAG,
            "on_headers: Failed to open temp TAR file for writing: %s",
            furi_string_get_cstr(update_ctx->temp_tar_path));
        MG_REPLY_INTERNAL_ERROR(conn, "Failed to save update package (file open error).");
        conn->is_draining = 1;
        return true;
    }
    FURI_LOG_I(
        TAG,
        "on_headers: Opened temp TAR file for writing: %s",
        furi_string_get_cstr(update_ctx->temp_tar_path));

    // Set up raw data handlers
    conn_ctx->raw.on_data = http_api_update_on_data_cb;
    conn_ctx->on_close = http_api_update_on_close_cb;

    mg_iobuf_del(&conn->recv, 0, msg->head.len); // Delete HTTP headers
    conn->pfn = NULL; // Silence HTTP protocol handler, we'll use MG_EV_READ

    // Also handle possible data in the buffer
    http_api_update_on_data_cb(conn, &conn->recv);

    return true;
}
