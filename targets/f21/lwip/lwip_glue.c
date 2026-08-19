#include <furi.h>

#include <lwip/debug.h>
#include <lwip/sys.h>
#include <lwip/mem.h>
#include <lwip/stats.h>
#include <lwip/tcpip.h>

#define TAG "LwipGlue"

static FuriThread* lwip_tcpip_thread;
static FuriMutex* lwip_protect_mutex;

/* Initialize this module (see description in sys.h) */
void sys_init(void) {
    furi_check(lwip_protect_mutex == NULL);
    /* initialize sys_arch_protect global mutex */
    lwip_protect_mutex = furi_mutex_alloc(FuriMutexTypeRecursive);
}

uint32_t sys_now(void) {
    return furi_get_tick();
}

uint32_t sys_jiffies(void) {
    return furi_get_tick();
}

sys_prot_t sys_arch_protect(void) {
    FuriStatus ret = furi_mutex_acquire(lwip_protect_mutex, FuriWaitForever);
    furi_check(ret == FuriStatusOk);
    return 1;
}

void sys_arch_unprotect(sys_prot_t pval) {
    UNUSED(pval);
    FuriStatus ret = furi_mutex_release(lwip_protect_mutex);
    furi_check(ret == FuriStatusOk);
}

void sys_arch_msleep(uint32_t delay_ms) {
    furi_delay_ms(delay_ms);
}

#if !LWIP_COMPAT_MUTEX

/* Create a new mutex*/
err_t sys_mutex_new(sys_mutex_t* mutex) {
    furi_check(mutex);
    furi_check(mutex->mut == NULL);

    mutex->mut = furi_mutex_alloc(FuriMutexTypeNormal);
    SYS_STATS_INC_USED(mutex);
    return ERR_OK;
}

void sys_mutex_lock(sys_mutex_t* mutex) {
    furi_check(mutex);

    FuriStatus ret = furi_mutex_acquire(mutex->mut, FuriWaitForever);
    furi_check(ret == FuriStatusOk);
}

void sys_mutex_unlock(sys_mutex_t* mutex) {
    furi_check(mutex);
    FuriStatus ret = furi_mutex_release(mutex->mut);
    furi_check(ret == FuriStatusOk);
}

void sys_mutex_free(sys_mutex_t* mutex) {
    furi_check(mutex);
    SYS_STATS_DEC(mutex.used);
    furi_mutex_free(mutex->mut);
    mutex->mut = NULL;
}

#endif

err_t sys_sem_new(sys_sem_t* sem, uint8_t initial_count) {
    furi_check(sem);
    furi_check(initial_count <= 1);

    sem->sem = furi_semaphore_alloc(1, initial_count);
    SYS_STATS_INC_USED(sem);
    return ERR_OK;
}

void sys_sem_signal(sys_sem_t* sem) {
    furi_check(sem);

    furi_semaphore_release(sem->sem);
}

uint32_t sys_arch_sem_wait(sys_sem_t* sem, uint32_t timeout_ms) {
    furi_check(sem);

    if(!timeout_ms) {
        FuriStatus ret = furi_semaphore_acquire(sem->sem, FuriWaitForever);
        furi_check(ret == FuriStatusOk);
    } else {
        FuriStatus ret = furi_semaphore_acquire(sem->sem, timeout_ms);
        if(ret == FuriStatusErrorTimeout) {
            return SYS_ARCH_TIMEOUT;
        }
    }

    /* Old versions of lwIP required us to return the time waited.
     This is not the case any more. Just returning != SYS_ARCH_TIMEOUT
     here is enough. */
    return 1;
}

void sys_sem_free(sys_sem_t* sem) {
    furi_check(sem);
    SYS_STATS_DEC(sem.used);
    furi_semaphore_free(sem->sem);
    sem->sem = NULL;
}

err_t sys_mbox_new(sys_mbox_t* mbox, int size) {
    furi_check(mbox);
    furi_check(size > 0);

    mbox->mbx = furi_message_queue_alloc(size, sizeof(void*));
    SYS_STATS_INC_USED(mbox);
    return ERR_OK;
}

void sys_mbox_post(sys_mbox_t* mbox, void* msg) {
    furi_check(mbox);

    FuriStatus ret = furi_message_queue_put(mbox->mbx, &msg, FuriWaitForever);
    furi_check(ret == FuriStatusOk);
}

err_t sys_mbox_trypost(sys_mbox_t* mbox, void* msg) {
    furi_check(mbox);

    FuriStatus ret = furi_message_queue_put(mbox->mbx, &msg, 0);
    if(ret != FuriStatusOk) {
        SYS_STATS_INC(mbox.err);
        return ERR_MEM;
    }

    return ERR_OK;
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t* mbox, void* msg) {
    return sys_mbox_trypost(mbox, msg);
}

