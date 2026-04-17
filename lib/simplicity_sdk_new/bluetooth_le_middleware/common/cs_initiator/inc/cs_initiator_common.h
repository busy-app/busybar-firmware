/***************************************************************************//**
 * @file
 * @brief CS initiator - common header
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
#ifndef CS_INITIATOR_COMMON_H
#define CS_INITIATOR_COMMON_H

// -----------------------------------------------------------------------------
// Includes

#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"
#include "sl_bt_api.h"
#include "sl_rtl_clib_api.h"
#include "app_timer.h"

#include "cs_result.h"
#include "cs_result_config.h"
#include "cs_initiator.h"
#include "cs_initiator_client.h"
#include "cs_initiator_config.h"
#include "cs_ras_client.h"

#ifdef __cplusplus
extern "C"
{
#endif

// -----------------------------------------------------------------------------
// Definitions

// Maximum number of subevents per procedure
#define CS_INITIATOR_MAX_SUBEVENTS_PER_PROCEDURE 32

// CS initiator state machine

/// CS initiator state machine event enumerator
typedef enum {
  INITIATOR_EVT_INIT_STARTED = 0U,
  INITIATOR_EVT_INIT_COMPLETED,
  INITIATOR_EVT_START_PROCEDURE,
  INITIATOR_EVT_PROCEDURE_ENABLE_STARTING,
  INITIATOR_EVT_PROCEDURE_ENABLE_COMPLETED,
  INITIATOR_EVT_PROCEDURE_DISABLE_COMPLETED,
  INITIATOR_EVT_CS_RESULT,
  INITIATOR_EVT_CS_RESULT_CONTINUE,
  INITIATOR_EVT_RANGING_DATA,
  INITIATOR_EVT_DELETE_INSTANCE,
  INITIATOR_EVT_ERROR
} state_machine_event_t;

/// CS initiator state machine states enumerator
typedef enum {
  INITIATOR_STATE_UNINITIALIZED = 0,
  INITIATOR_STATE_INIT,
  INITIATOR_STATE_START_PROCEDURE,
  INITIATOR_STATE_WAIT_PROCEDURE_ENABLE_COMPLETE,
  INITIATOR_STATE_WAIT_PROCEDURE_DISABLE_COMPLETE,
  INITIATOR_STATE_IN_PROCEDURE,
  INITIATOR_STATE_WAIT_REFLECTOR_PROCEDURE_COMPLETE,
  INITIATOR_STATE_WAIT_REFLECTOR_PROCEDURE_ABORTED,
  INITIATOR_STATE_DELETE,
  INITIATOR_STATE_ERROR
} initiator_state_t;

/// CS result event union
typedef union {
  sl_bt_evt_cs_result_t cs_result_raw;
  sl_bt_evt_cs_result_continue_t cs_result_continue_raw;
} cs_result_event_t;

/// CS result data structure
typedef struct {
  sl_bt_msg_t *cs_event;
  uint8_t procedure_done_status;
  uint8_t subevent_done_status;
  uint8_t num_steps;
  bool first_cs_result;
} cs_result_data_t;

/// CS procedure triggering
typedef enum {
  CS_PROCEDURE_STATE_IN_PROGRESS = 0u,
  CS_PROCEDURE_STATE_ABORTED,
  CS_PROCEDURE_STATE_COMPLETED
} cs_procedure_state_t;

/// Ranging data event data type
typedef struct {
  uint8_t *data;
  uint32_t data_size;
  uint16_t ranging_counter;
  cs_procedure_state_t procedure_state;
  bool initiator_part;
} ranging_data_t;

/// CS initiator state machine event data
typedef union {
  struct {
    cs_error_event_t error_type;
    sl_status_t sc;
  } evt_error;
  cs_result_data_t evt_cs_result;
  ranging_data_t evt_ranging_data;
  bool evt_init_completed;
  bool evt_procedure_enable_starting;
  sl_bt_evt_cs_procedure_enable_complete_t *evt_procedure_enable_completed;
} state_machine_event_data_t;

// CS procedure

/// CS procedure triggering
typedef enum {
  CS_PROCEDURE_ACTION_CONTINUE = 0u,
  CS_PROCEDURE_ACTION_TRIGGER_RESET,
  CS_PROCEDURE_ACTION_TRIGGER_START,
  CS_PROCEDURE_ACTION_TRIGGER_STOP
} cs_procedure_action_t;

// RAS

/// RAS client state machine
typedef enum {
  RAS_STATE_INIT = 0,
  RAS_STATE_SERVICE_DISCOVERY,
  RAS_STATE_CHARACTERISTIC_DISCOVERY,
  RAS_STATE_CLIENT_INIT,
  RAS_STATE_SET_MODE_REAL_TIME,
  RAS_STATE_MODE_REAL_TIME,
  RAS_STATE_SET_MODE_ON_DEMAND,
  RAS_STATE_MODE_ON_DEMAND,
  RAS_STATE_MODE_ON_DEMAND_RETRIEVE_LOST,
  RAS_STATE_MODE_ON_DEMAND_ACK,
  RAS_STATE_MODE_ON_DEMAND_GET,
  RAS_STATE_MODE_ON_DEMAND_ABORT,
  RAS_STATE_MODE_REAL_TIME_REENABLE
} ras_state_t;

/// RAS client properties
typedef struct {
  cs_ras_client_config_t config;          // Defines wether notification and/or
                                          // indication is used for the
                                          // different modes
  bool real_time_mode;                    // Real-time or on-demand mode
  uint32_t service;                       // Ranging service handle
  cs_ras_gattdb_handles_t gattdb_handles; // Ranging characteristic handles
  ras_state_t state;                      // State of the RAS client
  uint16_t mtu;                           // MTU setup of the connection
  bool overwritten;                       // true if the data of the given
                                          // ranging counter is overwritten
} ras_client_t;

/// Ranging data array type
typedef struct {
  uint32_t ranging_data_size;                               // Actual size
  uint8_t ranging_data[CS_INITIATOR_MAX_RANGING_DATA_SIZE]; // Ranging data
} ranging_data_array_t;

/// Unified ranging data
typedef struct {
  uint8_t num_steps;                        // Number of steps
  uint8_t step_channels[CS_MAX_STEP_COUNT]; // Step channel array
  ranging_data_array_t initiator;           // Initiator ranging data
  ranging_data_array_t reflector;           // Reflector ranging data
} unified_ranging_data_t;

/// CS Initiator main class
typedef struct {
  unified_ranging_data_t data;
  uint16_t ranging_counter;
  cs_ras_subevent_header_t *last_subevent_header;
  sl_rtl_cs_procedure_config cs_procedure_config;
  rtl_config_t rtl_config;
  sl_rtl_cs_params cs_parameters;
  cs_result_session_data_t result_data;
  uint8_t result[CS_RESULT_MAX_BUFFER_SIZE];
  cs_initiator_config_t config;
  ras_client_t ras_client;
  uint8_t conn_handle;
  uint16_t conn_interval;
  sl_bt_connection_security_t security_mode;
  bool cs_security_enabled;
  bool connection_parameters_set;
  sl_rtl_cs_libitem rtl_handle;
  uint8_t instance_id;
  cs_result_cb_t result_cb;
  cs_intermediate_result_cb_t intermediate_result_cb;
  cs_error_cb_t error_cb;
  uint32_t procedure_start_time_ms;
  uint32_t procedure_stop_time_ms;
  uint32_t log_error_counter;
  uint8_t drop_counter;
  app_timer_t timer_handle;
  bool error_timer_started;
  bool error_timer_elapsed;
  uint8_t initiator_state;
  uint8_t procedure_enable_retry_counter;
  uint8_t num_antenna_path;
  uint8_t antenna_config;
  cs_ranging_data_t ranging_data_result;
} cs_initiator_t;

#ifdef __cplusplus
}
#endif

#endif // CS_INITIATOR_COMMON_H
