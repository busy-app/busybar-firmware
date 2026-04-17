/***************************************************************************//**
 * @file app.c
 * @brief Application code
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
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdio.h>

#include "sl_assert.h"
#include "app.h"
#include "sl_wisun_app_core_util.h"
#include "sl_bluetooth.h"
#include "sl_cli.h"
#include "sl_iostream.h"
#include "sl_iostream_ble_spp.h"
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
/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc = SL_STATUS_FAIL;
  uint8_t address_type = 0U;
  bd_addr address = { .addr = { 0U } };

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Print boot message.
      printf("[BLE stack booted: v%d.%d.%d+%08lx]\n",
             evt->data.evt_system_boot.major,
             evt->data.evt_system_boot.minor,
             evt->data.evt_system_boot.patch,
             evt->data.evt_system_boot.hash);

      // Extract unique ID from BT Address.
      sc = sl_bt_gap_get_identity_address(&address, &address_type);
      EFM_ASSERT(sc == SL_STATUS_OK);

      // Print device address.
      printf("[BLE %s address: %02X:%02X:%02X:%02X:%02X:%02X]\n",
             address_type ? "static random" : "public device",
             address.addr[5],
             address.addr[4],
             address.addr[3],
             address.addr[2],
             address.addr[1],
             address.addr[0]);

      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      // Print connection opened message.
      printf("[BLE connection (%u) is opened by %02X:%02X:%02X:%02X:%02X:%02X]\n",
             evt->data.evt_connection_opened.connection,
             evt->data.evt_connection_opened.address.addr[5],
             evt->data.evt_connection_opened.address.addr[4],
             evt->data.evt_connection_opened.address.addr[3],
             evt->data.evt_connection_opened.address.addr[2],
             evt->data.evt_connection_opened.address.addr[1],
             evt->data.evt_connection_opened.address.addr[0]);

      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      // Print connection closed message.
      printf("[BLE connection (%u) is closed]\n",
             evt->data.evt_connection_closed.connection);

      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

/**************************************************************************//**
 * SPP CLI session handler.
 * Override the default weak sli_cli_session_handler and set the task-specific I/O Stream.
 *
 * @param[in] handle CLI handle.
 *****************************************************************************/
sl_status_t sli_cli_session_handler(sl_cli_handle_t handle)
{
  sl_iostream_t *iostream = NULL;
  sl_status_t mode_status = SL_STATUS_FAIL;
  sl_iostream_ble_spp_mode_t spp_mode = SL_IOSTREAM_BLE_SPP_MODE_OFF;

  (void) handle;

  mode_status = sl_iostream_ble_spp_get_mode(&spp_mode);

  if ((sl_iostream_ble_spp_get_connection_handle() != SL_BT_INVALID_CONNECTION_HANDLE)
      && (mode_status == SL_STATUS_OK)
      && (spp_mode == SL_IOSTREAM_BLE_SPP_MODE_ON)) {
    iostream = sl_iostream_get_handle("spp");
  } else {
    iostream = sl_iostream_get_handle("vcom");
  }

  // Default I/O Stream is RTOS task specific, make sure tick_task
  // is polling the right I/O Stream.
  if (iostream != NULL) {
    // Switch default system-wide I/O Stream dynamically
    sl_iostream_set_system_default(iostream);
    // Set to NULL to use the system default I/O Stream
    sl_iostream_set_default(NULL);
  }

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * App task function
 * @param[in] args Arguments
 *****************************************************************************/
void app_task(void *args)
{
  (void) args;

  // Connect to the wisun network
  sl_wisun_app_core_util_connect_and_wait();

  while (1) {
    ///////////////////////////////////////////////////////////////////////////
    // Put your application code here!                                       //
    ///////////////////////////////////////////////////////////////////////////
    sl_wisun_app_core_util_dispatch_thread();
  }
}
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
