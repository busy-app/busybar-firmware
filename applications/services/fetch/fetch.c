#include "fetch.h"
#include "fetch_chunked.h"

#include <network/network.h>
#include <toolbox/timers.h>
#include <toolbox/memory_pool.h>

#include <mongoose_dns.h>
#include <mongoose_tls.h>

#include <furi/furi.h>
#include <api_lock.h>

#define TAG "Fetch"

#define FETCH_MAX_JOBS 16

#define FETCH_INACTIVITY_TIMEOUT_MS (5 * 1000)

#define FETCH_USER_AGENT                         \
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) " \
    "AppleWebKit/537.36 (KHTML, like Gecko) "    \
    "Chrome/138.0.0.0 Safari/537.36"

#ifdef FETCH_DEBUG
#define FETCH_LOG_D(...) FURI_LOG_D(__VA_ARGS__)
#define FETCH_LOG_I(...) FURI_LOG_I(__VA_ARGS__)
#define FETCH_LOG_E(...) FURI_LOG_E(__VA_ARGS__)
#else
#define FETCH_LOG_D(...)
#define FETCH_LOG_I(...)
#define FETCH_LOG_E(...)
#endif

#define FETCH_JOB_EVENT_FLAG_DONE (1 << 0)

#define HTTP_STATUS_INFORMATIONAL_MIN (100)
#define HTTP_STATUS_INFORMATIONAL_MAX (199)
#define HTTP_STATUS_NO_CONTENT        (204)
#define HTTP_STATUS_NOT_MODIFIED      (304)

typedef enum {
    FetchTransferEncodingNone,
    FetchTransferEncodingChunked,
    FetchTransferEncodingUnsupported,
} FetchTransferEncoding;

typedef enum FetchJobState {
    FetchJobStateFree = 0,
    FetchJobStateScheduled,
    FetchJobStateRunning,
    FetchJobStateDone,
} FetchJobState;

typedef struct FetchJobSlot FetchJobSlot;

struct Fetch {
    Network* network;

    FuriMutex* jobs_mutex;
    FuriSemaphore* jobs_semaphore;

    MemoryPool* jobs_pool;
    FetchJobSlot* jobs_head;
    FetchJobSlot* jobs_tail;
};

typedef struct FetchJob {
    FetchJobState state;
    FuriEventFlag* event_flag_done;

    const FetchRequest* request;
    FetchProgress progress;

    FetchChunked* chunked;

    uint32_t started_raw_ticks;
    uint32_t started_download_ticks;
    size_t delta_received_bytes;
    uint32_t count_receive_packets;
    CoarseTimer activity_timer;

    FetchCallbacks callbacks;

    bool is_running;
    bool is_error_occurred;
    _Atomic bool is_stop_requested;

    FetchStatus status;
} FetchJob;

struct FetchHandle {
    Fetch* instance;

    FetchJobSlot* job_slot;
};

struct FetchJobSlot {
    FetchJobSlot* next;
    FetchJobSlot* prev;

    FetchJob job;
    FetchHandle handle;
};

// ============================
// Stateless internal functions
// ============================
static void fetch_on_data(SizedBufferConst data, void* context);
static void fetch_on_data_error(void* context);

static bool fetch_is_expecting_headers(const struct mg_connection* conn) {
    return conn->pfn != NULL;
}

static uint32_t fetch_calc_download_speed(size_t size_delta, uint32_t start_timestamp_ticks) {
    const uint32_t delta_ticks = furi_get_tick() - start_timestamp_ticks + 1;
    const float delta_s = (float)delta_ticks / furi_kernel_get_tick_frequency();
    return size_delta / delta_s;
}

// ===========================
// Stateful internal functions
// ===========================

