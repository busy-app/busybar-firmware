/***************************************************************************//**
 * @file
 * @brief CS NCP host
 *
 * Reference implementation of a CS host with initiator and reflector support.
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include "app.h"
#include "ncp_host.h"
#include "app_log.h"
#include "app_log_cli.h"
#include "app_assert.h"
#include "sl_bt_api.h"
#include "host_comm.h"
#include "gatt_db.h"
#include "ble_peer_manager_common.h"
#include "ble_peer_manager_connections.h"
#include "ble_peer_manager_central.h"
#include "ble_peer_manager_peripheral.h"
#include "ble_peer_manager_filter.h"
#include "cs_reflector_config.h"
#include "cs_acp.h"
#include "cs_result.h"
#include "cs_result_config.h"
#include "cs_initiator_client.h"
#include "cs_antenna.h"
#include "extended_result.h"
#include "security.h"
#include "sl_rtl_clib_api.h"
#include "cs_sync_antenna.h"
#include "app_config.h"

// MAX_CONNECTIONS should be less or equal to the SL_BT_CONFIG_MAX_CONNECTIONS in the CS NCP example.
// Sum of reflector and initiator instances should not exceed MAX_CONNECTIONS.
#define MAX_CONNECTIONS                             4u
// MAX_INITIATOR_INSTANCES should be less or equal to the CS_INITIATOR_MAX_CONNECTIONS in the CS NCP example.
#define MAX_INITIATOR_INSTANCES                     MAX_CONNECTIONS
// MAX_REFLECTOR_INSTANCES should be less or equal to the SL_BT_CONFIG_MAX_CONNECTIONS in the CS NCP example.
#define MAX_REFLECTOR_INSTANCES                     MAX_CONNECTIONS
// Subfeature bitmask for CS Channel Selection Algorithm #3c
#define CS_CHANNEL_SELECTION_ALGORITHM_3C_SUBFEATURE_BITMASK 2
// Subfeature bitmask for CS Role Initiator
#define CS_ROLE_INITIATOR_ROLE_BITMASK 0
// Subfeature bitmask for CS Role Reflector
#define CS_ROLE_REFLECTOR_ROLE_BITMASK 1

// Extended result format, used by tooling.
#ifndef EXTENDED_RESULT
#define EXTENDED_RESULT 0
#endif

// Prefixes
#define APP_PREFIX                  "[APP] "
#define INSTANCE_PREFIX             "[%u] "
#define APP_INSTANCE_PREFIX         APP_PREFIX INSTANCE_PREFIX

// Connection TX power settings
#define CS_INITIATOR_MIN_TX_POWER_DBM               -3
#define CS_INITIATOR_MAX_TX_POWER_DBM               20

#define CREATE_REFLECTOR_MSG_LEN (sizeof(cs_acp_cmd_id_t) + sizeof(cs_acp_create_reflector_cmd_data_t))
#define REFLECTOR_ACTION_MSG_LEN (sizeof(cs_acp_cmd_id_t) + sizeof(cs_acp_reflector_action_cmd_data_t))

#define CREATE_INITATOR_MSG_LEN (sizeof(cs_acp_cmd_id_t) + sizeof(cs_acp_create_initiator_cmd_data_t))
#define INITATOR_ACTION_MSG_LEN (sizeof(cs_acp_cmd_id_t) + sizeof(cs_acp_initiator_action_cmd_data_t))

#define SL_BT_CS_DEVICE_NAME_MAX_LEN 32

// Optstring argument for getopt
#define OPTSTRING NCP_HOST_OPTSTRING APP_LOG_OPTSTRING "m:R:I:F:wo:p:a:q:s:d:TPhM:S"

// Usage info
#define USAGE APP_LOG_NL "%s " NCP_HOST_USAGE APP_LOG_USAGE                                                \
  "\n[-m <cs_main_mode>] [-M <cs_sub_mode>] [-R <max_reflector_instances>] [-I <max_initiator_instances>]" \
  "[-F <reflector_ble_address>] [-w] [-o] [-p <channel_map_preset>] [-a <cs_tone_antenna_config_idx_req>]" \
  "[-q <cs_sync_antenna_req>] [-s <cs_procedure_scheduling>] [-d <device_name>] [-T] [-P] [-S] [-h]" APP_LOG_NL

// Detailed argument list
#define CS_HOST_OPTIONS                                                            \
  "    -m  CS main mode.\n"                                                        \
  "        <cs_main_mode> Integer representing CS main mode, default: 2, PBR.\n"   \
  "        Used only for initiator instances\n"                                    \
  "        1 : RTT\n"                                                              \
  "        2 : PBR\n"                                                              \
  "        Note: see -M\n"                                                         \
  "    -M  CS sub mode\n"                                                          \
  "        <cs_sub_mode> Integer representing CS sub mode, default: No sub mode\n" \
  "        Used only for initiator instances\n"                                    \
  "        1: RTT\n"                                                               \
  "        255: no sub mode\n"                                                     \
  "        Note: Only main mode = PBR, sub mode = RTT supported now\n"             \
  "        see -m\n"                                                               \
  "    -R  Maximum number of reflector instances, default: 1\n"                    \
  "        <max_reflector_instances>\n"                                            \
  "    -I  Maximum number of initiator instances, default: 1\n"                    \
  "        <max_initiator_instances>\n"                                            \
  "    -F  Enable reflector BLE address filtering in format:\n"                    \
  "        AA:BB:CC:DD:EE:FF or aabbccddeeff\n"                                    \
  "        <reflector_ble_address>\n"                                              \
  "        Used only for initiator instances\n"                                    \
  "    -w  Use wired antenna offset\n"                                             \
  "    -o  Object tracking mode, default: 2\n"                                     \
  "        Used only for initiator instances\n"                                    \
  "        0 : moving object tracking (REAL_TIME_BASIC)\n"                         \
  "        1 : stationary object tracking (STATIC_HIGH_ACCURACY)\n"                \
  "        2 : moving object tracking fast (REAL_TIME_FAST)\n"                     \
  "    -p  Pre-set parameters for channel map selection, default: 2\n"             \
  "        Used only for initiator instances\n"                                    \
  "        0 : low (channel spacing: 1, number of channels: 20)\n"                 \
  "        1 : medium (channel spacing: 2, number of channels: 38)\n"              \
  "        2 : high (channel spacing: 1, number of channels: 72)\n"                \
  "        3 : load custom from configuration macro CS_CUSTOM_CHANNEL_MAP\n"       \
  "    -a  Antenna configuration index for antenna switching, default: 7\n"        \
  "        Used only for initiator instances\n"                                    \
  "        0 : Single antennas on both sides\n"                                    \
  "        1 : Dual antenna initiator & single antenna reflector\n"                \
  "        4 : Single antenna initiator & dual antenna reflector\n"                \
  "        7 : Dual antennas on both sides\n"                                      \
  "        Note: considered only with CS main mode: PBR!\n"                        \
  "    -q  Antenna usage for CS SYNC packets, default: 0xFE\n"                     \
  "        Used for both initiator and reflector instances\n"                      \
  "        1 : use antenna ID1 only\n"                                             \
  "        2 : use antenna ID2 only\n"                                             \
  "        0xFE : Switching between antennas for each channel\n"                   \
  "        Note: considered only with CS main mode: RTT!\n"                        \
  "    -s  Optimized procedure scheduling\n"                                       \
  "        Used only for initiator instances\n"                                    \
  "        0 : Optimized for frequency update\n"                                   \
  "        1 : Optimized for energy consumption\n"                                 \
  "        2 : Custom\n"                                                           \
  "        Note: Default is frequency optimized procedure scheduling\n"            \
  "    -d  Device name.\n"                                                         \
  "    -T  Enable RTT trace including BGAPI messages and RTL log.\n"               \
  "        Note that the RTT blocks the target if no client is connected.\n"       \
  "    -P  Use 1M connection PHY\n"                                                \
  "        Used only for initiator instances\n"                                    \
  "        Note: Default is 2M\n"                                                  \
  "    -S  Enable synchronized mode\n"                                             \
  "        max_procedure_count = 1\n"

// Options info
#define OPTIONS    \
  "\nOPTIONS\n"    \
  NCP_HOST_OPTIONS \
  APP_LOG_OPTIONS  \
  CS_HOST_OPTIONS  \
  "    -h  Print this help message.\n"

#define BT_ADDR_LEN 6u
#define BT_MAX_CHANNELS 79u

typedef struct {
  bool use_antenna_wired_offset;
  bd_addr accepted_bt_address_list[MAX_REFLECTOR_INSTANCES];
  uint32_t accepted_bt_address_count;
  uint32_t max_reflector_instances;
  uint32_t max_initiator_instances;
  bool trace;
  char device_name[SL_BT_CS_DEVICE_NAME_MAX_LEN];
} cs_host_config_t;

typedef struct {
  uint32_t num_reflector_connections;
  uint32_t num_initiator_connections;
  uint8_t reflector_conn_handles[MAX_REFLECTOR_INSTANCES];
  uint8_t initiator_conn_handles[MAX_INITIATOR_INSTANCES];
  bool trace;
  bool read_remote_capabilities;
  bool security_increased;
} cs_host_state_t;

cs_host_config_t cs_host_config = {
  .use_antenna_wired_offset = false,
  .accepted_bt_address_count = 0u,
  .max_reflector_instances = 0u,
  .max_initiator_instances = 0u,
  .trace = false,
  .device_name = REFLECTOR_DEVICE_NAME
};

cs_host_state_t cs_host_state = {
  .num_reflector_connections = 0u,
  .num_initiator_connections = 0u,
  .reflector_conn_handles = { SL_BT_INVALID_CONNECTION_HANDLE },
  .initiator_conn_handles = { SL_BT_INVALID_CONNECTION_HANDLE },
  .trace = false
};

uint16_t service_uuid = 0x185B; // CS RAS Service UUID

static rtl_config_t rtl_config = RTL_CONFIG_DEFAULT;
static cs_reflector_config_t reflector_config = REFLECTOR_CONFIG_DEFAULT;
static cs_initiator_config_t initiator_config = INITIATOR_CONFIG_DEFAULT;
static uint8_t measurement_counter = 0u;

static bool initiator_cs_tone_antenna_config_index_set = false;
static bool initiator_cs_sync_antenna_req_set = false;

static const char *antenna_usage_to_str(const cs_initiator_config_t *config);
static const char *algo_mode_to_str(uint8_t algo_mode);
static void enable_trace(void);
static void disable_trace(void);
static void cs_on_result(const uint8_t conn_handle,
                         const uint8_t *result,
                         uint8_t result_len);
static void cs_on_intermediate_result(const cs_acp_intermediate_result_evt_t *intermediate_result,
                                      uint8_t connection);
static void cs_on_error(uint8_t conn_handle,
                        cs_error_event_t err_evt,
                        sl_status_t sc);
static void on_connection_opened_with_initiator(uint8_t conn_handle);
static void on_connection_opened_with_reflector(uint8_t conn_handle);
static void create_initiator(uint8_t conn_handle);
static void on_connection_closed(uint8_t conn_handle);
static void stop_procedure(void);
static void get_target_config(void);
static void check_supported_capabilities(sl_bt_msg_t *evt);

/******************************************************************************
 * Application initialization
 *****************************************************************************/
