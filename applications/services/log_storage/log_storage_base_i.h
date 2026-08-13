#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TAG "LogStorage"

#define LOG_STORAGE_BASE_SNAPSHOT_CHUNKS_COUNT (2u)

typedef struct {
    uint8_t* buffer;
    size_t buffer_size;

    size_t head_idx;
    bool did_wrap;
} LogStorageBase;

typedef struct {
    const uint8_t* data;
    size_t length;
} LogStorageBaseSnapshotChunk;

typedef struct {
    LogStorageBaseSnapshotChunk chunks[LOG_STORAGE_BASE_SNAPSHOT_CHUNKS_COUNT];
} LogStorageBaseSnapshot;

void log_storage_base_internal_capture(LogStorageBase* instance, const uint8_t* data, size_t size);
void log_storage_base_internal_snapshot(LogStorageBase* instance, LogStorageBaseSnapshot* snapshot);

void log_storage_base_internal_init(LogStorageBase* instance, size_t buffer_size);

#ifdef __cplusplus
}
#endif
