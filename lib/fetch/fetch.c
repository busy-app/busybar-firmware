#include "fetch.h"
#include "fetch_i.h"

#include <network/network.h>
#include <storage/storage.h>

#include <toolbox/timers.h>

#include <mongoose_glue.h>

#define TAG "Fetch"

#define FETCH_CLIENT_INACTIVITY_TIMEOUT_MS (5 * 1000)

//#define FETCH_CLIENT_DEBUG

#ifdef FETCH_CLIENT_DEBUG
#define FETCH_CLIENT_INFO(...)  FURI_LOG_I(__VA_ARGS__)
#define FETCH_CLIENT_ERROR(...) FURI_LOG_E(__VA_ARGS__)
#else
#define FETCH_CLIENT_INFO(...)
#define FETCH_CLIENT_ERROR(...)
#endif

struct Fetch {
    struct mg_mgr mgr;
    const FetchRequest* request;
    FetchStatus status;

    uint32_t started_raw_ticks;
    uint32_t started_download_ticks;
    size_t delta_received_bytes;
    uint32_t count_receive_packets;
    CoarseTimer activity_timer;

    FetchCallbackRawData callback_raw_data;
    FetchCallbackHeader callback_header;
    FetchCallbackError callback_error;
    FetchCallbackStatus callback_status;
    FetchCallbackFinished callback_finished;
    void* callback_context;

    _Atomic bool is_running;
    _Atomic bool is_force_stop;
    _Atomic bool is_finished;
};

static uint32_t fetch_calc_download_speed(size_t size_delta, uint32_t start_timestamp_ticks) {
    const uint32_t delta_ticks = furi_get_tick() - start_timestamp_ticks + 1;
    const float delta_s = (float)delta_ticks / furi_kernel_get_tick_frequency();
    return size_delta / delta_s;
}

static void fetch_on_close(struct mg_connection* conn) {
    FetchConnectionContext* conn_ctx = (FetchConnectionContext*)conn->data;
    Fetch* instance = (Fetch*)conn_ctx->context;

    FETCH_CLIENT_INFO(TAG, "on_close");

    instance->status.speed_bytes_per_sec = fetch_calc_download_speed(
        instance->status.received_download_size, instance->started_download_ticks);

    if(instance->callback_status) {
        instance->callback_status(&instance->status, instance->callback_context);
    }

    conn_ctx->on_data = NULL;
    conn_ctx->on_close = NULL;

    instance->is_running = false;
}

static void fetch_update_on_data_cb(struct mg_connection* conn, struct mg_iobuf* io) {
    FetchConnectionContext* conn_ctx = (FetchConnectionContext*)conn->data;
    Fetch* instance = (Fetch*)conn_ctx->context;
    furi_assert(instance);

    FETCH_CLIENT_INFO(TAG, "on_data: Received %zu bytes", io->len);

    instance->activity_timer = coarse_timer_create(FETCH_CLIENT_INACTIVITY_TIMEOUT_MS);

    instance->status.received_download_size += io->len;
    instance->delta_received_bytes += io->len;
    instance->count_receive_packets++;

    if((instance->count_receive_packets % (12 * 8)) == 0) {
        instance->status.speed_bytes_per_sec =
            fetch_calc_download_speed(instance->delta_received_bytes, instance->started_raw_ticks);

        instance->started_raw_ticks = furi_get_tick();

        if(instance->callback_status) {
            instance->callback_status(&instance->status, instance->callback_context);
        }
        instance->delta_received_bytes = 0;
    }

    if(instance->callback_raw_data) {
        instance->callback_raw_data(io->buf, io->len, instance->callback_context);
    }

    mg_iobuf_del(io, 0, io->len); // Consume all data from buffer
}