void app_cli_init(int argc, char *argv[])
{
  sl_status_t sc;
  int cli_opt;
  int cs_main_mode = sl_bt_cs_mode_pbr;
  int cs_sub_mode = sl_bt_cs_submode_disabled;
  bool max_initiator_instances_set = false;
  bool max_reflector_instances_set = false;
  unsigned int arg_data;
  int position;

  // Initialize the list of accepted BT addresses
  for (uint32_t i = 0u; i < MAX_REFLECTOR_INSTANCES; i++) {
    memset(cs_host_config.accepted_bt_address_list[i].addr, 0xFF, BT_ADDR_LEN);
  }
  security_set_config_flags();

  app_log_info("+-[CS Host by Silicon Labs]------------------------+" APP_LOG_NL);
  app_log_info("+--------------------------------------------------+" APP_LOG_NL APP_LOG_NL);

  // Process command line options
  while ((cli_opt = getopt(argc, argv, OPTSTRING)) != -1) {
    switch (cli_opt) {
      // Print help
      case 'h':
        app_log(USAGE, argv[0]);
        app_log(OPTIONS);
        exit(EXIT_SUCCESS);

      case 'm':
        // Mode.
        // 1 - RTT
        // 2 - Phase based
        cs_main_mode = atoi(optarg);
        break;

      case 'R':
        cs_host_config.max_reflector_instances = atoi(optarg);
        max_reflector_instances_set = true;
        if (cs_host_config.max_reflector_instances > MAX_REFLECTOR_INSTANCES) {
          app_log_error(APP_PREFIX "Invalid number of 'maximum reflectors' argument (%d) provided, "
                                   "must be in the range of 0 to %u" APP_LOG_NL,
                        cs_host_config.max_reflector_instances,
                        MAX_REFLECTOR_INSTANCES);
          exit(EXIT_FAILURE);
        }
        break;

      case 'I':
        cs_host_config.max_initiator_instances = atoi(optarg);
        max_initiator_instances_set = true;
        if (cs_host_config.max_initiator_instances > MAX_INITIATOR_INSTANCES) {
          app_log_error(APP_PREFIX "Invalid number of 'maximum initiators' argument (%d) provided, "
                                   "must be in the range of 0 to %u" APP_LOG_NL,
                        cs_host_config.max_initiator_instances,
                        MAX_INITIATOR_INSTANCES);
          exit(EXIT_FAILURE);
        }
        break;

      case 'F':
      {
        // Check if we have room for more accepted addresses
        if (cs_host_config.accepted_bt_address_count >= MAX_REFLECTOR_INSTANCES) {
          app_log_error(APP_PREFIX "Maximum number of accepted BLE addresses (%u) reached, "
                                   "ignoring additional addresses" APP_LOG_NL,
                        MAX_REFLECTOR_INSTANCES);
          break;
        }
        // Add the accepted BLE address to the list
        bd_addr *bt_addr_to_add = &cs_host_config.accepted_bt_address_list[cs_host_config.accepted_bt_address_count];
        sc = ble_peer_manager_str_to_address(optarg, bt_addr_to_add);
        if (sc != SL_STATUS_OK) {
          app_log_error(APP_PREFIX "Invalid BLE address filter provided" APP_LOG_NL);
          exit(EXIT_FAILURE);
        }
        app_log_info(APP_PREFIX "BLE address accept filter added for: '%02X:%02X:%02X:%02X:%02X:%02X'" APP_LOG_NL,
                     bt_addr_to_add->addr[5],
                     bt_addr_to_add->addr[4],
                     bt_addr_to_add->addr[3],
                     bt_addr_to_add->addr[2],
                     bt_addr_to_add->addr[1],
                     bt_addr_to_add->addr[0]);
        cs_host_config.accepted_bt_address_count++;
      }
      break;

      case 'w':
        cs_host_config.use_antenna_wired_offset = true;
        break;

      case 'P':
        initiator_config.conn_phy = sl_bt_gap_phy_1m;
        break;

      case 'o':
      {
        int object_tracking_mode = atoi(optarg);

        if (object_tracking_mode != SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC
            && object_tracking_mode != SL_RTL_CS_ALGO_MODE_STATIC_HIGH_ACCURACY
            && object_tracking_mode != SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST) {
          app_log_info(APP_PREFIX "Invalid object tracking mode (%d) provided!" APP_LOG_NL, object_tracking_mode);
          exit(EXIT_FAILURE);
        } else {
          rtl_config.algo_mode = (uint8_t)object_tracking_mode;
        }
      }
      break;

      case 'p':
      {
        int preset = atoi(optarg);
        if (preset != CS_CHANNEL_MAP_PRESET_LOW
            && preset != CS_CHANNEL_MAP_PRESET_MEDIUM
            && preset != CS_CHANNEL_MAP_PRESET_HIGH
            && preset != CS_CHANNEL_MAP_PRESET_CUSTOM) {
          app_log_error(APP_PREFIX "Unsupported preset (%d) provided!" APP_LOG_NL, preset);
          exit(EXIT_FAILURE);
        }
        initiator_config.channel_map_preset = preset;
        app_log_info(APP_PREFIX "Preset (%d) provided!" APP_LOG_NL, preset);
      }
      break;

      case 'a':
      {
        int cs_tone_antenna_config_idx_req = atoi(optarg);
        if (cs_tone_antenna_config_idx_req != CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY
            && cs_tone_antenna_config_idx_req != CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY
            && cs_tone_antenna_config_idx_req != CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R
            && cs_tone_antenna_config_idx_req != CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R) {
          app_log_error(APP_PREFIX "Invalid antenna usage for PBR (%d) provided!" APP_LOG_NL, cs_tone_antenna_config_idx_req);
          exit(EXIT_FAILURE);
        } else {
          initiator_config.cs_tone_antenna_config_idx_req = (uint8_t)cs_tone_antenna_config_idx_req;
          initiator_cs_tone_antenna_config_index_set = true;
        }
      }
      break;

      case 'd':
      {
        // Device name
        size_t device_name_len = strlen(optarg);
        if (device_name_len > SL_BT_CS_DEVICE_NAME_MAX_LEN) {
          app_log_error(APP_PREFIX "Device name too long (%llu), maximum length is %d characters!" APP_LOG_NL,
                        ((unsigned long long)device_name_len),
                        SL_BT_CS_DEVICE_NAME_MAX_LEN);
          exit(EXIT_FAILURE);
        }
        strncpy((char *)cs_host_config.device_name, optarg, SL_BT_CS_DEVICE_NAME_MAX_LEN);
        cs_host_config.device_name[SL_BT_CS_DEVICE_NAME_MAX_LEN - 1] = '\0'; // Ensure null termination
      }
      break;

      case 'q':
      {
        int items_read = sscanf(optarg, "%x%n", &arg_data, &position);
        if (items_read != 1 || (size_t)position != strlen(optarg)) {
          app_log_error(APP_PREFIX "Invalid antenna usage for RTT (%s) provided!" APP_LOG_NL, optarg);
          exit(EXIT_FAILURE);
        } else {
          if (arg_data != CS_SYNC_ANTENNA_1
              && arg_data != CS_SYNC_ANTENNA_2
              && arg_data != CS_SYNC_SWITCHING) {
            app_log_error(APP_PREFIX "Invalid antenna usage for RTT (%d) provided!" APP_LOG_NL, arg_data);
            exit(EXIT_FAILURE);
          } else {
            initiator_config.cs_sync_antenna_req = (uint8_t)arg_data;
            reflector_config.cs_sync_antenna = (uint8_t)arg_data;
            initiator_cs_sync_antenna_req_set = true;
          }
        }
      }
      break;

      case 'T':
        cs_host_config.trace = true;
        break;

      case 's':
      {
        int procedure_scheduling = atoi(optarg);
        if (procedure_scheduling !=  CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY
            && procedure_scheduling != CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY
            && procedure_scheduling != CS_PROCEDURE_SCHEDULING_CUSTOM) {
          app_log_error(APP_PREFIX "Unsupported procedure scheduling (%d) provided!" APP_LOG_NL,
                        procedure_scheduling);
          exit(EXIT_FAILURE);
        }
        initiator_config.procedure_scheduling = procedure_scheduling;
      }
      break;

      case 'M':
        cs_sub_mode = atoi(optarg);
        break;

      case 'S':
        initiator_config.max_procedure_count = 1; // Synchronized mode
        break;

      default:
        sc = ncp_host_set_option((char)cli_opt, optarg);
        if (sc == SL_STATUS_NOT_FOUND) {
          sc = app_log_set_option((char)cli_opt, optarg);
        }
        if (sc != SL_STATUS_OK) {
          app_log(USAGE, argv[0]);
          exit(EXIT_FAILURE);
        }
        break;
    }
  }

  // Check for unknown parameters and write them to to console if there is any
  if (optind < argc) {
    app_log_critical("Unknown %d parameter", (argc - optind));
    if (argc - optind > 1) {
      app_log_append_critical("s \'");
      for (int i = optind; i < argc; i++) {
        app_log_append_critical("%s", argv[i]);
        if (i < argc - 1) {
          app_log_append_critical(", ");
        }
      }
      app_log_append_critical("\'");
    } else {
      app_log_append_critical(" \'%s\'", argv[optind]);
    }
    app_log_append_critical("! Usage:\n");

    app_log(USAGE, argv[0]);
    exit(EXIT_FAILURE);
  }

  // Sanity check of argument combinations
  if ((cs_host_config.max_initiator_instances + cs_host_config.max_reflector_instances) > MAX_CONNECTIONS) {
    app_log_info(APP_PREFIX "Sum of <max_initiator_instances> and <max_reflector_instances> "
                            "exceeds maximum connection count (%u)" APP_LOG_NL,
                 MAX_CONNECTIONS);
    exit(EXIT_FAILURE);
  }

  if (cs_host_config.max_initiator_instances == 0 && cs_host_config.max_reflector_instances == 0) {
    if (!max_initiator_instances_set && !max_reflector_instances_set) {
      cs_host_config.max_initiator_instances = 1;
      cs_host_config.max_reflector_instances = 1;
      app_log_info(APP_PREFIX "Not specified <max_initiator_instances> and "
                              "<max_reflector_instances>. Using 1-1 of each" APP_LOG_NL);
    } else {
      app_log_error(APP_PREFIX "<max_initiator_instances> and <max_reflector_instances>"
                               " cannot be both zero" APP_LOG_NL);
      exit(EXIT_FAILURE);
    }
  }

  if (cs_host_config.max_initiator_instances > 0) {
    initiator_config.cs_main_mode = cs_main_mode;
    initiator_config.cs_sub_mode = cs_sub_mode;
    if (cs_main_mode == (int)sl_bt_cs_mode_pbr && cs_sub_mode == (int)sl_bt_cs_mode_rtt) {
      // Set mode and submode. Currently, only main mode = pbr and submode = rtt is supported
      initiator_config.channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH;
      app_log_info(APP_PREFIX "Channel map preset set to high" APP_LOG_NL);
    }

    if (!((cs_main_mode == (int)sl_bt_cs_mode_pbr
           && cs_sub_mode == (int)sl_bt_cs_mode_rtt)
          || cs_sub_mode == sl_bt_cs_submode_disabled)) {
      app_log_error(APP_PREFIX "Invalid CS mode/submode (%d/%d) provided" APP_LOG_NL,
                    cs_main_mode,
                    cs_sub_mode);
      exit(EXIT_FAILURE);
    }
    if (initiator_config.cs_main_mode == sl_bt_cs_mode_rtt && initiator_config.channel_map_preset != CS_CHANNEL_MAP_PRESET_HIGH) {
      app_log_error(APP_PREFIX "Only preset HIGH is supported with RTT mode!" APP_LOG_NL);
      exit(EXIT_FAILURE);
    }
    if ((initiator_config.cs_main_mode == sl_bt_cs_mode_rtt)
        && (initiator_config.cs_sub_mode == sl_bt_cs_mode_rtt)) {
      app_log_error(APP_PREFIX "CS main mode RTT and CS sub mode RTT combination is not supported!" APP_LOG_NL);
      exit(EXIT_FAILURE);
    }
    if (initiator_cs_tone_antenna_config_index_set && initiator_config.cs_main_mode == sl_bt_cs_mode_rtt) {
      app_log_warning(APP_PREFIX "PBR antenna configuration is omitted in RTT mode!" APP_LOG_NL);
    }
    if (initiator_cs_sync_antenna_req_set && initiator_config.cs_main_mode == sl_bt_cs_mode_pbr) {
      app_log_warning(APP_PREFIX "RTT antenna configuration is omitted in PBR mode!" APP_LOG_NL);
    }
    if (rtl_config.algo_mode == SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST
        && initiator_config.cs_main_mode == sl_bt_cs_mode_rtt) {
      app_log_error(APP_PREFIX "Real-time fast mode is not supported with main mode RTT!" APP_LOG_NL);
      exit(EXIT_FAILURE);
    }
    // Log mode based on max_procedure_count
    if (initiator_config.max_procedure_count == 1) {
      app_log_info(APP_PREFIX "Start new procedure after one finished." APP_LOG_NL);
    } else {
      app_log_info(APP_PREFIX "Free running." APP_LOG_NL);
    }
    // Log procedure scheduling
    if (initiator_config.procedure_scheduling == CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY) {
      app_log_info(APP_PREFIX "Using frequency optimized procedure scheduling." APP_LOG_NL);
    } else if (initiator_config.procedure_scheduling == CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY) {
      app_log_info(APP_PREFIX "Using energy optimized procedure scheduling." APP_LOG_NL);
    } else {
      app_log_info(APP_PREFIX "Using custom procedure scheduling." APP_LOG_NL);
    }
  } else {
    app_log_info(APP_PREFIX "Only reflector instances - most parameters will be configured by the initiator" APP_LOG_NL);
    initiator_config.cs_main_mode = cs_main_mode;
  }

  // Log configuration parameters
  app_log_info("+-------------------------------------------------------+" APP_LOG_NL);
  app_log_info(APP_PREFIX "Maximum number of reflector instances: %u" APP_LOG_NL,
               cs_host_config.max_reflector_instances);
  app_log_info(APP_PREFIX "Maximum number of initiator instances: %u" APP_LOG_NL,
               cs_host_config.max_initiator_instances);
  if (cs_host_config.max_initiator_instances > 0) {
    if (cs_host_config.accepted_bt_address_count > 0) {
      app_log_info(APP_PREFIX "BLE address filtering: enabled with %u addresses" APP_LOG_NL,
                   cs_host_config.accepted_bt_address_count);
    } else {
      app_log_info(APP_PREFIX "BLE address filtering: disabled" APP_LOG_NL);
    }
    app_log_info(APP_PREFIX "CS main mode: %s (%u)" APP_LOG_NL,
                 (initiator_config.cs_main_mode  == sl_bt_cs_mode_pbr) ? "PBR" : "RTT",
                 (unsigned int)initiator_config.cs_main_mode);
    app_log_info(APP_PREFIX "CS sub mode: %s (%u)" APP_LOG_NL,
                 (initiator_config.cs_sub_mode == sl_bt_cs_submode_disabled) ? "Disabled" : "RTT",
                 (unsigned int)initiator_config.cs_sub_mode);
    app_log_info(APP_PREFIX "CS channel map preset: %d" APP_LOG_NL, initiator_config.channel_map_preset);
    app_log_info(APP_PREFIX "Object tracking mode: %s" APP_LOG_NL,
                 algo_mode_to_str(rtl_config.algo_mode));
  }
  if ((initiator_config.cs_main_mode == sl_bt_cs_mode_pbr && initiator_cs_tone_antenna_config_index_set)
      || (initiator_config.cs_main_mode == sl_bt_cs_mode_rtt && initiator_cs_sync_antenna_req_set)) {
    app_log_info(APP_PREFIX "Requested antenna usage: %s" APP_LOG_NL,
                 antenna_usage_to_str(&initiator_config));
  } else {
    app_log_info(APP_PREFIX "Default antenna usage: %s" APP_LOG_NL,
                 antenna_usage_to_str(&initiator_config));
  }
  app_log_info(APP_PREFIX "RSSI reference TX power @ 1m: %d dBm" APP_LOG_NL,
               (int)initiator_config.rssi_ref_tx_power);
  app_log_info("+-------------------------------------------------------+" APP_LOG_NL);

  // Initialize the list of reflector connection handles
  for (uint32_t i = 0u; i < cs_host_config.max_initiator_instances; i++) {
    cs_host_state.reflector_conn_handles[i] = SL_BT_INVALID_CONNECTION_HANDLE;
  }
  // Initialize the list of initiator connection handles
  for (uint32_t i = 0u; i < cs_host_config.max_reflector_instances; i++) {
    cs_host_state.initiator_conn_handles[i] = SL_BT_INVALID_CONNECTION_HANDLE;
  }

  // Initialize the NCP connection
  sc = ncp_host_init();
  if (sc == SL_STATUS_INVALID_PARAMETER) {
    app_log(USAGE, argv[0]);
    exit(EXIT_FAILURE);
  }
  app_assert_status(sc);
  app_log_info(APP_PREFIX "NCP host initialized" APP_LOG_NL);
  app_log_info(APP_PREFIX "Press Crtl+C to quit" APP_LOG_NL);
  app_log_info("+-------------------------------------------------------+" APP_LOG_NL APP_LOG_NL);
}

