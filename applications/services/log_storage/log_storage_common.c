#include "log_storage_common_i.h"

#include <string.h>

#define LOG_STORAGE_LOCK_TIMEOUT_MS 1000u

static void log_storage_base_on_log(const uint8_t* data, size_t size, void* context) {
    LogStorageBase* instance = context;

    if(size >= LOG_STORAGE_LOG_BUFFER_SIZE) return;
    if(furi_mutex_acquire(instance->lock, 0) != FuriStatusOk) return;

    size_t space_till_wrap = MIN(LOG_STORAGE_LOG_BUFFER_SIZE - instance->head_idx, size);
    memcpy(&instance->log_buffer[instance->head_idx], data, space_till_wrap);
    if(size > space_till_wrap) {
        memcpy(instance->log_buffer, &data[space_till_wrap], size - space_till_wrap);
        instance->head_idx = size - space_till_wrap;
        instance->did_wrap = true;
    } else {
        instance->head_idx = instance->head_idx + size;
    }

    furi_check(furi_mutex_release(instance->lock) == FuriStatusOk);
}

bool log_storage_base_snapshot_take(LogStorageBase* instance, LogStorageSnapshot* snapshot) {
    furi_check(instance);
    furi_check(snapshot);

    if(furi_mutex_acquire(instance->lock, furi_ms_to_ticks(LOG_STORAGE_LOCK_TIMEOUT_MS)) !=
       FuriStatusOk) {
        return false;
    }

    LogStorageSnapshotChunk* head_chunk = &snapshot->chunks[0];
    head_chunk->data = NULL;
    head_chunk->length = 0;

    LogStorageSnapshotChunk* tail_chunk = &snapshot->chunks[1];
    tail_chunk->data = NULL;
    tail_chunk->length = 0;

    size_t head_idx = instance->head_idx;
    if(instance->did_wrap) {
        size_t head_length = LOG_STORAGE_LOG_BUFFER_SIZE - head_idx;

        uint8_t* head_trim = memchr(&instance->log_buffer[head_idx], '\n', head_length) ?:
                                 memchr(&instance->log_buffer, '\n', head_idx);
        uint8_t* tail_trim = memrchr(instance->log_buffer, '\n', head_idx) ?:
                                 memrchr(&instance->log_buffer[head_idx], '\n', head_length);

        if(head_trim != tail_trim) {
            size_t head_trim_idx =
                (size_t)(head_trim - instance->log_buffer + 1) % LOG_STORAGE_LOG_BUFFER_SIZE;
            size_t tail_trim_idx =
                (size_t)(tail_trim - instance->log_buffer + 1) % LOG_STORAGE_LOG_BUFFER_SIZE;

            head_chunk->data = &instance->log_buffer[head_trim_idx];
            if(tail_trim_idx > head_trim_idx) {
                head_chunk->length = tail_trim_idx - head_trim_idx;
            } else {
                head_chunk->length = LOG_STORAGE_LOG_BUFFER_SIZE - head_trim_idx;

                tail_chunk->data = instance->log_buffer;
                tail_chunk->length = tail_trim_idx;
            }
        }
    } else {
        uint8_t* tail_trim = memrchr(instance->log_buffer, '\n', head_idx);

        if(tail_trim) {
            head_chunk->data = instance->log_buffer;
            head_chunk->length = tail_trim - instance->log_buffer + 1;
        }
    }

    return true;
}

void log_storage_base_snapshot_release(LogStorageBase* instance) {
    furi_check(instance);
    furi_check(furi_mutex_release(instance->lock) == FuriStatusOk);
}

void log_storage_base_init(LogStorageBase* instance) {
    furi_check(instance);

    instance->lock = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->head_idx = 0;
    instance->did_wrap = false;

    furi_check(furi_log_add_handler((FuriLogHandler){
        .callback = log_storage_base_on_log,
        .context = instance,
    }));
}