static FetchJobSlot* fetch_alloc_job(Fetch* instance) {
    furi_check(furi_mutex_acquire(instance->jobs_mutex, FuriWaitForever) == FuriStatusOk);
    FetchJobSlot* slot = memory_pool_alloc_object(instance->jobs_pool);
    if(slot != NULL) {
        FetchJob* job = &slot->job;
        memset(job, 0, sizeof(FetchJob));
        job->state = FetchJobStateFree;
        job->event_flag_done = furi_event_flag_alloc();

        FetchJobSlot* prev = instance->jobs_tail;
        slot->next = NULL;
        slot->prev = prev;
        if(prev) {
            prev->next = slot;
        }
        instance->jobs_tail = slot;
        if(instance->jobs_head == NULL) {
            instance->jobs_head = slot;
        }
    }
    furi_mutex_release(instance->jobs_mutex);
    return slot;
}

static void fetch_free_job(Fetch* instance, FetchJobSlot* slot) {
    FetchJob* job = &slot->job;
    furi_check(furi_mutex_acquire(instance->jobs_mutex, FuriWaitForever) == FuriStatusOk);

    FetchJobSlot* prev = slot->prev;
    FetchJobSlot* next = slot->next;

    if(prev) {
        prev->next = next;
    }
    if(next) {
        next->prev = prev;
    }
    if(instance->jobs_head == slot) {
        instance->jobs_head = next;
    }
    if(instance->jobs_tail == slot) {
        instance->jobs_tail = prev;
    }

    furi_event_flag_free(job->event_flag_done);
    if(job->chunked) {
        fetch_chunked_free(job->chunked);
    }
    memory_pool_free_object(instance->jobs_pool, slot);
    furi_mutex_release(instance->jobs_mutex);
}

static FetchJobSlot* fetch_take_job(Fetch* instance) {
    FetchJobSlot* result = NULL;
    furi_check(furi_semaphore_acquire(instance->jobs_semaphore, FuriWaitForever) == FuriStatusOk);
    furi_check(furi_mutex_acquire(instance->jobs_mutex, FuriWaitForever) == FuriStatusOk);
    for(FetchJobSlot* slot = instance->jobs_head; slot; slot = slot->next) {
        FetchJob* job = &slot->job;
        if(job->state == FetchJobStateScheduled) {
            result = slot;
            job->state = FetchJobStateRunning;
            break;
        }
    }
    furi_mutex_release(instance->jobs_mutex);
    return result;
}

static void fetch_schedule_job(Fetch* instance, FetchJobSlot* slot) {
    furi_check(furi_mutex_acquire(instance->jobs_mutex, FuriWaitForever) == FuriStatusOk);
    furi_check(slot->job.state == FetchJobStateFree);
    slot->job.state = FetchJobStateScheduled;
    furi_mutex_release(instance->jobs_mutex);
    furi_check(furi_semaphore_release(instance->jobs_semaphore) == FuriStatusOk);
}

