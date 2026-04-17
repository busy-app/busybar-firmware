/***************************************************************************//**
 * @file
 * @brief Bluetooth Mesh Runtime Adaptor implementation
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

// -----------------------------------------------------------------------------
// Includes
#include "app_rta.h"
#include "app_assert.h"
#include "app_btmesh_rta.h"
#include "app_btmesh_rta_internal.h"
#include "app_btmesh_rta_config.h"
#include "sl_common.h"
#include "sl_component_catalog.h"

#if APP_BTMESH_RTA_GUARD_TRACE_LOG || APP_BTMESH_RTA_ACQUIRE_TIMEOUT_TRACE_LOG
#ifdef SL_CATALOG_APP_LOG_PRESENT
#include "app_log.h"
#else
#error "The app_log component is not present in this project, which is a requirement for logging!"
#endif // SL_CATALOG_APP_LOG_PRESENT
#endif // APP_BTMESH_RTA_GUARD_TRACE_LOG || APP_BTMESH_RTA_ACQUIRE_TIMEOUT_TRACE_LOG

// -----------------------------------------------------------------------------
// Static variables

// Context of the Bluetooth Mesh RTA
static app_rta_context_t btmesh_rta_ctx = APP_RTA_INVALID_CONTEXT;

// -----------------------------------------------------------------------------
// Public function definitions

/*******************************************************************************
 * Acquire access to the Bluetooth Mesh RTA context
 ******************************************************************************/
sl_status_t _app_btmesh_rta_acquire(void)
{
  return app_rta_acquire(btmesh_rta_ctx);
}

/*******************************************************************************
 * Acquire access to the Bluetooth Mesh RTA context with debug capabilities
 ******************************************************************************/
sl_status_t _app_btmesh_rta_acquire_trace(const char* func_name)
{
#if !APP_BTMESH_RTA_GUARD_TRACE_LOG && !APP_BTMESH_RTA_ACQUIRE_TIMEOUT_TRACE_LOG
  (void)func_name;
#endif

#if APP_BTMESH_RTA_GUARD_TRACE_LOG
  app_log("app_btmesh_rta_acquire: %s" APP_LOG_NL, func_name);
#endif

  sl_status_t status = app_rta_acquire(btmesh_rta_ctx);

#if APP_BTMESH_RTA_ACQUIRE_TIMEOUT_TRACE_LOG
  if (status == SL_STATUS_FAIL) {
    app_log("app_btmesh_rta_acquire timeout: %s" APP_LOG_NL, func_name);
  }

#elif APP_BTMESH_RTA_ACQUIRE_TIMEOUT_TRACE_ASSERT
  // In case of a timeout during the acquire the execution is halted
  app_assert_status(status == SL_STATUS_FAIL);
#endif

  return status;
}

/*******************************************************************************
 * Release access to the Bluetooth Mesh RTA context
 ******************************************************************************/
sl_status_t _app_btmesh_rta_release(void)
{
  return app_rta_release(btmesh_rta_ctx);
}

/*******************************************************************************
 * Release access to the Bluetooth Mesh RTA context with debug capabilities
 ******************************************************************************/
sl_status_t _app_btmesh_rta_release_trace(const char* func_name)
{
#if APP_BTMESH_RTA_GUARD_TRACE_LOG
  app_log("app_btmesh_rta_release: %s" APP_LOG_NL, func_name);
#else
  (void)func_name;
#endif

  sl_status_t status = app_rta_release(btmesh_rta_ctx);

  return status;
}

// -----------------------------------------------------------------------------
// Internal function definitions

/*******************************************************************************
 * Initialize the Bluetooth Mesh RTA context
 ******************************************************************************/
void _app_btmesh_rta_init(void)
{
  sl_status_t sc;

  // Create context
  app_rta_config_t config = {
    .requirement.runtime = false,
    .requirement.guard   = true,
    .error               = app_btmesh_rta_on_runtime_error,
    .wait_for_guard      = APP_BTMESH_RTA_CONFIG_WAIT
  };
  sc = app_rta_create_context(&config, &btmesh_rta_ctx);
  if (sc != SL_STATUS_OK) {
    app_btmesh_rta_on_runtime_error(APP_RTA_ERROR_RUNTIME_INIT_FAILED, sc);
  }
}

// -----------------------------------------------------------------------------
// Event / callback definitions

/*******************************************************************************
* Error callback function for Bluetooth Mesh RTA
*******************************************************************************/
SL_WEAK void app_btmesh_rta_on_runtime_error(app_rta_error_t error,
                                             sl_status_t     result)
{
  (void) result;
  app_assert_status_f((error == APP_RTA_ERROR_RUNTIME_INIT_FAILED), "Failed to initialize btmesh runtime context!");
  btmesh_rta_ctx = APP_RTA_INVALID_CONTEXT;
}
