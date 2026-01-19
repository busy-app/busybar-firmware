#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include <FreeRTOS.h>

// not used therefore defined to smallest possible type to save space
typedef uint8_t osal_semaphore_def_t;
typedef uint8_t osal_mutex_def_t;

typedef FuriSemaphore* osal_semaphore_t;
typedef FuriMutex* osal_mutex_t;
typedef FuriMessageQueue* osal_queue_t;

typedef struct {
    uint16_t depth;
    uint16_t item_sz;
} osal_queue_def_t;

#define OSAL_QUEUE_DEF(_int_set, _name, _depth, _type) \
    osal_queue_def_t _name = {.depth = _depth, .item_sz = sizeof(_type)}

FURI_ALWAYS_INLINE static inline uint32_t _osal_ms2tick(uint32_t msec) {
    if(msec == OSAL_TIMEOUT_WAIT_FOREVER) {
        return FuriWaitForever;
    }
    if(msec == 0) {
        return 0;
    }

    uint32_t ticks = furi_ms_to_ticks(msec);

    // we still need to delay at least 1 tick
    if(ticks == 0) {
        ticks = 1;
    }

    return ticks;
}

FURI_ALWAYS_INLINE static inline void osal_task_delay(uint32_t msec) {
    furi_delay_ms(msec);
}

#define OSAL_SPINLOCK_DEF(_name, _int_set) osal_spinlock_t _name

typedef UBaseType_t osal_spinlock_t;

FURI_ALWAYS_INLINE static inline void osal_spin_init(osal_spinlock_t* ctx) {
    UNUSED(ctx);
}

FURI_ALWAYS_INLINE static inline void osal_spin_lock(osal_spinlock_t* ctx, bool in_isr) {
    if(in_isr) {
        return; // single core MCU does not need to lock in ISR
    }
    UNUSED(ctx);
    // FURI_CRITICAL_ENTER is not usable here
    vPortEnterCritical();
}

FURI_ALWAYS_INLINE static inline void osal_spin_unlock(osal_spinlock_t* ctx, bool in_isr) {
    if(in_isr) {
        return; // single core MCU does not need to lock in ISR
    }
    UNUSED(ctx);
    // FURI_CRITICAL_EXIT is not usable here
    vPortExitCritical();
}

FURI_ALWAYS_INLINE static inline osal_semaphore_t
    osal_semaphore_create(osal_semaphore_def_t* semdef) {
    UNUSED(semdef);
    return furi_semaphore_alloc(1, 0);
}

FURI_ALWAYS_INLINE static inline bool osal_semaphore_delete(osal_semaphore_t semd_hdl) {
    furi_semaphore_free(semd_hdl);
    return true;
}

FURI_ALWAYS_INLINE static inline bool osal_semaphore_post(osal_semaphore_t sem_hdl, bool in_isr) {
    UNUSED(in_isr);
    FuriStatus stat = furi_semaphore_release(sem_hdl);
    return (stat == FuriStatusOk);
}

FURI_ALWAYS_INLINE static inline bool
    osal_semaphore_wait(osal_semaphore_t sem_hdl, uint32_t msec) {
    FuriStatus stat = furi_semaphore_acquire(sem_hdl, _osal_ms2tick(msec));
    return (stat == FuriStatusOk);
}

FURI_ALWAYS_INLINE static inline void osal_semaphore_reset(osal_semaphore_t const sem_hdl) {
    furi_semaphore_acquire(sem_hdl, 0);
}

FURI_ALWAYS_INLINE static inline osal_mutex_t osal_mutex_create(osal_mutex_def_t* mdef) {
    UNUSED(mdef);
    return furi_mutex_alloc(FuriMutexTypeNormal);
}

FURI_ALWAYS_INLINE static inline bool osal_mutex_delete(osal_mutex_t mutex_hdl) {
    furi_mutex_free(mutex_hdl);
    return true;
}

FURI_ALWAYS_INLINE static inline bool osal_mutex_lock(osal_mutex_t mutex_hdl, uint32_t msec) {
    FuriStatus stat = furi_mutex_acquire(mutex_hdl, _osal_ms2tick(msec));
    return (stat == FuriStatusOk);
}

FURI_ALWAYS_INLINE static inline bool osal_mutex_unlock(osal_mutex_t mutex_hdl) {
    FuriStatus stat = furi_mutex_release(mutex_hdl);
    return (stat == FuriStatusOk);
}

FURI_ALWAYS_INLINE static inline osal_queue_t osal_queue_create(osal_queue_def_t* qdef) {
    return furi_message_queue_alloc(qdef->depth, qdef->item_sz);
}

FURI_ALWAYS_INLINE static inline bool osal_queue_delete(osal_queue_t qhdl) {
    furi_message_queue_free(qhdl);
    return true;
}

FURI_ALWAYS_INLINE static inline bool
    osal_queue_receive(osal_queue_t qhdl, void* data, uint32_t msec) {
    FuriStatus stat = furi_message_queue_get(qhdl, data, _osal_ms2tick(msec));
    return (stat == FuriStatusOk);
}

FURI_ALWAYS_INLINE static inline bool
    osal_queue_send(osal_queue_t qhdl, void const* data, bool in_isr) {
    UNUSED(in_isr);
    FuriStatus stat = furi_message_queue_put(qhdl, data, in_isr ? 0 : FuriWaitForever);
    return (stat == FuriStatusOk);
}

FURI_ALWAYS_INLINE static inline bool osal_queue_empty(osal_queue_t qhdl) {
    return (furi_message_queue_get_count(qhdl) == 0);
}

#ifdef __cplusplus
}
#endif
