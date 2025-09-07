#include "fetch_client.h"
#include "fetch_client_i.h"

#include <storage/storage.h>
#include <toolbox/path.h>

#define TAG "FetchClient"

//#define FETCH_CLIENT_DEBUG

#ifdef FETCH_CLIENT_DEBUG
#define FETCH_CLIENT_INFO(...)  FURI_LOG_I(__VA_ARGS__)
#define FETCH_CLIENT_ERROR(...) FURI_LOG_E(__VA_ARGS__)
#else
#define FETCH_CLIENT_INFO(...)
#define FETCH_CLIENT_ERROR(...)
#endif

struct FetchClient {
    FuriThread* thread;
    Network* network;
    struct mg_mgr mgr;
    bool done;
    bool is_working;
    FuriString* url;
    FetchClientStatus status;

    uint32_t time_started_raw;
    uint32_t time_started_download;
    size_t delta_received_bytes;
    uint32_t count_receive_packets;

    FetchClientCallbackRawData callback_raw_data;
    void* context_raw_data;
    FetchClientCallbackHeader callback_header;
    void* context_header;
    FetchClientCallbackError callback_error;
    void* context_error;
    FetchClientCallbackStatus callback_status;
    void* context_status;
};

static void fetch_client_on_close(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    FetchClient* instance = (FetchClient*)conn_ctx->context;

    FETCH_CLIENT_INFO(TAG, "on_close");

    instance->status.speed_kbytes_per_sec =
        (float_t)instance->status.received_download_size / 1024.0f /
        ((furi_get_tick() - instance->time_started_download + 1) / 1000.0f);

    if(instance->callback_status) {
        instance->callback_status(instance->status, instance->context_status);
    }

    // Clear callbacks
    conn_ctx->raw.on_data = NULL;
    conn_ctx->on_close = NULL;
    instance->done = true;
}

static void fetch_client_update_on_data_cb(struct mg_connection* conn, struct mg_iobuf* io) {
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    FetchClient* instance = (FetchClient*)conn_ctx->context;
    furi_assert(instance);
    size_t data_len = io->len;

    // Not downloading, just consume
    FURI_LOG_I(TAG, "on_data: Received %zu bytes", data_len);
    // #ifdef FETCH_CLIENT_DEBUG
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

    instance->status.received_download_size += data_len;
    instance->delta_received_bytes += data_len;
    instance->count_receive_packets++;

    if((instance->count_receive_packets % 12) == 0) {
        instance->status.speed_kbytes_per_sec =
            (float_t)instance->delta_received_bytes / 1024.0f /
            ((furi_get_tick() - instance->time_started_raw + 1) / 1000.0f);

        instance->time_started_raw = furi_get_tick();

        if(instance->callback_status) {
            instance->callback_status(instance->status, instance->context_status);
        }
        instance->delta_received_bytes = 0;
    }

    if(instance->callback_raw_data) {
        instance->callback_raw_data((uint8_t*)io->buf, io->len, instance->context_raw_data);
    }

    mg_iobuf_del(io, 0, io->len); // Consume all data from buffer
}

void fetch_client_switching_to_raw_protocol(
    struct mg_connection* conn,
    struct mg_http_message* msg) {
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    FetchClient* instance = (FetchClient*)conn->fn_data;
    conn_ctx->context = instance;

    FETCH_CLIENT_INFO(TAG, "body size: %d", (int)msg->body.len);
    instance->time_started_raw = furi_get_tick();
    instance->time_started_download = instance->time_started_raw;

    if((int)msg->body.len != -1) instance->status.total_download_size = msg->body.len;

    // Set up raw data handlers
    conn_ctx->raw.on_data = fetch_client_update_on_data_cb;
    conn_ctx->on_close = fetch_client_on_close;

    // if(instance->is_downloading) {
    mg_iobuf_del(&conn->recv, 0, msg->head.len); // Delete HTTP headers

    // }
    conn->pfn = NULL; // Silence HTTP protocol handler, we'll use MG_EV_READ
}

