#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    FuriThread* furi_thread;
    void (*callback)(void*);
    void* user_data;
} lv_thread_t;

typedef struct {
    FuriMutex* furi_mutex;
} lv_mutex_t;

typedef struct {
    FuriThreadId sync_thread;
    bool sync_pending;
} lv_thread_sync_t;

#ifdef __cplusplus
} /*extern "C"*/
#endif
