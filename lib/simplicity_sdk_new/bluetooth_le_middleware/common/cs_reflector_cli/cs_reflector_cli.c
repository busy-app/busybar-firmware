/***************************************************************************//**
 * @file
 * @brief CS Reflector command line interface
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

#include "sl_bt_api.h"
#include "sl_status.h"
#include "sl_cli.h"
#include "sl_cli_config_example.h"
#include "sl_iostream.h"
#include "app_log.h"
#include "ble_peer_manager_peripheral.h"

// -----------------------------------------------------------------------------
// Macros

// HCI specification for CS SYNC antenna usage
// In RTT mode these values specify the antenna usage for CS SYNC packets
#define CS_SYNC_ANT_ID_1            1
#define CS_SYNC_ANT_ID_2            2
#define CS_SYNC_ANT_SWITCHING    0xFE

#define cli_print(...) \
  sl_iostream_printf(SL_CLI_EXAMPLE_IOSTREAM_HANDLE, __VA_ARGS__)

// -----------------------------------------------------------------------------
// Static variables

static bool start = false;
static uint8_t cs_sync_antenna_usage = CS_SYNC_ANT_SWITCHING;

// -----------------------------------------------------------------------------
// Public functions

/*******************************************************************************
 * CLI Getter for Antenna usage for CS SYNC packets
 ******************************************************************************/
uint8_t cs_reflector_cli_get_cs_sync_antenna_usage(void)
{
  return cs_sync_antenna_usage;
}

/*******************************************************************************
 * CLI Callback for "log_level" command
 * @param[in] arguments pointer to CLI arguments
 ******************************************************************************/
void cs_reflector_cli_log_level(sl_cli_command_arg_t *arguments)
{
  uint8_t arg_data;
  sl_status_t sc;
  arg_data = sl_cli_get_argument_uint8(arguments, 0);
  app_log_filter_threshold_enable(true);
  sc = app_log_filter_threshold_set((uint8_t)arg_data);
  if (sc != SL_STATUS_OK) {
    cli_print("ERROR: 0x%04lx\n", sc);
  } else {
    cli_print("OK.\n");
  }
  (void)arg_data;
}

/*******************************************************************************
 * CLI Callback for "start" command
 * @param[in] arguments pointer to CLI arguments
 ******************************************************************************/
void cs_reflector_cli_start(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t sc;
  if (start == false) {
    sc = ble_peer_manager_peripheral_start_advertising(SL_BT_INVALID_ADVERTISING_SET_HANDLE);
    if (sc != SL_STATUS_OK) {
      cli_print("ERROR. Advertising not started.\n");
    } else {
      cli_print("OK.\n");
      start = true;
    }
  } else {
    cli_print("ERROR. Reflector already started.\n");
  }
}

/*******************************************************************************
 * CLI Callback for "cs_sync_antenna_usage" command
 * @param[in] arguments pointer to CLI arguments
 ******************************************************************************/
void cs_reflector_cli_cs_sync_antenna_usage(sl_cli_command_arg_t *arguments)
{
  char *hex_string;
  unsigned int arg_data;
  int position;
  hex_string = sl_cli_get_argument_string(arguments, 0);
  int items_read = sscanf(hex_string, "%X%n", &arg_data, &position);
  if (items_read != 1 || (size_t)position != strlen(hex_string)) {
    cli_print("ERROR. Invalid argument.\n");
    return;
  }
  if (arg_data != CS_SYNC_ANT_ID_1
      && arg_data != CS_SYNC_ANT_ID_2
      && arg_data != CS_SYNC_ANT_SWITCHING) {
    cli_print("ERROR. Only (%x, %x, %#x) are supported.\n",
              CS_SYNC_ANT_ID_1, CS_SYNC_ANT_ID_2, CS_SYNC_ANT_SWITCHING);
  } else {
    cli_print("OK. Antenna usage for CS SYNC packets set to %#x\n", arg_data);
    cs_sync_antenna_usage = arg_data;
  }
}
