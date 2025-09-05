#include "curl_client.h"
#include "curl_client_i.h"

#include <storage/storage.h>
#include <toolbox/path.h>

#define TAG "CurlClient"

#define CURL_CLIENT_DEBUG

#ifdef CURL_CLIENT_DEBUG
#define CURL_CLIENT_INFO(...)  FURI_LOG_I(__VA_ARGS__)
#define CURL_CLIENT_ERROR(...) FURI_LOG_E(__VA_ARGS__)
#else
#define CURL_CLIENT_INFO(...)
#define CURL_CLIENT_ERROR(...)
#endif

struct CurlClient {
    FuriThread* thread;
    Network* network;
    struct mg_mgr mgr;
    bool done;
    FuriString* url;
    int event;
    FuriString* response;
    uint8_t* data_body;
    size_t data_body_size;

    Storage* storage;
    File* temp_file_handle; // File handle for the temp file being written
    FuriString* temp_file_path; // Path to the temporary file being saved
    size_t total_file_size; // Expected total size from Content-Length
    size_t received_file_size; // Bytes received so far
    bool file_fully_received; // Flag: true if all bytes received and file closed
};

// static void curl_client_raw_on_data(struct mg_connection* conn, struct mg_iobuf* io) {
//     ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;

//     CurlClient* instance = (CurlClient*)conn_ctx->context;
//     UNUSED(instance);

//     // if(!instance || !instance->temp_tar_file_handle ||
//     //    !storage_file_is_open(instance->temp_tar_file_handle)) {
//     //     FURI_LOG_E(TAG, "on_data: Context or file handle invalid/closed. Draining.");
//     //     mg_iobuf_del(io, 0, io->len); // Consume data to prevent further calls
//     //     conn->is_draining = 1; // Mark connection to be closed
//     //     return;
//     // }

//     size_t data_len = io->len;
//     CURL_CLIENT_INFO(
//         TAG,
//         "on_data: Received %zu bytes. Total received: %zu / %zu",
//         data_len,
//         instance->received_file_size,
//         instance->total_file_size);

//     if(data_len > 0) {
//         // if(instance->received_file_size + data_len > instance->total_file_size) {
//         //     FURI_LOG_E(
//         //         TAG,
//         //         "on_data: Received more data than expected. Expected %zu, got %zu more.",
//         //         instance->total_file_size,
//         //         (instance->received_file_size + data_len) - instance->total_file_size);
//         //     MG_REPLY_PAYLOAD_TOO_LARGE(conn);
//         //     storage_file_close(instance->temp_tar_file_handle); // Close file on error
//         //     conn->is_draining = 1;
//         //     mg_iobuf_del(io, 0, io->len);
//         //     return;
//         // }

//         // size_t written = storage_file_write(instance->temp_tar_file_handle, io->buf, data_len);
//         // if(written != data_len) {
//         //     FURI_LOG_E(
//         //         TAG,
//         //         "on_data: Failed to write data to temp TAR file. Wrote %zu of %zu.",
//         //         written,
//         //         data_len);
//         //     MG_REPLY_INTERNAL_ERROR(conn, "Failed to save update package (write error).");
//         //     storage_file_close(instance->temp_tar_file_handle); // Close file on error
//         //     conn->is_draining = 1;
//         //     mg_iobuf_del(io, 0, io->len);
//         //     return;
//         // }
//         //instance->received_file_size += written;
//     }
//     CURL_CLIENT_INFO(TAG, "on_data: Received %zu bytes", data_len);
// #ifdef CURL_CLIENT_DEBUG
//     if(data_len) {
//         for(size_t i = 0; i < data_len; i++) {
//             if(!io->buf[i]) {
//                 FURI_LOG_RAW_I(" [00] ");
//             } else {
//                 FURI_LOG_RAW_I("%c", io->buf[i]);
//             }
//         }
//         FURI_LOG_RAW_I("\r\n");
//     }

// #endif
//     mg_iobuf_del(io, 0, io->len); // Consume all data from buffer

//     // if(instance->received_file_size >= instance->total_file_size) {
//     //     FURI_LOG_I(TAG, "on_data: All data received (%zu bytes)", instance->received_file_size);
//     //     if(storage_file_is_open(instance->temp_tar_file_handle)) {
//     //         storage_file_close(instance->temp_tar_file_handle);
//     //     }
//     //     instance->file_fully_received = true;

//     //     if(!handle_completed_upload_and_reboot(instance, conn)) {
//     //         // Error response already sent by handle_completed_upload_and_reboot
//     //         FURI_LOG_E(TAG, "on_data: package handling failed.");
//     //         conn->is_draining = 1;
//     //     }
//     // }
// }

