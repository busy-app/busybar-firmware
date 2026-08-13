#include "log_storage.h"
#include "log_storage_local_i.h"
#include "log_storage_remote_i.h"

#include <storage/storage.h>
#include <device_info/device_info.h>

#include <toolbox/api_lock.h>

#define LOG_STORAGE_MESSAGE_QUEUE_SIZE (4u)

#define LOG_STORAGE_DUMP_DEVICE_INFO_HEADER "[------ Device Info ------]\r\n"
#define LOG_STORAGE_DUMP_U5_SECTION_HEADER  "\r\n[------ STM32U5 ------]\r\n"
#define LOG_STORAGE_DUMP_917_SECTION_HEADER "\r\n[------ SiWG917 ------]\r\n"

struct LogStorage {
    LogStorageLocal local;
    LogStorageRemote remote;

    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;

    Storage* storage;
};

typedef enum {
    LogStorageMessageTypeDump,
    LogStorageMessageTypeSuspend,
    LogStorageMessageTypeResume,

    LogStorageMessageTypesCount,
} LogStorageMessageType;

typedef struct {
    const char* dump_path;
    bool* is_successful;
} LogStorageMessageDump;

typedef union {
    LogStorageMessageDump as_dump;
} LogStorageMessageData;

typedef struct {
    FuriApiLock api_lock;
    LogStorageMessageType type;
    LogStorageMessageData data;
} LogStorageMessage;

typedef void (*LogStorageMessageHandler)(LogStorage* instance, const LogStorageMessage* message);

static void log_storage_do_dump(LogStorage* instance, const LogStorageMessage* message);
static void log_storage_do_remote_suspend(LogStorage* instance, const LogStorageMessage* message);
static void log_storage_do_remote_resume(LogStorage* instance, const LogStorageMessage* message);

static const LogStorageMessageHandler log_storage_handlers[] = {
    [LogStorageMessageTypeDump] = log_storage_do_dump,
    [LogStorageMessageTypeSuspend] = log_storage_do_remote_suspend,
    [LogStorageMessageTypeResume] = log_storage_do_remote_resume,
};

static_assert(COUNT_OF(log_storage_handlers) == LogStorageMessageTypesCount);

static bool log_storage_file_write_all(File* file, const void* data, size_t size) {
    bool is_successful;
    if(storage_file_write(file, data, size) == size) {
        is_successful = true;
    } else {
        FURI_LOG_E(TAG, "Failed to write log data to file");
        is_successful = false;
    }

    return is_successful;
}

static void
    log_storage_device_info_callback(const char* key, const char* value, bool last, void* context) {
    furi_assert(key);
    furi_assert(value);
    furi_assert(context);

    UNUSED(last);

    furi_string_cat_printf(context, "%s: %s\r\n", key, value);
}

static bool log_storage_dump_device_info(File* file) {
    DeviceInfo* device_info = furi_record_open(RECORD_DEVICE_INFO);

    FuriString* string = furi_string_alloc();
    device_info_query(device_info, log_storage_device_info_callback, '_', string);
    bool is_successful =
        log_storage_file_write_all(file, furi_string_get_cstr(string), furi_string_size(string));
    furi_string_free(string);

    furi_record_close(RECORD_DEVICE_INFO);
    return is_successful;
}

static bool log_storage_dump_base(LogStorageBase* base, File* file) {
    bool is_successful = false;
    do {
        LogStorageBaseSnapshot snapshot;
        log_storage_base_internal_snapshot(base, &snapshot);

        is_successful = true;
        for(size_t i = 0; i < COUNT_OF(snapshot.chunks); i++) {
            const LogStorageBaseSnapshotChunk* chunk = &snapshot.chunks[i];

            if(chunk->length == 0) {
                continue;
            }

            if(!log_storage_file_write_all(file, chunk->data, chunk->length)) {
                is_successful = false;
                break;
            }
        }
    } while(false);

    return is_successful;
}

