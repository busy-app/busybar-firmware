/***************************************************************************//**
 * @file
 * @brief CS RAS Server - Core implementation
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
#include "cs_ras_server.h"
#include "cs_ras_server_internal.h"
#include "cs_ras_server_config.h"
#include "cs_ras_server_log.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif // SL_CATALOG_POWER_MANAGER_PRESENT
#include "cs_ras_server_messaging.h"
#include "cs_ras_server_messaging_internal.h"
#include "cs_ras_server_control_point.h"
#include "sl_bluetooth_connection_config.h"
#include "app_queue.h"
#include "cs_ras_server_database.h"
#include "cs_ras_format_converter.h"

// -----------------------------------------------------------------------------
// Definitions

#define INDICATION(status)                  \
  ((status == sl_bt_gatt_server_indication) \
   || (status == sl_bt_gatt_server_notification_and_indication))

#define NOTIFICATION(status)                  \
  ((status == sl_bt_gatt_server_notification) \
   || (status == sl_bt_gatt_server_notification_and_indication))

#define CCCD_UUID 0x2902
#define INVALID_POWER_LEVEL_DBM 0x07F

// -----------------------------------------------------------------------------
// Forward declaration of privare functions

static uint32_t get_features(void);
static bool handle_read(cs_ras_server_t *server,
                        sl_bt_evt_gatt_server_user_read_request_t *req);
static bool handle_user_cccd_read(cs_ras_server_t *server,
                                  sl_bt_evt_gatt_server_user_read_request_t *req);
static bool handle_user_cccd_write(cs_ras_server_t *server,
                                   sl_bt_evt_gatt_server_user_write_request_t *req);
static sl_status_t tx_queue_send(cs_ras_server_t *server,
                                 bool            indication,
                                 uint16_t        characteristic,
                                 uint8_t         data_size,
                                 uint8_t         *data);
static void tx_queue_sent(cs_ras_server_t *server,
                          bool            indication,
                          uint16_t        characteristic,
                          uint8_t         data_size,
                          uint8_t         *data);
static void real_time_transmit_step(void);
static void tx_queue_step(void);
static void response_timer_raised(app_timer_t *timer, void *data);
static void retention_timer_raised(app_timer_t *timer, void *data);
static sl_status_t cs_ras_server_ranging_data_arrived(uint8_t connection,
                                                      sl_bt_msg_t *evt);
static bool gattdb_init(void);

// -----------------------------------------------------------------------------
// Private variables

static const uint16_t ras_uuids[CS_RAS_CHARACTERISTIC_INDEX_COUNT] = {
  CS_RAS_CHAR_UUID_RAS_FEATURES,
  CS_RAS_CHAR_UUID_REAL_TIME_RANGING_DATA,
  CS_RAS_CHAR_UUID_CONTROL_POINT,
  CS_RAS_CHAR_UUID_RANGING_DATA_READY,
  CS_RAS_CHAR_UUID_RANGING_DATA_OVERWRITTEN,
  CS_RAS_CHAR_UUID_ON_DEMAND_RANGING_DATA
};
static cs_ras_gattdb_handles_t ras_gattdb_handles;
static cs_ras_gattdb_handles_t ras_gattdb_cccd_handles;
static cs_ras_server_t storage[SL_BT_CONFIG_MAX_CONNECTIONS];
static bool gattdb_initialized = false;

// -----------------------------------------------------------------------------
// Internal functions

static sl_status_t cs_ras_server_ranging_data_arrived(uint8_t connection,
                                                      sl_bt_msg_t *evt)
{
  cs_ras_server_t *server;
  sl_status_t sc;

  if (connection == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }

  if (evt == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  server = cs_ras_server_find(connection);
  if (server == NULL) {
    return SL_STATUS_NOT_FOUND;
  }
  cs_ras_mode_t mode;
  bool indication;

  cs_ras_server_get_mode(server, &mode, &indication);
  switch (mode) {
    case CS_RAS_MODE_REAL_TIME_RANGING_DATA:
      sc = cs_ras_server_database_store_result(evt, mode);
      if (sc == SL_STATUS_FULL) {
        server->transmit_request = true;
      }
      break;
    case CS_RAS_MODE_ON_DEMAND_RANGING_DATA:
      (void)cs_ras_server_database_store_result(evt, mode);
      break;
    default:
      return SL_STATUS_INVALID_STATE;
  }
  return SL_STATUS_OK;
}

void cs_ras_server_init(void)
{
  memset(storage, 0, sizeof(storage));
  for (uint8_t i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
    storage[i].connection = SL_BT_INVALID_CONNECTION_HANDLE;
    storage[i].tx_power_dbm = INVALID_POWER_LEVEL_DBM;
    storage[i].antenna_config = INVALID_ANTENNA_CONF;
    app_queue_init(&storage[i].tx_queue,
                   CS_RAS_SERVER_CONFIG_TX_QUEUE_SIZE,
                   sizeof(cs_ras_server_queue_item_t),
                   (uint8_t *)storage[i].tx_queue_data);
  }
  cs_ras_server_messaging_init();
  cs_ras_server_database_init();
}

void cs_ras_server_step(void)
{
  tx_queue_step();
  real_time_transmit_step();
  cs_ras_server_messaging_step();
}

bool cs_ras_server_on_bt_event(sl_bt_msg_t *evt)
{
  cs_ras_server_t *server;
  bool handled = false;
  handled |= !cs_ras_server_messaging_on_bt_event(evt);
  handled |= !cs_ras_server_control_point_on_bt_event(evt);
  bool handled_internal = false;

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_gatt_server_user_read_request_id:
      server = cs_ras_server_find(evt->data.evt_gatt_server_user_read_request.connection);
      if (server == NULL) {
        break;
      }
      handled_internal = handle_read(server,
                                     &evt->data.evt_gatt_server_user_read_request);
      handled |= handled_internal;
      if (handled_internal) {
        break;
      }
      handled_internal = handle_user_cccd_read(server,
                                               &evt->data.evt_gatt_server_user_read_request);
      handled |= handled_internal;
      break;
    case sl_bt_evt_gatt_server_user_write_request_id:
      server = cs_ras_server_find(evt->data.evt_gatt_server_user_write_request.connection);
      if (server == NULL) {
        break;
      }
      handled |= handle_user_cccd_write(server,
                                        &evt->data.evt_gatt_server_user_write_request);
      break;
    case sl_bt_evt_gatt_mtu_exchanged_id:
      server = cs_ras_server_find(evt->data.evt_gatt_mtu_exchanged.connection);
      if (server == NULL) {
        break;
      }
      // Save MTU
      server->att_mtu = evt->data.evt_gatt_mtu_exchanged.mtu;
      cs_ras_server_log_debug(CONN_PREFIX "MTU is %u" LOG_NL,
                              server->connection,
                              server->att_mtu);
      // Send event in NCP case
      handled = false;
      break;
    case sl_bt_evt_connection_opened_id:
      if (!gattdb_initialized) {
        gattdb_initialized = gattdb_init();
        if (!gattdb_initialized) {
          cs_ras_server_gattdb_init_failed();
          break;
        }
      }
      // Find a slot
      server = cs_ras_server_find(SL_BT_INVALID_CONNECTION_HANDLE);
      if (server == NULL) {
        break;
      }
      // Initialize and assign connection handle
      memset(server, 0, sizeof(cs_ras_server_t));
      server->att_mtu = ATT_MTU_MIN;
      server->data_overwritten_counter = CS_RAS_INITIAL_RANGING_COUNTER;
      server->data_ready_counter = CS_RAS_INITIAL_RANGING_COUNTER;
      // Initialize queue
      app_queue_init(&server->tx_queue,
                     CS_RAS_SERVER_CONFIG_TX_QUEUE_SIZE,
                     sizeof(cs_ras_server_queue_item_t),
                     (uint8_t *)server->tx_queue_data);
      server->connection = evt->data.evt_connection_opened.connection;
      // Mode change to default
      cs_ras_server_on_mode_change(server->connection, CS_RAS_MODE_NONE, false);
      cs_ras_server_log_info(CONN_PREFIX "Client is connected." LOG_NL,
                             server->connection);

      // Send event in NCP case
      handled = false;
      break;
    case sl_bt_evt_connection_closed_id:
      server = cs_ras_server_find(evt->data.evt_connection_closed.connection);
      if (server == NULL) {
        break;
      }
      // Stop timers
      (void)app_timer_stop(&server->timer.retention);
      (void)app_timer_stop(&server->timer.response);
      // Cancel any operation
      (void)cs_ras_server_messaging_cancel(&server->transmit);
      cs_ras_server_log_info(CONN_PREFIX "Client disconnected, reason: 0x%04x" LOG_NL,
                             server->connection,
                             evt->data.evt_connection_closed.reason);
      server->connection = SL_BT_INVALID_CONNECTION_HANDLE;
      cs_ras_server_database_clear_connection(evt->data.evt_connection_closed.connection);
      // Send event in NCP case
      handled = false;
      break;
    case sl_bt_evt_cs_result_id:
      server = cs_ras_server_find(evt->data.evt_cs_result.connection);
      if (server == NULL) {
        break;
      }
      cs_ras_server_ranging_data_arrived(evt->data.evt_cs_result.connection, evt);
      handled = true;
      break;
    case sl_bt_evt_cs_result_continue_id:
      server = cs_ras_server_find(evt->data.evt_cs_result_continue.connection);
      if (server == NULL) {
        break;
      }
      cs_ras_server_ranging_data_arrived(evt->data.evt_cs_result_continue.connection, evt);
      handled = true;
      break;
    case sl_bt_evt_cs_procedure_enable_complete_id:
      server = cs_ras_server_find(evt->data.evt_cs_procedure_enable_complete.connection);
      if (server == NULL) {
        break;
      }
      server->tx_power_dbm = evt->data.evt_cs_procedure_enable_complete.tx_power;
      server->antenna_config = evt->data.evt_cs_procedure_enable_complete.antenna_config;
      cs_ras_server_log_info(CONN_PREFIX "CS output power set to %d dBm" LOG_NL,
                             server->connection,
                             server->tx_power_dbm);
      handled = false;
      break;
    default:
      break;
  }
  return !handled;
}

uint16_t cs_ras_server_get_handle(cs_ras_characteristic_index_t index)
{
  if (index < CS_RAS_CHARACTERISTIC_INDEX_COUNT) {
    return ras_gattdb_handles.array[index];
  }
  return CS_RAS_INVALID_CHARACTERISTIC_HANDLE;
}

uint16_t cs_ras_server_get_cccd_handle(cs_ras_characteristic_index_t index)
{
  if (index < CS_RAS_CHARACTERISTIC_INDEX_COUNT
      && index != CS_RAS_CHARACTERISTIC_INDEX_RAS_FEATURES) {
    return ras_gattdb_cccd_handles.array[index];
  }
  return CS_RAS_INVALID_CHARACTERISTIC_HANDLE;
}

cs_ras_server_t *cs_ras_server_find(uint8_t connection)
{
  for (uint8_t i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
    if (storage[i].connection == connection) {
      return &storage[i];
    }
  }
  return NULL;
}

void cs_ras_server_get_mode(cs_ras_server_t *server,
                            cs_ras_mode_t   *mode,
                            bool            *indication)
{
  if (server->cccd.real_time_indication || server->cccd.real_time_notification) {
    if (mode != NULL) {
      *mode = CS_RAS_MODE_REAL_TIME_RANGING_DATA;
    }
    if (indication != NULL) {
      *indication = !server->cccd.real_time_notification;
    }
  } else if (server->cccd.on_demand_indication || server->cccd.on_demand_notification) {
    if (mode != NULL) {
      *mode = CS_RAS_MODE_ON_DEMAND_RANGING_DATA;
    }
    if (indication != NULL) {
      *indication = !server->cccd.on_demand_notification;
    }
  } else {
    if (mode != NULL) {
      *mode = CS_RAS_MODE_NONE;
    }
    if (indication != NULL) {
      *indication = false;
    }
  }
}

static void tx_queue_sent(cs_ras_server_t *server,
                          bool            indication,
                          uint16_t        characteristic,
                          uint8_t         data_size,
                          uint8_t         *data)
{
  (void)indication;
  (void)data_size;

  if (characteristic == cs_ras_server_get_handle(CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT)) {
    cs_ras_cp_response_t *rsp = (cs_ras_cp_response_t *)data;
    switch (rsp->op_code) {
      case CS_RAS_CP_RESPONSE_COMPLETE:
        // Register data as sent.
        cs_ras_server_database_sent(server->connection,
                                    rsp->data.complete_ranging_data);
        // Start timer for data retention
        (void)app_timer_start(&server->timer.retention,
                              CS_RAS_SERVER_CONFIG_DATA_RETENTION_TIMEOUT_MS,
                              retention_timer_raised,
                              (void *)server,
                              false);
        // Start timer for response
        (void)app_timer_start(&server->timer.response,
                              CS_RAS_SERVER_RESPONSE_TIMEOUT_MS,
                              response_timer_raised,
                              (void *)server,
                              false);
        break;
      case CS_RAS_CP_RESPONSE_COMPLETE_LOST_SEGMENT:
        // Start timer for response
        (void)app_timer_start(&server->timer.response,
                              CS_RAS_SERVER_RESPONSE_TIMEOUT_MS,
                              response_timer_raised,
                              (void *)server,
                              false);
        break;
      case CS_RAS_CP_RESPONSE_CODE:
        break;
      default:
        break;
    }
  }
}

static sl_status_t tx_queue_send(cs_ras_server_t *server,
                                 bool            indication,
                                 uint16_t        characteristic,
                                 uint8_t         data_size,
                                 uint8_t         *data)
{
  sl_status_t sc;

  // Send indication or notification
  if (indication) {
    cs_ras_server_log_debug(CONN_PREFIX "TX Queue - Indicating 0x%04x" LOG_NL,
                            server->connection,
                            characteristic);
    sc = sl_bt_gatt_server_send_indication(server->connection,
                                           characteristic,
                                           data_size,
                                           data);
  } else {
    cs_ras_server_log_debug(CONN_PREFIX "TX Queue - Notifying 0x%04x" LOG_NL,
                            server->connection,
                            characteristic);
    sc = sl_bt_gatt_server_send_notification(server->connection,
                                             characteristic,
                                             data_size,
                                             data);
  }
  if (sc == SL_STATUS_OK) {
    // Call action
    tx_queue_sent(server, indication, characteristic, data_size, data);
  }
  return sc;
}

sl_status_t cs_ras_server_send(cs_ras_server_t *server,
                               bool            indication,
                               uint16_t        characteristic,
                               uint8_t         data_size,
                               uint8_t         *data)
{
  sl_status_t sc = SL_STATUS_IN_PROGRESS;

  if (app_queue_is_empty(&server->tx_queue)) {
    // Try to send indication/notification
    sc = tx_queue_send(server, indication, characteristic, data_size, data);
    if (sc == SL_STATUS_OK) {
      cs_ras_server_log_debug(CONN_PREFIX "TX Queue - The %s sent immediately for 0x%04x" LOG_NL,
                              server->connection,
                              indication ? "indication" : "notification",
                              characteristic);
    }
  }
  // If not sent, add to the queue
  if (sc == SL_STATUS_IN_PROGRESS
      || sc == SL_STATUS_NO_MORE_RESOURCE) {
    cs_ras_server_queue_item_t item;
    item.characteristic = characteristic;
    item.indication = indication;
    item.size = data_size;
    memcpy(item.data, data, data_size);
    // Try to add to the queue
    sc = app_queue_add(&server->tx_queue, (uint8_t*)&item);
    if (sc == SL_STATUS_OK) {
      cs_ras_server_log_debug(CONN_PREFIX "TX Queue - The %s added to the queue for 0x%04x" LOG_NL,
                              server->connection,
                              indication ? "indication" : "notification",
                              characteristic);
    } else {
      cs_ras_server_log_error(CONN_PREFIX "TX Queue - Adding %s to the queue failed for 0x%04x, sc = 0x%04lx" LOG_NL,
                              server->connection,
                              indication ? "indication" : "notification",
                              characteristic,
                              (unsigned long)sc);
    }
  } else if (sc != SL_STATUS_OK) {
    cs_ras_server_log_error(CONN_PREFIX "TX Queue - Sending %s failed for 0x%04x, sc = 0x%04lx" LOG_NL,
                            server->connection,
                            indication ? "indication" : "notification",
                            characteristic,
                            (unsigned long)sc);
  }
  return sc;
}

#ifdef SL_CATALOG_POWER_MANAGER_PRESENT

bool cs_ras_server_is_ok_to_sleep(void)
{
  if (!cs_ras_server_messaging_is_ok_to_sleep()) {
    return false;
  }
  return true;
}

sl_power_manager_on_isr_exit_t cs_ras_server_sleep_on_isr_exit(void)
{
  if (!cs_ras_server_messaging_is_ok_to_sleep()) {
    return SL_POWER_MANAGER_WAKEUP;
  }
  return SL_POWER_MANAGER_IGNORE;
}

#endif // SL_CATALOG_POWER_MANAGER_PRESENT

// -----------------------------------------------------------------------------
// Private functions

static uint32_t get_features(void)
{
  uint32_t features = 0;
  #ifdef FEATURE_REAL_TIME_RANGING_DATA
  features |= CS_RAS_FEATURE_RT_RANGING_DATA_MASK;
  #endif // FEATURE_REAL_TIME_RANGING_DATA
  #ifdef FEATURE_RETRIEVE_LOST_SEGMENTS
  features |= CS_RAS_FEATURE_RETRIEVE_LOST_SEGMENT_MASK;
  #endif // FEATURE_RETREUVE_LOST_SEGMENTS
  #ifdef FEATURE_ABORT
  features |= CS_RAS_FEATURE_ABORT_OP_MASK;
  #endif // FEATURE_ABORT
  return features;
}

static bool handle_user_cccd_read(cs_ras_server_t *server,
                                  sl_bt_evt_gatt_server_user_read_request_t *req)
{
  uint16_t sent_len;

  if ((req->characteristic != cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA))
      && (req->characteristic != cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA))
      && (req->characteristic != cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY))
      && (req->characteristic != cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_OVERWRITTEN))
      && (req->characteristic != cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT))) {
    return false;
  }
  if (req->att_opcode != sl_bt_gatt_read_request) {
    (void)sl_bt_gatt_server_send_user_read_response(req->connection,
                                                    req->characteristic,
                                                    ATT_ERR_REQUEST_NOT_SUPPORTED,
                                                    0,
                                                    NULL,
                                                    &sent_len);
    return true;
  }
  if (req->offset >= sizeof(uint16_t)) {
    (void)sl_bt_gatt_server_send_user_read_response(req->connection,
                                                    req->characteristic,
                                                    ATT_ERR_INVALID_OFFSET,
                                                    0,
                                                    NULL,
                                                    &sent_len);
    return true;
  }
  uint16_t cccd_value = 0;
  if (req->characteristic == cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA)) {
    if (server->cccd.real_time_indication
        && server->cccd.real_time_notification) {
      cccd_value = sl_bt_gatt_server_notification_and_indication;
    } else if (server->cccd.real_time_indication) {
      cccd_value = sl_bt_gatt_server_indication;
    } else if (server->cccd.real_time_notification) {
      cccd_value = sl_bt_gatt_server_notification;
    }
  }
  if (req->characteristic == cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA)) {
    if (server->cccd.on_demand_indication
        && server->cccd.on_demand_notification) {
      cccd_value = sl_bt_gatt_server_notification_and_indication;
    } else if (server->cccd.on_demand_indication) {
      cccd_value = sl_bt_gatt_server_indication;
    } else if (server->cccd.on_demand_notification) {
      cccd_value = sl_bt_gatt_server_notification;
    }
  }
  if (req->characteristic == cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY)) {
    if (server->cccd.data_ready_indication
        && server->cccd.data_ready_notification) {
      cccd_value = sl_bt_gatt_server_notification_and_indication;
    } else if (server->cccd.data_ready_indication) {
      cccd_value = sl_bt_gatt_server_indication;
    } else if (server->cccd.data_ready_notification) {
      cccd_value = sl_bt_gatt_server_notification;
    }
  }
  if (req->characteristic == cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_OVERWRITTEN)) {
    if (server->cccd.overwritten_indication
        && server->cccd.overwritten_notification) {
      cccd_value = sl_bt_gatt_server_notification_and_indication;
    } else if (server->cccd.overwritten_indication) {
      cccd_value = sl_bt_gatt_server_indication;
    } else if (server->cccd.overwritten_notification) {
      cccd_value = sl_bt_gatt_server_notification;
    }
  }
  if (req->characteristic == cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT)) {
    if (server->cccd.control_point) {
      cccd_value = sl_bt_gatt_server_indication;
    }
  }
  cs_ras_server_log_debug(CONN_PREFIX "CCCD read for 0x%04x: 0x%04x" LOG_NL,
                          req->connection,
                          req->characteristic,
                          cccd_value);
  (void)sl_bt_gatt_server_send_user_read_response(req->connection,
                                                  req->characteristic,
                                                  ATT_ERR_SUCCESS,
                                                  sizeof(cccd_value) - req->offset,
                                                  &((uint8_t *)&cccd_value)[req->offset],
                                                  &sent_len);
  return true;
}

static bool handle_user_cccd_write(cs_ras_server_t *server,
                                   sl_bt_evt_gatt_server_user_write_request_t *req)
{
  bool indication;
  bool notification;
  uint16_t status;
  bool handled = true;
  cs_ras_mode_t mode;

  if ((req->characteristic != cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA))
      && (req->characteristic != cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA))
      && (req->characteristic != cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY))
      && (req->characteristic != cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_OVERWRITTEN))
      && (req->characteristic != cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT))) {
    return false;
  }

  // Check for writes
  if (req->att_opcode != sl_bt_gatt_write_request) {
    (void)sl_bt_gatt_server_send_user_write_response(req->connection,
                                                     req->characteristic,
                                                     ATT_ERR_REQUEST_NOT_SUPPORTED);
    return true;
  }
  if (req->value.len != 2) {
    (void)sl_bt_gatt_server_send_user_write_response(req->connection,
                                                     req->characteristic,
                                                     ATT_ERR_INVALID_ATT_LENGTH);
    return true;
  }
  if (req->offset != 0) {
    (void)sl_bt_gatt_server_send_user_write_response(req->connection,
                                                     req->characteristic,
                                                     ATT_ERR_INVALID_OFFSET);
    return true;
  }
  // Parse CCCD value
  status = *((uint16_t *)req->value.data);
  indication = INDICATION(status);
  notification = NOTIFICATION(status);

  if (req->characteristic
      == cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA)) {
    if ((server->cccd.on_demand_indication || server->cccd.on_demand_notification)
        && (status != sl_bt_gatt_disable)) {
      (void)sl_bt_gatt_server_send_user_write_response(req->connection,
                                                       req->characteristic,
                                                       ATT_ERR_CCCD_IMPROPERLY_CONFIGURED);
      cs_ras_server_log_warning(CONN_PREFIX "CCCD Real-Time - Improper CCCD configuration requested" LOG_NL,
                                server->connection);
      return true;
    }
    // Check for switching off
    if ((!indication && !notification)
        && (server->cccd.real_time_indication
            || server->cccd.real_time_notification)) {
      // Cancel ongoing transfer if present
      (void)cs_ras_server_messaging_cancel(&server->transmit);
    }
    server->cccd.real_time_indication = indication;
    server->cccd.real_time_notification = notification;
    cs_ras_server_log_debug(CONN_PREFIX "CCCD Real-Time - Indication: %u Notification: %u" LOG_NL,
                            server->connection,
                            server->cccd.real_time_indication,
                            server->cccd.real_time_notification);
    cs_ras_server_get_mode(server, &mode, &indication);
    cs_ras_server_on_mode_change(server->connection, mode, indication);
  } else if (req->characteristic
             ==  cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA)) {
    if (server->cccd.real_time_indication || server->cccd.real_time_notification) {
      (void)sl_bt_gatt_server_send_user_write_response(req->connection,
                                                       req->characteristic,
                                                       ATT_ERR_CCCD_IMPROPERLY_CONFIGURED);
      cs_ras_server_log_warning(CONN_PREFIX "CCCD On-Demand - Improper CCCD configuration requested" LOG_NL,
                                server->connection);
      return true;
    }
    // Check for switching off
    if ((!indication && !notification)
        && (server->cccd.on_demand_indication
            || server->cccd.on_demand_notification)) {
      // Cancel ongoing transfer if present
      (void)cs_ras_server_messaging_cancel(&server->transmit);
    }
    server->cccd.on_demand_indication = indication;
    server->cccd.on_demand_notification = notification;
    cs_ras_server_log_debug(CONN_PREFIX "CCCD On-Demand - Indication: %u Notification: %u" LOG_NL,
                            server->connection,
                            server->cccd.on_demand_indication,
                            server->cccd.on_demand_notification);
    cs_ras_server_get_mode(server, &mode, &indication);
    cs_ras_server_on_mode_change(server->connection, mode, indication);
  } else if (req->characteristic
             == cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY)) {
    #ifndef FEATURE_DATA_READY_NOTIFICATIONS
    if (notification) {
      (void)sl_bt_gatt_server_send_user_write_response(req->connection,
                                                       req->characteristic,
                                                       ATT_ERR_REQUEST_REJECTED);
      return true;
    }
    #endif
    server->cccd.data_ready_notification = notification;
    server->cccd.data_ready_indication = indication;
    cs_ras_server_log_debug(CONN_PREFIX "CCCD Data Ready - Indication: %u Notification: %u" LOG_NL,
                            server->connection,
                            server->cccd.data_ready_indication,
                            server->cccd.data_ready_notification);
  } else if (req->characteristic
             == cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_OVERWRITTEN)) {
    #ifndef FEATURE_DATA_OVERWRITTEN_NOTIFICATIONS
    if (notification) {
      (void)sl_bt_gatt_server_send_user_write_response(req->connection,
                                                       req->characteristic,
                                                       ATT_ERR_REQUEST_REJECTED);
      return true;
    }
    #endif
    server->cccd.overwritten_indication = indication;
    server->cccd.overwritten_notification = notification;
    cs_ras_server_log_debug(CONN_PREFIX "CCCD Overwritten - Indication: %u Notification: %u" LOG_NL,
                            server->connection,
                            server->cccd.overwritten_indication,
                            server->cccd.overwritten_notification);
  } else if (req->characteristic
             == cs_ras_server_get_cccd_handle(CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT)) {
    server->cccd.control_point = indication;
    cs_ras_server_log_debug(CONN_PREFIX "CCCD CP - Indication: %u" LOG_NL,
                            server->connection,
                            server->cccd.control_point);
  } else {
    handled = false;
    return handled;
  }
  (void)sl_bt_gatt_server_send_user_write_response(req->connection,
                                                   req->characteristic,
                                                   ATT_ERR_SUCCESS);
  return handled;
}

static bool handle_read(cs_ras_server_t *server,
                        sl_bt_evt_gatt_server_user_read_request_t *req)
{
  uint32_t features;
  uint16_t sent_len;
  bool handled = false;

  if (req->att_opcode != sl_bt_gatt_read_request) {
    cs_ras_server_log_debug(CONN_PREFIX "Read RAS characteristic - Invalid ATT opcode %u" LOG_NL,
                            req->connection,
                            req->att_opcode);
    (void)sl_bt_gatt_server_send_user_read_response(server->connection,
                                                    req->characteristic,
                                                    ATT_ERR_REQUEST_NOT_SUPPORTED,
                                                    0,
                                                    NULL,
                                                    NULL);
    return true;
  }
  // RAS features
  if (req->characteristic
      == cs_ras_server_get_handle(CS_RAS_CHARACTERISTIC_INDEX_RAS_FEATURES)) {
    if (req->offset >= sizeof(features)) {
      cs_ras_server_log_debug(CONN_PREFIX "Read RAS Features - Invalid offset %u" LOG_NL,
                              req->connection,
                              req->offset);
      (void)sl_bt_gatt_server_send_user_read_response(server->connection,
                                                      req->characteristic,
                                                      ATT_ERR_INVALID_OFFSET,
                                                      0,
                                                      NULL,
                                                      NULL);
      handled = true;
      return handled;
    }
    features = get_features();
    (void)sl_bt_gatt_server_send_user_read_response(server->connection,
                                                    req->characteristic,
                                                    ATT_ERR_SUCCESS,
                                                    sizeof(features) - req->offset,
                                                    &(((uint8_t *)&features)[req->offset]),
                                                    &sent_len);
    cs_ras_server_log_info(CONN_PREFIX "RAS features sent: 0x%08lx" LOG_NL,
                           req->connection,
                           (unsigned long)features);
    handled = true;
  } else if (req->characteristic
             == cs_ras_server_get_handle(CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY)) {
    #ifdef FEATURE_DATA_READY_READ
    if (req->offset >= sizeof(server->data_ready_counter)) {
      cs_ras_server_log_debug(CONN_PREFIX "Read RAS Data Ready - Invalid offset %u" LOG_NL,
                              req->connection,
                              req->offset);
      (void)sl_bt_gatt_server_send_user_read_response(server->connection,
                                                      req->characteristic,
                                                      ATT_ERR_INVALID_OFFSET,
                                                      0,
                                                      NULL,
                                                      NULL);
      handled = true;
      return handled;
    }
    (void)sl_bt_gatt_server_send_user_read_response(server->connection,
                                                    req->characteristic,
                                                    ATT_ERR_SUCCESS,
                                                    sizeof(server->data_ready_counter) - req->offset,
                                                    &(((uint8_t *)&server->data_ready_counter)[req->offset]),
                                                    &sent_len);
    #else // FEATURE_DATA_READY_READ
    (void)sl_bt_gatt_server_send_user_read_response(server->connection,
                                                    req->characteristic,
                                                    ATT_ERR_REQUEST_NOT_SUPPORTED,
                                                    0,
                                                    NULL,
                                                    &sent_len);
    #endif // FEATURE_DATA_READY_READ
  } else if (req->characteristic
             == cs_ras_server_get_handle(CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_OVERWRITTEN)) {
    #ifdef FEATURE_DATA_OVERWRITTEN_READ
    if (req->offset >= sizeof(server->data_overwritten_counter)) {
      cs_ras_server_log_debug(CONN_PREFIX "Read RAS Data Overwritten - Invalid offset %u" LOG_NL,
                              req->connection,
                              req->offset);
      (void)sl_bt_gatt_server_send_user_read_response(server->connection,
                                                      req->characteristic,
                                                      ATT_ERR_INVALID_OFFSET,
                                                      0,
                                                      NULL,
                                                      NULL);
      handled = true;
      return handled;
    }
    (void)sl_bt_gatt_server_send_user_read_response(server->connection,
                                                    req->characteristic,
                                                    ATT_ERR_SUCCESS,
                                                    sizeof(server->data_overwritten_counter) - req->offset,
                                                    &(((uint8_t *)&server->data_overwritten_counter)[req->offset]),
                                                    &sent_len);
    #else // FEATURE_DATA_OVERWRITTEN_READ
    (void)sl_bt_gatt_server_send_user_read_response(server->connection,
                                                    req->characteristic,
                                                    ATT_ERR_REQUEST_NOT_SUPPORTED,
                                                    0,
                                                    NULL,
                                                    &sent_len);
    #endif // FEATURE_DATA_OVERWRITTEN_READ
  }
  return handled;
}

bool cs_ras_server_get_ranging_data(cs_ras_server_t           *server,
                                    cs_ras_ranging_counter_t  counter,
                                    bool                      *sent,
                                    uint8_t                   **data,
                                    uint32_t                  *size)
{
  bool has_data = false;

  cs_procedure_data_t *procedure;

  procedure = cs_ras_server_database_find_next_slot(server->connection, counter);

  if (procedure != NULL) {
    if (data != NULL) {
      *data = procedure->data;
    }
    if (size != NULL) {
      *size = procedure->data_len;
    }
    if (sent != NULL) {
      *sent = procedure->sent;
    }
    has_data = true;
  }

  return has_data;
}

bool cs_ras_server_delete_ranging_data(cs_ras_server_t           *server,
                                       cs_ras_ranging_counter_t  counter)
{
  bool found = cs_ras_server_get_ranging_data(server,
                                              counter,
                                              NULL,
                                              NULL,
                                              NULL);
  if (!found) {
    return false;
  }

  cs_ras_server_database_clear(server->connection, counter);

  return true;
}

sl_status_t cs_ras_send_data_ready(uint8_t connection,
                                   cs_ras_ranging_counter_t ranging_counter)
{
  cs_ras_server_t *server = cs_ras_server_find(connection);
  sl_status_t sc;
  if (server == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (!server->cccd.on_demand_indication
      && !server->cccd.on_demand_notification) {
    return SL_STATUS_INVALID_STATE;
  }
  server->data_ready_counter = ranging_counter;
  sc = cs_ras_server_send(server,
                          server->cccd.data_ready_indication,
                          cs_ras_server_get_handle(CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY),
                          sizeof(ranging_counter),
                          (uint8_t *)&ranging_counter);
  return sc;
}

sl_status_t cs_ras_send_overwritten(uint8_t connection,
                                    cs_ras_ranging_counter_t ranging_counter)
{
  cs_ras_server_t *server = cs_ras_server_find(connection);
  sl_status_t sc;
  if (server == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (!server->cccd.on_demand_indication
      && !server->cccd.on_demand_notification) {
    return SL_STATUS_INVALID_STATE;
  }
  server->data_overwritten_counter = ranging_counter;
  sc = cs_ras_server_send(server,
                          server->cccd.overwritten_indication,
                          cs_ras_server_get_handle(CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_OVERWRITTEN),
                          sizeof(ranging_counter),
                          (uint8_t *)&ranging_counter);
  return sc;
}

static void real_time_transmit_step(void)
{
  sl_status_t sc;
  cs_ras_mode_t mode;
  bool indication;
  uint8_t *ranging_data;
  uint32_t ranging_data_size;
  bool sent;

  for (uint8_t i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
    if (storage[i].transmit_request) {
      cs_ras_server_t *server = &storage[i];
      cs_ras_server_get_mode(server, &mode, &indication);
      if (mode != CS_RAS_MODE_REAL_TIME_RANGING_DATA) {
        storage[i].transmit_request = false;
        cs_ras_server_log_warning(CONN_PREFIX RC_PREFIX "Real-Time transmission requested in invalid state" LOG_NL,
                                  server->connection,
                                  server->ranging_counter);
        continue;
      }

      // Get assigned ranging counter
      bool has_ranging_data = cs_ras_server_get_ranging_data(server,
                                                             CS_RAS_INVALID_RANGING_COUNTER,
                                                             &sent,
                                                             &ranging_data,
                                                             &ranging_data_size);
      if (!has_ranging_data) {
        storage[i].transmit_request = false;
        cs_ras_server_log_warning(CONN_PREFIX RC_PREFIX "No ranging data found for Real-Time transmit." LOG_NL,
                                  server->connection,
                                  server->ranging_counter);
        continue;
      }

      cs_ras_messaging_config_t config;
      config.conn_handle = server->connection;
      config.att_mtu = server->att_mtu;
      config.real_time = true;
      config.indication = indication;
      config.data = ranging_data;
      config.data_size = ranging_data_size;
      config.start_segment = 0;
      config.end_segment = CS_RAS_MESSAGING_ALL_SEGMENTS;

      sc = cs_ras_server_messaging_transmit_all(&server->transmit, &config);
      if (sc != SL_STATUS_OK) {
        cs_ras_server_log_error(CONN_PREFIX RC_PREFIX "Failed to start sending Real-Time Ranging Data, sc = 0x%04lx" LOG_NL,
                                server->connection,
                                server->ranging_counter,
                                (unsigned long)sc);
      } else {
        cs_ras_server_log_debug(CONN_PREFIX RC_PREFIX "Sending Real-Time Ranging Data of size %lu" LOG_NL,
                                server->connection,
                                server->ranging_counter,
                                (unsigned long)ranging_data_size);
      }
      // Clean flag
      server->transmit_request = false;
    }
  }
}

static void tx_queue_step(void)
{
  sl_status_t sc;
  cs_ras_server_queue_item_t item;
  for (uint8_t i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
    cs_ras_server_t *server = &storage[i];
    bool get_next;
    do {
      get_next = false;
      sc = app_queue_peek(&server->tx_queue, (uint8_t *)&item);
      if (sc == SL_STATUS_OK) {
        // Try to send indication/notification
        sc = tx_queue_send(server,
                           item.indication,
                           item.characteristic,
                           item.size,
                           item.data);
        if (sc == SL_STATUS_OK) {
          // Sent out, move on
          (void)app_queue_remove(&server->tx_queue, (uint8_t *)&item);
          get_next = true;
          cs_ras_server_log_debug(CONN_PREFIX "TX Queue - The %s sent immediately for 0x%04x" LOG_NL,
                                  server->connection,
                                  item.indication ? "indication" : "notification",
                                  item.characteristic);
        } else if (sc == SL_STATUS_IN_PROGRESS
                   || sc == SL_STATUS_NO_MORE_RESOURCE) {
          // Try to send it later
          get_next = false;
        } else {
          cs_ras_server_log_error(CONN_PREFIX "TX Queue - Sending %s failed for 0x%04x, sc = 0x%04lx" LOG_NL,
                                  server->connection,
                                  item.indication ? "indication" : "notification",
                                  item.characteristic,
                                  (unsigned long)sc);
          // Other error, move on
          (void)app_queue_remove(&server->tx_queue, (uint8_t *)&item);
          get_next = true;
        }
      }
    } while (get_next);
  }
}

static void retention_timer_raised(app_timer_t *timer, void *data)
{
  (void)timer;
  cs_ras_server_t *server = (cs_ras_server_t *)data;
  cs_ras_server_log_warning(CONN_PREFIX "Procedure %u - retention timeout. Deleting data." LOG_NL,
                            server->connection,
                            server->ranging_counter);
  // Delete ranging data
  cs_ras_server_delete_ranging_data(server,
                                    server->ranging_counter);
  server->ranging_counter = CS_RAS_INVALID_RANGING_COUNTER;
}

static void response_timer_raised(app_timer_t *timer, void *data)
{
  (void)timer;
  cs_ras_server_t *server = (cs_ras_server_t *)data;
  (void)server;
  cs_ras_server_log_warning(CONN_PREFIX "Procedure %u - transfer procedure timed out." LOG_NL,
                            server->connection,
                            server->ranging_counter);
}

static bool gattdb_init(void)
{
  sl_status_t sc;
  bool ret = true;
  uint16_t cccd_uuid = CCCD_UUID;
  for (uint8_t i = 0; i < CS_RAS_CHARACTERISTIC_INDEX_COUNT; i++) {
    ras_gattdb_handles.array[i] = CS_RAS_INVALID_CHARACTERISTIC_HANDLE;
    ras_gattdb_cccd_handles.array[i] = CS_RAS_INVALID_CHARACTERISTIC_HANDLE;
    sc = sl_bt_gatt_server_find_attribute(0u,
                                          sizeof(ras_uuids[i]),
                                          (uint8_t *)&ras_uuids[i],
                                          &ras_gattdb_handles.array[i]);
    if (sc != SL_STATUS_OK) {
      ret = false;
      cs_ras_server_log_error("RAS characteristic %u was not found." LOG_NL, i);
    } else if (i != CS_RAS_CHARACTERISTIC_INDEX_RAS_FEATURES) {
      // Get CCCD for mandatory ones.
      sc = sl_bt_gatt_server_find_attribute(ras_gattdb_handles.array[i],
                                            sizeof(cccd_uuid),
                                            (uint8_t *)&cccd_uuid,
                                            &ras_gattdb_cccd_handles.array[i]);
      if (sc != SL_STATUS_OK) {
        ret = false;
        cs_ras_server_log_error("RAS CCCD %u was not found." LOG_NL, i);
      }
    }
  }
  return ret;
}

// -----------------------------------------------------------------------------
// Weak implementation of callbacks

SL_WEAK void cs_ras_server_on_mode_change(uint8_t       connection,
                                          cs_ras_mode_t mode,
                                          bool          indication)
{
  (void)connection;
  (void)mode;
  (void)indication;
}

SL_WEAK void cs_ras_server_gattdb_init_failed()
{
}
