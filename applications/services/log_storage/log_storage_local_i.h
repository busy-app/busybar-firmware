#pragma once

#include "log_storage_base_i.h"

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    LogStorageBase base;

    FuriStreamBuffer* rx_stream;

    _Atomic bool did_overrun;
} LogStorageLocal;

void log_storage_local_internal_init(LogStorageLocal* instance, FuriEventLoop* event_loop);
void log_storage_local_internal_flush(LogStorageLocal* instance);

#ifdef __cplusplus
}
#endif