void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
}

void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  sc = on_event_security(evt);
  if (sc != SL_STATUS_OK) {
    app_log_error(APP_PREFIX "Security event handler failed: 0x%04lx" APP_LOG_NL, (unsigned long)sc);
  }
  switch (SL_BT_MSG_ID(evt->header)) {
    // --------------------------------
    case sl_bt_evt_system_boot_id:
    {
      if (cs_host_config.trace) {
        enable_trace();
      }

      // Set TX power
      int16_t min_tx_power_x10 = CS_INITIATOR_MIN_TX_POWER_DBM * 10;
      int16_t max_tx_power_x10 = CS_INITIATOR_MAX_TX_POWER_DBM * 10;
      sc = sl_bt_system_set_tx_power(min_tx_power_x10,
                                     max_tx_power_x10,
                                     &min_tx_power_x10,
                                     &max_tx_power_x10);
      app_assert_status(sc);
      app_log_debug(APP_PREFIX "Minimum system TX power is set to: %d dBm" APP_LOG_NL, min_tx_power_x10 / 10);
      app_log_debug(APP_PREFIX "Maximum system TX power is set to: %d dBm" APP_LOG_NL, max_tx_power_x10 / 10);

      // Reset to initial state
      ble_peer_manager_central_init();
      ble_peer_manager_peripheral_init();
      ble_peer_manager_filter_init();
      cs_host_state.num_reflector_connections = 0u;
      cs_host_state.num_initiator_connections = 0u;
      cs_host_state.read_remote_capabilities = false;
      cs_host_state.security_increased = false;

      // Initialize the list of reflector connection handles
      for (uint32_t i = 0u; i < cs_host_config.max_initiator_instances; i++) {
        cs_host_state.reflector_conn_handles[i] = SL_BT_INVALID_CONNECTION_HANDLE;
      }
      // Initialize the list of initiator connection handles
      for (uint32_t i = 0u; i < cs_host_config.max_reflector_instances; i++) {
        cs_host_state.initiator_conn_handles[i] = SL_BT_INVALID_CONNECTION_HANDLE;
      }

      // Print the Bluetooth address
      bd_addr address;
      uint8_t address_type;
      sc = sl_bt_gap_get_identity_address(&address, &address_type);
      app_assert_status(sc);
      app_log_info(APP_PREFIX "Bluetooth %s address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                   address_type ? "static random" : "public device",
                   address.addr[5],
                   address.addr[4],
                   address.addr[3],
                   address.addr[2],
                   address.addr[1],
                   address.addr[0]);

      // Filter for advertised name (CS_RFLCT)
      sc = ble_peer_manager_set_filter_device_name(cs_host_config.device_name, strlen(cs_host_config.device_name), false);
      app_assert_status(sc);

      // Filter for advertised service UUID (RAS)
      sc = ble_peer_manager_set_filter_service_uuid16((sl_bt_uuid_16_t *)&service_uuid);
      app_assert_status(sc);

      // Filter for BLE address if enabled
      if (cs_host_config.accepted_bt_address_count > 0) {
        // Enable Peer Manager BT address filtering
        ble_peer_manager_set_filter_bt_address(true);
        // Add all user specified BT addresses to the allowed list
        for (uint32_t i = 0u; i < cs_host_config.accepted_bt_address_count; i++) {
          sc = ble_peer_manager_add_allowed_bt_address(&cs_host_config.accepted_bt_address_list[i]);
          app_assert_status(sc);
        }
      }

      // Start scanning for reflector connections
      if (cs_host_config.max_initiator_instances > 0 ) {
        sc = ble_peer_manager_central_create_connection();
        app_assert_status(sc);
        app_log_info(APP_PREFIX "Scanning started for reflector connections..." APP_LOG_NL);
      }

      // Start advertising for initiator connections
      if (cs_host_config.max_reflector_instances > 0 ) {
        sc = ble_peer_manager_peripheral_start_advertising(SL_BT_INVALID_ADVERTISING_SET_HANDLE);
        app_assert_status(sc);
        app_log_info(APP_PREFIX "Advertising started for initiator connections..." APP_LOG_NL);
      }

      // Set antenna configuration
      sc = cs_antenna_configure(cs_host_config.use_antenna_wired_offset);
      app_assert_status(sc);

      // If there are initiator instances, read the target side configuration
      if (cs_host_config.max_initiator_instances > 0 ) {
        get_target_config();
      }
      // Set PHY to 2M if possible
      sc = sl_bt_connection_set_default_preferred_phy(initiator_config.conn_phy,
                                                      sl_bt_gap_phy_any);
      app_assert_status(sc);
      break;
    }
    case sl_bt_evt_connection_parameters_id:
      for (uint32_t i = 0u; i < cs_host_config.max_initiator_instances; i++) {
        if (cs_host_state.reflector_conn_handles[i] == evt->data.evt_connection_parameters.connection) {
          if (evt->data.evt_connection_parameters.security_mode != sl_bt_connection_mode1_level1) {
            app_log_info(APP_PREFIX "Increasing security completed for connection %u." APP_LOG_NL,
                         evt->data.evt_connection_parameters.connection);
            if (!cs_host_state.read_remote_capabilities) {
              sc = sl_bt_cs_read_remote_supported_capabilities(evt->data.evt_connection_parameters.connection);
              if (sc == SL_STATUS_INVALID_PARAMETER) {
                app_log_error(APP_PREFIX "Connection not found." APP_LOG_NL);
                sc = ble_peer_manager_central_create_connection();
                app_assert_status(sc);
                app_log_info(APP_PREFIX "Scanning restarted for new reflector connections..." APP_LOG_NL);
              } else {
                app_assert_status(sc);
                cs_host_state.read_remote_capabilities = true;
              }
            }
          } else {
            if (!cs_host_state.security_increased) {
              sc = sl_bt_sm_increase_security(evt->data.evt_connection_parameters.connection);
              if (sc == SL_STATUS_INVALID_PARAMETER) {
                app_log_error(APP_PREFIX "Connection not found." APP_LOG_NL);
                sc = ble_peer_manager_central_create_connection();
                app_assert_status(sc);
                app_log_info(APP_PREFIX "Scanning restarted for new reflector connections..." APP_LOG_NL);
              } else {
                app_assert_status(sc);
                cs_host_state.security_increased = true;
                app_log_info(APP_PREFIX "Increasing security for connection %u" APP_LOG_NL,
                             evt->data.evt_connection_parameters.connection);
              }
            }
          }
          break;
        }
      }
      break;
    case sl_bt_evt_cs_read_remote_supported_capabilities_complete_id:
    {
      uint16_t proc_interval;
      uint16_t conn_interval;
      uint8_t cs_tone_antenna_config_index_temp = initiator_config.cs_tone_antenna_config_idx;
      for (uint32_t i = 0u; i < cs_host_config.max_initiator_instances; i++) {
        if (cs_host_state.reflector_conn_handles[i] == evt->data.evt_cs_read_remote_supported_capabilities_complete.connection) {
          check_supported_capabilities(evt);
          cs_host_state.read_remote_capabilities = false;
          if (initiator_config.max_procedure_count == 0) {
            sc = cs_initiator_get_intervals(initiator_config.cs_main_mode,
                                            initiator_config.cs_sub_mode,
                                            initiator_config.procedure_scheduling,
                                            initiator_config.channel_map_preset,
                                            rtl_config.algo_mode,
                                            initiator_config.cs_tone_antenna_config_idx,
                                            initiator_config.use_real_time_ras_mode,
                                            &conn_interval,
                                            &proc_interval);
            if (sc == SL_STATUS_NOT_SUPPORTED) {
              app_log_info(APP_PREFIX "Parameter optimization is not supported with the given input parameters" APP_LOG_NL);
            } else if (sc == SL_STATUS_IDLE) {
              app_log_info(APP_PREFIX "No optimization - using custom procedure scheduling" APP_LOG_NL);
            } else if (sc == SL_STATUS_OK) {
              initiator_config.max_connection_interval = initiator_config.min_connection_interval = conn_interval;
              initiator_config.max_procedure_interval = initiator_config.min_procedure_interval = proc_interval;
              app_log_info(APP_PREFIX "Optimized parameters for connection interval and procedure interval." APP_LOG_NL);
            } else {
              app_log_error(APP_PREFIX "Invalid input, cannot optimize parameters: %d." APP_LOG_NL, sc);
            }
            float period_ms = initiator_config.max_connection_interval * 1.25f * initiator_config.max_procedure_interval;
            app_log_info(APP_PREFIX "Connection interval: %u  Procedure interval: %u  Period: %d ms  Frequency: %2.3f Hz" APP_LOG_NL,
                         initiator_config.max_connection_interval,
                         initiator_config.max_procedure_interval,
                         (int)period_ms,
                         (1000.0f / period_ms));
            // put remote antenna num into cs_tone_antenna_config_idx
            initiator_config.cs_tone_antenna_config_idx = evt->data.evt_cs_read_remote_supported_capabilities_complete.num_antennas;
          }
          create_initiator(evt->data.evt_cs_read_remote_supported_capabilities_complete.connection);
          // set cs_tone_antenna_config_idx to default
          initiator_config.cs_tone_antenna_config_idx = cs_tone_antenna_config_index_temp;
          break;
        }
      }
    }
    break;

    // --------------------------------
    case sl_bt_evt_gatt_mtu_exchanged_id:
    {
      initiator_config.mtu = evt->data.evt_gatt_mtu_exchanged.mtu;
      app_log_info(APP_PREFIX "MTU exchanged: %d" APP_LOG_NL, initiator_config.mtu);
    }
    break;

    // --------------------------------
    case sl_bt_evt_user_cs_service_message_to_host_id:
    {
      cs_acp_event_t *cs_evt = (cs_acp_event_t *)evt->data.evt_user_cs_service_message_to_host.message.data;
      switch (cs_evt->acp_evt_id) {
        case CS_ACP_EVT_RESULT_ID:
          // Only the result content required
          cs_on_result(cs_evt->connection_id,
                       cs_evt->data.result.type_value_list,
                       (evt->data.evt_user_cs_service_message_to_host.message.len - sizeof(cs_acp_event_id_t) - sizeof(uint8_t)));
          break;

        case CS_ACP_EVT_EXTENDED_RESULT_ID:
          on_extended_result_event(&cs_evt->data.ext_result, cs_evt->connection_id);
          if (cs_evt->data.ext_result.fragments_left & CS_ACP_FIRST_FRAGMENT_MASK
              && cs_evt->data.ext_result.fragment.len >= (sizeof(cs_acp_result_evt_t))) {
            // The first element in the serialized extended result data is the result event.
            // The first byte in it is the result size, and from the second byte on, the result data.
            cs_on_result(cs_evt->connection_id,
                         cs_evt->data.ext_result.fragment.data + 1,
                         cs_evt->data.ext_result.fragment.data[0]);
          }
          break;

        case CS_ACP_EVT_INTERMEDIATE_RESULT_ID:
          cs_on_intermediate_result(&cs_evt->data.intermediate_result, cs_evt->connection_id);
          break;

        case CS_ACP_EVT_STATUS_ID:
          cs_on_error(cs_evt->connection_id, cs_evt->data.stat.error, cs_evt->data.stat.sc);
          break;
        default:
          break;
      }
      break;
    }
    case sl_bt_evt_user_message_to_host_id:
    {
      // handle button press security confirmation
      sl_bt_evt_user_message_to_host_t *msg = &evt->data.evt_user_message_to_host;
      if (security_is_confirmation_in_progress()) {
        if (msg->message.data[0] == 1) {
          security_set_confirmation(SECURITY_ALLOW_CONFIRMATION);
        } else {
          security_set_confirmation(SECURITY_DENY_CONFIRMATION);
        }
      }
      break;
    }
    // -------------------------------
    // This event indicates that the BT stack buffer resources were exhausted
    case sl_bt_evt_system_resource_exhausted_id:
      app_log_error(APP_PREFIX "BT stack buffers exhausted, data loss may have occurred! "
                               "buf_discarded='%u' buf_alloc_fail='%u' heap_alloc_fail='%u'" APP_LOG_NL,
                    evt->data.evt_system_resource_exhausted.num_buffers_discarded,
                    evt->data.evt_system_resource_exhausted.num_buffer_allocation_failures,
                    evt->data.evt_system_resource_exhausted.num_heap_allocation_failures);
      break;
    default:
      break;
  }
}

