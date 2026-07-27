#include "log_storage_i.h"

#ifdef SRV_INTERCOM
#include <intercom/intercom.h>
#include <intercom/intercom_frame.h>
#endif /* SRV_INTERCOM */

#define LOG_STORAGE_INTERCOM_TX_TIMEOUT_MS    500u
#define LOG_STORAGE_INTERCOM_RX_TIMEOUT_MS    100u
#define LOG_STORAGE_INTERCOM_FETCH_PERIOD_MS  (60u * 1000u)
#define LOG_STORAGE_INTERCOM_BUFFER_SIZE      (8u * 1024u)
#define LOG_STORAGE_INTERCOM_WRITE_CHUNK_SIZE 256u

#define LOG_STORAGE_INTERCOM_LINK_DOWN_HEADER   "<SiWG917 link down - cached data>\r\n"
#define LOG_STORAGE_INTERCOM_UNAVAILABLE_HEADER "<SiWG917 link not available>\r\n"

#ifdef SRV_INTERCOM
struct LogStorageIntercom {
    FuriEventLoop* event_loop;

    IntercomChannel* channel;
    FuriStreamBuffer* rx_stream;
    FuriEventLoopTimer* fetch_timer;

    uint8_t* remote_log_buffer;
    size_t remote_log_length;

    _Atomic bool is_intercom_link_up;
    _Atomic bool is_remote_fetch_active;
};

static void log_storage_intercom_fetch_to_buffer(LogStorageIntercom* log_intercom);

static void log_storage_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    LogStorageIntercom* log_intercom = context;

    if(log_intercom->is_remote_fetch_active) {
        furi_stream_buffer_send(
            log_intercom->rx_stream, data, data_size, LOG_STORAGE_INTERCOM_RX_TIMEOUT_MS);
    }
}

static void log_storage_intercom_state_callback(const void* item, void* context) {
    LogStorageIntercom* log_intercom = context;
    IntercomStatus status = *(const IntercomStatus*)item;

    if(status != IntercomStatusUnknown) {
        uint32_t event;
        if(status == IntercomStatusOk) {
            event = LogStorageEventIntercomLinkUp;
            log_intercom->is_intercom_link_up = true;
        } else {
            event = LogStorageEventIntercomLinkDown;
            log_intercom->is_intercom_link_up = false;
        }

        furi_event_loop_set_custom_event(log_intercom->event_loop, event);
    }
}

static void log_storage_intercom_timer_callback(void* context) {
    LogStorageIntercom* log_intercom = context;

    if(log_intercom->is_intercom_link_up) {
        FURI_LOG_D(TAG, "Starting remote log caching...");

        log_intercom->is_remote_fetch_active = true;
        log_storage_intercom_fetch_to_buffer(log_intercom);
        log_intercom->is_remote_fetch_active = false;
    }
}

static bool
    log_storage_intercom_send_dump_request(LogStorageIntercom* log_intercom, size_t length) {
    LogStorageBaseIntercomRequest request = {
        .length = length,
    };

    if(intercom_tx(
           log_intercom->channel, &request, sizeof(request), LOG_STORAGE_INTERCOM_TX_TIMEOUT_MS) !=
       sizeof(request)) {
        FURI_LOG_W(TAG, "Failed to send remote dump request");
        return false;
    }

    return true;
}

static bool log_storage_intercom_receive_dump_length(
    LogStorageIntercom* log_intercom,
    size_t* dump_length) {
    LogStorageBaseIntercomResponseHeader response_header;
    if(furi_stream_buffer_receive(
           log_intercom->rx_stream,
           &response_header,
           sizeof(response_header),
           LOG_STORAGE_INTERCOM_TX_TIMEOUT_MS) != sizeof(response_header)) {
        FURI_LOG_W(TAG, "Remote dump length receive timed out");
        return false;
    }

    *dump_length = response_header.length;
    return true;
}

static size_t log_storage_intercom_receive_dump_data(
    LogStorageIntercom* log_intercom,
    void* data,
    size_t length) {
    return furi_stream_buffer_receive(
        log_intercom->rx_stream, data, length, LOG_STORAGE_INTERCOM_TX_TIMEOUT_MS);
}

static void log_storage_intercom_fetch_to_buffer(LogStorageIntercom* log_intercom) {
    furi_stream_buffer_reset(log_intercom->rx_stream);

    if(!log_storage_intercom_send_dump_request(log_intercom, LOG_STORAGE_INTERCOM_BUFFER_SIZE)) {
        return;
    }

    size_t dump_length;
    if(!log_storage_intercom_receive_dump_length(log_intercom, &dump_length)) {
        return;
    }

    log_intercom->remote_log_length = 0;
    while(dump_length > 0) {
        size_t received_chunk_length = log_storage_intercom_receive_dump_data(
            log_intercom,
            log_intercom->remote_log_buffer + log_intercom->remote_log_length,
            MIN(LOG_STORAGE_INTERCOM_WRITE_CHUNK_SIZE, dump_length));

        if(received_chunk_length == 0) {
            FURI_LOG_W(TAG, "Remote log stream timed out");
            break;
        }

        log_intercom->remote_log_length += received_chunk_length;
        dump_length -= received_chunk_length;
    }
}