static void curl_client_on_close(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    CurlClient* instance = (CurlClient*)conn_ctx->context;
    UNUSED(instance);

    FURI_LOG_I(TAG, "on_close");

    //bool reboot_was_initiated = false;
    // if(instance) {
    //     reboot_was_initiated = instance->reboot_initiated;
    //     free_raw_update_context(instance);
    //     conn_ctx->context = NULL;
    // }

    // Clear callbacks
    conn_ctx->raw.on_data = NULL;
    conn_ctx->on_close = NULL;

    // if(reboot_was_initiated) {
    //     FURI_LOG_I(TAG, "Rebooting device now after response sent and connection closed.");
    //     furi_delay_ms(100); // Brief delay for safety before reset
    //     furi_hal_power_reset();
    // }
}

static void curl_client_update_on_data_cb(struct mg_connection* conn, struct mg_iobuf* io) {
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    CurlClient* instance = (CurlClient*)conn_ctx->context;

    if(!instance || !instance->temp_file_handle ||
       !storage_file_is_open(instance->temp_file_handle)) {
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
        instance->received_file_size,
        instance->total_file_size);

    if(data_len > 0) {
        if(instance->received_file_size + data_len > instance->total_file_size) {
            FURI_LOG_E(
                TAG,
                "on_data: Received more data than expected. Expected %zu, got %zu more.",
                instance->total_file_size,
                (instance->received_file_size + data_len) - instance->total_file_size);
            //MG_REPLY_PAYLOAD_TOO_LARGE(conn);
            storage_file_close(instance->temp_file_handle); // Close file on error
            conn->is_draining = 1;
            mg_iobuf_del(io, 0, io->len);
            return;
        }

        size_t written = storage_file_write(instance->temp_file_handle, io->buf, data_len);
        if(written != data_len) {
            FURI_LOG_E(
                TAG,
                "on_data: Failed to write data to temp file. Wrote %zu of %zu.",
                written,
                data_len);
            //MG_REPLY_INTERNAL_ERROR(conn, "Failed to save update package (write error).");
            storage_file_close(instance->temp_file_handle); // Close file on error
            conn->is_draining = 1;
            mg_iobuf_del(io, 0, io->len);
            return;
        }
        instance->received_file_size += written;
    }

    mg_iobuf_del(io, 0, io->len); // Consume all data from buffer

    if(instance->received_file_size >= instance->total_file_size) {
        FURI_LOG_I(TAG, "on_data: All data received (%zu bytes)", instance->received_file_size);
        if(storage_file_is_open(instance->temp_file_handle)) {
            storage_file_close(instance->temp_file_handle);
        }
        instance->file_fully_received = true;

        // if(!handle_completed_upload_and_reboot(instance, conn)) {
        //     // Error response already sent by handle_completed_upload_and_reboot
        //     FURI_LOG_E(TAG, "on_data: package handling failed.");
        //     conn->is_draining = 1;
        // }
    }
}

