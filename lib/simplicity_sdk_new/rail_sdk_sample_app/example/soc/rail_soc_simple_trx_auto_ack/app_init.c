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

#include "sl_component_catalog.h"
#include "sl_common.h"
#include "sl_rail.h"
#include "rail_config.h"
#include "sl_rail_util_init.h"
#include "app_process.h"
#include "sl_rail_sdk_simple_assistance.h"

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
  sl_rail_status_t status;
  // Get RAIL handle, used later by the application
  sl_rail_handle_t rail_handle = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);

  // Turn OFF LEDs
  clear_receive_led();
  clear_send_led();

  // Setup state timings for Auto-ACK
  sl_rail_state_timing_t timings = { 0 };
  timings.idle_to_tx = 100;
  timings.idle_to_rx = 100;
  timings.rx_to_tx = 192;
  // Make tx_to_rx a little lower than desired. See documentation on sl_rail_config_auto_ack.
  timings.tx_to_rx = 182;
  timings.rxsearch_timeout = 0;
  timings.tx_to_rxsearch_timeout = 0;

  status = sl_rail_set_state_timing(rail_handle, &timings);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_warning("After sl_rail_set_state_timing() result: %lu\n ", status);
  }

  // Setup Auto-ACK message
  sl_rail_auto_ack_config_t autoAckConfig = {
    .enable = true,
    .ack_timeout_us = 30000,
    // "error" param ignored
    .rx_transitions = { SL_RAIL_RF_STATE_RX, SL_RAIL_RF_STATE_RX },
    // "error" param ignored
    .tx_transitions = { SL_RAIL_RF_STATE_RX, SL_RAIL_RF_STATE_RX }
  };

#ifdef RAIL0_CHANNEL_GROUP_1_PROFILE_LONG_RANGE
  uint8_t ackData[]  = { 0x0e, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e };
#else
  uint8_t ackData[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10 };
#endif
  status = sl_rail_write_auto_ack_fifo(rail_handle, ackData, sizeof(ackData));
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_warning("After sl_rail_write_auto_ack_fifo() result: %lu\n", status);
  }
  // Enable Auto-ACK
  status = sl_rail_config_auto_ack(rail_handle, &autoAckConfig);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_warning("After sl_rail_config_auto_ack() result: %lu\n", status);
  }

  // CLI info message
  print_sample_app_name("Simple TRX with Auto-ACK");
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