static FURI_ALWAYS_INLINE void
    fetch_switching_to_raw_protocol(struct mg_connection* conn, struct mg_http_message* msg) {
    FetchConnectionContext* conn_ctx = (FetchConnectionContext*)conn->data;
    Fetch* instance = (Fetch*)conn->fn_data;
    conn_ctx->context = instance;

    FETCH_CLIENT_INFO(TAG, "body size: %d", (int)msg->body.len);
    instance->started_raw_ticks = furi_get_tick();
    instance->started_download_ticks = instance->started_raw_ticks;

    if((int)msg->body.len != -1) instance->status.total_download_size = msg->body.len;

    // Set up raw data handlers
    conn_ctx->on_data = fetch_update_on_data_cb;
    conn_ctx->on_close = fetch_on_close;

    mg_iobuf_del(&conn->recv, 0, msg->head.len); // Delete HTTP headers
    conn->pfn = NULL; // Silence HTTP protocol handler, we'll use MG_EV_READ
}

static bool fetch_init_tls(Fetch* instance, struct mg_connection* conn, struct mg_str hostname) {
    bool success = false;

    struct mg_str ca_data =
        mg_file_read((struct mg_fs*)http_fs_get(), FETCH_CLIENT_CA_BUNDLE_PATH);

    if(ca_data.buf != NULL && ca_data.len > 0) {
        const struct mg_tls_opts opts = {
            .ca = ca_data,
            .name = hostname,
        };

        mg_tls_init(conn, &opts);

        success = true;

    } else {
        FETCH_CLIENT_ERROR(TAG, "Failed to read CA bundle from %s", FETCH_CLIENT_CA_BUNDLE_PATH);

        if(instance->callback_error) {
            instance->callback_error(
                "Failed to read CA certificate bundle", instance->callback_context);
        }

        conn->is_draining = 1;
    }

    if(ca_data.buf != NULL) {
        free(ca_data.buf);
    }

    return success;
}

static bool fetch_init_connection(Fetch* instance, struct mg_connection* conn) {
    bool success = true;

    const char* url = instance->request->url;

    if(mg_url_is_ssl(url)) {
        success = fetch_init_tls(instance, conn, mg_url_host(url));
    }

    return success;
}

static void fetch_send_request_method(const FetchRequest* request, struct mg_connection* conn) {
    const char* method = request->method;
    const char* uri = mg_url_uri(request->url);

    if(method == NULL) {
        method = "GET";
    }

    mg_printf(conn, "%s %s HTTP/1.0\r\n", method, uri);
}

static void fetch_send_request_headers(const FetchRequest* request, struct mg_connection* conn) {
    const struct mg_str hostname = mg_url_host(request->url);

    mg_printf(
        conn,
        "Host: %.*s\r\n"
        "User-Agent: %s\r\n"
        "Accept: */*\r\n"
        "Connection: close\r\n",
        hostname.len,
        hostname.buf,
        FETCH_CLIENT_USER_AGENT);

    const uint32_t body_length = request->body.length;

    if(body_length > 0) {
        mg_printf(conn, "Content-length: %u\r\n", body_length);
    }
}

static void
    fetch_send_extra_request_headers(const FetchRequest* request, struct mg_connection* conn) {
    for(uint32_t i = 0; i < request->headers.count; ++i) {
        mg_printf(conn, "%s\r\n", request->headers.data[i]);
    }

    mg_send(conn, "\r\n", 2);
}

static void fetch_send_request_body(const FetchRequest* request, struct mg_connection* conn) {
    const char* body_data = request->body.data;
    const uint32_t body_length = request->body.length;

    if(body_data != NULL && body_length > 0) {
        mg_send(conn, body_data, body_length);
    }
}

static FURI_ALWAYS_INLINE void fetch_connect_event(Fetch* instance, struct mg_connection* conn) {
    const FetchRequest* request = instance->request;
    furi_assert(request);

    if(fetch_init_connection(instance, conn)) {
        fetch_send_request_method(request, conn);
        fetch_send_request_headers(request, conn);
        fetch_send_extra_request_headers(request, conn);
        fetch_send_request_body(request, conn);
    }
}

