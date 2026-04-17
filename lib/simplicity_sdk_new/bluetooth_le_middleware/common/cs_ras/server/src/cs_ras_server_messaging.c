/***************************************************************************//**
 * @file
 * @brief CS RAS Server - Messaging implementation
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
#include "cs_ras_server_messaging.h"
#include "cs_ras_server_messaging_internal.h"
#include "cs_ras_server_internal.h"
#include "cs_ras_server_log.h"
#include "sl_status.h"
#include "sl_slist.h"
#include "sl_core.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif // SL_CATALOG_POWER_MANAGER_PRESENT

// -----------------------------------------------------------------------------
// Definitions

#define SEGMENT_PREFIX "Segment %u "

// -----------------------------------------------------------------------------
// Private variables

static sl_slist_node_t *transmit_list_head = NULL;

// -----------------------------------------------------------------------------
// Forward declaration of private functions

static sl_status_t transmit(cs_ras_server_messaging_transmit_t *tx,
                            cs_ras_messaging_config_t          *config,
                            bool                               all);
static bool find_transmit(cs_ras_server_messaging_transmit_t *tx);
static cs_ras_server_messaging_transmit_t *find_transmit_by_connection(uint8_t conn_handle);
static uint16_t segment_count(uint16_t att_mtu, uint32_t data_size);
static uint16_t segments_left(cs_ras_server_messaging_transmit_t *tx);
static sl_status_t transmit_segment(cs_ras_server_messaging_transmit_t *tx);
static sl_status_t send_data(uint8_t  conn_handle,
                             bool     real_time,
                             bool     indication,
                             uint8_t  *data,
                             uint16_t data_size);

// -----------------------------------------------------------------------------
// Public functions

void cs_ras_server_messaging_init(void)
{
  sl_slist_init(&transmit_list_head);
  cs_ras_server_log_info("Initialized" LOG_NL);
}

void cs_ras_server_messaging_step(void)
{
  CORE_DECLARE_IRQ_STATE;
  sl_slist_node_t *iterator;
  sl_status_t sc;

  SL_SLIST_FOR_EACH(transmit_list_head, iterator) {
    cs_ras_server_messaging_transmit_t *tx
      = (cs_ras_server_messaging_transmit_t *)iterator;
    // Transmit as much segment as we can
    sc = transmit_segment(tx);
    (void)sc;
    cs_ras_server_log_debug(CONN_PREFIX "Segment transmit, sc = 0x%04lx" LOG_NL,
                            tx->config.conn_handle,
                            (unsigned long)sc);
  }

  CORE_ENTER_ATOMIC();
  SL_SLIST_FOR_EACH(transmit_list_head, iterator) {
    cs_ras_server_messaging_transmit_t *tx
      = (cs_ras_server_messaging_transmit_t *)iterator;
    uint16_t remaining_segments;
    remaining_segments = segments_left(tx);
    cs_ras_server_log_debug(CONN_PREFIX "Segment left: %u" LOG_NL,
                            tx->config.conn_handle,
                            remaining_segments);
    if (remaining_segments == 0 || tx->message_status.stopped) {
      // Transmission finished
      sl_slist_remove(&transmit_list_head, &tx->node);

      cs_ras_messaging_status_t transfer_status;
      switch (tx->status) {
        case SL_STATUS_OK:
          transfer_status = CS_RAS_MESSAGING_STATUS_SUCCESS;
          break;
        case SL_STATUS_TIMEOUT:
          transfer_status = CS_RAS_MESSAGING_STATUS_TIMEOUT;
          break;
        case SL_STATUS_ABORT:
          transfer_status = CS_RAS_MESSAGING_STATUS_CANCELLED;
          break;
        default:
          transfer_status = CS_RAS_MESSAGING_STATUS_FAILURE;
          break;
      }
      cs_ras_server_log_info(CONN_PREFIX "Transmit ended, status = %u, sc = 0x%04lx" LOG_NL,
                             tx->config.conn_handle,
                             transfer_status,
                             (unsigned long)tx->status);
      // Call callback
      cs_ras_server_messaging_on_transmit_finished(tx,
                                                   transfer_status,
                                                   tx->status);
    }
  }
  CORE_EXIT_ATOMIC();
}

bool cs_ras_server_messaging_on_bt_event(sl_bt_msg_t *evt)
{
  bool handled = false;
  cs_ras_server_messaging_transmit_t *tx;

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_connection_closed_id:
      // Find transmit based on connection handle in the event
      tx = find_transmit_by_connection(evt->data.evt_connection_closed.connection);
      if (tx == NULL) {
        break;
      }
      tx->status = SL_STATUS_DELETED;
      tx->message_status.stopped = true;
      break;
    case sl_bt_evt_gatt_mtu_exchanged_id:
      // Find transmit based on connection handle in the event
      tx = find_transmit_by_connection(evt->data.evt_gatt_mtu_exchanged.connection);
      if (tx == NULL) {
        break;
      }
      if (evt->data.evt_gatt_mtu_exchanged.mtu != tx->config.att_mtu) {
        // Misconfiguration
        tx->status = SL_STATUS_DELETED;
        tx->message_status.stopped = true;
      }
      break;
    case sl_bt_evt_gatt_server_characteristic_status_id:
      // Check for characteristic confirmation
      if ((sl_bt_gatt_server_confirmation == evt->data.evt_gatt_server_characteristic_status.status_flags)
          && ((cs_ras_server_get_handle(CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA)
               == evt->data.evt_gatt_server_characteristic_status.characteristic)
              || (cs_ras_server_get_handle(CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA)
                  == evt->data.evt_gatt_server_characteristic_status.characteristic))) {
        cs_ras_server_log_debug(CONN_PREFIX "confirm arrived" LOG_NL,
                                evt->data.evt_gatt_server_characteristic_status.connection);
        // Find transmit based on connection handle in the event
        tx = find_transmit_by_connection(evt->data.evt_gatt_server_characteristic_status.connection);
        if (tx != NULL) {
          // Set confirmed state
          tx->message_status.confirmed = true;
          cs_ras_server_log_debug(CONN_PREFIX "confirmed" LOG_NL,
                                  evt->data.evt_gatt_server_characteristic_status.connection);
        }
      }
      break;
    default:
      break;
  }
  return !handled;
}

sl_status_t cs_ras_server_messaging_transmit_all(cs_ras_server_messaging_transmit_t *tx,
                                                 cs_ras_messaging_config_t          *config)
{
  sl_status_t sc = SL_STATUS_OK;
  sc = transmit(tx, config, true);
  return sc;
}

sl_status_t cs_ras_server_messaging_transmit(cs_ras_server_messaging_transmit_t *tx,
                                             cs_ras_messaging_config_t          *config)
{
  sl_status_t sc = SL_STATUS_OK;
  sc = transmit(tx, config, false);
  return sc;
}

sl_status_t cs_ras_server_messaging_cancel(cs_ras_server_messaging_transmit_t *tx)
{
  CORE_DECLARE_IRQ_STATE;
  sl_status_t sc = SL_STATUS_OK;

  if (tx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  CORE_ENTER_ATOMIC();
  if (find_transmit(tx)) {
    tx->status = SL_STATUS_ABORT;
    tx->message_status.stopped = true;
    sc = SL_STATUS_OK;
    cs_ras_server_log_debug(CONN_PREFIX "Cancel requested" LOG_NL,
                            tx->config.conn_handle);
  } else {
    sc = SL_STATUS_NOT_FOUND;
  }
  CORE_EXIT_ATOMIC();
  return sc;
}

#ifdef SL_CATALOG_POWER_MANAGER_PRESENT

bool cs_ras_server_messaging_is_ok_to_sleep(void)
{
  return (transmit_list_head == NULL);
}

#endif // SL_CATALOG_POWER_MANAGER_PRESENT

// -----------------------------------------------------------------------------
// Private functions

static sl_status_t transmit(cs_ras_server_messaging_transmit_t *tx,
                            cs_ras_messaging_config_t          *config,
                            bool                               all)
{
  sl_status_t sc = SL_STATUS_OK;
  CORE_DECLARE_IRQ_STATE;

  if (tx == NULL || config == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (config->data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (config->data_size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (config->conn_handle == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  if (config->att_mtu < ATT_MTU_MIN || config->att_mtu > ATT_MTU_MAX) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  uint16_t end_segment = segment_count(config->att_mtu, config->data_size) - 1;
  // Check segments in config
  if (!all) {
    if (config->start_segment > config->end_segment) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    // Request beyond last
    if (config->start_segment > end_segment) {
      return SL_STATUS_NOT_FOUND;
    }
    // Request beyond last
    if (config->end_segment != CS_RAS_MESSAGING_ALL_SEGMENTS
        && config->end_segment > end_segment) {
      return SL_STATUS_NOT_FOUND;
    }
  }
  // Check for transfer in the list
  CORE_ENTER_ATOMIC();
  if (find_transmit(tx)
      || (find_transmit_by_connection(config->conn_handle) != NULL)) {
    CORE_EXIT_ATOMIC();
    return SL_STATUS_ALREADY_EXISTS;
  }
  memset(tx, 0, sizeof(cs_ras_server_messaging_transmit_t));
  memcpy(&tx->config, config, sizeof(tx->config));
  if (all) {
    tx->config.start_segment   = 0;
    tx->config.end_segment     = end_segment;
  }
  if (tx->config.end_segment == CS_RAS_MESSAGING_ALL_SEGMENTS) {
    tx->config.end_segment     = end_segment;
  }
  tx->segment_counter          = tx->config.start_segment;
  tx->segment_size             = 0;
  tx->message_status.created   = false;
  tx->message_status.sent      = false;
  tx->message_status.confirmed = false;
  tx->message_status.stopped   = false;
  tx->status                   = SL_STATUS_OK;

  // Add to the list
  sl_slist_push_back(&transmit_list_head, &tx->node);

  CORE_EXIT_ATOMIC();

  cs_ras_server_log_debug(CONN_PREFIX "Added transmission, segments %u -> %u" LOG_NL,
                          tx->config.conn_handle,
                          tx->config.start_segment,
                          tx->config.end_segment);
  return sc;
}

static bool find_transmit(cs_ras_server_messaging_transmit_t *tx)
{
  sl_slist_node_t *iterator;

  SL_SLIST_FOR_EACH(transmit_list_head, iterator)
  {
    if ( ((cs_ras_server_messaging_transmit_t *)(iterator)) == tx ) {
      return true;
    }
  }
  return false;
}

static cs_ras_server_messaging_transmit_t *find_transmit_by_connection(uint8_t conn_handle)
{
  sl_slist_node_t *iterator;
  cs_ras_server_messaging_transmit_t *tx = NULL;

  SL_SLIST_FOR_EACH(transmit_list_head, iterator)
  {
    if ( ((cs_ras_server_messaging_transmit_t *)(iterator))->config.conn_handle == conn_handle) {
      tx = (cs_ras_server_messaging_transmit_t *)(iterator);
    }
  }
  return tx;
}

static uint16_t segment_count(uint16_t att_mtu, uint32_t data_size)
{
  uint16_t count;
  uint16_t segment_data_size = CS_RAS_SEGMENT_DATA_SIZE(att_mtu);
  count = (uint16_t)((data_size + (segment_data_size - 1)) / segment_data_size);
  return count;
}

static uint16_t segments_left(cs_ras_server_messaging_transmit_t *tx)
{
  if (tx->segment_counter > tx->config.end_segment) {
    return 0;
  }
  return tx->config.end_segment + 1 - tx->segment_counter;
}

static sl_status_t transmit_segment(cs_ras_server_messaging_transmit_t *tx)
{
  sl_status_t sc;
  uint16_t remaining_segments;
  uint32_t segment_data_size;
  uint32_t segment_pos;

  if (tx->message_status.stopped) {
    cs_ras_server_log_debug(CONN_PREFIX "Stopped at " SEGMENT_PREFIX LOG_NL,
                            tx->config.conn_handle,
                            tx->segment_counter);
    return SL_STATUS_OK;
  }

  if (tx->message_status.sent && !tx->message_status.confirmed) {
    // If sent but not confirmed yet, move on to the next
    cs_ras_server_log_debug(CONN_PREFIX SEGMENT_PREFIX "Sent but not confirmed." LOG_NL,
                            tx->config.conn_handle,
                            tx->segment_counter);
    return SL_STATUS_OK;
  }

  if (tx->message_status.sent && tx->message_status.confirmed) {
    // Require to create message
    tx->message_status.created   = false;
    tx->message_status.sent      = false;
    tx->message_status.confirmed = false;
    cs_ras_server_log_debug(CONN_PREFIX SEGMENT_PREFIX "sent." LOG_NL,
                            tx->config.conn_handle,
                            tx->segment_counter);
    tx->segment_counter++;
  }

  remaining_segments = segments_left(tx);
  cs_ras_server_log_debug(CONN_PREFIX "Remaining: %u." LOG_NL,
                          tx->config.conn_handle,
                          remaining_segments);
  if (remaining_segments == 0) {
    // Move on to the next in case of complete
    return SL_STATUS_OK;
  }

  // Create message if required
  if (!tx->message_status.created) {
    cs_ras_server_log_debug(CONN_PREFIX SEGMENT_PREFIX "creating" LOG_NL,
                            tx->config.conn_handle,
                            tx->segment_counter);
    // Clear header
    tx->storage.segment.header = CS_RAS_SEGMENT_HEADER_EMPTY;
    if (tx->segment_counter == 0) {
      // Set first segment
      CS_RAS_SET_FIRST_SEGMENT(tx->storage.segment.header);
      cs_ras_server_log_debug(CONN_PREFIX SEGMENT_PREFIX "is the first segment." LOG_NL,
                              tx->config.conn_handle,
                              tx->segment_counter);
    }
    segment_data_size = CS_RAS_SEGMENT_DATA_SIZE(tx->config.att_mtu);
    segment_pos = tx->segment_counter * segment_data_size;
    // Apply segment counter
    CS_RAS_SET_SEGMENT_COUNTER(tx->storage.segment.header, tx->segment_counter);
    uint16_t end_segment
      = segment_count(tx->config.att_mtu, tx->config.data_size) - 1;
    if (tx->segment_counter == end_segment) {
      // Last segment
      CS_RAS_SET_LAST_SEGMENT(tx->storage.segment.header);
      cs_ras_server_log_debug(CONN_PREFIX SEGMENT_PREFIX "is the last segment." LOG_NL,
                              tx->config.conn_handle,
                              tx->segment_counter);
      // Remaining bytes in the segment
      tx->segment_size = tx->config.data_size + CS_RAS_SEGMENT_HEADER_SIZE
                         - (tx->segment_counter * segment_data_size);
    } else {
      // Complete segment
      tx->segment_size = segment_data_size + CS_RAS_SEGMENT_HEADER_SIZE;
    }
    // Copy data to the data part of the segment
    memcpy(tx->storage.segment.data,
           &tx->config.data[segment_pos],
           tx->segment_size);
    // Set created
    tx->message_status.created   = true;
    tx->message_status.sent      = false;
    tx->message_status.confirmed = false;
  }
  // Try to transmit data
  sc = send_data(tx->config.conn_handle,
                 tx->config.real_time,
                 tx->config.indication,
                 tx->storage.data,
                 tx->segment_size);
  // Check result of segment transmission
  if (sc == SL_STATUS_OK) {
    cs_ras_server_log_debug(CONN_PREFIX SEGMENT_PREFIX "send requested." LOG_NL,
                            tx->config.conn_handle,
                            tx->segment_counter);
    tx->message_status.sent = true;
    // Set confirmation automatically in case of notificaion
    if (tx->config.indication == false) {
      // Require to create message
      tx->message_status.created   = false;
      tx->message_status.sent      = false;
      tx->message_status.confirmed = false;
      cs_ras_server_log_debug(CONN_PREFIX SEGMENT_PREFIX "sent." LOG_NL,
                              tx->config.conn_handle,
                              tx->segment_counter);
      tx->segment_counter++;
    } else {
      tx->message_status.confirmed = false;
      cs_ras_server_log_debug(CONN_PREFIX SEGMENT_PREFIX "waiting for confirmation." LOG_NL,
                              tx->config.conn_handle,
                              tx->segment_counter);
      return SL_STATUS_OK;
    }
  } else if (sc == SL_STATUS_IN_PROGRESS
             || sc == SL_STATUS_NO_MORE_RESOURCE) {
    cs_ras_server_log_debug(CONN_PREFIX SEGMENT_PREFIX "sending in progress." LOG_NL,
                            tx->config.conn_handle,
                            tx->segment_counter);
    // Move on to the next in case of busy
    return SL_STATUS_OK;
  } else {
    cs_ras_server_log_debug(CONN_PREFIX SEGMENT_PREFIX "sending failed, sc = 0x%04lx." LOG_NL,
                            tx->config.conn_handle,
                            tx->segment_counter,
                            (unsigned long)sc);
    // Stop transmission
    tx->message_status.stopped = true;
    tx->status = sc;
    return sc;
  }
  return SL_STATUS_TRANSMIT_INCOMPLETE;
}

static sl_status_t send_data(uint8_t  conn_handle,
                             bool     real_time,
                             bool     indication,
                             uint8_t  *data,
                             uint16_t data_size)
{
  sl_status_t sc;

  // Select characteristic handle
  uint16_t char_handle
    = real_time ? cs_ras_server_get_handle(CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA) : cs_ras_server_get_handle(CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA);

  // Send indication or notification
  if (indication) {
    cs_ras_server_log_debug(CONN_PREFIX "Indicating %d" LOG_NL,
                            conn_handle,
                            char_handle);
    sc = sl_bt_gatt_server_send_indication(conn_handle,
                                           char_handle,
                                           data_size,
                                           data);
  } else {
    cs_ras_server_log_debug(CONN_PREFIX "Notifying %d" LOG_NL,
                            conn_handle,
                            char_handle);
    sc = sl_bt_gatt_server_send_notification(conn_handle,
                                             char_handle,
                                             data_size,
                                             data);
  }
  return sc;
}