uint32_t sys_arch_mbox_fetch(sys_mbox_t* mbox, void** msg, uint32_t timeout_ms) {
    furi_check(mbox);

    void* msg_dummy;
    if(!msg) {
        msg = &msg_dummy;
    }

    if(!timeout_ms) {
        FuriStatus ret = furi_message_queue_get(mbox->mbx, &(*msg), FuriWaitForever);
        furi_check(ret == FuriStatusOk);
    } else {
        FuriStatus ret = furi_message_queue_get(mbox->mbx, &(*msg), timeout_ms);
        if(ret == FuriStatusErrorTimeout) {
            *msg = NULL;
            return SYS_ARCH_TIMEOUT;
        } else if(ret != FuriStatusOk) {
            furi_crash("sys_arch_mbox_fetch: unexpected error");
        }
    }

    /* Old versions of lwIP required us to return the time waited.
     This is not the case any more. Just returning != SYS_ARCH_TIMEOUT
     here is enough. */
    return 1;
}

uint32_t sys_arch_mbox_tryfetch(sys_mbox_t* mbox, void** msg) {
    furi_check(mbox);

    void* msg_dummy;
    if(!msg) {
        msg = &msg_dummy;
    }

    FuriStatus ret = furi_message_queue_get(mbox->mbx, &(*msg), 0);
    if(ret != FuriStatusOk) {
        *msg = NULL;
        return SYS_ARCH_TIMEOUT;
    }

    return 0;
}

void sys_mbox_free(sys_mbox_t* mbox) {
    furi_check(mbox);

    furi_message_queue_free(mbox->mbx);
    SYS_STATS_DEC(mbox.used);
}

struct ThreadWrapper {
    FuriThread* thread;
    lwip_thread_fn function;
    void* arg;
};

static int32_t sys_thread_wrapper(void* arg) {
    furi_check(arg);
    ThreadWrapper* thread_wrapper = arg;

    thread_wrapper->function(thread_wrapper->arg);

    furi_thread_free(thread_wrapper->thread);
    free(thread_wrapper);
    return 0;
}

sys_thread_t
    sys_thread_new(const char* name, lwip_thread_fn thread, void* arg, int stacksize, int prio) {
    UNUSED(prio);
    ThreadWrapper* thread_wrapper = malloc(sizeof(ThreadWrapper));
    thread_wrapper->function = thread;
    thread_wrapper->arg = arg;

    thread_wrapper->thread =
        furi_thread_alloc_ex(name, stacksize, sys_thread_wrapper, thread_wrapper);
    furi_thread_start(thread_wrapper->thread);

    sys_thread_t lwip_thread;
    lwip_thread.thread_handle = thread_wrapper;
    return lwip_thread;
}

// netconn per thread semaphores

#define FURI_THREAD_LOCAL_SEM_INDEX 0

sys_sem_t* sys_arch_netconn_sem_get(void) {
    return furi_thread_local_storage_pointer_get(NULL, FURI_THREAD_LOCAL_SEM_INDEX);
}

void sys_arch_netconn_sem_alloc(void) {
    void* ret = furi_thread_local_storage_pointer_get(NULL, FURI_THREAD_LOCAL_SEM_INDEX);
    if(ret == NULL) {
        sys_sem_t* sem;
        err_t err;
        /* need to allocate the memory for this semaphore */
        sem = mem_malloc(sizeof(sys_sem_t));
        LWIP_ASSERT("sem != NULL", sem != NULL);
        err = sys_sem_new(sem, 0);
        LWIP_ASSERT("err == ERR_OK", err == ERR_OK);
        LWIP_ASSERT("sem invalid", sys_sem_valid(sem));
        furi_thread_local_storage_pointer_set(NULL, FURI_THREAD_LOCAL_SEM_INDEX, sem);
    }
}

void sys_arch_netconn_sem_free(void) {
    void* ret = sys_arch_netconn_sem_get();
    if(ret != NULL) {
        sys_sem_t* sem = ret;
        sys_sem_free(sem);
        mem_free(sem);
        furi_thread_local_storage_pointer_set(NULL, FURI_THREAD_LOCAL_SEM_INDEX, sem);
    }
}

void sys_mark_tcpip_thread(void) {
    lwip_tcpip_thread = furi_thread_get_current();
}

void sys_check_core_locking(void) {
    LWIP_ASSERT("Function called from an ISR", !FURI_IS_ISR());
    if(lwip_tcpip_thread != NULL) {
        const FuriThread* current_thread = furi_thread_get_current();
#if LWIP_TCPIP_CORE_LOCKING
        LWIP_ASSERT(
            "Function called without core lock",
            current_thread == furi_mutex_get_owner(lock_tcpip_core.mut));
#else /* LWIP_TCPIP_CORE_LOCKING */
        LWIP_ASSERT("Function called from wrong thread", current_thread == lwip_tcpip_thread);
#endif /* LWIP_TCPIP_CORE_LOCKING */
    }
}

uint32_t lwip_glue_rand(void) {
    // TODO: Better rand() implementation
    return (uint32_t)rand();
}

void lwip_glue_log(const char* fmt, ...) {
    FuriString* string = furi_string_alloc();

    va_list args;
    va_start(args, fmt);
    furi_string_vprintf(string, fmt, args);
    va_end(args);

    furi_string_trim(string, "\r\n");

    if(!furi_string_empty(string)) {
        FURI_LOG_D(TAG, "%s", furi_string_get_cstr(string));
    }

    furi_string_free(string);
}
