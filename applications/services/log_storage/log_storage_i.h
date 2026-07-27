#pragma once

#include "log_storage_common_i.h"
#include "log_storage.h"

#include <storage/storage.h>

#define TAG "LogStorage"

typedef enum {
    LogStorageEventIntercomLinkUp = 1 << 0,
    LogStorageEventIntercomLinkDown = 1 << 1,
} LogStorageEvent;

typedef struct LogStorageIntercom LogStorageIntercom;

struct LogStorage {
    LogStorageBase base;

    FuriEventLoop* event_loop;
    FuriMessageQueue* api_message_queue;

    LogStorageIntercom* log_intercom;

    Storage* storage;
};

LogStorageIntercom* log_storage_intercom_init(FuriEventLoop* event_loop);
void log_storage_intercom_on_event(LogStorageIntercom* log_intercom, uint32_t events);
bool log_storage_intercom_remote_dump(LogStorageIntercom* log_intercom, File* file);
