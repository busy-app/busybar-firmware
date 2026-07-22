#include "log_storage_common_i.h"

#include <intercom/intercom.h>

#define TAG "LogStorage"

#define LOG_STORAGE_INTERCOM_TX_TIMEOUT_MS 1500u

typedef struct {
    LogStorageBase base;
    IntercomChannel* channel;
    FuriThreadId* thread_id;
} LogStorage;

typedef enum {
    LogStorageThreadFlagDumpRequest = 1 << 0,
} LogStorageThreadFlag;

static void log_storage_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data_size == sizeof(LogStorageBaseIntercomRequestType));

    LogStorage* instance = context;

    LogStorageBaseIntercomRequestType request;
    memcpy(&request, data, sizeof(request));

    if(request == LogStorageBaseIntercomRequestDump) {
        furi_thread_flags_set(instance->thread_id, LogStorageThreadFlagDumpRequest);
    } else {
        FURI_LOG_E(TAG, "Unknown request type received: %u", request);
    }
}

static void log_storage_send_dump(IntercomChannel* channel, const LogStorageSnapshot* snapshot) {
    LogStorageBaseIntercomLengthType dump_length = 0;
    for(size_t i = 0; i < LOG_STORAGE_SNAPSHOT_CHUNKS_COUNT; i++) {
        dump_length += snapshot->chunks[i].length;
    }

    if(intercom_tx(
           channel, &dump_length, sizeof(dump_length), LOG_STORAGE_INTERCOM_TX_TIMEOUT_MS) !=
       sizeof(dump_length)) {
        FURI_LOG_E(TAG, "Failed to send dump length");
        return;
    }

    for(size_t i = 0; i < LOG_STORAGE_SNAPSHOT_CHUNKS_COUNT; i++) {
        const LogStorageSnapshotChunk* chunk = &snapshot->chunks[i];
        if(chunk->length == 0) continue;

        if(intercom_tx(channel, chunk->data, chunk->length, LOG_STORAGE_INTERCOM_TX_TIMEOUT_MS) !=
           chunk->length) {
            FURI_LOG_E(TAG, "Failed to send log chunk");
            return;
        }
    }
}

static void log_storage_handle_request(LogStorage* instance) {
    LogStorageSnapshot snapshot;
    if(log_storage_base_snapshot_take(&instance->base, &snapshot)) {
        FURI_LOG_D(TAG, "Log dump request handling started");

        log_storage_send_dump(instance->channel, &snapshot);
        log_storage_base_snapshot_release(&instance->base);
    } else {
        FURI_LOG_E(TAG, "Failed to acquire log snapshot");

        uint32_t dump_length = 0;
        intercom_tx(
            instance->channel,
            &dump_length,
            sizeof(dump_length),
            LOG_STORAGE_INTERCOM_TX_TIMEOUT_MS);
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
        uint32_t flags = furi_thread_flags_wait(
            LogStorageThreadFlagDumpRequest, FuriFlagWaitAny, FuriWaitForever);

        furi_check((flags & FuriFlagError) == 0);

        if(flags & LogStorageThreadFlagDumpRequest) {
            log_storage_handle_request(instance);
        }
    }

    return 0;
}