static FURI_ALWAYS_INLINE void
    fetch_http_msg_event(Fetch* instance, struct mg_connection* conn, void* ev_data) {
    struct mg_http_message* msg = (struct mg_http_message*)ev_data;
    FetchConnectionContext* conn_ctx = (void*)conn->data;
    if(conn_ctx->on_data == NULL) { // Skip raw connections
        FuriString* path = furi_string_alloc_printf("%.*s", msg->uri.len, msg->uri.buf);

        FETCH_CLIENT_INFO(TAG, "Data received: %.*s", (int)msg->message.len, msg->message.buf);
        FETCH_CLIENT_INFO(TAG, "Path: %s", furi_string_get_cstr(path));

        furi_string_free(path);
    }

    conn->is_draining = 1;
    instance->is_running = false;
}

static FURI_ALWAYS_INLINE void
    fetch_http_hdrs_event(Fetch* instance, struct mg_connection* conn, void* ev_data) {
    struct mg_http_message* msg = (struct mg_http_message*)ev_data;
    FuriString* path = furi_string_alloc_printf("%.*s", msg->uri.len, msg->uri.buf);

    FETCH_CLIENT_INFO(TAG, "Headers received: %.*s", (int)msg->message.len, msg->message.buf);
    FETCH_CLIENT_INFO(TAG, "Path: %s", furi_string_get_cstr(path));
    furi_string_free(path);

    if(instance->callback_header) {
        instance->callback_header(msg->head.buf, msg->head.len, instance->callback_context);
    }

    fetch_switching_to_raw_protocol(conn, msg);
}

static FURI_ALWAYS_INLINE void fetch_read_event(Fetch* instance, struct mg_connection* conn) {
    UNUSED(instance);
    FETCH_CLIENT_INFO(TAG, "MG_EV_READ");
    if(!conn->is_websocket) {
        FetchConnectionContext* conn_ctx = (void*)conn->data;
        if(conn_ctx->on_data) {
            conn_ctx->on_data(conn, &conn->recv);
        }
    }
}

static FURI_ALWAYS_INLINE void fetch_close_event(Fetch* instance, struct mg_connection* conn) {
    FETCH_CLIENT_INFO(TAG, "MG_EV_CLOSE");
    FetchConnectionContext* conn_ctx = (void*)conn->data;
    if(conn_ctx->on_close) {
        conn_ctx->on_close(conn);
    }

    conn->is_draining = 1;
    instance->is_running = false;
}

static FURI_ALWAYS_INLINE void
    fetch_error_event(Fetch* instance, struct mg_connection* conn, void* ev_data) {
    UNUSED(conn);
    FETCH_CLIENT_ERROR(TAG, "Error occurred: %s", (char*)ev_data);

    if(instance->callback_error) {
        instance->callback_error((const char*)ev_data, instance->callback_context);
    }

    instance->is_running = false;
}

static void fetch_mg_handler(struct mg_connection* conn, int event, void* ev_data) {
    Fetch* instance = conn->fn_data;

    if(event == MG_EV_CONNECT) {
        fetch_connect_event(instance, conn);
    } else if(event == MG_EV_HTTP_MSG) {
        fetch_http_msg_event(instance, conn, ev_data);
    } else if(event == MG_EV_HTTP_HDRS) {
        fetch_http_hdrs_event(instance, conn, ev_data);
    } else if(event == MG_EV_READ) {
        fetch_read_event(instance, conn);
    } else if(event == MG_EV_CLOSE) {
        fetch_close_event(instance, conn);
    } else if(event == MG_EV_ERROR) {
        fetch_error_event(instance, conn, ev_data);
    } else if(event == MG_EV_TLS_HS) {
        FETCH_CLIENT_INFO(TAG, "TLS handshake successful");
    }
}

//########## Thread callbacks ##########
static void fetch_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    furi_assert(thread);
    Fetch* instance = context;

    if(state == FuriThreadStateStopped) {
        furi_assert(!instance->is_finished);

        FETCH_CLIENT_INFO(TAG, "Stop");

        if(instance->callback_finished) {
            instance->callback_finished(instance->callback_context);
        }

        instance->is_finished = true;

        furi_thread_free(thread);
    }
}

