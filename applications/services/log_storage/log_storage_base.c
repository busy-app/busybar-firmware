#include "log_storage_base_i.h"

#include <furi.h>

void log_storage_base_internal_capture(LogStorageBase* instance, const uint8_t* data, size_t size) {
    furi_check(data);
    furi_check(instance);

    size_t buffer_size = instance->buffer_size;

    if(size <= buffer_size) {
        size_t head_idx = instance->head_idx;
        uint8_t* buffer = instance->buffer;

        size_t space_till_wrap = MIN(buffer_size - head_idx, size);
        memcpy(&buffer[head_idx], data, space_till_wrap);

        if(size > space_till_wrap) {
            memcpy(buffer, &data[space_till_wrap], size - space_till_wrap);

            instance->head_idx = size - space_till_wrap;
            instance->did_wrap = true;
        } else {
            instance->head_idx += size;
        }
    }
}

void log_storage_base_internal_snapshot(LogStorageBase* instance, LogStorageBaseSnapshot* snapshot) {
    furi_check(instance);
    furi_check(snapshot);

    LogStorageBaseSnapshotChunk* head_chunk = &snapshot->chunks[0];
    head_chunk->data = NULL;
    head_chunk->length = 0;

    LogStorageBaseSnapshotChunk* tail_chunk = &snapshot->chunks[1];
    tail_chunk->data = NULL;
    tail_chunk->length = 0;

    size_t head_idx = instance->head_idx;
    if(instance->did_wrap) {
        size_t head_length = instance->buffer_size - head_idx;

        uint8_t* head_trim = memchr(&instance->buffer[head_idx], '\n', head_length) ?:
                                 memchr(instance->buffer, '\n', head_idx);
        uint8_t* tail_trim = memrchr(instance->buffer, '\n', head_idx) ?:
                                 memrchr(&instance->buffer[head_idx], '\n', head_length);

        if(head_trim != tail_trim) {
            size_t head_trim_idx = (head_trim - instance->buffer + 1) % instance->buffer_size;
            size_t tail_trim_idx = (tail_trim - instance->buffer + 1) % instance->buffer_size;

            head_chunk->data = &instance->buffer[head_trim_idx];
            if(tail_trim_idx > head_trim_idx) {
                head_chunk->length = tail_trim_idx - head_trim_idx;
            } else {
                head_chunk->length = instance->buffer_size - head_trim_idx;

                tail_chunk->data = instance->buffer;
                tail_chunk->length = tail_trim_idx;
            }
        }
    } else {
        uint8_t* tail_trim = memrchr(instance->buffer, '\n', head_idx);

        if(tail_trim) {
            head_chunk->data = instance->buffer;
            head_chunk->length = tail_trim - instance->buffer + 1;
        }
    }
}

void log_storage_base_internal_init(LogStorageBase* instance, size_t buffer_size) {
    furi_check(instance);
    furi_check(buffer_size > 0);

    instance->buffer = malloc(buffer_size);
    instance->buffer_size = buffer_size;

    instance->head_idx = 0;
    instance->did_wrap = false;
}
