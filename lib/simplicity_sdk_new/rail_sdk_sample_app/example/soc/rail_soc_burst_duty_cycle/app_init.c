/***************************************************************************//**
 * @file
 * @brief app_init.c
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>

#include "sl_rail.h"
#include "rail_config.h"
#include "sl_rail_util_init.h"
#include "app_init.h"
#include "app_process.h"
#include "sl_component_catalog.h"
#include "sl_rail_sdk_channel_selector.h"
#include "sl_duty_cycle_config.h"
#include "sl_duty_cycle_utility.h"
#include "sl_power_manager.h"
#include "sl_rail_sdk_simple_assistance.h"

#if DUTY_CYCLE_USE_LCD_BUTTON == 1
  #include "app_graphics.h"
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "app_task_init.h"
#endif

#if defined(SL_CATALOG_APP_LOG_PRESENT)
#include "app_log.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// Config for the correct timing of the dutycycle API
sl_rail_rx_duty_cycle_config_t duty_cycle_config = {
  .delay_us = ((uint32_t) DUTY_CYCLE_OFF_TIME),
  .mode = SL_RAIL_RX_CHANNEL_HOPPING_MODE_PREAMBLE_SENSE,
  .parameter = ((uint32_t) DUTY_CYCLE_ON_TIME)
};

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/******************************************************************************
 * Print sample app name
 *****************************************************************************/
SL_WEAK void print_sample_app_name(const char* app_name)
{
#ifdef SL_CATALOG_APP_LOG_PRESENT
  app_log_info("%s\n", app_name);
#endif
  // Avoid unused parameter warning
  (void)app_name;
}

/******************************************************************************
 * The function is used for some basic initialization related to the app.
 *****************************************************************************/
void rail_app_init(void)
{
  // For handling error codes
  sl_rail_status_t rail_status = SL_RAIL_STATUS_NO_ERROR;

  // To calculate proper preamble
  uint32_t bit_rate = 0UL;

  // Get RAIL handle, used later by the application
  sl_rail_handle_t rail_handle = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);
  // Set to IDLE (channel select automatically start RX)
  rail_status = sl_rail_idle(rail_handle, SL_RAIL_IDLE, true);
  if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_warning("Couldn't enter into IDLE, error code %lu\n", rail_status);
  }

  // Get current bitrate
  bit_rate = sl_rail_get_bit_rate(rail_handle);

  // Turn OFF LEDs
  clear_receive_led();
  clear_send_led();

#if DUTY_CYCLE_USE_LCD_BUTTON == 1
  // LCD start
  graphics_init();
#endif

  print_sample_app_name("Burst Duty Cycle");
  app_log_info("Bitrate is %lu b/s with %lu us off time and %lu us on time\n",
               bit_rate,
               duty_cycle_config.delay_us,
               duty_cycle_config.parameter);

  // Allow state machine to run without interrupt
  set_first_run(true);

  // Config DutyCycle API
  sl_rail_config_rx_duty_cycle(rail_handle, &duty_cycle_config);

  // Enable duty cycle mode
  sl_rail_enable_rx_duty_cycle(rail_handle, true);

  // Start RX
  sl_rail_start_rx(rail_handle, get_selected_channel(), NULL);

#if DUTY_CYCLE_ALLOW_EM2 == 1
  // EM2 sleep level
  sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif
}

void app_init(void)
{
#if !defined(SL_CATALOG_KERNEL_PRESENT)
  rail_app_init();
#else
  app_task_init();
#endif
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
