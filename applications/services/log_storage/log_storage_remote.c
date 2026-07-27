#include "log_storage_common_i.h"

#include <intercom/intercom.h>

#define TAG "LogStorage"

#define LOG_STORAGE_INTERCOM_TX_TIMEOUT_MS 500u

typedef struct {
    LogStorageBase base;
    IntercomChannel* channel;
    FuriThreadId* thread_id;

    LogStorageBaseIntercomRequest request;
} LogStorage;

typedef enum {
    LogStorageThreadFlagRequest = 1 << 0,
} LogStorageThreadFlag;

static void log_storage_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data_size == sizeof(LogStorageBaseIntercomRequest));

    LogStorage* instance = context;
    memcpy(&instance->request, data, sizeof(instance->request));

    furi_thread_flags_set(instance->thread_id, LogStorageThreadFlagRequest);
}

static bool log_storage_send_dump_header(IntercomChannel* channel, size_t length) {
    LogStorageBaseIntercomResponseHeader response_header = {
        .length = length,
    };

    return intercom_tx(
               channel,
               &response_header,
               sizeof(response_header),
               LOG_STORAGE_INTERCOM_TX_TIMEOUT_MS) == sizeof(response_header);
}

static bool log_storage_send_dump_data(IntercomChannel* channel, const void* data, size_t length) {
    return intercom_tx(channel, data, length, LOG_STORAGE_INTERCOM_TX_TIMEOUT_MS) == length;
}

static void log_storage_send_dump(
    IntercomChannel* channel,
    const LogStorageSnapshot* snapshot,
    size_t requested_length) {
    size_t full_length = 0;
    for(size_t i = 0; i < LOG_STORAGE_SNAPSHOT_CHUNKS_COUNT; i++) {
        full_length += snapshot->chunks[i].length;
    }

    size_t effective_length = MIN(full_length, requested_length);
    if(!log_storage_send_dump_header(channel, effective_length)) {
        FURI_LOG_E(TAG, "Failed to send dump length");
        return;
    }

    size_t skip_length = full_length - effective_length;
    for(size_t i = 0; i < LOG_STORAGE_SNAPSHOT_CHUNKS_COUNT; i++) {
        const LogStorageSnapshotChunk* chunk = &snapshot->chunks[i];
        if(chunk->length == 0) continue;

        if(skip_length >= chunk->length) {
            skip_length -= chunk->length;
            continue;
        }

        if(!log_storage_send_dump_data(
               channel, chunk->data + skip_length, chunk->length - skip_length)) {
            FURI_LOG_E(TAG, "Failed to send log chunk");
            return;
        }

        skip_length = 0;
    }
}

static void log_storage_handle_request(LogStorage* instance) {
    LogStorageSnapshot snapshot;
    if(log_storage_base_snapshot_take(&instance->base, &snapshot)) {
        FURI_LOG_D(TAG, "Log dump request handling started");

        log_storage_send_dump(instance->channel, &snapshot, instance->request.length);
        log_storage_base_snapshot_release(&instance->base);
    } else {
        FURI_LOG_E(TAG, "Failed to acquire log snapshot");

        log_storage_send_dump_header(instance->channel, 0);
    }
}

int32_t log_storage_srv(void* context) {
    UNUSED(context);

    LogStorage* instance = malloc(sizeof(*instance));
    log_storage_base_init(&instance->base);

    instance->thread_id = furi_thread_get_current_id();

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    instance->channel = intercom_channel_open(
        intercom, IntercomChannelIdLogDump, log_storage_intercom_rx_callback, instance);
    furi_record_close(RECORD_INTERCOM);

    for(;;) {
        uint32_t flags =
            furi_thread_flags_wait(LogStorageThreadFlagRequest, FuriFlagWaitAny, FuriWaitForever);

        furi_check((flags & FuriFlagError) == 0);

        if(flags & LogStorageThreadFlagRequest) {
            log_storage_handle_request(instance);
        }
    }

    return 0;
}