static bool fetch_is_rx_complete(const FetchJob* job) {
    bool is_complete = false;

    const FetchProgress* progress = &job->progress;

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

static void fetch_raise_error(FetchJob* job, const char* error_message) {
    FETCH_LOG_E(TAG, "Error: %s", error_message);

    if(job->callbacks.error_callback) {
        job->callbacks.error_callback(error_message, job->callbacks.callback_context);
    }

    job->is_error_occurred = true;
}

static void fetch_set_total_download_size(FetchJob* job, size_t size) {
    FetchProgress* progress = &job->progress;
    progress->total_download_size = size;
    progress->has_total_download_size = true;
}

static bool fetch_is_bodyless_method(const FetchRequest* request) {
    bool is_bodyless = false;

    const char* method = request->method;
    if((method != NULL) && (strcmp(method, "HEAD") == 0)) {
        is_bodyless = true;
    }

    return is_bodyless;
}

static bool fetch_is_bodyless_response(const struct mg_http_message* msg) {
    bool is_bodyless = false;

    const int status = mg_http_status(msg);
    // RFC 9112 6.1 Responses with the following
    // status codes are not expected to have a body
    if(status == HTTP_STATUS_NO_CONTENT || status == HTTP_STATUS_NOT_MODIFIED) {
        is_bodyless = true;
    }

    return is_bodyless;
}

static bool fetch_is_informational_response(const struct mg_http_message* msg) {
    const int status = mg_http_status(msg);
    return (status >= HTTP_STATUS_INFORMATIONAL_MIN) && (status <= HTTP_STATUS_INFORMATIONAL_MAX);
}

static FetchTransferEncoding fetch_get_transfer_encoding(const struct mg_http_message* msg) {
    FetchTransferEncoding encoding = FetchTransferEncodingNone;

    const struct mg_str* transfer_encoding =
        mg_http_get_header((struct mg_http_message*)msg, "transfer-encoding");

    if(transfer_encoding != NULL) {
        if(mg_strcasecmp(*transfer_encoding, mg_str("chunked")) == 0) {
            encoding = FetchTransferEncodingChunked;
        } else {
            encoding = FetchTransferEncodingUnsupported;
        }
    }

    return encoding;
}

static bool fetch_setup_body_framing(FetchJob* job, const struct mg_http_message* msg) {
    bool success = true;

    const bool is_bodyless = fetch_is_bodyless_method(job->request) ||
                             fetch_is_bodyless_response(msg);
    if(is_bodyless) {
        // RFC 9112 6.1 forbids sending Transfer-Encoding with status code 204.
        // This implementation allows such responses for simplicity.
        fetch_set_total_download_size(job, 0);
    } else {
        const FetchTransferEncoding transfer_encoding = fetch_get_transfer_encoding(msg);

        if(transfer_encoding == FetchTransferEncodingNone) {
            const int32_t content_length = msg->body.len;
            if(content_length != -1) {
                fetch_set_total_download_size(job, content_length);
            }
        } else if(transfer_encoding == FetchTransferEncodingChunked) {
            furi_assert(job->chunked == NULL);
            job->chunked = fetch_chunked_alloc(fetch_on_data, fetch_on_data_error, job);
            FETCH_LOG_I(TAG, "chunked");
        } else {
            fetch_raise_error(job, "Unsupported Transfer-Encoding");
            success = false;
        }
    }

    return success;
}

static void fetch_discard_response(struct mg_connection* conn) {
    fetch_consume_rx_data(conn, conn->recv.len);
    conn->is_draining = 1;
}

static void fetch_switch_to_raw_protocol(
    FetchJob* job,
    struct mg_connection* conn,
    const struct mg_http_message* msg) {
    job->started_raw_ticks = furi_get_tick();
    job->started_download_ticks = job->started_raw_ticks;

    if(fetch_setup_body_framing(job, msg)) {
        fetch_consume_rx_data(conn, msg->head.len);
    } else {
        fetch_discard_response(conn);
    }
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

    mg_printf(conn, "%s %s HTTP/1.1\r\n", method, uri);
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

static FURI_ALWAYS_INLINE void fetch_connect_event(FetchJob* job, struct mg_connection* conn) {
    const FetchRequest* request = job->request;
    furi_assert(request);

    if(fetch_init_connection(request, conn)) {
        fetch_send_request_method(request, conn);
        fetch_send_request_headers(request, conn);
        fetch_send_extra_request_headers(request, conn);
        fetch_send_request_body(request, conn);

    } else {
        fetch_raise_error(job, "Failed to establish TLS connection");
        conn->is_draining = 1;
    }
}

static FURI_ALWAYS_INLINE void
    fetch_http_hdrs_event(FetchJob* job, struct mg_connection* conn, void* ev_data) {
    const struct mg_http_message* msg = ev_data;

    FETCH_LOG_I(TAG, "MG_EV_HTTP_HDRS: %.*s", msg->message.len, msg->message.buf);
    FETCH_LOG_I(TAG, "Path: %.*s", msg->uri.len, msg->uri.buf);

    if(fetch_is_informational_response(msg)) {
        // RFC 9110 15.2: Informational responses (1xx) are not supported
        fetch_raise_error(job, "Unsupported informational response");
        fetch_discard_response(conn);

    } else {
        if(job->callbacks.header_callback) {
            job->callbacks.header_callback(
                msg->head.buf, msg->head.len, job->callbacks.callback_context);
        }

        fetch_switch_to_raw_protocol(job, conn, msg);
    }

    /* Mongoose will detach the protocol handler function automatically
     * after the received data has been altered in the header callback.
     * Here it is done preemptively for good measure. */
    conn->pfn = NULL;
}

static void fetch_on_data(SizedBufferConst data, void* context) {
    FetchJob* job = context;

    job->progress.received_download_size += data.size;
    job->delta_received_bytes += data.size;

    if(job->callbacks.rx_data_callback) {
        job->callbacks.rx_data_callback(data.buffer, data.size, job->callbacks.callback_context);
    }
}

static void fetch_on_data_error(void* context) {
    FetchJob* job = context;
    fetch_raise_error(job, "Chunked transfer error");
}

static bool fetch_process_packet(FetchJob* job, SizedBufferConst packet) {
    if(job->chunked) {
        return fetch_chunked_process(job->chunked, packet);
    } else {
        fetch_on_data(packet, job);
        return fetch_is_rx_complete(job);
    }
}

static FURI_ALWAYS_INLINE void fetch_read_event(FetchJob* job, struct mg_connection* conn) {
    FETCH_LOG_I(TAG, "MG_EV_READ");

    if(fetch_is_expecting_headers(conn)) {
        // Ignore read events before having processed the headers
        return;
    }

    struct mg_iobuf* recv = &conn->recv;
    const size_t recv_len = recv->len;

    FETCH_LOG_I(TAG, "Received %zu bytes", recv_len);

    if(job->is_error_occurred) {
        fetch_consume_rx_data(conn, recv_len);
        return;
    }

    bool is_complete = fetch_process_packet(
        job,
        (SizedBufferConst){
            .buffer = recv->buf,
            .size = recv_len,
        });

    job->activity_timer = coarse_timer_create(FETCH_INACTIVITY_TIMEOUT_MS);
    job->count_receive_packets++;
    if((job->count_receive_packets % (12 * 8)) == 0) {
        job->progress.speed_bytes_per_sec =
            fetch_calc_download_speed(job->delta_received_bytes, job->started_raw_ticks);

        job->started_raw_ticks = furi_get_tick();

        if(job->callbacks.progress_callback) {
            job->callbacks.progress_callback(&job->progress, job->callbacks.callback_context);
        }

        job->delta_received_bytes = 0;
    }

    fetch_consume_rx_data(conn, recv_len);

    if(is_complete) {
        conn->is_draining = 1;
    }
}

static FURI_ALWAYS_INLINE void fetch_close_event(FetchJob* job, struct mg_connection* conn) {
    FETCH_LOG_I(TAG, "MG_EV_CLOSE");

    if(fetch_is_expecting_headers(conn)) {
        /* Mongoose closes the connection upon receiving a malformed
        * response header without emitting the MG_EV_ERROR event. */
        if(!job->is_error_occurred && !job->is_stop_requested) {
            fetch_raise_error(job, "Invalid or missing response header");
        }
    }

    job->progress.speed_bytes_per_sec = fetch_calc_download_speed(
        job->progress.received_download_size, job->started_download_ticks);

    if(job->callbacks.progress_callback) {
        job->callbacks.progress_callback(&job->progress, job->callbacks.callback_context);
    }

    conn->is_draining = 1;
    job->is_running = false;
}

static FURI_ALWAYS_INLINE void
    fetch_error_event(FetchJob* job, struct mg_connection* conn, const void* ev_data) {
    UNUSED(conn);

    const char* error_msg;

    if(ev_data != NULL) {
        error_msg = ev_data;
    } else {
        error_msg = "Unknown error";
    }

    fetch_raise_error(job, error_msg);
    job->is_running = false;
}

static void fetch_mg_handler(struct mg_connection* conn, int event, void* ev_data) {
    FetchJob* job = conn->fn_data;

    if(event == MG_EV_CONNECT) {
        fetch_connect_event(job, conn);
    } else if(event == MG_EV_HTTP_HDRS) {
        fetch_http_hdrs_event(job, conn, ev_data);
    } else if(event == MG_EV_HTTP_MSG) {
        FURI_LOG_W(TAG, "BUG: MG_EV_HTTP_MSG should never happen");
    } else if(event == MG_EV_READ) {
        fetch_read_event(job, conn);
    } else if(event == MG_EV_CLOSE) {
        fetch_close_event(job, conn);
    } else if(event == MG_EV_ERROR) {
        fetch_error_event(job, conn, ev_data);
    } else if(event == MG_EV_TLS_HS) {
        FETCH_LOG_I(TAG, "TLS handshake successful");
    }
}

static FetchStatus fetch_get_status(const FetchJob* job) {
    FetchStatus status;

    if(job->is_error_occurred) {
        status = FetchStatusError;
    } else if(job->is_stop_requested) {
        status = FetchStatusAborted;
    } else {
        status = FetchStatusOk;
    }

    return status;
}

static bool fetch_verify_response_body_size(FetchJob* job) {
    bool success = true;

    if(!(job->is_stop_requested || job->is_error_occurred)) {
        if(job->chunked) {
            success = fetch_chunked_is_complete(job->chunked);
        } else {
            const FetchProgress* progress = &job->progress;

            if(progress->has_total_download_size) {
                const size_t expected_size = progress->total_download_size;
                const size_t actual_size = progress->received_download_size;

                if(expected_size != actual_size) {
                    success = false;
                }
            }
        }
    }

    return success;
}

static Fetch* fetch_alloc(void) {
    Fetch* instance = malloc(sizeof(Fetch));

    instance->jobs_pool = memory_pool_alloc(sizeof(FetchJobSlot), FETCH_MAX_JOBS);
    instance->jobs_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->jobs_semaphore = furi_semaphore_alloc(FETCH_MAX_JOBS, 0);
    instance->jobs_head = NULL;
    instance->jobs_tail = NULL;

    instance->network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(instance->network);

    return instance;
}

static void fetch_process_job(FetchJob* job) {
    if(job->is_stop_requested) {
        job->status = FetchStatusAborted;
        return;
    }

    const FetchRequest* request = job->request;
#ifdef FETCH_DEBUG
    mg_log_set(MG_LL_VERBOSE);
#endif

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mongoose_dns_init(&mgr);
    FETCH_LOG_D(TAG, "job: %s", request->url);

    struct mg_connection* conn = mg_http_connect(&mgr, request->url, fetch_mg_handler, job);

    if(conn != NULL) {
        job->activity_timer = coarse_timer_create(FETCH_INACTIVITY_TIMEOUT_MS);

        job->is_running = true;

        while(job->is_running) {
            if(coarse_timer_is_expired(job->activity_timer)) {
                fetch_raise_error(job, "Inactivity timeout");
                conn->is_draining = 1;
                break;
            }

            if(job->is_stop_requested) {
                FETCH_LOG_I(TAG, "Force stopped");
                conn->is_draining = 1;
                FETCH_LOG_I(TAG, "Connection closed");
            }

            mg_mgr_poll(&mgr, 1000);
        }

        if(!fetch_verify_response_body_size(job)) {
            fetch_raise_error(job, "Incomplete response body");
        }

    } else {
        fetch_raise_error(job, "Failed to connect to server");
    }

    mongoose_dns_deinit(&mgr);
    mg_mgr_free(&mgr);

    if(job->chunked) {
        fetch_chunked_free(job->chunked);
        job->chunked = NULL;
    }
    job->status = fetch_get_status(job);
}

static void fetch_loop(Fetch* instance) {
    while(true) {
        FetchJobSlot* slot = fetch_take_job(instance);
        if(!slot) {
            FETCH_LOG_D(TAG, "no job");
            continue;
        }
        FetchJob* job = &slot->job;
        FETCH_LOG_D(TAG, "got job");
        fetch_process_job(job);
        furi_check(furi_mutex_acquire(instance->jobs_mutex, FuriWaitForever) == FuriStatusOk);
        job->state = FetchJobStateDone;
        furi_event_flag_set(job->event_flag_done, FETCH_JOB_EVENT_FLAG_DONE);
        furi_mutex_release(instance->jobs_mutex);
        if(job->callbacks.done_callback) {
            job->callbacks.done_callback(&slot->handle, job->callbacks.callback_context);
        }
    }
}

// ==========
// Public API
// ==========

FetchHandle*
    fetch_init(Fetch* instance, const FetchRequest* request, const FetchCallbacks* callbacks) {
    furi_check(instance);

    furi_check(request);
    furi_check(request->url);
    furi_check(request->headers.count <= FETCH_HEADERS_COUNT_MAX);

    FETCH_LOG_D(TAG, "run %s", request->url);

    FetchJobSlot* job_slot = fetch_alloc_job(instance);

    if(!job_slot) {
        FETCH_LOG_D(TAG, "no slots");
        return NULL;
    }

    FetchJob* job = &job_slot->job;

    job->request = request;

    memset(&job->progress, 0, sizeof(FetchProgress));
    job->delta_received_bytes = 0;
    job->count_receive_packets = 0;
    job->is_error_occurred = false;
    job->is_stop_requested = false;

    job->callbacks = *callbacks;

    FetchHandle* handle = &job_slot->handle;
    handle->instance = instance;
    handle->job_slot = job_slot;

    return handle;
}

void fetch_start(FetchHandle* handle) {
    furi_check(handle);

    Fetch* instance = handle->instance;
    fetch_schedule_job(instance, handle->job_slot);
}

FetchStatus fetch_join(FetchHandle* handle, uint32_t timeout) {
    furi_check(handle);
    FETCH_LOG_D(TAG, "join %p", handle);

    Fetch* instance = handle->instance;
    FetchJob* job = &handle->job_slot->job;

    uint32_t status = furi_event_flag_wait(
        job->event_flag_done, FETCH_JOB_EVENT_FLAG_DONE, FuriFlagWaitAny, timeout);
    if(status == (uint32_t)FuriStatusErrorTimeout) {
        return FetchStatusTimeout;
    } else if(timeout == 0 && status == (uint32_t)FuriStatusErrorResource) {
        return FetchStatusTimeout;
    }

    furi_check(status == FETCH_JOB_EVENT_FLAG_DONE);

    FetchStatus job_status = job->status;

    fetch_free_job(instance, handle->job_slot);

    return job_status;
}

void fetch_stop(FetchHandle* handle) {
    furi_check(handle);
    FETCH_LOG_D(TAG, "stop %p", handle);

    Fetch* instance = handle->instance;
    FetchJob* job = &handle->job_slot->job;

    furi_check(furi_mutex_acquire(instance->jobs_mutex, FuriWaitForever) == FuriStatusOk);
    furi_check(job->state != FetchJobStateFree);
    if(job->state != FetchJobStateDone) {
        job->is_stop_requested = true;
    }
    furi_mutex_release(instance->jobs_mutex);
}

int32_t fetch_srv(void* p) {
    UNUSED(p);
    FURI_LOG_I(TAG, "Service startup");
    Fetch* instance = fetch_alloc();
    furi_record_create(RECORD_FETCH, instance);
    fetch_loop(instance);

    return 0;
}
