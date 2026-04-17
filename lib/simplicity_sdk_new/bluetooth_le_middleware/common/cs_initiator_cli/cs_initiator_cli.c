/***************************************************************************//**
 * @file
 * @brief CS Initiator command line interface
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

#include "sl_bt_api.h"
#include "sl_status.h"
#include "sl_cli.h"
#include "sl_cli_config_example.h"
#include "sl_iostream.h"
#include "app_log.h"
#include "sl_rtl_clib_api.h"
#include "ble_peer_manager_filter.h"
#include "ble_peer_manager_central.h"

// -----------------------------------------------------------------------------
// Macros
#define CHANNEL_MAP_PRESET_LOW       0u
#define CHANNEL_MAP_PRESET_MEDIUM    1u
#define CHANNEL_MAP_PRESET_HIGH      2u // default
#define CHANNEL_MAP_PRESET_CUSTOM    3u
// Channel Sounding specification for Antenna Configuration Index (ACI)
// In PBR mode these values specify the antenna configuration of the initiator
// and reflector device
#define ACI_SINGLE           0u
#define ACI_DUAL_I_SINGLE_R  1u
#define ACI_SINGLE_I_DUAL_R  4u
#define ACI_DUAL             7u
// HCI specification for CS SYNC antenna usage
// In RTT mode these values specify the antenna usage for CS SYNC packets
#define CS_SYNC_ANT_ID_1             1
#define CS_SYNC_ANT_ID_2             2
#define CS_SYNC_ANT_SWITCHING     0xFE

#define cli_print(...) \
  sl_iostream_printf(SL_CLI_EXAMPLE_IOSTREAM_HANDLE, __VA_ARGS__)

// -----------------------------------------------------------------------------
// Static variables
static bool start = false;
static uint8_t cs_main_mode = sl_bt_cs_mode_pbr;
static uint8_t cs_sub_mode = sl_bt_cs_submode_disabled;
static uint8_t algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC;
static uint8_t channel_map_preset = CHANNEL_MAP_PRESET_HIGH;
static uint8_t antenna_config_idx = ACI_DUAL;
static uint8_t cs_sync_antenna_usage = CS_SYNC_ANT_SWITCHING;
static uint8_t conn_phy = sl_bt_gap_phy_2m;
static uint8_t procedure_counter = 0; // Number of procedures to be executed

// -----------------------------------------------------------------------------
// Public functions

/*******************************************************************************
 * CLI Getter for CS mode
 ******************************************************************************/
uint8_t cs_initiator_cli_get_mode(void)
{
  return cs_main_mode;
}

/*******************************************************************************
 * CLI Getter for CS sub mode
 ******************************************************************************/
uint8_t cs_initiator_cli_get_sub_mode(void)
{
  return cs_sub_mode;
}

/*******************************************************************************
 * CLI Getter for CS algorithm mode
 ******************************************************************************/
uint8_t cs_initiator_cli_get_algo_mode(void)
{
  return algo_mode;
}

/*******************************************************************************
 * CLI Getter for CS algorithm mode
 ******************************************************************************/
uint8_t cs_initiator_cli_get_procedure_counter(void)
{
  return procedure_counter;
}

/*******************************************************************************
 * CLI Getter for CS channel map preset
 ******************************************************************************/
uint8_t cs_initiator_cli_get_preset(void)
{
  return channel_map_preset;
}

/*******************************************************************************
 * CLI Getter for CS channel map preset
 ******************************************************************************/
uint8_t cs_initiator_cli_get_conn_phy(void)
{
  return conn_phy;
}

/*******************************************************************************
 * CLI Getter for CS antenna configuration index
 ******************************************************************************/
uint8_t cs_initiator_cli_get_antenna_config_index(void)
{
  return antenna_config_idx;
}

/*******************************************************************************
 * CLI Getter for CS SYNC antenna usage
 ******************************************************************************/
uint8_t cs_initiator_cli_get_cs_sync_antenna_usage(void)
{
  return cs_sync_antenna_usage;
}

/*******************************************************************************
 * CLI Callback for "mode" command
 * @param[in] arguments pointer to CLI arguments
 ******************************************************************************/
