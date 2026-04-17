/***************************************************************************//**
 * @file
 * @brief Application Runtime Adaptor (RTA) - Runtime FreeRTOS implementation
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
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#if !defined(configSUPPORT_DYNAMIC_ALLOCATION) || configSUPPORT_DYNAMIC_ALLOCATION != 1
#error "Application Runtime Adaptor (RTA): Please enable configSUPPORT_DYNAMIC_ALLOCATION in FreeRTOSConfig.h"
#endif
#if !defined(configUSE_RECURSIVE_MUTEXES) || configUSE_RECURSIVE_MUTEXES != 1
#error "Application Runtime Adaptor (RTA): Please enable configSUPPORT_DYNAMIC_ALLOCATION in FreeRTOSConfig.h"
#endif
#if !defined(configUSE_TASK_NOTIFICATIONS) || configUSE_TASK_NOTIFICATIONS != 1
#error "Application Runtime Adaptor (RTA): Please enable configUSE_TASK_NOTIFICATIONS in FreeRTOSConfig.h"
#endif

// Runtime
typedef struct {
  TaskHandle_t task_handle;
  SemaphoreHandle_t common_semaphore;
} runtime_data_t;

// Context
typedef struct {
  SemaphoreHandle_t     signal_semaphore;
  SemaphoreHandle_t     semaphore;
  SemaphoreHandle_t     mutex;
  QueueHandle_t         queue;
} context_data_t;

static const uint8_t priority_table[] = {
  8,    // APP_RTA_PRIORITY_LOW
  16,   // APP_RTA_PRIORITY_BELOW_NORMAL
  24,   // APP_RTA_PRIORITY_NORMAL
  32,   // APP_RTA_PRIORITY_ABOVE_NORMAL
  40,   // APP_RTA_PRIORITY_HIGH
};

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

  context_data_t *ctx_data = (context_data_t *)ctx->runtime_data;

  // Create semaphore for proceed
  if (ctx->config.requirement.runtime
      || ctx->config.requirement.runtime_separate ) {
    ctx_data->semaphore = xSemaphoreCreateCounting(UINT16_MAX, 0);
    if (ctx_data->semaphore == NULL) {
      sc = SL_STATUS_ALLOCATION_FAILED;
      goto cleanup;
    }
  }
  // Create mutex for quard
  if (ctx->config.requirement.guard) {
    ctx_data->mutex = xSemaphoreCreateRecursiveMutex();
    if (ctx_data->mutex == NULL) {
      sc = SL_STATUS_ALLOCATION_FAILED;
      goto cleanup;
    }
  }
  // Create semaphore for signal
  if (ctx->config.requirement.signal) {
    ctx_data->signal_semaphore = xSemaphoreCreateCounting(UINT16_MAX, 0);
    if (ctx_data->signal_semaphore == NULL) {
      sc = SL_STATUS_ALLOCATION_FAILED;
      goto cleanup;
    }
  }
  // Create Queue
  if (ctx->config.requirement.queue) {
    ctx_data->queue = xQueueCreate(ctx->config.queue_size,
                                   ctx->config.queue_element_size);
    if (ctx_data->queue == NULL) {
      sc = SL_STATUS_ALLOCATION_FAILED;
      goto cleanup;
    }
  }
  // Note: ISR signaling requires task notification
  cleanup:
  if (sc != SL_STATUS_OK) {
    (void)app_rta_runtime_deinit_context(ctx);
  }
  return sc;
}

sl_status_t app_rta_runtime_deinit_context(app_rta_internal_context_t *ctx)
{
  // Check runtime data
  if (ctx->runtime_data == NULL) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  context_data_t *ctx_data = (context_data_t *)ctx->runtime_data;
  // Delete semaphore
  if (ctx_data->semaphore != NULL) {
    vSemaphoreDelete(ctx_data->semaphore);
  }
  // Delete queue
  if (ctx_data->queue != NULL) {
    vQueueDelete(ctx_data->queue);
  }
  // Delete mutex
  if (ctx_data->mutex != NULL) {
    vSemaphoreDelete(ctx_data->mutex);
  }
  // Delete mutex
  if (ctx_data->signal_semaphore != NULL) {
    vSemaphoreDelete(ctx_data->signal_semaphore);
  }
  // Delete runtime data
  sl_free(ctx->runtime_data);
  ctx->runtime_data = NULL;

  return SL_STATUS_OK;
}

sl_status_t app_rta_runtime_init(app_rta_runtime_t *runtime)
{
  BaseType_t ret;
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

  // Calculate stack size
  const configSTACK_DEPTH_TYPE stack_size
    = (configSTACK_DEPTH_TYPE)(runtime->stack_size / sizeof(configSTACK_DEPTH_TYPE));

  // Create common semaphore
  rt_data->common_semaphore = xSemaphoreCreateCounting(UINT16_MAX, 0);
  if (rt_data->common_semaphore == NULL) {
    sc = SL_STATUS_ALLOCATION_FAILED;
    goto cleanup;
  }

  // Create task for runtime with the required stack size and priority
  ret = xTaskCreate(runtime_function,
                    NULL,
                    stack_size,
                    (void *)runtime,
                    priority_table[runtime->priority],
                    &rt_data->task_handle);
  if (ret != pdPASS) {
    sc = SL_STATUS_ALLOCATION_FAILED;
  }

  cleanup:
  if (sc != SL_STATUS_OK) {
    (void)app_rta_runtime_deinit(runtime);
  }
  return sc;
}

sl_status_t app_rta_runtime_deinit(app_rta_runtime_t *runtime)
{
  // Check runtime data
  if (runtime->runtime_data == NULL) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  runtime_data_t *rt_data = (runtime_data_t *)runtime->runtime_data;

  // Delete task
  if (rt_data->task_handle != NULL) {
    if (rt_data->task_handle == xTaskGetCurrentTaskHandle()) {
      // Delete this task.
      vTaskDelete(NULL);
    } else {
      // Delete the task.
      vTaskDelete(rt_data->task_handle);
    }
  }

  // Delete common semaphore
  if (rt_data->common_semaphore != NULL) {
    vSemaphoreDelete(rt_data->common_semaphore);
  }

  // Free up space
  sl_free(runtime->runtime_data);
  runtime->runtime_data = NULL;

  return SL_STATUS_OK;
}

sl_status_t app_rta_runtime_proceed(app_rta_runtime_t          *runtime,
                                    app_rta_internal_context_t *ctx)
{
  BaseType_t result;
  BaseType_t  higher_prio_task_woken = pdFALSE;

  SemaphoreHandle_t semaphore
    = ((context_data_t *)ctx->runtime_data)->semaphore;
  SemaphoreHandle_t common_semaphore
    = ((runtime_data_t *)runtime->runtime_data)->common_semaphore;

  if (xPortIsInsideInterrupt()) {
    // Interrupt context
    result = xSemaphoreGiveFromISR(semaphore, &higher_prio_task_woken);
    if (result == pdTRUE) {
      result = xSemaphoreGiveFromISR(common_semaphore, &higher_prio_task_woken);
    }
    portYIELD_FROM_ISR(higher_prio_task_woken);
    if (result != pdTRUE) {
      return SL_STATUS_FAIL;
    }
  } else {
    // Non-interrupt context
    result = xSemaphoreGive(semaphore);
    if (result == pdTRUE) {
      result = xSemaphoreGive(common_semaphore);
    }
    if (result != pdTRUE) {
      return SL_STATUS_FAIL;
    }
  }
  return SL_STATUS_OK;
}

bool app_rta_runtime_check_context(app_rta_internal_context_t *ctx)
{
  SemaphoreHandle_t semaphore
    = ((context_data_t *)ctx->runtime_data)->semaphore;
  if (xSemaphoreTake(semaphore, 0) == pdTRUE) {
    return true;
  }
  return false;
}

bool app_rta_runtime_common_check(app_rta_runtime_t *runtime)
{
  SemaphoreHandle_t common_semaphore
    = ((runtime_data_t *)runtime->runtime_data)->common_semaphore;
  if (xSemaphoreTake(common_semaphore, portMAX_DELAY) == pdTRUE) {
    return true;
  }
  return false;
}

bool app_rta_runtime_acquire(app_rta_internal_context_t *ctx)
{
  SemaphoreHandle_t mutex
    = ((context_data_t *)ctx->runtime_data)->mutex;
  BaseType_t response;

  response = xSemaphoreTakeRecursive(mutex,
                                     (TickType_t)ctx->config.wait_for_guard);
  if (response != pdTRUE) {
    return false;
  } else {
    return true;
  }
}

void app_rta_runtime_release(app_rta_internal_context_t *ctx)
{
  SemaphoreHandle_t mutex
    = ((context_data_t *)ctx->runtime_data)->mutex;
  (void)xSemaphoreGiveRecursive(mutex);
}

sl_status_t app_rta_runtime_signal(app_rta_internal_context_t *ctx)
{
  sl_status_t sc = SL_STATUS_OK;
  BaseType_t  result;
  BaseType_t  higher_prio_task_woken = pdFALSE;
  SemaphoreHandle_t signal_semaphore
    = ((context_data_t *)ctx->runtime_data)->signal_semaphore;
  app_rta_runtime_t *runtime = app_rta_list_find_runtime(ctx);
  if (xPortIsInsideInterrupt()) {
    result = xSemaphoreGiveFromISR(signal_semaphore, &higher_prio_task_woken);
    portYIELD_FROM_ISR(higher_prio_task_woken);
    if (result != pdTRUE) {
      sc = SL_STATUS_FAIL;
    }
  } else {
    result = xSemaphoreGive(signal_semaphore);
    if (result != pdTRUE) {
      sc = SL_STATUS_FAIL;
    }
  }
  if (sc == SL_STATUS_OK) {
    sc = app_rta_runtime_proceed(runtime, ctx);
  }
  return sc;
}

sl_status_t app_rta_runtime_signal_check(app_rta_internal_context_t *ctx)
{
  BaseType_t  result;
  SemaphoreHandle_t signal_semaphore
    = ((context_data_t *)ctx->runtime_data)->signal_semaphore;
  result = xSemaphoreTake(signal_semaphore, 0);
  if (result != pdTRUE) {
    return SL_STATUS_EMPTY;
  }
  return SL_STATUS_OK;
}

sl_status_t app_rta_runtime_queue_push(app_rta_internal_context_t *ctx,
                                       uint8_t                    *data,
                                       size_t                     size)
{
  (void)size;
  sl_status_t sc                     = SL_STATUS_OK;
  BaseType_t  higher_prio_task_woken = pdFALSE;
  BaseType_t  result;

  app_rta_runtime_t *runtime = app_rta_list_find_runtime(ctx);
  QueueHandle_t queue_handle = ((context_data_t *)ctx->runtime_data)->queue;

  if (xPortIsInsideInterrupt()) {
    result = xQueueSendToBackFromISR(queue_handle,
                                     (void *)data,
                                     &higher_prio_task_woken);
    if ( result != pdTRUE ) {
      sc = SL_STATUS_FULL;
    }
    portYIELD_FROM_ISR(higher_prio_task_woken);
  } else {
    result = xQueueSendToBack(queue_handle,
                              (void *)data,
                              ctx->config.wait_for_guard);
    if ( result != pdTRUE ) {
      sc = SL_STATUS_FULL;
    }
  }
  if (sc == SL_STATUS_OK) {
    sc = app_rta_runtime_proceed(runtime, ctx);
  }
  return sc;
}

sl_status_t app_rta_runtime_queue_read(app_rta_internal_context_t *ctx,
                                       uint8_t                    *data,
                                       size_t                     *size)
{
  BaseType_t    result;
  QueueHandle_t queue_handle = ((context_data_t *)ctx->runtime_data)->queue;

  result = xQueueReceive(queue_handle,
                         (void *)data,
                         0);
  if (result != pdTRUE) {
    return SL_STATUS_EMPTY;
  }
  // Set size output on success
  *size = ctx->config.queue_element_size;
  return SL_STATUS_OK;
}

sl_status_t app_rta_runtime_protected_begin(void)
{
  // Suspend other tasks to protect current context.
  vTaskSuspendAll();
  return SL_STATUS_OK;
}

// Resume others
sl_status_t app_rta_runtime_protected_end(void)
{
  // Resume other tasks to leave the safe context.
  (void)xTaskResumeAll();
  return SL_STATUS_OK;
}
