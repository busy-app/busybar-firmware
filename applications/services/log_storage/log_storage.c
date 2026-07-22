#include "log_storage_common_i.h"
#include "log_storage.h"

#include <storage/storage.h>

#ifdef SRV_INTERCOM
#include <intercom/intercom.h>
#include <intercom/intercom_frame.h>
#endif /* SRV_INTERCOM */

#define TAG "LogStorage"

#define LOG_STORAGE_INTERCOM_TX_TIMEOUT_MS 1500u
#define LOG_STORAGE_INTERCOM_RX_TIMEOUT_MS 150u
#define LOG_STORAGE_FILE_WRITE_CHUNK_SIZE  256u

#define LOG_STORAGE_U5_SECTION_HEADER  "[------ STM32U5 ------]\r\n"
#define LOG_STORAGE_917_SECTION_HEADER "\r\n[------ SiWG917 ------]\r\n"

struct LogStorage {
    LogStorageBase base;

    FuriMutex* dump_lock;

#ifdef SRV_INTERCOM
    IntercomChannel* channel;
    FuriStreamBuffer* rx_stream;

    _Atomic bool is_remote_dump_active;
#endif /* SRV_INTERCOM */
};

#ifdef SRV_INTERCOM
static void log_storage_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    LogStorage* instance = context;

    if(instance->is_remote_dump_active) {
        furi_stream_buffer_send(
            instance->rx_stream, data, data_size, LOG_STORAGE_INTERCOM_RX_TIMEOUT_MS);
    }
}

static bool log_storage_send_dump_request(LogStorage* instance) {
    LogStorageBaseIntercomRequestType request = LogStorageBaseIntercomRequestDump;
    return intercom_tx(
               instance->channel, &request, sizeof(request), LOG_STORAGE_INTERCOM_TX_TIMEOUT_MS) ==
           sizeof(request);
}

static bool log_storage_receive_dump_length(
    LogStorage* instance,
    LogStorageBaseIntercomLengthType* dump_length) {
    return furi_stream_buffer_receive(
               instance->rx_stream,
               dump_length,
               sizeof(*dump_length),
               LOG_STORAGE_INTERCOM_TX_TIMEOUT_MS) == sizeof(*dump_length);
}

static size_t log_storage_receive_dump_data(LogStorage* instance, void* data, size_t max_size) {
    return furi_stream_buffer_receive(
        instance->rx_stream, data, max_size, LOG_STORAGE_INTERCOM_TX_TIMEOUT_MS);
}

static void log_storage_dump_remote(LogStorage* instance, File* file) {
    furi_stream_buffer_reset(instance->rx_stream);

    if(!log_storage_send_dump_request(instance)) {
        FURI_LOG_W(TAG, "Failed to send remote dump request");
        return;
    }

    LogStorageBaseIntercomLengthType dump_length;
    if(!log_storage_receive_dump_length(instance, &dump_length)) {
        FURI_LOG_W(TAG, "Remote dump length receive timed out");
        return;
    }

    uint8_t buffer[LOG_STORAGE_FILE_WRITE_CHUNK_SIZE];
    while(dump_length > 0) {
        size_t received_length =
            log_storage_receive_dump_data(instance, buffer, MIN(sizeof(buffer), dump_length));

        if(received_length == 0) {
            FURI_LOG_W(TAG, "Remote log stream timed out");
            return;
        }

        if(storage_file_write(file, buffer, received_length) != received_length) {
            FURI_LOG_W(TAG, "Failed to write remote log data");
            return;
        }

        dump_length -= received_length;
    }
}
#endif /* SRV_INTERCOM */

static bool log_storage_dump_local(LogStorage* instance, File* file) {
    LogStorageSnapshot snapshot;
    if(!log_storage_base_snapshot_take(&instance->base, &snapshot)) {
        FURI_LOG_E(TAG, "Failed to acquire local log snapshot");
        return false;
    }

    bool is_successful = true;
    for(size_t i = 0; i < LOG_STORAGE_SNAPSHOT_CHUNKS_COUNT; i++) {
        const LogStorageSnapshotChunk* chunk = &snapshot.chunks[i];

        if(chunk->length == 0) continue;
        if(storage_file_write(file, chunk->data, chunk->length) != chunk->length) {
            is_successful = false;
            break;
        }
    }

    log_storage_base_snapshot_release(&instance->base);
    return is_successful;
}

bool log_storage_dump(LogStorage* instance, const char* path) {
    furi_check(instance);

    if(furi_mutex_acquire(instance->dump_lock, 0) != FuriStatusOk) return false;

    const char* target_path = path ?: LOG_STORAGE_DUMP_DEFAULT_FILE_PATH;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    bool is_successful = false;
    do {
        if(!storage_file_open(file, target_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            FURI_LOG_E(TAG, "Failed to open file %s", target_path);
            break;
        }

        do {
            size_t length = strlen(LOG_STORAGE_U5_SECTION_HEADER);
            if(storage_file_write(file, LOG_STORAGE_U5_SECTION_HEADER, length) != length) {
                FURI_LOG_E(TAG, "Failed to write local section header");
                break;
            }

            if(!log_storage_dump_local(instance, file)) {
                break;
            }

            length = strlen(LOG_STORAGE_917_SECTION_HEADER);
            if(storage_file_write(file, LOG_STORAGE_917_SECTION_HEADER, length) != length) {
                FURI_LOG_E(TAG, "Failed to write remote section header");
                break;
            }

#ifdef SRV_INTERCOM
            instance->is_remote_dump_active = true;
            log_storage_dump_remote(instance, file);
            instance->is_remote_dump_active = false;
#endif /* SRV_INTERCOM */

            is_successful = true;
            storage_file_sync(file);

            FURI_LOG_D(TAG, "Log dump saved to %s", target_path);
        } while(false);

        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
    } while(false);

    furi_check(furi_mutex_release(instance->dump_lock) == FuriStatusOk);

    return is_successful;
}

void log_storage_on_system_start(void) {
    LogStorage* instance = malloc(sizeof(*instance));
    log_storage_base_init(&instance->base);

    instance->dump_lock = furi_mutex_alloc(FuriMutexTypeNormal);

#ifdef SRV_INTERCOM
    instance->rx_stream = furi_stream_buffer_alloc(INTERCOM_FRAME_DATA_SIZE, 1);
    instance->is_remote_dump_active = false;

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    instance->channel = intercom_channel_open(
        intercom, IntercomChannelIdLogDump, log_storage_intercom_rx_callback, instance);
#endif /* SRV_INTERCOM */

    furi_record_create(RECORD_LOG_STORAGE, instance);
}
