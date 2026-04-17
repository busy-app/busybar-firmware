/***************************************************************************//**
 * @file
 * @brief CS RAS Server - RAS Control Point implementation
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
#include "cs_ras_server_control_point.h"
#include "cs_ras_server_log.h"
#include "sl_status.h"
#include "sl_common.h"
#include "sl_bt_api.h"
#include "cs_ras_common.h"
#include "cs_ras_server_internal.h"
#include "cs_ras_server_messaging.h"
#include "cs_ras_server_messaging_internal.h"
#include "cs_ras_server_database.h"

// -----------------------------------------------------------------------------
// Forward declaration of private functions

// Send general response to the Control Point
static void send_response(cs_ras_server_t *server,
                          uint8_t         data_size,
                          uint8_t         *data);
// Send a response code for a Control Point request
static void send_response_code(cs_ras_server_t *server,
                               cs_ras_cp_response_code_value_t value);
// Handle Abort opcode
static void handle_abort(cs_ras_server_t *server);
// Handle Retrieve Lost Segments opcode
static void handle_retreive_lost_segments(cs_ras_server_t *server,
                                          cs_ras_cp_retreive_lost_t *data);
// Handle Get Ranging Data opcode
static void handle_get(cs_ras_server_t              *server,
                       cs_ras_cp_get_ranging_data_t *data);
// Handle Ack opcode
static void handle_ack(cs_ras_server_t              *server,
                       cs_ras_cp_ack_ranging_data_t *data);

// -----------------------------------------------------------------------------
// Public functions

bool cs_ras_server_control_point_on_bt_event(sl_bt_msg_t *evt)
{
  bool handled = false;
  cs_ras_cp_command_t *cmd;
  cs_ras_server_t *server;

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_gatt_server_user_write_request_id:
      server = cs_ras_server_find(evt->data.evt_gatt_server_user_write_request.connection);
      if (server == NULL
          || (evt->data.evt_gatt_server_user_write_request.characteristic
              != cs_ras_server_get_handle(CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT))) {
        break;
      }
      // Only write without response is supported
      if (evt->data.evt_gatt_server_user_write_request.att_opcode
          != sl_bt_gatt_write_command) {
        (void)sl_bt_gatt_server_send_user_write_response(evt->data.evt_gatt_server_user_write_request.connection,
                                                         evt->data.evt_gatt_server_user_write_request.characteristic,
                                                         ATT_ERR_REQUEST_REJECTED);
        handled = true;
        break;
      }
      if (evt->data.evt_gatt_server_user_write_request.offset != 0) {
        send_response_code(server,
                           CS_RAS_CP_RESPONSE_CODE_INVALID_PARAMETER);
        handled = true;
        break;
      }
      cmd = (cs_ras_cp_command_t *)evt->data.evt_gatt_server_user_write_request.value.data;
      cs_ras_server_log_debug(CONN_PREFIX "Opcode 0x%02x" LOG_NL,
                              evt->data.evt_gatt_server_user_write_request.connection,
                              cmd->op_code);
      switch (cmd->op_code) {
        case CS_RAS_CP_OPCODE_GET:
          // Check size
          if (evt->data.evt_gatt_server_user_write_request.value.len
              != sizeof(cs_ras_cp_opcode_t) + sizeof(cs_ras_cp_get_ranging_data_t)) {
            send_response_code(server,
                               CS_RAS_CP_RESPONSE_CODE_INVALID_PARAMETER);
            break;
          }
          // Handle get ranging data
          handle_get(server,
                     &cmd->data.get_ranging_data);
          break;
        case CS_RAS_CP_OPCODE_ACK:
          // Check size
          if (evt->data.evt_gatt_server_user_write_request.value.len
              != sizeof(cs_ras_cp_opcode_t) + sizeof(cs_ras_cp_ack_ranging_data_t)) {
            send_response_code(server,
                               CS_RAS_CP_RESPONSE_CODE_INVALID_PARAMETER);
            break;
          }
          // Handle ack ranging data
          handle_ack(server,
                     &cmd->data.ack_ranging_data);
          break;
        case CS_RAS_CP_OPCODE_RETRIEVE_LOST_SEGMENTS:
          if (evt->data.evt_gatt_server_user_write_request.value.len
              != sizeof(cs_ras_cp_opcode_t) + sizeof(cs_ras_cp_retreive_lost_t)) {
            send_response_code(server,
                               CS_RAS_CP_RESPONSE_CODE_INVALID_PARAMETER);
            break;
          }
          // Handle retreive lost segments
          handle_retreive_lost_segments(server,
                                        &cmd->data.retreive_lost);
          break;
        case CS_RAS_CP_OPCODE_ABORT:
          // Check size
          if (evt->data.evt_gatt_server_user_write_request.value.len
              != sizeof(cs_ras_cp_opcode_t)) {
            send_response_code(server,
                               CS_RAS_CP_RESPONSE_CODE_INVALID_PARAMETER);
            break;
          }
          handle_abort(server);
          break;
        case CS_RAS_CP_OPCODE_SET_FILTER:
          // Not supported
          send_response_code(server,
                             CS_RAS_CP_RESPONSE_CODE_NOT_SUPPORTED);
          break;
        default:
          // Not supported
          send_response_code(server,
                             CS_RAS_CP_RESPONSE_CODE_NOT_SUPPORTED);
          break;
      }
      handled = true;
      break;
    default:
      break;
  }

  return !handled;
}

// -----------------------------------------------------------------------------
// Private functions

static void handle_get(cs_ras_server_t              *server,
                       cs_ras_cp_get_ranging_data_t *data)
{
  sl_status_t sc;
  cs_ras_mode_t mode;
  bool indication;
  uint8_t *ranging_data;
  uint32_t ranging_data_size;

  cs_ras_server_log_debug(CONN_PREFIX "Get requested for %u" LOG_NL,
                          server->connection,
                          data->ranging_counter);
  if (server->state == SERVER_STATE_IN_CP_PROCEDURE) {
    send_response_code(server,
                       CS_RAS_CP_RESPONSE_CODE_SERVER_BUSY);
    return;
  }

  cs_ras_server_get_mode(server, &mode, &indication);
  // Real time or nothing selected
  if (mode != CS_RAS_MODE_ON_DEMAND_RANGING_DATA) {
    send_response_code(server,
                       CS_RAS_CP_RESPONSE_CODE_PROCEDURE_NOT_COMPLETED);
    return;
  }

  bool sent;
  bool has_ranging_data = cs_ras_server_get_ranging_data(server,
                                                         data->ranging_counter,
                                                         &sent,
                                                         &ranging_data,
                                                         &ranging_data_size);
  if (!has_ranging_data) {
    send_response_code(server,
                       CS_RAS_CP_RESPONSE_CODE_NO_RECORDS_FOUND);
    return;
  }

  cs_ras_messaging_config_t config;
  config.conn_handle = server->connection;
  config.att_mtu = server->att_mtu;
  config.real_time = false;
  config.indication = indication;
  config.data = ranging_data;
  config.data_size = ranging_data_size;

  sc = cs_ras_server_messaging_transmit_all(&server->transmit, &config);

  if (sc == SL_STATUS_OK) {
    server->ranging_counter = data->ranging_counter;
    server->op_code = CS_RAS_CP_OPCODE_GET;
    server->state = SERVER_STATE_IN_CP_PROCEDURE;
  } else {
    send_response_code(server,
                       CS_RAS_CP_RESPONSE_CODE_PROCEDURE_NOT_COMPLETED);
  }
}

static void handle_ack(cs_ras_server_t              *server,
                       cs_ras_cp_ack_ranging_data_t *data)
{
  cs_ras_server_log_debug(CONN_PREFIX "Ack requested for %u" LOG_NL,
                          server->connection,
                          data->ranging_counter);
  if (server->state == SERVER_STATE_IN_CP_PROCEDURE) {
    send_response_code(server,
                       CS_RAS_CP_RESPONSE_CODE_SERVER_BUSY);
    return;
  }
  if (!cs_ras_server_delete_ranging_data(server, data->ranging_counter)) {
    send_response_code(server,
                       CS_RAS_CP_RESPONSE_CODE_NO_RECORDS_FOUND);
  } else {
    // Stop timers
    (void)app_timer_stop(&server->timer.retention);
    (void)app_timer_stop(&server->timer.response);
    send_response_code(server,
                       CS_RAS_CP_RESPONSE_CODE_SUCCESS);
  }
}

static void handle_retreive_lost_segments(cs_ras_server_t           *server,
                                          cs_ras_cp_retreive_lost_t *data)
{
#ifdef FEATURE_RETRIEVE_LOST_SEGMENTS
  sl_status_t sc;
  cs_ras_mode_t mode;
  bool indication;
  uint8_t *ranging_data;
  uint32_t ranging_data_size;
  cs_ras_server_log_debug(CONN_PREFIX RC_PREFIX "Retrieve lost segments requested, segments: %u -> %u" LOG_NL,
                          server->connection,
                          data->ranging_counter,
                          data->start_segment,
                          data->end_segment);

  // Ignore new retransmission request while one is in progress.
  if (server->state == SERVER_STATE_IN_CP_PROCEDURE) {
    send_response_code(server,
                       CS_RAS_CP_RESPONSE_CODE_SERVER_BUSY);
    return;
  }

  cs_ras_server_get_mode(server, &mode, &indication);
  // Real time or nothing selected
  if (mode != CS_RAS_MODE_ON_DEMAND_RANGING_DATA) {
    send_response_code(server,
                       CS_RAS_CP_RESPONSE_CODE_PROCEDURE_NOT_COMPLETED);
    return;
  }

  bool sent;
  bool has_ranging_data = cs_ras_server_get_ranging_data(server,
                                                         data->ranging_counter,
                                                         &sent,
                                                         &ranging_data,
                                                         &ranging_data_size);
  if (!has_ranging_data) {
    // Requested data not found.
    send_response_code(server,
                       CS_RAS_CP_RESPONSE_CODE_NO_RECORDS_FOUND);
    return;
  }

  if (!sent) {
    // Has not been sent yet.
    send_response_code(server,
                       CS_RAS_CP_RESPONSE_CODE_INVALID_PARAMETER);
    return;
  }

  if (data->start_segment > data->end_segment) {
    // Invalid segment parameters
    send_response_code(server,
                       CS_RAS_CP_RESPONSE_CODE_INVALID_PARAMETER);
    return;
  }

  cs_ras_messaging_config_t config;
  config.conn_handle = server->connection;
  config.att_mtu = server->att_mtu;
  config.real_time = false;
  config.indication = indication;
  config.data = ranging_data;
  config.data_size = ranging_data_size;
  config.start_segment = data->start_segment;
  config.end_segment = data->end_segment;

  sc = cs_ras_server_messaging_transmit(&server->transmit, &config);

  if (sc == SL_STATUS_OK) {
    // Stop timer
    (void)app_timer_stop(&server->timer.retention);

    server->ranging_counter = data->ranging_counter;
    server->op_code = CS_RAS_CP_OPCODE_RETRIEVE_LOST_SEGMENTS;
    server->state = SERVER_STATE_IN_CP_PROCEDURE;
  } else {
    if (sc == SL_STATUS_NOT_FOUND) {
      // Requested segments not found
      send_response_code(server,
                         CS_RAS_CP_RESPONSE_CODE_NO_RECORDS_FOUND);
    } else {
      send_response_code(server,
                         CS_RAS_CP_RESPONSE_CODE_PROCEDURE_NOT_COMPLETED);
    }
  }

#else // FEATURE_RETREUVE_LOST_SEGMENTS
  (void)data;
  send_response_code(server, CS_RAS_CP_RESPONSE_CODE_NOT_SUPPORTED);
#endif // FEATURE_RETREUVE_LOST_SEGMENTS
}

static void handle_abort(cs_ras_server_t *server)
{
  (void)server;
#ifdef FEATURE_ABORT
  cs_ras_server_log_debug(CONN_PREFIX "Abort requested" LOG_NL,
                          server->connection);
  if (server->state == SERVER_STATE_IN_CP_PROCEDURE) {
    // Flush all ranging data belongs to the current operation/counter
    (void)cs_ras_server_messaging_cancel(&server->transmit);
    server->state = SERVER_STATE_IDLE;
    send_response_code(server, CS_RAS_CP_RESPONSE_CODE_SUCCESS);
  } else {
    send_response_code(server, CS_RAS_CP_RESPONSE_CODE_ABORT_UNSUCCESSFUL);
  }
#else // FEATURE_ABORT
  send_response_code(server, CS_RAS_CP_RESPONSE_CODE_NOT_SUPPORTED);
#endif // FEATURE_ABORT
}

static void send_response_code(cs_ras_server_t                 *server,
                               cs_ras_cp_response_code_value_t value)
{
  cs_ras_cp_response_t rsp;
  rsp.op_code = CS_RAS_CP_RESPONSE_CODE;
  rsp.data.response_code.value = value;
  send_response(server,
                sizeof(cs_ras_cp_response_opcode_t) + sizeof(cs_ras_cp_response_code_t),
                (uint8_t *)&rsp);
  if (value != CS_RAS_CP_RESPONSE_CODE_SUCCESS) {
    cs_ras_server_log_warning(CONN_PREFIX "Response code is 0x%02x" LOG_NL,
                              server->connection,
                              value);
  }
}

static void send_response(cs_ras_server_t *server,
                          uint8_t         data_size,
                          uint8_t         *data)
{
  (void)cs_ras_server_send(server,
                           true,
                           cs_ras_server_get_handle(CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT),
                           data_size,
                           data);
}

// -----------------------------------------------------------------------------
// Weak implementation of callback functions

// Transmission of Ranging Data has been finished
void cs_ras_server_messaging_on_transmit_finished(cs_ras_server_messaging_transmit_t *tx,
                                                  cs_ras_messaging_status_t          status,
                                                  sl_status_t                        sc)
{
  (void)sc;

  cs_ras_server_t *server;
  server = cs_ras_server_find(tx->config.conn_handle);
  if (server == NULL) {
    cs_ras_server_log_error("Transmit finished, server not found for connection %u" LOG_NL,
                            tx->config.conn_handle);
    return;
  }
  // Real-Time transmit finished
  if (tx->config.real_time) {
    if (status == CS_RAS_MESSAGING_STATUS_SUCCESS) {
      cs_ras_server_log_info(CONN_PREFIX RC_PREFIX "Sending Real-time data succeeded." LOG_NL,
                             server->connection,
                             server->ranging_counter);
    } else {
      cs_ras_server_log_warning(CONN_PREFIX RC_PREFIX "Sending Real-time data failed, Status = %u, sc = 0x%04lx." LOG_NL,
                                server->connection,
                                server->ranging_counter,
                                status,
                                (unsigned long)sc);
    }
    cs_ras_server_database_clear(server->connection, CS_RAS_INVALID_RANGING_COUNTER);
    return;
  }
  // On-Demand transmit finished
  if (status == CS_RAS_MESSAGING_STATUS_SUCCESS) {
    cs_ras_cp_response_t response;
    uint16_t size = sizeof(cs_ras_cp_response_opcode_t);
    if (server->state == SERVER_STATE_IN_CP_PROCEDURE) {
      if (server->op_code == CS_RAS_CP_OPCODE_GET) {
        response.op_code = CS_RAS_CP_RESPONSE_COMPLETE;
        response.data.complete_ranging_data = server->ranging_counter;
        size += sizeof(cs_ras_cp_complete_response_t);
        send_response(server, size, (uint8_t *)&response);
        server->state = SERVER_STATE_IDLE;
        cs_ras_server_log_debug(CONN_PREFIX RC_PREFIX "Get succeeded." LOG_NL,
                                server->connection,
                                server->ranging_counter);
      } else if (server->op_code == CS_RAS_CP_OPCODE_RETRIEVE_LOST_SEGMENTS) {
        response.op_code = CS_RAS_CP_RESPONSE_COMPLETE_LOST_SEGMENT;
        response.data.complete_lost_segment.ranging_counter = server->ranging_counter;
        response.data.complete_lost_segment.start_segment = tx->config.start_segment;
        response.data.complete_lost_segment.end_segment = tx->config.end_segment;
        size += sizeof(cs_ras_cp_complete_lost_response_t);
        send_response(server, size, (uint8_t *)&response);
        server->state = SERVER_STATE_IDLE;
        cs_ras_server_log_debug(CONN_PREFIX RC_PREFIX "Retrieve lost segments succeeded." LOG_NL,
                                server->connection,
                                server->ranging_counter);
      } else {
        cs_ras_server_log_error(CONN_PREFIX "Transfer finished for invalid opcode %d" LOG_NL,
                                server->connection,
                                server->op_code);
      }
    } else {
      cs_ras_server_log_error(CONN_PREFIX "Transmit state error, not in progress" LOG_NL,
                              server->connection);
    }
  } else if (status == CS_RAS_MESSAGING_STATUS_FAILURE
             || status == CS_RAS_MESSAGING_STATUS_TIMEOUT) {
    server->state = SERVER_STATE_IDLE;
    send_response_code(server,
                       CS_RAS_CP_RESPONSE_CODE_PROCEDURE_NOT_COMPLETED);
    cs_ras_server_log_error(CONN_PREFIX RC_PREFIX "Sending failed. Status = %u, sc = 0x%04lx" LOG_NL,
                            server->connection,
                            server->ranging_counter,
                            status,
                            (unsigned long)sc);
  }
}
