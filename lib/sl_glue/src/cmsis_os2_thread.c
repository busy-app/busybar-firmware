#include "cmsis_os2.h"

#include <furi.h>
#include <FreeRTOS.h>
#include <task.h>

#define TAG "SlGlue"

#define SL_GLUE_THREAD_DEFAULT_STACK_SIZE 4096U
#define SL_GLUE_MAX_THREADS 8

typedef enum SlGlueThreadState {
    SlGlueThreadStateIdle = 0,
    SlGlueThreadStateAllocated,
    SlGlueThreadStateRunning,
    SlGlueThreadStateFinished,
} SlGlueThreadState;

typedef struct SlGlueThread {
    SlGlueThreadState state;

    TaskHandle_t task;

    osThreadFunc_t callback;
    void* context;

    FuriSemaphore* join_semaphore;
} SlGlueThread;

// Protected by vTaskSuspendAll()
static SlGlueThread sl_glue_threads[SL_GLUE_MAX_THREADS];

static SlGlueThread* sl_glue_thread_alloc(void) {
    SlGlueThread* result = NULL;
    vTaskSuspendAll();
    for(size_t i = 0; i != SL_GLUE_MAX_THREADS; ++i) {
        SlGlueThread* thread = sl_glue_threads + i;
        if(thread->state == SlGlueThreadStateIdle) {
            thread->state = SlGlueThreadStateAllocated;
            result = thread;
            break;
        }
    }
    xTaskResumeAll();
    return result;
}

static SlGlueThread* sl_glue_thread_find(TaskHandle_t task) {
    for(size_t i = 0; i != SL_GLUE_MAX_THREADS; ++i) {
        SlGlueThread* thread = sl_glue_threads + i;
        if(thread->task == task) {
            return thread;
        }
    }
    return NULL;
}

static void sl_glue_thread_free(SlGlueThread* thread) {
    vTaskSuspendAll();
    thread->state = SlGlueThreadStateIdle;
    xTaskResumeAll();
}

static FURI_NORETURN void sl_glue_thread_exit_current(void) {
    TaskHandle_t self = xTaskGetCurrentTaskHandle();
    FuriSemaphore* join_semaphore = NULL;

    vTaskSuspendAll();
    SlGlueThread* thread = sl_glue_thread_find(self);
    if(thread) {
        if(thread->join_semaphore) {
            thread->state = SlGlueThreadStateFinished;
            join_semaphore = thread->join_semaphore;
        } else {
            thread->state = SlGlueThreadStateIdle;
        }
    }
    xTaskResumeAll();

    furi_check(thread, "osThreadExit/Terminate on non-CMSIS thread");

    if(join_semaphore) {
        furi_semaphore_release(join_semaphore);
    }
    vTaskDelete(NULL);

    while(true) ;
}

static void sl_glue_thread_entry(void* context) {
    SlGlueThread* thread = context;
    thread->callback(thread->context);
    sl_glue_thread_exit_current();
}

osThreadId_t osThreadNew(osThreadFunc_t func, void* argument, const osThreadAttr_t* attr) {
    furi_check(func);
    furi_check(!FURI_IS_IRQ_MODE());

    const char* name = (attr && attr->name) ? attr->name : "cmsis";
    const uint32_t stack_size = (attr && attr->stack_size) ? attr->stack_size : SL_GLUE_THREAD_DEFAULT_STACK_SIZE;
    uint32_t prio = (attr && attr->priority != 0 && attr->priority != osPriorityNone) ?
                        (uint32_t)attr->priority :
                        (uint32_t)osPriorityNormal;
    if(prio >= configMAX_PRIORITIES) prio = configMAX_PRIORITIES - 1;
    const bool is_joinable = attr && (attr->attr_bits & osThreadJoinable);

    SlGlueThread* thread = sl_glue_thread_alloc();
    if(!thread) {
        FURI_LOG_E(TAG, "No threads left for SL");
        furi_crash();
    }

    thread->callback = func;
    thread->context = argument;
    if(is_joinable) {
        thread->join_semaphore = furi_semaphore_alloc(1, 0);
    } else {
        thread->join_semaphore = NULL;
    }

    BaseType_t ok = xTaskCreate(sl_glue_thread_entry, name, stack_size / sizeof(StackType_t), thread, prio, &thread->task);
    if(ok == pdPASS) {
        thread->state = SlGlueThreadStateRunning;
        return (osThreadId_t)thread->task;
    } else {
        if(thread->join_semaphore) {
            furi_semaphore_free(thread->join_semaphore);
        }
        sl_glue_thread_free(thread);
        return NULL;
    }
}

