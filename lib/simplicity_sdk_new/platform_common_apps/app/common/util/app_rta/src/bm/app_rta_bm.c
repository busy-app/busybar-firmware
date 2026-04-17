/***************************************************************************//**
 * @file
 * @brief Application Runtime Adaptor implementation for bare-metal
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "app_rta.h"
#include "app_rta_internal_bm.h"
#include "sl_core.h"
#include "app_queue.h"
#include "sl_slist.h"
#include "sl_memory_manager.h"
#include "sl_power_manager.h"
#include "sl_component_catalog.h"

// -----------------------------------------------------------------------------
// Definitions

// Queue type
typedef struct {
  app_queue_t      queue;
  uint8_t          queue_data[];
} app_rta_queue_t;

// Internal context type
typedef struct {
  sl_slist_node_t  node;
  app_rta_config_t config;
  bool             enabled;
  uint16_t         signal;
  app_rta_queue_t  *queue_handle;
  uint32_t         run;
} app_rta_internal_context_t;

// -----------------------------------------------------------------------------
// Forward declaration of private functions

static sl_status_t enable_context(app_rta_internal_context_t *ctx_int,
                                  bool                       enable);

// -----------------------------------------------------------------------------
// Private variables

// Context list
static sl_slist_node_t *ctx_list = NULL;

// -----------------------------------------------------------------------------
// Public functions

sl_status_t app_rta_create_context(app_rta_config_t *config,
                                   app_rta_context_t *ctx_out)
{
  sl_status_t sc = SL_STATUS_OK;
  // Check for config and output references
  if ((config == NULL) || (ctx_out == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }
  // Check for runtime requirement and step function
  if (config->requirement.runtime
      || config->requirement.runtime_separate) {
    if (config->step == NULL) {
      // Runtime was requested but no step function was specified.
      return SL_STATUS_INVALID_PARAMETER;
    }
  } else {
    // Runtime was not requested but a valid step function was passed.
    if (config->step != NULL) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }
  // Check for guard mutex
  if (config->requirement.guard && (config->wait_for_guard == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Check for queue size and element size
  if (config->requirement.queue
      && ((config->queue_size == 0) || (config->queue_element_size == 0))) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Allocate context storage
  app_rta_internal_context_t *ctx
    = (app_rta_internal_context_t *)sl_malloc(sizeof(app_rta_internal_context_t));
  if (ctx == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  // Clear area
  memset(ctx, 0, sizeof(app_rta_internal_context_t));
  // Save config
  memcpy(&ctx->config, config, sizeof(ctx->config));

  if (!config->requirement.start_disabled) {
    sc = enable_context(ctx, true);
  }
  if (sc == SL_STATUS_OK) {
    *ctx_out = (app_rta_context_t)ctx;
    // Check for runtime requirement (has step function)
    if (config->step != NULL) {
      // Add to the list only if runtime was requested.
      sl_slist_push_back(&ctx_list, &ctx->node);
    }
  } else {
    // Free up context
    sl_free(ctx);
  }
  return sc;
}

bool app_rta_is_enabled(app_rta_context_t ctx)
{
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return false;
  }
  return ctx_int->enabled;
}

sl_status_t app_rta_enable_context(app_rta_context_t ctx, bool enable)
{
  sl_status_t sc;
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (enable == ctx_int->enabled) {
    return SL_STATUS_INVALID_STATE;
  }

  sc = enable_context(ctx_int, enable);
  return sc;
}

sl_status_t app_rta_proceed(app_rta_context_t ctx)
{
  sl_status_t sc = SL_STATUS_OK;
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  // Check context handle
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  // Check runtime support
  if (!ctx_int->config.requirement.runtime
      && !ctx_int->config.requirement.runtime_separate) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Check enabled state
  if (!ctx_int->enabled) {
    return SL_STATUS_INVALID_STATE;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  if (ctx_int->run == UINT32_MAX) {
    sc = SL_STATUS_FAIL;
  } else {
    ctx_int->run++;
  }
  CORE_EXIT_CRITICAL();

  return sc;
}

sl_status_t app_rta_acquire(app_rta_context_t ctx)
{
  // Check context handle
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  // Check mutex support
  if (!((app_rta_internal_context_t *)ctx)->config.requirement.guard) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Check enabled state
  if (!(((app_rta_internal_context_t *)ctx)->enabled)) {
    return SL_STATUS_INVALID_STATE;
  }
  // There are no tasks to protect shared resources from.
  return SL_STATUS_OK;
}

sl_status_t app_rta_release(app_rta_context_t ctx)
{
  // Check context handle
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  // Check mutex support
  if (!((app_rta_internal_context_t *)ctx)->config.requirement.guard) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Check enabled state
  if (!(((app_rta_internal_context_t *)ctx)->enabled)) {
    return SL_STATUS_INVALID_STATE;
  }
  // There are no tasks to protect shared resources from.
  return SL_STATUS_OK;
}

sl_status_t app_rta_signal(app_rta_context_t ctx)
{
  sl_status_t sc = SL_STATUS_OK;
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  // Check context handle
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  // Check signal support
  if (!ctx_int->config.requirement.signal) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Check enabled state
  if (!ctx_int->enabled) {
    return SL_STATUS_INVALID_STATE;
  }
  // Set signal and proceed
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  // Check overflow
  if (ctx_int->signal == UINT16_MAX) {
    CORE_EXIT_CRITICAL();
    return SL_STATUS_WOULD_OVERFLOW;
  }
  ctx_int->signal++;
  // Proceed
  if (sc == SL_STATUS_OK) {
    if (ctx_int->run == UINT32_MAX) {
      sc = SL_STATUS_FAIL;
    } else {
      ctx_int->run++;
    }
  }
  CORE_EXIT_CRITICAL();
  return sc;
}

sl_status_t app_rta_signal_check(app_rta_context_t ctx)
{
  sl_status_t sc;
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  // Check context handle
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  // Check ISR signal support
  if (!ctx_int->config.requirement.signal) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Check enabled state
  if (!ctx_int->enabled) {
    return SL_STATUS_INVALID_STATE;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  if (ctx_int->signal) {
    ctx_int->signal--;
    sc = SL_STATUS_OK;
  } else {
    sc = SL_STATUS_EMPTY;
  }
  CORE_EXIT_CRITICAL();

  return sc;
}

sl_status_t app_rta_signal_check_and_acquire(app_rta_context_t ctx)
{
  sl_status_t sc;
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  // Check context handle
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  // Check ISR signal support
  if (!ctx_int->config.requirement.signal) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Check guard support
  if (!ctx_int->config.requirement.guard) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Check enabled state
  if (!ctx_int->enabled) {
    return SL_STATUS_INVALID_STATE;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  // Check signal
  if (ctx_int->signal) {
    ctx_int->signal--;
    sc = SL_STATUS_OK;
  } else {
    sc = SL_STATUS_EMPTY;
  }
  CORE_EXIT_CRITICAL();
  return sc;
}

sl_status_t app_rta_queue_push(app_rta_context_t ctx,
                               uint8_t           *data,
                               size_t            size)
{
  sl_status_t sc = SL_STATUS_OK;
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  // Check context handle
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  // Check data and size
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Check queue support
  if (!ctx_int->config.requirement.queue) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  if (size > ctx_int->config.queue_element_size) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Check enabled state
  if (!ctx_int->enabled) {
    return SL_STATUS_INVALID_STATE;
  }

  // Add data to the queue
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  sc = app_queue_add(&ctx_int->queue_handle->queue, data);
  // Proceed
  if (sc == SL_STATUS_OK) {
    if (ctx_int->run == UINT32_MAX) {
      sc = SL_STATUS_FAIL;
    } else {
      ctx_int->run++;
    }
  }
  CORE_EXIT_CRITICAL();
  return sc;
}

sl_status_t app_rta_queue_read(app_rta_context_t ctx,
                               uint8_t           *data,
                               size_t            *size)
{
  sl_status_t sc = SL_STATUS_OK;
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  // Check context handle
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  // Check data and size parameters
  if (data == NULL || size == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Check for queue support
  if (!ctx_int->config.requirement.queue) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Check enabled state
  if (!ctx_int->enabled) {
    return SL_STATUS_INVALID_STATE;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  sc = app_queue_remove(&ctx_int->queue_handle->queue, data);
  if (sc == SL_STATUS_OK) {
    *size = ctx_int->config.queue_element_size;
  }
  CORE_EXIT_CRITICAL();
  return sc;
}

sl_status_t app_rta_queue_read_and_acquire(app_rta_context_t ctx,
                                           uint8_t           *data,
                                           size_t            *size)
{
  sl_status_t sc = SL_STATUS_OK;
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  // Check context handle
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  // Check data and size parameters
  if (data == NULL || size == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Check for queue support
  if (!ctx_int->config.requirement.queue) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Check for guard support
  if (!ctx_int->config.requirement.guard) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Check enabled state
  if (!ctx_int->enabled) {
    return SL_STATUS_INVALID_STATE;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  sc = app_queue_remove(&ctx_int->queue_handle->queue, data);
  if (sc == SL_STATUS_OK) {
    *size = ctx_int->config.queue_element_size;
  }
  CORE_EXIT_CRITICAL();
  return sc;
}

// -----------------------------------------------------------------------------
// Internal functions

void app_rta_internal_init(void)
{
  // Initialize list
  sl_slist_init(&ctx_list);
  // Initialize contributors
  app_rta_init_contributors();
#ifndef SL_CATALOG_SL_MAIN_PRESENT
  // sl_system is used. There is only one init event and app_rta_internal_init
  // is subscribed to it. Invoke app_rta_ready().
  app_rta_ready();
#endif // SL_CATALOG_SL_MAIN_PRESENT
}

bool app_rta_is_ok_to_sleep(void)
{
  sl_slist_node_t *ctx_iterator;

  SL_SLIST_FOR_EACH(ctx_list, ctx_iterator) {
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_CRITICAL();
    app_rta_internal_context_t *ctx = (app_rta_internal_context_t *)ctx_iterator;
    // Check context
    if (ctx->enabled && (ctx->run > 0)) {
      CORE_EXIT_CRITICAL();
      return false;
    }
    CORE_EXIT_CRITICAL();
  }
  return true;
}

sl_power_manager_on_isr_exit_t app_rta_on_isr_exit(void)
{
  sl_slist_node_t *ctx_iterator;

  SL_SLIST_FOR_EACH(ctx_list, ctx_iterator) {
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_CRITICAL();
    app_rta_internal_context_t *ctx = (app_rta_internal_context_t *)ctx_iterator;
    // Check context
    if (ctx->enabled && (ctx->run > 0)) {
      CORE_EXIT_CRITICAL();
      return SL_POWER_MANAGER_WAKEUP;
    }
    CORE_EXIT_CRITICAL();
  }

  return SL_POWER_MANAGER_IGNORE;
}

void app_rta_step(void)
{
  sl_slist_node_t *ctx_iterator;
  // Iterate over context list.
  SL_SLIST_FOR_EACH(ctx_list, ctx_iterator) {
    app_rta_internal_context_t *ctx
      = (app_rta_internal_context_t *)ctx_iterator;
    // Disable interrupts.
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_CRITICAL();
    // Check context.
    if ((ctx->enabled) && (ctx->run > 0)) {
      ctx->run--;
      // Enable interrupts.
      CORE_EXIT_CRITICAL();
      // Execute step function of the context.
      ctx->config.step();
    } else {
      // Enable interrupts, no execution is required.
      CORE_EXIT_CRITICAL();
    }
  }
}

// -----------------------------------------------------------------------------
// Private functions

static sl_status_t enable_context(app_rta_internal_context_t *ctx_int,
                                  bool                       enable)
{
  sl_status_t sc = SL_STATUS_OK;
  if (!enable) {
    if (ctx_int->config.requirement.queue) {
      // Free up queue storage
      sl_free(ctx_int->queue_handle);
    }
    ctx_int->enabled = false;
    return sc;
  }
  // Context enabling requested
  if (ctx_int->config.requirement.queue) {
    size_t size = sizeof(app_rta_queue_t)
                  + ctx_int->config.queue_size * ctx_int->config.queue_element_size;
    ctx_int->queue_handle = (app_rta_queue_t *)sl_malloc(size);
    if (ctx_int->queue_handle == NULL) {
      sc = SL_STATUS_ALLOCATION_FAILED;
    } else {
      // Init context variables
      ctx_int->run = 0;
      ctx_int->signal = 0u;
      // Initialize queue
      sc = app_queue_init(&ctx_int->queue_handle->queue,
                          ctx_int->config.queue_size,
                          (uint16_t)ctx_int->config.queue_element_size,
                          ctx_int->queue_handle->queue_data);
      if (sc != SL_STATUS_OK) {
        // Free up queue storage
        sl_free(ctx_int->queue_handle);
      }
    }
  }
  if (sc == SL_STATUS_OK) {
    ctx_int->enabled = true;
  }
  return sc;
}
