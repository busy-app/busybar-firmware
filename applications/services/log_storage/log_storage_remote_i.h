#pragma once

#include "log_storage_base_i.h"

#include <furi.h>
#include <furi_hal_serial.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    LogStorageBase base;

    FuriHalSerialHandle* serial;
    FuriStreamBuffer* rx_stream;

    _Atomic bool did_overrun;
} LogStorageRemote;

void log_storage_remote_internal_suspend(LogStorageRemote* instance);
void log_storage_remote_internal_resume(LogStorageRemote* instance);

void log_storage_remote_internal_init(LogStorageRemote* instance, FuriEventLoop* event_loop);
void log_storage_remote_internal_flush(LogStorageRemote* instance);

#ifdef __cplusplus
}
#endif