static void log_storage_do_dump(LogStorage* instance, const LogStorageMessage* message) {
    const char* path = message->data.as_dump.dump_path;
    File* file = storage_file_alloc(instance->storage);

    bool is_successful = false;
    if(storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        do {
            size_t length;

            length = strlen(LOG_STORAGE_DUMP_DEVICE_INFO_HEADER);
            if(!log_storage_file_write_all(file, LOG_STORAGE_DUMP_DEVICE_INFO_HEADER, length)) {
                break;
            }

            if(!log_storage_dump_device_info(file)) {
                break;
            }

            length = strlen(LOG_STORAGE_DUMP_U5_SECTION_HEADER);
            if(!log_storage_file_write_all(file, LOG_STORAGE_DUMP_U5_SECTION_HEADER, length)) {
                break;
            }

            log_storage_local_internal_flush(&instance->local);
            if(!log_storage_dump_base(&instance->local.base, file)) {
                break;
            }

            length = strlen(LOG_STORAGE_DUMP_917_SECTION_HEADER);
            if(!log_storage_file_write_all(file, LOG_STORAGE_DUMP_917_SECTION_HEADER, length)) {
                break;
            }

            log_storage_remote_internal_flush(&instance->remote);
            if(!log_storage_dump_base(&instance->remote.base, file)) {
                break;
            }

            FURI_LOG_I(TAG, "Log dump saved to %s", path);

            is_successful = true;
        } while(false);
    } else {
        FURI_LOG_E(TAG, "Failed to open file %s", path);
    }

    storage_file_free(file);
    *message->data.as_dump.is_successful = is_successful;
}

static void log_storage_do_remote_suspend(LogStorage* instance, const LogStorageMessage* message) {
    UNUSED(message);

    log_storage_remote_internal_suspend(&instance->remote);
}

static void log_storage_do_remote_resume(LogStorage* instance, const LogStorageMessage* message) {
    UNUSED(message);

    log_storage_remote_internal_resume(&instance->remote);
}

static void log_storage_message_queue_callback(FuriEventLoopObject* object, void* context) {
    LogStorage* instance = context;

    furi_assert(object == instance->message_queue);

    LogStorageMessage message;
    furi_check(
        furi_message_queue_get(instance->message_queue, &message, FuriWaitForever) ==
        FuriStatusOk);

    log_storage_handlers[message.type](instance, &message);

    api_lock_unlock(message.api_lock);
}

static void log_storage_send_message(LogStorage* instance, LogStorageMessage* message) {
    message->api_lock = api_lock_alloc_locked();
    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(message->api_lock);
}

bool log_storage_dump(LogStorage* instance, const char* path) {
    furi_check(instance);

    bool is_successful;
    LogStorageMessage message = {
        .type = LogStorageMessageTypeDump,
        .data.as_dump =
            {
                .dump_path = path ?: LOG_STORAGE_DUMP_DEFAULT_FILE_PATH,
                .is_successful = &is_successful,
            },
    };

    log_storage_send_message(instance, &message);
    return is_successful;
}

void log_storage_suspend_remote(LogStorage* instance) {
    furi_check(instance);

    LogStorageMessage message = {
        .type = LogStorageMessageTypeSuspend,
    };

    log_storage_send_message(instance, &message);
}

void log_storage_resume_remote(LogStorage* instance) {
    furi_check(instance);

    LogStorageMessage message = {
        .type = LogStorageMessageTypeResume,
    };

    log_storage_send_message(instance, &message);
}

int32_t log_storage_srv(void* context) {
    UNUSED(context);

    LogStorage* instance = malloc(sizeof(*instance));

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue =
        furi_message_queue_alloc(LOG_STORAGE_MESSAGE_QUEUE_SIZE, sizeof(LogStorageMessage));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        log_storage_message_queue_callback,
        instance);

    instance->storage = furi_record_open(RECORD_STORAGE);

    log_storage_local_internal_init(&instance->local, instance->event_loop);
    log_storage_remote_internal_init(&instance->remote, instance->event_loop);

    furi_record_create(RECORD_LOG_STORAGE, instance);
    furi_event_loop_run(instance->event_loop);

    return 0;
}
