#include "lv_furi.h"

#include <lvgl.h>

#define TAG "FuriOsal"

#define THREAD_SYNC_FLAG (1UL << 0)

// Thread

static const FuriThreadPriority lv_prio_map[] = {
    [LV_THREAD_PRIO_LOWEST] = FuriThreadPriorityLowest,
    [LV_THREAD_PRIO_LOW] = FuriThreadPriorityLow,
    [LV_THREAD_PRIO_MID] = FuriThreadPriorityNormal,
    [LV_THREAD_PRIO_HIGH] = FuriThreadPriorityHigh,
    [LV_THREAD_PRIO_HIGHEST] = FuriThreadPriorityHighest,
};

static FURI_ALWAYS_INLINE int32_t lv_thread_furi_callback(void* arg) {
    lv_thread_t* thread = arg;
    thread->callback(thread->user_data);
    return 0;
}

lv_result_t lv_thread_init(
    lv_thread_t* thread,
    const char* const name,
    lv_thread_prio_t prio,
    void (*callback)(void*),
    size_t stack_size,
    void* user_data) {
    furi_assert(thread);
    furi_assert(callback);

    thread->furi_thread = furi_thread_alloc_ex(name, stack_size, lv_thread_furi_callback, thread);
    thread->callback = callback;
    thread->user_data = user_data;

    furi_thread_set_priority(thread->furi_thread, lv_prio_map[prio]);
    furi_thread_start(thread->furi_thread);

    return LV_RESULT_OK;
}

lv_result_t lv_thread_delete(lv_thread_t* thread) {
    furi_assert(thread);
    furi_crash(TAG ": Cannot delete threads");
    return LV_RESULT_OK;
}

// Mutex

lv_result_t lv_mutex_init(lv_mutex_t* mutex) {
    furi_assert(mutex);

    mutex->furi_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_lock(lv_mutex_t* mutex) {
    furi_assert(mutex);

    const FuriStatus status = furi_mutex_acquire(mutex->furi_mutex, FuriWaitForever);
    return status == FuriStatusOk ? LV_RESULT_OK : LV_RESULT_INVALID;
}

lv_result_t lv_mutex_lock_isr(lv_mutex_t* mutex) {
    return lv_mutex_lock(mutex);
}

lv_result_t lv_mutex_unlock(lv_mutex_t* mutex) {
    furi_assert(mutex);

    const FuriStatus status = furi_mutex_release(mutex->furi_mutex);
    return status == FuriStatusOk ? LV_RESULT_OK : LV_RESULT_INVALID;
}

lv_result_t lv_mutex_delete(lv_mutex_t* mutex) {
    furi_assert(mutex);

    furi_mutex_free(mutex->furi_mutex);
    return LV_RESULT_OK;
}

// Thread Sync

lv_result_t lv_thread_sync_init(lv_thread_sync_t* sync) {
    furi_assert(sync);

    sync->furi_event_flag = furi_event_flag_alloc();
    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_wait(lv_thread_sync_t* sync) {
    furi_assert(sync);

    uint32_t flags = furi_event_flag_wait(
        sync->furi_event_flag, THREAD_SYNC_FLAG, FuriFlagWaitAll, FuriWaitForever);
    return flags & FuriFlagError ? LV_RESULT_INVALID : LV_RESULT_OK;
}

lv_result_t lv_thread_sync_signal(lv_thread_sync_t* sync) {
    furi_assert(sync);

    furi_event_flag_set(sync->furi_event_flag, THREAD_SYNC_FLAG);
    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_signal_isr(lv_thread_sync_t* sync) {
    return lv_thread_sync_signal(sync);
}

lv_result_t lv_thread_sync_delete(lv_thread_sync_t* sync) {
    furi_assert(sync);

    furi_event_flag_free(sync->furi_event_flag);
    return LV_RESULT_OK;
}
