/***************************************************************************//**
 * @file
 * @brief Application Runtime Adaptor (RTA) - Runtime Micrium OS implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sl_status.h"
#include "app_rta_internal.h"
#include "app_rta_list.h"
#include "app_rta_runtime.h"
#include "sl_memory_manager.h"
#include "os.h"
#include "os_cfg.h"

// -----------------------------------------------------------------------------
// Definitions

#define SLOT_FREE     0
#define SLOT_RESERVED 1

// Runtime
typedef struct {
  OS_TCB  task_handle;
  CPU_STK *task_stack;
  OS_SEM  *common_semaphore;
} runtime_data_t;

// Queue type
typedef struct {
  OS_Q     *handle;
  uint8_t  *data;
  uint8_t  *slots;
  uint16_t size;
} queue_t;

// Context
typedef struct {
  OS_SEM   *semaphore;
  OS_SEM   *signal;
  OS_MUTEX *mutex;
  queue_t  *queue;
} context_data_t;

// -----------------------------------------------------------------------------
// Forward declaration of private functions

static sl_status_t slot_alloc(queue_t *queue, uint16_t *slot);
static void slot_free(queue_t *queue, uint16_t slot);
static sl_status_t create_queue(app_rta_internal_context_t *ctx);
static sl_status_t create_signal_semaphore(app_rta_internal_context_t *ctx);
static sl_status_t create_mutex(app_rta_internal_context_t *ctx);
static sl_status_t create_proceed_semaphore(app_rta_internal_context_t *ctx);

// -----------------------------------------------------------------------------
// Private variables

// Set to 55-x for Micrium OS
static const uint8_t priority_table[] = {
  47,   // APP_RTA_PRIORITY_LOW
  39,   // APP_RTA_PRIORITY_BELOW_NORMAL
  31,   // APP_RTA_PRIORITY_NORMAL
  23,   // APP_RTA_PRIORITY_ABOVE_NORMAL
  15,   // APP_RTA_PRIORITY_HIGH
};

// -----------------------------------------------------------------------------
// Public functions

sl_status_t app_rta_runtime_init_context(app_rta_internal_context_t *ctx)
{
  sl_status_t sc = SL_STATUS_OK;

  if (ctx->runtime_data != NULL) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  // Allocate context data
  ctx->runtime_data = sl_malloc(sizeof(context_data_t));
  if (ctx->runtime_data == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  memset(ctx->runtime_data, 0, sizeof(context_data_t));

  // Create semaphore for proceed
  if (ctx->config.requirement.runtime
      || ctx->config.requirement.runtime_separate) {
    sc = create_proceed_semaphore(ctx);
    if (sc != SL_STATUS_OK) {
      (void)app_rta_runtime_deinit_context(ctx);
      return sc;
    }
  }
  // Create mutex for guard
  if (ctx->config.requirement.guard) {
    sc = create_mutex(ctx);
    if (sc != SL_STATUS_OK) {
      (void)app_rta_runtime_deinit_context(ctx);
      return sc;
    }
  }
  // Create semaphore for signaling
  if (ctx->config.requirement.signal) {
    sc = create_signal_semaphore(ctx);
    if (sc != SL_STATUS_OK) {
      (void)app_rta_runtime_deinit_context(ctx);
      return sc;
    }
  }
  // Create Queue
  if (ctx->config.requirement.queue) {
    sc = create_queue(ctx);
    if (sc != SL_STATUS_OK) {
      (void)app_rta_runtime_deinit_context(ctx);
      return sc;
    }
  }

  return sc;
}

sl_status_t app_rta_runtime_deinit_context(app_rta_internal_context_t *ctx)
{
  RTOS_ERR err;
  // Check runtime data
  if (ctx->runtime_data == NULL) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  context_data_t *ctx_data = (context_data_t *)ctx->runtime_data;
  // Delete semaphore
  if (ctx_data->semaphore != NULL) {
    OSSemDel(ctx_data->semaphore, OS_OPT_DEL_ALWAYS, &err);
    sl_free(ctx_data->semaphore);
  }
  // Delete mutex
  if (ctx_data->mutex != NULL) {
    OSMutexDel(ctx_data->mutex, OS_OPT_DEL_ALWAYS, &err);
    sl_free(ctx_data->mutex);
  }
  // Delete signal semaphore
  if (ctx_data->signal != NULL) {
    OSSemDel(ctx_data->signal, OS_OPT_DEL_ALWAYS, &err);
    sl_free(ctx_data->signal);
  }
  // Delete queue
  if (ctx_data->queue != NULL) {
    if (ctx_data->queue->handle != NULL) {
      OSQDel(ctx_data->queue->handle, OS_OPT_DEL_ALWAYS, &err);
    }
    if (ctx_data->queue->slots != NULL) {
      sl_free(ctx_data->queue->slots);
    }
    if (ctx_data->queue->data != NULL) {
      sl_free(ctx_data->queue->data);
    }
    sl_free(ctx_data->queue);
  }
  // Delete runtime data
  sl_free(ctx->runtime_data);
  ctx->runtime_data = NULL;

  return SL_STATUS_OK;
}

sl_status_t app_rta_runtime_init(app_rta_runtime_t *runtime)
{
  RTOS_ERR err;
  sl_status_t sc = SL_STATUS_OK;
  // Check runtime data
  if (runtime->runtime_data != NULL) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }
  // Allocate runtime data
  runtime->runtime_data = sl_malloc(sizeof(runtime_data_t));
  if (runtime->runtime_data == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  memset(runtime->runtime_data, 0, sizeof(runtime_data_t));

  runtime_data_t *rt_data = (runtime_data_t *)runtime->runtime_data;
  size_t stack_size = runtime->stack_size;

  // Create common semaphore
  rt_data->common_semaphore = (OS_SEM *)sl_malloc(sizeof(OS_SEM));
  if (rt_data->common_semaphore == NULL) {
    sc = SL_STATUS_ALLOCATION_FAILED;
    goto cleanup;
  }
  OSSemCreate(rt_data->common_semaphore,
              NULL,
              0,
              &err);
  if (err.Code != RTOS_ERR_NONE) {
    sc = SL_STATUS_FAIL;
    goto cleanup;
  }

  // Calculate stack size
  rt_data->task_stack = (CPU_STK *)sl_malloc(stack_size);
  if (rt_data->task_stack == NULL) {
    sc = SL_STATUS_ALLOCATION_FAILED;
    goto cleanup;
  }

  stack_size -= (stack_size % CPU_CFG_STK_ALIGN_BYTES);

  // Create task for runtime with the required stack size and priority
  OSTaskCreate(&rt_data->task_handle,
               NULL,
               runtime_function,
               (void *)runtime,
               priority_table[runtime->priority],
               rt_data->task_stack,
               0u,
               stack_size / sizeof(CPU_STK),
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &err);

  if (err.Code != RTOS_ERR_NONE) {
    sc = SL_STATUS_FAIL;
  }

  cleanup:
  if (sc != SL_STATUS_OK) {
    (void)app_rta_runtime_deinit(runtime);
  }
  return sc;
}

sl_status_t app_rta_runtime_deinit(app_rta_runtime_t *runtime)
{
  RTOS_ERR err;
  // Check runtime data
  if (runtime->runtime_data == NULL) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  runtime_data_t *rt_data = (runtime_data_t *)runtime->runtime_data;

  // Delete common semaphore
  if (rt_data->common_semaphore != NULL) {
    OSSemDel(rt_data->common_semaphore, OS_OPT_DEL_ALWAYS, &err);
    sl_free(rt_data->common_semaphore);
  }

  // Delete task
  if (rt_data->task_stack != NULL) {
    OSTaskDel(&rt_data->task_handle, &err);
    sl_free(rt_data->task_stack);
  }

  sl_free(runtime->runtime_data);
  runtime->runtime_data = NULL;

  return SL_STATUS_OK;
}

sl_status_t app_rta_runtime_proceed(app_rta_runtime_t          *runtime,
                                    app_rta_internal_context_t *ctx)
{
  RTOS_ERR err;
  OS_SEM *semaphore = ((context_data_t *)ctx->runtime_data)->semaphore;
  runtime_data_t *rt_data = (runtime_data_t *)runtime->runtime_data;
  OS_SEM *common_semaphore = rt_data->common_semaphore;
  OSSemPost(semaphore, OS_OPT_POST_ALL, &err);
  if (err.Code != RTOS_ERR_NONE) {
    return SL_STATUS_FAIL;
  }

  OSSemPost(common_semaphore, OS_OPT_POST_ALL, &err);
  if (err.Code != RTOS_ERR_NONE) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

bool app_rta_runtime_check_context(app_rta_internal_context_t *ctx)
{
  RTOS_ERR err;
  OS_SEM *semaphore = ((context_data_t *)ctx->runtime_data)->semaphore;
  OSSemPend(semaphore,
            0,
            OS_OPT_PEND_NON_BLOCKING,
            NULL,
            &err);
  if (err.Code == RTOS_ERR_NONE) {
    return true;
  }
  return false;
}

bool app_rta_runtime_common_check(app_rta_runtime_t *runtime)
{
  RTOS_ERR err;
  OS_SEM *common_semaphore
    = ((runtime_data_t *)runtime->runtime_data)->common_semaphore;
  OSSemPend(common_semaphore,
            0,
            OS_OPT_PEND_BLOCKING,
            NULL,
            &err);
  if (err.Code == RTOS_ERR_NONE) {
    return true;
  }
  return false;
}

bool app_rta_runtime_acquire(app_rta_internal_context_t *ctx)
{
  RTOS_ERR err;
  OS_MUTEX *mutex = ((context_data_t *)ctx->runtime_data)->mutex;

  OSMutexPend(mutex,
              (OS_TICK)ctx->config.wait_for_guard,
              OS_OPT_PEND_BLOCKING,
              DEF_NULL,
              &err);

  if (err.Code != RTOS_ERR_NONE && err.Code != RTOS_ERR_IS_OWNER) {
    return false;
  }
  return true;
}

void app_rta_runtime_release(app_rta_internal_context_t *ctx)
{
  RTOS_ERR err;
  OS_MUTEX *mutex = ((context_data_t *)ctx->runtime_data)->mutex;

  OSMutexPost(mutex,
              OS_OPT_POST_NONE,
              &err);
}

sl_status_t app_rta_runtime_signal(app_rta_internal_context_t *ctx)
{
  RTOS_ERR err;
  sl_status_t sc;
  app_rta_runtime_t *runtime = app_rta_list_find_runtime(ctx);
  OS_SEM *signal_semaphore = ((context_data_t *)ctx->runtime_data)->signal;

  (void)OSSemPost(signal_semaphore, OS_OPT_POST_ALL, &err);
  if (err.Code != RTOS_ERR_NONE) {
    return SL_STATUS_FAIL;
  }

  sc = app_rta_runtime_proceed(runtime, ctx);
  return sc;
}

sl_status_t app_rta_runtime_signal_check(app_rta_internal_context_t *ctx)
{
  RTOS_ERR err;
  OS_SEM *signal_semaphore = ((context_data_t *)ctx->runtime_data)->signal;
  OSSemPend(signal_semaphore,
            0,
            OS_OPT_PEND_NON_BLOCKING,
            NULL,
            &err);
  if (err.Code != RTOS_ERR_NONE) {
    return SL_STATUS_EMPTY;
  }
  return SL_STATUS_OK;
}

sl_status_t app_rta_runtime_queue_push(app_rta_internal_context_t *ctx,
                                       uint8_t                    *data,
                                       size_t                     size)
{
  RTOS_ERR err;
  sl_status_t sc = SL_STATUS_OK;
  app_rta_runtime_t *runtime = app_rta_list_find_runtime(ctx);
  queue_t *queue = ((context_data_t *)ctx->runtime_data)->queue;

  // Allocate a free slot
  uint16_t slot;
  sc =  slot_alloc(queue, &slot);
  if (sc != SL_STATUS_OK) {
    return sc;
  }
  // Copy data
  memcpy(&queue->data[slot * ctx->config.queue_element_size], data, size);

  // Post slot index to the queue
  OSQPost(queue->handle,
          (void *)((uint32_t)slot),
          (OS_MSG_SIZE)size,
          OS_OPT_POST_FIFO | OS_OPT_POST_ALL,
          &err);
  if (err.Code != RTOS_ERR_NONE) {
    // Free up slot in case of error
    slot_free(queue, slot);
    return SL_STATUS_FULL;
  }
  if (sc == SL_STATUS_OK) {
    sc = app_rta_runtime_proceed(runtime,
                                 ctx);
  }
  return sc;
}

sl_status_t app_rta_runtime_queue_read(app_rta_internal_context_t *ctx,
                                       uint8_t                    *data,
                                       size_t                     *size)
{
  RTOS_ERR err;
  uint8_t *item;
  queue_t *queue = ((context_data_t *)ctx->runtime_data)->queue;
  OS_MSG_SIZE item_size = (OS_MSG_SIZE)ctx->config.queue_element_size;
  uint16_t slot;

  // Check the queue
  item = OSQPend(queue->handle,
                 0,
                 OS_OPT_PEND_NON_BLOCKING,
                 &item_size,
                 NULL,
                 &err);

  // Check for size and pointer
  if (item_size == 0) {
    return SL_STATUS_EMPTY;
  }
  slot = (uint16_t)((uint32_t)item);

  // Copy data to result
  memcpy(data, &queue->data[slot * ctx->config.queue_element_size], item_size);
  *size = item_size;

  // Free the slot
  slot_free(queue, slot);
  return SL_STATUS_OK;
}

sl_status_t app_rta_runtime_protected_begin(void)
{
  // Suspend other tasks to protect current context.
  RTOS_ERR err;
  sl_status_t sc = SL_STATUS_OK;
  OSSchedLock(&err);
  if (err.Code != RTOS_ERR_NONE) {
    sc = SL_STATUS_FAIL;
  }
  return sc;
}

// Resume others
sl_status_t app_rta_runtime_protected_end(void)
{
  // Resume other tasks to leave the safe context.
  RTOS_ERR err;
  sl_status_t sc = SL_STATUS_OK;
  OSSchedUnlock(&err);
  if (err.Code != RTOS_ERR_NONE) {
    sc = SL_STATUS_FAIL;
  }
  return sc;
}

// -----------------------------------------------------------------------------
// Private functions

static sl_status_t slot_alloc(queue_t *queue, uint16_t *slot)
{
  for (uint16_t i = 0; i < queue->size; i++) {
    if (queue->slots[i] == SLOT_FREE) {
      queue->slots[i] = SLOT_RESERVED;
      *slot = i;
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_FULL;
}

static void slot_free(queue_t *queue, uint16_t slot)
{
  if (queue->slots[slot] != SLOT_FREE) {
    queue->slots[slot] = SLOT_FREE;
  }
}

static sl_status_t create_proceed_semaphore(app_rta_internal_context_t *ctx)
{
  RTOS_ERR err;
  sl_status_t sc = SL_STATUS_OK;
  context_data_t *ctx_data = (context_data_t *)ctx->runtime_data;

  ctx_data->semaphore = (OS_SEM *)sl_malloc(sizeof(OS_SEM));
  if (ctx_data->semaphore == NULL) {
    (void)app_rta_runtime_deinit_context(ctx);
    return SL_STATUS_ALLOCATION_FAILED;
  }
  OSSemCreate(ctx_data->semaphore,
              NULL,
              0,
              &err);
  if (err.Code != RTOS_ERR_NONE) {
    return SL_STATUS_FAIL;
  }

  return sc;
}

static sl_status_t create_mutex(app_rta_internal_context_t *ctx)
{
  RTOS_ERR err;
  sl_status_t sc = SL_STATUS_OK;
  context_data_t *ctx_data = (context_data_t *)ctx->runtime_data;

  ctx_data->mutex = (OS_MUTEX *)sl_malloc(sizeof(OS_MUTEX));
  if (ctx_data->mutex == NULL) {
    (void)app_rta_runtime_deinit_context(ctx);
    return SL_STATUS_ALLOCATION_FAILED;
  }
  OSMutexCreate(ctx_data->mutex,
                NULL,
                &err);
  if (err.Code != RTOS_ERR_NONE) {
    (void)app_rta_runtime_deinit_context(ctx);
    return SL_STATUS_FAIL;
  }

  return sc;
}

static sl_status_t create_signal_semaphore(app_rta_internal_context_t *ctx)
{
  RTOS_ERR err;
  sl_status_t sc = SL_STATUS_OK;
  context_data_t *ctx_data = (context_data_t *)ctx->runtime_data;

  ctx_data->signal = (OS_SEM *)sl_malloc(sizeof(OS_SEM));
  if (ctx_data->signal == NULL) {
    (void)app_rta_runtime_deinit_context(ctx);
    return SL_STATUS_ALLOCATION_FAILED;
  }
  OSSemCreate(ctx_data->signal,
              NULL,
              0,
              &err);
  if (err.Code != RTOS_ERR_NONE) {
    (void)app_rta_runtime_deinit_context(ctx);
    return SL_STATUS_FAIL;
  }

  return sc;
}

static sl_status_t create_queue(app_rta_internal_context_t *ctx)
{
  RTOS_ERR err;
  sl_status_t sc = SL_STATUS_OK;
  context_data_t *ctx_data = (context_data_t *)ctx->runtime_data;

  // Allocate queue structure
  ctx_data->queue = sl_malloc(sizeof(queue_t));
  if (ctx_data->queue == NULL) {
    (void)app_rta_runtime_deinit_context(ctx);
    return SL_STATUS_ALLOCATION_FAILED;
  }
  memset(ctx_data->queue, 0, sizeof(queue_t));

  // Allocate queue data
  ctx_data->queue->data = sl_malloc(ctx->config.queue_element_size
                                    * ctx->config.queue_size);
  if (ctx_data->queue->data == NULL) {
    (void)app_rta_runtime_deinit_context(ctx);
    return SL_STATUS_ALLOCATION_FAILED;
  }

  // Allocate slots
  ctx_data->queue->slots = sl_malloc(ctx->config.queue_size);
  if (ctx_data->queue->slots == NULL) {
    (void)app_rta_runtime_deinit_context(ctx);
    return SL_STATUS_ALLOCATION_FAILED;
  }
  memset(ctx_data->queue->slots, SLOT_FREE, ctx->config.queue_size);

  // Allocate OS queue
  ctx_data->queue->handle = (OS_Q *)sl_malloc(sizeof(OS_Q));
  if (ctx_data->queue->handle == NULL) {
    (void)app_rta_runtime_deinit_context(ctx);
    return SL_STATUS_ALLOCATION_FAILED;
  }
  // Create OS queue
  OSQCreate(ctx_data->queue->handle,
            NULL,
            ctx->config.queue_size,
            &err);

  if (err.Code != RTOS_ERR_NONE) {
    (void)app_rta_runtime_deinit_context(ctx);
    return SL_STATUS_FAIL;
  }
  // Set size
  ctx_data->queue->size = ctx->config.queue_size;

  return sc;
}
