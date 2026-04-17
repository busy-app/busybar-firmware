/***************************************************************************//**
 * @file
 * @brief CS initiator - core implementation
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

// -----------------------------------------------------------------------------
// Includes

#include <stdbool.h>
#include <stdint.h>
#include "sl_bt_api.h"
#include "sl_component_catalog.h"
#include "sl_rtl_clib_api.h"
#include "sl_status.h"

#include "cs_initiator_config.h"
#include "cs_initiator_common.h"
#include "cs_initiator_client.h"
#include "cs_initiator_error.h"
#include "cs_initiator_estimate.h"
#include "cs_initiator_extract.h"
#include "cs_initiator_log.h"
#include "cs_initiator_state_machine.h"
#include "cs_ras_client.h"
#include "cs_ras_format_converter.h"
#include "cs_sync_antenna.h"

#ifdef SL_CATALOG_CS_INITIATOR_REPORT_PRESENT
#include "cs_initiator_report.h"
#else
#define cs_initiator_report(evt)
#endif // SL_CATALOG_CS_INITIATOR_REPORT_PRESENT

#include "cs_initiator.h"

// -----------------------------------------------------------------------------
// Macros

#define INVALID_SERVICE_HANDLE         (UINT32_MAX)

// Length of UUID in bytes
#define UUID_LEN                       16

// -----------------------------------------------------------------------------
// Enums, structs, typedefs

// CS Controller capabilities
typedef struct {
  uint8_t num_config;
  uint16_t max_consecutive_procedures;
  uint8_t num_antennas;
  uint8_t max_antenna_paths;
  uint8_t roles;
  uint8_t optional_modes;
  uint8_t rtt_capability;
  uint8_t rtt_aa_only;
  uint8_t rtt_sounding;
  uint8_t rtt_random_payload;
  uint8_t optional_cs_sync_phys;
  uint16_t optional_subfeatures;
  uint16_t optional_t_ip1_times;
  uint16_t optional_t_ip2_times;
  uint16_t optional_t_fcs_times;
  uint16_t optional_t_pm_times;
  uint8_t t_sw_times;
  uint8_t optional_tx_snr_capability;
} cs_controller_capabilities_t;

// -----------------------------------------------------------------------------
// Static function declarations

static cs_initiator_t *cs_initiator_get_instance(const uint8_t conn_handle);
static cs_initiator_t *cs_initiator_get_free_slot();
static bool cs_initiator_check_connection_parameters(cs_initiator_t *initiator,
                                                     sl_bt_evt_connection_parameters_t *parameters);
static void init_cs_configuration(const uint8_t conn_handle);
static void cs_initiator_select_antennas(uint8_t conn_handle,
                                         uint8_t cs_initiator_local_antenna_num,
                                         uint8_t cs_initiator_remote_antenna_num);
static void process_remote_ranging_data(cs_initiator_t *initiator,
                                        uint8_t *data,
                                        uint32_t data_size);
static bool ras_client_handler(cs_initiator_t *initiator, sl_bt_msg_t *evt);
static void reset_ras_config(cs_initiator_t* initiator);
#if defined (CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE) && (CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE == 0)
static void cs_initiator_get_lost_segments(uint64_t lost_segments,
                                           uint8_t *start_segment,
                                           uint8_t *end_segment);
#endif

// -----------------------------------------------------------------------------
// Static variables

static cs_initiator_t cs_initiator_instances[CS_INITIATOR_MAX_CONNECTIONS];

// RAS client
uint16_t service_uuid = CS_RAS_SERVICE_UUID;
uint16_t char_uuids[CS_RAS_CHARACTERISTIC_INDEX_COUNT] = {
  CS_RAS_CHAR_UUID_RAS_FEATURES,
  CS_RAS_CHAR_UUID_REAL_TIME_RANGING_DATA,
  CS_RAS_CHAR_UUID_CONTROL_POINT,
  CS_RAS_CHAR_UUID_RANGING_DATA_READY,
  CS_RAS_CHAR_UUID_RANGING_DATA_OVERWRITTEN,
  CS_RAS_CHAR_UUID_ON_DEMAND_RANGING_DATA
};

// -----------------------------------------------------------------------------
// Private function definitions

/******************************************************************************
 * Get pointer to initiator instance based on the connection handle.
 *****************************************************************************/
static cs_initiator_t *cs_initiator_get_instance(const uint8_t conn_handle)
{
  for (uint8_t i = 0u; i < CS_INITIATOR_MAX_CONNECTIONS; i++) {
    if (cs_initiator_instances[i].conn_handle == conn_handle) {
      return &cs_initiator_instances[i];
    }
  }
  initiator_log_error("No matching instance found for connection handle %u!" LOG_NL,
                      conn_handle);
  return NULL;
}

/******************************************************************************
 * Get free slot for new initiator instance.
 * @return pointer to the free slot.
 * @return NULL if no free slot is available.
 *****************************************************************************/
static cs_initiator_t *cs_initiator_get_free_slot()
{
  for (uint8_t i = 0u; i < CS_INITIATOR_MAX_CONNECTIONS; i++) {
    if (cs_initiator_instances[i].conn_handle == SL_BT_INVALID_CONNECTION_HANDLE) {
      initiator_log_debug("free slot found." LOG_NL);
      return &cs_initiator_instances[i];
    }
  }
  initiator_log_error("no free slot!" LOG_NL);
  return NULL;
}

/******************************************************************************
 * Check if the connection parameters are set correctly.
 *****************************************************************************/
static bool cs_initiator_check_connection_parameters(cs_initiator_t *initiator,
                                                     sl_bt_evt_connection_parameters_t *parameters)
{
  if (initiator->config.max_connection_interval < parameters->interval
      || initiator->config.min_connection_interval > parameters->interval) {
    initiator_log_warning(INSTANCE_PREFIX "CS - connection interval isn't in the configured range!"
                                          " [configured min: %u, configured max: %u, actual: %u]" LOG_NL,
                          initiator->conn_handle,
                          initiator->config.min_connection_interval,
                          initiator->config.max_connection_interval,
                          parameters->interval);
  } else if (initiator->config.latency != parameters->latency) {
    initiator_log_warning(INSTANCE_PREFIX "CS - latency mismatch!"
                                          " [expected: %u, actual: %u]" LOG_NL,
                          initiator->conn_handle,
                          initiator->config.latency,
                          parameters->latency);
  } else if (initiator->config.timeout != parameters->timeout) {
    initiator_log_warning(INSTANCE_PREFIX "CS - supervision timeout mismatch!"
                                          " [expected: %u, actual: %u]" LOG_NL,
                          initiator->conn_handle,
                          initiator->config.timeout,
                          parameters->timeout);
  } else {
    return true;
  }

  return false;
}

/******************************************************************************
 * Initialize CS configuration.
 *
 * @param[in] conn_handle connection handle
 *****************************************************************************/
static void init_cs_configuration(const uint8_t conn_handle)
{
  sl_status_t sc = SL_STATUS_OK;

  cs_initiator_t *initiator = cs_initiator_get_instance(conn_handle);
  if (initiator == NULL) {
    initiator_log_error(INSTANCE_PREFIX "unknown connection id!" LOG_NL,
                        conn_handle);
    on_error(initiator,
             CS_ERROR_EVENT_INITIATOR_INSTANCE_NULL,
             SL_STATUS_NULL_POINTER);
    return;
  }
  cs_error_event_t initiator_err = CS_ERROR_EVENT_UNHANDLED;

  initiator_log_debug(INSTANCE_PREFIX "CS - set default settings" LOG_NL,
                      initiator->conn_handle);
  sc = sl_bt_cs_set_default_settings(initiator->conn_handle,
                                     sl_bt_cs_role_status_enable,
                                     sl_bt_cs_role_status_disable,
                                     initiator->config.cs_sync_antenna,
                                     initiator->config.max_tx_power_dbm);
  if (sc != SL_STATUS_OK) {
    initiator_log_error(INSTANCE_PREFIX "set default CS settings failed" LOG_NL,
                        initiator->conn_handle);
    on_error(initiator,
             CS_ERROR_EVENT_INITIATOR_FAILED_TO_SET_DEFAULT_CS_SETTINGS,
             sc);
    return;
  }

  initiator_log_debug(INSTANCE_PREFIX "CS - enable security" LOG_NL,
                      initiator->conn_handle);

  sc = sl_bt_cs_security_enable(initiator->conn_handle);
  if (sc != SL_STATUS_OK) {
    initiator_log_error(INSTANCE_PREFIX "CS - security enable failed! [sc: 0x%lx]" LOG_NL,
                        initiator->conn_handle,
                        (unsigned long)sc);
    initiator_err = CS_ERROR_EVENT_INITIATOR_FAILED_TO_ENABLE_CS_SECURITY;
    on_error(initiator,
             initiator_err,
             sc);
    return;
  } else {
    initiator_log_debug(INSTANCE_PREFIX "CS - security enabled." LOG_NL,
                        initiator->conn_handle);
  }

  initiator_log_debug(INSTANCE_PREFIX "CS - create configuration ..." LOG_NL,
                      initiator->conn_handle);
  sc = sl_bt_cs_create_config(initiator->conn_handle,
                              initiator->config.config_id,
                              initiator->config.create_context,
                              initiator->config.cs_main_mode,
                              initiator->config.cs_sub_mode,
                              initiator->config.min_main_mode_steps,
                              initiator->config.max_main_mode_steps,
                              initiator->config.main_mode_repetition,
                              initiator->config.mode0_step,
                              sl_bt_cs_role_initiator,
                              initiator->config.rtt_type,
                              initiator->config.cs_sync_phy,
                              (const sl_bt_cs_channel_map_t*)&initiator->config.channel_map,
                              initiator->config.channel_map_repetition,
                              initiator->config.channel_selection_type,
                              initiator->config.ch3c_shape,
                              initiator->config.ch3c_jump,
                              initiator->config.reserved); // Reserved for future use

  if (sc != SL_STATUS_OK) {
    initiator_log_error(INSTANCE_PREFIX "CS - configuration create failed! [sc: 0x%lx]" LOG_NL,
                        initiator->conn_handle,
                        (unsigned long)sc);
    initiator_err = CS_ERROR_EVENT_INITIATOR_FAILED_TO_CREATE_CONFIG;
    on_error(initiator, initiator_err, sc);
    return;
  } else {
    initiator_log_info(INSTANCE_PREFIX "CS - configuration created. " LOG_NL,
                       initiator->conn_handle);
  }

  if (sc != SL_STATUS_OK) {
    on_error(initiator, initiator_err, sc);
  }
}

