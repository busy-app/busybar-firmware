/***************************************************************************//**
 * @file
 * @brief Application Runtime Adaptor (RTA) Core implementation for RTOS
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
#include "sl_core.h"
#include "sl_status.h"
#include "app_rta.h"
#include "app_rta_internal.h"
#include "app_rta_runtime.h"
#include "app_rta_list.h"
#include "sl_memory_manager.h"
#include "sl_component_catalog.h"

// -----------------------------------------------------------------------------
// Private variables

#ifndef SL_CATALOG_SL_MAIN_PRESENT
/// Flag indicating if the scheduler has started or not
static bool scheduler_started = false;
#endif // SL_CATALOG_SL_MAIN_PRESENT

// -----------------------------------------------------------------------------
// Public functions

sl_status_t app_rta_create_context(app_rta_config_t  *config,
                                   app_rta_context_t *ctx_out)
{
  sl_status_t sc = SL_STATUS_OK;
  // Check for config and output references
  if ((config == NULL) || (ctx_out == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }
  // Check for runtime requirement and step function
  if ((config->requirement.runtime)
      || (config->requirement.runtime_separate)) {
    if (config->step == NULL || config->stack_size == 0) {
      // Runtime was requested but no step function was specified.
      return SL_STATUS_INVALID_PARAMETER;
    }
  } else {
    // Runtime was not requested but a valid step function was passed.
    if (config->step != NULL) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }
  // Check for guard
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
  // Clear context
  memset(ctx, 0, sizeof(app_rta_internal_context_t));
  // Set config for the context
  memcpy(&ctx->config, config, sizeof(ctx->config));

  // Check if enabling is required or not
  if (!ctx->config.requirement.start_disabled) {
    // Initialize runtime data of the context
    sc = app_rta_runtime_init_context(ctx);
    if (sc != SL_STATUS_OK) {
      goto cleanup;
    }
    ctx->enabled = true;
  }

  // Check if a runtime is required to add
  if (ctx->config.requirement.runtime
      || ctx->config.requirement.runtime_separate) {
    // Add the context to the list
    sc = app_rta_list_add_context(ctx);
  }

  cleanup:
  if (sc == SL_STATUS_OK) {
    // Output context handle
    *ctx_out = ctx;
  } else {
    sl_free(ctx);
  }

  return sc;
}

sl_status_t app_rta_enable_context(app_rta_context_t ctx, bool enable)
{
  sl_status_t sc;
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (ctx_int->enabled == enable) {
    return SL_STATUS_INVALID_STATE;
  }
  // Enable/disable context
  sc = app_rta_list_enable_context(ctx_int, enable);

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

sl_status_t app_rta_proceed(app_rta_context_t ctx)
{
  sl_status_t sc;
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;

  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }

  // Find runtime for the context
  app_rta_runtime_t *rt = app_rta_list_find_runtime(ctx_int);
  if (rt == NULL) {
    return SL_STATUS_NOT_FOUND;
  }

  if (ctx_int->enabled == false) {
    return SL_STATUS_INVALID_STATE;
  }

  // Check runtime
  if (!ctx_int->config.requirement.runtime
      && !ctx_int->config.requirement.runtime_separate) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Do a proceed on runtime and context
  sc = app_rta_runtime_proceed(rt, ctx_int);
  return sc;
}

sl_status_t app_rta_acquire(app_rta_context_t ctx)
{
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;

  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (!ctx_int->enabled) {
    return SL_STATUS_INVALID_STATE;
  }
  // Check guard
  if (!ctx_int->config.requirement.guard) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (!app_rta_runtime_acquire(ctx_int)) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

sl_status_t app_rta_release(app_rta_context_t ctx)
{
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;

  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (!ctx_int->enabled) {
    return SL_STATUS_INVALID_STATE;
  }
  if (!ctx_int->config.requirement.guard) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  app_rta_runtime_release(ctx_int);
  return SL_STATUS_OK;
}

sl_status_t app_rta_signal(app_rta_context_t ctx)
{
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (!ctx_int->enabled) {
    return SL_STATUS_INVALID_STATE;
  }
  // Check signal
  if (!ctx_int->config.requirement.signal) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Emit signal
  return app_rta_runtime_signal(ctx_int);
}

sl_status_t app_rta_signal_check(app_rta_context_t ctx)
{
  sl_status_t sc;
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (!ctx_int->enabled) {
    return SL_STATUS_INVALID_STATE;
  }
  // Check signal
  if (!ctx_int->config.requirement.signal) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Check for signal
  sc = app_rta_runtime_signal_check(ctx_int);
  return sc;
}

sl_status_t app_rta_signal_check_and_acquire(app_rta_context_t ctx)
{
  sl_status_t sc;
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  // Check for guard and signal
  if (!ctx_int->config.requirement.guard
      || !ctx_int->config.requirement.signal) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Check for the signal
  sc = app_rta_signal_check(ctx);
  if (sc == SL_STATUS_OK) {
    // Acquire guard
    if (!app_rta_runtime_acquire(ctx_int)) {
      return SL_STATUS_FAIL;
    }
  }
  return sc;
}

sl_status_t app_rta_queue_push(app_rta_context_t ctx, uint8_t *data, size_t size)
{
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (!ctx_int->enabled) {
    return SL_STATUS_INVALID_STATE;
  }
  // Check queue
  if (!ctx_int->config.requirement.queue) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  if (size != ctx_int->config.queue_element_size) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Push to the queue
  return app_rta_runtime_queue_push(ctx_int, data, size);
}

sl_status_t app_rta_queue_read(app_rta_context_t ctx,
                               uint8_t           *data,
                               size_t            *size)
{
  sl_status_t sc;
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (data == NULL || size == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (!ctx_int->enabled) {
    return SL_STATUS_INVALID_STATE;
  }
  // Check for queue
  if (!ctx_int->config.requirement.queue) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  if (CORE_InIrqContext()) {
    return SL_STATUS_ISR;
  }
  // Read the queue
  sc = app_rta_runtime_queue_read(ctx_int, data, size);
  return sc;
}

sl_status_t app_rta_queue_read_and_acquire(app_rta_context_t ctx,
                                           uint8_t           *data,
                                           size_t            *size)
{
  sl_status_t sc;
  app_rta_internal_context_t *ctx_int = (app_rta_internal_context_t *)ctx;
  if (ctx == APP_RTA_INVALID_CONTEXT) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (data == NULL || size == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Check guard and queue
  if (!ctx_int->config.requirement.guard
      || !ctx_int->config.requirement.queue) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Read the queue
  sc = app_rta_queue_read(ctx, data, size);
  if (sc == SL_STATUS_OK) {
    // Acquire guard
    if (!app_rta_runtime_acquire(ctx_int)) {
      return SL_STATUS_FAIL;
    }
  }
  return sc;
}

// Starting point
void app_rta_internal_init(void)
{
  // Initialize runtime list
  app_rta_list_init();

  // Initialize contributors
  app_rta_init_contributors();

  // Notify list complete: start runtime environments
  app_rta_list_complete();
}

void runtime_function(void *arg)
{
#ifndef SL_CATALOG_SL_MAIN_PRESENT
  // sl_system is used, so there is only one init event.
  // Signal to the contributors that the scheduler has started
  sl_status_t sc;
  sc = app_rta_runtime_protected_begin();
  if (sc == SL_STATUS_OK) {
    if (!scheduler_started) {
      scheduler_started = true;
      (void)app_rta_runtime_protected_end();
      app_rta_ready();
    } else {
      (void)app_rta_runtime_protected_end();
    }
  }
#endif // SL_CATALOG_SL_MAIN_PRESENT

  app_rta_runtime_t *runtime = (app_rta_runtime_t *)arg;
  for (;; ) {
    if (app_rta_runtime_common_check(runtime)) {
      app_rta_internal_context_t *ctx = app_rta_list_rr_next(runtime);
      if (ctx != NULL && ctx->config.step != NULL) {
        // Execute
        ctx->config.step();
      }
    }
  }
}
