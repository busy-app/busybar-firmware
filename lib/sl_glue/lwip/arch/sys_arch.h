#ifndef CHIP_LWIP_FREERTOS_ARCH_SYS_ARCH_H
#define CHIP_LWIP_FREERTOS_ARCH_SYS_ARCH_H

#include <cmsis_os2.h>

#include "arch/sys_arch.h"
#include "lwip/opt.h"
//#include "lwip/sys.h"

#ifdef mem_clib_free
#undef mem_clib_free
#endif

#ifdef mem_clib_malloc
#undef mem_clib_malloc
#endif

#ifndef LWIP_COMPAT_MUTEX
#define LWIP_COMPAT_MUTEX 0
#endif

/** This is returned by _fromisr() sys functions to tell the outermost function
 * that a higher priority task was woken and the scheduler needs to be invoked.
 */
#define ERR_NEED_SCHED 123

/* This port includes FreeRTOS headers in sys_arch.c only.
 *  FreeRTOS uses pointers as object types. We use wrapper structs instead of
 * void pointers directly to get a tiny bit of type safety.
 */
void sys_arch_msleep(uint32_t delay_ms);
#define sys_msleep(ms) sys_arch_msleep(ms)

#if SYS_LIGHTWEIGHT_PROT
typedef uint32_t sys_prot_t;
#endif /* SYS_LIGHTWEIGHT_PROT */

#if !LWIP_COMPAT_MUTEX

typedef struct {
    FuriMutex* mut;
} sys_mutex_t;

#define sys_mutex_valid_val(mutex)   ((mutex).mut != NULL)
#define sys_mutex_valid(mutex)       (((mutex) != NULL) && sys_mutex_valid_val(*(mutex)))
#define sys_mutex_set_invalid(mutex) ((mutex)->mut = NULL)
#endif

typedef struct {
    FuriSemaphore* sem;
} sys_sem_t;
#define sys_sem_valid_val(sema)   ((sema).sem != NULL)
#define sys_sem_valid(sema)       (((sema) != NULL) && sys_sem_valid_val(*(sema)))
#define sys_sem_set_invalid(sema) ((sema)->sem = NULL)

typedef struct {
    FuriMessageQueue* mbx;
} sys_mbox_t;

#define sys_mbox_valid_val(mbox)   ((mbox).mbx != NULL)
#define sys_mbox_valid(mbox)       (((mbox) != NULL) && sys_mbox_valid_val(*(mbox)))
#define sys_mbox_set_invalid(mbox) ((mbox)->mbx = NULL)

typedef struct ThreadWrapper ThreadWrapper;
typedef struct {
    ThreadWrapper* thread_handle;
} sys_thread_t;

#if LWIP_NETCONN_SEM_PER_THREAD
sys_sem_t* sys_arch_netconn_sem_get(void);
void sys_arch_netconn_sem_alloc(void);
void sys_arch_netconn_sem_free(void);
#define LWIP_NETCONN_THREAD_SEM_GET()   sys_arch_netconn_sem_get()
#define LWIP_NETCONN_THREAD_SEM_ALLOC() sys_arch_netconn_sem_alloc()
#define LWIP_NETCONN_THREAD_SEM_FREE()  sys_arch_netconn_sem_free()
#endif /* LWIP_NETCONN_SEM_PER_THREAD */

#define LWIP_RAND() ((uint32_t)rand())

#define sys_profile_interval_set_pbuf_highwatermark(...)

#endif /* CHIP_LWIP_FREERTOS_ARCH_SYS_ARCH_H */