void cs_initiator_cli_mode(sl_cli_command_arg_t *arguments)
{
  uint8_t arg_data;
  arg_data = sl_cli_get_argument_uint8(arguments, 0);
  if (arg_data == sl_bt_cs_mode_pbr || arg_data == sl_bt_cs_mode_rtt) {
    cs_main_mode = arg_data;
    if (arg_data == sl_bt_cs_mode_rtt
        && channel_map_preset != CHANNEL_MAP_PRESET_HIGH) {
      cli_print("ERROR. Only preset HIGH is supported with RTT mode!\n");
      return;
    }
    if (arg_data == sl_bt_cs_mode_rtt
        && cs_sub_mode == sl_bt_cs_mode_rtt) {
      cli_print("ERROR. CS main mode RTT and CS sub mode RTT combination is not supported!\n");
      return;
    }
    cli_print("OK. Initiator mode set to: %d\n", arg_data);
  } else {
    cli_print("ERROR. Mode should be %d or %d\n",
              sl_bt_cs_mode_pbr, sl_bt_cs_mode_rtt);
  }
}

/*******************************************************************************
 * CLI Callback for "sub_mode" command
 * @param[in] arguments pointer to CLI arguments
 ******************************************************************************/
void cs_initiator_cli_sub_mode(sl_cli_command_arg_t *arguments)
{
  uint8_t arg_data;
  arg_data = sl_cli_get_argument_uint8(arguments, 0);
  if (arg_data == sl_bt_cs_submode_disabled || arg_data == sl_bt_cs_mode_rtt) {
    cs_sub_mode = arg_data;
    if (arg_data == sl_bt_cs_mode_rtt
        && channel_map_preset != CHANNEL_MAP_PRESET_HIGH) {
      cli_print("ERROR. Only preset HIGH is supported with RTT mode!\n");
      return;
    }
    if (arg_data == sl_bt_cs_mode_rtt
        && cs_main_mode == sl_bt_cs_mode_rtt) {
      cli_print("ERROR. CS main mode RTT and CS sub mode RTT combination is not supported!\n");
      return;
    }
    cli_print("OK. Initiator sub mode set to: %d\n", arg_data);
  } else {
    cli_print("ERROR. Mode should be %d or %d\n",
              sl_bt_cs_submode_disabled, sl_bt_cs_mode_rtt);
  }
}

/*******************************************************************************
 * CLI Callback for "algo_mode" command
 * @param[in] arguments pointer to CLI arguments
 ******************************************************************************/
void cs_initiator_cli_algo_mode(sl_cli_command_arg_t *arguments)
{
  uint8_t arg_data;
  arg_data = sl_cli_get_argument_uint8(arguments, 0);
  if (arg_data == SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC) {
    cli_print("OK. Initiator object tracking mode set: real time basic "
              "(moving object tracking).\n");
    algo_mode = arg_data;
  } else if (arg_data == SL_RTL_CS_ALGO_MODE_STATIC_HIGH_ACCURACY) {
    cli_print("OK. Initiator object tracking mode set: static high accuracy "
              "(stationary object tracking).\n");
    algo_mode = arg_data;
  } else if (arg_data == SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST) {
    cli_print("OK. Initiator object tracking mode set: real time fast "
              "(moving object tracking).\n");
    algo_mode = arg_data;
  } else {
    cli_print("ERROR. Object tracking mode should be %d, %d or %d\n",
              SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC,
              SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST,
              SL_RTL_CS_ALGO_MODE_STATIC_HIGH_ACCURACY);
  }
}

/*******************************************************************************
 * CLI Callback for "preset" command
 * @param[in] arguments pointer to CLI arguments
 ******************************************************************************/
void cs_initiator_cli_preset(sl_cli_command_arg_t *arguments)
{
  uint8_t arg_data;
  arg_data = sl_cli_get_argument_uint8(arguments, 0);
  if (arg_data != CHANNEL_MAP_PRESET_LOW
      && arg_data != CHANNEL_MAP_PRESET_MEDIUM
      && arg_data != CHANNEL_MAP_PRESET_HIGH
      && arg_data != CHANNEL_MAP_PRESET_CUSTOM) {
    cli_print("ERROR. Unsupported preset (%d) provided!\n", arg_data);
  } else if (cs_main_mode == sl_bt_cs_mode_rtt
             && arg_data != CHANNEL_MAP_PRESET_HIGH) {
    cli_print("ERROR. Only preset HIGH is supported with RTT mode!\n");
  } else {
    cli_print("OK. Preset set to %d\n", arg_data);
    channel_map_preset = arg_data;
  }
}

/*******************************************************************************
 * CLI Callback for "conn_phy" command
 * @param[in] arguments pointer to CLI arguments
 ******************************************************************************/
