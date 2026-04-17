/***************************************************************************//**
 * @file
 * @brief Core application logic for Bare Metal.
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
#include "sl_core.h"
#include "app.h"
#include "sl_main_init.h"

#ifdef SL_CATALOG_APP_LOG_PRESENT
#include "app_log.h"
#endif // SL_CATALOG_APP_LOG_PRESENT

// Semaphore indicating that it is required to execute application process action.
static uint16_t proceed_semaphore;

// Initialization steps for bare metal.
void app_permanent_memory_alloc(void)
{
  proceed_semaphore = 0;
}

// Application Runtime Init.
void app_init_runtime(void)
{
  app_log("BT mesh Sensor Server initialized" APP_LOG_NL);
  // Ensure right init order in case of shared pin for enabling buttons
  app_change_buttons_to_leds();
  // Change LEDs to buttons in case of shared pin
  app_change_leds_to_buttons();
  app_handle_reset_conditions();
}

// Proceed with execution.
void app_proceed(void)
{
  if (proceed_semaphore < UINT16_MAX) {
    proceed_semaphore++;
  }
}

// Check for any last-minute events that should prevent going to sleep
bool app_is_ok_to_sleep(void)
{
  return 0 == proceed_semaphore;
}

// Check if it is required to process with execution.
bool app_is_process_required(void)
{
  bool ret = false;
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  if (proceed_semaphore > 0) {
    proceed_semaphore--;
    ret = true;
  }
  CORE_EXIT_CRITICAL();
  return ret;
}
