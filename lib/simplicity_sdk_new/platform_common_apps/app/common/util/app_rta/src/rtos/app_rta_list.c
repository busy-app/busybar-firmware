/***************************************************************************//**
 * @file
 * @brief Application Runtime Adaptor (RTA) - List management implementation
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
#include <stdbool.h>
#include <stdint.h>
#include "sl_memory_manager.h"
#include "app_rta_internal.h"
#include "app_rta_runtime.h"
#include "app_rta_list.h"

// -----------------------------------------------------------------------------
// Private variables
static app_rta_runtime_list_t rt_list = NULL;

// -----------------------------------------------------------------------------
// Forward declaration of private functions
static app_rta_runtime_t *find_required_runtime(app_rta_internal_context_t *ctx);

// -----------------------------------------------------------------------------
// Public functions

void app_rta_list_init(void)
{
  // Initialize list
  sl_slist_init(&rt_list);
}

// Add a context with a priority: create runtime if required
sl_status_t app_rta_list_add_context(app_rta_internal_context_t *ctx)
{
  bool add_runtime = false;
  // Find a runtime if present
  app_rta_runtime_t *runtime = find_required_runtime(ctx);
  if (runtime == NULL) {
    // New runtime is required
    // Allocate memory
    runtime = sl_malloc(sizeof(app_rta_runtime_t));
    if (runtime == NULL) {
      return SL_STATUS_ALLOCATION_FAILED;
    }
    // Set priority and exclusiveness
    runtime->priority = ctx->config.priority;
    runtime->separate = ctx->config.requirement.runtime_separate;
    runtime->stack_size = 0;
    runtime->enabled = ctx->enabled;
    // Initialize runtime data
    runtime->runtime_data = NULL;
    runtime->rr_state = NULL;
    runtime->node.node = NULL;
    // Initialize list
    sl_slist_init(&runtime->context_list);
    // Add runtime to the list later
    add_runtime = true;
  }

  if (ctx->enabled) {
    // Enable runtime if there is an enabled context.
    runtime->enabled = true;
  }

  // Runtime has not been started
  // Adjust required stack size first.
  if (runtime->stack_size < ctx->config.stack_size) {
    runtime->stack_size = ctx->config.stack_size;
  }

  // Add context to runtime without guarding since it has not been started yet.
  sl_slist_push_back(&runtime->context_list, &ctx->node);

  // Add runtime if required
  if (add_runtime) {
    sl_slist_push_back(&rt_list, &runtime->node);
  }

  return SL_STATUS_OK;
}

void app_rta_list_complete(void)
{
  sl_status_t sc = SL_STATUS_OK;
  sl_slist_node_t *rt_iterator;
  sl_slist_node_t *ctx_iterator;

  // Iterate over runtimes
  SL_SLIST_FOR_EACH(rt_list, rt_iterator) {
    app_rta_runtime_t *runtime = (app_rta_runtime_t *)rt_iterator;
    if (runtime->runtime_data == NULL) {
      // Initialize RR state to the context list head
      runtime->rr_state = runtime->context_list;
      // Start runtime
      sc = app_rta_runtime_init(runtime);
      if (sc != SL_STATUS_OK) {
        SL_SLIST_FOR_EACH(runtime->context_list, ctx_iterator) {
          app_rta_internal_context_t *ctx = (app_rta_internal_context_t *)ctx_iterator;
          if (ctx->config.error != NULL) {
            ctx->config.error(APP_RTA_ERROR_RUNTIME_INIT_FAILED, sc);
          }
        }
      }
    }
  }
}

// Enable or disable context and manage runtime accordingly
sl_status_t app_rta_list_enable_context(app_rta_internal_context_t *ctx, bool enable)
{
  sl_status_t sc = SL_STATUS_OK;

  // Execute manipulation in a protected context
  app_rta_runtime_protected_begin();

  // Check for context in the lists
  app_rta_runtime_t *runtime = app_rta_list_find_runtime(ctx);
  if (runtime == NULL) {
    app_rta_runtime_protected_end();
    return SL_STATUS_NOT_FOUND;
  }

  if (enable) {
    // Initialize context
    sc = app_rta_runtime_init_context(ctx);
    if (sc != SL_STATUS_OK) {
      app_rta_runtime_protected_end();
      return sc;
    }
    if (ctx->config.requirement.guard) {
      if (!app_rta_runtime_acquire(ctx)) {
        app_rta_runtime_protected_end();
        return SL_STATUS_FAIL;
      }
    }
    // Check if runtime is enabled
    if (!runtime->enabled) {
      // Initialize runtime
      sc = app_rta_runtime_init(runtime);
      if (sc != SL_STATUS_OK) {
        app_rta_runtime_protected_end();
        return sc;
      }
      // Set runtime enabled
      runtime->enabled = true;
    }
    // Enable context
    ctx->enabled = true;
    app_rta_runtime_protected_end();
    return sc;
  }
  // Deinitialize context
  sc = app_rta_runtime_deinit_context(ctx);
  if (sc != SL_STATUS_OK) {
    app_rta_runtime_protected_end();
    return sc;
  }
  // Disable context
  ctx->enabled = false;
  // Check if all contexts in runtime are disabled.
  bool empty_runtime = true;
  sl_slist_node_t *ctx_iterator;
  SL_SLIST_FOR_EACH(runtime->context_list, ctx_iterator) {
    if (((app_rta_internal_context_t *)ctx_iterator)->enabled) {
      empty_runtime = false;
      break;
    }
  }
  if (empty_runtime) {
    // Deinitialize runtime
    sc = app_rta_runtime_deinit(runtime);
    if (sc != SL_STATUS_OK) {
      app_rta_runtime_protected_end();
      return sc;
    }
    // Disable runtime
    runtime->enabled = false;
  }
  app_rta_runtime_protected_end();
  return sc;
}

// Search for a runtime
app_rta_runtime_t *app_rta_list_find_runtime(app_rta_internal_context_t *ctx)
{
  sl_slist_node_t *rt_iterator;
  sl_slist_node_t *ctx_iterator;

  // Iterate over runtimes
  SL_SLIST_FOR_EACH(rt_list, rt_iterator) {
    // Iterate over contexts
    sl_slist_node_t *ctx_list = ((app_rta_runtime_t *)rt_iterator)->context_list;
    SL_SLIST_FOR_EACH(ctx_list, ctx_iterator) {
      // Check context
      if ((app_rta_internal_context_t *)ctx_iterator == ctx) {
        return (app_rta_runtime_t *)rt_iterator;
      }
    }
  }
  return NULL;
}

app_rta_internal_context_t *app_rta_list_rr_next(app_rta_runtime_t *runtime)
{
  if (runtime == NULL
      || runtime->context_list == NULL
      || runtime->rr_state == NULL) {
    return NULL;
  }
  app_rta_internal_context_t *result = NULL;
  app_rta_internal_context_t *ctx;
  // Iterate circularly over the list until we get back to the current
  // RR state or context needs execution.
  sl_slist_node_t *ctx_start = runtime->rr_state;
  sl_slist_node_t *ctx_iterator = ctx_start;
  bool run = true;
  while (run) {
    if (ctx_iterator->node == NULL) {
      ctx_iterator = runtime->context_list;
    } else {
      ctx_iterator = ctx_iterator->node;
    }
    ctx = (app_rta_internal_context_t *)ctx_iterator;
    if (ctx->enabled) {
      // Check if context needs runtime execution.
      if (app_rta_runtime_check_context(ctx)) {
        // Context needs execution, save RR state.
        runtime->rr_state = ctx_iterator;
        // Set the result.
        result = ctx;
        // Stop searching
        run = false;
      }
    }

    if (ctx_iterator == ctx_start) {
      run = false;
    }
  }
  return result;
}

static app_rta_runtime_t *find_required_runtime(app_rta_internal_context_t *ctx)
{
  // Check config
  if (ctx->config.requirement.runtime_separate) {
    // Separate runtime is required
    return NULL;
  }
  // Iterate over runtimes
  sl_slist_node_t *rt_iterator;
  SL_SLIST_FOR_EACH(rt_list, rt_iterator) {
    app_rta_runtime_t *runtime = (app_rta_runtime_t *)rt_iterator;
    // Check if actual runtime is common and it has the same priority
    if (!(runtime->separate) && (runtime->priority == ctx->config.priority)) {
      return runtime;
    }
  }
  // No runtime found, new runtime is required
  return NULL;
}