static void process_remote_ranging_data(cs_initiator_t *initiator,
                                        uint8_t *data,
                                        uint32_t data_size)
{
  sl_status_t sc;
  cs_ras_ranging_header_t *ranging_header = (cs_ras_ranging_header_t *)data;
  initiator_log_info(INSTANCE_PREFIX "Ranging Data for Procedure %u arrived, size = %lu" LOG_NL,
                     initiator->conn_handle,
                     ranging_header->ranging_counter,
                     data_size);

  // Pass Ranging Data to the state machine
  state_machine_event_data_t evt_data;
  evt_data.evt_ranging_data.ranging_counter = ranging_header->ranging_counter;
  evt_data.evt_ranging_data.initiator_part = false;
  evt_data.evt_ranging_data.data = data;
  evt_data.evt_ranging_data.data_size = data_size;
  // Check done status of the procedure
  evt_data.evt_ranging_data.procedure_state = ranging_data_is_complete(initiator->data.reflector.ranging_data,
                                                                       initiator->data.reflector.ranging_data_size,
                                                                       false,
                                                                       initiator->num_antenna_path);
  sc = initiator_state_machine_event_handler(initiator,
                                             INITIATOR_EVT_RANGING_DATA,
                                             &evt_data);
  if (sc != SL_STATUS_OK) {
    initiator_log_error(INSTANCE_PREFIX "RAS - failed pass reflector ranging data [sc: 0x%lx]" LOG_NL,
                        initiator->conn_handle,
                        (unsigned long)sc);
  }
}

/******************************************************************************
 * RAS client event handler.
 *****************************************************************************/
static bool ras_client_handler(cs_initiator_t *initiator, sl_bt_msg_t *evt)
{
  bool handled = false;
  uint8_t evt_conn_handle = evt->data.evt_gatt_procedure_completed.connection;

  if (evt_conn_handle != initiator->conn_handle) {
    return handled;
  }

  uint16_t procedure_result = evt->data.evt_gatt_procedure_completed.result;
  sl_status_t sc;

  if (procedure_result != SL_STATUS_OK) {
    initiator_log_error(INSTANCE_PREFIX "RAS: GATT procedure failed" LOG_NL,
                        initiator->conn_handle);
    if (initiator->ras_client.state != RAS_STATE_CLIENT_INIT) {
      on_error(initiator,
               CS_ERROR_EVENT_GATT_PROCEDURE_FAILED,
               procedure_result);
    }
    return handled;
  }

  switch (initiator->ras_client.state) {
    case RAS_STATE_SERVICE_DISCOVERY:
    {
      handled = true;
      if (initiator->ras_client.service == INVALID_SERVICE_HANDLE) {
        initiator_log_error(INSTANCE_PREFIX "RAS - service not found!" LOG_NL,
                            initiator->conn_handle);
        on_error(initiator,
                 CS_ERROR_EVENT_RAS_SERVICE_DISCOVERY_FAILED,
                 SL_STATUS_FAIL);
      } else {
        initiator->ras_client.state = RAS_STATE_CHARACTERISTIC_DISCOVERY;
        sc = sl_bt_gatt_discover_characteristics(initiator->conn_handle,
                                                 initiator->ras_client.service);
        if (sc != SL_STATUS_OK) {
          initiator_log_error(INSTANCE_PREFIX "RAS - starting characteristic discovery failed!" LOG_NL,
                              initiator->conn_handle);
          on_error(initiator,
                   CS_ERROR_EVENT_START_CHARACTERISTIC_DISCOVERY_FAILED,
                   sc);
        }
      }
    }
    break;

    case RAS_STATE_CHARACTERISTIC_DISCOVERY:
    {
      handled = true;
      stop_error_timer(initiator);
      if (initiator->ras_client.real_time_mode
          && !initiator->ras_client.gattdb_handles.array[CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA]) {
        initiator_log_error(INSTANCE_PREFIX "RAS - discovery - "
                                            "real time ranging data characteristic not found!" LOG_NL,
                            initiator->conn_handle);
        on_error(initiator,
                 CS_ERROR_EVENT_RAS_REAL_TIME_RANGING_DATA_CHARACTERISTIC_NOT_FOUND,
                 SL_STATUS_FAIL);
        break;
      }
      initiator_log_debug(INSTANCE_PREFIX "RAS - discovery - characteristics found" LOG_NL,
                          initiator->conn_handle);
      sc = cs_ras_client_create(initiator->conn_handle,
                                &initiator->ras_client.gattdb_handles,
                                initiator->config.mtu);
      if (sc != SL_STATUS_OK) {
        initiator_log_error(INSTANCE_PREFIX "RAS - client create failed! [sc: 0x%lx]" LOG_NL,
                            initiator->conn_handle,
                            sc);
        on_error(initiator,
                 CS_ERROR_EVENT_RAS_CLIENT_CREATE_FAILED,
                 sc);
        break;
      } else {
        initiator->ras_client.state = RAS_STATE_CLIENT_INIT;
        initiator_log_debug(INSTANCE_PREFIX "RAS - client create started" LOG_NL,
                            initiator->conn_handle);
      }
    }
    break;

    case RAS_STATE_CLIENT_INIT:
      break;

    default:
      break;
  }
  return handled;
}

/******************************************************************************
 * Set the RAS config flags for indication and notification.
 *****************************************************************************/
static void reset_ras_config(cs_initiator_t* initiator)
{
  initiator->ras_client.real_time_mode
    = CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE;

  initiator->ras_client.service = INVALID_SERVICE_HANDLE;
  initiator->ras_client.mtu = ATT_MTU_MIN;

  initiator->ras_client.config.real_time_ranging_data_indication =
    CS_INITIATOR_RAS_REAL_TIME_INDICATION;
  initiator->ras_client.config.on_demand_ranging_data_indication =
    CS_INITIATOR_RAS_ON_DEMAND_INDICATION;
  initiator->ras_client.config.ranging_data_ready_notification =
    CS_INITIATOR_RAS_DATA_READY_NOTIFICATION;
  initiator->ras_client.config.ranging_data_overwritten_notification =
    CS_INITIATOR_RAS_DATA_OVERWRITTEN_NOTIFICATION;

  initiator->data.reflector.ranging_data_size = 0;
  memset(&initiator->data.reflector.ranging_data,
         0,
         sizeof(initiator->data.reflector.ranging_data));
  initiator->ras_client.overwritten = false;
}

/******************************************************************************
 * Select antennas for the CS mode.
 *****************************************************************************/
