/***************************************************************************//**
 * @file
 * @brief Runtime Adaptor interface for throughput peripheral role.
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

#include "sl_status.h"
#include "app_assert.h"
#include "app_rta.h"
#include "app_log.h"
#include "throughput_peripheral.h"
#include "throughput_peripheral_rta.h"

static app_rta_context_t ctx;

static void on_runtime_error(app_rta_error_t error, sl_status_t result)
{
  switch (error) {
    case APP_RTA_ERROR_RUNTIME_INIT_FAILED:
      app_assert_status_f(result, "RTA runtime init failed" APP_LOG_NL);
      break;

    case APP_RTA_ERROR_ACQUIRE_FAILED:
      app_assert_status_f(result, "RTA acquire failed" APP_LOG_NL);
      break;

    case APP_RTA_ERROR_RELEASE_FAILED:
      app_assert_status_f(result, "RTA release failed" APP_LOG_NL);
      break;

    default:
      app_assert_status_f(result, "RTA generic error" APP_LOG_NL);
      break;
  }
}

void throughput_peripheral_rta_init(void)
{
  app_rta_config_t config = {
    .requirement.runtime = true,
    .requirement.guard   = true,
    .requirement.signal  = true,
    .step                = throughput_peripheral_step,
    .priority            = APP_RTA_PRIORITY_NORMAL,
    .stack_size          = 1024,
    .error               = on_runtime_error,
    .wait_for_guard      = 10
  };
  sl_status_t sc = app_rta_create_context(&config, &ctx);
  if (sc != SL_STATUS_OK) {
    on_runtime_error(APP_RTA_ERROR_RUNTIME_INIT_FAILED, sc);
  }
}

void throughput_peripheral_rta_ready(void)
{
  (void)app_rta_proceed(ctx);
}

void throughput_peripheral_rta_acquire(void)
{
  sl_status_t sc = app_rta_acquire(ctx);
  if (sc != SL_STATUS_OK) {
    on_runtime_error(APP_RTA_ERROR_ACQUIRE_FAILED, sc);
  }
}

void throughput_peripheral_rta_release(void)
{
  sl_status_t sc = app_rta_release(ctx);
  if (sc != SL_STATUS_OK) {
    on_runtime_error(APP_RTA_ERROR_RELEASE_FAILED, sc);
  }
}

void throughput_peripheral_rta_proceed(void)
{
  (void)app_rta_proceed(ctx);
}
