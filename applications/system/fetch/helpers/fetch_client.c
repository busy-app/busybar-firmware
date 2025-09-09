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
    bool is_processing;
    FuriString* url;
    FetchClientStatus status;

    uint32_t started_raw_ticks;
    uint32_t started_download_ticks;
    size_t delta_received_bytes;
    uint32_t count_receive_packets;

    FetchClientCallbackRawData callback_raw_data;
    FetchClientCallbackHeader callback_header;
    FetchClientCallbackError callback_error;
    FetchClientCallbackStatus callback_status;
    void* context;
};

static void fetch_client_on_close(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
    FetchClient* instance = (FetchClient*)conn_ctx->context;

    FETCH_CLIENT_INFO(TAG, "on_close");

    instance->status.speed_bytes_per_sec =
        (uint32_t)((float)instance->status.received_download_size /
                   ((furi_get_tick() - instance->started_download_ticks + 1) /
                    furi_kernel_get_tick_frequency()));

    if(instance->callback_status) {
        instance->callback_status(instance->status, instance->context);
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

    FETCH_CLIENT_INFO(TAG, "on_data: Received %zu bytes", io->len);

    instance->status.received_download_size += io->len;
    instance->delta_received_bytes += io->len;
    instance->count_receive_packets++;

    if((instance->count_receive_packets % 12) == 0) {
        instance->status.speed_bytes_per_sec =
            (uint32_t)((float)instance->delta_received_bytes /
                       ((furi_get_tick() - instance->started_raw_ticks + 1) /
                        (float)furi_kernel_get_tick_frequency()));

        instance->started_raw_ticks = furi_get_tick();

        if(instance->callback_status) {
            instance->callback_status(instance->status, instance->context);
        }
        instance->delta_received_bytes = 0;
    }

    if(instance->callback_raw_data) {
        instance->callback_raw_data((uint8_t*)io->buf, io->len, instance->context);
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
    instance->started_raw_ticks = furi_get_tick();
    instance->started_download_ticks = instance->started_raw_ticks;

    if((int)msg->body.len != -1) instance->status.total_download_size = msg->body.len;

    // Set up raw data handlers
    conn_ctx->raw.on_data = fetch_client_update_on_data_cb;
    conn_ctx->on_close = fetch_client_on_close;

    mg_iobuf_del(&conn->recv, 0, msg->head.len); // Delete HTTP headers
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
            "GET %s HTTP/1.0\r\nHost: %.*s\r\nUser-Agent: %s\r\nAccept: */*\r\n\r\n",
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
            FETCH_CLIENT_INFO(TAG, "Path: %s", furi_string_get_cstr(path));

            furi_string_free(path);
        }

        conn->is_draining = 1;
        instance->done = true;

    } else if(event == MG_EV_HTTP_HDRS) {
        struct mg_http_message* msg = (struct mg_http_message*)ev_data;
        FuriString* path = furi_string_alloc_printf("%.*s", msg->uri.len, msg->uri.buf);

        FETCH_CLIENT_INFO(TAG, "Headers received: %.*s", (int)msg->message.len, msg->message.buf);
        FETCH_CLIENT_INFO(TAG, "Path: %s", furi_string_get_cstr(path));
        furi_string_free(path);

        if(instance->callback_header) {
            instance->callback_header((uint8_t*)msg->head.buf, msg->head.len, instance->context);
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
            instance->callback_error((const char*)ev_data, instance->context);
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
        instance->is_processing = false;
        instance->thread = NULL;
    }
}

static int32_t fetch_client_thread_callback(void* context) {
    furi_assert(context);
    FetchClient* instance = context;
    FETCH_CLIENT_INFO(TAG, "Start");

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

    FETCH_CLIENT_INFO(TAG, "Stopping thread");

    return 0;
}

FetchClient* fetch_client_alloc(void) {
    FetchClient* instance = malloc(sizeof(FetchClient));

    instance->url = NULL;
    instance->status.total_download_size = 0;
    instance->status.received_download_size = 0;
    instance->status.speed_bytes_per_sec = 0;
    instance->is_processing = false;
    instance->done = false;
    instance->thread = NULL;

    return instance;
}

void fetch_client_free(FetchClient* instance) {
    furi_check(instance);
    furi_check(!instance->is_processing);

    furi_string_free(instance->url);
    free(instance);
}

void fetch_client_run(FetchClient* instance, FuriString* url) {
    furi_check(instance);
    furi_check(!instance->is_processing);

    instance->url = furi_string_alloc_set(furi_string_get_cstr(url));

    instance->is_processing = true;
    instance->thread = furi_thread_alloc_ex(
        "FetchClient", FETCH_CLIENT_THREAD_STACK_SIZE, fetch_client_thread_callback, instance);
    furi_thread_set_state_context(instance->thread, instance);
    furi_thread_set_state_callback(instance->thread, fetch_client_thread_state_callback);
    FETCH_CLIENT_INFO(TAG, "Starting thread");

    furi_thread_start(instance->thread);
}

bool fetch_client_is_processing_done(FetchClient* instance) {
    furi_check(instance);
    return !instance->is_processing;
}

void fetch_client_set_context(FetchClient* instance, void* context) {
    furi_check(instance);
    instance->context = context;
}

void fetch_client_set_callback_raw_data(FetchClient* instance, FetchClientCallbackRawData callback) {
    furi_check(instance);
    instance->callback_raw_data = callback;
}

void fetch_client_set_callback_header(FetchClient* instance, FetchClientCallbackHeader callback) {
    furi_check(instance);
    instance->callback_header = callback;
}

void fetch_client_set_callback_error(FetchClient* instance, FetchClientCallbackError callback) {
    furi_check(instance);
    instance->callback_error = callback;
}

void fetch_client_set_callback_status(FetchClient* instance, FetchClientCallbackStatus callback) {
    furi_check(instance);
    instance->callback_status = callback;
}
