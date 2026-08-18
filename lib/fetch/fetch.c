#include "fetch.h"

#include <network/network.h>
#include <toolbox/timers.h>

#include <mongoose_dns.h>
#include <mongoose_tls.h>

#define TAG "Fetch"

#define FETCH_INACTIVITY_TIMEOUT_MS (5 * 1000)

#define FETCH_USER_AGENT                         \
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) " \
    "AppleWebKit/537.36 (KHTML, like Gecko) "    \
    "Chrome/138.0.0.0 Safari/537.36"

#ifdef FETCH_DEBUG
#define FETCH_LOG_I(...) FURI_LOG_I(__VA_ARGS__)
#define FETCH_LOG_E(...) FURI_LOG_E(__VA_ARGS__)
#else
#define FETCH_LOG_I(...)
#define FETCH_LOG_E(...)
#endif

// =====
// Types
// =====

struct Fetch {
    struct mg_mgr mgr;
    const FetchRequest* request;
    FetchProgress progress;

    uint32_t started_raw_ticks;
    uint32_t started_download_ticks;
    size_t delta_received_bytes;
    uint32_t count_receive_packets;
    CoarseTimer activity_timer;

    FetchRxDataCallback rx_data_callback;
    FetchHeaderCallback header_callback;
    FetchErrorCallback error_callback;
    FetchProgressCallback progress_callback;
    void* callback_context;

    bool is_running;
    bool is_error_occurred;
    _Atomic bool is_stop_requested;
};

// ============================
// Stateless internal functions
// ============================

static uint32_t fetch_calc_download_speed(size_t size_delta, uint32_t start_timestamp_ticks) {
    const uint32_t delta_ticks = furi_get_tick() - start_timestamp_ticks + 1;
    const float delta_s = (float)delta_ticks / furi_kernel_get_tick_frequency();
    return size_delta / delta_s;
}

// ===========================
// Stateful internal functions
// ===========================

static bool fetch_is_rx_complete(const Fetch* instance) {
    bool is_complete = false;

    const FetchProgress* progress = &instance->progress;

    if(progress->has_total_download_size) {
        const size_t expected_size = progress->total_download_size;
        const size_t actual_size = progress->received_download_size;

        if(actual_size >= expected_size) {
            is_complete = true;
        }
    }

    return is_complete;
}

static void fetch_consume_rx_data(struct mg_connection* conn, size_t length) {
    mg_iobuf_del(&conn->recv, 0, length);
}

static void fetch_switch_to_raw_protocol(
    Fetch* instance,
    struct mg_connection* conn,
    const struct mg_http_message* msg) {
    const int32_t body_length = (int32_t)msg->body.len;
    FETCH_LOG_I(TAG, "body size: %ld", body_length);

    instance->started_raw_ticks = furi_get_tick();
    instance->started_download_ticks = instance->started_raw_ticks;

    if(body_length != -1) {
        FetchProgress* progress = &instance->progress;
        progress->total_download_size = body_length;
        progress->has_total_download_size = true;
    }

    fetch_consume_rx_data(conn, msg->head.len);
    /* Mongoose will detach the protocol handler function automatically
     * after the received data has been altered in the header callback.
     * Here it is done preemptively for good measure. */
    conn->pfn = NULL;
}

static void fetch_raise_error(Fetch* instance, const char* error_message) {
    FETCH_LOG_E(TAG, "Error: %s", error_message);

    if(instance->error_callback) {
        instance->error_callback(error_message, instance->callback_context);
    }

    instance->is_error_occurred = true;
}

static bool fetch_init_connection(const FetchRequest* request, struct mg_connection* conn) {
    bool success = true;

    const char* url = request->url;

    if(mg_url_is_ssl(url)) {
        success = mongoose_tls_init(conn, url, &request->tls_config);
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
        FETCH_USER_AGENT);

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

    if(fetch_init_connection(request, conn)) {
        fetch_send_request_method(request, conn);
        fetch_send_request_headers(request, conn);
        fetch_send_extra_request_headers(request, conn);
        fetch_send_request_body(request, conn);

    } else {
        fetch_raise_error(instance, "Failed to establish TLS connection");
        conn->is_draining = 1;
    }
}