void curl_client_switching_to_raw_protocol(struct mg_connection* conn, struct mg_http_message* msg) {
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    CurlClient* instance = (CurlClient*)conn->fn_data;
    conn_ctx->context = instance;

    CURL_CLIENT_INFO(TAG, "body size: %d", (int)msg->body.len);

    if(msg->body.len != -1) instance->total_file_size = msg->body.len;

    if(instance->total_file_size == 0) {
        FURI_LOG_W(TAG, "on_headers: Content-Length is 0 or missing/invalid. No file to upload?");
        //MG_REPLY_BAD_REQUEST(conn);
        conn->is_draining = 1;
        return;
    }

    if(instance->total_file_size > MAX_UPLOAD_FILE_SIZE) {
        FURI_LOG_E(
            TAG,
            "on_headers: File size %zu exceeds max %u.",
            instance->total_file_size,
            MAX_UPLOAD_FILE_SIZE);
        //MG_REPLY_PAYLOAD_TOO_LARGE(conn);
        conn->is_draining = 1;
        return;
    }

    FURI_LOG_I(TAG, "on_headers: Expecting file of size: %zu bytes", instance->total_file_size);

    // // Ensure staging root /ext/update exists
    // if(!storage_dir_exists(instance->storage, STORAGE_EXT_PATH_PREFIX)) {
    //     if(storage_common_mkdir(instance->storage, STORAGE_EXT_PATH_PREFIX) != FSE_OK) {
    //         FURI_LOG_E(
    //             TAG,
    //             "on_headers: Failed to create staging root directory: %s",
    //             STORAGE_EXT_PATH_PREFIX);
    //         MG_REPLY_INTERNAL_ERROR(conn, "Failed to create update staging directory.");
    //         conn->is_draining = 1;
    //         return true;
    //     }
    // }

    if(storage_file_exists(instance->storage, furi_string_get_cstr(instance->temp_file_path))) {
        storage_simply_remove(instance->storage, furi_string_get_cstr(instance->temp_file_path));
    }

    if(!storage_file_open(
           instance->temp_file_handle,
           furi_string_get_cstr(instance->temp_file_path),
           FSAM_WRITE,
           FSOM_CREATE_ALWAYS)) {
        FURI_LOG_E(
            TAG,
            "on_headers: Failed to open temp file for writing: %s",
            furi_string_get_cstr(instance->temp_file_path));
        //MG_REPLY_INTERNAL_ERROR(conn, "Failed to save update package (file open error).");
        conn->is_draining = 1;
        return;
    }
    FURI_LOG_I(
        TAG,
        "on_headers: Opened temp a file for writing: %s",
        furi_string_get_cstr(instance->temp_file_path));

    // Set up raw data handlers
    conn_ctx->raw.on_data = curl_client_update_on_data_cb;
    conn_ctx->on_close = curl_client_on_close;

    mg_iobuf_del(&conn->recv, 0, msg->head.len); // Delete HTTP headers
    conn->pfn = NULL; // Silence HTTP protocol handler, we'll use MG_EV_READ
}

static void curl_client_mg_handler(struct mg_connection* conn, int event, void* ev_data) {
    CurlClient* instance = conn->fn_data;

    if(event == MG_EV_CONNECT) {
        const struct mg_str name = mg_url_host(furi_string_get_cstr(instance->url));

        if(mg_url_is_ssl(furi_string_get_cstr(instance->url))) {
            struct mg_str ca_data =
                mg_file_read((struct mg_fs*)http_fs_get(), CURL_CLIENT_CA_BUNDLE_PATH);
            const struct mg_tls_opts opts = {.ca = ca_data, .name = name};
            mg_tls_init(conn, &opts);
            free(ca_data.buf);
        }

        mg_printf(
            conn,
            "GET %s HTTP/1.0\r\nHost: %.*s\r\nUser-Agent: %s\r\n\r\n",
            mg_url_uri(furi_string_get_cstr(instance->url)),
            name.len,
            name.buf,
            CURL_CLIENT_USER_AGENT);

    } else if(event == MG_EV_HTTP_MSG) {
        struct mg_http_message* msg = (struct mg_http_message*)ev_data;
        ConnectionContext* conn_ctx = (void*)conn->data;
        if(conn_ctx->raw.on_data == NULL) { // Skip raw connections
            FuriString* path = furi_string_alloc_printf("%.*s", msg->uri.len, msg->uri.buf);

            CURL_CLIENT_INFO(TAG, "Data received: %.*s", (int)msg->message.len, msg->message.buf);
            CURL_CLIENT_INFO(TAG, "path: %s", furi_string_get_cstr(path));

            //bool result = http_handle_request(path, context->handlers, conn, msg);
            furi_string_free(path);
            // if(!result) {
            //     MG_REPLY_BAD_REQUEST(conn);
            // }
        }

        conn->is_draining = 1;
        instance->done = true;

    } else if(event == MG_EV_HTTP_HDRS) {
        struct mg_http_message* msg = (struct mg_http_message*)ev_data;
        FuriString* path = furi_string_alloc_printf("%.*s", msg->uri.len, msg->uri.buf);
        //http_handle_headers(path, context->handlers, conn, msg);
        CURL_CLIENT_INFO(TAG, "Headers received: %.*s", (int)msg->message.len, msg->message.buf);
        CURL_CLIENT_INFO(TAG, "path: %s", furi_string_get_cstr(path));
        furi_string_free(path);

        curl_client_switching_to_raw_protocol(conn, msg);

    } else if(event == MG_EV_READ) {
        CURL_CLIENT_INFO(TAG, "MG_EV_READ");
        if(!conn->is_websocket) {
            ConnectionContext* conn_ctx = (void*)conn->data;
            if(conn_ctx->raw.on_data) {
                conn_ctx->raw.on_data(conn, &conn->recv);
            }
        }
        // } else if(event == MG_EV_WS_MSG || event == MG_EV_WS_CTL) {
        //     CURL_CLIENT_INFO(TAG, "MG_EV_WS_MSG || MG_EV_WS_CTL");
        //     struct mg_ws_message* ws_msg = (struct mg_ws_message*)ev_data;
        //     ConnectionContext* conn_ctx = (void*)conn->data;
        //     if(conn_ctx->ws.on_message) {
        //         conn_ctx->ws.on_message(conn, ws_msg);
        //     }
        // } else if(event == MG_EV_WS_OPEN) {
        //     CURL_CLIENT_INFO(TAG, "MG_EV_WS_OPEN");
        //     ConnectionContext* conn_ctx = (void*)conn->data;
        //     if(conn_ctx->ws.on_open) {
        //         conn_ctx->ws.on_open(conn);
        //     }
    } else if(event == MG_EV_CLOSE) {
        CURL_CLIENT_INFO(TAG, "MG_EV_CLOSE");
        ConnectionContext* conn_ctx = (void*)conn->data;
        if(conn_ctx->on_close) {
            conn_ctx->on_close(conn);
        }

        conn->is_draining = 1;
        instance->done = true;

    } else if(event == MG_EV_ERROR) {
        CURL_CLIENT_ERROR(TAG, "Error occurred: %s", (char*)ev_data);

        furi_string_printf(instance->response, "Error occurred: %s", (char*)ev_data);
        instance->event = event;

        instance->done = true;
    } else if(event == MG_EV_TLS_HS) {
        CURL_CLIENT_INFO(TAG, "TLS handshake successful");
    }
}

