#include "log_storage_i.h"

#include <toolbox/api_lock.h>

#define LOG_STORAGE_API_MESSAGE_QUEUE_SIZE 4u

#define LOG_STORAGE_REMOTE_FAILURE_DUMP_PATH "/ext/intercom_failure_log.txt"

#define LOG_STORAGE_DUMP_U5_SECTION_HEADER  "[------ STM32U5 ------]\r\n"
#define LOG_STORAGE_DUMP_917_SECTION_HEADER "\r\n[------ SiWG917 ------]\r\n"

typedef enum {
    LogStorageApiMessageTypeDump,
} LogStorageApiMessageType;

typedef struct {
    union {
        const char* dump_path;
    };

    FuriApiLock api_lock;
    bool* is_successful;
    LogStorageApiMessageType type;
} LogStorageApiMessage;

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
            FURI_LOG_E(TAG, "Failed to write log data to file");
            is_successful = false;
            break;
        }
    }

    log_storage_base_snapshot_release(&instance->base);
    return is_successful;
}

static bool log_storage_do_dump(LogStorage* instance, const char* path) {
    File* file = storage_file_alloc(instance->storage);

    bool is_successful = false;
    if(storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        do {
            size_t length = strlen(LOG_STORAGE_DUMP_U5_SECTION_HEADER);
            if(storage_file_write(file, LOG_STORAGE_DUMP_U5_SECTION_HEADER, length) != length) {
                FURI_LOG_E(TAG, "Failed to write log data to file");
                break;
            }

            if(!log_storage_dump_local(instance, file)) break;

            length = strlen(LOG_STORAGE_DUMP_917_SECTION_HEADER);
            if(storage_file_write(file, LOG_STORAGE_DUMP_917_SECTION_HEADER, length) != length) {
                FURI_LOG_E(TAG, "Failed to write log data to file");
                break;
            }

            if(!log_storage_intercom_remote_dump(instance->log_intercom, file)) {
                break;
            }

            FURI_LOG_I(TAG, "Log dump saved to %s", path);

            is_successful = true;
        } while(false);
    } else {
        FURI_LOG_E(TAG, "Failed to open file %s", path);
    }

    storage_file_free(file);
    return is_successful;
}

static void log_storage_api_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    LogStorage* instance = context;

    LogStorageApiMessage message;
    furi_message_queue_get(instance->api_message_queue, &message, 0);

    if(message.type == LogStorageApiMessageTypeDump) {
        *message.is_successful = log_storage_do_dump(instance, message.dump_path);
    }

    api_lock_unlock(message.api_lock);
}

static void log_storage_custom_event_callback(uint32_t events, void* context) {
    LogStorage* instance = context;

    log_storage_intercom_on_event(instance->log_intercom, events);

    if(events & LogStorageEventIntercomLinkDown) {
        log_storage_do_dump(instance, LOG_STORAGE_REMOTE_FAILURE_DUMP_PATH);
    }
}

bool log_storage_dump(LogStorage* instance, const char* path) {
    furi_check(instance);

    bool is_successful = false;
    LogStorageApiMessage api_message = {
        .dump_path = path ?: LOG_STORAGE_DUMP_DEFAULT_FILE_PATH,

        .type = LogStorageApiMessageTypeDump,
        .api_lock = api_lock_alloc_locked(),
        .is_successful = &is_successful,
    };

    furi_check(
        furi_message_queue_put(instance->api_message_queue, &api_message, FuriWaitForever) ==
        FuriStatusOk);

    api_lock_wait_unlock_and_free(api_message.api_lock);
    return is_successful;
}

int32_t log_storage_srv(void* context) {
    UNUSED(context);

    LogStorage* instance = malloc(sizeof(*instance));
    log_storage_base_init(&instance->base);

    instance->event_loop = furi_event_loop_alloc();
    instance->api_message_queue =
        furi_message_queue_alloc(LOG_STORAGE_API_MESSAGE_QUEUE_SIZE, sizeof(LogStorageApiMessage));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->api_message_queue,
        FuriEventLoopEventIn,
        log_storage_api_queue_callback,
        instance);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, log_storage_custom_event_callback, instance);

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->log_intercom = log_storage_intercom_init(instance->event_loop);

    furi_record_create(RECORD_LOG_STORAGE, instance);
    furi_event_loop_run(instance->event_loop);

    return 0;
}