static bool log_storage_intercom_dump_to_file(LogStorageIntercom* log_intercom, File* file) {
    furi_stream_buffer_reset(log_intercom->rx_stream);

    if(!log_storage_intercom_send_dump_request(log_intercom, UINT32_MAX)) {
        return false;
    }

    size_t dump_length;
    if(!log_storage_intercom_receive_dump_length(log_intercom, &dump_length)) {
        return false;
    }

    uint8_t buffer[LOG_STORAGE_INTERCOM_WRITE_CHUNK_SIZE];
    while(dump_length > 0) {
        size_t received_chunk_length = log_storage_intercom_receive_dump_data(
            log_intercom, buffer, MIN(sizeof(buffer), dump_length));

        if(received_chunk_length == 0) {
            FURI_LOG_W(TAG, "Remote log stream timed out");
            return false;
        }

        if(storage_file_write(file, buffer, received_chunk_length) != received_chunk_length) {
            FURI_LOG_W(TAG, "Failed to write log data to file");
            return false;
        }

        dump_length -= received_chunk_length;
    }

    return true;
}

static bool log_storage_intercom_write_cached(LogStorageIntercom* log_intercom, File* file) {
    size_t length = strlen(LOG_STORAGE_INTERCOM_LINK_DOWN_HEADER);
    if(storage_file_write(file, LOG_STORAGE_INTERCOM_LINK_DOWN_HEADER, length) != length) {
        FURI_LOG_W(TAG, "Failed to write log data to file");
        return false;
    }

    if(storage_file_write(file, log_intercom->remote_log_buffer, log_intercom->remote_log_length) !=
       log_intercom->remote_log_length) {
        FURI_LOG_W(TAG, "Failed to write log data to file");
        return false;
    }

    return true;
}
#endif /* SRV_INTERCOM */

LogStorageIntercom* log_storage_intercom_init(FuriEventLoop* event_loop) {
#ifdef SRV_INTERCOM
    LogStorageIntercom* log_intercom = malloc(sizeof(LogStorageIntercom));

    log_intercom->event_loop = event_loop;
    log_intercom->rx_stream = furi_stream_buffer_alloc(INTERCOM_FRAME_DATA_SIZE, 1);
    log_intercom->remote_log_buffer = malloc(LOG_STORAGE_INTERCOM_BUFFER_SIZE);
    log_intercom->remote_log_length = 0;
    log_intercom->is_intercom_link_up = false;
    log_intercom->is_remote_fetch_active = false;

    log_intercom->fetch_timer = furi_event_loop_timer_alloc(
        log_intercom->event_loop,
        log_storage_intercom_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        log_intercom);

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    furi_state_subscribe(
        intercom_get_state(intercom), log_storage_intercom_state_callback, log_intercom);
    log_intercom->channel = intercom_channel_open(
        intercom, IntercomChannelIdLogDump, log_storage_intercom_rx_callback, log_intercom);

    return log_intercom;
#else /* SRV_INTERCOM */
    UNUSED(event_loop);

    return NULL;
#endif /* SRV_INTERCOM */
}

void log_storage_intercom_on_event(LogStorageIntercom* log_intercom, uint32_t events) {
#ifdef SRV_INTERCOM
    if(events & LogStorageEventIntercomLinkUp) {
        furi_event_loop_timer_start(
            log_intercom->fetch_timer, furi_ms_to_ticks(LOG_STORAGE_INTERCOM_FETCH_PERIOD_MS));
    }

    if(events & LogStorageEventIntercomLinkDown) {
        furi_event_loop_timer_stop(log_intercom->fetch_timer);
    }
#else /* SRV_INTERCOM */
    UNUSED(log_intercom);
    UNUSED(events);
#endif /* SRV_INTERCOM */
}

bool log_storage_intercom_remote_dump(LogStorageIntercom* log_intercom, File* file) {
#ifdef SRV_INTERCOM
    if(!log_intercom->is_intercom_link_up) {
        return log_storage_intercom_write_cached(log_intercom, file);
    }

    uint64_t file_offset = storage_file_tell(file);

    log_intercom->is_remote_fetch_active = true;
    bool is_fetch_successful = log_storage_intercom_dump_to_file(log_intercom, file);
    log_intercom->is_remote_fetch_active = false;

    if(is_fetch_successful) return true;

    if(!storage_file_seek(file, file_offset, true)) {
        FURI_LOG_W(TAG, "Failed to seek to remote section");
        return false;
    }

    bool is_write_successful = log_storage_intercom_write_cached(log_intercom, file);

    if(!storage_file_truncate(file)) {
        FURI_LOG_W(TAG, "Failed to truncate remote section");
        return false;
    }

    return is_write_successful;
#else /* SRV_INTERCOM */
    UNUSED(log_intercom);

    size_t length = strlen(LOG_STORAGE_INTERCOM_UNAVAILABLE_HEADER);
    if(storage_file_write(file, LOG_STORAGE_INTERCOM_UNAVAILABLE_HEADER, length) != length) {
        FURI_LOG_W(TAG, "Failed to write log data to file");
        return false;
    }

    return true;
#endif /* SRV_INTERCOM */
}
