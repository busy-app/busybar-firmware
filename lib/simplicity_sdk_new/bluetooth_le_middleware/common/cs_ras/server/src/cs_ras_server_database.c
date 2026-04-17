/***************************************************************************//**
 * @file
 * @brief CS RAS Server - Ranging Database implementation
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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sl_bt_api.h"
#include "cs_ras_server_database.h"
#include "cs_ras_server_log.h"
#include "sl_status.h"
#include "cs_ras_common.h"
#include "cs_ras_format_converter.h"
#include "cs_ras_server_config.h"
#include "cs_ras_server_internal.h"
#include "sl_bluetooth_connection_config.h"

// -----------------------------------------------------------------------------
// Macros

#define CS_RAS_NO_FILTER                      0xffffu
#define SLOT_INVALID                          CS_RAS_PROCEDURE_PER_CONNECTION
#define CS_RAS_MAX_CONNECTIONS                SL_BT_CONFIG_MAX_CONNECTIONS
#define CS_RAS_RANGING_HEADER_LEN             (sizeof(cs_ras_ranging_header_t))
#define CS_RAS_SUBEVENT_HEADER_LEN            (sizeof(cs_ras_subevent_header_t))

// -----------------------------------------------------------------------------
// Type definitions
typedef struct {
  cs_procedure_data_t cs_procedures[CS_RAS_PROCEDURE_PER_CONNECTION];
  cs_ras_filter_t filter;                     // RAS filters
  uint8_t conn_handle;                        // Handle of the connection which initiated the CS procedure
  uint8_t slot_written;
  uint8_t slot_read;
  uint8_t conn_buffer[CS_PROCEDURE_MAX_LEN];  // Data buffer
  uint8_t *write_index;
  #if defined(CS_RAS_SERVER_CONFIG_LOG_DATA) && (CS_RAS_SERVER_CONFIG_LOG_DATA == 1)
  uint8_t step_channels[255];
  uint8_t step_channel_size;
  #endif // defined(CS_RAS_SERVER_CONFIG_LOG_DATA) && (CS_RAS_SERVER_CONFIG_LOG_DATA == 1)
} cs_ras_conn_t;

// -----------------------------------------------------------------------------
// Forward declaration of private functions

static sl_status_t get_write_slot(uint8_t conn_handle,
                                  cs_ras_ranging_counter_t ranging_counter,
                                  uint8_t *result);
static void clear_filter(void);
static void clear_connections(void);
static cs_ras_conn_t *get_connection(uint8_t conn_handle);
static sl_status_t add_data(cs_ras_conn_t *connection, uint8_t slot,
                            const uint8_t *src, size_t size, bool save_ptr);
static void clear_slot(cs_procedure_data_t *slot);

// -----------------------------------------------------------------------------
// Private variables

static cs_ras_conn_t cs_ras_connections[CS_RAS_MAX_CONNECTIONS];

// -----------------------------------------------------------------------------
// Public functions

void cs_ras_server_database_init(void)
{
  clear_filter();
  clear_connections();
  cs_ras_server_log_info(" Ranging database initialized" LOG_NL);
}

sl_status_t cs_ras_server_database_set_filter(uint8_t conn_handle,
                                              cs_ras_filter_t filter)
{
  cs_ras_conn_t *local_connection;
  local_connection = get_connection(conn_handle);

  if (local_connection == NULL) {
    return SL_STATUS_NOT_FOUND;
  }

  local_connection->filter = filter;
  #if defined(CS_RAS_SERVER_CONFIG_LOG_DATA) && (CS_RAS_SERVER_CONFIG_LOG_DATA == 1)
  local_connection->step_channel_size = 0;
  #endif // defined(CS_RAS_SERVER_CONFIG_LOG_DATA) && (CS_RAS_SERVER_CONFIG_LOG_DATA == 1)
  cs_ras_server_log_debug(CONN_PREFIX "Adding filter" LOG_NL, conn_handle);
  return SL_STATUS_OK;
}

cs_procedure_data_t *cs_ras_server_database_find_next_slot(const uint8_t conn_handle,
                                                           cs_ras_ranging_counter_t ranging_counter)
{
  cs_ras_conn_t *local_connection;

  local_connection = get_connection(conn_handle);

  if (local_connection == NULL) {
    return NULL;
  }

  if (ranging_counter == CS_RAS_INVALID_RANGING_COUNTER) {
    for (uint8_t i = 0; i < CS_RAS_PROCEDURE_PER_CONNECTION; i++) {
      if (local_connection->cs_procedures[i].ranging_counter != CS_RAS_INVALID_RANGING_COUNTER) {
        return &local_connection->cs_procedures[i];
      }
    }
  } else {
    for (uint8_t i = 0; i < CS_RAS_PROCEDURE_PER_CONNECTION; i++) {
      if (local_connection->cs_procedures[i].ranging_counter == ranging_counter) {
        return &local_connection->cs_procedures[i];
      }
    }
  }

  return NULL;
}

sl_status_t cs_ras_server_database_store_result(sl_bt_msg_t *cs_res,
                                                cs_ras_mode_t mode)
{
  cs_ras_server_t *server;
  sl_status_t sc;
  cs_ras_ranging_counter_t ranging_counter;
  uint8_t conn_handle;
  uint8_t procedure_done_status;
  uint8_t *data;
  bool continue_evt = false;
  bool save_address_to_db;
  bool overwritten = false;

  switch (SL_BT_MSG_ID(cs_res->header)) {
    case sl_bt_evt_cs_result_id:
      conn_handle = cs_res->data.evt_cs_result.connection;
      procedure_done_status = cs_res->data.evt_cs_result.procedure_done_status;
      ranging_counter = CS_RAS_RANGING_COUNTER_MASK & cs_res->data.evt_cs_result.procedure_counter;
      break;
    case sl_bt_evt_cs_result_continue_id:
      continue_evt = true;
      conn_handle = cs_res->data.evt_cs_result_continue.connection;
      procedure_done_status = cs_res->data.evt_cs_result_continue.procedure_done_status;
      ranging_counter = CS_RAS_INVALID_RANGING_COUNTER;
      break;
    default:
      return SL_STATUS_FAIL;
  }

  #if defined(CS_RAS_SERVER_CONFIG_LOG_DATA) && (CS_RAS_SERVER_CONFIG_LOG_DATA == 1)
  cs_ras_server_log_debug(CONN_PREFIX "RAS DB - Event" LOG_NL,
                          conn_handle);
  cs_ras_server_log_hexdump_debug(cs_res,
                                  SL_BT_MSG_LEN(cs_res->header) + SL_BT_MSG_HEADER_LEN);
  cs_ras_server_log_append_debug(LOG_NL);
  #endif // defined(CS_RAS_SERVER_CONFIG_LOG_DATA) && (CS_RAS_SERVER_CONFIG_LOG_DATA == 1)

  cs_ras_server_log_debug(CONN_PREFIX "Adding result. Counter = 0x%x." LOG_NL,
                          conn_handle,
                          ranging_counter);

  cs_ras_conn_t *local_connection;
  uint8_t dst_slot = SLOT_INVALID;

  local_connection = get_connection(conn_handle);

  if (local_connection == NULL) {
    cs_ras_server_log_error(CONN_PREFIX "Could not find the connection" LOG_NL, conn_handle);
    return SL_STATUS_NULL_POINTER;
  }

  server = cs_ras_server_find(conn_handle);

  if (server == NULL) {
    cs_ras_server_log_error(CONN_PREFIX "Could not find the server" LOG_NL, conn_handle);
    return SL_STATUS_NULL_POINTER;
  }

  sc = get_write_slot(conn_handle, ranging_counter, &dst_slot);
  if (dst_slot == SLOT_INVALID) {
    return SL_STATUS_NOT_FOUND;
  }

  if (sc == SL_STATUS_HAS_OVERFLOWED) {
    if (mode == CS_RAS_MODE_ON_DEMAND_RANGING_DATA) {
      cs_ras_send_overwritten(local_connection->conn_handle,
                              local_connection->cs_procedures[dst_slot].ranging_counter);
    }
    cs_ras_server_log_warning(CONN_PREFIX "Overwriting slot %u" LOG_NL,
                              conn_handle,
                              dst_slot);
    local_connection->cs_procedures[dst_slot].data_len = 0;
    overwritten = true;
  }

  if (ranging_counter != CS_RAS_INVALID_RANGING_COUNTER) {
    local_connection->cs_procedures[dst_slot].ranging_counter = ranging_counter;
  }

  // Header must be added to the beginning of the buffer.
  if (local_connection->cs_procedures[dst_slot].empty || overwritten) {
    save_address_to_db = true;
  } else {
    save_address_to_db = false;
  }

  cs_ras_server_log_debug(CONN_PREFIX "Storing result to slot %u" LOG_NL,
                          conn_handle,
                          dst_slot);

  uint8_t num_steps;
  uint8_t subevent_done_status;

  // Convert event fields to RAS mandated data format
  sc = cs_ras_format_convert_header(&local_connection->cs_procedures[dst_slot].ras_subevent_header,
                                    &local_connection->cs_procedures[dst_slot].ras_header,
                                    cs_res,
                                    server->antenna_config,
                                    continue_evt);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_NULL_POINTER;
  }

  local_connection->cs_procedures[dst_slot].ras_header.selected_tx_power =
    server->tx_power_dbm;

  // Add ranging header to the slot
  if (save_address_to_db) {
    local_connection->cs_procedures[dst_slot].ranging_counter = ranging_counter;
    local_connection->cs_procedures[dst_slot].data_len = 0;

    (void)add_data(local_connection,
                   dst_slot,
                   (uint8_t *)&local_connection->cs_procedures[dst_slot].ras_header,
                   CS_RAS_RANGING_HEADER_LEN,
                   true);
    local_connection->cs_procedures[dst_slot].empty = false;
    cs_ras_server_log_debug(CONN_PREFIX "Slot %u created" LOG_NL,
                            conn_handle,
                            dst_slot);
  }

  static cs_ras_step_header_t step_header;
  uint8_t step_header_len;
  static size_t subevent_header_pos = 0;

  if (continue_evt) {
    num_steps = cs_res->data.evt_cs_result_continue.num_steps;
    data = cs_res->data.evt_cs_result_continue.data.data;
    subevent_done_status = cs_res->data.evt_cs_result_continue.subevent_done_status;
  } else {
    num_steps = cs_res->data.evt_cs_result.num_steps;
    data = cs_res->data.evt_cs_result.data.data;
    subevent_done_status = cs_res->data.evt_cs_result.subevent_done_status;

    // Add the subevent header to the buffer, and save its pointer
    subevent_header_pos = local_connection->write_index
                          - local_connection->cs_procedures[dst_slot].data;
    (void)add_data(local_connection,
                   dst_slot,
                   (uint8_t *)&local_connection->cs_procedures[dst_slot].ras_subevent_header,
                   CS_RAS_SUBEVENT_HEADER_LEN,
                   false);
  }

  step_header_len = sizeof(cs_ras_step_header_t);

  if (num_steps > 160) {
    cs_ras_server_log_error(CONN_PREFIX "Too many steps %d" LOG_NL, conn_handle, num_steps);
    return SL_STATUS_FAIL;
  }

  uint8_t *next_data = data;
  uint8_t step_mode;
  for (uint8_t i = 0; i < num_steps; i++) {
    memcpy(&step_header, next_data, step_header_len);
    step_mode = step_header.step_mode & CS_RAS_STEP_MODE_MASK;
    if (subevent_done_status == sl_bt_cs_done_status_aborted) {
      step_mode |= CS_RAS_STEP_ABORTED_MASK;
    }
    #if defined(CS_RAS_SERVER_CONFIG_LOG_DATA) && (CS_RAS_SERVER_CONFIG_LOG_DATA == 1)
    local_connection->step_channels[local_connection->step_channel_size]
      = step_header.step_channel;
    local_connection->step_channel_size++;
    #endif // defined(CS_RAS_SERVER_CONFIG_LOG_DATA) && (CS_RAS_SERVER_CONFIG_LOG_DATA == 1)
    (void)add_data(local_connection,
                   dst_slot,
                   (uint8_t *)&step_mode,
                   sizeof(step_mode),
                   false);
    // Move on with the source - size of the step header
    next_data += step_header_len;
    if (subevent_done_status != sl_bt_cs_done_status_aborted) {
      (void)add_data(local_connection,
                     dst_slot,
                     next_data,
                     step_header.step_data_length,
                     false);
    }
    // Move on with the source - size of the step data
    next_data += step_header.step_data_length;
  }

  switch (subevent_done_status) {
    case sl_bt_cs_done_status_aborted:
    case sl_bt_cs_done_status_complete:
      // Subevent done
      // Rewrite the subevent header
      cs_ras_server_log_info(CONN_PREFIX "Subevent %s with %d steps" LOG_NL,
                             conn_handle,
                             ((subevent_done_status == sl_bt_cs_done_status_complete) ? "done" : "aborted"),
                             local_connection->cs_procedures[dst_slot].ras_subevent_header.number_of_steps_reported);
      if (subevent_header_pos != 0) {
        memcpy(local_connection->cs_procedures[dst_slot].data + subevent_header_pos,
               &local_connection->cs_procedures[dst_slot].ras_subevent_header,
               CS_RAS_SUBEVENT_HEADER_LEN);
      }
      subevent_header_pos = 0;
      break;
    case sl_bt_cs_done_status_partial_results_continue:
      cs_ras_server_log_debug(CONN_PREFIX "Subevent continue" LOG_NL, conn_handle);
      // Subevent continue, more data to follow
      // Don't do anything. Data are buffered to the temporary buffer.
      break;
    default:
      cs_ras_server_log_debug(CONN_PREFIX "Subevent unknown" LOG_NL, conn_handle);
      break;
  }

  switch (procedure_done_status) {
    case sl_bt_cs_done_status_complete:
    case sl_bt_cs_done_status_aborted:
      sc = SL_STATUS_FULL;
      #if defined(CS_RAS_SERVER_CONFIG_LOG_DATA) && (CS_RAS_SERVER_CONFIG_LOG_DATA == 1)
      cs_ras_server_log_debug(CONN_PREFIX "RAS DB - Procedure %u channels" LOG_NL,
                              local_connection->conn_handle,
                              local_connection->cs_procedures[dst_slot].ranging_counter);
      cs_ras_server_log_hexdump_debug(local_connection->step_channels,
                                      local_connection->step_channel_size);
      local_connection->step_channel_size = 0;
      cs_ras_server_log_append_debug(LOG_NL "--------" LOG_NL);
      cs_ras_server_log_debug(CONN_PREFIX "RAS DB - Procedure %u data ready" LOG_NL,
                              local_connection->conn_handle,
                              local_connection->cs_procedures[dst_slot].ranging_counter);
      cs_ras_server_log_hexdump_debug(local_connection->cs_procedures[dst_slot].data,
                                      local_connection->cs_procedures[dst_slot].data_len);
      cs_ras_server_log_append_debug(LOG_NL "--------" LOG_NL);
      #endif // defined(CS_RAS_SERVER_CONFIG_LOG_DATA) && (CS_RAS_SERVER_CONFIG_LOG_DATA == 1)
      if (mode == CS_RAS_MODE_ON_DEMAND_RANGING_DATA) {
        cs_ras_send_data_ready(local_connection->conn_handle,
                               local_connection->cs_procedures[dst_slot].ranging_counter);
      }
      break;
    case sl_bt_cs_done_status_partial_results_continue:
      // Continue gathering data
      break;
    default:
      cs_ras_server_log_error(CONN_PREFIX "Unknown procedure done status" LOG_NL, conn_handle);
      break;
  }

  return sc;
}

void cs_ras_server_database_sent(uint8_t conn_handle, cs_ras_ranging_counter_t ranging_counter)
{
  cs_ras_conn_t *local_connection = get_connection(conn_handle);

  if (local_connection == NULL) {
    return;
  }

  for (uint8_t i = 0; i < CS_RAS_PROCEDURE_PER_CONNECTION; i++) {
    if (local_connection->cs_procedures[i].ranging_counter == ranging_counter) {
      local_connection->cs_procedures[i].sent = true;
    }
  }
}

void cs_ras_server_database_clear_connection(const uint8_t conn_handle)
{
  cs_ras_conn_t *local_connection;

  local_connection = get_connection(conn_handle);

  if (local_connection == NULL) {
    return;
  }

  for (uint8_t i = 0u; i < CS_RAS_PROCEDURE_PER_CONNECTION; i++) {
    clear_slot(&local_connection->cs_procedures[i]);
  }

  local_connection->filter = CS_RAS_NO_FILTER;
  local_connection->conn_handle = SL_BT_INVALID_CONNECTION_HANDLE;
  local_connection->slot_written = SLOT_INVALID;
  local_connection->write_index = local_connection->conn_buffer;
}

void cs_ras_server_database_clear(uint8_t conn_handle,
                                  cs_ras_ranging_counter_t ranging_counter)
{
  cs_ras_server_log_debug(CONN_PREFIX "Clearing db" LOG_NL, conn_handle);
  cs_ras_conn_t *local_connection = get_connection(conn_handle);

  if (local_connection == NULL) {
    return;
  }

  if (ranging_counter == CS_RAS_INVALID_RANGING_COUNTER) {
    for (uint8_t i = 0; i < CS_RAS_PROCEDURE_PER_CONNECTION; i++) {
      clear_slot(&local_connection->cs_procedures[i]);
    }
    local_connection->write_index = local_connection->conn_buffer;
  } else {
    for (uint8_t i = 0; i < CS_RAS_PROCEDURE_PER_CONNECTION; i++) {
      if (local_connection->cs_procedures[i].ranging_counter == ranging_counter) {
        clear_slot(&local_connection->cs_procedures[i]);
      }
    }
  }
}

// -----------------------------------------------------------------------------
// Private functions

static sl_status_t get_write_slot(uint8_t conn_handle,
                                  cs_ras_ranging_counter_t ranging_counter,
                                  uint8_t *result)
{
  if (result == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  cs_ras_conn_t *local_connection;
  local_connection = get_connection(conn_handle);

  if (local_connection == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Get the current ranging counter
  cs_ras_ranging_counter_t last_ranging_counter;

  last_ranging_counter =
    local_connection->cs_procedures[local_connection->slot_written].ranging_counter;

  if (last_ranging_counter == CS_RAS_INVALID_RANGING_COUNTER) {
    local_connection->slot_written = 0;
  } else if (last_ranging_counter != ranging_counter
             && ranging_counter != CS_RAS_INVALID_RANGING_COUNTER) {
    ++local_connection->slot_written;
    if (local_connection->slot_written >= CS_RAS_PROCEDURE_PER_CONNECTION) {
      local_connection->slot_written = 0;
    }
  }

  *result = local_connection->slot_written;

  if (ranging_counter != CS_RAS_INVALID_RANGING_COUNTER) {
    // Slot is in use
    if (local_connection->cs_procedures[*result].ranging_counter != ranging_counter
        && local_connection->cs_procedures[*result].empty == false) {
      return SL_STATUS_HAS_OVERFLOWED;
    }
    // Slot is free, save the ranging counter
    local_connection->cs_procedures[*result].ranging_counter = ranging_counter;
  }

  return SL_STATUS_OK;
}

static cs_ras_conn_t *get_connection(uint8_t conn_handle)
{
  for (uint8_t i = 0u; i < CS_RAS_MAX_CONNECTIONS; i++) {
    if (cs_ras_connections[i].conn_handle == conn_handle) {
      return &cs_ras_connections[i];
    }
  }

  for (uint8_t i = 0u; i < CS_RAS_MAX_CONNECTIONS; i++) {
    if (cs_ras_connections[i].conn_handle == SL_BT_INVALID_CONNECTION_HANDLE) {
      cs_ras_connections[i].conn_handle = conn_handle;
      cs_ras_connections[i].filter = CS_RAS_NO_FILTER;
      cs_ras_connections[i].slot_written = 0;
      cs_ras_connections[i].write_index = cs_ras_connections[i].conn_buffer;
      for (uint8_t j = 0; j < CS_RAS_PROCEDURE_PER_CONNECTION; j++) {
        cs_ras_connections[i].cs_procedures[j].ranging_counter =
          CS_RAS_INVALID_RANGING_COUNTER;
      }
      cs_ras_server_log_info(CONN_PREFIX "New connection added" LOG_NL, conn_handle);
      return &cs_ras_connections[i];
    }
  }

  return NULL;
}

static sl_status_t add_data(cs_ras_conn_t *connection, uint8_t slot,
                            const uint8_t *src, size_t size, bool save_ptr)
{
  if (connection == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if ( src == NULL) {
    cs_ras_server_log_debug(CONN_PREFIX "Source is NULL" LOG_NL, connection->conn_handle);
    return SL_STATUS_NULL_POINTER;
  }

  if (size == 0) {
    cs_ras_server_log_debug(CONN_PREFIX "Source is empty" LOG_NL, connection->conn_handle);
    return SL_STATUS_EMPTY;
  }

  // Test if we have enough space in the buffer.
  if (CS_PROCEDURE_MAX_LEN < size) {
    return SL_STATUS_WOULD_OVERFLOW;
  }

  // Test if we going to overflow.
  if (connection->write_index - connection->conn_buffer + size > CS_PROCEDURE_MAX_LEN) {
    if (connection->cs_procedures[slot].data_len > 0) {
      memcpy(connection->conn_buffer, connection->cs_procedures[slot].data, connection->cs_procedures[slot].data_len);
    }
    connection->write_index = connection->conn_buffer + connection->cs_procedures[slot].data_len;
    connection->cs_procedures[slot].data = connection->conn_buffer;
  }

  if (save_ptr) {
    connection->cs_procedures[slot].data = connection->write_index;
  }

  for (uint32_t input_data_index = 0u; input_data_index < size; input_data_index++) {
    // Check if we overwrite any other data
    for (uint32_t i = 0u; i < CS_RAS_PROCEDURE_PER_CONNECTION; i++) {
      if (i == slot) {
        continue;
      }
      if (connection->cs_procedures[i].empty == false
          && connection->write_index < connection->cs_procedures[i].data
          + connection->cs_procedures[i].data_len
          && connection->write_index >= connection->cs_procedures[i].data) {
        connection->cs_procedures[i].overwritten = true;
      }
    }
    *connection->write_index = src[input_data_index];
    connection->write_index++;
    connection->cs_procedures[slot].data_len++;
    connection->cs_procedures[slot].empty = false;
  }

  for (uint8_t i = 0; i < CS_RAS_PROCEDURE_PER_CONNECTION; i++) {
    if (connection->cs_procedures[i].overwritten) {
      cs_ras_send_overwritten(connection->conn_handle,
                              connection->cs_procedures[i].ranging_counter);
      connection->cs_procedures[i].overwritten = false;
    }
  }

  return SL_STATUS_OK;
}

static void clear_slot(cs_procedure_data_t *slot)
{
  slot->empty = true;
  slot->data_len = 0;
  slot->overwritten = false;
  slot->sent = false;
  slot->ranging_counter = CS_RAS_INVALID_RANGING_COUNTER;
}

static void clear_connections(void)
{
  for (uint8_t i = 0u; i < CS_RAS_MAX_CONNECTIONS; i++) {
    cs_ras_server_database_clear_connection(cs_ras_connections[i].conn_handle);
  }
}

static void clear_filter(void)
{
  for (uint32_t i = 0u; i < CS_RAS_MAX_CONNECTIONS; i++) {
    cs_ras_connections[i].conn_handle = SL_BT_INVALID_CONNECTION_HANDLE;
    cs_ras_connections[i].filter = CS_RAS_NO_FILTER;       // I.e. allow all.
  }
}