static FURI_ALWAYS_INLINE void
    fetch_http_hdrs_event(Fetch* instance, struct mg_connection* conn, void* ev_data) {
    const struct mg_http_message* msg = ev_data;

    FETCH_LOG_I(TAG, "MG_EV_HTTP_HDRS: %.*s", msg->message.len, msg->message.buf);
    FETCH_LOG_I(TAG, "Path: %.*s", msg->uri.len, msg->uri.buf);

    if(instance->header_callback) {
        instance->header_callback(msg->head.buf, msg->head.len, instance->callback_context);
    }

    fetch_switch_to_raw_protocol(instance, conn, msg);
}

static FURI_ALWAYS_INLINE void fetch_read_event(Fetch* instance, struct mg_connection* conn) {
    FETCH_LOG_I(TAG, "MG_EV_READ");

    if(conn->pfn) {
        // Ignore read events if the protocol handler function is still set
        return;
    }

    struct mg_iobuf* recv = &conn->recv;
    const size_t recv_len = recv->len;

    FETCH_LOG_I(TAG, "Received %zu bytes", recv_len);

    instance->activity_timer = coarse_timer_create(FETCH_INACTIVITY_TIMEOUT_MS);
    instance->progress.received_download_size += recv_len;
    instance->delta_received_bytes += recv_len;
    instance->count_receive_packets++;

    if((instance->count_receive_packets % (12 * 8)) == 0) {
        instance->progress.speed_bytes_per_sec =
            fetch_calc_download_speed(instance->delta_received_bytes, instance->started_raw_ticks);

        instance->started_raw_ticks = furi_get_tick();

        if(instance->progress_callback) {
            instance->progress_callback(&instance->progress, instance->callback_context);
        }

        instance->delta_received_bytes = 0;
    }

    if(instance->rx_data_callback) {
        instance->rx_data_callback(recv->buf, recv_len, instance->callback_context);
    }

    fetch_consume_rx_data(conn, recv_len);

    if(fetch_is_rx_complete(instance)) {
        conn->is_draining = 1;
    }
}

static FURI_ALWAYS_INLINE void fetch_close_event(Fetch* instance, struct mg_connection* conn) {
    FETCH_LOG_I(TAG, "MG_EV_CLOSE");

    instance->progress.speed_bytes_per_sec = fetch_calc_download_speed(
        instance->progress.received_download_size, instance->started_download_ticks);

    if(instance->progress_callback) {
        instance->progress_callback(&instance->progress, instance->callback_context);
    }

    conn->is_draining = 1;
    instance->is_running = false;
}

static FURI_ALWAYS_INLINE void
    fetch_error_event(Fetch* instance, struct mg_connection* conn, const void* ev_data) {
    UNUSED(conn);

    const char* error_msg;

    if(ev_data != NULL) {
        error_msg = ev_data;
    } else {
        error_msg = "Unknown error";
    }

    fetch_raise_error(instance, error_msg);
    instance->is_running = false;
}

static void fetch_mg_handler(struct mg_connection* conn, int event, void* ev_data) {
    Fetch* instance = conn->fn_data;

    if(event == MG_EV_CONNECT) {
        fetch_connect_event(instance, conn);
    } else if(event == MG_EV_HTTP_HDRS) {
        fetch_http_hdrs_event(instance, conn, ev_data);
    } else if(event == MG_EV_HTTP_MSG) {
        FURI_LOG_W(TAG, "BUG: MG_EV_HTTP_MSG should never happen");
    } else if(event == MG_EV_READ) {
        fetch_read_event(instance, conn);
    } else if(event == MG_EV_CLOSE) {
        fetch_close_event(instance, conn);
    } else if(event == MG_EV_ERROR) {
        fetch_error_event(instance, conn, ev_data);
    } else if(event == MG_EV_TLS_HS) {
        FETCH_LOG_I(TAG, "TLS handshake successful");
    }
}

