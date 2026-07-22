#pragma once

#include <furi.h>

#define LOG_STORAGE_LOCK_TIMEOUT_MS 1000u

/* default value - overridden by application.fam */
#ifndef LOG_STORAGE_LOG_BUFFER_SIZE
#define LOG_STORAGE_LOG_BUFFER_SIZE (4u * 1024u)
#endif /* LOG_STORAGE_LOG_BUFFER_SIZE */

#define LOG_STORAGE_SNAPSHOT_CHUNKS_COUNT 2u

typedef enum {
    LogIntercomRequestDump = 0x01,
} LogIntercomRequest;

typedef struct {
    FuriMutex* lock;

    uint8_t log_buffer[LOG_STORAGE_LOG_BUFFER_SIZE];
    size_t head_idx;
    bool did_not_wrap;
} LogStorageBase;

typedef struct {
    const uint8_t* data;
    size_t length;
} LogStorageSnapshotChunk;

typedef struct {
    LogStorageSnapshotChunk chunks[LOG_STORAGE_SNAPSHOT_CHUNKS_COUNT];
} LogStorageSnapshot;

void log_storage_base_init(LogStorageBase* instance);

bool log_storage_base_snapshot_take(LogStorageBase* instance, LogStorageSnapshot* snapshot);
void log_storage_base_snapshot_release(LogStorageBase* instance);