static void cs_initiator_select_antennas(uint8_t conn_handle, uint8_t cs_initiator_local_antenna_num, uint8_t cs_initiator_remote_antenna_num)
{
  cs_initiator_t *initiator;
  initiator = cs_initiator_get_instance(conn_handle);
  // Prepare for the CS main mode: PBR antenna usage
  if (initiator->config.cs_main_mode == sl_bt_cs_mode_pbr) {
    switch (initiator->config.cs_tone_antenna_config_idx_req) {
      case CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY:
        initiator->cs_parameters.num_antenna_paths = 1;
        initiator_log_info(INSTANCE_PREFIX "CS - PBR - 1:1 antenna usage set" LOG_NL,
                           initiator->conn_handle);
        break;
      case CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R:
        if (cs_initiator_local_antenna_num < 2) {
          initiator_log_warning(INSTANCE_PREFIX "CS - PBR - 1:1 antenna usage "
                                                "is possible only!" LOG_NL,
                                initiator->conn_handle);
          on_error(initiator,
                   CS_ERROR_EVENT_INITIATOR_PBR_ANTENNA_USAGE_NOT_SUPPORTED,
                   SL_STATUS_FAIL);
          initiator->config.cs_tone_antenna_config_idx_req = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY;
          initiator->cs_parameters.num_antenna_paths = 1;
        } else {
          initiator->cs_parameters.num_antenna_paths = 2;
          initiator_log_info(INSTANCE_PREFIX "CS - PBR - 2:1 antenna usage set" LOG_NL,
                             initiator->conn_handle);
        }
        break;
      case CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R:
        if (cs_initiator_remote_antenna_num < 2) {
          initiator_log_warning(INSTANCE_PREFIX "CS - PBR - 1:1 antenna usage "
                                                "is possible only!" LOG_NL,
                                initiator->conn_handle);
          on_error(initiator,
                   CS_ERROR_EVENT_INITIATOR_PBR_ANTENNA_USAGE_NOT_SUPPORTED,
                   SL_STATUS_FAIL);
          initiator->config.cs_tone_antenna_config_idx_req = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY;
          initiator->cs_parameters.num_antenna_paths = 1;
        } else {
          initiator->cs_parameters.num_antenna_paths = 2;
          initiator_log_info(INSTANCE_PREFIX "CS - PBR - 1:2 antenna usage set" LOG_NL,
                             initiator->conn_handle);
        }
        break;
      case CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY:
        if (cs_initiator_remote_antenna_num >= 2 && cs_initiator_local_antenna_num >= 2) {
          initiator->cs_parameters.num_antenna_paths = 4;
          initiator_log_info(INSTANCE_PREFIX "CS - PBR - 2:2 antenna usage set" LOG_NL,
                             initiator->conn_handle);
        } else {
          on_error(initiator,
                   CS_ERROR_EVENT_INITIATOR_PBR_ANTENNA_USAGE_NOT_SUPPORTED,
                   SL_STATUS_FAIL);
          if (cs_initiator_remote_antenna_num == 1 && cs_initiator_local_antenna_num == 2) {
            initiator->cs_parameters.num_antenna_paths = 2;
            initiator->config.cs_tone_antenna_config_idx_req = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R;
            initiator_log_info(INSTANCE_PREFIX "CS - PBR - 2:1 antenna usage set" LOG_NL,
                               initiator->conn_handle);
          } else if (cs_initiator_remote_antenna_num == 2 && cs_initiator_local_antenna_num == 1) {
            initiator->cs_parameters.num_antenna_paths = 2;
            initiator->config.cs_tone_antenna_config_idx_req = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R;
            initiator_log_info(INSTANCE_PREFIX "CS - PBR - 1:2 antenna usage set" LOG_NL,
                               initiator->conn_handle);
          } else {
            initiator_log_warning(INSTANCE_PREFIX "CS - PBR - 1:1 antenna usage is possible only!" LOG_NL,
                                  initiator->conn_handle);

            initiator->config.cs_tone_antenna_config_idx_req = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY;
            initiator->cs_parameters.num_antenna_paths = 1;
          }
        }
        break;
      default:
        initiator_log_warning(INSTANCE_PREFIX "CS - PBR - unknown antenna usage! "
                                              "Using the default setting: 1:1 antenna" LOG_NL,
                              initiator->conn_handle);
        initiator->cs_parameters.num_antenna_paths = 1;
        initiator->config.cs_tone_antenna_config_idx_req = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY;
        break;
    }
    initiator_log_debug(INSTANCE_PREFIX "CS - PBR - using %u antenna paths" LOG_NL,
                        initiator->conn_handle,
                        initiator->cs_parameters.num_antenna_paths);
  }

  initiator->config.cs_tone_antenna_config_idx = initiator->config.cs_tone_antenna_config_idx_req;
  initiator_log_debug(INSTANCE_PREFIX "Using tone antenna configuration index: %u" LOG_NL,
                      initiator->conn_handle,
                      initiator->config.cs_tone_antenna_config_idx);

  // Prepare for the CS main mode: RTT antenna usage
  if (initiator->config.cs_main_mode == sl_bt_cs_mode_rtt) {
    switch (initiator->config.cs_sync_antenna_req) {
      case CS_SYNC_ANTENNA_1:
        initiator_log_warning(INSTANCE_PREFIX "CS - RTT - 1. antenna device! Using the antenna ID 1" LOG_NL,
                              initiator->conn_handle);
        initiator->config.cs_sync_antenna = CS_SYNC_ANTENNA_1;
        break;
      case CS_SYNC_ANTENNA_2:
        if (cs_initiator_local_antenna_num >= 2) {
          initiator_log_warning(INSTANCE_PREFIX "CS - RTT - 2. antenna device! Using the antenna ID 1" LOG_NL,
                                initiator->conn_handle);
          initiator->config.cs_sync_antenna = CS_SYNC_ANTENNA_2;
        } else {
          initiator_log_warning(INSTANCE_PREFIX "CS - RTT - only 1 antenna device! Using the antenna ID 1" LOG_NL,
                                initiator->conn_handle);
          initiator->config.cs_sync_antenna = CS_SYNC_ANTENNA_1;
          on_error(initiator,
                   CS_ERROR_EVENT_INITIATOR_RTT_ANTENNA_USAGE_NOT_SUPPORTED,
                   SL_STATUS_FAIL);
        }
        break;
      case CS_SYNC_SWITCHING:
        initiator_log_info(INSTANCE_PREFIX "CS - RTT - switching between %u available antennas" LOG_NL,
                           initiator->conn_handle,
                           initiator->config.num_antennas);
        initiator->config.cs_sync_antenna = CS_SYNC_SWITCHING;
        break;
      default:
        initiator_log_warning(INSTANCE_PREFIX "CS - RTT - unknown antenna usage! "
                                              "Using the default setting: antenna ID 1" LOG_NL,
                              initiator->conn_handle);
        initiator->config.cs_sync_antenna_req = CS_SYNC_ANTENNA_1;
        break;
    }
    // In case of RTT num_antenna_paths is ignored
    initiator->cs_parameters.num_antenna_paths = 0;
  }
}

#if defined (CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE) && (CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE == 0)
/******************************************************************************
 * Get start and end segments of the lost_segments bitfield.
 *****************************************************************************/
static void cs_initiator_get_lost_segments(uint64_t lost_segments,
                                           uint8_t *start_segment,
                                           uint8_t *end_segment)
{
  bool found_start_segment = false;
  uint64_t bitmask;
  for (uint64_t i = 0; i < sizeof(uint64_t) * 8; i++) {
    bitmask = 1ULL << i;
    if ((bitmask & lost_segments) > 0) {
      if (!found_start_segment) {
        *start_segment = i;
        *end_segment = i;
        found_start_segment = true;
      } else {
        *end_segment = i;
      }
    }
  }
}
#endif

// -----------------------------------------------------------------------------
// Public function definitions

/******************************************************************************
 * Init function that sets the first discovery and initiator config
 * and register the selected callback function pointer as a
 * callback for the distance measurement.
 *****************************************************************************/
