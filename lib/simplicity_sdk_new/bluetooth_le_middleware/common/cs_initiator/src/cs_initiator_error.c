/***************************************************************************//**
 * @file
 * @brief CS initiator - error handler implementation
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

// -----------------------------------------------------------------------------
// Includes
#include <stdbool.h>
#include "sl_status.h"
#include "app_timer.h"

#include "cs_initiator_client.h"
#include "cs_initiator_common.h"
#include "cs_initiator_config.h"
#include "cs_initiator_log.h"

#include "cs_initiator_error.h"

// -----------------------------------------------------------------------------
// Static function declarations

static void error_timer_cb(app_timer_t *handle, void *data);

// -----------------------------------------------------------------------------
// Internal function definitions

/******************************************************************************
 * Start error timer of the selected instance
 *****************************************************************************/
void start_error_timer(cs_initiator_t *initiator)
{
  sl_status_t sc = SL_STATUS_OK;
  if (!initiator->error_timer_elapsed) {
    if (initiator->error_timer_started) {
      app_timer_stop(&initiator->timer_handle);
      initiator->error_timer_started = false;
    }
    sc = app_timer_start(&initiator->timer_handle,
                         CS_INITIATOR_ERROR_TIMEOUT_MS,
                         error_timer_cb,
                         (void *)initiator,
                         false);
    if (sc != SL_STATUS_OK) {
      initiator_log_error(INSTANCE_PREFIX "error timer could not start! "
                                          "[sc: 0x%lx]" LOG_NL,
                          initiator->conn_handle,
                          (unsigned long)sc);
      on_error(initiator,
               CS_ERROR_EVENT_TIMER_START_ERROR,
               sc);
    } else {
      initiator_log_debug(INSTANCE_PREFIX "Timer started. [%u ms]" LOG_NL,
                          initiator->conn_handle,
                          CS_INITIATOR_ERROR_TIMEOUT_MS);
      initiator->error_timer_started = true;
    }
  }
}

/******************************************************************************
 * Stop error timer of the selected instance
 *****************************************************************************/
void stop_error_timer(cs_initiator_t *initiator)
{
  sl_status_t sc = SL_STATUS_OK;
  sc = app_timer_stop(&initiator->timer_handle);
  if (sc != SL_STATUS_OK) {
    initiator_log_error(INSTANCE_PREFIX "could not stop the error timer! "
                                        "[sc: 0x%lx]" LOG_NL,
                        initiator->conn_handle,
                        (unsigned long)sc);
    on_error(initiator,
             CS_ERROR_EVENT_TIMER_STOP_ERROR,
             sc);
    return;
  }
  initiator->error_timer_started = false;
  initiator->error_timer_elapsed = false;
  initiator_log_debug(INSTANCE_PREFIX "Timer stopped." LOG_NL,
                      initiator->conn_handle);
}

/******************************************************************************
 * Call user error callback on error if possible
 *
 * @param cs_initiator_t instance reference
 * @param cs_error_event_t initiator error event
 * @param sl_status_t status code
 *****************************************************************************/
void on_error(cs_initiator_t *initiator,
              cs_error_event_t evt,
              sl_status_t      sc)
{
  if (initiator == NULL) {
    initiator_log_error("[#?] Instance is NULL! (sc: 0x%lx)" LOG_NL,
                        (unsigned long)sc);
    return;
  }

  cs_error_cb_t cb = initiator->error_cb;
  initiator_log_error(INSTANCE_PREFIX "Error occurred (sc: 0x%lx)" LOG_NL,
                      initiator->conn_handle,
                      (unsigned long)sc);
  if (cb != NULL) {
    cb(initiator->conn_handle, evt, sc);
  }
}

// -----------------------------------------------------------------------------
// Event / callback definitions

/******************************************************************************
 * Error timer callback.
 *****************************************************************************/
static void error_timer_cb(app_timer_t *handle, void *data)
{
  cs_initiator_t *initiator = (cs_initiator_t *)data;
  if (handle == &initiator->timer_handle) {
    initiator->error_timer_started = false;
    initiator->error_timer_elapsed = true;
    on_error(initiator,
             CS_ERROR_EVENT_TIMER_ELAPSED,
             SL_STATUS_TIMEOUT);
  }
}