static FetchStatus fetch_get_status(const Fetch* instance) {
    FetchStatus status;

    if(instance->is_error_occurred) {
        status = FetchStatusError;
    } else if(instance->is_stop_requested) {
        status = FetchStatusAborted;
    } else {
        status = FetchStatusOk;
    }

    return status;
}

static void fetch_reset(Fetch* instance) {
    memset(&instance->progress, 0, sizeof(FetchProgress));

    instance->delta_received_bytes = 0;
    instance->count_receive_packets = 0;
    instance->is_error_occurred = false;
    instance->is_stop_requested = false;
}

static bool fetch_verify_response_body_size(Fetch* instance) {
    bool success = true;

    if(!(instance->is_stop_requested || instance->is_error_occurred)) {
        const FetchProgress* progress = &instance->progress;

        if(progress->has_total_download_size) {
            const size_t expected_size = progress->total_download_size;
            const size_t actual_size = progress->received_download_size;

            if(expected_size != actual_size) {
                success = false;
            }
        }
    }

    return success;
}

// ==========
// Public API
// ==========

Fetch* fetch_alloc(void) {
    Fetch* instance = malloc(sizeof(Fetch));
    return instance;
}

void fetch_free(Fetch* instance) {
    furi_check(instance);
    furi_check(!instance->is_running);

    free(instance);
}

FetchStatus fetch_run(Fetch* instance, const FetchRequest* request) {
    furi_check(instance);
    furi_check(!instance->is_running);

    furi_check(request);
    furi_check(request->url);
    furi_check(request->headers.count <= FETCH_HEADERS_COUNT_MAX);

    if(instance->is_stop_requested) {
        return FetchStatusAborted;
    }

    fetch_reset(instance);

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);

#ifdef FETCH_DEBUG
    mg_log_set(MG_LL_VERBOSE);
#endif

    mg_mgr_init(&instance->mgr);
    mongoose_dns_init(&instance->mgr);

    struct mg_connection* conn =
        mg_http_connect(&instance->mgr, request->url, fetch_mg_handler, instance);

    if(conn != NULL) {
        instance->request = request;
        instance->activity_timer = coarse_timer_create(FETCH_INACTIVITY_TIMEOUT_MS);

        instance->is_running = true;

        while(instance->is_running) {
            if(coarse_timer_is_expired(instance->activity_timer)) {
                fetch_raise_error(instance, "Inactivity timeout");
                conn->is_draining = 1;
                break;
            }

            if(instance->is_stop_requested) {
                FETCH_LOG_I(TAG, "Force stopped");
                conn->is_draining = 1;
                FETCH_LOG_I(TAG, "Connection closed");
            }

            mg_mgr_poll(&instance->mgr, 1000);
        }

        if(!fetch_verify_response_body_size(instance)) {
            fetch_raise_error(instance, "Incomplete response body");
        }

    } else {
        fetch_raise_error(instance, "Failed to connect to server");
    }

    mongoose_dns_deinit(&instance->mgr);
    mg_mgr_free(&instance->mgr);

    network_deinit_current_thread(network);
    furi_record_close(RECORD_NETWORK);

    return fetch_get_status(instance);
}

void fetch_stop(Fetch* instance) {
    furi_check(instance);
    instance->is_stop_requested = true;
}

void fetch_set_callback_context(Fetch* instance, void* context) {
    furi_check(instance);
    instance->callback_context = context;
}

void fetch_set_rx_data_callback(Fetch* instance, FetchRxDataCallback callback) {
    furi_check(instance);
    instance->rx_data_callback = callback;
}

void fetch_set_header_callback(Fetch* instance, FetchHeaderCallback callback) {
    furi_check(instance);
    instance->header_callback = callback;
}

void fetch_set_error_callback(Fetch* instance, FetchErrorCallback callback) {
    furi_check(instance);
    instance->error_callback = callback;
}

void fetch_set_progress_callback(Fetch* instance, FetchProgressCallback callback) {
    furi_check(instance);
    instance->progress_callback = callback;
}
