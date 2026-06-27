#include "log_storage.h"

#include <storage/storage.h>
#include <string.h>

#define LOG_STORAGE_LOG_BUFFER_SIZE (8u * 1024u)
#define LOG_STORAGE_LOCK_TIMEOUT_MS 1500u

struct LogStorage {
    FuriMutex* lock;

    uint8_t log_buffer[LOG_STORAGE_LOG_BUFFER_SIZE];
    size_t head_idx;
    size_t bytes_count;
};

static void log_storage_on_log(const uint8_t* data, size_t size, void* context) {
    LogStorage* instance = context;

    if(size >= LOG_STORAGE_LOG_BUFFER_SIZE) return;
    if(furi_mutex_acquire(instance->lock, furi_ms_to_ticks(LOG_STORAGE_LOCK_TIMEOUT_MS)) !=
       FuriStatusOk) {
        return;
    }

    size_t space_till_wrap = MIN(LOG_STORAGE_LOG_BUFFER_SIZE - instance->head_idx, size);
    memcpy(&instance->log_buffer[instance->head_idx], data, space_till_wrap);
    if(size > space_till_wrap) {
        memcpy(instance->log_buffer, &data[space_till_wrap], size - space_till_wrap);
    }

    instance->head_idx = (instance->head_idx + size) % LOG_STORAGE_LOG_BUFFER_SIZE;
    instance->bytes_count = MIN(instance->bytes_count + size, LOG_STORAGE_LOG_BUFFER_SIZE);

    furi_check(furi_mutex_release(instance->lock) == FuriStatusOk);
}

bool log_storage_dump(LogStorage* instance, const char* path) {
    furi_check(instance);

    if(furi_mutex_acquire(instance->lock, furi_ms_to_ticks(LOG_STORAGE_LOCK_TIMEOUT_MS)) !=
       FuriStatusOk) {
        return false;
    }

    /* create log snapshot */
    size_t bytes_count = instance->bytes_count;
    size_t start_idx = (instance->head_idx + LOG_STORAGE_LOG_BUFFER_SIZE - bytes_count) %
                       LOG_STORAGE_LOG_BUFFER_SIZE;
    uint8_t* log_snapshot = malloc(bytes_count > 0 ? bytes_count : 1);

    size_t space_till_wrap = MIN(LOG_STORAGE_LOG_BUFFER_SIZE - start_idx, bytes_count);
    memcpy(log_snapshot, &instance->log_buffer[start_idx], space_till_wrap);
    if(bytes_count > space_till_wrap) {
        memcpy(
            &log_snapshot[space_till_wrap], instance->log_buffer, bytes_count - space_till_wrap);
    }

    furi_check(furi_mutex_release(instance->lock) == FuriStatusOk);

    /* trim leading (only in case of wrapping) */
    size_t begin = 0;
    if(bytes_count == LOG_STORAGE_LOG_BUFFER_SIZE) {
        const uint8_t* new_line = memchr(log_snapshot, '\n', bytes_count);
        begin = new_line ? (size_t)(new_line - log_snapshot) + 1 : bytes_count;
    }

    /* trim trailing (log entries are printed in multiple TX calls) */
    const uint8_t* new_line = memrchr(log_snapshot + begin, '\n', bytes_count - begin);
    size_t end = new_line ? (size_t)(new_line - log_snapshot) + 1 : begin;

    /* save log dump to file */
    size_t length = end - begin;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool is_successful = storage_simply_write_entire_file(
        storage,
        path ?: LOG_STORAGE_DUMP_DEFAULT_FILE_PATH,
        length ? log_snapshot + begin : log_snapshot,
        length);
    if(length == 0) is_successful = true;
    furi_record_close(RECORD_STORAGE);

    free(log_snapshot);
    return is_successful;
}

void log_storage_on_system_start(void) {
    LogStorage* instance = malloc(sizeof(*instance));

    instance->lock = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->head_idx = 0;
    instance->bytes_count = 0;

    furi_record_create(RECORD_LOG_STORAGE, instance);

    furi_check(furi_log_add_handler((FuriLogHandler){
        .callback = log_storage_on_log,
        .context = instance,
    }));
}
