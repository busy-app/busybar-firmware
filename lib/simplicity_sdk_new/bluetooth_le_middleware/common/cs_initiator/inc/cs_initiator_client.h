/***************************************************************************//**
 * @file
 * @brief CS initiator - client header
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

#ifndef CS_INITIATOR_CLIENT_H
#define CS_INITIATOR_CLIENT_H

// -----------------------------------------------------------------------------
// Includes

#include <stdbool.h>
#include <stdint.h>
#include "sl_enum.h"
#include "sl_common.h"
#include "sl_bt_api.h"
#include "cs_sync_antenna.h"

#include "cs_initiator_config.h"
// -----------------------------------------------------------------------------
// Macros

// The number of times the channel_map field will
// be cycled through for non-Mode 0 steps within a CS procedure.
// Default: 1 Range: 1-3
#define CS_INITIATOR_DEFAULT_CHANNEL_MAP_REPETITION   1

// Main mode steps are ignored if submode is disabled.
// Default: 2 Range: 2-160
#define CS_INITIATOR_DEFAULT_MIN_MAIN_MODE_STEPS      2

// Default: 2 Range: 2-160
#define CS_INITIATOR_DEFAULT_MAX_MAIN_MODE_STEPS      2

// The number of main mode steps taken from the end of the last CS subevent
// to be repeated at the beginning of the current CS subevent directly after
// the last Mode 0 step of that event.
// Default: 0 Range: 0-3
#define CS_INITIATOR_DEFAULT_MAIN_MODE_REPETITION      0

// Added for backward compatibility
#ifndef CS_INITIATOR_DEFAULT_CS_MAIN_MODE
#define CS_INITIATOR_DEFAULT_CS_MAIN_MODE CS_INITIATOR_DEFAULT_CS_MODE
#endif

#define CS_INITIATOR_DEFAULT_CHANNEL_MAP \
  { 0xFC, 0xFF, 0x7F, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F }

#define INITIATOR_CONFIG_DEFAULT                                                           \
  {                                                                                        \
    .procedure_scheduling =           CS_INITIATOR_DEFAULT_PROCEDURE_SCHEDULING,           \
    .cs_main_mode =                   CS_INITIATOR_DEFAULT_CS_MAIN_MODE,                   \
    .cs_sub_mode =                    CS_INITIATOR_DEFAULT_CS_SUB_MODE,                    \
    .min_subevent_len =               CS_INITIATOR_DEFAULT_MIN_SUBEVENT_LEN,               \
    .max_subevent_len =               CS_INITIATOR_DEFAULT_MAX_SUBEVENT_LEN,               \
    .min_procedure_interval =         CS_INITIATOR_DEFAULT_MIN_PROCEDURE_INTERVAL,         \
    .max_procedure_interval =         CS_INITIATOR_DEFAULT_MAX_PROCEDURE_INTERVAL,         \
    .min_connection_interval =        CS_INITIATOR_DEFAULT_MIN_CONNECTION_INTERVAL,        \
    .max_connection_interval =        CS_INITIATOR_DEFAULT_MAX_CONNECTION_INTERVAL,        \
    .max_procedure_count =            CS_INITIATOR_DEFAULT_MAX_PROCEDURE_COUNT,            \
    .conn_phy =                       CS_INITIATOR_DEFAULT_CONN_PHY,                       \
    .cs_sync_phy =                    CS_INITIATOR_DEFAULT_CS_SYNC_PHY,                    \
    .config_id =                      CS_INITIATOR_DEFAULT_CONFIG_ID,                      \
    .min_main_mode_steps =            CS_INITIATOR_DEFAULT_MIN_MAIN_MODE_STEPS,            \
    .max_main_mode_steps =            CS_INITIATOR_DEFAULT_MAX_MAIN_MODE_STEPS,            \
    .main_mode_repetition =           CS_INITIATOR_DEFAULT_MAIN_MODE_REPETITION,           \
    .mode0_step =                     CS_INITIATOR_DEFAULT_MODE0_STEPS,                    \
    .channel_map_repetition =         CS_INITIATOR_DEFAULT_CHANNEL_MAP_REPETITION,         \
    .ch3c_jump =                      CS_INITIATOR_DEFAULT_CH3C_JUMP,                      \
    .max_procedure_duration =         CS_INITIATOR_DEFAULT_MAX_PROCEDURE_DURATION,         \
    .tx_pwr_delta =                   CS_INITIATOR_DEFAULT_TX_PWR_DELTA,                   \
    .num_antennas =                   0,                                                   \
    .cs_tone_antenna_config_idx_req = CS_INITIATOR_DEFAULT_CS_TONE_ANTENNA_CONFIG_IDX_REQ, \
    .cs_tone_antenna_config_idx =     CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY,                   \
    .cs_sync_antenna_req =            CS_INITIATOR_DEFAULT_CS_SYNC_ANTENNA_REQ,            \
    .preferred_peer_antenna =         CS_INITIATOR_DEFAULT_PREFERRED_PEER_ANTENNA,         \
    .max_tx_power_dbm =               CS_INITIATOR_DEFAULT_MAX_TX_POWER,                   \
    .create_context =                 CS_INITIATOR_DEFAULT_CREATE_CONTEXT,                 \
    .rssi_ref_tx_power =              CS_INITIATOR_DEFAULT_RSSI_REF_TX_POWER,              \
    .latency =                        CS_INITIATOR_DEFAULT_CONNECTION_PERIPHERAL_LATENCY,  \
    .timeout =                        CS_INITIATOR_DEFAULT_TIMEOUT,                        \
    .mtu =                            23,                                                  \
    .min_ce_length =                  CS_INITIATOR_DEFAULT_MIN_CE_LENGTH,                  \
    .max_ce_length =                  CS_INITIATOR_DEFAULT_MAX_CE_LENGTH,                  \
    .cs_sync_antenna =                CS_SYNC_SWITCHING,                                   \
    .rtt_type =                       CS_INITIATOR_DEFAULT_RTT_TYPE,                       \
    .channel_selection_type =         CS_INITIATOR_DEFAULT_CHANNEL_SELECTION_TYPE,         \
    .ch3c_shape =                     CS_INITIATOR_DEFAULT_CH3C_SHAPE,                     \
    .reserved =                       0,                                                   \
    .snr_control_initiator =          sl_bt_cs_snr_control_adjustment_not_applied,         \
    .snr_control_reflector =          sl_bt_cs_snr_control_adjustment_not_applied,         \
    .use_real_time_ras_mode =         CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE,            \
    .channel_map.data =               CS_INITIATOR_DEFAULT_CHANNEL_MAP,                    \
    .channel_map_preset =             CS_INITIATOR_DEFAULT_CHANNEL_MAP_PRESET              \
  }

#define RTL_CONFIG_DEFAULT                       \
  {                                              \
    .algo_mode = CS_INITIATOR_DEFAULT_ALGO_MODE, \
    .rtl_logging_enabled = CS_INITIATOR_RTL_LOG, \
  }

// -----------------------------------------------------------------------------
// Enums, structs, typedefs

// CS channel map preset
SL_ENUM(cs_channel_map_preset_t) {
  CS_CHANNEL_MAP_PRESET_LOW,
  CS_CHANNEL_MAP_PRESET_MEDIUM,
  CS_CHANNEL_MAP_PRESET_HIGH,
  CS_CHANNEL_MAP_PRESET_CUSTOM
};

// CS antenna configuration index
SL_ENUM(cs_tone_antenna_config_index_t) {
  CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY = 0,
  CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R = 1,
  CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R = 4,
  CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY = 7
};


// Parameters optimization for energy or frequency
// Has an effect on procedure and connection intervals
SL_ENUM(cs_procedure_scheduling_t) {
  CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY = 0,
  CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY,
  CS_PROCEDURE_SCHEDULING_CUSTOM
};

SL_ENUM(cs_error_event_t) {
  CS_ERROR_EVENT_UNHANDLED,
  CS_ERROR_EVENT_TIMER_HANDLE_NULL_REFERENCE,
  CS_ERROR_EVENT_TIMER_START_ERROR,
  CS_ERROR_EVENT_TIMER_ELAPSED,
  CS_ERROR_EVENT_TIMER_STOP_ERROR,
  CS_ERROR_EVENT_START_SERVICE_DISCOVERY,
  CS_ERROR_EVENT_RAS_SERVICE_DISCOVERY_FAILED,
  CS_ERROR_EVENT_GATT_PROCEDURE_FAILED,
  CS_ERROR_EVENT_START_CHARACTERISTIC_DISCOVERY_FAILED,
  CS_ERROR_EVENT_RAS_REAL_TIME_RANGING_DATA_CHARACTERISTIC_NOT_FOUND,
  CS_ERROR_EVENT_RAS_CLIENT_CREATE_FAILED,
  CS_ERROR_EVENT_RAS_CLIENT_INIT_FEATURE_NOT_SUPPORTED,
  CS_ERROR_EVENT_RAS_CLIENT_INIT_FAILED,
  CS_ERROR_EVENT_RAS_CLIENT_CONFIG_FAILED,
  CS_ERROR_EVENT_RAS_CLIENT_MODE_CHANGE_FAILED,
  CS_ERROR_EVENT_RAS_CLIENT_REALTIME_RECEIVE_FAILED,
  CS_ERROR_EVENT_RAS_CLIENT_SCOOP_DATA_OUT_OF_BOUNDS,
  CS_ERROR_EVENT_RAS_CLIENT_DATA_RECEPTION_FINISH_FAILED,
  CS_ERROR_EVENT_RAS_CLIENT_RANGING_DATA_READY_FAILED,
  CS_ERROR_EVENT_RAS_CLIENT_GET_RANGING_DATA_FAILED,
  CS_ERROR_EVENT_RAS_CLIENT_RANGING_DATA_OVERWRITTEN_FAILED,
  CS_ERROR_EVENT_RAS_CLIENT_ABORT_FINISHED_FAILED,
  CS_ERROR_EVENT_RAS_CLIENT_ACK_FAILED,
  CS_ERROR_EVENT_RAS_CLIENT_REQUEST_LOST_SEGMENTS_FAILED,
  CS_ERROR_EVENT_RAS_CLIENT_ON_ACK_FINISHED_FAILED,
  CS_ERROR_EVENT_SEND_NOTIFICATION_FAILED,
  CS_ERROR_EVENT_SEND_INDICATION_FAILED,
  CS_ERROR_EVENT_SEND_CONTROL_POINT_RESPONSE_FAILED,
  CS_ERROR_EVENT_WRITE_CHARACTERISTIC_FAILED,
  CS_ERROR_EVENT_SEND_CHARACTERISTIC_CONFIRMATION_FAILED,
  CS_ERROR_EVENT_CS_SET_PROCEDURE_PARAMETERS_FAILED,
  CS_ERROR_EVENT_CS_PROCEDURE_ENABLE_FAILED,
  CS_ERROR_EVENT_CS_PROCEDURE_START_FAILED,
  CS_ERROR_EVENT_CS_PROCEDURE_START_TIMER_FAILED,
  CS_ERROR_EVENT_CS_PROCEDURE_STOP_TIMER_FAILED,
  CS_ERROR_EVENT_CS_PROCEDURE_STOP_FAILED,
  CS_ERROR_EVENT_CS_PROCEDURE_UNEXPECTED_DATA,
  CS_ERROR_EVENT_CS_PROCEDURE_COMPLETE_FAILED,
  CS_ERROR_EVENT_CS_PROCEDURE_COUNTER_MISMATCH,
  CS_ERROR_EVENT_INITIATOR_INSTANCE_NULL,
  CS_ERROR_EVENT_INITIATOR_FAILED_TO_SET_DEFAULT_CS_SETTINGS,
  CS_ERROR_EVENT_INITIATOR_PBR_ANTENNA_USAGE_NOT_SUPPORTED,
  CS_ERROR_EVENT_INITIATOR_RTT_ANTENNA_USAGE_NOT_SUPPORTED,
  CS_ERROR_EVENT_INITIATOR_FAILED_TO_CREATE_CONFIG,
  CS_ERROR_EVENT_INITIATOR_FAILED_TO_ENABLE_CS_SECURITY,
  CS_ERROR_EVENT_INITIATOR_FAILED_TO_GET_SECURITY_STATUS,
  CS_ERROR_EVENT_INITIATOR_FAILED_TO_INCREASE_SECURITY,
  CS_ERROR_EVENT_INITIATOR_FAILED_TO_INIT_RTL_LIB,
  CS_ERROR_EVENT_INITIATOR_FAILED_TO_GET_CHANNEL_MAP,
  CS_ERROR_EVENT_INITIATOR_FAILED_TO_SET_CONNECTION_PARAMETERS,
  CS_ERROR_EVENT_INITIATOR_FAILED_TO_SET_INTERVALS,
  CS_ERROR_EVENT_INITIATOR_FAILED_TO_SET_CONNECTION_PHY,
  CS_ERROR_EVENT_FILE_LOGGER_INIT_FAILED,
  CS_ERROR_EVENT_STATE_MACHINE_FAILED,
  CS_ERROR_EVENT_INIT_FAILED,
  CS_ERROR_EVENT_INITIATOR_FAILED_TO_DELETE_INSTANCE,
  CS_ERROR_EVENT_INITIATOR_FAILED_TO_FINALIZE_CLEANUP,
  CS_ERROR_EVENT_RTL_ERROR,
  CS_ERROR_EVENT_RTL_PROCESS_ERROR
};

SL_PACK_START(1)
typedef struct {
  uint8_t procedure_scheduling;
  uint8_t conn_phy;
  uint8_t cs_sync_phy;
  uint8_t cs_main_mode;
  uint8_t cs_sub_mode;
  uint8_t max_main_mode_steps;
  uint8_t min_main_mode_steps;
  uint8_t main_mode_repetition;
  uint8_t mode0_step;
  uint8_t channel_map_repetition;
  uint8_t ch3c_jump;
  uint8_t ch3c_shape;
  uint8_t num_antennas;
  uint8_t cs_tone_antenna_config_idx_req;
  uint8_t cs_tone_antenna_config_idx;
  uint8_t rtt_type;
  uint8_t channel_selection_type;
  uint8_t cs_sync_antenna_req;
  uint8_t cs_sync_antenna;
  uint8_t reserved;
  uint8_t config_id;
  uint8_t preferred_peer_antenna;
  uint8_t create_context;
  int8_t tx_pwr_delta;
  int8_t max_tx_power_dbm;
  float rssi_ref_tx_power;
  uint32_t min_subevent_len;
  uint32_t max_subevent_len;
  uint16_t min_connection_interval;
  uint16_t max_connection_interval;
  uint16_t min_procedure_interval;
  uint16_t max_procedure_interval;
  uint16_t max_procedure_duration;
  uint16_t max_procedure_count;
  uint16_t latency;
  uint16_t timeout;
  uint16_t mtu;
  uint16_t min_ce_length;
  uint16_t max_ce_length;
  uint8_t snr_control_initiator;
  uint8_t snr_control_reflector;
  uint8_t use_real_time_ras_mode;
  uint8_t channel_map_preset;
  sl_bt_cs_channel_map_t channel_map;
} SL_ATTRIBUTE_PACKED cs_initiator_config_t;
SL_PACK_END()

SL_PACK_START(1)
typedef struct {
  uint8_t algo_mode;
  bool rtl_logging_enabled;
} SL_ATTRIBUTE_PACKED rtl_config_t;
SL_PACK_END()

// -----------------------------------------------------------------------------
// Function declarations

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * Set channel map based on the preset.
 * @param[in] preset Channel map preset.
 * @param[out] channel_map Channel map to be filled.
 *****************************************************************************/
void cs_initiator_apply_channel_map_preset(cs_channel_map_preset_t preset, uint8_t *channel_map);

/**************************************************************************//**
 * Get the connection and procedure intervals
 * @param[in] main_mode CS main mode.
 * @param[in] sub_mode CS sub mode.
 * @param[in] procedure_scheduling Procedure scheduling.
 * @param[in] channel_map_preset Channel map preset.
 * @param[in] algo_mode Algorithm mode.
 * @param[in] antenna_path Antenna path.
 * @param[in] use_real_time_ras_mode Use real-time RAS mode.
 * @param[out] conn_interval Connection interval.
 * @param[out] proc_interval CS procedure interval.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_initiator_get_intervals(uint8_t main_mode,
                                       uint8_t sub_mode,
                                       cs_procedure_scheduling_t procedure_scheduling,
                                       uint8_t channel_map_preset,
                                       uint8_t algo_mode,
                                       uint8_t antenna_path,
                                       uint8_t use_real_time_ras_mode,
                                       uint16_t *conn_interval,
                                       uint16_t *proc_interval);

#ifdef __cplusplus
};
#endif

#endif // CS_INITIATOR_CLIENT_H