void cs_initiator_cli_conn_phy(sl_cli_command_arg_t *arguments)
{
  uint8_t arg_data;
  arg_data = sl_cli_get_argument_uint8(arguments, 0);
  if (arg_data != sl_bt_gap_phy_1m && arg_data != sl_bt_gap_phy_2m) {
    app_log("ERROR. Only %d and %d are supported." APP_LOG_NL,
            sl_bt_gap_phy_1m, sl_bt_gap_phy_2m);
  } else {
    app_log("OK. Connection PHY set to %d" APP_LOG_NL,
            arg_data);
    conn_phy = arg_data;
  }
}

/*******************************************************************************
 * CLI Callback for "conn_phy" command
 * @param[in] arguments pointer to CLI arguments
 ******************************************************************************/
void cs_initiator_cli_procedure_counter(sl_cli_command_arg_t *arguments)
{
  uint8_t arg_data;
  arg_data = sl_cli_get_argument_uint8(arguments, 0);
  if (arg_data != 0 && arg_data != 1) {
    app_log("ERROR. Only 0 and 1 are supported." APP_LOG_NL);
  } else {
    app_log("OK. Procedure counter set to %d" APP_LOG_NL,
            arg_data);
    procedure_counter = arg_data;
  }
}

/*******************************************************************************
 * CLI Callback for "filter_address" command
 * @param[in] arguments pointer to CLI arguments
 ******************************************************************************/
void cs_initiator_cli_filter_address(sl_cli_command_arg_t *arguments)
{
  char *address_str;
  bd_addr ble_address;
  sl_status_t sc;
  address_str = sl_cli_get_argument_string(arguments, 0);
  sc = ble_peer_manager_str_to_address(address_str, &ble_address);
  if (sc != SL_STATUS_OK) {
    cli_print("ERROR. Failed to parse BLE address.\n");
    return;
  }
  ble_peer_manager_set_filter_bt_address(true);
  ble_peer_manager_add_allowed_bt_address(&ble_address);
  cli_print("OK.\n");
}

/*******************************************************************************
 * CLI Callback for "log_level" command
 * @param[in] arguments pointer to CLI arguments
 ******************************************************************************/
void cs_initiator_cli_log_level(sl_cli_command_arg_t *arguments)
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
void cs_initiator_cli_start(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t sc;
  if (start == false) {
    sc = ble_peer_manager_central_create_connection();
    if (sc != SL_STATUS_OK) {
      cli_print("ERROR. Scanning not started.\n");
    } else {
      cli_print("OK.\n");
      start = true;
    }
  } else {
    cli_print("ERROR. Initiator already started.\n");
  }
}

/*******************************************************************************
 * CLI Callback for "antenna_configuration_index" command
 * @param[in] arguments pointer to CLI arguments
 ******************************************************************************/
void cs_initiator_cli_aci(sl_cli_command_arg_t *arguments)
{
  uint8_t arg_data;
  arg_data = sl_cli_get_argument_uint8(arguments, 0);
  if (cs_main_mode == sl_bt_cs_mode_rtt) {
    cli_print("ERROR. Changing Antenna configuration index is unavailable in RTT mode.\n");
  } else {
    if (arg_data != ACI_SINGLE
        && arg_data != ACI_DUAL_I_SINGLE_R
        && arg_data != ACI_SINGLE_I_DUAL_R
        && arg_data != ACI_DUAL) {
      cli_print("ERROR. Only (%d, %d, %d, %d) are supported.\n",
                ACI_SINGLE, ACI_DUAL_I_SINGLE_R,
                ACI_SINGLE_I_DUAL_R, ACI_DUAL);
    } else {
      cli_print("OK. Antenna configuration index set to %d\n", arg_data);
      antenna_config_idx = arg_data;
    }
  }
}

/*******************************************************************************
 * CLI Callback for "cs_sync_antenna_usage" command
 * @param[in] arguments pointer to CLI arguments
 ******************************************************************************/
void cs_initiator_cli_cs_sync_antenna_usage(sl_cli_command_arg_t *arguments)
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
  if (cs_main_mode == sl_bt_cs_mode_pbr) {
    cli_print("ERROR. Changing CS SYNC antenna usage is unavailable in PBR mode.\n");
    return;
  }
  if (arg_data != CS_SYNC_ANT_ID_1
      && arg_data != CS_SYNC_ANT_ID_2
      && arg_data != CS_SYNC_ANT_SWITCHING) {
    cli_print("ERROR. Only (%x, %x, %#x) are supported.\n",
              CS_SYNC_ANT_ID_1, CS_SYNC_ANT_ID_2, CS_SYNC_ANT_SWITCHING);
  } else {
    cli_print("OK. Antenna usage for CS SYNC packets set to %#x\n",
              arg_data);
    cs_sync_antenna_usage = arg_data;
  }
}