/******************************************************************************
 * Application process action
 *****************************************************************************/
void app_process_action(void)
{
  sl_status_t sc = security_send_confirmation();
  if (sc != SL_STATUS_OK) {
    app_log_error(APP_PREFIX "Failed to send security confirmation: 0x%04lx" APP_LOG_NL, (unsigned long)sc);
  }
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

/******************************************************************************
 * Application deinit
 *****************************************************************************/
void app_deinit(void)
{
  stop_procedure();
  disable_trace();
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application deinit code here!                       //
  // This is called once during termination.                                 //
  /////////////////////////////////////////////////////////////////////////////
}

static const char *antenna_usage_to_str(const cs_initiator_config_t *config)
{
  if (config->cs_main_mode == sl_bt_cs_mode_rtt) {
    switch (config->cs_sync_antenna_req) {
      case CS_SYNC_ANTENNA_1:
        return "antenna ID 1";
      case CS_SYNC_ANTENNA_2:
        return "antenna ID 2";
      case CS_SYNC_SWITCHING:
        return "switch between all antenna IDs";
      default:
        return "unknown";
    }
  } else {
    switch (config->cs_tone_antenna_config_idx_req) {
      case CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY:
        return "single antenna on both sides (1:1)";
      case CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R:
        return "dual antenna initiator & single antenna reflector (2:1)";
      case CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R:
        return "single antenna initiator & dual antenna reflector (1:2)";
      case CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY:
        return "dual antennas on both sides (2:2)";
      default:
        return "unknown";
    }
  }
}

/******************************************************************************
 * Get algo mode as string
 *****************************************************************************/
static const char *algo_mode_to_str(uint8_t algo_mode)
{
  switch (algo_mode) {
    case SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC:
      return "real time basic (moving)";
    case SL_RTL_CS_ALGO_MODE_STATIC_HIGH_ACCURACY:
      return "stationary object tracking";
    case SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST:
      return "real time fast (moving)";
    default:
      return "unknown";
  }
}

static void enable_trace(void)
{
  if (cs_host_state.trace) {
    return;
  }
  sl_status_t sc;
  cs_acp_cmd_t cmd;
  cmd.cmd_id = CS_ACP_CMD_ENABLE_TRACE;
  cmd.data.enable_trace = 1;
  const size_t cmd_len = sizeof(cmd.cmd_id) + sizeof(cmd.data.enable_trace);

  sc = sl_bt_user_cs_service_message_to_target(cmd_len,
                                               (const uint8_t *)&cmd,
                                               0,
                                               NULL,
                                               NULL);
  if (sc == SL_STATUS_OK) {
    cs_host_state.trace = true;
  } else {
    app_log_status_error_f(sc, APP_PREFIX "Failed to enable trace" APP_LOG_NL);
  }
}

static void check_supported_capabilities(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  uint8_t local_cs_sync_phy;
  uint16_t local_subfeatures;
  uint8_t local_roles;
  uint8_t local_rtt_aa_only;
  uint8_t local_rtt_sounding;
  uint8_t local_rtt_random;
  sc = sl_bt_cs_read_local_supported_capabilities(NULL,
                                                  NULL,
                                                  &initiator_config.num_antennas,
                                                  NULL,
                                                  &local_roles,
                                                  NULL,
                                                  NULL,
                                                  &local_rtt_aa_only,
                                                  &local_rtt_sounding,
                                                  &local_rtt_random,
                                                  NULL,
                                                  NULL,
                                                  &local_cs_sync_phy,
                                                  &local_subfeatures,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL);
  app_assert_status(sc);
  // initiator config is set to CS_SYNC_PHY 2M
  // but local/remote device only supports CS_SYNC_PHY 1M
  if (initiator_config.cs_sync_phy == sl_bt_gap_phy_2m
      && (local_cs_sync_phy == 0 || evt->data.evt_cs_read_remote_supported_capabilities_complete.cs_sync_phys == 0)) {
    app_log_error(APP_PREFIX "Requested CS SYNC PHY (%d) is not supported by the local/remote device" APP_LOG_NL,
                  initiator_config.cs_sync_phy);
    initiator_config.cs_sync_phy = sl_bt_gap_phy_1m;
    app_log_info(APP_PREFIX "Using CS SYNC PHY %d instead" APP_LOG_NL,
                 initiator_config.cs_sync_phy);
  }
  // initiator config algorithm #3c is selected but local/remote device doesn't support it
  if (initiator_config.channel_selection_type == sl_bt_cs_channel_selection_algorithm_3c
      && ((local_subfeatures >> CS_CHANNEL_SELECTION_ALGORITHM_3C_SUBFEATURE_BITMASK & 0x01) == 0
          || ((evt->data.evt_cs_read_remote_supported_capabilities_complete.subfeatures
               >> CS_CHANNEL_SELECTION_ALGORITHM_3C_SUBFEATURE_BITMASK) & 0x01) == 0)) {
    app_log_error(APP_PREFIX "Requested CS channel selection algorithm #3c is not supported by the local/remote device" APP_LOG_NL);
    exit(EXIT_FAILURE);
  }
  // host is started as initiator but local device doesn't support this role
  // or remote device doesn't support reflector role
  if (cs_host_config.max_initiator_instances > 0
      && ((local_roles >> CS_ROLE_INITIATOR_ROLE_BITMASK & 0x01) == 0
          || ((evt->data.evt_cs_read_remote_supported_capabilities_complete.roles
               >> CS_ROLE_REFLECTOR_ROLE_BITMASK) & 0x01) == 0)) {
    app_log_error(APP_PREFIX "Requested CS role is not supported by the local/remote device" APP_LOG_NL);
    exit(EXIT_FAILURE);
  }
  // host is started as reflector but local device doesn't support this role
  if (cs_host_config.max_reflector_instances > 0
      && ((local_roles >> CS_ROLE_REFLECTOR_ROLE_BITMASK) & 0x01) == 0) {
    app_log_error(APP_PREFIX "Requested CS role is not supported by the local/remote device" APP_LOG_NL);
    exit(EXIT_FAILURE);
  }
  // if mode is RTT check the RTT capabilities
  if (initiator_config.cs_main_mode == sl_bt_cs_mode_rtt) {
    // [RTT AA only] is set but local/remote device doesn't support it
    if ((initiator_config.rtt_type == sl_bt_cs_rtt_type_aa_only)
        && (local_rtt_aa_only == 0
            || evt->data.evt_cs_read_remote_supported_capabilities_complete.rtt_aa_only == 0)) {
      app_log_error(APP_PREFIX "RTT AA only is not supported by the local/remote device" APP_LOG_NL);
      exit(EXIT_FAILURE);
    }
    // [RTT Sounding] is set but local/remote device doesn't support it
    if ((initiator_config.rtt_type == sl_bt_cs_rtt_type_fractional_32_bit_sounding
         || initiator_config.rtt_type == sl_bt_cs_rtt_type_fractional_96_bit_sounding)
        && (local_rtt_sounding == 0
            || evt->data.evt_cs_read_remote_supported_capabilities_complete.rtt_sounding == 0)) {
      app_log_error(APP_PREFIX "RTT sounding is not supported by the local/remote device" APP_LOG_NL);
      exit(EXIT_FAILURE);
    }
    // [RTT Random] is set but local/remote device doesn't support it
    if ((initiator_config.rtt_type == sl_bt_cs_rtt_type_fractional_32_bit_random
         || initiator_config.rtt_type == sl_bt_cs_rtt_type_fractional_64_bit_random
         || initiator_config.rtt_type == sl_bt_cs_rtt_type_fractional_96_bit_random
         || initiator_config.rtt_type == sl_bt_cs_rtt_type_fractional_128_bit_random)
        && (local_rtt_random == 0
            || evt->data.evt_cs_read_remote_supported_capabilities_complete.rtt_random_payload == 0)) {
      app_log_error(APP_PREFIX "RTT random is not supported by the local/remote device" APP_LOG_NL);
      exit(EXIT_FAILURE);
    }
  }
}

static void get_target_config(void)
{
  sl_status_t sc;
  cs_acp_cmd_t cmd;
  uint8_t rsp[sizeof(cs_acp_get_target_config_rsp_t)];
  size_t rsp_len = 0;
  cmd.cmd_id = CS_ACP_CMD_GET_TARGET_CONFIG;

  sc = sl_bt_user_cs_service_message_to_target(sizeof(cmd.cmd_id),
                                               (const uint8_t *)&cmd,
                                               sizeof(rsp),
                                               &rsp_len,
                                               rsp);
  app_log_status_error_f(sc, APP_PREFIX "Failed to get target configuration" APP_LOG_NL);
  if (rsp_len == sizeof(cs_acp_get_target_config_rsp_t)) {
    cs_acp_get_target_config_rsp_t *cs_target_config_data = (cs_acp_get_target_config_rsp_t *)rsp;
    if (0 < (cs_target_config_data->target_config_bitfield & (1 << CS_ACP_TARGET_CONFIG_RAS_MODE_BIT_POS))) {
      app_log_info(APP_PREFIX "RAS On demand mode" APP_LOG_NL);
    } else {
      app_log_info(APP_PREFIX "RAS Real time mode" APP_LOG_NL);
    }
  }
}

static void disable_trace(void)
{
  if (!cs_host_state.trace) {
    return;
  }
  sl_status_t sc;
  cs_acp_cmd_t cmd;
  cmd.cmd_id = CS_ACP_CMD_ENABLE_TRACE;
  cmd.data.enable_trace = 0;
  const size_t cmd_len = sizeof(cmd.cmd_id) + sizeof(cmd.data.enable_trace);

  sc = sl_bt_user_cs_service_message_to_target(cmd_len,
                                               (const uint8_t *)&cmd,
                                               0,
                                               NULL,
                                               NULL);
  if (sc == SL_STATUS_OK) {
    cs_host_state.trace = false;
  } else {
    app_log_status_error_f(sc, APP_PREFIX "Failed to disable trace" APP_LOG_NL);
  }
}

static void on_connection_opened_with_reflector(uint8_t conn_handle)
{
  sl_status_t sc;

  // Check if we can accept one more reflector connection
  if (cs_host_state.num_reflector_connections >= cs_host_config.max_initiator_instances) {
    app_log_error(APP_PREFIX "Maximum number of initiator instances (%u) reached, "
                             "dropping connection..." APP_LOG_NL,
                  cs_host_config.max_initiator_instances);
    sc = sl_bt_connection_close(conn_handle);
    app_assert_status(sc);
    return;
  }

  // Store the new reflector connection handle
  bool could_store = false;
  for (uint32_t i = 0u; i < cs_host_config.max_initiator_instances; i++) {
    if (cs_host_state.reflector_conn_handles[i] == SL_BT_INVALID_CONNECTION_HANDLE) {
      cs_host_state.reflector_conn_handles[i] = conn_handle;
      cs_host_state.num_reflector_connections++;
      could_store = true;
      break;
    }
  }
  if (!could_store) {
    app_log_error(APP_PREFIX "Failed to store reflector connection handle" APP_LOG_NL);
    return;
  }
}

static void create_initiator(uint8_t conn_handle)
{
  // Call the ACP API to create a new initiator instance for the connection handle
  sl_status_t sc;
  cs_acp_cmd_t cmd;
  cmd.cmd_id = CS_ACP_CMD_CREATE_INITIATOR;
  cmd.data.initiator_cmd_data.connection_id = conn_handle;
  cmd.data.initiator_cmd_data.extended_result = EXTENDED_RESULT;
  memcpy(&cmd.data.initiator_cmd_data.initiator_config, &initiator_config, sizeof(cmd.data.initiator_cmd_data.initiator_config));
  memcpy(&cmd.data.initiator_cmd_data.rtl_config, &rtl_config, sizeof(cmd.data.initiator_cmd_data.rtl_config));
  cs_initiator_apply_channel_map_preset(cmd.data.initiator_cmd_data.initiator_config.channel_map_preset, cmd.data.initiator_cmd_data.initiator_config.channel_map.data);
  uint8_t rsp[1];
  size_t rsp_len = 0;
  uint8_t instance_id = 0;

  sc = sl_bt_user_cs_service_message_to_target(CREATE_INITATOR_MSG_LEN,
                                               (const uint8_t *)&cmd,
                                               sizeof(rsp),
                                               &rsp_len,
                                               rsp);
  if (sc != SL_STATUS_OK) {
    app_log_status_error_f(sc, APP_PREFIX "Failed to create initiator instance" APP_LOG_NL);
    sl_bt_connection_close(conn_handle);
    return;
  }
  if (rsp_len > 0) {
    instance_id = rsp[0];
  }

  app_log_info(APP_INSTANCE_PREFIX "New initiator instance created with RTL lib instance %u" APP_LOG_NL, conn_handle, instance_id);

  app_log_info(APP_PREFIX "Initiator instances in use: %u/%u" APP_LOG_NL,
               cs_host_state.num_reflector_connections,
               cs_host_config.max_initiator_instances);
  // Scan for new reflector connections if we have room for more
  if (cs_host_state.num_reflector_connections < cs_host_config.max_initiator_instances) {
    sl_status_t sc = ble_peer_manager_central_create_connection();
    app_assert_status(sc);
    app_log_info(APP_PREFIX "Scanning restarted for new reflector connections..." APP_LOG_NL);
  }
}

static void on_connection_opened_with_initiator(uint8_t conn_handle)
{
  // Check if we can accept one more initiator connection
  if (cs_host_state.num_initiator_connections >= cs_host_config.max_reflector_instances) {
    app_log_error(APP_PREFIX "Maximum number of reflector instances (%u) reached, "
                             "dropping connection..." APP_LOG_NL,
                  cs_host_config.max_reflector_instances);
    sl_status_t sc = sl_bt_connection_close(conn_handle);
    app_assert_status(sc);
    return;
  }

  // Store the new initiator connection handle
  for (uint32_t i = 0u; i < cs_host_config.max_reflector_instances; i++) {
    if (cs_host_state.initiator_conn_handles[i] == SL_BT_INVALID_CONNECTION_HANDLE) {
      cs_host_state.initiator_conn_handles[i] = conn_handle;
      cs_host_state.num_initiator_connections++;
      break;
    }
  }
  // Call the ACP API to create a new reflector instance for the connection handle
  sl_status_t sc;
  cs_acp_cmd_t cmd;

  cmd.cmd_id = CS_ACP_CMD_CREATE_REFLECTOR;
  cmd.data.reflector_cmd_data.connection_id = conn_handle;
  memcpy(&cmd.data.reflector_cmd_data.reflector_config, &reflector_config, sizeof(cmd.data.reflector_cmd_data.reflector_config));
  sc = sl_bt_user_cs_service_message_to_target(CREATE_REFLECTOR_MSG_LEN,
                                               (const uint8_t *)&cmd,
                                               0,
                                               NULL,
                                               NULL);
  app_assert_status(sc);
  app_log_info(APP_INSTANCE_PREFIX "New reflector instance created" APP_LOG_NL, conn_handle);

  app_log_info(APP_PREFIX "Reflector instances in use: %u/%u" APP_LOG_NL,
               cs_host_state.num_initiator_connections,
               cs_host_config.max_reflector_instances);

  // Advertise for new initiator connections if we have room for more
  if (cs_host_state.num_initiator_connections < cs_host_config.max_reflector_instances) {
    sl_status_t sc = ble_peer_manager_peripheral_start_advertising(SL_BT_INVALID_ADVERTISING_SET_HANDLE);
    app_assert_status(sc);
    app_log_info(APP_PREFIX "Advertising restarted for new initiator connections..." APP_LOG_NL);
  }
}

static void on_connection_closed(uint8_t conn_handle)
{
  // Check if the connection handle is a reflector connection - if yes remove the corresponding initiator instance
  for (uint32_t i = 0u; i < cs_host_config.max_initiator_instances; i++) {
    if (cs_host_state.reflector_conn_handles[i] == conn_handle) {
      cs_host_state.reflector_conn_handles[i] = SL_BT_INVALID_CONNECTION_HANDLE;
      app_log_info(APP_INSTANCE_PREFIX "Reflector connection closed" APP_LOG_NL, conn_handle);

      // Call the ACP API to remove the initiator instance for the connection handle
      cs_acp_cmd_t cmd;
      sl_status_t sc;

      cmd.cmd_id = CS_ACP_CMD_INITIATOR_ACTION;
      cmd.data.initiator_action_data.connection_id = conn_handle;
      cmd.data.initiator_action_data.initiator_action = CS_ACP_ACTION_DELETE_INITIATOR;
      sc = sl_bt_user_cs_service_message_to_target(INITATOR_ACTION_MSG_LEN,
                                                   (const uint8_t *)&cmd,
                                                   0,
                                                   NULL,
                                                   NULL);
      if ((sc == SL_STATUS_NOT_FOUND) || (sc == SL_STATUS_INVALID_HANDLE)) {
        app_log_warning(APP_INSTANCE_PREFIX "Initiator instance not found" APP_LOG_NL, conn_handle);
      } else {
        app_assert_status(sc);
        app_log_info(APP_INSTANCE_PREFIX "Initiator instance removed" APP_LOG_NL, conn_handle);
      }

      // Restart scanning for new reflector connections
      sc = ble_peer_manager_central_create_connection();
      app_assert_status(sc);
      app_log_info(APP_PREFIX "Scanning restarted for new reflector connections..." APP_LOG_NL);

      cs_host_state.num_reflector_connections--;
      app_log_info(APP_PREFIX "Initiator instances in use: %u/%u" APP_LOG_NL,
                   cs_host_state.num_reflector_connections,
                   cs_host_config.max_initiator_instances);
      break;
    }
  }

  // Check if the connection handle is an initiator connection - if yes remove the corresponding reflector instance
  for (uint32_t i = 0u; i < cs_host_config.max_reflector_instances; i++) {
    if (cs_host_state.initiator_conn_handles[i] == conn_handle) {
      cs_host_state.initiator_conn_handles[i] = SL_BT_INVALID_CONNECTION_HANDLE;
      app_log_info(APP_INSTANCE_PREFIX "Initiator connection closed" APP_LOG_NL, conn_handle);

      // Call the ACP API to remove the reflector instance for the connection handle
      sl_status_t sc;
      cs_acp_cmd_t cmd;

      cmd.cmd_id = CS_ACP_CMD_REFLECTOR_ACTION;
      cmd.data.reflector_action_data.connection_id = conn_handle;
      cmd.data.reflector_action_data.reflector_action = CS_ACP_ACTION_DELETE_REFLECTOR;
      sc = sl_bt_user_cs_service_message_to_target(REFLECTOR_ACTION_MSG_LEN,
                                                   (const uint8_t *)&cmd,
                                                   0,
                                                   NULL,
                                                   NULL);
      if (sc == SL_STATUS_NOT_FOUND) {
        app_log_warning(APP_INSTANCE_PREFIX "Reflector instance not found" APP_LOG_NL, conn_handle);
      } else {
        app_assert_status(sc);
        app_log_info(APP_INSTANCE_PREFIX "Reflector instance removed" APP_LOG_NL, conn_handle);
      }

      // Restart advertising for new initiator connections if we were at the limit
      if (cs_host_state.num_initiator_connections == cs_host_config.max_reflector_instances) {
        sl_status_t sc = ble_peer_manager_peripheral_start_advertising(SL_BT_INVALID_ADVERTISING_SET_HANDLE);
        app_assert_status(sc);
        app_log_info(APP_PREFIX "Advertising restarted for new initiator connections..." APP_LOG_NL);
      }

      cs_host_state.num_initiator_connections--;
      app_log_info(APP_PREFIX "Reflector instances in use: %u/%u" APP_LOG_NL,
                   cs_host_state.num_initiator_connections,
                   cs_host_config.max_reflector_instances);
      break;
    }
  }
}

void app_ble_peer_manager_on_event(ble_peer_manager_evt_type_t *event)
{
  const bd_addr *address;
  switch (event->evt_id) {
    case BLE_PEER_MANAGER_ON_CONN_OPENED_CENTRAL:
      address = ble_peer_manager_get_bt_address(event->connection_id);
      cs_host_state.read_remote_capabilities = false;
      cs_host_state.security_increased = false;
      app_log_info(APP_INSTANCE_PREFIX "Connection opened as central with CS Reflector"
                                       " '%02X:%02X:%02X:%02X:%02X:%02X'" APP_LOG_NL,
                   event->connection_id,
                   address->addr[5],
                   address->addr[4],
                   address->addr[3],
                   address->addr[2],
                   address->addr[1],
                   address->addr[0]);
      on_connection_opened_with_reflector(event->connection_id);
      break;

    case BLE_PEER_MANAGER_ON_CONN_OPENED_PERIPHERAL:
      address = ble_peer_manager_get_bt_address(event->connection_id);
      app_log_info(APP_INSTANCE_PREFIX "Connection opened as peripheral with CS Initiator"
                                       " '%02X:%02X:%02X:%02X:%02X:%02X'" APP_LOG_NL,
                   event->connection_id,
                   address->addr[5],
                   address->addr[4],
                   address->addr[3],
                   address->addr[2],
                   address->addr[1],
                   address->addr[0]);
      on_connection_opened_with_initiator(event->connection_id);
      break;

    case BLE_PEER_MANAGER_ON_CONN_CLOSED:
      app_log_info(APP_INSTANCE_PREFIX "Connection closed" APP_LOG_NL, event->connection_id);
      on_connection_closed(event->connection_id);
      break;

    case BLE_PEER_MANAGER_ON_ADV_STOPPED:
      app_log_info(APP_INSTANCE_PREFIX "Advertisement stopped" APP_LOG_NL, event->connection_id);
      break;

    case BLE_PEER_MANAGER_ERROR:
      app_log_error(APP_INSTANCE_PREFIX "Peer Manager error" APP_LOG_NL, event->connection_id);
      break;

    default:
      app_log_info(APP_INSTANCE_PREFIX "Unhandled Peer Manager event (%u)" APP_LOG_NL, event->connection_id, event->evt_id);
      break;
  }
}

/******************************************************************************
 * Extract and display measurement results
 *****************************************************************************/
static void cs_on_result(const uint8_t conn_handle,
                         const uint8_t *result,
                         uint8_t result_len)
{
  sl_status_t sc = SL_STATUS_OK;
  float value = .0f;
  cs_result_session_data_t result_data;
  
  const bd_addr *bt_address = ble_peer_manager_get_bt_address(conn_handle);
  for (uint8_t is_data = ((measurement_counter % CS_HOST_HEADER_LOG) > 0); is_data <= 1; is_data++) {

    app_log_info(APP_INSTANCE_PREFIX, conn_handle);
    cs_initiator_print_bt_address(!is_data, bt_address);

    memset(&result_data, 0, sizeof(cs_result_session_data_t));
    sc = cs_result_create_session_data((uint8_t *)result, result_len, &result_data);
    if (sc != SL_STATUS_OK) {
      app_log_error(APP_INSTANCE_PREFIX "Failed to create result metadata!" APP_LOG_NL,
                    conn_handle);
      return;
    }

    // --------------------------------
    // Get distance
    if (is_data != 0) {
      sc = cs_result_extract_field(&result_data,
                                   CS_RESULT_FIELD_DISTANCE_MAINMODE,
                                   (uint8_t *)result,
                                   (uint8_t *)&value);
      if (sc != SL_STATUS_OK) {
        app_log_error(APP_INSTANCE_PREFIX "Failed to extract distance value!" APP_LOG_NL,
                      conn_handle);
        return;
      }
    }
    cs_initiator_print_result(CS_RESULT_FIELD_DISTANCE_MAINMODE,
                              !is_data,
                              &value);

    if (initiator_config.cs_sub_mode != sl_bt_cs_submode_disabled) {
      if (is_data != 0) {
        sc = cs_result_extract_field(&result_data,
                                     CS_RESULT_FIELD_DISTANCE_SUBMODE,
                                     (uint8_t *)result,
                                     (uint8_t *)&value);
        if (sc != SL_STATUS_OK) {
          app_log_error(APP_INSTANCE_PREFIX "Failed to extract distance value for sub mode!" APP_LOG_NL,
                        conn_handle);
          return;
        }
      }
      cs_initiator_print_result(CS_RESULT_FIELD_DISTANCE_SUBMODE,
                                !is_data,
                                &value);
    }

    // --------------------------------
    // Get RAW distance
    if (is_data != 0) {
      sc = cs_result_extract_field(&result_data,
                                   CS_RESULT_FIELD_DISTANCE_RAW_MAINMODE,
                                   (uint8_t *)result,
                                   (uint8_t *)&value);
      if (sc != SL_STATUS_OK) {
        app_log_error(APP_INSTANCE_PREFIX "Failed to extract RAW distance value!" APP_LOG_NL,
                      conn_handle);
        return;
      }
    }
    cs_initiator_print_result(CS_RESULT_FIELD_DISTANCE_RAW_MAINMODE,
                              !is_data,
                              &value);

    if (initiator_config.cs_sub_mode != sl_bt_cs_submode_disabled) {
      if (is_data != 0) {
        sc = cs_result_extract_field(&result_data,
                                     CS_RESULT_FIELD_DISTANCE_RAW_SUBMODE,
                                     (uint8_t *)result,
                                     (uint8_t *)&value);
        if (sc != SL_STATUS_OK) {
          app_log_error(APP_INSTANCE_PREFIX "Failed to extract sub mode RAW distance value!" APP_LOG_NL,
                        conn_handle);
        }
      }
      cs_initiator_print_result(CS_RESULT_FIELD_DISTANCE_RAW_SUBMODE,
                                !is_data,
                                &value);
    }

    // --------------------------------
    // Get likeliness
    if (is_data != 0) {
      sc = cs_result_extract_field(&result_data,
                                   CS_RESULT_FIELD_LIKELINESS_MAINMODE,
                                   (uint8_t *)result,
                                   (uint8_t *)&value);
      if (sc != SL_STATUS_OK) {
        app_log_error(APP_INSTANCE_PREFIX "Failed to extract likeliness value!" APP_LOG_NL,
                      conn_handle);
        return;
      }
    }
    cs_initiator_print_result(CS_RESULT_FIELD_LIKELINESS_MAINMODE,
                              !is_data,
                              &value);

    if (initiator_config.cs_sub_mode != sl_bt_cs_submode_disabled) {
      if (is_data != 0) {
        sc = cs_result_extract_field(&result_data,
                                     CS_RESULT_FIELD_LIKELINESS_SUBMODE,
                                     (uint8_t *)result,
                                     (uint8_t *)&value);
        if (sc != SL_STATUS_OK) {
          app_log_error(APP_INSTANCE_PREFIX "Failed to extract sub mode likeliness value!" APP_LOG_NL,
                        conn_handle);
          return;
        }
      }
      cs_initiator_print_result(CS_RESULT_FIELD_LIKELINESS_SUBMODE,
                                !is_data,
                                &value);
    }

    // --------------------------------
    // Get RSSI distance
    if (is_data != 0) {
      sc = cs_result_extract_field(&result_data,
                                   CS_RESULT_FIELD_DISTANCE_RSSI,
                                   (uint8_t *)result,
                                   (uint8_t *)&value);
      if (sc != SL_STATUS_OK) {
        app_log_error(APP_INSTANCE_PREFIX "Failed to extract RSSI distance value!" APP_LOG_NL,
                      conn_handle);
        return;
      }
    }
    cs_initiator_print_result(CS_RESULT_FIELD_DISTANCE_RSSI,
                              !is_data,
                              &value);

    // --------------------------------
    // Get velocity
    if (rtl_config.algo_mode == SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST
        && initiator_config.cs_main_mode == sl_bt_cs_mode_pbr
        && (initiator_config.channel_map_preset == CS_CHANNEL_MAP_PRESET_HIGH
            || initiator_config.channel_map_preset == CS_CHANNEL_MAP_PRESET_MEDIUM)) {
      if (is_data != 0) {
        sc = cs_result_extract_field(&result_data,
                                     CS_RESULT_FIELD_VELOCITY_MAINMODE,
                                     (uint8_t *)result,
                                     (uint8_t *)&value);
        if (sc != SL_STATUS_OK) {
          app_log_error(APP_INSTANCE_PREFIX "Failed to extract velocity value!" APP_LOG_NL,
                        conn_handle);
          return;
        }
      }
      cs_initiator_print_result(CS_RESULT_FIELD_VELOCITY_MAINMODE,
                                !is_data,
                                &value);
    }

    // --------------------------------
    // Get BER (only for RTT)
    if (initiator_config.cs_main_mode == sl_bt_cs_mode_rtt) {
      if (is_data != 0) {
        sc = cs_result_extract_field(&result_data,
                                     CS_RESULT_FIELD_BIT_ERROR_RATE,
                                     (uint8_t *)result,
                                     (uint8_t *)&value);
        if (sc != SL_STATUS_OK) {
          app_log_error(APP_INSTANCE_PREFIX "Failed to extract BER!" APP_LOG_NL,
                        conn_handle);
          return;
        }
      }
      if (!isnan(value)) {
        cs_initiator_print_result(CS_RESULT_FIELD_BIT_ERROR_RATE,
                                  !is_data,
                                  &value);
      }
    }
    app_log_append(APP_LOG_NL);
  }
  measurement_counter++;
}


/******************************************************************************
 * Extract and display intermediate results between measurement results
 * Note: only called when stationary object tracking used
 *****************************************************************************/
static void cs_on_intermediate_result(const cs_acp_intermediate_result_evt_t *intermediate_result,
                                      uint8_t connection)
{
  app_log_info("---" APP_LOG_NL);
  uint32_t percentage_whole = (uint32_t)intermediate_result->progress_percentage;
  uint32_t percentage_frac =
    (uint32_t)((intermediate_result->progress_percentage - (float)percentage_whole) * 100.0f);

  app_log_info(APP_INSTANCE_PREFIX "Estimation in progress: %lu.%02lu%%" APP_LOG_NL,
               connection,
               (unsigned long)percentage_whole,
               (unsigned long)percentage_frac);
}

/******************************************************************************
 * CS error handler
 *****************************************************************************/
static void cs_on_error(uint8_t conn_handle, cs_error_event_t err_evt, sl_status_t sc)
{
  switch (err_evt) {
    // Assert
    case CS_ERROR_EVENT_CS_PROCEDURE_STOP_TIMER_FAILED:
    case CS_ERROR_EVENT_CS_PROCEDURE_UNEXPECTED_DATA:
      app_assert(false,
                 APP_INSTANCE_PREFIX "Unrecoverable CS procedure error happened!"
                                     "[E: 0x%x sc: 0x%x]" APP_LOG_NL,
                 conn_handle,
                 err_evt,
                 sc);
      break;
    case CS_ERROR_EVENT_INITIATOR_FAILED_TO_GET_CHANNEL_MAP:
      app_assert(false,
                 APP_INSTANCE_PREFIX "Failed to create channel map!"
                                     "Input parameters might be wrong!"
                                     "[E: 0x%x sc: 0x%x]" APP_LOG_NL,
                 conn_handle,
                 err_evt,
                 sc);
      break;
    // Discard
    case CS_ERROR_EVENT_RTL_PROCESS_ERROR:
      app_log_error(APP_INSTANCE_PREFIX "RTL processing error happened!"
                                        "[E: 0x%x rtl_err: 0x%x]" APP_LOG_NL,
                    conn_handle,
                    err_evt,
                    sc);
      break;
    case CS_ERROR_EVENT_INITIATOR_FAILED_TO_SET_INTERVALS:
      app_log_error(APP_INSTANCE_PREFIX "Failed to set CS procedure scheduling!"
                                        "[E: 0x%x sc: 0x%x]" APP_LOG_NL,
                    conn_handle,
                    err_evt,
                    sc);
      break;
    // PBR antenna usage not supported
    case CS_ERROR_EVENT_INITIATOR_PBR_ANTENNA_USAGE_NOT_SUPPORTED:
      if (initiator_cs_tone_antenna_config_index_set) {
        app_log_error(APP_INSTANCE_PREFIX "The requested PBR antenna configuration is not supported!"
                                          " Will use the closest one and continue."
                                          "[E: 0x%x sc: 0x%x]" APP_LOG_NL,
                      conn_handle,
                      err_evt,
                      sc);
      } else {
        app_log_info(APP_INSTANCE_PREFIX "Default PBR antenna configuration not supported!"
                                         " Will use the closest one and continue."
                                         "[E: 0x%x sc: 0x%x]" APP_LOG_NL,
                     conn_handle,
                     err_evt,
                     sc);
      }
      break;
    // RTT antenna usage not supported
    case CS_ERROR_EVENT_INITIATOR_RTT_ANTENNA_USAGE_NOT_SUPPORTED:
      if (initiator_cs_sync_antenna_req_set) {
        app_log_error(APP_INSTANCE_PREFIX "The requested RTT antenna configuration is not supported!"
                                          " Will use the closest one and continue."
                                          "[E: 0x%x sc: 0x%x]" APP_LOG_NL,
                      conn_handle,
                      err_evt,
                      sc);
      } else {
        app_log_info(APP_INSTANCE_PREFIX "Default RTT antenna configuration not supported!"
                                         " Will use the closest one and continue."
                                         "[E: 0x%x sc: 0x%x]" APP_LOG_NL,
                     conn_handle,
                     err_evt,
                     sc);
      }
      break;
    case CS_ERROR_EVENT_RAS_CLIENT_REALTIME_RECEIVE_FAILED:
      app_log_error(APP_INSTANCE_PREFIX "RAS reception error!"
                                        "[E: 0x%x sc: 0x%x]" APP_LOG_NL,
                    conn_handle,
                    err_evt,
                    sc);
      break;
    // Close connection
    default:
      app_log_error(APP_INSTANCE_PREFIX "Error happened! Closing connection"
                                        "[E: 0x%x sc: 0x%x]" APP_LOG_NL,
                    conn_handle,
                    err_evt,
                    sc);
      // Common errors
      if (err_evt == CS_ERROR_EVENT_TIMER_ELAPSED) {
        app_log_error(APP_INSTANCE_PREFIX "Operation timeout." APP_LOG_NL, conn_handle);
      } else if (err_evt == CS_ERROR_EVENT_INITIATOR_FAILED_TO_INCREASE_SECURITY) {
        app_log_error(APP_INSTANCE_PREFIX "Security level increase failed." APP_LOG_NL, conn_handle);
      }
      // Close the connection
      app_log_info(APP_INSTANCE_PREFIX "Closing connection" APP_LOG_NL, conn_handle);
      sl_status_t status = ble_peer_manager_central_close_connection(conn_handle);
      // If closing the connection fails no connnection_closed event will be received
      // so we need to restart scanning here if needed
      if (status != SL_STATUS_OK) {
        sc = ble_peer_manager_central_create_connection();
        app_assert_status(sc);
        app_log_info(APP_PREFIX "Scanning restarted for new reflector connections..." APP_LOG_NL);
      }
      break;
  }
}

static void stop_procedure(void)
{
  cs_acp_cmd_t cmd;
  sl_status_t sc;
  uint8_t conn_handle;

  // Close all reflector connections, and delete initiator instances on the NCP
  for (uint32_t i = 0u; i < cs_host_config.max_initiator_instances; i++) {
    conn_handle = cs_host_state.reflector_conn_handles[i];
    if (conn_handle != SL_BT_INVALID_CONNECTION_HANDLE) {
      app_log_info(APP_PREFIX "Removing Initiator instance; connection_handle='%u'" APP_LOG_NL, conn_handle);
      cs_host_state.reflector_conn_handles[i] = SL_BT_INVALID_CONNECTION_HANDLE;

      // Call the ACP API to remove the initiator instance for the connection handle
      cmd.cmd_id = CS_ACP_CMD_INITIATOR_ACTION;
      cmd.data.initiator_action_data.connection_id = conn_handle;
      cmd.data.initiator_action_data.initiator_action = CS_ACP_ACTION_DELETE_INITIATOR;
      sc = sl_bt_user_cs_service_message_to_target(INITATOR_ACTION_MSG_LEN,
                                                   (const uint8_t *)&cmd,
                                                   0,
                                                   NULL,
                                                   NULL);
      if ((sc == SL_STATUS_NOT_FOUND) || (sc == SL_STATUS_INVALID_HANDLE)) {
        app_log_warning(APP_INSTANCE_PREFIX "Initiator instance not found" APP_LOG_NL, conn_handle);
      } else {
        app_assert_status(sc);
        app_log_info(APP_INSTANCE_PREFIX "Initiator instance removed" APP_LOG_NL, conn_handle);
      }
      sc = ble_peer_manager_central_close_connection(conn_handle);
      app_assert_status(sc);
      app_log_info(APP_INSTANCE_PREFIX "Reflector connection closed" APP_LOG_NL, conn_handle);

      cs_host_state.num_reflector_connections--;
      app_log_info(APP_PREFIX "Initiator instances in use: %u/%u" APP_LOG_NL,
                   cs_host_state.num_reflector_connections,
                   cs_host_config.max_initiator_instances);
    }
  }

  // Close all initiator connections, and delete reflector instances on the NCP
  for (uint32_t i = 0u; i < cs_host_config.max_reflector_instances; i++) {
    conn_handle = cs_host_state.initiator_conn_handles[i];
    if (conn_handle != SL_BT_INVALID_CONNECTION_HANDLE) {
      cs_host_state.initiator_conn_handles[i] = SL_BT_INVALID_CONNECTION_HANDLE;

      // Call the ACP API to remove the initiator instance for the connection handle
      cmd.cmd_id = CS_ACP_CMD_REFLECTOR_ACTION;
      cmd.data.reflector_action_data.connection_id = conn_handle;
      cmd.data.reflector_action_data.reflector_action = CS_ACP_ACTION_DELETE_REFLECTOR;
      sc = sl_bt_user_cs_service_message_to_target(REFLECTOR_ACTION_MSG_LEN,
                                                   (const uint8_t *)&cmd,
                                                   0,
                                                   NULL,
                                                   NULL);
      if (sc == SL_STATUS_NOT_FOUND) {
        app_log_warning(APP_INSTANCE_PREFIX "Reflector instance not found" APP_LOG_NL, conn_handle);
      } else {
        app_assert_status(sc);
        app_log_info(APP_INSTANCE_PREFIX "Reflector instance removed" APP_LOG_NL, conn_handle);
      }

      sc = ble_peer_manager_peripheral_close_connection(conn_handle);
      app_assert_status(sc);
      app_log_info(APP_INSTANCE_PREFIX "Initiator connection closed" APP_LOG_NL, conn_handle);

      cs_host_state.num_initiator_connections--;
      app_log_info(APP_PREFIX "Reflector instances in use: %u/%u" APP_LOG_NL,
                   cs_host_state.num_initiator_connections,
                   cs_host_config.max_reflector_instances);
    }
  }
}