//########## Tread callbacks ##########
static void
    curl_client_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    furi_assert(thread);
    UNUSED(context);

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
        CURL_CLIENT_INFO(TAG, "Stop");
    }
}

static int32_t curl_client_thread_callback(void* context) {
    furi_assert(context);
    CurlClient* instance = context;
    CURL_CLIENT_INFO(TAG, "Start");

    uint32_t start_time = furi_get_tick();

    instance->network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(instance->network);
#ifdef CURL_CLIENT_DEBUG
    mg_log_set(MG_LL_VERBOSE);
#endif
    mg_mgr_init(&instance->mgr);
    mg_http_connect(
        &instance->mgr, furi_string_get_cstr(instance->url), curl_client_mg_handler, instance);

    while(!instance->done) {
        mg_mgr_poll(&instance->mgr, 1000);
    }

    mg_mgr_free(&instance->mgr);

    network_deinit_current_thread(instance->network);
    furi_record_close(RECORD_NETWORK);

    FURI_LOG_I(TAG, "Thread duration: %lu ms", furi_get_tick() - start_time);

    CURL_CLIENT_INFO(TAG, "Stopping thread");

    return 0;
}

CurlClient* curl_client_alloc(FuriString* url, FuriString* file_path) {
    CurlClient* instance = malloc(sizeof(CurlClient));
    instance->url = furi_string_alloc_printf("%s", furi_string_get_cstr(url));
    instance->response = furi_string_alloc();

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->temp_file_path = furi_string_alloc_printf(
        "%s/%s", STORAGE_EXT_PATH_PREFIX, furi_string_get_cstr(file_path));
    instance->temp_file_handle = storage_file_alloc(instance->storage);

    instance->done = false;
    instance->total_file_size = 0;
    instance->received_file_size = 0;
    instance->file_fully_received = false;

    return instance;
}

void curl_client_free(CurlClient* instance) {
    furi_check(instance);

    furi_string_free(instance->url);
    furi_string_free(instance->response);

    if(instance->temp_file_handle) {
        if(storage_file_is_open(instance->temp_file_handle)) {
            storage_file_close(instance->temp_file_handle);
        }
        storage_file_free(instance->temp_file_handle);
        instance->temp_file_handle = NULL; // Nullify after free
    }

    furi_string_free(instance->temp_file_path);

    if(instance->storage) {
        furi_record_close(RECORD_STORAGE);
        instance->storage = NULL;
    }

    if(instance->data_body) {
        free(instance->data_body);
        instance->data_body = NULL;
    }
    free(instance);
}

void curl_client_run(CurlClient* instance) {
    furi_check(instance);

    instance->thread = furi_thread_alloc_ex(
        "CurlClient", CURL_CLIENT_THREAD_STACK_SIZE, curl_client_thread_callback, instance);
    furi_thread_set_state_callback(instance->thread, curl_client_thread_state_callback);
    CURL_CLIENT_INFO(TAG, "Starting thread");

    furi_thread_start(instance->thread);
}

bool curl_client_is_done(CurlClient* instance) {
    furi_check(instance);
    return instance->done;
}
