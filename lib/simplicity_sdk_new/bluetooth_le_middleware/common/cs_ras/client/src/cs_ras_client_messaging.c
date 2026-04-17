/***************************************************************************//**
 * @file
 * @brief CS RAS Client - Messaging implementation
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
#include "cs_ras_common.h"
#include "cs_ras_client_messaging.h"
#include "cs_ras_client_log.h"
#include "sl_status.h"
#include "sl_core.h"
#include "sl_slist.h"
#include "sl_bt_api.h"
#include "sl_common.h"

#include <stdio.h>

// -----------------------------------------------------------------------------
// Private variables

static sl_slist_node_t *reception_list_head = NULL;

// -----------------------------------------------------------------------------
// Forward declaration of private functions

static cs_ras_client_messaging_reception_t *find_reception_by_connection(uint8_t conn_handle);
static bool find_reception(cs_ras_client_messaging_reception_t *rx);
static void stop_reception(cs_ras_client_messaging_reception_t *rx,
                           cs_ras_messaging_status_t           status,
                           sl_status_t                         sc);
static void handle_data(cs_ras_client_messaging_reception_t *rx,
                        uint8_t                             size,
                        uint8_t                             *data);
static sl_status_t start_reception(cs_ras_client_messaging_reception_t *rx,
                                   cs_ras_messaging_config_t           *config,
                                   cs_ras_gattdb_handles_t             *handles,
                                   uint8_t                             start_segment,
                                   uint8_t                             end_segment);

// -----------------------------------------------------------------------------
// Public functions

void cs_ras_client_messaging_init(void)
{
  sl_slist_init(&reception_list_head);
  cs_ras_client_log_info("Initialized" LOG_NL);
}

bool cs_ras_client_messaging_on_bt_event(sl_bt_msg_t *evt)
{
  bool handled = false;
  cs_ras_client_messaging_reception_t *rx;
  sl_status_t sc;

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_gatt_mtu_exchanged_id:
      rx = find_reception_by_connection(evt->data.evt_gatt_mtu_exchanged.connection);
      if (rx == NULL) {
        break;
      }
      if (evt->data.evt_gatt_mtu_exchanged.mtu != rx->config.att_mtu) {
        cs_ras_client_log_warning(CONN_PREFIX "Reception MTU changed" LOG_NL,
                                  rx->config.conn_handle);
        stop_reception(rx, CS_RAS_MESSAGING_STATUS_FAILURE, SL_STATUS_DELETED);
      }
      break;
    case sl_bt_evt_connection_closed_id:
      rx = find_reception_by_connection(evt->data.evt_connection_closed.connection);
      if (rx == NULL) {
        break;
      }
      stop_reception(rx, CS_RAS_MESSAGING_STATUS_FAILURE, SL_STATUS_DELETED);
      break;
    case sl_bt_evt_gatt_characteristic_value_id:
      if ((evt->data.evt_gatt_characteristic_value.att_opcode
           != sl_bt_gatt_handle_value_indication)
          && (evt->data.evt_gatt_characteristic_value.att_opcode
              != sl_bt_gatt_handle_value_notification)) {
        break;
      }
      // Find reception
      rx = find_reception_by_connection(evt->data.evt_gatt_characteristic_value.connection);
      if (rx == NULL) {
        break;
      }
      if (!rx->config.indication
          && (evt->data.evt_gatt_characteristic_value.att_opcode
              == sl_bt_gatt_handle_value_indication)) {
        break;
      }
      if (rx->config.indication
          && (evt->data.evt_gatt_characteristic_value.att_opcode
              == sl_bt_gatt_handle_value_notification)) {
        break;
      }
      if (rx->config.real_time
          && (evt->data.evt_gatt_characteristic_value.characteristic
              != rx->handles->array[CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA])) {
        break;
      }
      if (!rx->config.real_time
          && (evt->data.evt_gatt_characteristic_value.characteristic
              != rx->handles->array[CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA])) {
        break;
      }
      if (evt->data.evt_gatt_characteristic_value.att_opcode
          == sl_bt_gatt_handle_value_indication) {
        sc = sl_bt_gatt_send_characteristic_confirmation(evt->data.evt_gatt_characteristic_value.connection);
        if (sc != SL_STATUS_OK) {
          cs_ras_client_log_error(CONN_PREFIX "Send confirm failed" LOG_NL,
                                  evt->data.evt_gatt_characteristic_value.connection);
        } else {
          cs_ras_client_log_debug(CONN_PREFIX "Confirm sent" LOG_NL,
                                  evt->data.evt_gatt_characteristic_value.connection);
        }
      }
      handle_data(rx,
                  evt->data.evt_gatt_characteristic_value.value.len,
                  evt->data.evt_gatt_characteristic_value.value.data);

      handled = true;
      break;
    default:
      break;
  }

  return !handled;
}

sl_status_t cs_ras_client_messaging_receive(cs_ras_client_messaging_reception_t *rx,
                                            cs_ras_messaging_config_t           *config,
                                            cs_ras_gattdb_handles_t             *handles)
{
  return start_reception(rx,
                         config,
                         handles,
                         0,
                         CS_RAS_MESSAGING_ALL_SEGMENTS);
}

sl_status_t cs_ras_client_messaging_receive_lost(cs_ras_client_messaging_reception_t *rx,
                                                 cs_ras_messaging_config_t           *config,
                                                 cs_ras_gattdb_handles_t             *handles,
                                                 uint8_t                             start_segment,
                                                 uint8_t                             end_segment)
{
  return start_reception(rx,
                         config,
                         handles,
                         start_segment,
                         end_segment);
}

sl_status_t cs_ras_client_messaging_stop(cs_ras_client_messaging_reception_t *rx)
{
  sl_status_t sc;
  CORE_DECLARE_IRQ_STATE;
  if (rx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  CORE_ENTER_ATOMIC();

  if (find_reception(rx)) {
    if (rx->segment_status.stopped) {
      sc = SL_STATUS_INVALID_STATE;
    } else {
      stop_reception(rx, CS_RAS_MESSAGING_STATUS_CANCELLED, SL_STATUS_ABORT);
      sc = SL_STATUS_OK;
    }
  } else {
    sc = SL_STATUS_NOT_FOUND;
  }

  CORE_EXIT_ATOMIC();

  return sc;
}

sl_status_t cs_ras_client_messaging_get_complete(cs_ras_client_messaging_reception_t *rx,
                                                 cs_ras_client_messaging_complete_t  *complete)
{
  if (rx == NULL || complete == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (!rx->segment_status.stopped) {
    return SL_STATUS_INVALID_STATE;
  }
  complete->sc = rx->status;
  complete->start_segment = rx->config.start_segment;
  complete->end_segment = rx->config.end_segment;
  complete->recoverable = !rx->segment_status.overflow;
  complete->size = rx->size;
  complete->last_arrived = rx->segment_status.last_received;
  complete->last_known_segment = rx->counter.last;
  complete->lost_segments = rx->lost_segments;
  return SL_STATUS_OK;
}

// -----------------------------------------------------------------------------
// Private functions

static cs_ras_client_messaging_reception_t *find_reception_by_connection(uint8_t conn_handle)
{
  sl_slist_node_t *iterator;
  cs_ras_client_messaging_reception_t *rx = NULL;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();
  SL_SLIST_FOR_EACH(reception_list_head, iterator)
  {
    if (((cs_ras_client_messaging_reception_t *)(iterator))->config.conn_handle
        == conn_handle) {
      rx = (cs_ras_client_messaging_reception_t *)(iterator);
    }
  }
  CORE_EXIT_ATOMIC();
  return rx;
}

static bool find_reception(cs_ras_client_messaging_reception_t *rx)
{
  sl_slist_node_t *iterator;
  bool ret = false;
  SL_SLIST_FOR_EACH(reception_list_head, iterator)
  {
    if ( ((cs_ras_client_messaging_reception_t *)(iterator)) == rx) {
      ret = true;
      break;
    }
  }
  return ret;
}

static void stop_reception(cs_ras_client_messaging_reception_t *rx,
                           cs_ras_messaging_status_t           status,
                           sl_status_t                         sc)
{
  rx->status = status;
  rx->segment_status.started = false;
  rx->segment_status.stopped = true;

  sl_slist_remove(&reception_list_head, &rx->node);

  cs_ras_client_log_info(CONN_PREFIX "Reception stopped @ segment %u, Last is %s,"
                                     "status = %d, sc = 0x%04lx" LOG_NL,
                         rx->config.conn_handle,
                         rx->counter.current,
                         rx->segment_status.last_received ? "received" : "not received",
                         status,
                         (unsigned long)sc);
  cs_ras_client_messaging_reception_stopped(rx,                               // Reception
                                            status,                           // Reception status
                                            sc,                               // Status code
                                            !rx->segment_status.overflow,     // Recoverable
                                            rx->size,                         // Size of data
                                            rx->segment_status.last_received, // Last arrived
                                            rx->counter.current,              // Last known index
                                            rx->lost_segments);               // Lost segments
}

static void handle_data(cs_ras_client_messaging_reception_t *rx,
                        uint8_t                             size,
                        uint8_t                             *data)
{
  // Parse segment
  cs_ras_segment_storage_t *segment = (cs_ras_segment_storage_t *)data;

  // Check message length
  if ((size < CS_RAS_SEGMENT_HEADER_SIZE + 1)
      || (CS_RAS_SEGMENT_SIZE(rx->config.att_mtu) < size)) {
    stop_reception(rx,
                   CS_RAS_MESSAGING_STATUS_FAILURE,
                   SL_STATUS_INVALID_COUNT);
    return;
  }
  // Parse header
  uint8_t segment_counter = CS_RAS_GET_SEGMENT_COUNTER(segment->segment.header);
  bool segment_is_first   = CS_RAS_IS_FIRST_SEGMENT(segment->segment.header);
  bool segment_is_last    = CS_RAS_IS_LAST_SEGMENT(segment->segment.header);
  uint8_t segment_skip    = 0;

  if (segment_is_first) {
    rx->counter.current = segment_counter;
    rx->segment_status.first_received = true;
  } else {
    if (!rx->segment_status.first_received) {
      rx->segment_status.first_received = true;
      segment_skip = segment_counter;

      // Maintain current counter value
      rx->counter.current = segment_counter;

      // Signal lost segments
      for (uint8_t i = rx->config.start_segment; i < segment_counter; i++) {
        rx->lost_segments |= (1 << i);
      }
    } else {
      // Calculate the segments skipped (value of 1 means no data loss)
      segment_skip = (((uint32_t)segment_counter)
                      + (CS_RAS_SEGMENT_COUNTER_MOD)
                      -((uint32_t)rx->counter.previous))
                     % CS_RAS_SEGMENT_COUNTER_MOD - 1;

      // Maintain current counter value
      rx->counter.current = rx->counter.previous + segment_skip + 1;
      // Check and set the overflow
      if (rx->counter.current >= CS_RAS_SEGMENT_COUNTER_MOD) {
        rx->segment_status.overflow = true;
      }

      if (!rx->segment_status.overflow) {
        // Signal lost segments
        for (uint8_t i = segment_counter - 1; i > rx->counter.previous; i--) {
          if ((i >= rx->config.start_segment)
              && (i <= rx->config.end_segment)) {
            rx->lost_segments |= (1 << i);
          }
        }
      }
    }

    // Count lost segments in current the reception
    rx->counter.lost += segment_skip;
  }
  cs_ras_client_log_debug(CONN_PREFIX "Data[%u]: segment_counter = %u, "
                                      "calc_counter = %u, lost = %u, total_lost = %d" LOG_NL,
                          rx->config.conn_handle,
                          (unsigned int)(size - CS_RAS_SEGMENT_HEADER_SIZE),
                          segment_counter,
                          rx->counter.current,
                          segment_skip,
                          rx->counter.lost);
  // Check and maintain last segment
  if (segment_is_last) {
    rx->segment_status.last_received = true;
    rx->counter.last = rx->counter.current;
  }

  // Check for the required segments
  if ((rx->counter.current >= rx->config.start_segment)
      && ((rx->config.end_segment == CS_RAS_MESSAGING_ALL_SEGMENTS)
          || (rx->counter.current <= rx->config.end_segment))) {
    // Calculate index from data size and counter value
    uint32_t index = rx->counter.current
                     * CS_RAS_SEGMENT_DATA_SIZE(rx->config.att_mtu);
    // Check for fit in the buffer
    if ((index + size - CS_RAS_SEGMENT_HEADER_SIZE) > rx->config.data_size) {
      cs_ras_client_log_warning(CONN_PREFIX "Data overflow: current = %u, "
                                            "index = %lu, size = %u, data_size= %lu" LOG_NL,
                                rx->config.conn_handle,
                                rx->counter.current,
                                (unsigned long)index,
                                size,
                                (unsigned long)rx->config.data_size);
      stop_reception(rx,
                     CS_RAS_MESSAGING_STATUS_FAILURE,
                     SL_STATUS_WOULD_OVERFLOW);
      return;
    }
    rx->size = index + size - CS_RAS_SEGMENT_HEADER_SIZE;
    // Copy data into the destination array
    memcpy(&rx->config.data[index],
           segment->segment.data,
           size - CS_RAS_SEGMENT_HEADER_SIZE);
  }

  cs_ras_client_messaging_segment_received(rx, rx->counter.current);

  // Set previous to current
  rx->counter.previous = rx->counter.current;

  // Check finish
  if (rx->config.end_segment == rx->counter.current
      || (segment_is_last
          && (rx->config.end_segment == CS_RAS_MESSAGING_ALL_SEGMENTS))) {
    stop_reception(rx,
                   CS_RAS_MESSAGING_STATUS_SUCCESS,
                   SL_STATUS_OK);
  }
}

static sl_status_t start_reception(cs_ras_client_messaging_reception_t *rx,
                                   cs_ras_messaging_config_t           *config,
                                   cs_ras_gattdb_handles_t             *handles,
                                   uint8_t                             start_segment,
                                   uint8_t                             end_segment)
{
  CORE_DECLARE_IRQ_STATE;

  if (rx == NULL || config == NULL || config->data == NULL || handles == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (config->conn_handle == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (config->real_time) {
    if (handles->array[CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA]
        == CS_RAS_INVALID_CHARACTERISTIC_HANDLE) {
      return SL_STATUS_INVALID_HANDLE;
    }
  } else {
    if (handles->array[CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA]
        == CS_RAS_INVALID_CHARACTERISTIC_HANDLE) {
      return SL_STATUS_INVALID_HANDLE;
    }
  }
  if (config->data_size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (config->att_mtu < ATT_MTU_MIN || config->att_mtu > ATT_MTU_MAX) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();
  if (find_reception(rx)
      || (find_reception_by_connection(config->conn_handle) != NULL)) {
    CORE_EXIT_ATOMIC();
    return SL_STATUS_ALREADY_EXISTS;
  }

  cs_ras_messaging_config_t config_save;
  memcpy(&config_save, config, sizeof(config_save));

  // Clear all
  memset(rx, 0x00, sizeof(cs_ras_client_messaging_reception_t));

  // Copy config
  memcpy(&rx->config, &config_save, sizeof(rx->config));
  // Set start and end segments
  rx->config.start_segment = start_segment;
  rx->config.end_segment = end_segment;
  // Link handles
  rx->handles = handles;
  // Set state
  rx->segment_status.started = true;
  rx->segment_status.stopped = false;
  rx->segment_status.first_received = false;
  // Reset counter
  rx->counter.current = rx->config.start_segment;
  rx->counter.previous = rx->counter.current > 0 ? rx->counter.current - 1 : rx->counter.current;

  // Add to list
  sl_slist_push_back(&reception_list_head, &rx->node);

  CORE_EXIT_ATOMIC();

  cs_ras_client_log_info(CONN_PREFIX "Reception started %u end segment: %u" LOG_NL,
                         rx->config.conn_handle,
                         rx->config.start_segment,
                         rx->config.end_segment);

  return SL_STATUS_OK;
}

SL_WEAK void cs_ras_client_messaging_segment_received(cs_ras_client_messaging_reception_t *rx,
                                                      cs_ras_ranging_counter_t            counter)
{
  (void)rx;
  (void)counter;
}
