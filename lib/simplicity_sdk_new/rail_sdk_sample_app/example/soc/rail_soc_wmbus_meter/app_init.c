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
#include <stdlib.h>
#include "em_cmu.h"
#include "sl_rail.h"
#include "sl_rail_util_init.h"
#include "app_init.h"
#include "app_process.h"
#include "sl_rail_sdk_simple_assistance.h"
#include "sl_rail_sdk_wmbus_support.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// Which rx channel will be used, it will be calculated from the TX channel
uint16_t rx_channel = 0U;

/// Access number for the Wireless M-Bus protocol
uint8_t access_number = 0U;

/// Time for calculation for the proper sending timing
extern sl_rail_time_t last_tx_start_time;

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
/// Security key for the app
static const uint8_t crypto_key[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                      0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/******************************************************************************
 * The function is used for some basic initialization related to the app.
 *****************************************************************************/
void rail_app_init(void)
{
  // Get RAIL handle, used later by the application
  sl_rail_handle_t rail_handle = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);

  // Initialization of Wireless M-Bus
  sl_rail_sdk_wmbus_init();

  // Get the mode of Wireless M-Bus
  sl_rail_sdk_wmbus_mode_t mode = sl_rail_sdk_wmbus_get_selected_mode();

  clear_receive_led();
  clear_send_led();

  if ( sl_rail_sdk_wmbus_get_accessibility() == WMBUS_ACCESSIBILITY_UNLIMITED_ACCESS ) {
    sl_rail_state_transitions_t transitions = {
      .error = SL_RAIL_RF_STATE_RX,
      .success = SL_RAIL_RF_STATE_RX,
    };
    sl_rail_set_rx_transitions(rail_handle, &transitions);
  }

  if ( mode == WMBUS_MODE_T_METER || mode == WMBUS_MODE_T_COLLECTOR || mode == WMBUS_MODE_C ) {
    rx_channel = DEFAULT_CHANNEL + 1; //asymmetric config, rx will be on a different channel
  } else {
    rx_channel = DEFAULT_CHANNEL;
  }

  //calibrate IR on rx channel before starting the main loop, since it takes longer than most rx operation
  sl_rail_prepare_channel(rail_handle, rx_channel);
  sl_rail_ir_cal_values_t image_rejection;
  sl_rail_antenna_sel_t rf_path;
  sl_rail_get_rf_path(rail_handle, &rf_path);
  sl_rail_calibrate_ir(rail_handle, &image_rejection, rf_path);

  sl_rail_sdk_wmbus_frame_crypto5_init();
  sl_rail_sdk_wmbus_frame_crypto5_set_key(crypto_key);

  access_number = (uint8_t)(rand() % 256);

  set_next_state(S_SCHEDULE_TX);
  last_tx_start_time = sl_rail_get_time(rail_handle);
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