osStatus_t osThreadTerminate(osThreadId_t thread_id) {
    if(FURI_IS_IRQ_MODE()) {
        return osErrorISR;
    }
    if(thread_id == NULL) {
        return osErrorParameter;
    }
    if((TaskHandle_t)thread_id == xTaskGetCurrentTaskHandle()) {
        sl_glue_thread_exit_current();
        // no return
    }

    osStatus_t status;
    FuriSemaphore* join_semaphore = NULL;

    vTaskSuspendAll();
    SlGlueThread* thread = sl_glue_thread_find((TaskHandle_t)thread_id);
    if(thread == NULL) {
        status = osErrorParameter; // FuriThread or unknown handle: refuse
    } else if(thread->state != SlGlueThreadStateRunning) {
        status = osErrorResource;
    } else {
        vTaskDelete(thread->task);
        if(thread->join_semaphore) {
            thread->state = SlGlueThreadStateFinished;
            join_semaphore = thread->join_semaphore;
        } else {
            thread->state = SlGlueThreadStateIdle;
        }
        status = osOK;
    }
    xTaskResumeAll();

    if(join_semaphore) {
        furi_semaphore_release(join_semaphore);
    }
    return status;
}

void osThreadExit(void) {
    sl_glue_thread_exit_current();
}

osStatus_t osThreadJoin(osThreadId_t thread_id) {
    if(FURI_IS_IRQ_MODE()) {
        return osErrorISR;
    }
    if(thread_id == NULL || (TaskHandle_t)thread_id == xTaskGetCurrentTaskHandle()) {
        return osErrorParameter;
    }

    vTaskSuspendAll();
    SlGlueThread* thread = sl_glue_thread_find((TaskHandle_t)thread_id);
    FuriSemaphore* join_semaphore = thread ? thread->join_semaphore : NULL;
    xTaskResumeAll();
    if(join_semaphore == NULL) {
        return osErrorResource;
    }

    furi_check(furi_semaphore_acquire(join_semaphore, FuriWaitForever) == FuriStatusOk);

    sl_glue_thread_free(thread);
    furi_semaphore_free(join_semaphore);
    return osOK;
}

osPriority_t osThreadGetPriority(osThreadId_t thread_id) {
    if(thread_id == NULL || FURI_IS_IRQ_MODE()) {
        return osPriorityError;
    }
    return (osPriority_t)uxTaskPriorityGet((TaskHandle_t)thread_id);
}

const char *osThreadGetName (osThreadId_t thread_id) {
    return pcTaskGetName((TaskHandle_t)thread_id);
}

osThreadId_t osThreadGetId(void) {
    return (osThreadId_t)xTaskGetCurrentTaskHandle();
}

osStatus_t osThreadYield (void) {
    taskYIELD();
    return osOK;
}

osStatus_t osThreadSuspend (osThreadId_t thread_id) {
    vTaskSuspend((TaskHandle_t)thread_id);
    return osOK;
}

uint32_t osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags) {
    return furi_thread_flags_set((FuriThreadId)thread_id, flags);
}

uint32_t osThreadFlagsWait(uint32_t flags, uint32_t options, uint32_t timeout) {
    return furi_thread_flags_wait(flags, options, timeout);
}
