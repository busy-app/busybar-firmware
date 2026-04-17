/***************************************************************************//**
 * @file
 * @brief CS RAS Client - RAS Control Point implementation
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
#include "cs_ras_client_control_point.h"
#include "cs_ras_client_log.h"
#include "cs_ras_client.h"
#include "cs_ras_client_internal.h"
#include "cs_ras_client_config.h"
#include "cs_ras_client_messaging.h"
#include "app_timer.h"

// -----------------------------------------------------------------------------
// Definitions

#define CP_PREFIX "CP - "

// -----------------------------------------------------------------------------
// Forward declaration of privare functions

static void handle_complete(cs_ras_client_t          *client,
                            cs_ras_ranging_counter_t counter);
static void handle_lost_complete(cs_ras_client_t                    *client,
                                 cs_ras_cp_complete_lost_response_t *data);
static void handle_response_code(cs_ras_client_t                 *client,
                                 cs_ras_cp_response_code_value_t response_code);

// -----------------------------------------------------------------------------
// Internal functions

sl_status_t cs_ras_client_control_point_abort(cs_ras_client_t *client)
{
  sl_status_t sc;
  uint16_t sent_len;
  cs_ras_cp_command_t command;
  command.op_code = CS_RAS_CP_OPCODE_ABORT;

  sc = sl_bt_gatt_write_characteristic_value_without_response(client->connection,
                                                              client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT],
                                                              sizeof(cs_ras_cp_opcode_t),
                                                              (uint8_t *)&command,
                                                              &sent_len);
  if (sc == SL_STATUS_OK) {
    client->abort = true;
    client->operation = true;
    cs_ras_client_log_debug(CONN_PREFIX CP_PREFIX "Abort requested." LOG_NL,
                            client->connection);
  }
  return sc;
}

sl_status_t cs_ras_client_control_point_ack(cs_ras_client_t          *client,
                                            cs_ras_ranging_counter_t ranging_counter)
{
  sl_status_t sc;
  uint16_t sent_len;
  cs_ras_cp_command_t command;
  command.op_code = CS_RAS_CP_OPCODE_ACK;
  command.data.ack_ranging_data.ranging_counter = ranging_counter;

  sc = sl_bt_gatt_write_characteristic_value_without_response(client->connection,
                                                              client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT],
                                                              sizeof(cs_ras_cp_opcode_t) + sizeof(cs_ras_cp_ack_ranging_data_t),
                                                              (uint8_t *)&command,
                                                              &sent_len);
  if (sc == SL_STATUS_OK) {
    client->op_code = CS_RAS_CP_OPCODE_ACK;
    client->operation = true;
    cs_ras_client_log_debug(CONN_PREFIX CP_PREFIX "Ack requested." LOG_NL,
                            client->connection);
  }
  return sc;
}

sl_status_t cs_ras_client_control_point_get_ranging_data(cs_ras_client_t          *client,
                                                         cs_ras_ranging_counter_t ranging_counter)
{
  sl_status_t sc;
  uint16_t sent_len;
  cs_ras_cp_command_t command;
  command.op_code = CS_RAS_CP_OPCODE_GET;
  command.data.get_ranging_data.ranging_counter = ranging_counter;

  sc = sl_bt_gatt_write_characteristic_value_without_response(client->connection,
                                                              client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT],
                                                              sizeof(cs_ras_cp_opcode_t) + sizeof(cs_ras_cp_get_ranging_data_t),
                                                              (uint8_t *)&command,
                                                              &sent_len);
  if (sc == SL_STATUS_OK) {
    client->op_code = CS_RAS_CP_OPCODE_GET;
    client->operation = true;
    cs_ras_client_log_debug(CONN_PREFIX CP_PREFIX "Get ranging data requested for counter: %u" LOG_NL,
                            client->connection,
                            ranging_counter);
  }
  return sc;
}

sl_status_t cs_ras_client_control_point_retreive_lost_segments(cs_ras_client_t          *client,
                                                               cs_ras_ranging_counter_t ranging_counter,
                                                               uint8_t                  start_segment,
                                                               uint8_t                  end_segment)
{
  sl_status_t sc;
  uint16_t sent_len;
  cs_ras_cp_command_t command;
  command.op_code = CS_RAS_CP_OPCODE_RETRIEVE_LOST_SEGMENTS;
  command.data.retreive_lost.ranging_counter = ranging_counter;
  command.data.retreive_lost.start_segment = start_segment;
  command.data.retreive_lost.end_segment = end_segment;

  sc = sl_bt_gatt_write_characteristic_value_without_response(client->connection,
                                                              client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT],
                                                              sizeof(cs_ras_cp_opcode_t) + sizeof(cs_ras_cp_retreive_lost_t),
                                                              (uint8_t *)&command,
                                                              &sent_len);
  if (sc == SL_STATUS_OK) {
    client->op_code = CS_RAS_CP_OPCODE_RETRIEVE_LOST_SEGMENTS;
    client->operation = true;
    cs_ras_client_log_debug(CONN_PREFIX CP_PREFIX "Get ranging data requested for counter: %u (%u -> %u)" LOG_NL,
                            client->connection,
                            ranging_counter,
                            start_segment,
                            end_segment);
  }
  return sc;
}

// -----------------------------------------------------------------------------
// Internal functions

bool cs_ras_client_control_point_on_bt_event(sl_bt_msg_t *evt)
{
  cs_ras_client_t *client;
  cs_ras_cp_response_t *rsp;
  bool handled = false;

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_gatt_characteristic_value_id:
      client = cs_ras_client_find(evt->data.evt_gatt_characteristic_value.connection);
      if (client == NULL) {
        break;
      }
      // Indication should be handled only
      if (evt->data.evt_gatt_characteristic_value.att_opcode
          != sl_bt_gatt_handle_value_indication) {
        break;
      }
      // RAS Control Point indication
      if (evt->data.evt_gatt_characteristic_value.characteristic
          != client->handles->array[CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT]) {
        break;
      }
      handled = true;
      if (evt->data.evt_gatt_characteristic_value.value.len
          < (sizeof(cs_ras_cp_response_opcode_t)
             + sizeof(cs_ras_cp_response_code_value_t))) {
        (void)sl_bt_gatt_send_characteristic_confirmation(client->connection);
        break;
      }
      rsp = (cs_ras_cp_response_t *)evt->data.evt_gatt_characteristic_value.value.data;
      switch (rsp->op_code) {
        case CS_RAS_CP_RESPONSE_COMPLETE:
          if (evt->data.evt_gatt_characteristic_value.value.len
              != sizeof(cs_ras_cp_response_opcode_t)
              + sizeof(cs_ras_cp_complete_response_t)) {
            break;
          }
          handle_complete(client, rsp->data.complete_ranging_data);
          break;
        case CS_RAS_CP_RESPONSE_COMPLETE_LOST_SEGMENT:
          if (evt->data.evt_gatt_characteristic_value.value.len
              != sizeof(cs_ras_cp_response_opcode_t)
              + sizeof(cs_ras_cp_complete_lost_response_t)) {
            break;
          }
          handle_lost_complete(client, &rsp->data.complete_lost_segment);
          break;
        case CS_RAS_CP_RESPONSE_CODE:
          if (evt->data.evt_gatt_characteristic_value.value.len
              != sizeof(cs_ras_cp_response_opcode_t)
              + sizeof(cs_ras_cp_response_code_value_t)) {
            break;
          }
          handle_response_code(client, rsp->data.response_code.value);
          break;
        default:
          // Invalid opcode
          break;
      }
      // Send confirmation
      (void)sl_bt_gatt_send_characteristic_confirmation(client->connection);
      break;
    default:
      break;
  }
  return !handled;
}

// -----------------------------------------------------------------------------
// Private functions

static void handle_complete(cs_ras_client_t          *client,
                            cs_ras_ranging_counter_t counter)
{
  sl_status_t sc;
  cs_ras_client_messaging_complete_t complete;
  if ((client->state != CLIENT_STATE_ON_DEMAND)
      || !client->operation
      || (client->op_code != CS_RAS_CP_OPCODE_GET)) {
    if (client->op_code != CS_RAS_CP_OPCODE_ABORT) {
      // Invalid state
      cs_ras_client_log_warning(CONN_PREFIX CP_PREFIX "Get ranging data completed in invalid state." LOG_NL,
                                client->connection);
    }
    return;
  }
  if (counter != client->ranging_counter) {
    cs_ras_client_log_warning(CONN_PREFIX CP_PREFIX "Get ranging data completed for invalid counter: %u instead of %u" LOG_NL,
                              client->connection,
                              counter,
                              client->ranging_counter);
    return;
  }

  // Stop timers
  (void)app_timer_stop(&client->timer.control_point);
  (void)app_timer_stop(&client->timer.data_arrived);

  cs_ras_client_log_debug(CONN_PREFIX CP_PREFIX "Get ranging data completed for counter %u." LOG_NL,
                          client->connection,
                          counter);
  // Immediate stop (in case of ongoing reception)
  (void)cs_ras_client_messaging_stop(&client->messaging);
  // Operation finished
  client->operation = false;

  sc = cs_ras_client_messaging_get_complete(&client->messaging, &complete);
  sl_status_t status_send = (sc == SL_STATUS_OK) ? complete.sc : sc;
  // Callback
  cs_ras_client_on_ranging_data_reception_finished(client->connection,
                                                   false,
                                                   false,
                                                   status_send,
                                                   CS_RAS_CP_RESPONSE_CODE_SUCCESS,
                                                   counter,
                                                   complete.start_segment,
                                                   complete.end_segment,
                                                   complete.recoverable,
                                                   complete.size,
                                                   complete.last_arrived,
                                                   complete.last_known_segment,
                                                   complete.lost_segments);
}

static void handle_lost_complete(cs_ras_client_t                    *client,
                                 cs_ras_cp_complete_lost_response_t *data)
{
  cs_ras_client_messaging_complete_t complete;
  sl_status_t sc;

  if ((client->state != CLIENT_STATE_ON_DEMAND)
      || !client->operation
      || (client->op_code != CS_RAS_CP_OPCODE_RETRIEVE_LOST_SEGMENTS)) {
    // Invalid state
    cs_ras_client_log_warning(CONN_PREFIX CP_PREFIX "Retrieve lost segments completed in invalid state." LOG_NL,
                              client->connection);
    return;
  }
  if (data->ranging_counter != client->ranging_counter) {
    // Invalid counter value
    cs_ras_client_log_warning(CONN_PREFIX CP_PREFIX "Retrieve lost segments completed for invalid counter: %u instead of %u" LOG_NL,
                              client->connection,
                              data->ranging_counter,
                              client->ranging_counter);
    return;
  }

  // Stop control point timer
  (void)app_timer_stop(&client->timer.control_point);
  (void)app_timer_stop(&client->timer.data_arrived);

  cs_ras_client_log_debug(CONN_PREFIX CP_PREFIX "Retrieve lost segments completed for counter %u." LOG_NL,
                          client->connection,
                          data->ranging_counter);

  // Immediate stop (in case of ongoing reception)
  (void)cs_ras_client_messaging_stop(&client->messaging);
  // Operation finished
  client->operation = false;

  sc = cs_ras_client_messaging_get_complete(&client->messaging, &complete);
  sl_status_t status_send = (sc == SL_STATUS_OK) ? complete.sc : sc;
  // Callback
  cs_ras_client_on_ranging_data_reception_finished(client->connection,
                                                   false,
                                                   true,
                                                   status_send,
                                                   CS_RAS_CP_RESPONSE_CODE_SUCCESS,
                                                   data->ranging_counter,
                                                   data->start_segment,
                                                   data->end_segment,
                                                   complete.recoverable,
                                                   complete.size,
                                                   complete.last_arrived,
                                                   complete.last_known_segment,
                                                   complete.lost_segments);
}

static void handle_response_code(cs_ras_client_t                 *client,
                                 cs_ras_cp_response_code_value_t response_code)
{
  cs_ras_client_messaging_complete_t complete;
  sl_status_t sc;

  if (!client->operation) {
    cs_ras_client_log_warning(CONN_PREFIX CP_PREFIX
                              "Response code without operation: 0x%02x" LOG_NL,
                              client->connection,
                              response_code);
    return;
  }
  cs_ras_client_log_debug(CONN_PREFIX CP_PREFIX
                          "Response code arrived: %02x." LOG_NL,
                          client->connection,
                          response_code);

  // Stop control point timer
  (void)app_timer_stop(&client->timer.control_point);

  if (client->abort) {
    if (response_code == CS_RAS_CP_RESPONSE_CODE_SUCCESS) {
      // Successful abort, stop timer
      (void)app_timer_stop(&client->timer.data_arrived);
      // Operation finished if abort was successful
      client->operation = false;
    }
    client->abort = false;
    cs_ras_client_on_operation_finished(client->connection,
                                        CS_RAS_CP_OPCODE_ABORT,
                                        SL_STATUS_OK,
                                        response_code);
    cs_ras_client_on_abort_finished(client->connection,
                                    SL_STATUS_OK,
                                    response_code);
    return;
  } else {
    // Operation finished if it was not abort
    client->operation = false;
  }

  // Common callbacks
  switch (client->op_code) {
    case CS_RAS_CP_OPCODE_ACK:
    case CS_RAS_CP_OPCODE_SET_FILTER:
      cs_ras_client_on_operation_finished(client->connection,
                                          client->op_code,
                                          SL_STATUS_OK,
                                          response_code);
      break;
    default:
      break;
  }

  // Individual callbacks
  switch (client->op_code) {
    case CS_RAS_CP_OPCODE_ACK:
      cs_ras_client_on_ack_finished(client->connection,
                                    SL_STATUS_OK,
                                    response_code);
      break;
    case CS_RAS_CP_OPCODE_GET:
    case CS_RAS_CP_OPCODE_RETRIEVE_LOST_SEGMENTS:
      if (response_code == CS_RAS_CP_RESPONSE_CODE_SUCCESS) {
        cs_ras_client_log_warning(CONN_PREFIX CP_PREFIX
                                  "Response code success is not valid for opcode 0x%02x." LOG_NL,
                                  client->connection,
                                  client->op_code);
      }
      // Immediate stop (in case of ongoing reception)
      (void)cs_ras_client_messaging_stop(&client->messaging);

      sc = cs_ras_client_messaging_get_complete(&client->messaging, &complete);
      sl_status_t status_send = (sc == SL_STATUS_OK) ? complete.sc : sc;
      // Callback
      cs_ras_client_on_ranging_data_reception_finished(client->connection,
                                                       false,
                                                       (client->op_code == CS_RAS_CP_OPCODE_RETRIEVE_LOST_SEGMENTS),
                                                       status_send,
                                                       response_code,
                                                       client->ranging_counter,
                                                       complete.start_segment,
                                                       complete.end_segment,
                                                       complete.recoverable,
                                                       complete.size,
                                                       complete.last_arrived,
                                                       complete.last_known_segment,
                                                       complete.lost_segments);
      break;
    default:
      break;
  }
}

void cs_ras_client_messaging_reception_stopped(cs_ras_client_messaging_reception_t *rx,
                                               cs_ras_messaging_status_t           status,
                                               sl_status_t                         sc,
                                               bool                                recoverable,
                                               uint32_t                            size,
                                               bool                                last_arrived,
                                               uint8_t                             last_known_segment,
                                               uint64_t                            lost_segments)
{
  cs_ras_client_t *client = cs_ras_client_find(rx->config.conn_handle);
  if (client == NULL) {
    return;
  }
  // Stop data timer
  (void)app_timer_stop(&client->timer.data_arrived);
  // Real-Time mode only
  if (client->state != CLIENT_STATE_REAL_TIME) {
    return;
  }
  // Finish operation since reception has been stopped
  client->operation = false;
  // Callback
  cs_ras_client_on_ranging_data_reception_finished(client->connection,
                                                   true,
                                                   false,
                                                   sc,
                                                   status,
                                                   client->ranging_counter,
                                                   0,
                                                   CS_RAS_ALL_SEGMENTS,
                                                   recoverable,
                                                   size,
                                                   last_arrived,
                                                   last_known_segment,
                                                   lost_segments);
}

// -----------------------------------------------------------------------------
// Weak implementation of callback functions

SL_WEAK void cs_ras_client_on_ranging_data_reception_finished(uint8_t                         connection,
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
  (void)connection;
  (void)real_time;
  (void)retrieve_lost;
  (void)sc;
  (void)response;
  (void)ranging_counter;
  (void)start_segment;
  (void)end_segment;
  (void)recoverable;
  (void)size;
  (void)last_arrived;
  (void)last_known_segment;
  (void)lost_segments;
}

SL_WEAK void cs_ras_client_on_operation_finished(uint8_t                   connection,
                                                 cs_ras_cp_opcode_t        opcode,
                                                 sl_status_t               sc,
                                                 cs_ras_cp_response_code_value_t response)
{
  (void)connection;
  (void)opcode;
  (void)sc;
  (void)response;
}

SL_WEAK void cs_ras_client_on_ack_finished(uint8_t                         connection,
                                           sl_status_t                     sc,
                                           cs_ras_cp_response_code_value_t response)
{
  (void)connection;
  (void)sc;
  (void)response;
}

SL_WEAK void cs_ras_client_on_abort_finished(uint8_t                         connection,
                                             sl_status_t                     sc,
                                             cs_ras_cp_response_code_value_t response)
{
  (void)connection;
  (void)sc;
  (void)response;
}

SL_WEAK void cs_ras_client_on_set_filter_finished(uint8_t                         connection,
                                                  sl_status_t                     sc,
                                                  cs_ras_cp_response_code_value_t response)
{
  (void)connection;
  (void)sc;
  (void)response;
}
