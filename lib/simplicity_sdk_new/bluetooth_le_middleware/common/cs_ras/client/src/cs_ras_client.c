/***************************************************************************//**
 * @file
 * @brief CS RAS Client - Core implementation
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
#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"
#include "sl_common.h"
#include "sl_bt_api.h"
#include "cs_ras_common.h"
#include "cs_ras_client.h"
#include "cs_ras_client_internal.h"
#include "cs_ras_client_messaging.h"
#include "cs_ras_client_config.h"
#include "cs_ras_client_log.h"
#include "sl_bluetooth_connection_config.h"
#include "cs_ras_client_control_point.h"

// -----------------------------------------------------------------------------
// Private variables

// Client storage
static cs_ras_client_t storage[SL_BT_CONFIG_MAX_CONNECTIONS];

// Default configuration
static const cccd_config_t default_config = {
  .array = {
    sl_bt_gatt_disable,        // RAS features
    sl_bt_gatt_notification,   // Real-Time Ranging Data
    sl_bt_gatt_indication,     // Control Point
    sl_bt_gatt_indication,     // Ranging Data Ready
    sl_bt_gatt_indication,     // Ranging Data Overwritten
    sl_bt_gatt_notification    // On-Demand Ranging Data
  }
};

// -----------------------------------------------------------------------------
// Forward declaration of private functions

static void set_state(cs_ras_client_t *client,
                      cs_ras_client_state_t new_state);
static sl_status_t subscribe_next(cs_ras_client_t *client);
static bool handle_value(cs_ras_client_t *client,
                         sl_bt_evt_gatt_characteristic_value_t *data);
static bool handle_read_response(cs_ras_client_t *client,
                                 sl_bt_evt_gatt_characteristic_value_t *data);
static bool handle_procedure_completed(cs_ras_client_t *client,
                                       sl_bt_evt_gatt_procedure_completed_t *data);
static void clear_mode(cs_ras_client_t *client);
static sl_status_t change_mode(cs_ras_client_t *client,
                               cs_ras_mode_t   mode);
static void data_arrived_timer_rised(app_timer_t *timer, void *next);
static void data_ready_timer_rised(app_timer_t *timer, void *next);
static void control_point_timer_rised(app_timer_t *timer, void *data);

// -----------------------------------------------------------------------------
// Public functions

sl_status_t cs_ras_client_create(uint8_t                 connection,
                                 cs_ras_gattdb_handles_t *handles,
                                 uint16_t                att_mtu)
{
  sl_status_t sc;
  if (connection == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (handles == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if ((att_mtu > ATT_MTU_MAX) || (att_mtu < ATT_MTU_MIN)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Check for mandatory GATT database handles
  if ((handles->array[CS_RAS_CHARACTERISTIC_INDEX_RAS_FEATURES] == CS_RAS_INVALID_CHARACTERISTIC_HANDLE)
      || (handles->array[CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT] == CS_RAS_INVALID_CHARACTERISTIC_HANDLE)
      || (handles->array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY] == CS_RAS_INVALID_CHARACTERISTIC_HANDLE)
      || (handles->array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_OVERWRITTEN] == CS_RAS_INVALID_CHARACTERISTIC_HANDLE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Find empty slot
  cs_ras_client_t *client = cs_ras_client_find(SL_BT_INVALID_CONNECTION_HANDLE);
  if (client == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  // Start reading RAS server features
  sc = sl_bt_gatt_read_characteristic_value(connection,
                                            handles->array[CS_RAS_CHARACTERISTIC_INDEX_RAS_FEATURES]);
  if (sc == SL_STATUS_OK) {
    // Clean storage
    memset(client, 0, sizeof(cs_ras_client_t));
    // Apply default config
    memcpy(&client->subscription.cccd.config,
           &default_config,
           sizeof(client->subscription.cccd.config));
    // Register handles
    client->connection = connection;
    client->handles = handles;
    // Set initial MTU
    client->att_mtu = att_mtu;
    // Set initial state
    set_state(client, CLIENT_STATE_READ_FEATURES);
  }
  return sc;
}

sl_status_t cs_ras_client_get_features(uint8_t           connection,
                                       cs_ras_features_t *features)
{
  if (connection == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (features == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  cs_ras_client_t *client = cs_ras_client_find(connection);
  if (client == NULL) {
    return SL_STATUS_NOT_FOUND;
  }
  // Check for state validity
  if (client->state == CLIENT_STATE_NOT_INITIALIZED
      || client->state == CLIENT_STATE_READ_FEATURES) {
    return SL_STATUS_INVALID_STATE;
  }
  // Assign features
  *features = client->features;
  return SL_STATUS_OK;
}

sl_status_t cs_ras_client_real_time_receive(uint8_t  connection,
                                            uint32_t data_size,
                                            uint8_t  *data)
{
  sl_status_t sc;
  if (connection == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (data_size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  cs_ras_client_t *client = cs_ras_client_find(connection);
  if (client == NULL) {
    return SL_STATUS_NOT_FOUND;
  }
  // Check for state validity
  if (client->state != CLIENT_STATE_REAL_TIME) {
    return SL_STATUS_INVALID_STATE;
  }
  if (client->operation) {
    return SL_STATUS_IN_PROGRESS;
  }
  // Configure messaging
  cs_ras_messaging_config_t config = {
    .conn_handle = client->connection,
    .att_mtu = client->att_mtu,
    .real_time = true,
    .indication =  (sl_bt_gatt_indication
                    == client->subscription.cccd.state.array[CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA]),
    .data_size = data_size,
    .data = data,
    .start_segment = 0,
    .end_segment = CS_RAS_MESSAGING_ALL_SEGMENTS,
  };

  sc = cs_ras_client_messaging_receive(&client->messaging,
                                       &config,
                                       client->handles);
  if (sc == SL_STATUS_OK) {
    client->operation = true;
  }
  return sc;
}

sl_status_t cs_ras_client_abort(uint8_t connection)
{
  sl_status_t sc;
  if (connection == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  cs_ras_client_t *client = cs_ras_client_find(connection);
  if (client == NULL) {
    return SL_STATUS_NOT_FOUND;
  }
  // Check for state validity
  if (client->state != CLIENT_STATE_ON_DEMAND) {
    return SL_STATUS_INVALID_STATE;
  }
  if (!client->operation
      || (client->op_code != CS_RAS_CP_OPCODE_GET
          && client->op_code != CS_RAS_CP_OPCODE_RETRIEVE_LOST_SEGMENTS)) {
    return SL_STATUS_INVALID_STATE;
  }
  if ((client->features & CS_RAS_FEATURE_ABORT_OP_MASK) == 0) {
    // Abort Op Code not supported by the RAS server
    return SL_STATUS_NOT_SUPPORTED;
  }
  sc = cs_ras_client_control_point_abort(client);
  if (sc == SL_STATUS_OK) {
    // Start timer
    (void)app_timer_start(&client->timer.control_point,
                          CS_RAS_CLIENT_CONTROL_POINT_TIMEOUT_MS,
                          control_point_timer_rised,
                          NULL,
                          false);
  }
  return sc;
}

sl_status_t cs_ras_client_ack(uint8_t                  connection,
                              cs_ras_ranging_counter_t ranging_counter)
{
  sl_status_t sc;
  if (connection == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  cs_ras_client_t *client = cs_ras_client_find(connection);
  if (client == NULL) {
    return SL_STATUS_NOT_FOUND;
  }
  // Check for state validity
  if (client->state != CLIENT_STATE_ON_DEMAND) {
    return SL_STATUS_INVALID_STATE;
  }
  if (client->operation) {
    return SL_STATUS_IN_PROGRESS;
  }
  sc = cs_ras_client_control_point_ack(client, ranging_counter);
  if (sc == SL_STATUS_OK) {
    // Start timer
    (void)app_timer_start(&client->timer.control_point,
                          CS_RAS_CLIENT_CONTROL_POINT_TIMEOUT_MS,
                          control_point_timer_rised,
                          NULL,
                          false);
  }
  return sc;
}

sl_status_t cs_ras_client_configure(uint8_t                connection,
                                    cs_ras_client_config_t config)
{
  if (connection == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  cs_ras_client_t *client = cs_ras_client_find(connection);
  if (client == NULL) {
    return SL_STATUS_NOT_FOUND;
  }
  // Configuration is permitted only in CS_RAS_MODE_NONE
  if (client->state != CLIENT_STATE_INITIALIZED) {
    return SL_STATUS_INVALID_STATE;
  }
  // Apply configuration to CCCD subscription configuration
  client->subscription.cccd.config.array[CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA]
    = config.on_demand_ranging_data_indication ? sl_bt_gatt_indication : sl_bt_gatt_notification;

  client->subscription.cccd.config.array[CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA]
    = config.real_time_ranging_data_indication ? sl_bt_gatt_indication : sl_bt_gatt_notification;

  client->subscription.cccd.config.array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY]
    = config.ranging_data_ready_notification ? sl_bt_gatt_notification : sl_bt_gatt_indication;

  client->subscription.cccd.config.array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_OVERWRITTEN]
    = config.ranging_data_overwritten_notification ? sl_bt_gatt_notification : sl_bt_gatt_indication;

  return SL_STATUS_OK;
}

sl_status_t cs_ras_client_select_mode(uint8_t connection,
                                      cs_ras_mode_t mode)
{
  sl_status_t sc;
  cs_ras_mode_t current_mode;

  // Check handle
  if (connection == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  // Check for invalid mode
  if (mode == CS_RAS_MODE_CHANGE_IN_PROGRESS) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  cs_ras_client_t *client = cs_ras_client_find(connection);
  if (client == NULL) {
    return SL_STATUS_NOT_FOUND;
  }
  // Check state validity (no operation should be in progress)
  if (client->state != CLIENT_STATE_INITIALIZED
      && client->state != CLIENT_STATE_ON_DEMAND
      && client->state != CLIENT_STATE_REAL_TIME) {
    return SL_STATUS_INVALID_STATE;
  }
  if (mode == CS_RAS_MODE_REAL_TIME_RANGING_DATA) {
    // Real-Time Ranging Data requested
    // Check if Real-Time Ranging Data characteristic
    if (client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA]
        == CS_RAS_INVALID_CHARACTERISTIC_HANDLE) {
      return SL_STATUS_NOT_SUPPORTED;
    }
    if ((client->features & CS_RAS_FEATURE_RT_RANGING_DATA_MASK) == 0) {
      // Real-Time Ranging Data is requested but that is not supported by the RAS
      // server.
      return SL_STATUS_NOT_SUPPORTED;
    }
  }

  (void)cs_ras_client_get_mode(client->connection, &current_mode);

  cs_ras_client_log_info(CONN_PREFIX "Select mode: %u. Current mode: %u" LOG_NL,
                         client->connection,
                         mode,
                         current_mode);

  if (current_mode == mode) {
    // Pass current mode
    cs_ras_client_on_mode_changed(client->connection,
                                  current_mode,
                                  SL_STATUS_OK);
    return SL_STATUS_OK;
  }

  // Changing mode is required
  sc = change_mode(client, mode);

  return sc;
}

sl_status_t cs_ras_client_get_mode(uint8_t connection, cs_ras_mode_t *mode)
{
  if (mode == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (connection == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  cs_ras_client_t *client = cs_ras_client_find(connection);
  if (client == NULL) {
    return SL_STATUS_NOT_FOUND;
  }
  // Get mode based on the current state
  switch (client->state) {
    case CLIENT_STATE_ON_DEMAND:
      *mode = CS_RAS_MODE_ON_DEMAND_RANGING_DATA;
      return SL_STATUS_OK;
      break;
    case CLIENT_STATE_REAL_TIME:
      *mode = CS_RAS_MODE_REAL_TIME_RANGING_DATA;
      return SL_STATUS_OK;
      break;
    case CLIENT_STATE_INITIALIZED:
      *mode = CS_RAS_MODE_NONE;
      return SL_STATUS_OK;
      break;
    default:
      *mode = CS_RAS_MODE_CHANGE_IN_PROGRESS;
      return SL_STATUS_OK;
      break;
  }
  cs_ras_client_log_info(CONN_PREFIX "Get mode: %u" LOG_NL,
                         client->connection,
                         *mode);
}

sl_status_t cs_ras_client_get_ranging_data(uint8_t  connection,
                                           uint16_t ranging_counter,
                                           uint32_t data_size,
                                           uint8_t  *data)
{
  sl_status_t sc;
  if (connection == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (data_size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  cs_ras_client_t *client = cs_ras_client_find(connection);
  if (client == NULL) {
    return SL_STATUS_NOT_FOUND;
  }
  // Check for state validity
  if (client->state != CLIENT_STATE_ON_DEMAND) {
    return SL_STATUS_INVALID_STATE;
  }
  if (client->operation) {
    return SL_STATUS_IN_PROGRESS;
  }
  cs_ras_messaging_config_t config = {
    .conn_handle = client->connection,
    .att_mtu = client->att_mtu,
    .real_time = false,
    .indication =  (sl_bt_gatt_indication
                    == client->subscription.cccd.state.array[CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA]),
    .data_size = data_size,
    .data = data,
    .start_segment = 0,
    .end_segment = CS_RAS_MESSAGING_ALL_SEGMENTS,
  };

  sc = cs_ras_client_messaging_receive(&client->messaging,
                                       &config,
                                       client->handles);
  if (sc != SL_STATUS_OK) {
    return sc;
  }

  // Send request using RAS control point
  sc = cs_ras_client_control_point_get_ranging_data(client, ranging_counter);
  if (sc == SL_STATUS_OK) {
    // Configure messaging

    // Start timer with timeout for Get On-Demand Ranging Data
    (void)app_timer_start(&client->timer.data_arrived,
                          CS_RAS_CLIENT_GET_ON_DEMAND_RANGING_DATA_TIMEOUT_MS,
                          data_arrived_timer_rised,
                          0,
                          false);
    // Save requested counter
    client->ranging_counter = ranging_counter;
  } else {
    // Stop reception
    (void)cs_ras_client_messaging_stop(&client->messaging);
  }
  return sc;
}

sl_status_t cs_ras_client_retreive_lost_segments(uint8_t  connection,
                                                 uint16_t ranging_counter,
                                                 uint8_t  start_segment,
                                                 uint8_t  end_segment,
                                                 uint32_t data_size,
                                                 uint8_t  *data)
{
  sl_status_t sc;
  if (connection == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (data_size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (start_segment > end_segment) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  cs_ras_client_t *client = cs_ras_client_find(connection);
  if (client == NULL) {
    return SL_STATUS_NOT_FOUND;
  }
  // Check for state validity
  if (client->state != CLIENT_STATE_ON_DEMAND) {
    return SL_STATUS_INVALID_STATE;
  }
  if (client->operation) {
    return SL_STATUS_IN_PROGRESS;
  }
  if ((client->features & CS_RAS_FEATURE_RETRIEVE_LOST_SEGMENT_MASK) == 0) {
    // Retrieve Lost Segments is requested but that is not supported by the RAS
    // server.
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Configure messaging
  cs_ras_messaging_config_t config = {
    .conn_handle = client->connection,
    .att_mtu = client->att_mtu,
    .real_time = false,
    .indication =  (sl_bt_gatt_indication
                    == client->subscription.cccd.state.array[CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA]),
    .data_size = data_size,
    .data = data,
    .start_segment = start_segment,
    .end_segment = end_segment,
  };

  sc = cs_ras_client_messaging_receive_lost(&client->messaging,
                                            &config,
                                            client->handles,
                                            start_segment,
                                            end_segment);
  if (sc != SL_STATUS_OK) {
    return sc;
  }

  // Send request using RAS control point
  sc = cs_ras_client_control_point_retreive_lost_segments(client,
                                                          ranging_counter,
                                                          start_segment,
                                                          end_segment);
  if (sc == SL_STATUS_OK) {
    // Start timer with timeout for Get On-Demand Ranging Data
    (void)app_timer_start(&client->timer.data_arrived,
                          CS_RAS_CLIENT_GET_ON_DEMAND_RANGING_DATA_TIMEOUT_MS,
                          data_arrived_timer_rised,
                          0,
                          false);
    // Save requested counter
    client->ranging_counter = ranging_counter;
  } else {
    // Stop reception
    (void)cs_ras_client_messaging_stop(&client->messaging);
  }
  return sc;
}

sl_status_t cs_ras_client_read_data_ready(uint8_t connection)
{
  if (connection == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  cs_ras_client_t *client = cs_ras_client_find(connection);
  if (client == NULL) {
    return SL_STATUS_NOT_FOUND;
  }
  if (client->state != CLIENT_STATE_ON_DEMAND) {
    return SL_STATUS_INVALID_STATE;
  }
  sl_status_t sc = sl_bt_gatt_read_characteristic_value(client->connection,
                                                        client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY]);
  return sc;
}

sl_status_t cs_ras_client_read_data_overwritten(uint8_t connection)
{
  if (connection == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  cs_ras_client_t *client = cs_ras_client_find(connection);
  if (client == NULL) {
    return SL_STATUS_NOT_FOUND;
  }
  if (client->state != CLIENT_STATE_ON_DEMAND) {
    return SL_STATUS_INVALID_STATE;
  }
  sl_status_t sc = sl_bt_gatt_read_characteristic_value(client->connection,
                                                        client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_OVERWRITTEN]);
  return sc;
}

sl_status_t cs_ras_client_procedure_enabled(uint8_t connection,
                                            bool    enabled)
{
  sl_status_t sc;
  if (connection == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  cs_ras_client_t *client = cs_ras_client_find(connection);
  if (client == NULL) {
    return SL_STATUS_NOT_FOUND;
  }

  if (!enabled) {
    // Stop timers
    (void)app_timer_stop(&client->timer.data_ready);
    (void)app_timer_stop(&client->timer.data_arrived);
    return SL_STATUS_OK;
  }

  switch (client->state) {
    case CLIENT_STATE_ON_DEMAND:
      // Start timer with procedure timeout Data Ready (indication/notification).
      sc = app_timer_start(&client->timer.data_ready,
                           CS_RAS_CLIENT_RANGING_DATA_READY_PROCEDURE_TIMEOUT_MS,
                           data_ready_timer_rised,
                           0,
                           false);
      break;
    case CLIENT_STATE_REAL_TIME:
      // Start timer with procedure timeout for Real-Time Ranging Data
      sc = app_timer_start(&client->timer.data_arrived,
                           CS_RAS_CLIENT_REAL_TIME_RANGING_DATA_PROCEDURE_TIMEOUT_MS,
                           data_arrived_timer_rised,
                           0,
                           false);
      break;
    default:
      sc = SL_STATUS_INVALID_STATE;
      break;
  }
  return sc;
}

// -----------------------------------------------------------------------------
// Internal functions

void cs_ras_client_init(void)
{
  memset(storage, 0, sizeof(storage));
  for (uint8_t i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
    storage[i].connection = SL_BT_INVALID_CONNECTION_HANDLE;
  }
}

cs_ras_client_t *cs_ras_client_find(uint8_t connection)
{
  for (uint8_t i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
    if (storage[i].connection == connection) {
      return &storage[i];
    }
  }
  return NULL;
}

cs_ras_client_t *cs_ras_client_find_by_timer(app_timer_t *timer)
{
  for (uint8_t i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
    if (&storage[i].timer.data_arrived == timer
        || &storage[i].timer.data_ready == timer
        || &storage[i].timer.control_point == timer) {
      return &storage[i];
    }
  }
  return NULL;
}

bool cs_ras_client_on_bt_event(sl_bt_msg_t *evt)
{
  cs_ras_client_t *client;
  bool handled = false;

  handled |= !cs_ras_client_messaging_on_bt_event(evt);
  handled |= !cs_ras_client_control_point_on_bt_event(evt);

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_connection_closed_id:
      client = cs_ras_client_find(evt->data.evt_connection_closed.connection);
      if (client == NULL) {
        break;
      }
      (void)app_timer_stop(&client->timer.control_point);
      (void)app_timer_stop(&client->timer.data_arrived);
      (void)app_timer_stop(&client->timer.data_ready);
      // Remove
      set_state(client, CLIENT_STATE_NOT_INITIALIZED);
      client->connection = SL_BT_INVALID_CONNECTION_HANDLE;
      break;
    case sl_bt_evt_gatt_characteristic_value_id:
      client = cs_ras_client_find(evt->data.evt_gatt_characteristic_value.connection);
      if (client == NULL) {
        break;
      }
      if (evt->data.evt_gatt_characteristic_value.att_opcode
          == sl_bt_gatt_read_response) {
        // Read response
        handled |= handle_read_response(client,
                                        &evt->data.evt_gatt_characteristic_value);
      } else if ((evt->data.evt_gatt_characteristic_value.att_opcode
                  == sl_bt_gatt_handle_value_notification)
                 || (evt->data.evt_gatt_characteristic_value.att_opcode
                     == sl_bt_gatt_handle_value_indication)) {
        handled |= handle_value(client,
                                &evt->data.evt_gatt_characteristic_value);
      }
      break;
    case sl_bt_evt_gatt_mtu_exchanged_id:
      client = cs_ras_client_find(evt->data.evt_gatt_mtu_exchanged.connection);
      if (client == NULL) {
        break;
      }
      client->att_mtu = evt->data.evt_gatt_mtu_exchanged.mtu;
      cs_ras_client_log_info(CONN_PREFIX "MTU exchanged: %u" LOG_NL,
                             client->connection,
                             client->att_mtu);
      break;
    case sl_bt_evt_gatt_procedure_completed_id:
      client = cs_ras_client_find(evt->data.evt_gatt_procedure_completed.connection);
      if (client == NULL) {
        break;
      }
      handled |= handle_procedure_completed(client,
                                            &evt->data.evt_gatt_procedure_completed);
      break;
  }
  return !handled;
}

// -----------------------------------------------------------------------------
// Private functions

static sl_status_t change_mode(cs_ras_client_t *client,
                               cs_ras_mode_t   mode)
{
  sl_status_t sc;
  cs_ras_client_state_t new_state;

  // Start unsubscribing
  if (mode == CS_RAS_MODE_REAL_TIME_RANGING_DATA) {
    client->subscription.mode_index = CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA;
    if (client->state == CLIENT_STATE_INITIALIZED) {
      // Subscription required from Real-Time Ranging Data
      client->subscription.current_index = CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA;
      new_state = CLIENT_STATE_SELECT_REAL_TIME_MODE_SUBSCRIBE;
      client->subscription.cccd.request
        = client->subscription.cccd.config.array[client->subscription.current_index];
    } else {
      // Unsubscription required from On-Demand Ranging Data
      client->subscription.current_index = CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA;
      new_state = CLIENT_STATE_SELECT_REAL_TIME_MODE_UNSUBSCRIBE;
      client->subscription.cccd.request = sl_bt_gatt_disable;
    }
  } else if (mode == CS_RAS_MODE_ON_DEMAND_RANGING_DATA) {
    client->subscription.mode_index = CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA;
    if (client->state == CLIENT_STATE_INITIALIZED) {
      // Subscription required from On-Demand Ranging Data
      client->subscription.current_index = CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT;
      new_state = CLIENT_STATE_SELECT_ON_DEMAND_MODE_SUBSCRIBE;
      client->subscription.cccd.request
        = client->subscription.cccd.config.array[client->subscription.current_index];
    } else {
      // Unsubscription required from Real-Time Ranging Data
      client->subscription.current_index = CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA;
      new_state = CLIENT_STATE_SELECT_ON_DEMAND_MODE_UNSUBSCRIBE;
      client->subscription.cccd.request = sl_bt_gatt_disable;
    }
  } else {
    // No mode selected
    if (client->state == CLIENT_STATE_ON_DEMAND) {
      // Unsubscription required from On-Demand Ranging Data
      client->subscription.current_index = CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA;
      client->subscription.cccd.request = sl_bt_gatt_disable;
    } else {
      // Unsubscription required from Real-Time Ranging Data
      client->subscription.current_index = CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA;
      client->subscription.cccd.request = sl_bt_gatt_disable;
    }
    new_state = CLIENT_STATE_DESELECT_MODE;
  }

  // Change subscription (disable)
  sc = sl_bt_gatt_set_characteristic_notification(client->connection,
                                                  client->handles->array[client->subscription.current_index],
                                                  client->subscription.cccd.request);
  cs_ras_client_log_debug(CONN_PREFIX "Subscribe %u - value: 0x%04x,  sc = 0x%04lx" LOG_NL,
                          client->connection,
                          client->handles->array[client->subscription.current_index],
                          client->subscription.cccd.request,
                          (unsigned long)sc);
  // Apply state change
  if (sc == SL_STATUS_OK) {
    // Stop active reception if mode change is requested
    (void)cs_ras_client_messaging_stop(&client->messaging);
    // Apply state
    set_state(client, new_state);
    // Change to intermediate mode (mode change in progress)
    cs_ras_client_on_mode_changed(client->connection,
                                  CS_RAS_MODE_CHANGE_IN_PROGRESS,
                                  SL_STATUS_OK);
  }
  return sc;
}

static void set_state(cs_ras_client_t *client,
                      cs_ras_client_state_t new_state)
{
  cs_ras_client_state_t old_state = client->state;
  client->state = new_state;
  (void)old_state;
  cs_ras_client_log_debug(CONN_PREFIX "State change %u -> %u" LOG_NL,
                          client->connection,
                          old_state,
                          client->state);
}

static sl_status_t subscribe_next(cs_ras_client_t *client)
{
  sl_status_t sc;
  client->subscription.cccd.request
    = client->subscription.cccd.config.array[client->subscription.current_index];
  // Change subscription
  sc = sl_bt_gatt_set_characteristic_notification(client->connection,
                                                  client->handles->array[client->subscription.current_index],
                                                  client->subscription.cccd.request);
  cs_ras_client_log_debug(CONN_PREFIX "Subscribe %u - value: 0x%04x,  sc = 0x%04lx" LOG_NL,
                          client->connection,
                          client->handles->array[client->subscription.current_index],
                          client->subscription.cccd.request,
                          (unsigned long)sc);
  return sc;
}

static bool handle_value(cs_ras_client_t *client,
                         sl_bt_evt_gatt_characteristic_value_t *data)
{
  // Mark events as handled
  if (data->characteristic == client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA]
      || data->characteristic == client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA]) {
    return true;
  }
  // Check for characteristic
  if (data->characteristic != client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY]
      && data->characteristic != client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_OVERWRITTEN]) {
    return false;
  }
  // Send confirmation
  if (data->att_opcode == sl_bt_gatt_handle_value_indication) {
    (void)sl_bt_gatt_send_characteristic_confirmation(client->connection);
  }
  // Check size
  if (data->value.len != sizeof(cs_ras_ranging_counter_t)) {
    cs_ras_client_log_warning(CONN_PREFIX "Invalid size for handle value for characteristic 0x%04x." LOG_NL,
                              client->connection,
                              data->characteristic);
    return true;
  }
  // Check for settings
  if (data->characteristic == client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY]) {
    if (data->att_opcode == sl_bt_gatt_handle_value_indication
        && (client->subscription.cccd.state.array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY]
            != sl_bt_gatt_indication)) {
      goto att_opcode_error;
    }
    if (data->att_opcode == sl_bt_gatt_handle_value_notification
        && (client->subscription.cccd.state.array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY]
            != sl_bt_gatt_notification)) {
      goto att_opcode_error;
    }
  }
  if (data->characteristic == client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_OVERWRITTEN]) {
    if (data->att_opcode == sl_bt_gatt_handle_value_indication
        && (client->subscription.cccd.state.array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_OVERWRITTEN]
            != sl_bt_gatt_indication)) {
      goto att_opcode_error;
    }
    if (data->att_opcode == sl_bt_gatt_handle_value_notification
        && (client->subscription.cccd.state.array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_OVERWRITTEN]
            != sl_bt_gatt_notification)) {
      goto att_opcode_error;
    }
  }
  // Get ranging counter
  cs_ras_ranging_counter_t ranging_counter;
  memcpy(&ranging_counter, data->value.data, sizeof(ranging_counter));
  if (data->characteristic == client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY]) {
    // Stop timer
    (void)app_timer_stop(&client->timer.data_ready);
    // Call data ready
    cs_ras_client_on_ranging_data_ready(client->connection,
                                        ranging_counter);
  } else {
    cs_ras_client_on_ranging_data_overwritten(client->connection,
                                              ranging_counter);
  }
  return true;

  att_opcode_error:
  cs_ras_client_log_warning(CONN_PREFIX "Improper ATT opcode for characteristic 0x%04x." LOG_NL,
                            client->connection,
                            data->characteristic);
  return true;
}

static bool handle_read_response(cs_ras_client_t *client,
                                 sl_bt_evt_gatt_characteristic_value_t *data)
{
  bool handled = false;
  // RAS features value arrived
  if (data->characteristic
      == client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_RAS_FEATURES]) {
    if (data->value.len == sizeof(cs_ras_features_t)
        && client->state == CLIENT_STATE_READ_FEATURES) {
      client->features = *((cs_ras_features_t *)data->value.data);
      client->features_read = true;
      cs_ras_client_log_info(CONN_PREFIX "RAS features received: 0x%08lx" LOG_NL,
                             client->connection,
                             (unsigned long)client->features);
    }
    handled = true;
  } else if ((data->characteristic
              == client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY])
             || (data->characteristic
                 == client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_OVERWRITTEN])) {
    // Get ranging counter
    cs_ras_ranging_counter_t ranging_counter;
    memcpy(&ranging_counter, data->value.data, sizeof(ranging_counter));
    if (data->characteristic == client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY]) {
      // Stop timer
      (void)app_timer_stop(&client->timer.data_ready);
      // Call data ready
      cs_ras_client_on_ranging_data_ready(client->connection,
                                          ranging_counter);
    } else {
      cs_ras_client_on_ranging_data_overwritten(client->connection,
                                                ranging_counter);
    }
  }
  return handled;
}

static bool handle_procedure_completed(cs_ras_client_t *client,
                                       sl_bt_evt_gatt_procedure_completed_t *data)
{
  bool handled = false;
  sl_status_t sc;

  cs_ras_client_log_debug(CONN_PREFIX "Procedure completed: 0x%04x" LOG_NL,
                          client->connection,
                          data->result);

  // Procedure failed
  if (data->result != SL_STATUS_OK) {
    switch (client->state) {
      case CLIENT_STATE_READ_FEATURES:
        set_state(client, CLIENT_STATE_NOT_INITIALIZED);
        cs_ras_client_on_initialized(client->connection,
                                     client->features,
                                     data->result);
        handled = true;
        break;
      case CLIENT_STATE_DESELECT_MODE:
      case CLIENT_STATE_SELECT_ON_DEMAND_MODE_UNSUBSCRIBE:
      case CLIENT_STATE_SELECT_REAL_TIME_MODE_UNSUBSCRIBE:
        clear_mode(client);
        // Finished with error, go back to original state
        if (client->subscription.current_index
            == CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA) {
          set_state(client, CLIENT_STATE_ON_DEMAND);
          cs_ras_client_on_mode_changed(client->connection,
                                        CS_RAS_MODE_ON_DEMAND_RANGING_DATA,
                                        data->result);
        } else if (client->subscription.current_index
                   == CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA) {
          set_state(client, CLIENT_STATE_REAL_TIME);
          cs_ras_client_on_mode_changed(client->connection,
                                        CS_RAS_MODE_REAL_TIME_RANGING_DATA,
                                        data->result);
        }
        handled = true;
        break;
      case CLIENT_STATE_SELECT_ON_DEMAND_MODE_SUBSCRIBE:
      case CLIENT_STATE_SELECT_REAL_TIME_MODE_SUBSCRIBE:
        // Finished with error after unsubscribe. go to initialized.
        set_state(client, CLIENT_STATE_INITIALIZED);
        cs_ras_client_on_mode_changed(client->connection,
                                      CS_RAS_MODE_NONE,
                                      data->result);
        break;
      default:
        break;
    }
    return handled;
  }

  // Successful procedure
  switch (client->state) {
    case CLIENT_STATE_READ_FEATURES:
      if (client->features_read) {
        set_state(client, CLIENT_STATE_INITIALIZED);
        cs_ras_client_log_info(CONN_PREFIX "RAS features completed." LOG_NL,
                               client->connection);
        cs_ras_client_on_initialized(client->connection,
                                     client->features,
                                     SL_STATUS_OK);
      } else {
        set_state(client, CLIENT_STATE_NOT_INITIALIZED);
        cs_ras_client_log_info(CONN_PREFIX "RAS features read failed." LOG_NL,
                               client->connection);
        cs_ras_client_on_initialized(client->connection,
                                     client->features,
                                     SL_STATUS_FAIL);
      }
      break;
    case CLIENT_STATE_DESELECT_MODE:
      // Set CCCD state to requested
      client->subscription.cccd.state.array[client->subscription.current_index]
        = client->subscription.cccd.request;
      set_state(client, CLIENT_STATE_INITIALIZED);
      cs_ras_client_on_mode_changed(client->connection,
                                    CS_RAS_MODE_NONE,
                                    SL_STATUS_OK);
      handled = true;
      break;
    case CLIENT_STATE_SELECT_ON_DEMAND_MODE_UNSUBSCRIBE:
      client->subscription.cccd.state.array[client->subscription.current_index]
        = client->subscription.cccd.request;
      client->subscription.current_index = CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT;
      client->subscription.mode_index = CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA;
      sc = subscribe_next(client);
      if (sc == SL_STATUS_OK) {
        set_state(client, CLIENT_STATE_SELECT_ON_DEMAND_MODE_SUBSCRIBE);
      } else {
        // Finished with error
        set_state(client, CLIENT_STATE_INITIALIZED);
        cs_ras_client_on_mode_changed(client->connection,
                                      CS_RAS_MODE_NONE,
                                      sc);
      }
      handled = true;
      break;
    case CLIENT_STATE_SELECT_REAL_TIME_MODE_UNSUBSCRIBE:
      client->subscription.cccd.state.array[client->subscription.current_index]
        = client->subscription.cccd.request;
      client->subscription.current_index = CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA;
      client->subscription.mode_index = CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA;
      sc = subscribe_next(client);
      if (sc == SL_STATUS_OK) {
        set_state(client, CLIENT_STATE_SELECT_REAL_TIME_MODE_SUBSCRIBE);
      } else {
        // Finished with error
        set_state(client, CLIENT_STATE_INITIALIZED);
        cs_ras_client_on_mode_changed(client->connection,
                                      CS_RAS_MODE_NONE,
                                      sc);
      }
      handled = true;
      break;
    case CLIENT_STATE_SELECT_ON_DEMAND_MODE_SUBSCRIBE:
      client->subscription.cccd.state.array[client->subscription.current_index]
        = client->subscription.cccd.request;
      if (client->subscription.current_index == client->subscription.mode_index) {
        // Finished
        clear_mode(client);
        set_state(client, CLIENT_STATE_ON_DEMAND);
        cs_ras_client_on_mode_changed(client->connection,
                                      CS_RAS_MODE_ON_DEMAND_RANGING_DATA,
                                      SL_STATUS_OK);
      } else {
        client->subscription.current_index++;
        sc = subscribe_next(client);
        if (sc == SL_STATUS_OK) {
          set_state(client, CLIENT_STATE_SELECT_ON_DEMAND_MODE_SUBSCRIBE);
        } else {
          // Finished with error
          set_state(client, CLIENT_STATE_INITIALIZED);
          cs_ras_client_on_mode_changed(client->connection,
                                        CS_RAS_MODE_NONE,
                                        sc);
        }
      }
      handled = true;
      break;
    case CLIENT_STATE_SELECT_REAL_TIME_MODE_SUBSCRIBE:
      client->subscription.cccd.state.array[client->subscription.current_index]
        = client->subscription.cccd.request;
      clear_mode(client);
      set_state(client, CLIENT_STATE_REAL_TIME);
      cs_ras_client_on_mode_changed(client->connection,
                                    CS_RAS_MODE_REAL_TIME_RANGING_DATA,
                                    SL_STATUS_OK);
      handled = true;
      break;
    default:
      break;
  }
  return handled;
}

static void clear_mode(cs_ras_client_t *client)
{
  client->operation = false;
  client->ranging_counter = 0;
}

static sl_status_t do_action(cs_ras_client_t                *client,
                             cs_ras_client_timeout_action_t action)
{
  sl_status_t sc = SL_STATUS_OK;

  cs_ras_client_log_debug(CONN_PREFIX "Executing timeout action %u." LOG_NL,
                          client->connection,
                          action);

  switch (action) {
    case CS_RAS_CLIENT_TIMEOUT_ACTION_NOTHING:
      break;
    case CS_RAS_CLIENT_TIMEOUT_ACTION_ABORT:
      // Abort current transfer
      sc = cs_ras_client_control_point_abort(client);
      break;
    case CS_RAS_CLIENT_TIMEOUT_ACTION_DISABLE_REAL_TIME_MODE:
      // Change mode to None
      sc = change_mode(client, CS_RAS_MODE_NONE);
      break;
    case CS_RAS_CLIENT_TIMEOUT_ACTION_CLOSE_CONNECTION:
      // Close connection
      sc = sl_bt_connection_close(client->connection);
      break;
    case CS_RAS_CLIENT_TIMEOUT_ACTION_READ_DATA_READY:
      // Read Data Ready characteristic
      sc = sl_bt_gatt_read_characteristic_value(client->connection,
                                                client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY]);
      break;
    default:
      break;
  }
  if (sc != SL_STATUS_OK) {
    cs_ras_client_log_warning(CONN_PREFIX "Timeout action %u failed. Status: 0x%04lu" LOG_NL,
                              client->connection,
                              action,
                              (unsigned long)sc);
  }
  return sc;
}

static void data_arrived_timer_rised(app_timer_t *timer, void *next)
{
  // Search for client
  cs_ras_client_t *client = cs_ras_client_find_by_timer(timer);
  if (client == NULL) {
    return;
  }
  cs_ras_client_timeout_t timeout;
  cs_ras_client_timeout_action_t action = CS_RAS_CLIENT_TIMEOUT_ACTION_NOTHING;
  if (client->state == CLIENT_STATE_ON_DEMAND) {
    timeout = next ? CS_RAS_CLIENT_TIMEOUT_ON_DEMAND : CS_RAS_CLIENT_TIMEOUT_ON_DEMAND_START;
    // Check if Abort Opcode is supported
    if ((client->features & CS_RAS_FEATURE_ABORT_OP_MASK) != 0) {
      action = CS_RAS_CLIENT_TIMEOUT_ACTION_ABORT;
    }
  } else {
    timeout = next ? CS_RAS_CLIENT_TIMEOUT_REAL_TIME : CS_RAS_CLIENT_TIMEOUT_REAL_TIME_START;
    action = CS_RAS_CLIENT_TIMEOUT_ACTION_DISABLE_REAL_TIME_MODE;
  }

  cs_ras_client_log_debug(CONN_PREFIX "Data timeout %u, action %u." LOG_NL,
                          client->connection,
                          timeout,
                          action);

  bool response = cs_ras_client_on_timeout(client->connection,
                                           timeout,
                                           action);
  if (!response) {
    (void)do_action(client, action);
  }
}

static void data_ready_timer_rised(app_timer_t *timer, void *data)
{
  (void)data;
  // Search for client
  cs_ras_client_t *client = cs_ras_client_find_by_timer(timer);
  if (client == NULL) {
    return;
  }

  cs_ras_client_timeout_t timeout = CS_RAS_CLIENT_TIMEOUT_DATA_READY_INDICATION;
  cs_ras_client_timeout_action_t action = CS_RAS_CLIENT_TIMEOUT_ACTION_NOTHING;

  if (client->subscription.cccd.state.array[CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY]
      == sl_bt_gatt_notification) {
    timeout = CS_RAS_CLIENT_TIMEOUT_DATA_READY_NOTIFICATION;
    action = CS_RAS_CLIENT_TIMEOUT_ACTION_READ_DATA_READY;
  }

  cs_ras_client_log_debug(CONN_PREFIX "Data Ready timeout %u, action %u." LOG_NL,
                          client->connection,
                          timeout,
                          action);

  bool response = cs_ras_client_on_timeout(client->connection,
                                           timeout,
                                           action);
  if (!response) {
    (void)do_action(client, action);
  }
}

static void control_point_timer_rised(app_timer_t *timer, void *data)
{
  (void)data;
  // Search for client
  cs_ras_client_t *client = cs_ras_client_find_by_timer(timer);
  if (client == NULL) {
    return;
  }
  client->abort = false;
  client->operation = false;
  cs_ras_client_timeout_t timeout = CS_RAS_CLIENT_TIMEOUT_CONTROL_POINT;
  cs_ras_client_timeout_action_t action = CS_RAS_CLIENT_TIMEOUT_ACTION_CLOSE_CONNECTION;

  cs_ras_client_log_debug(CONN_PREFIX "Control Point timeout %u, action %u." LOG_NL,
                          client->connection,
                          timeout,
                          action);

  bool response = cs_ras_client_on_timeout(client->connection,
                                           timeout,
                                           action);
  if (!response) {
    (void)do_action(client, action);
  }
}

void cs_ras_client_messaging_segment_received(cs_ras_client_messaging_reception_t *rx,
                                              cs_ras_ranging_counter_t            counter)
{
  (void)counter;
  cs_ras_client_t *client = cs_ras_client_find(rx->config.conn_handle);
  if (client == NULL) {
    return;
  }
  (void)app_timer_stop(&client->timer.data_arrived);
  (void)app_timer_start(&client->timer.data_arrived,
                        CS_RAS_CLIENT_INTER_EVENT_TIMEOUT_MS,
                        data_arrived_timer_rised,
                        (void *)1,
                        false);
}

// -----------------------------------------------------------------------------
// Weak implementation of callback functions

SL_WEAK void cs_ras_client_on_initialized(uint8_t           connection,
                                          cs_ras_features_t features,
                                          sl_status_t       sc)
{
  (void)connection;
  (void)features;
  (void)sc;
}

SL_WEAK void cs_ras_client_on_mode_changed(uint8_t       connection,
                                           cs_ras_mode_t mode,
                                           sl_status_t   sc)
{
  (void)connection;
  (void)mode;
  (void)sc;
}

SL_WEAK void cs_ras_client_on_ranging_data_ready(uint8_t                  connection,
                                                 cs_ras_ranging_counter_t ranging_counter)
{
  (void)connection;
  (void)ranging_counter;
}

SL_WEAK void cs_ras_client_on_ranging_data_overwritten(uint8_t                  connection,
                                                       cs_ras_ranging_counter_t ranging_counter)
{
  (void)connection;
  (void)ranging_counter;
}

SL_WEAK bool cs_ras_client_on_timeout(uint8_t                        connection,
                                      cs_ras_client_timeout_t        timeout,
                                      cs_ras_client_timeout_action_t action)
{
  (void)connection;
  (void)timeout;
  (void)action;
  // Perform the action automatically
  return false;
}