sl_status_t cs_initiator_create(const uint8_t               conn_handle,
                                cs_initiator_config_t       *initiator_config,
                                const rtl_config_t          *rtl_config,
                                cs_result_cb_t              result_cb,
                                cs_intermediate_result_cb_t intermediate_result_cb,
                                cs_error_cb_t               error_cb,
                                uint8_t                     *instance_id)
{
  sl_status_t sc = SL_STATUS_OK;
  enum sl_rtl_error_code rtl_err = SL_RTL_ERROR_SUCCESS;
  uint32_t enabled_channels;
  cs_error_event_t initiator_err = CS_ERROR_EVENT_UNHANDLED;
  uint8_t cs_initiator_local_antenna_num;
  uint8_t cs_initiator_remote_antenna_num;

  if (conn_handle == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (initiator_config == NULL
      || result_cb == NULL
      || error_cb == NULL
      || intermediate_result_cb == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  initiator_log_info(INSTANCE_PREFIX "Creating new initiator instance..." LOG_NL, conn_handle);

  cs_initiator_t *initiator = cs_initiator_get_free_slot();
  if (initiator == NULL) {
    initiator_log_error(INSTANCE_PREFIX "no more free slots available!" LOG_NL,
                        conn_handle);
    return SL_STATUS_FULL;
  }

  initiator_log_debug(INSTANCE_PREFIX "clean-up initiator and reflector data" LOG_NL,
                      initiator->conn_handle);
  memset(initiator, 0, sizeof(cs_initiator_t));
  reset_subevent_data(initiator, false);
  reset_ras_config(initiator);

  // Assign connection handle
  initiator->conn_handle = conn_handle;
  initiator_log_debug(INSTANCE_PREFIX "load initiator configuration" LOG_NL,
                      initiator->conn_handle);
  memcpy(&initiator->config, initiator_config, sizeof(cs_initiator_config_t));
  cs_initiator_local_antenna_num = initiator->config.num_antennas;
  cs_initiator_remote_antenna_num = initiator->config.cs_tone_antenna_config_idx;
  initiator_log_info(INSTANCE_PREFIX "CS - number of antennas received"
                                     "[local antennas: %u, remote antennas: %u]" LOG_NL,
                     initiator->conn_handle,
                     cs_initiator_local_antenna_num,
                     cs_initiator_remote_antenna_num);

  sc  = sl_bt_connection_get_security_status(initiator->conn_handle, &initiator->security_mode, NULL, NULL);
  if (sc != SL_STATUS_OK) {
    initiator_log_error(INSTANCE_PREFIX "failed to get security status" LOG_NL,
                        initiator->conn_handle);
    initiator_err = CS_ERROR_EVENT_INITIATOR_FAILED_TO_GET_SECURITY_STATUS;
    goto cleanup;
  }

  if (initiator->security_mode != sl_bt_connection_mode1_level1) {
    initiator_log_debug(INSTANCE_PREFIX "connection already encrypted [level: %u]" LOG_NL,
                        initiator->conn_handle,
                        initiator->security_mode);
  } else {
    initiator_log_debug(INSTANCE_PREFIX "connection not encrypted yet, increase security" LOG_NL,
                        initiator->conn_handle);

    sc = sl_bt_sm_increase_security(initiator->conn_handle);
    if (sc != SL_STATUS_OK) {
      initiator_log_error(INSTANCE_PREFIX "failed to increase security!" LOG_NL,
                          initiator->conn_handle);
      initiator_err = CS_ERROR_EVENT_INITIATOR_FAILED_TO_INCREASE_SECURITY;
      goto cleanup;
    }
  }

  // take over RTL lib configuration into the instance
  memcpy(&initiator->rtl_config,
         rtl_config,
         sizeof(initiator->rtl_config));

  if (initiator->rtl_config.algo_mode == SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC) {
    initiator_log_info(INSTANCE_PREFIX "RTL - algo mode selected: real-time basic"
                                       "(moving objects tracking)" LOG_NL,
                       initiator->conn_handle);
  } else if (initiator->rtl_config.algo_mode == SL_RTL_CS_ALGO_MODE_STATIC_HIGH_ACCURACY) {
    initiator_log_info(INSTANCE_PREFIX "RTL - algo mode selected: static high accuracy "
                                       "(stationary object tracking)" LOG_NL,
                       initiator->conn_handle);
  } else if (initiator->rtl_config.algo_mode == SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST) {
    initiator_log_info(INSTANCE_PREFIX "RTL - algo mode selected: real time fast "
                                       "(moving object fast)" LOG_NL,
                       initiator->conn_handle);
  } else {
    initiator_log_warning(INSTANCE_PREFIX "unknown algo_mode: %u!"
                                          "Will use the default setting: real-time basic "
                                          "(moving objects tracking)!" LOG_NL,
                          initiator->conn_handle, initiator->rtl_config.algo_mode);
  }
  initiator_log_debug(INSTANCE_PREFIX "ch3c_jump=%u, ch3c_shape=%u" LOG_NL,
                      conn_handle,
                      initiator->cs_parameters.ch3c_jump,
                      initiator->cs_parameters.ch3c_shape);
  initiator_log_debug(INSTANCE_PREFIX "channel_map_repetition=%u, cs_sync_phy=%u" LOG_NL,
                      conn_handle,
                      initiator->cs_parameters.channel_map_repetition,
                      initiator->cs_parameters.cs_sync_phy);
  initiator_log_debug(INSTANCE_PREFIX "main_mode_repetition=%lu, rtt_type=%u" LOG_NL,
                      conn_handle,
                      (unsigned long)initiator->cs_parameters.main_mode_repetition,
                      initiator->cs_parameters.rtt_type);

  initiator_log_debug(INSTANCE_PREFIX "initialize discover state machine" LOG_NL,
                      initiator->conn_handle);

  initiator->result_cb = result_cb;
  initiator->intermediate_result_cb = intermediate_result_cb;
  initiator->error_cb = error_cb;
  initiator_log_debug(INSTANCE_PREFIX "registered callbacks" LOG_NL,
                      initiator->conn_handle);

  // Validate the channel map
  rtl_err = sl_rtl_util_validate_bluetooth_cs_channel_map(initiator->config.cs_main_mode,
                                                          initiator->rtl_config.algo_mode,
                                                          initiator->config.channel_map.data);
  if (rtl_err != SL_RTL_ERROR_SUCCESS) {
    initiator_log_error(INSTANCE_PREFIX "RTL - invalid channel map! [E: 0x%x]" LOG_NL,
                        conn_handle,
                        rtl_err);
    initiator_err = CS_ERROR_EVENT_INITIATOR_FAILED_TO_GET_CHANNEL_MAP;
    sc = SL_STATUS_INVALID_PARAMETER;
    goto cleanup;
  } else {
    initiator_log_debug(INSTANCE_PREFIX "RTL - channel map validated." LOG_NL,
                        initiator->conn_handle);
  }

  enabled_channels =
    get_num_tones_from_channel_map(initiator->config.channel_map.data,
                                   sizeof(initiator->config.channel_map));

  initiator_log_info(INSTANCE_PREFIX "CS channel map - channel count: %lu" LOG_NL,
                     initiator->conn_handle,
                     (unsigned long)enabled_channels);
  (void)enabled_channels;

  cs_initiator_select_antennas(initiator->conn_handle, cs_initiator_local_antenna_num, cs_initiator_remote_antenna_num);
  uint16_t conn_interval;
  uint16_t proc_interval;
  // Set optimized intervals for PBR mode
  if (initiator->config.max_procedure_count == 0) {
    sc = cs_initiator_get_intervals(initiator->config.cs_main_mode,
                                    initiator->config.cs_sub_mode,
                                    initiator->config.procedure_scheduling,
                                    initiator->config.channel_map_preset,
                                    initiator->rtl_config.algo_mode,
                                    initiator->config.cs_tone_antenna_config_idx,
                                    initiator->config.use_real_time_ras_mode,
                                    &conn_interval,
                                    &proc_interval);
    if (sc != SL_STATUS_OK) {
      if (sc == SL_STATUS_NOT_SUPPORTED) {
        initiator_log_warning(INSTANCE_PREFIX "Parameter optimization is not supported in RTT mode or with CUSTOM preset" LOG_NL,
                              initiator->conn_handle);
      } else if (sc == SL_STATUS_IDLE) {
        initiator_log_warning(INSTANCE_PREFIX "No optimization - using custom procedure scheduling" LOG_NL,
                              initiator->conn_handle);
      } else {
        initiator_log_error(INSTANCE_PREFIX "CS - failed to set procedure and connection intervals! "
                                            "[sc: 0x%04lx]" LOG_NL,
                            initiator->conn_handle,
                            (unsigned long)sc);
        initiator_err = CS_ERROR_EVENT_INITIATOR_FAILED_TO_SET_INTERVALS;
        goto cleanup;
      }
    } else {
      initiator->config.max_connection_interval = initiator->config.min_connection_interval = conn_interval;
      initiator->config.max_procedure_interval = initiator->config.min_procedure_interval = proc_interval;
      initiator_log_info(INSTANCE_PREFIX "CS - optimized intervals: conn_interval: %u, proc_interval: %u" LOG_NL,
                         initiator->conn_handle,
                         initiator->config.max_connection_interval,
                         initiator->config.max_procedure_interval);
    }
  }

  // Request connection parameter update.
  sc = sl_bt_connection_set_parameters(initiator->conn_handle,
                                       initiator->config.min_connection_interval,
                                       initiator->config.max_connection_interval,
                                       initiator->config.latency,
                                       initiator->config.timeout,
                                       initiator->config.min_ce_length,
                                       initiator->config.max_ce_length);
  if (sc != SL_STATUS_OK) {
    initiator_log_error(INSTANCE_PREFIX "CS - failed to set connection parameters! "
                                        "[sc: 0x%04lx]" LOG_NL,
                        initiator->conn_handle,
                        (unsigned long)sc);
    initiator_err =
      CS_ERROR_EVENT_INITIATOR_FAILED_TO_SET_CONNECTION_PARAMETERS;
    goto cleanup;
  }

  sc = sl_bt_connection_set_preferred_phy(conn_handle, initiator->config.conn_phy, sl_bt_gap_phy_any);
  if (sc != SL_STATUS_OK) {
    initiator_log_error(INSTANCE_PREFIX "CS - failed to set connection PHY! "
                                        "[sc: 0x%04lx]" LOG_NL,
                        initiator->conn_handle,
                        (unsigned long)sc);
    initiator_err =
      CS_ERROR_EVENT_INITIATOR_FAILED_TO_SET_CONNECTION_PHY;
    goto cleanup;
  }

  initiator_log_debug(INSTANCE_PREFIX "CS - set connection parameters ..." LOG_NL,
                      initiator->conn_handle);

  // trying to initialize RTL lib within error-timeout
  initiator_log_debug(INSTANCE_PREFIX "RTL - initialize lib item" LOG_NL,
                      initiator->conn_handle);
  rtl_err = rtl_library_init(initiator->conn_handle,
                             &initiator->rtl_handle,
                             &initiator->rtl_config,
                             &initiator->instance_id);
  if (rtl_err != SL_RTL_ERROR_SUCCESS) {
    initiator_log_error(INSTANCE_PREFIX "RTL - failed to init lib item! [E: 0x%x]" LOG_NL,
                        initiator->conn_handle,
                        rtl_err);
    initiator_err = CS_ERROR_EVENT_INITIATOR_FAILED_TO_INIT_RTL_LIB;
    goto cleanup;
  }
  initiator_log_info(INSTANCE_PREFIX "RTL - lib item initialized." LOG_NL,
                     initiator->conn_handle);
  if (instance_id != NULL) {
    *instance_id = initiator->instance_id;
  }
  (void)initiator_state_machine_event_handler(initiator,
                                              INITIATOR_EVT_INIT_STARTED,
                                              NULL);
  cleanup:
  if (sc != SL_STATUS_OK) {
    on_error(initiator, initiator_err, sc);
    return sc;
  }
  if (rtl_err != SL_RTL_ERROR_SUCCESS) {
    on_error(initiator, initiator_err, rtl_err);
    sc = SL_STATUS_FAIL;
  }
  return sc;
}

/******************************************************************************
 * Initialize instance slots.
 *****************************************************************************/
void cs_initiator_init(void)
{
  for (uint8_t i = 0u; i < CS_INITIATOR_MAX_CONNECTIONS; i++) {
    cs_initiator_t *initiator = &cs_initiator_instances[i];
    memset(initiator, 0, sizeof(cs_initiator_instances[0]));
    initiator->conn_handle = SL_BT_INVALID_CONNECTION_HANDLE;
    initiator->initiator_state = (uint8_t)INITIATOR_STATE_UNINITIALIZED;
    initiator->antenna_config = INVALID_ANTENNA_CONF;
    reset_ras_config(initiator);
    reset_subevent_data(initiator, true);
  }

  cs_initiator_report(CS_INITIATOR_REPORT_INIT);
}

/******************************************************************************
 * Delete existing initiator instance after closing its connection to
 * reflector(s).
 *****************************************************************************/
sl_status_t cs_initiator_delete(const uint8_t conn_handle)
{
  sl_status_t sc;
  cs_initiator_t *initiator = cs_initiator_get_instance(conn_handle);
  if (initiator == NULL) {
    return SL_STATUS_NOT_FOUND;
  }
  if (initiator->conn_handle == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  sc = initiator_state_machine_event_handler(initiator,
                                             INITIATOR_EVT_DELETE_INSTANCE,
                                             NULL);
  return sc;
}

/******************************************************************************
 * Deinitialization function of CS Initiator.
 * Delete initiator for all existing instances.
 *****************************************************************************/
void cs_initiator_deinit(void)
{
  for (uint8_t i = 0u; i < CS_INITIATOR_MAX_CONNECTIONS; i++) {
    cs_initiator_t *initiator = &cs_initiator_instances[i];
    if (initiator->conn_handle != SL_BT_INVALID_CONNECTION_HANDLE) {
      cs_initiator_delete(initiator->conn_handle);
    }
  }
}

// -----------------------------------------------------------------------------
// Event / callback definitions

/******************************************************************************
 * RAS client initialized callback.
 *****************************************************************************/
void cs_ras_client_on_initialized(uint8_t connection,
                                  cs_ras_features_t features,
                                  sl_status_t sc_in)
{
  cs_initiator_t *initiator = cs_initiator_get_instance(connection);
  if (initiator == NULL) {
    initiator_log_error(INSTANCE_PREFIX "RAS - unknown connection id!" LOG_NL,
                        connection);
    on_error(initiator,
             CS_ERROR_EVENT_INITIATOR_INSTANCE_NULL,
             SL_STATUS_NULL_POINTER);
    return;
  }

  if (initiator->ras_client.config.real_time_ranging_data_indication
      && !(features & CS_RAS_FEATURE_RT_RANGING_DATA_MASK)) {
    initiator_log_error(INSTANCE_PREFIX "RAS - client initialized - "
                                        "real-time ranging data indication feature not supported!" LOG_NL,
                        initiator->conn_handle);
    on_error(initiator,
             CS_ERROR_EVENT_RAS_CLIENT_INIT_FEATURE_NOT_SUPPORTED,
             sc_in);
    return;
  }

  if (sc_in != SL_STATUS_OK) {
    initiator_log_error(INSTANCE_PREFIX "RAS - failed to initialize client! [sc: 0x%lx]" LOG_NL,
                        initiator->conn_handle,
                        (unsigned long)sc_in);
    on_error(initiator,
             CS_ERROR_EVENT_RAS_CLIENT_INIT_FAILED,
             sc_in);
    return;
  }

  initiator_log_info(INSTANCE_PREFIX "RAS - client initialized [features: 0x%08lx]" LOG_NL,
                     initiator->conn_handle,
                     features);

  sl_status_t sc = cs_ras_client_configure(initiator->conn_handle, initiator->ras_client.config);
  if (sc != SL_STATUS_OK) {
    initiator_log_error(INSTANCE_PREFIX "RAS - failed to configure client! [sc: 0x%lx]" LOG_NL,
                        initiator->conn_handle,
                        (unsigned long)sc);
    on_error(initiator,
             CS_ERROR_EVENT_RAS_CLIENT_CONFIG_FAILED,
             sc);
    return;
  }

  initiator_log_debug(INSTANCE_PREFIX "RAS - client configured." LOG_NL
                      " - real-time ranging indication: %s" LOG_NL
                      " - on-demand ranging indication: %s" LOG_NL
                      " - ranging data ready notification: %s" LOG_NL
                      " - ranging data overwritten notification: %s" LOG_NL,
                      initiator->conn_handle,
                      ((initiator->ras_client.config.real_time_ranging_data_indication) ? "on" : "off"),
                      ((initiator->ras_client.config.on_demand_ranging_data_indication) ? "on" : "off"),
                      ((initiator->ras_client.config.ranging_data_ready_notification) ? "on" : "off"),
                      ((initiator->ras_client.config.ranging_data_overwritten_notification) ? "on" : "off"));
  #if defined (CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE) && (CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE == 0)
  cs_ras_mode_t ras_mode = CS_RAS_MODE_ON_DEMAND_RANGING_DATA;
  #else
  cs_ras_mode_t ras_mode = CS_RAS_MODE_REAL_TIME_RANGING_DATA;
  #endif
  sc = cs_ras_client_select_mode(initiator->conn_handle, ras_mode);
  if (sc != SL_STATUS_OK) {
    initiator_log_error(INSTANCE_PREFIX "RAS - failed to select mode! [sc: 0x%lx]" LOG_NL,
                        initiator->conn_handle,
                        (unsigned long)sc);
    on_error(initiator,
             CS_ERROR_EVENT_RAS_CLIENT_MODE_CHANGE_FAILED,
             sc);
    return;
  }
}

/******************************************************************************
 * RAS client on mode change callback.
 *****************************************************************************/
void cs_ras_client_on_mode_changed(uint8_t       connection,
                                   cs_ras_mode_t mode,
                                   sl_status_t   sc_in)
{
  cs_initiator_t *initiator = cs_initiator_get_instance(connection);
  if (initiator == NULL) {
    initiator_log_error(INSTANCE_PREFIX "RAS - mode change - unknown connection id!" LOG_NL,
                        connection);
    on_error(initiator,
             CS_ERROR_EVENT_RAS_CLIENT_MODE_CHANGE_FAILED,
             SL_STATUS_NULL_POINTER);
    return;
  }

  if (sc_in != SL_STATUS_OK) {
    initiator_log_error(INSTANCE_PREFIX "RAS - failed to change mode to %u! [sc: 0x%lx]" LOG_NL,
                        connection,
                        mode,
                        (unsigned long)sc_in);
    on_error(initiator,
             CS_ERROR_EVENT_RAS_CLIENT_MODE_CHANGE_FAILED,
             sc_in);
    return;
  }

  state_machine_event_data_t evt_data;
  sl_status_t sc;

  initiator_log_debug(INSTANCE_PREFIX "RAS - mode changed to %u" LOG_NL,
                      initiator->conn_handle,
                      mode);

  switch (mode) {
    case CS_RAS_MODE_REAL_TIME_RANGING_DATA:
      // Emit the event for the initialization phase
      if (initiator->ras_client.state != RAS_STATE_MODE_REAL_TIME_REENABLE) {
        evt_data.evt_init_completed = true;
        (void)initiator_state_machine_event_handler(initiator,
                                                    INITIATOR_EVT_INIT_COMPLETED,
                                                    &evt_data);
      } else {
        sc = cs_ras_client_real_time_receive(initiator->conn_handle,
                                             sizeof(initiator->data.reflector.ranging_data),
                                             initiator->data.reflector.ranging_data);
        if (sc != SL_STATUS_OK) {
          initiator_log_error(INSTANCE_PREFIX "RAS - failed to receive real-time data! [sc: 0x%lx]" LOG_NL,
                              initiator->conn_handle,
                              (unsigned long)sc);
          on_error(initiator,
                   CS_ERROR_EVENT_RAS_CLIENT_REALTIME_RECEIVE_FAILED,
                   sc);
        }
      }
      initiator->ras_client.state = RAS_STATE_MODE_REAL_TIME;
      break;
    case CS_RAS_MODE_ON_DEMAND_RANGING_DATA:
      initiator->ras_client.state = RAS_STATE_MODE_ON_DEMAND;
      initiator->ras_client.overwritten = false;
      evt_data.evt_init_completed = true;
      (void)initiator_state_machine_event_handler(initiator,
                                                  INITIATOR_EVT_INIT_COMPLETED,
                                                  &evt_data);
      break;
    case CS_RAS_MODE_CHANGE_IN_PROGRESS:
      initiator_log_debug(INSTANCE_PREFIX "RAS - mode change in progress ..." LOG_NL,
                          initiator->conn_handle);
      break;
    case CS_RAS_MODE_NONE:
      if (initiator->ras_client.state == RAS_STATE_MODE_REAL_TIME) {
        initiator->ras_client.state = RAS_STATE_MODE_REAL_TIME_REENABLE;
        sc = cs_ras_client_select_mode(initiator->conn_handle,
                                       CS_RAS_MODE_REAL_TIME_RANGING_DATA);
        if (sc != SL_STATUS_OK) {
          initiator_log_error(INSTANCE_PREFIX "RAS - failed to select mode! [sc: 0x%lx]" LOG_NL,
                              initiator->conn_handle,
                              (unsigned long)sc);
          on_error(initiator,
                   CS_ERROR_EVENT_RAS_CLIENT_MODE_CHANGE_FAILED,
                   sc);
          return;
        }
      }
      break;
    default:
      break;
  }
}

/******************************************************************************
 * RAS client callback that indicates the end of reception of ranging data.
 *****************************************************************************/
void cs_ras_client_on_ranging_data_reception_finished(uint8_t                         connection,
                                                      bool                            real_time,
                                                      bool                            retrieve_lost,
                                                      sl_status_t                     sc,
                                                      cs_ras_cp_response_code_value_t response,
                                                      cs_ras_ranging_counter_t        ranging_counter,
                                                      uint8_t                         start_segment,
                                                      uint8_t                         end_segment,
                                                      bool                            recoverable,
                                                      uint32_t                        size,
                                                      bool                            last_arrived,
                                                      uint8_t                         last_known_segment,
                                                      uint64_t                        lost_segments)
{
  sl_status_t status;
  cs_initiator_t *initiator = cs_initiator_get_instance(connection);
  if (initiator == NULL) {
    initiator_log_error(INSTANCE_PREFIX "RAS - reception finished - unknown connection id!" LOG_NL,
                        connection);
    on_error(initiator,
             CS_ERROR_EVENT_RAS_CLIENT_DATA_RECEPTION_FINISH_FAILED,
             SL_STATUS_NULL_POINTER);
    return;
  }
  cs_initiator_report(CS_INITIATOR_REPORT_LAST_CS_RESULT_BEGIN);
  if (initiator->ras_client.real_time_mode) {
    // Re-enable reception for Real-Time mode
    status = cs_ras_client_real_time_receive(initiator->conn_handle,
                                             sizeof(initiator->data.reflector.ranging_data),
                                             initiator->data.reflector.ranging_data);
    if (status != SL_STATUS_OK) {
      initiator_log_error(INSTANCE_PREFIX "RAS - failed to receive real-time data! [sc: 0x%lx]" LOG_NL,
                          initiator->conn_handle,
                          (unsigned long)status);
      on_error(initiator,
               CS_ERROR_EVENT_RAS_CLIENT_REALTIME_RECEIVE_FAILED,
               status);
      return;
    }

    initiator_log_debug(INSTANCE_PREFIX "RAS - real-time data reception restarted" LOG_NL,
                        initiator->conn_handle);
  }
  if (sc != SL_STATUS_OK) {
    initiator_log_error(INSTANCE_PREFIX "RAS - reception finished - failure! [sc: 0x%lx]" LOG_NL,
                        initiator->conn_handle,
                        (unsigned long)sc);
    if ((lost_segments > 0) && (sc != SL_STATUS_ABORT)) {
      on_error(initiator,
               CS_ERROR_EVENT_RAS_CLIENT_DATA_RECEPTION_FINISH_FAILED,
               sc);
    }
    return;
  }

  initiator_log_debug(INSTANCE_PREFIX "RAS - %s reception finished, "
                                      "lost:%u counter:%u, resp.code:0x%02x, "
                                      "segment: %u -> %u %s, size:%lu, %s, "
                                      "last known segment: %u, lost segments mask: %16llx" LOG_NL,
                      initiator->conn_handle,
                      (real_time ? "real-time" : "on-demand"),
                      retrieve_lost,
                      ranging_counter,
                      response,
                      start_segment,
                      end_segment,
                      (recoverable ? "recoverable" : "non-recoverable"),
                      size,
                      (last_arrived ? "last arrived" : "more to come"),
                      last_known_segment,
                      lost_segments);
  if (real_time) {
    initiator->data.reflector.ranging_data_size = size;
    process_remote_ranging_data(initiator,
                                initiator->data.reflector.ranging_data,
                                initiator->data.reflector.ranging_data_size);
    return;
  }
  (void)status;
  (void)real_time;
  (void)retrieve_lost;
  (void)response;
  (void)ranging_counter;
  (void)start_segment;
  (void)end_segment;
  (void)recoverable;
  (void)last_arrived;
  (void)last_known_segment;
  #if defined (CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE) && (CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE == 0)
  // Received Complete Ranging Data or Complete Lost Ranging Segment Response
  if (response == CS_RAS_CP_RESPONSE_CODE_SUCCESS) {
    if (lost_segments == 0) {
      if (retrieve_lost) {
        initiator_log_info(INSTANCE_PREFIX "RAS - Received Complete Lost Ranging Segment Response" LOG_NL,
                           initiator->conn_handle);
      } else {
        initiator_log_info(INSTANCE_PREFIX "RAS - Received Complete Ranging Data Response" LOG_NL,
                           initiator->conn_handle);
      }
      // Sending ACK
      status = cs_ras_client_ack(initiator->conn_handle,
                                 ranging_counter);
      if (status != SL_STATUS_OK) {
        initiator_log_error(INSTANCE_PREFIX "RAS - failed to send ACK! [sc: 0x%lx]" LOG_NL,
                            initiator->conn_handle,
                            (unsigned long)sc);
        on_error(initiator,
                 CS_ERROR_EVENT_RAS_CLIENT_ACK_FAILED,
                 sc);
        return;
      }
      initiator_log_info(INSTANCE_PREFIX "RAS - ACK was sent!" LOG_NL,
                         initiator->conn_handle);
      initiator->data.reflector.ranging_data_size = size;
      process_remote_ranging_data(initiator,
                                  initiator->data.reflector.ranging_data,
                                  initiator->data.reflector.ranging_data_size);
      return;
    } else {
      if (!retrieve_lost && recoverable && initiator->config.max_procedure_count != 0) {
        // Get start and end segment
        cs_initiator_get_lost_segments(lost_segments,
                                       &start_segment,
                                       &end_segment);
        // Request lost segments
        status = cs_ras_client_retreive_lost_segments(initiator->conn_handle,
                                                      ranging_counter,
                                                      start_segment,
                                                      end_segment,
                                                      sizeof(initiator->data.reflector.ranging_data),
                                                      initiator->data.reflector.ranging_data);
        if (status != SL_STATUS_OK) {
          initiator_log_error(INSTANCE_PREFIX "RAS - failed to request lost segments! [sc: 0x%lx]" LOG_NL,
                              initiator->conn_handle,
                              (unsigned long)sc);
          on_error(initiator,
                   CS_ERROR_EVENT_RAS_CLIENT_REQUEST_LOST_SEGMENTS_FAILED,
                   sc);
          return;
        }
      }
      // Complete Lost Ranging Segment Response returned with lost segments
      // Or not recoverable lost segments arrived
      // sending ACK, no calculation
      initiator_log_error(INSTANCE_PREFIX "RAS - unrecoverable lost segments, sending ACK!" LOG_NL,
                          initiator->conn_handle);
      status = cs_ras_client_ack(initiator->conn_handle,
                                 ranging_counter);
      if (status != SL_STATUS_OK) {
        initiator_log_error(INSTANCE_PREFIX "RAS - failed to send ACK! [sc: 0x%lx]" LOG_NL,
                            initiator->conn_handle,
                            (unsigned long)sc);
        on_error(initiator,
                 CS_ERROR_EVENT_RAS_CLIENT_ACK_FAILED,
                 sc);
        return;
      }
    }
  }
  #endif
}

#if defined (CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE) && (CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE == 0)
void cs_ras_client_on_ack_finished(uint8_t connection, sl_status_t sc, cs_ras_cp_response_code_value_t response)
{
  cs_initiator_t *initiator = cs_initiator_get_instance(connection);
  if (initiator == NULL) {
    initiator_log_error(INSTANCE_PREFIX "RAS - ranging data ready - unknown connection id!" LOG_NL,
                        connection);
    on_error(initiator,
             CS_ERROR_EVENT_RAS_CLIENT_ON_ACK_FINISHED_FAILED,
             SL_STATUS_NULL_POINTER);
    return;
  }
  initiator_log_info(INSTANCE_PREFIX "RAS - ACK finished, [sc: 0x%lx], [response: 0x%lx]" LOG_NL,
                     initiator->conn_handle,
                     (unsigned long)sc,
                     (unsigned long)response);
}

void cs_ras_client_on_ranging_data_ready(uint8_t connection,
                                         cs_ras_ranging_counter_t ranging_counter)
{
  cs_initiator_t *initiator = cs_initiator_get_instance(connection);
  if (initiator == NULL) {
    initiator_log_error(INSTANCE_PREFIX "RAS - ranging data ready - unknown connection id!" LOG_NL,
                        connection);
    on_error(initiator,
             CS_ERROR_EVENT_RAS_CLIENT_RANGING_DATA_READY_FAILED,
             SL_STATUS_NULL_POINTER);
    return;
  }
  initiator_log_info(INSTANCE_PREFIX "RAS - ranging data ready, counter: %u" LOG_NL,
                     initiator->conn_handle,
                     ranging_counter);
  // write GET to RAS CP
  if (((ranging_counter & CS_RAS_RANGING_COUNTER_MASK) == initiator->ranging_counter)
      && (initiator->ras_client.overwritten == false || initiator->ranging_counter != ranging_counter)) {
    sl_status_t sc = cs_ras_client_get_ranging_data(initiator->conn_handle,
                                                    (uint16_t)ranging_counter,
                                                    sizeof(initiator->data.reflector.ranging_data),
                                                    initiator->data.reflector.ranging_data);
    if (sc != SL_STATUS_OK) {
      initiator_log_error(INSTANCE_PREFIX "RAS - failed to get ranging data! [sc: 0x%lx]" LOG_NL,
                          initiator->conn_handle,
                          (unsigned long)sc);
      on_error(initiator,
               CS_ERROR_EVENT_RAS_CLIENT_GET_RANGING_DATA_FAILED,
               sc);
      return;
    }
    initiator_log_info(INSTANCE_PREFIX "RAS - GET ranging data, counter: %u" LOG_NL,
                       initiator->conn_handle,
                       ranging_counter);
  }
}

void cs_ras_client_on_abort_finished(uint8_t connection, sl_status_t sc, cs_ras_cp_response_code_value_t response)
{
  cs_initiator_t *initiator = cs_initiator_get_instance(connection);
  if (initiator == NULL) {
    initiator_log_error(INSTANCE_PREFIX "RAS - abort finished - unknown connection id!" LOG_NL,
                        connection);
    on_error(initiator,
             CS_ERROR_EVENT_RAS_CLIENT_ABORT_FINISHED_FAILED,
             SL_STATUS_NULL_POINTER);
    return;
  }
  initiator_log_info(INSTANCE_PREFIX "RAS - abort finished, [sc: 0x%lx], [response: 0x%lx]" LOG_NL,
                     initiator->conn_handle,
                     (unsigned long)sc,
                     (unsigned long)response);
  initiator->initiator_state = (uint8_t)INITIATOR_STATE_IN_PROCEDURE;
  initiator_log_info(INSTANCE_PREFIX "Instance new state: IN_PROCEDURE" LOG_NL,
                     initiator->conn_handle);
}

void cs_ras_client_on_ranging_data_overwritten(uint8_t connection, cs_ras_ranging_counter_t ranging_counter)
{
  cs_initiator_t *initiator = cs_initiator_get_instance(connection);
  if (initiator == NULL) {
    initiator_log_error(INSTANCE_PREFIX "RAS - ranging data overwritten - unknown connection id!" LOG_NL,
                        connection);
    on_error(initiator,
             CS_ERROR_EVENT_RAS_CLIENT_RANGING_DATA_OVERWRITTEN_FAILED,
             SL_STATUS_NULL_POINTER);
    return;
  }
  initiator->ranging_counter = ranging_counter;
  initiator->ras_client.overwritten = true;
  initiator_log_info(INSTANCE_PREFIX "RAS - ranging data overwritten, counter: %u" LOG_NL,
                     initiator->conn_handle,
                     ranging_counter);
}
#endif // CS_RAS_MODE_ON_DEMAND_RANGING_DATA

bool cs_ras_client_on_timeout(uint8_t connection,
                              cs_ras_client_timeout_t timeout,
                              cs_ras_client_timeout_action_t action)
{
  cs_initiator_t *initiator = cs_initiator_get_instance(connection);
  initiator_log_debug(INSTANCE_PREFIX "RAS timeout: %u, action: %u" LOG_NL,
                      connection,
                      timeout,
                      action);
  (void)timeout;
  (void)action;
  on_error(initiator,
           CS_ERROR_EVENT_RAS_CLIENT_REALTIME_RECEIVE_FAILED,
           SL_STATUS_TIMEOUT);
  // Perform the action automatically
  return false;
}

/******************************************************************************
 * Bluetooth stack event handler.
 *****************************************************************************/
bool cs_initiator_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  cs_initiator_t *initiator;
  state_machine_event_data_t evt_data;
  bool handled = false;
  enum sl_rtl_error_code rtl_err;

  switch (SL_BT_MSG_ID(evt->header)) {
    // --------------------------------
    // Connection parameters set
    case sl_bt_evt_connection_parameters_id:
      initiator = cs_initiator_get_instance(evt->data.evt_connection_parameters.connection);
      if (initiator == NULL) {
        initiator_log_error("Unexpected event "
                            "[sl_bt_evt_connection_parameters_id]!"
                            "Unknown target connection id: %u" LOG_NL,
                            evt->data.evt_connection_parameters.connection);
        break;
      }

      // Make sure that the current connection interval is set on the initiator
      // instance. The interval could be the value from the central at connection
      // initiation or the value set by local or the peer user.
      initiator->conn_interval = evt->data.evt_connection_parameters.interval;

      // Check if the connection parameters are set correctly the first time.
      if (!cs_initiator_check_connection_parameters(initiator, &evt->data.evt_connection_parameters)
          && !initiator->connection_parameters_set) {
        initiator_log_warning(INSTANCE_PREFIX "CS - failed to set connection parameters, retrying once" LOG_NL,
                              initiator->conn_handle);
        sc = sl_bt_connection_set_parameters(initiator->conn_handle,
                                             initiator->config.min_connection_interval,
                                             initiator->config.max_connection_interval,
                                             initiator->config.latency,
                                             initiator->config.timeout,
                                             initiator->config.min_ce_length,
                                             initiator->config.max_ce_length);
        if (sc != SL_STATUS_OK) {
          initiator_log_warning(INSTANCE_PREFIX "CS - failed to set connection parameters again!"
                                                "Proceeding with current values!" LOG_NL,
                                initiator->conn_handle);
          // Set connection parameters request failed again immediately,
          // proceed with the current values.
          initiator->connection_parameters_set = true;
        }
      } else {
        // Connection parameters set, no need to retry.
        initiator->connection_parameters_set = true;
      }

      // Only proceed with the RAS discovery if the connection parameters set.
      if (initiator->connection_parameters_set
          && evt->data.evt_connection_parameters.security_mode
          != sl_bt_connection_mode1_level1) {
        initiator_log_debug(INSTANCE_PREFIX "CS - connection parameters set: "
                                            "encryption on. " LOG_NL,
                            initiator->conn_handle);
        if (initiator->ras_client.state == RAS_STATE_INIT) {
          initiator_log_debug(INSTANCE_PREFIX "Start discovering RAS service "
                                              "& characteristic ..." LOG_NL,
                              initiator->conn_handle);
          sc = sl_bt_gatt_discover_primary_services(initiator->conn_handle);
          if (sc != SL_STATUS_OK && sc != SL_STATUS_IN_PROGRESS) {
            initiator_log_error(INSTANCE_PREFIX "failed to start RAS service discovery!" LOG_NL,
                                initiator->conn_handle);
            on_error(initiator,
                     CS_ERROR_EVENT_START_SERVICE_DISCOVERY,
                     sc);
            break;
          }
          initiator->ras_client.state = RAS_STATE_SERVICE_DISCOVERY;
          init_cs_configuration(initiator->conn_handle);
        }
        handled = true;
      } else {
        // Set connection parameter requested again successfully,
        // awaiting for the event to arrive again...
        initiator->connection_parameters_set = true;
      }
      break;

    case sl_bt_evt_connection_phy_status_id:
      initiator = cs_initiator_get_instance(evt->data.evt_connection_phy_status.connection);
      if (initiator == NULL) {
        break;
      }
      initiator->config.conn_phy = evt->data.evt_connection_phy_status.phy;
      initiator_log_debug(INSTANCE_PREFIX "Connection phy set to: %u" LOG_NL,
                          initiator->conn_handle,
                          initiator->config.conn_phy);
      break;

    // --------------------------------
    // CS Security
    case sl_bt_evt_cs_security_enable_complete_id:
      initiator = cs_initiator_get_instance(evt->data.evt_cs_security_enable_complete.connection);
      if (initiator == NULL) {
        break;
      }
      initiator->cs_security_enabled = true;
      initiator_log_debug(INSTANCE_PREFIX "CS security elevated" LOG_NL,
                          initiator->conn_handle);
      handled = true;
      break;

    // --------------------------------
    // GATT procedure completed
    case sl_bt_evt_gatt_procedure_completed_id:
      initiator = cs_initiator_get_instance(evt->data.evt_gatt_procedure_completed.connection);
      if (initiator == NULL) {
        initiator_log_error("Unexpected event "
                            "[sl_bt_evt_gatt_procedure_completed_id]!"
                            "Unknown target connection id: %u" LOG_NL,
                            evt->data.evt_connection_parameters.connection);
        break;
      }
      handled = ras_client_handler(initiator, evt);
      break;

    // --------------------------------
    // New GATT characteristic discovered
    case sl_bt_evt_gatt_characteristic_id:
      initiator = cs_initiator_get_instance(evt->data.evt_gatt_characteristic.connection);
      if (initiator == NULL) {
        initiator_log_error("Unexpected event "
                            "[sl_bt_evt_gatt_characteristic_id]!"
                            "Unknown target connection id: %u" LOG_NL,
                            evt->data.evt_gatt_service.connection);
        break;
      }

      for (int i = 0; i < CS_RAS_CHARACTERISTIC_INDEX_COUNT; i++) {
        if (memcmp(&char_uuids[i], evt->data.evt_gatt_characteristic.uuid.data, sizeof(char_uuids[i])) == 0) {
          initiator->ras_client.gattdb_handles.array[i] = evt->data.evt_gatt_characteristic.characteristic;
          initiator_log_debug(INSTANCE_PREFIX "RAS - found %u. characteristic: [0x%x]" LOG_NL,
                              initiator->conn_handle,
                              i,
                              initiator->ras_client.gattdb_handles.array[i]);
        }
      }
      break;

    // --------------------------------
    // New GATT service discovered
    case sl_bt_evt_gatt_service_id:
      initiator = cs_initiator_get_instance(evt->data.evt_gatt_service.connection);
      if (initiator == NULL) {
        initiator_log_error("Unexpected event [sl_bt_evt_gatt_service_id]!"
                            "Unknown target connection id: %u" LOG_NL,
                            evt->data.evt_gatt_service.connection);
        break;
      }
      if (memcmp(&service_uuid, evt->data.evt_gatt_service.uuid.data, sizeof(service_uuid)) == 0) {
        initiator->ras_client.service = evt->data.evt_gatt_service.service;
        initiator_log_debug(INSTANCE_PREFIX "RAS - found service: %lu" LOG_NL,
                            initiator->conn_handle,
                            initiator->ras_client.service);
        start_error_timer(initiator);
      }
      break;

    // --------------------------------
    // CS procedure enable action completed
    case sl_bt_evt_cs_procedure_enable_complete_id:
      initiator = cs_initiator_get_instance(evt->data.evt_cs_procedure_enable_complete.connection);
      if (initiator == NULL) {
        initiator_log_error("Unexpected event "
                            "[sl_bt_evt_cs_procedure_enable_complete_id]!"
                            "Unknown target connection id: %u" LOG_NL,
                            evt->data.evt_cs_procedure_enable_complete.connection);
        break;
      }

      handled = true;
      evt_data.evt_procedure_enable_completed = &evt->data.evt_cs_procedure_enable_complete;
      if (initiator->config.cs_main_mode == sl_bt_cs_mode_pbr) {
        initiator->antenna_config = evt->data.evt_cs_procedure_enable_complete.antenna_config;
      }

      // procedure enable/disable received
      (void)initiator_state_machine_event_handler(initiator,
                                                  INITIATOR_EVT_PROCEDURE_ENABLE_COMPLETED,
                                                  &evt_data);
      break;

    // --------------------------------
    // CS configuration completed
    case sl_bt_evt_cs_config_complete_id:
      initiator = cs_initiator_get_instance(evt->data.evt_cs_config_complete.connection);
      if (initiator == NULL) {
        initiator_log_error("Unexpected event "
                            "[sl_bt_evt_cs_config_complete_id]!"
                            "Unknown target connection id: %u" LOG_NL,
                            evt->data.evt_cs_config_complete.connection);
        break;
      } else {
        initiator_log_debug(INSTANCE_PREFIX "CS - configuration completed. "
                                            "Set CS procedure parameters ..." LOG_NL,
                            initiator->conn_handle);

        stop_error_timer(initiator);

        initiator->cs_parameters.num_calib_steps
          = evt->data.evt_cs_config_complete.mode_calibration_steps;
        initiator->cs_parameters.T_PM_time
          = evt->data.evt_cs_config_complete.t_pm_time;
        initiator->cs_parameters.T_IP1_time
          = evt->data.evt_cs_config_complete.t_ip1_time;
        initiator->cs_parameters.T_IP2_time
          = evt->data.evt_cs_config_complete.t_ip2_time;
        initiator->cs_parameters.T_FCS_time
          = evt->data.evt_cs_config_complete.t_fcs_time;
        initiator->cs_parameters.connection_interval
          = initiator->conn_interval;
        initiator->cs_parameters.ch3c_jump
          = evt->data.evt_cs_config_complete.ch3c_jump;
        initiator->cs_parameters.ch3c_shape
          = evt->data.evt_cs_config_complete.ch3c_shape;
        initiator->cs_parameters.channel_map_repetition
          = evt->data.evt_cs_config_complete.channel_map_repetition;
        initiator->cs_parameters.channel_selection_type
          = evt->data.evt_cs_config_complete.channel_selection_type;
        initiator->cs_parameters.cs_sync_phy
          = evt->data.evt_cs_config_complete.cs_sync_phy;
        initiator->cs_parameters.rtt_type
          = evt->data.evt_cs_config_complete.rtt_type;
        initiator->cs_parameters.main_mode_repetition
          = evt->data.evt_cs_config_complete.main_mode_repetition;
        initiator->cs_parameters.max_main_mode_steps
          = evt->data.evt_cs_config_complete.max_main_mode_steps;
        initiator->cs_parameters.min_main_mode_steps
          = evt->data.evt_cs_config_complete.min_main_mode_steps;
        memcpy(&initiator->cs_parameters.channel_map[0],
               &evt->data.evt_cs_config_complete.channel_map.data[0],
               sizeof(initiator->cs_parameters.channel_map));

        // Create estimator with the set CS configuration parameters
        initiator_log_debug(INSTANCE_PREFIX "CS - procedure parameters set,"
                                            "RTL - initialize lib item" LOG_NL,
                            initiator->conn_handle);
        rtl_err = rtl_library_create_estimator(initiator->conn_handle,
                                               &initiator->rtl_handle,
                                               &initiator->rtl_config,
                                               &initiator->cs_parameters,
                                               initiator->config.cs_main_mode,
                                               initiator->config.cs_sub_mode);
        if (rtl_err != SL_RTL_ERROR_SUCCESS) {
          initiator_log_error(INSTANCE_PREFIX "RTL - failed to init lib item! [E: 0x%x]" LOG_NL,
                              initiator->conn_handle,
                              rtl_err);
          on_error(initiator,
                   CS_ERROR_EVENT_INITIATOR_FAILED_TO_INIT_RTL_LIB,
                   rtl_err);
        }
        initiator_log_info(INSTANCE_PREFIX "RTL - lib item initialized." LOG_NL,
                           initiator->conn_handle);

        sc = sl_bt_cs_set_procedure_parameters(initiator->conn_handle,
                                               initiator->config.config_id,
                                               initiator->config.max_procedure_duration,
                                               initiator->config.min_procedure_interval,
                                               initiator->config.max_procedure_interval,
                                               initiator->config.max_procedure_count,
                                               initiator->config.min_subevent_len,
                                               initiator->config.max_subevent_len,
                                               initiator->config.cs_tone_antenna_config_idx,
                                               initiator->config.conn_phy,
                                               initiator->config.tx_pwr_delta,
                                               initiator->config.preferred_peer_antenna,
                                               initiator->config.snr_control_initiator,
                                               initiator->config.snr_control_reflector);

        if (sc != SL_STATUS_OK) {
          initiator_log_error(INSTANCE_PREFIX "CS procedure - failed to set parameters! "
                                              "[sc: 0x%lx]" LOG_NL,
                              initiator->conn_handle,
                              (unsigned long)sc);
          on_error(initiator,
                   CS_ERROR_EVENT_CS_SET_PROCEDURE_PARAMETERS_FAILED,
                   sc);
          break;
        }
      }
      handled = true;
      break;

    // --------------------------------
    // CS result (initiator) arrived
    case sl_bt_evt_cs_result_id:
      initiator = cs_initiator_get_instance(evt->data.evt_cs_result.connection);
      if (initiator == NULL) {
        break;
      }
      handled = true;
      #ifdef SL_CATALOG_BLUETOOTH_FEATURE_CS_TEST_PRESENT
      handled = false;
      #endif //SL_CATALOG_BLUETOOTH_FEATURE_CS_TEST_PRESENT
      initiator_log_info(INSTANCE_PREFIX "CS - received first initiator CS result" LOG_NL,
                         evt->data.evt_cs_result.connection);
      if (initiator->ranging_counter == CS_RAS_INVALID_RANGING_COUNTER) {
        if (initiator->initiator_state == INITIATOR_STATE_WAIT_REFLECTOR_PROCEDURE_COMPLETE
            || initiator->initiator_state == INITIATOR_STATE_WAIT_REFLECTOR_PROCEDURE_ABORTED) {
          initiator->initiator_state = (uint8_t)INITIATOR_STATE_IN_PROCEDURE;
          initiator_log_info(INSTANCE_PREFIX "Instance new state: IN_PROCEDURE" LOG_NL,
                             initiator->conn_handle);
        }
      }
      if (initiator->initiator_state == INITIATOR_STATE_WAIT_REFLECTOR_PROCEDURE_COMPLETE
          || initiator->initiator_state == INITIATOR_STATE_WAIT_REFLECTOR_PROCEDURE_ABORTED) {
        initiator->drop_counter++;
        if (initiator->drop_counter > CS_INITIATOR_MAX_DROP) {
          initiator->drop_counter = 0;
          initiator->ranging_counter = CS_RAS_INVALID_RANGING_COUNTER;
          reset_subevent_data(initiator, false);
          initiator->initiator_state = (uint8_t)INITIATOR_STATE_IN_PROCEDURE;
          initiator_log_info(INSTANCE_PREFIX "Instance new state: IN_PROCEDURE" LOG_NL,
                             initiator->conn_handle);
        } else {
          initiator_log_info(INSTANCE_PREFIX "CS - ongoing measurement, drop new result" LOG_NL,
                             evt->data.evt_cs_result.connection);
          break;
        }
      }
      evt_data.evt_cs_result.cs_event = evt;
      evt_data.evt_cs_result.first_cs_result = true;
      (void)initiator_state_machine_event_handler(initiator,
                                                  INITIATOR_EVT_CS_RESULT,
                                                  &evt_data);
      break;

    // --------------------------------
    // Consecutive CS result (initiator) arrived
    case sl_bt_evt_cs_result_continue_id:
      initiator = cs_initiator_get_instance(evt->data.evt_cs_result_continue.connection);
      if (initiator == NULL) {
        break;
      }
      handled = true;
      if (initiator->initiator_state != INITIATOR_STATE_WAIT_REFLECTOR_PROCEDURE_COMPLETE
          && initiator->initiator_state != INITIATOR_STATE_WAIT_REFLECTOR_PROCEDURE_ABORTED) {
        initiator_log_info(INSTANCE_PREFIX "CS - received initiator CS result" LOG_NL,
                           evt->data.evt_cs_result_continue.connection);
        evt_data.evt_cs_result.cs_event = evt;
        evt_data.evt_cs_result.first_cs_result = false;
        (void)initiator_state_machine_event_handler(initiator,
                                                    INITIATOR_EVT_CS_RESULT_CONTINUE,
                                                    &evt_data);
      } else {
        initiator_log_info(INSTANCE_PREFIX "CS - ongoing measurement, drop new result continue" LOG_NL,
                           evt->data.evt_cs_result.connection);
      }
      #ifdef SL_CATALOG_BLUETOOTH_FEATURE_CS_TEST_PRESENT
      handled = false;
      #endif //SL_CATALOG_BLUETOOTH_FEATURE_CS_TEST_PRESENT
      break;

    // --------------------------------
    // Bluetooth stack resource exhausted
    case sl_bt_evt_system_resource_exhausted_id:
    {
      initiator_log_error("BT stack buffers exhausted, "
                          "data loss may have occurred! [buf_discarded:%u, "
                          "buf_alloc_fail:%u, heap_alloc_fail:%u]" LOG_NL,
                          evt->data.evt_system_resource_exhausted.num_buffers_discarded,
                          evt->data.evt_system_resource_exhausted.num_buffer_allocation_failures,
                          evt->data.evt_system_resource_exhausted.num_heap_allocation_failures);
    }
    break;

    case sl_bt_evt_scanner_legacy_advertisement_report_id:
      //  Avoid spamming log messages for advertisement reports
      break;

    default:
      initiator_log_debug("unhandled BLE event: 0x%lx" LOG_NL,
                          (unsigned long)SL_BT_MSG_ID(evt->header));
      break;
  }

  // Return false if the event was handled above.
  return !handled;
}