static int32_t fetch_thread_callback(void* context) {
    furi_assert(context);
    Fetch* instance = context;

    FETCH_CLIENT_INFO(TAG, "Start");

    instance->is_running = true;

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);

#ifdef FETCH_CLIENT_DEBUG
    mg_log_set(MG_LL_VERBOSE);
#endif

    mg_mgr_init(&instance->mgr);

    struct mg_connection* conn =
        mg_http_connect(&instance->mgr, instance->request->url, fetch_mg_handler, instance);

    if(conn != NULL) {
        instance->activity_timer = coarse_timer_create(FETCH_CLIENT_INACTIVITY_TIMEOUT_MS);

        while(instance->is_running) {
            if(coarse_timer_is_expired(instance->activity_timer)) {
                FETCH_CLIENT_ERROR(TAG, "Inactivity timeout");

                if(instance->callback_error) {
                    instance->callback_error("Inactivity timeout", instance->callback_context);
                }

                conn->is_draining = 1;
                break;
            }

            if(instance->is_force_stop) {
                FETCH_CLIENT_INFO(TAG, "Force stopped");
                conn->is_draining = 1;
                FETCH_CLIENT_INFO(TAG, "Connection closed");
            }

            mg_mgr_poll(&instance->mgr, 1000);
        }

    } else {
        FETCH_CLIENT_ERROR(TAG, "Failed to connect to server");
    }

    mg_mgr_free(&instance->mgr);

    network_deinit_current_thread(network);
    furi_record_close(RECORD_NETWORK);

    FETCH_CLIENT_INFO(TAG, "Stopping thread");

    return 0;
}

Fetch* fetch_alloc(void) {
    Fetch* instance = malloc(sizeof(Fetch));

    instance->status.total_download_size = 0;
    instance->status.received_download_size = 0;
    instance->status.speed_bytes_per_sec = 0;

    return instance;
}

void fetch_free(Fetch* instance) {
    furi_check(instance);
    furi_check(instance->is_finished);
    furi_check(!instance->is_running);

    free(instance);
}

void fetch_start(Fetch* instance, const FetchRequest* request) {
    furi_check(instance);
    furi_assert(!instance->is_finished);
    furi_assert(!instance->is_running);

    furi_check(request);
    furi_check(request->url);
    furi_check(request->headers.count < FETCH_HEADERS_COUNT_MAX);

    instance->request = request;

    FuriThread* thread = furi_thread_alloc_ex(
        "Fetch", FETCH_CLIENT_THREAD_STACK_SIZE, fetch_thread_callback, instance);
    furi_thread_set_state_context(thread, instance);
    furi_thread_set_state_callback(thread, fetch_thread_state_callback);

    FETCH_CLIENT_INFO(TAG, "Starting thread");

    furi_thread_start(thread);
}

void fetch_stop(Fetch* instance) {
    furi_check(instance);
    instance->is_force_stop = true;
}

bool fetch_is_finished(Fetch* instance) {
    furi_check(instance);
    return instance->is_finished;
}

void fetch_set_callback_context(Fetch* instance, void* context) {
    furi_check(instance);
    instance->callback_context = context;
}

void fetch_set_callback_raw_data(Fetch* instance, FetchCallbackRawData callback) {
    furi_check(instance);
    instance->callback_raw_data = callback;
}

void fetch_set_callback_header(Fetch* instance, FetchCallbackHeader callback) {
    furi_check(instance);
    instance->callback_header = callback;
}

void fetch_set_callback_error(Fetch* instance, FetchCallbackError callback) {
    furi_check(instance);
    instance->callback_error = callback;
}

void fetch_set_callback_status(Fetch* instance, FetchCallbackStatus callback) {
    furi_check(instance);
    instance->callback_status = callback;
}

void fetch_set_callback_finished(Fetch* instance, FetchCallbackFinished callback) {
    furi_check(instance);
    instance->callback_finished = callback;
}