static void fetch_client_mg_handler(struct mg_connection* conn, int event, void* ev_data) {
    FetchClient* instance = conn->fn_data;

    if(event == MG_EV_CONNECT) {
        const struct mg_str name = mg_url_host(furi_string_get_cstr(instance->url));

        if(mg_url_is_ssl(furi_string_get_cstr(instance->url))) {
            struct mg_str ca_data =
                mg_file_read((struct mg_fs*)http_fs_get(), FETCH_CLIENT_CA_BUNDLE_PATH);
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
            FETCH_CLIENT_USER_AGENT);

    } else if(event == MG_EV_HTTP_MSG) {
        struct mg_http_message* msg = (struct mg_http_message*)ev_data;
        ConnectionContext* conn_ctx = (void*)conn->data;
        if(conn_ctx->raw.on_data == NULL) { // Skip raw connections
            FuriString* path = furi_string_alloc_printf("%.*s", msg->uri.len, msg->uri.buf);

            FETCH_CLIENT_INFO(TAG, "Data received: %.*s", (int)msg->message.len, msg->message.buf);
            FETCH_CLIENT_INFO(TAG, "path: %s", furi_string_get_cstr(path));

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
        FETCH_CLIENT_INFO(TAG, "Headers received: %.*s", (int)msg->message.len, msg->message.buf);
        FETCH_CLIENT_INFO(TAG, "path: %s", furi_string_get_cstr(path));
        furi_string_free(path);

        if(instance->callback_header) {
            instance->callback_header(
                (uint8_t*)msg->head.buf, msg->head.len, instance->context_header);
        }

        fetch_client_switching_to_raw_protocol(conn, msg);

    } else if(event == MG_EV_READ) {
        FETCH_CLIENT_INFO(TAG, "MG_EV_READ");
        if(!conn->is_websocket) {
            ConnectionContext* conn_ctx = (void*)conn->data;
            if(conn_ctx->raw.on_data) {
                conn_ctx->raw.on_data(conn, &conn->recv);
            }
        }
    } else if(event == MG_EV_CLOSE) {
        FETCH_CLIENT_INFO(TAG, "MG_EV_CLOSE");
        ConnectionContext* conn_ctx = (void*)conn->data;
        if(conn_ctx->on_close) {
            conn_ctx->on_close(conn);
        }

        conn->is_draining = 1;
        instance->done = true;

    } else if(event == MG_EV_ERROR) {
        FETCH_CLIENT_ERROR(TAG, "Error occurred: %s", (char*)ev_data);

        if(instance->callback_error) {
            instance->callback_error((const char*)ev_data, instance->context_error);
        }

        instance->done = true;
    } else if(event == MG_EV_TLS_HS) {
        FETCH_CLIENT_INFO(TAG, "TLS handshake successful");
    }
}

//########## Tread callbacks ##########
static void
    fetch_client_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    furi_assert(thread);
    FetchClient* instance = context;

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
        FETCH_CLIENT_INFO(TAG, "Stop");
        instance->is_working = false;
    }
}

static int32_t fetch_client_thread_callback(void* context) {
    furi_assert(context);
    FetchClient* instance = context;
    FETCH_CLIENT_INFO(TAG, "Start");

    uint32_t start_time = furi_get_tick();

    instance->network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(instance->network);
#ifdef FETCH_CLIENT_DEBUG
    mg_log_set(MG_LL_VERBOSE);
#endif
    mg_mgr_init(&instance->mgr);
    mg_http_connect(
        &instance->mgr, furi_string_get_cstr(instance->url), fetch_client_mg_handler, instance);

    while(!instance->done) {
        mg_mgr_poll(&instance->mgr, 1000);
    }

    mg_mgr_free(&instance->mgr);

    network_deinit_current_thread(instance->network);
    furi_record_close(RECORD_NETWORK);

    FURI_LOG_I(TAG, "Thread duration: %lu ms", furi_get_tick() - start_time);

    FETCH_CLIENT_INFO(TAG, "Stopping thread");

    return 0;
}

FetchClient* fetch_client_alloc(FuriString* url) {
    FetchClient* instance = malloc(sizeof(FetchClient));
    instance->url = furi_string_alloc_printf("%s", furi_string_get_cstr(url));

    instance->done = false;
    instance->status.total_download_size = 0;
    instance->status.received_download_size = 0;
    instance->status.speed_kbytes_per_sec = 0.0f;

    instance->is_working = true;

    return instance;
}

void fetch_client_free(FetchClient* instance) {
    furi_check(instance);

    furi_string_free(instance->url);
    free(instance);
}

void fetch_client_run(FetchClient* instance) {
    furi_check(instance);

    instance->thread = furi_thread_alloc_ex(
        "FetchClient", FETCH_CLIENT_THREAD_STACK_SIZE, fetch_client_thread_callback, instance);
    furi_thread_set_state_context(instance->thread, instance);
    furi_thread_set_state_callback(instance->thread, fetch_client_thread_state_callback);
    FETCH_CLIENT_INFO(TAG, "Starting thread");

    furi_thread_start(instance->thread);
}

bool fetch_client_is_done(FetchClient* instance) {
    furi_check(instance);
    return !instance->is_working;
}

void fetch_client_set_callback_raw_data(
    FetchClient* instance,
    FetchClientCallbackRawData callback,
    void* context) {
    furi_check(instance);
    instance->callback_raw_data = callback;
    instance->context_raw_data = context;
}

void fetch_client_set_callback_header(
    FetchClient* instance,
    FetchClientCallbackHeader callback,
    void* context) {
    furi_check(instance);
    instance->callback_header = callback;
    instance->context_header = context;
}

void fetch_client_set_callback_error(
    FetchClient* instance,
    FetchClientCallbackError callback,
    void* context) {
    furi_check(instance);
    instance->callback_error = callback;
    instance->context_error = context;
}

void fetch_client_set_callback_status(
    FetchClient* instance,
    FetchClientCallbackStatus callback,
    void* context) {
    furi_check(instance);
    instance->callback_status = callback;
    instance->context_status = context;
}
