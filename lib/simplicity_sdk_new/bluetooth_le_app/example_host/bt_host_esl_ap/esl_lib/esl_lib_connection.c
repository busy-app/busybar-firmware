/***************************************************************************//**
 * @file
 * @brief ESL Access Point Connection source file.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include "sl_status.h"
#include "sl_slist.h"
#include "esl_lib.h"
#include "esl_lib_core.h"
#include "esl_lib_connection.h"
#include "esl_lib_event_list.h"
#include "esl_lib_command_list.h"
#include "esl_lib_image_transfer.h"
#include "esl_lib_filter_accept_list.h"
#include "esl_lib_pawr.h"
#include "esl_lib_storage.h"
#include "esl_lib_memory.h"

// -----------------------------------------------------------------------------
// Definitions

#define CENTRAL_ROLE          1
#define GATT_OVERHEAD         7
#define PREFERRED_PHY         sl_bt_gap_phy_2m
#define ACCEPTED_PHY          sl_bt_gap_phy_any
#define CLOSE_TIMEOUT_MS      2000
#define CONNECTION_TIMEOUT_MS 10000
#define BONDING_TIMEOUT_MS    15000
#define RECONNECT_TIMEOUT_MS  250
#define GATT_TIMEOUT_MS       10000

// connection parameters for PAST
#define PAST_CONN_INTERVAL_MIN       ESL_LIB_CONN_INTERVAL_MIN
#define PAST_CONN_INTERVAL_MAX       ESL_LIB_CONN_INTERVAL_MAX
#define PAST_CONN_PERIPHERAL_LATENCY 2      // allow to skip two connection interval during PAST if there's no data
#define PAST_CONN_DEFAULT_TIMEOUT    1000   // value * 10ms, this is 10 seconds
#define PAST_CONN_MIN_TIMEOUT        ESL_LIB_CONN_MIN_TIMEOUT
#define PAST_CONN_MAX_TIMEOUT        ESL_LIB_CONN_MAX_TIMEOUT
#define PAST_CONN_MIN_CE_LENGTH      ESL_LIB_CONN_MIN_CE_LENGTH
#define PAST_CONN_MAX_CE_LENGTH      ESL_LIB_CONN_MAX_CE_LENGTH // can be tuned to fits specific radio timing needs
#define PAST_GRACE_INTERVAL_COUNT    6

#define PAWR_SERVICE_DATA            42

#define SCAN_INIT_INTERVAL           28
#define CONN_FMT                     ESL_LIB_LOG_HANDLE_FORMAT

/// GATT Database handles
typedef struct {
  struct {
    sl_bt_uuid_16_t esl;
    sl_bt_uuid_16_t ots;
    sl_bt_uuid_16_t dis;
  } services;
  sl_bt_uuid_16_t esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_SIZE];
  sl_bt_uuid_16_t dis_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_DIS_SIZE];
} esl_lib_connection_uuids_t;

// -----------------------------------------------------------------------------
// Forward declaration of private functions

static void esl_lib_connection_safe_remove_ptr(esl_lib_connection_t *ptr);
static void run_command(esl_lib_command_list_cmd_t *cmd);
static sl_status_t close_connection(esl_lib_connection_t *conn);
static sl_status_t send_retry_event(esl_lib_connection_t *conn,
                                    sl_status_t          reason);
static sl_status_t send_connection_status(esl_lib_connection_t *conn,
                                          esl_lib_bool_t       status,
                                          sl_status_t          reason);
static sl_status_t send_cp_notification_event(esl_lib_connection_t *conn,
                                              size_t               len,
                                              void                 *data);
static sl_status_t send_bonding_data(esl_lib_connection_t *conn,
                                     uint8_t              *data);
static sl_status_t send_bonding_finished(esl_lib_connection_t *conn);
static sl_status_t send_att_response(esl_lib_connection_t *conn,
                                     esl_lib_evt_type_t   type,
                                     sl_status_t          status);
static sl_status_t send_connection_error(esl_lib_connection_t        *conn,
                                         esl_lib_status_t            lib_status,
                                         sl_status_t                 status,
                                         esl_lib_connection_state_t  data);

static void gatt_timeout(app_timer_t *timer,
                         void        *data);
static void connection_timeout(app_timer_t *timer,
                               void *data);
static void reconnect_timeout(app_timer_t *timer,
                              void        *data);
static bool uuid_16_match(uint8_t *uuid_a, uint8_t *uuid_b);
static void connection_complete(esl_lib_connection_t *conn, sl_status_t result);
static bool is_eslp_ready(esl_lib_connection_t *conn);
static bool check_image_transfer(esl_lib_connection_t *conn);
static void on_image_transfer_type_arrived(esl_lib_image_transfer_handle_t handle,
                                           uint8_t                         connection,
                                           sl_status_t                     result,
                                           uint8_t                         image_index,
                                           uint8_t                         *object_type,
                                           uint8_t                         len);
static void on_image_transfer_finished(esl_lib_image_transfer_handle_t handle,
                                       uint8_t                         connection,
                                       sl_status_t                     result,
                                       uint8_t                         image_index);
static void on_image_transfer_status(esl_lib_image_transfer_handle_t handle,
                                     uint8_t                         connection,
                                     esl_image_transfer_state_t      state,
                                     sl_status_t                     result,
                                     esl_lib_ots_gattdb_handles_t    *gattdb_handles);
static sl_status_t get_next_tag_info(esl_lib_connection_t *conn);
static sl_status_t get_tag_info_finish(esl_lib_connection_t *conn, sl_status_t status);
static esl_lib_data_type_t get_next_type(esl_lib_data_type_t type);
static uint16_t get_handle_for_type(esl_lib_connection_t *conn,
                                    esl_lib_data_type_t  tag_info_type);
static void clean_tag_info(esl_lib_connection_t *conn);
static sl_status_t save_tag_info(esl_lib_connection_t *conn);
static sl_status_t write_value(esl_lib_connection_t *conn,
                               esl_lib_bool_t       response,
                               esl_lib_data_type_t  type,
                               uint32_t             len,
                               uint8_t              *data);
static sl_status_t write_next_config_value(esl_lib_connection_t *conn);
static bool find_tlv(esl_lib_command_list_cmd_t  *cmd,
                     esl_lib_connect_data_type_t type,
                     esl_lib_connect_tlv_t       **tlv_out);
static void *close_broken_connection(esl_lib_connection_t **conn, esl_lib_address_t *backup);
static sl_status_t esl_lib_initiate_auto_connection(esl_lib_connection_t *handle);
static void esl_lib_connection_emit_mass_errors(esl_lib_connection_t *conn);
// -----------------------------------------------------------------------------
// Private variables

// List of connections
static sl_slist_node_t *connection_list = NULL;

// Level1 Filter Accept List for bookkeeping of all acceptable ESLs of the ESL library level (i.e., all requested)
static filter_list_p auto_acceptance_list = NULL;

// Level2 Filter Accept List for Stack-Level Initiator Filter Policy (i.e., those that are actively scanned)
static filter_list_p auto_initiator_list = NULL;

// Connection mode
static esl_lib_connection_mode_t connection_mode = ESL_LIB_CONNECTION_MODE_SINGLE;

// Foreign initiator identity
static bool foreign_initiator_id = false;

// Last IO capability known to the Bluetooth stack
static sl_bt_sm_io_capability_t last_io_capabilities = sl_bt_sm_io_capability_noinputnooutput;

// Service UUIDs
static const esl_lib_connection_uuids_t uuid_map = {
  .services.esl.data = ESL_SERVICE_UUID,   // ESL Service UUID
  .services.ots.data = { 0x25, 0x18 },     // OTS Service UUID
  .services.dis.data = { 0x0A, 0x18 },     // Device Information Service UUID
  .esl_characteristics = {
    { .data = { 0xF6, 0x2B } },            // ESL_LIB_CHARACTERISTIC_INDEX_ESL_ADDRESS
    { .data = { 0xF7, 0x2B } },            // ESL_LIB_CHARACTERISTIC_INDEX_AP_SYNC_KEY_MATERIAL
    { .data = { 0xF8, 0x2B } },            // ESL_LIB_CHARACTERISTIC_INDEX_ESL_RESPONSE_KEY_MATERIAL
    { .data = { 0xF9, 0x2B } },            // ESL_LIB_CHARACTERISTIC_INDEX_ESL_CURRENT_ABSOLUTE_TIME
    { .data = { 0xFA, 0x2B } },            // ESL_LIB_CHARACTERISTIC_INDEX_ESL_DISPLAY_INFORMATION
    { .data = { 0xFB, 0x2B } },            // ESL_LIB_CHARACTERISTIC_INDEX_ESL_IMAGE_INFORMATION
    { .data = { 0xFC, 0x2B } },            // ESL_LIB_CHARACTERISTIC_INDEX_ESL_SENSOR_INFORMATION
    { .data = { 0xFD, 0x2B } },            // ESL_LIB_CHARACTERISTIC_INDEX_ESL_LED_INFORMATION
    { .data = { 0xFE, 0x2B } }             // ESL_LIB_CHARACTERISTIC_INDEX_ESL_CONTROL_POINT
  },
  .dis_characteristics = {
    { .data = { 0x50, 0x2A } },            // ESL_LIB_CHARACTERISTIC_INDEX_DIS_PNP_ID
    { .data = { 0x25, 0x2A } }             // ESL_LIB_CHARACTERISTIC_INDEX_DIS_SN
  }
};

// -----------------------------------------------------------------------------
// Public functions

esl_lib_connection_mode_t esl_lib_get_connection_mode_and_status(esl_lib_core_state_t *core_state,
                                                                 uint8_t *connections,
                                                                 bool *initiator_busy)
{
  esl_lib_core_state_t core_state_mock;
  uint8_t handle_count = 0;

  if (core_state == NULL) {
    (void)core_state_mock;         // Suppress unused variable warning...
    core_state = &core_state_mock; // ...yet allow NULL input parameter for core_state
  }

  *core_state = ESL_LIB_CORE_STATE_IDLE;

  if (!sl_slist_is_empty(connection_list)) {
    esl_lib_connection_t *entry;

    // Stack is busy while initiating a connection
    // Otherwise, the ESL library core is considered "idle" (i.e is ready to process requests)
    SL_SLIST_FOR_EACH_ENTRY(connection_list, entry, esl_lib_connection_t, node) {
      handle_count++;
      if (entry->connection_handle != SL_BT_INVALID_CONNECTION_HANDLE && entry->state == ESL_LIB_CONNECTION_STATE_CONNECTING) {
        *core_state = ESL_LIB_CORE_STATE_CONNECTING;
      }
    }
  }

  if (connections != NULL) {
    *connections = handle_count;
  }

  if (initiator_busy != NULL) {
    if (connection_mode == ESL_LIB_CONNECTION_MODE_SINGLE) {
      *initiator_busy = false;
    } else {
      *initiator_busy = filter_accept_list_get_size(auto_initiator_list) < filter_accept_list_get_size(auto_acceptance_list);
    }
  }

  return connection_mode;
}

sl_status_t esl_lib_change_connection_mode(esl_lib_connection_mode_t mode)
{
  sl_status_t sc = SL_STATUS_BUSY;
  esl_lib_core_state_t core_status;

  if (mode == esl_lib_get_connection_mode_and_status(&core_status, NULL, NULL)) {
    // Do nothing if the requested mode is the same as the current mode.
    return SL_STATUS_OK;  // Report as if it had just been set
  } else if (core_status != ESL_LIB_CORE_STATE_CONNECTING) {
    // Make sure we have a clean mode switch by clearing the Filter Acceptance list first.
    sc = esl_lib_initiator_filter_cleanup();

    if (sc == SL_STATUS_OK) {
      connection_mode = mode;
    }
  }
  return sc;
}

size_t esl_lib_get_initiator_filter_size(void)
{
  return filter_accept_list_get_size(auto_initiator_list);
}

sl_status_t esl_lib_connection_check_gattdb_handles(esl_lib_gattdb_handles_t *gattdb_handles)
{
  if (gattdb_handles != NULL) {
    if (gattdb_handles->services.esl == ESL_LIB_INVALID_SERVICE_HANDLE) {
      return SL_STATUS_INVALID_HANDLE;
    } else {
      // ESL service present, check mandatory fields
      if (gattdb_handles->esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_ADDRESS]
          == ESL_LIB_INVALID_CHARACTERISTIC_HANDLE) {
        return SL_STATUS_INVALID_HANDLE;
      }
      if (gattdb_handles->esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_AP_SYNC_KEY_MATERIAL]
          == ESL_LIB_INVALID_CHARACTERISTIC_HANDLE) {
        return SL_STATUS_INVALID_HANDLE;
      }
      if (gattdb_handles->esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_RESPONSE_KEY_MATERIAL]
          == ESL_LIB_INVALID_CHARACTERISTIC_HANDLE) {
        return SL_STATUS_INVALID_HANDLE;
      }
      if (gattdb_handles->esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_CURRENT_ABSOLUTE_TIME]
          == ESL_LIB_INVALID_CHARACTERISTIC_HANDLE) {
        return SL_STATUS_INVALID_HANDLE;
      }
      if (gattdb_handles->esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_CONTROL_POINT]
          == ESL_LIB_INVALID_CHARACTERISTIC_HANDLE) {
        return SL_STATUS_INVALID_HANDLE;
      }
    }
  } else {
    return SL_STATUS_NULL_POINTER;
  }
  return SL_STATUS_OK;
}

sl_status_t esl_lib_initiate_connection(esl_lib_command_list_cmd_t *cmd)
{
  sl_status_t             sc                = SL_STATUS_OK;
  uint8_t                 address_type      = sl_bt_gap_public_address;
  uint8_t                 connection_handle = SL_BT_INVALID_CONNECTION_HANDLE;
  sl_bt_sm_io_capability_t io_capabilities  = sl_bt_sm_io_capability_noinputnooutput;
  uint8_t                 flags             = 0;
  esl_lib_connection_t    *conn             = ESL_LIB_INVALID_HANDLE;
  esl_lib_address_t       *identity         = NULL;
  esl_lib_address_t       *address          = NULL;
  bd_addr                 *identity_addr    = NULL;
  bd_addr                 *addr             = NULL;
  esl_lib_connect_tlv_t   *tlv              = NULL;
  esl_lib_pawr_t          *pawr             = NULL;

  if (cmd == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (cmd->cmd_code != ESL_LIB_CMD_CONNECT) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (cmd->data.cmd_connect.retries_left <= ESL_LIB_CONNECTION_RETRY_COUNT_MAX) {
    // it's a retry attempt if handle is not an invalid one
    uint8_t retry_count = 1 + ESL_LIB_CONNECTION_RETRY_COUNT_MAX - cmd->data.cmd_connect.retries_left;

    esl_lib_log_connection_warning("Retry %d / %d connect to " ESL_LIB_LOG_ADDR_FORMAT APP_LOG_NL,
                                   retry_count,
                                   ESL_LIB_CONNECTION_RETRY_COUNT_MAX,
                                   ESL_LIB_LOG_ADDR(cmd->data.cmd_connect.address));
  } else {
    esl_lib_log_connection_debug("Initiate new connection" APP_LOG_NL);
  }

  if (cmd->data.cmd_connect.retries_left) {
    // Set address
    address = &cmd->data.cmd_connect.address;
    addr = (bd_addr *)address->addr;
    address_type = address->address_type;

    // Check for identity
    if (find_tlv(cmd, ESL_LIB_CONNECT_DATA_TYPE_IDENTITY_ADDRESS, &tlv)) {
      identity = (esl_lib_address_t*)tlv->data.data;
      esl_lib_log_connection_debug("Setting identity to " ESL_LIB_LOG_ADDR_FORMAT APP_LOG_NL,
                                   ESL_LIB_LOG_ADDR(*identity));
      identity_addr = (bd_addr *)identity;
      sc = sl_bt_gap_set_identity_address(*identity_addr, identity->address_type);
      if (sc != SL_STATUS_OK) {
        esl_lib_log_connection_error("Failed to set foreign identity address, sc = 0x%04x" APP_LOG_NL, sc);
        return sc;
      }
      foreign_initiator_id = true;
    } else if (foreign_initiator_id) {
      bd_addr default_identity;
      uint8_t default_address_type;

      sc = esl_lib_core_get_identity_address(&default_identity, &default_address_type);

      if (sc == SL_STATUS_OK) {
        sc = sl_bt_gap_set_identity_address(default_identity, default_address_type);
      }

      if (sc != SL_STATUS_OK) {
        esl_lib_log_connection_error("Identity address recovery failed, sc = 0x%04x" APP_LOG_NL, sc);
        return sc;
      }
      foreign_initiator_id = false;
    }

    // Check key specified
    if (find_tlv(cmd, ESL_LIB_CONNECT_DATA_TYPE_PASSKEY, &tlv)) {
      esl_lib_log_connection_debug("Use passkey for pairing with " ESL_LIB_LOG_ADDR_FORMAT APP_LOG_NL,
                                   ESL_LIB_LOG_ADDR(*address));
      io_capabilities = sl_bt_sm_io_capability_keyboardonly;
      flags |= SL_BT_SM_CONFIGURATION_MITM_REQUIRED;
    }
    if (find_tlv(cmd, ESL_LIB_CONNECT_DATA_TYPE_OOB_DATA, &tlv)) {
      // No action, use remote OOB data only
      esl_lib_log_connection_debug("Use OOB data for pairing with " ESL_LIB_LOG_ADDR_FORMAT APP_LOG_NL,
                                   ESL_LIB_LOG_ADDR(*address));
    }
    if (find_tlv(cmd, ESL_LIB_CONNECT_DATA_TYPE_LTK, &tlv)) {
      // No new bonding required
      esl_lib_log_connection_debug("Use LTK for connection with " ESL_LIB_LOG_ADDR_FORMAT APP_LOG_NL,
                                   ESL_LIB_LOG_ADDR(*address));
    } else {
      esl_lib_log_connection_debug("Expect new bonding with " ESL_LIB_LOG_ADDR_FORMAT APP_LOG_NL,
                                   ESL_LIB_LOG_ADDR(*address));
    }

    if (last_io_capabilities != io_capabilities) {
      sc = sl_bt_sm_configure(flags, io_capabilities);

      if (sc != SL_STATUS_OK) {
        esl_lib_log_connection_error("Failed to configure SM, sc = 0x%04x" APP_LOG_NL, sc);
        return sc;
      } else {
        last_io_capabilities = io_capabilities;
      }
    }

    // Search for re-usable esl_lib_connection_t type connection handle, ignore status
    (void)esl_lib_connection_find(SL_BT_INVALID_CONNECTION_HANDLE, &conn);
    // Adjust retry count on every actual connection request
    --(cmd->data.cmd_connect.retries_left);
    // Check PAwR presense to decide connection method
    if (find_tlv(cmd, ESL_LIB_CONNECT_DATA_TYPE_PAWR, &tlv)) {
      // Connect using PAwR
      esl_lib_pawr_subevent_t *pawr_sub = (esl_lib_pawr_subevent_t *)tlv->data.data;
      pawr = (esl_lib_pawr_t *)pawr_sub->handle;
      esl_lib_log_connection_debug("Opening connection via PAwR handle %d subevent %d to " ESL_LIB_LOG_ADDR_FORMAT APP_LOG_NL,
                                   pawr->pawr_handle,
                                   pawr_sub->subevent,
                                   ESL_LIB_LOG_ADDR(*address));
      sc = sl_bt_pawr_advertiser_create_connection(pawr->pawr_handle,
                                                   pawr_sub->subevent,
                                                   *addr,
                                                   address_type,
                                                   &connection_handle);
    } else if (connection_mode == ESL_LIB_CONNECTION_MODE_SINGLE
               || io_capabilities == sl_bt_sm_io_capability_keyboardonly
               || identity != NULL) {
      // Connect using the address only
      esl_lib_log_connection_debug("Opening connection to " ESL_LIB_LOG_ADDR_FORMAT APP_LOG_NL,
                                   ESL_LIB_LOG_ADDR(*address));
      sc = sl_bt_connection_open(*addr,
                                 address_type,
                                 sl_bt_gap_phy_1m,
                                 &connection_handle);
    } else {
      // Defer connection by adding the request to the Filter Acccept List
      esl_lib_log_connection_debug("Accept connection to " ESL_LIB_LOG_ADDR_FORMAT APP_LOG_NL,
                                   ESL_LIB_LOG_ADDR(*address));
      // Go to a different code path when requesting to connect in auto mode!
      size_t accept_list_size = filter_accept_list_get_size(auto_acceptance_list);

      if (accept_list_size < (1 << (ESL_LIB_SKIPLIST_MAX_LEVEL_LIB))) {
        (void)filter_accept_list_insert_command(auto_acceptance_list, cmd);
        esl_lib_core_connection_complete();
        return esl_lib_initiate_auto_connection(conn);
      } else {
        sc = SL_STATUS_BT_CTRL_CONNECTION_LIMIT_EXCEEDED;
      }
    }

    if (sc == SL_STATUS_OK) {
      // If not retry (i.e., no reusable connection found), create a new connection
      if (conn == ESL_LIB_INVALID_HANDLE) {
        // Allocate and add the connection to the connection list.
        sc = esl_lib_connection_add(connection_handle, &conn);
        if (sc == SL_STATUS_OK) {
          // Move to connecting state with no error present.
          conn->state = ESL_LIB_CONNECTION_STATE_CONNECTING;

          // Pass the ownership of initial connect command to conn. handle on success - otherwise ap_core will free it
          conn->command = cmd;

          // Set BLE address if available (for manual connection requests)
          conn->address_type = address_type;
          memcpy(conn->address.addr,
                 addr->addr,
                 sizeof(conn->address.addr));

          conn->command_complete = false;
          esl_lib_log_connection_debug(CONN_FMT "Pending new connection to " ESL_LIB_LOG_ADDR_FORMAT APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       ESL_LIB_LOG_ADDR(*address));
        } else if (connection_handle == SL_BT_INVALID_CONNECTION_HANDLE) { // a.k.a this is an auto mode connect request
          (void)sl_bt_accept_list_remove_device_by_address(*addr,
                                                           address_type);
          esl_lib_log_connection_error(CONN_FMT "Failed to make a pending new connection to " ESL_LIB_LOG_ADDR_FORMAT APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       ESL_LIB_LOG_ADDR(*address));
        } else {
          (void)sl_bt_connection_close(connection_handle);
          esl_lib_log_connection_error("Internal ESL library error, sc = 0x%04x while connecting to " ESL_LIB_LOG_ADDR_FORMAT APP_LOG_NL,
                                       sc,
                                       ESL_LIB_LOG_ADDR(*address));
        }
      } else {
        // Pass the connect command back to the existing conn. handle (link removed earlier by reconnect_timeout CB).
        conn->command = cmd;
        conn->command_complete = false;
        conn->connection_handle = connection_handle; // Old conn->connection_handle is invalid at this point: replace!
        conn->last_error = SL_STATUS_IN_PROGRESS; // Operation is in progress and not yet complete (pass or fail)
        conn->state = ESL_LIB_CONNECTION_STATE_CONNECTING;
        conn->established = false;

        sc = app_timer_start(&conn->timer,
                             CONNECTION_TIMEOUT_MS,
                             connection_timeout,
                             conn,
                             false);
        if (sc != SL_STATUS_OK) {
          esl_lib_log_connection_error("Connection timeout reinit failed, handle = %u as 0x%p, sc = 0x%04x. Closing." APP_LOG_NL,
                                       conn->connection_handle,
                                       conn,
                                       sc);
          (void)close_connection(conn);
        }
      }
    } else {
      esl_lib_log(((sc == SL_STATUS_BT_CTRL_CONNECTION_LIMIT_EXCEEDED       // connection handle limit reached
                    || sc == SL_STATUS_BT_CTRL_MEMORY_CAPACITY_EXCEEDED)    // filter accept list full result
                   ? ESL_LIB_LOG_LEVEL_WARNING : ESL_LIB_LOG_LEVEL_ERROR),
                  ESL_LIB_LOG_MODULE_CONNECTION,
                  "Connection request failed to " ESL_LIB_LOG_ADDR_FORMAT ", sc = 0x%04x" APP_LOG_NL,
                  ESL_LIB_LOG_ADDR(*address),
                  sc);
      if (conn != NULL) {
        // Let the caller free the command in case of error
        if (conn->command == cmd) {
          conn->command = NULL;
        }
        (void)esl_lib_connection_remove_ptr(conn);
      }
    }
  } else if (conn == ESL_LIB_INVALID_HANDLE) {
    // Just to avoid any NULL dereferencing below - otherwise this code path is implausible:
    // cmd->data.cmd_connect.retries_left shall never be initialized to zero!
    sc =  SL_STATUS_NOT_READY;
  } else {
    esl_lib_log_connection_error("Connection failure, no more retry attempts" APP_LOG_NL);
    sc = SL_STATUS_BT_CTRL_CONNECTION_FAILED_TO_BE_ESTABLISHED;
    // Let the caller free the command in case of error
    if (conn->command == cmd) {
      conn->command = NULL;
    }
    // Force removal of connection handle in case of no more retry
    (void)esl_lib_connection_remove_ptr(conn);
  }
  return sc;
}

sl_status_t esl_lib_connection_add(uint8_t                  conn,
                                   esl_lib_connection_t     **ptr_out)
{
  sl_status_t sc;
  esl_lib_connection_t *ptr;

  if (ptr_out == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  esl_lib_log_connection_debug("Add ESL library handle for BLE connection handle = %d" APP_LOG_NL, conn);

  // Check if it exists
  sc = esl_lib_connection_find(conn, &ptr);
  if (sc == SL_STATUS_OK) {
    sc = SL_STATUS_ALREADY_EXISTS;
    *ptr_out = ptr;
  } else {
    ptr = (esl_lib_connection_t *)esl_lib_memory_allocate(sizeof(esl_lib_connection_t));
    if (ptr != NULL) {
      memset(ptr, 0, sizeof(*ptr));
      ptr->connection_handle = conn;
      ptr->command_complete = true;
      ptr->last_error = SL_STATUS_IN_PROGRESS; // Operation is in progress and not yet complete (pass or fail)
      *ptr_out = ptr;

      sc = app_timer_start(&ptr->timer,
                           CONNECTION_TIMEOUT_MS,
                           connection_timeout,
                           ptr,
                           false);
      if (sc == SL_STATUS_OK) {
        esl_lib_log_connection_debug(CONN_FMT "Added ESL library handle for connection handle = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(ptr),
                                     conn);
      }
    } else {
      sc = SL_STATUS_ALLOCATION_FAILED;
      // Close connection
      (void)sl_bt_connection_close(conn);
    }
  }
  if (sc != SL_STATUS_OK) {
    esl_lib_log_connection_error("Add connection handle = %u failed = 0x%04x" APP_LOG_NL, conn, sc);
  } else {
    sl_slist_push(&connection_list, &ptr->node);
  }

  return sc;
}

sl_status_t esl_lib_connection_find(uint8_t              conn,
                                    esl_lib_connection_t **ptr_out)
{
  sl_status_t sc = SL_STATUS_NOT_FOUND;
  esl_lib_connection_t *ptr;

  if (ptr_out == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  SL_SLIST_FOR_EACH_ENTRY(connection_list, ptr, esl_lib_connection_t, node) {
    if (conn == ptr->connection_handle) {
      *ptr_out = ptr;
      return SL_STATUS_OK;
    }
  }

  *ptr_out = ESL_LIB_INVALID_HANDLE;
  return sc;
}

sl_status_t esl_lib_connection_remove_ptr(esl_lib_connection_t *ptr)
{
  if (esl_lib_connection_contains(ptr)) {
    esl_lib_connection_safe_remove_ptr(ptr);
  } else {
    esl_lib_log_connection_warning(CONN_FMT "Failed to remove the connection: handle not found" APP_LOG_NL, ESL_LIB_LOG_PTR(ptr));
  }

  return SL_STATUS_OK;
}

bool esl_lib_connection_contains(esl_lib_connection_t *ptr)
{
  bool ret = false;
  esl_lib_connection_t *iterator;

  SL_SLIST_FOR_EACH_ENTRY(connection_list, iterator, esl_lib_connection_t, node) {
    if (iterator == ptr) {
      return true;
    }
  }

  return ret;
}

void esl_lib_connection_cleanup(void)
{
  esl_lib_connection_t *conn;
  // Clean connection list
  while ((conn = (esl_lib_connection_t *)sl_slist_pop(&connection_list)) != NULL) {
    // Close connection
    (void)close_connection(conn);
    (void)esl_lib_connection_safe_remove_ptr(conn);
  }
  esl_lib_log_connection_debug("Connection cleanup complete" APP_LOG_NL);
}

sl_status_t esl_lib_initiator_filter_cleanup(void)
{
  sl_status_t sc;
  // Clean Filter Accept List local requests first
  filter_accept_list_clear_list(auto_acceptance_list);
  // Clean Filter Accept List local copy
  filter_accept_list_clear_list(auto_initiator_list);
  // Send appropriate clear command to NCP
  sc = sl_bt_accept_list_remove_all_devices();
  esl_lib_log(sc == SL_STATUS_OK ? ESL_LIB_LOG_LEVEL_DEBUG : ESL_LIB_LOG_LEVEL_ERROR,
              ESL_LIB_LOG_MODULE_CONNECTION,
              "Initiator Policy Filter Accept list cleanup complete with status 0x%04x" APP_LOG_NL,
              sc);
  return sc;
}

void esl_lib_auto_initiator_deinit(void)
{
  (void)esl_lib_initiator_filter_cleanup();
  filter_accept_list_destroy_list(&auto_acceptance_list);
  filter_accept_list_destroy_list(&auto_initiator_list);
}

sl_status_t esl_lib_connection_add_command(esl_lib_connection_t       *conn,
                                           esl_lib_command_list_cmd_t *cmd)
{
  sl_status_t sc;
  if (conn == ESL_LIB_INVALID_HANDLE) {
    return SL_STATUS_NULL_POINTER;
  }
  if (!esl_lib_connection_contains(conn)) {
    return SL_STATUS_BT_CTRL_UNKNOWN_CONNECTION_IDENTIFIER;
  }
  sc = esl_lib_command_list_put(&conn->command_list, cmd);
  if (sc == SL_STATUS_OK) {
    esl_lib_log_connection_debug(CONN_FMT "Added command %d" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(conn),
                                 cmd->cmd_code);
  } else {
    esl_lib_log_connection_error(CONN_FMT "Add command %d failed, sc = 0x%04x" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(conn),
                                 cmd->cmd_code,
                                 sc);
  }
  return sc;
}

void esl_lib_connection_step(void)
{
  esl_lib_connection_t       *conn;
  esl_lib_command_list_cmd_t *cmd;

  SL_SLIST_FOR_EACH_ENTRY(connection_list, conn, esl_lib_connection_t, node) {
    if (conn->command_complete) {
      // If there is a running but complete command, remove it - except for ESL_LIB_CMD_CONNECT requests,
      // which are inherited from esl_lib_core and are therefore handled slightly differently.
      if ((conn->command != NULL) && (conn->command->cmd_code != ESL_LIB_CMD_CONNECT)) {
        esl_lib_command_list_remove(&conn->command_list, conn->command);
        conn->command = NULL;
      }
      // Move and execute next command.
      cmd = esl_lib_command_list_get(&conn->command_list);
      if (cmd != NULL) {
        esl_lib_log_connection_debug(CONN_FMT "Running next command: %d, id = %#04" PRIx32 APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     cmd->cmd_code,
                                     cmd->id);
        conn->command = cmd;
        conn->command_complete = false;
        run_command(conn->command);
      }
    }
  }
}

void esl_lib_connection_on_bt_event(sl_bt_msg_t *evt)
{
  esl_lib_connection_t  *conn            = NULL;
  uint8_t               *bonding_data    = NULL;
  esl_lib_address_t     *addr            = NULL;
  esl_lib_pawr_t        *pawr            = NULL;
  esl_lib_connect_tlv_t *tlv             = NULL;
  esl_lib_status_t      lib_status       = ESL_LIB_STATUS_UNSPECIFIED_ERROR;
  sl_status_t           sc               = SL_STATUS_OK;
  uint8_t               bonding_data_len = 0;
  esl_lib_address_t     addr_backup;

  switch (SL_BT_MSG_ID(evt->header)) {
    // (Re)boot
    case sl_bt_evt_system_boot_id:
      // See esl_lib_core::esl_lib_core_internal_reset() called from its boot event handler for more connection related actions!
      connection_mode = ESL_LIB_CONNECTION_MODE_SINGLE;
      // Do lazy Filter Accept Lists creation
      if (auto_acceptance_list == NULL) {
        // Use "double buffering" to provide instant add / remove user experience.
        auto_acceptance_list = filter_accept_list_create_list(ESL_LIB_SKIPLIST_MAX_LEVEL_LIB,
                                                              filter_accept_list_compare);
      }
      if (auto_initiator_list == NULL) {
        auto_initiator_list = filter_accept_list_create_list(ESL_LIB_SKIPLIST_MAX_LEVEL_STACK,
                                                             filter_accept_list_compare);
      }
      last_io_capabilities = sl_bt_sm_io_capability_noinputnooutput; // set by esl_lib_core boot event handler
      break;
    // Connection
    case sl_bt_evt_connection_opened_id:
      // Filter for central role
      if (evt->data.evt_connection_opened.role == CENTRAL_ROLE) {
        uint64_t peripheral_address_u64 = 0;

        memcpy(&peripheral_address_u64,
               evt->data.evt_connection_opened.address.addr,
               sizeof(evt->data.evt_connection_opened.address.addr));
        esl_lib_log_connection_debug("Connection to %012" PRIx64 " opened in central role, connection handle = %u" APP_LOG_NL,
                                     peripheral_address_u64,
                                     evt->data.evt_connection_opened.connection);
        sc = esl_lib_connection_find(evt->data.evt_connection_opened.connection,
                                     &conn);
        if (sc == SL_STATUS_OK) {
          conn->last_error = app_timer_stop(&conn->timer);
          esl_lib_log_connection_debug(CONN_FMT "Connection found, connection handle = %u" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       conn->connection_handle);
          // Set BLE address
          conn->address_type = evt->data.evt_connection_opened.address_type;
          memcpy(conn->address.addr,
                 evt->data.evt_connection_opened.address.addr,
                 sizeof(conn->address.addr));

          // Check if the opened connection is an auto connect request
          if (conn->command == NULL) {
            // If it is, then get back its connect command from initiator list. Not only will it contain the data we
            // need to secure the connection, but we will also temporarily pass command ownership to the connection handle.
            conn->command = filter_accept_list_remove_command_by_address(auto_initiator_list,
                                                                         &(conn->address),
                                                                         conn->address_type);
            // Also try to remove the address from the link layer accept list, ignoring the result.
            // May return SL_STATUS_BT_CTRL_INVALID_COMMAND_PARAMETERS on try remove item not on the list, but its irrelevant.
            sl_status_t sc = sl_bt_accept_list_remove_device_by_address(conn->address, conn->address_type);
            if (sc != SL_STATUS_OK && sc != SL_STATUS_BT_CTRL_INVALID_COMMAND_PARAMETERS) {
              esl_lib_log_connection_warning(CONN_FMT "Accept list removal failed, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                                             ESL_LIB_LOG_PTR(conn),
                                             conn->connection_handle,
                                             sc);
            }
          }
          // Fallback check on the auto_acceptance_list just in case
          if (conn->command == NULL) {
            conn->command = filter_accept_list_remove_command_by_address(auto_acceptance_list,
                                                                         &(conn->address),
                                                                         conn->address_type);
            esl_lib_log_connection_debug(CONN_FMT "Connection request found on acceptance list, connection handle = %u will close" APP_LOG_NL,
                                         ESL_LIB_LOG_PTR(conn),
                                         conn->connection_handle);
          }
          // Verify again that the command was found (for auto initiator requests, only)
          if (conn->command == NULL) {
            // This happens if the application has withdrawn the request in the meantime, but it could not be done in
            // the stack because it has already occupied the list
            esl_lib_log_connection_debug(CONN_FMT "Connection request withdrawn, connection handle = %u will close" APP_LOG_NL,
                                         ESL_LIB_LOG_PTR(conn),
                                         conn->connection_handle);
            (void)close_connection(conn); // Overrides conn->last_error
            esl_lib_core_connection_complete();
            break;
          }

          // Copy GATT database if present
          if (find_tlv(conn->command, ESL_LIB_CONNECT_DATA_TYPE_GATTDB_HANDLES, &tlv)) {
            conn->gattdb_known = ESL_LIB_TRUE;
            memcpy(&conn->gattdb_handles,
                   tlv->data.data,
                   sizeof(conn->gattdb_handles));
            for (uint8_t i = 0; i < sizeof(conn->gattdb_handles.esl_characteristics) / sizeof(uint16_t); i++) {
              esl_lib_log_connection_debug(CONN_FMT "ESL %u characteristic handle = 0x%02x" APP_LOG_NL,
                                           ESL_LIB_LOG_PTR(conn),
                                           i,
                                           conn->gattdb_handles.esl_characteristics[i]);
            }
            for (uint8_t i = 0; i < sizeof(conn->gattdb_handles.dis_characteristics) / sizeof(uint16_t); i++) {
              esl_lib_log_connection_debug(CONN_FMT "DIS %u characteristic handle = 0x%02x" APP_LOG_NL,
                                           ESL_LIB_LOG_PTR(conn),
                                           i,
                                           conn->gattdb_handles.dis_characteristics[i]);
            }
          } else {
            conn->gattdb_known = ESL_LIB_FALSE;
            // Clean handles if not specified
            conn->gattdb_handles.services.dis = ESL_LIB_INVALID_SERVICE_HANDLE;
            conn->gattdb_handles.services.esl = ESL_LIB_INVALID_SERVICE_HANDLE;
            conn->gattdb_handles.services.ots = ESL_LIB_INVALID_SERVICE_HANDLE;
            for (uint8_t i = 0; i < sizeof(conn->gattdb_handles.dis_characteristics) / sizeof(uint16_t); i++) {
              conn->gattdb_handles.dis_characteristics[i] = ESL_LIB_INVALID_CHARACTERISTIC_HANDLE;
            }
            for (uint8_t i = 0; i < sizeof(conn->gattdb_handles.esl_characteristics) / sizeof(uint16_t); i++) {
              conn->gattdb_handles.esl_characteristics[i] = ESL_LIB_INVALID_CHARACTERISTIC_HANDLE;
            }
            for (uint8_t i = 0; i < sizeof(conn->gattdb_handles.ots_characteristics) / sizeof(uint16_t); i++) {
              conn->gattdb_handles.ots_characteristics[i] = ESL_LIB_INVALID_CHARACTERISTIC_HANDLE;
            }
            conn->ots_handle = ESL_LIB_INVALID_HANDLE;
          }

          // Check OOB
          if (find_tlv(conn->command, ESL_LIB_CONNECT_DATA_TYPE_OOB_DATA, &tlv)) {
            aes_key_128 *remote_random = (aes_key_128 *)&tlv->data.data[0];
            aes_key_128 *remote_confirm = (aes_key_128 *)&tlv->data.data[sizeof(aes_key_128)];
            // Use remote OOB
            sc = sl_bt_sm_set_remote_oob(1, *remote_random, *remote_confirm);
            esl_lib_log_connection_debug(CONN_FMT "Entering OOB data, connection handle = %u" APP_LOG_NL,
                                         ESL_LIB_LOG_PTR(conn),
                                         conn->connection_handle);
            if (sc != SL_STATUS_OK) {
              lib_status = ESL_LIB_STATUS_BONDING_FAILED;
              esl_lib_log_connection_error(CONN_FMT "Failed to set remote OOB data, connection handle = %u" APP_LOG_NL,
                                           ESL_LIB_LOG_PTR(conn),
                                           conn->connection_handle);
            }
          }
          // Restart timer to guard the bonding procedure
          (void)app_timer_start(&conn->timer,
                                BONDING_TIMEOUT_MS,
                                connection_timeout,
                                conn,
                                false);
          esl_lib_core_connection_complete();
          // Connection state must be updated properly before calling esl_lib_initiate_auto_connection()
          conn->state = ESL_LIB_CONNECTION_STATE_CONNECTION_OPENED;
        } else {
          // Suppress error event for unknown connections
          sc = SL_STATUS_OK;
        }
      }
      break;
    case sl_bt_evt_connection_remote_used_features_id:
      sc = esl_lib_connection_find(evt->data.evt_connection_remote_used_features.connection,
                                   &conn);
      if (sc == SL_STATUS_OK && !conn->established) {
        esl_lib_connection_t *reuseable_handle = ESL_LIB_INVALID_HANDLE;
        conn->established = true;
        esl_lib_log_connection_debug(CONN_FMT "Connection established, connection handle = %u, features: %014" PRIx64 APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle,
                                     // Following parameter is 8 bytes in LE order according to in Bluetooth Core Vol 6, Part B, 4.6.
                                     *((uint64_t *)(evt->data.evt_connection_remote_used_features.features.data)));
        // search for re-usable connection handle object
        (void)esl_lib_connection_find(SL_BT_INVALID_CONNECTION_HANDLE, &reuseable_handle);
        (void)esl_lib_initiate_auto_connection(reuseable_handle);
      } else {
        // Suppress error event for unknown connections
        sc = SL_STATUS_OK;
      }
      break;
    case sl_bt_evt_connection_closed_id:
      esl_lib_log_connection_debug("Connection close evt, reason = 0x%04x, connection handle = %u" APP_LOG_NL,
                                   evt->data.evt_connection_closed.reason,
                                   evt->data.evt_connection_closed.connection);
      sc = esl_lib_connection_find(evt->data.evt_connection_closed.connection,
                                   &conn);
      if (sc == SL_STATUS_OK) {
        sl_status_t reason = evt->data.evt_connection_closed.reason;
        const bool pending_connect = (conn->state == ESL_LIB_CONNECTION_STATE_CONNECTING);
        // Stop connection / reconnection timer
        (void)app_timer_stop(&conn->timer);
        (void)app_timer_stop(&conn->gatt_timer);
        esl_lib_log_connection_debug(CONN_FMT "Connection closing, connection handle = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle);
        if (is_eslp_ready(conn)) {
          esl_lib_log_connection_debug(CONN_FMT "Removing connection, connection handle = %u" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       conn->connection_handle);
          // Send disconnected status
          (void)send_connection_status(conn,
                                       ESL_LIB_FALSE,
                                       reason);
          // If close connection command is active
          if ((conn->command != NULL)
              && (conn->command->cmd_code == ESL_LIB_CMD_CLOSE_CONNECTION)) {
            conn->command_complete = true;
          }
          // And also remove connection from the list.
          (void)esl_lib_connection_remove_ptr(conn);
          conn = ESL_LIB_INVALID_HANDLE;
        } else if (conn->command != NULL) {
          // Not connected, check if a retry is required (link issue or bonding issue)
          if ((conn->command->cmd_code == ESL_LIB_CMD_CONNECT)
              && (conn->command->data.cmd_connect.retries_left)
              && ((reason == SL_STATUS_BT_CTRL_CONNECTION_FAILED_TO_BE_ESTABLISHED)
                  || (conn->state == ESL_LIB_CONNECTION_STATE_BONDING_RECOVERY))) {
            esl_lib_log_connection_debug(CONN_FMT "Connection retry scheduled, connection handle = %u" APP_LOG_NL,
                                         ESL_LIB_LOG_PTR(conn),
                                         conn->connection_handle);
            uint32_t timeout = RECONNECT_TIMEOUT_MS;
            // Check for PAwR connect request to set correct timeout
            if (find_tlv(conn->command, ESL_LIB_CONNECT_DATA_TYPE_PAWR, &tlv)) {
              esl_lib_pawr_subevent_t *pawr_sub = (esl_lib_pawr_subevent_t *)tlv->data.data;
              esl_lib_pawr_t *pawr = (esl_lib_pawr_t *)pawr_sub->handle;
              timeout = (pawr->config.adv_interval.max << 1) - (pawr->config.adv_interval.max >> 3); // = 1.5f * (pawr->config.adv_interval.max * 1.25f) [ms]
            }

            if (pending_connect) {
              // If a connection request via PAwR times out, the sl_bt_evt_connection_closed_id
              // event occurs without the preceding sl_bt_evt_connection_opened_id event!
              conn->command_complete = true;
              esl_lib_core_connection_complete();
            }
            // Schedule a reconnection to let tag process previous operation
            sc = app_timer_start(&conn->timer,
                                 timeout,
                                 reconnect_timeout,
                                 conn,
                                 false);
            if (sc != SL_STATUS_OK) {
              (void)send_connection_error(conn,
                                          ESL_LIB_STATUS_CONN_FAILED,
                                          evt->data.evt_connection_closed.reason,
                                          conn->state);
              esl_lib_log_connection_error(CONN_FMT "Failed to start retry timer, connection handle = %u" APP_LOG_NL,
                                           ESL_LIB_LOG_PTR(conn),
                                           conn->connection_handle);
              conn->command_complete = true;
            } else {
              // Let the AP know that the connection is now closed but will retry
              send_retry_event(conn, reason);
              break;
            }
          } else {
            // Check again for the out-of-retries reason - set SL_STATUS_ABORT in case
            if (conn->command->cmd_code == ESL_LIB_CMD_CONNECT && conn->command->data.cmd_connect.retries_left == 0) {
              reason = SL_STATUS_ABORT;
              esl_lib_log_connection_debug(CONN_FMT "No more connect retry for " ESL_LIB_LOG_ADDR_FORMAT ", last handle = %u" APP_LOG_NL,
                                           ESL_LIB_LOG_PTR(conn),
                                           conn->address_type,
                                           (conn->address_type ? "random" : "public"),
                                           ESL_LIB_LOG_BD_ADDR(conn->address),
                                           conn->connection_handle);
            } else if (reason == SL_STATUS_OK && !conn->established && conn->state == ESL_LIB_CONNECTION_STATE_BONDING) {
              // Handle special edge case where the AP tries to create new bond with an ESL already bonded to other AP
              // in which case the Silabs' ESL immediately refuses the connection attempt with an SL_STATUS_OK not
              // giving the actual reason to us (by design behavior of our stack).
              reason = SL_STATUS_BT_CTRL_AUTHENTICATION_FAILURE;
            }
            (void)send_connection_error(conn,
                                        ESL_LIB_STATUS_CONN_FAILED,
                                        reason,
                                        conn->state);
            if (pending_connect) {
              // If a connection request via PAwR times out, the sl_bt_evt_connection_closed_id
              // event occurs without the preceding sl_bt_evt_connection_opened_id event!
              conn->command_complete = true;
              esl_lib_core_connection_complete();
            }
          }
          // And also remove connection from the list in the end.
          (void)esl_lib_connection_remove_ptr(conn);
          conn = ESL_LIB_INVALID_HANDLE;
        } else {
          if (pending_connect && filter_accept_list_get_size(auto_initiator_list)) {
            esl_lib_log_connection_debug(CONN_FMT "Mass timeout occured on closed handle = %u!" APP_LOG_NL,
                                         ESL_LIB_LOG_PTR(conn),
                                         conn->connection_handle);
            (void)sl_bt_accept_list_remove_all_devices();
            esl_lib_connection_emit_mass_errors(conn);
          }
          // Remove connection from the list if it has no associated command (e.g., in case of a canceled auto requests)
          (void)esl_lib_connection_remove_ptr(conn);
          conn = ESL_LIB_INVALID_HANDLE;
        }
        // Try consuming a re-usable object
        (void)esl_lib_connection_find(SL_BT_INVALID_CONNECTION_HANDLE, &conn);
        (void)esl_lib_initiate_auto_connection(conn);
        conn = ESL_LIB_INVALID_HANDLE;
      } else {
        // Suppress error event for unknown / rejected connections
        sc = SL_STATUS_OK;
      }
      break;
    case sl_bt_evt_connection_parameters_id:
      sc = esl_lib_connection_find(evt->data.evt_connection_parameters.connection,
                                   &conn);
      if (sc == SL_STATUS_OK) {
        conn->security    = evt->data.evt_connection_parameters.security_mode;
        esl_lib_log_connection_debug(CONN_FMT "Connection parameters, connection handle = %u, security = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle,
                                     conn->security);
        // Bonding is considered finished when security has elevated.
        if (conn->security > sl_bt_connection_mode1_level1) {
          if (conn->state == ESL_LIB_CONNECTION_STATE_BONDING
              || conn->state == ESL_LIB_CONNECTION_STATE_APPLYING_LTK) {
            (void)app_timer_stop(&conn->timer);
            (void)send_bonding_finished(conn);
            if (conn->gattdb_known == ESL_LIB_TRUE) {
              esl_lib_log_connection_debug(CONN_FMT "GATTDB known, skipping discovery, connection handle = %u" APP_LOG_NL,
                                           ESL_LIB_LOG_PTR(conn),
                                           conn->connection_handle);
              // Subscribe to characteristics
              if (conn->gattdb_handles.esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_CONTROL_POINT]
                  != ESL_LIB_INVALID_CHARACTERISTIC_HANDLE) {
                esl_lib_log_connection_debug(CONN_FMT "Subscribe to ESL CP notifications, connection handle = %u" APP_LOG_NL,
                                             ESL_LIB_LOG_PTR(conn),
                                             conn->connection_handle);
                sc = sl_bt_gatt_set_characteristic_notification(conn->connection_handle,
                                                                conn->gattdb_handles.esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_CONTROL_POINT],
                                                                sl_bt_gatt_notification);
              } else {
                sc = SL_STATUS_INVALID_HANDLE;
              }
              if (sc == SL_STATUS_OK) {
                (void)app_timer_stop(&conn->gatt_timer);
                conn->state = ESL_LIB_CONNECTION_STATE_ESL_SUBSCRIBE;
                sc = app_timer_start(&conn->gatt_timer,
                                     GATT_TIMEOUT_MS,
                                     gatt_timeout,
                                     conn,
                                     false);
              } else {
                esl_lib_log_connection_error(CONN_FMT "ESL CP subscribe failed, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                                             ESL_LIB_LOG_PTR(conn),
                                             conn->connection_handle,
                                             sc);
                // Close connection, save address just in case
                addr = close_broken_connection(&conn, &addr_backup);
                lib_status = ESL_LIB_STATUS_CONN_SUBSCRIBE_FAILED;
              }
            } else {
              esl_lib_log_connection_debug(CONN_FMT "Starting service discovery, connection handle = %u" APP_LOG_NL,
                                           ESL_LIB_LOG_PTR(conn),
                                           conn->connection_handle);

              // No predefined GATT database, start service discovery
              sc = sl_bt_gatt_discover_primary_services(conn->connection_handle);
              if (sc == SL_STATUS_OK) {
                (void)app_timer_stop(&conn->gatt_timer);
                conn->state = ESL_LIB_CONNECTION_STATE_SERVICE_DISCOVERY;
                sc = app_timer_start(&conn->gatt_timer,
                                     GATT_TIMEOUT_MS,
                                     gatt_timeout,
                                     conn,
                                     false);
              } else {
                esl_lib_log_connection_error(CONN_FMT "Error starting service discovery, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                                             ESL_LIB_LOG_PTR(conn),
                                             conn->connection_handle,
                                             sc);
                // Close connection
                addr = close_broken_connection(&conn, &addr_backup);
                lib_status = ESL_LIB_STATUS_CONN_DISCOVERY_FAILED;
              }
            }
          } else if (conn->state == ESL_LIB_CONNECTION_STATE_PAST_INIT) {
            (void)app_timer_stop(&conn->timer);
            esl_lib_log_connection_debug(CONN_FMT "PAST transfer, connection handle = %u" APP_LOG_NL,
                                         ESL_LIB_LOG_PTR(conn),
                                         conn->connection_handle);
            pawr = (esl_lib_pawr_t *)conn->command->data.cmd_init_past.pawr_handle;
            if (esl_lib_pawr_contains(pawr)) {
              // Connection parameters changed by PAST init
              sc = sl_bt_advertiser_past_transfer(conn->connection_handle,
                                                  PAWR_SERVICE_DATA,
                                                  pawr->pawr_handle);
              if (sc == SL_STATUS_OK) {
                // calculate timeout as follows: timeout_value_ms = 6 * (pawr->config.adv_interval.max * 1.25f) [ms]
                uint32_t past_timeout = PAST_GRACE_INTERVAL_COUNT \
                                        * ((pawr->config.adv_interval.max) + (pawr->config.adv_interval.max >> 2));
                conn->state = ESL_LIB_CONNECTION_STATE_PAST_CLOSE_CONNECTION;
                sc = app_timer_start(&conn->timer,
                                     past_timeout + (uint32_t)(PAST_GRACE_INTERVAL_COUNT * 1.25f),  // round up margin
                                     connection_timeout,
                                     conn,
                                     false);

                if (sc == SL_STATUS_OK) {
                  esl_lib_log_connection_debug(CONN_FMT "PAST transfer started, connection handle = %u, PAwR = " ESL_LIB_LOG_HANDLE_FORMAT "timeout = %d ms" APP_LOG_NL,
                                               ESL_LIB_LOG_PTR(conn),
                                               conn->connection_handle,
                                               ESL_LIB_LOG_PTR(conn->command->data.cmd_init_past.pawr_handle),
                                               past_timeout);
                }
              }
              if (sc != SL_STATUS_OK) {
                lib_status = ESL_LIB_STATUS_PAST_INIT_FAILED;
              }
            } else {
              sc = SL_STATUS_NOT_FOUND;
              lib_status = ESL_LIB_STATUS_PAST_INIT_FAILED;
            }

            if (sc != SL_STATUS_OK) {
              esl_lib_log_connection_warning(CONN_FMT "PAST transfer unsuccessful, connection handle = %u, PAwR = " ESL_LIB_LOG_HANDLE_FORMAT "sc = 0x%04x" APP_LOG_NL,
                                             ESL_LIB_LOG_PTR(conn),
                                             conn->connection_handle,
                                             ESL_LIB_LOG_PTR(conn->command->data.cmd_init_past.pawr_handle),
                                             sc);
              addr = close_broken_connection(&conn, &addr_backup);
            } else if (pawr->config.advertise == ESL_LIB_TRUE && pawr->state == ESL_LIB_PAWR_STATE_RUNNING) {
              sc = sl_bt_extended_advertiser_start(pawr->pawr_handle,
                                                   sl_bt_extended_advertiser_non_connectable,
                                                   0);
              if (sc == SL_STATUS_OK) {
                esl_lib_log_connection_debug(CONN_FMT "Async PAwR status update requested for PAwR handle = %u" APP_LOG_NL,
                                             ESL_LIB_LOG_PTR(conn),
                                             pawr->pawr_handle);
                pawr->state = ESL_LIB_PAWR_STATE_RUNNING_ADVERTISING;
                esl_lib_get_pawr_status(pawr);
              }
            }
          }
        }
      } else {
        // Suppress error event for unknown connections
        sc = SL_STATUS_OK;
      }
      break;
    case sl_bt_evt_connection_data_length_id:
      sc = esl_lib_connection_find(evt->data.evt_connection_data_length.connection,
                                   &conn);
      if (sc == SL_STATUS_OK) {
        conn->max_payload = evt->data.evt_connection_data_length.tx_data_len - GATT_OVERHEAD;
        esl_lib_log_connection_debug(CONN_FMT "Connection payload = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->max_payload);
      } else {
        // Suppress error event for unknown connections
        sc = SL_STATUS_OK;
      }
      break;
    // Bonding
    case sl_bt_evt_external_bondingdb_data_request_id:
      sc = esl_lib_connection_find(evt->data.evt_external_bondingdb_data_request.connection,
                                   &conn);
      if (sc == SL_STATUS_OK) {
        if (evt->data.evt_external_bondingdb_data_request.type
            == sl_bt_external_bondingdb_data_remote_ltk) {
          esl_lib_log_connection_debug(CONN_FMT "Bonding LTK requested, connection handle = %u" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       conn->connection_handle);
          if (conn->state == ESL_LIB_CONNECTION_STATE_CONNECTION_OPENED) {
            // Check if LTK is set for the request
            if (find_tlv(conn->command, ESL_LIB_CONNECT_DATA_TYPE_LTK, &tlv)) {
              bonding_data = tlv->data.data;
              bonding_data_len = ESL_LIB_LTK_SIZE;
              conn->state = ESL_LIB_CONNECTION_STATE_APPLYING_LTK;
              lib_status = ESL_LIB_STATUS_CONN_FAILED;
              esl_lib_log_connection_debug(CONN_FMT "Applying LTK, connection handle = %u" APP_LOG_NL,
                                           ESL_LIB_LOG_PTR(conn),
                                           conn->connection_handle);
            } else {
              // New bond require if we do not have the LTK.
              conn->state = ESL_LIB_CONNECTION_STATE_NEW_BOND_REQUIRED;
              lib_status = ESL_LIB_STATUS_BONDING_FAILED;
              esl_lib_log_connection_debug(CONN_FMT "Creating new bond, connection handle = %u" APP_LOG_NL,
                                           ESL_LIB_LOG_PTR(conn),
                                           conn->connection_handle);
            }
          }
        }
        sc = sl_bt_external_bondingdb_set_data(evt->data.evt_external_bondingdb_data_request.connection,
                                               evt->data.evt_external_bondingdb_data_request.type,
                                               bonding_data_len,
                                               bonding_data);
        if (sc == SL_STATUS_OK) {
          lib_status = ESL_LIB_STATUS_NO_ERROR;
        } else {
          esl_lib_log_level_t level = conn->established ? ESL_LIB_LOG_LEVEL_ERROR : ESL_LIB_LOG_LEVEL_WARNING;
          // Set library status accordingly.
          lib_status = ESL_LIB_STATUS_BONDING_FAILED;
          conn->state = ESL_LIB_CONNECTION_STATE_BONDING_RECOVERY;
          // Defer forced close on error - normally the close event should come, this is just a watchdog
          (void)close_connection(conn);
          conn->last_error = sc; // Override last_error from close_connection
          esl_lib_log(level, ESL_LIB_LOG_MODULE_CONNECTION,
                      CONN_FMT "Bonding %s, reconnecting, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                      ESL_LIB_LOG_PTR(conn),
                      conn->established ? "procedure disrupted" : "is not possible",
                      conn->connection_handle,
                      sc);
        }
      } else {
        // Suppress error event for unknown connections
        sc = SL_STATUS_OK;
      }
      break;
    case sl_bt_evt_sm_passkey_request_id:
      sc = esl_lib_connection_find(evt->data.evt_sm_passkey_request.connection,
                                   &conn);
      if (sc == SL_STATUS_OK) {
        esl_lib_log_connection_debug(CONN_FMT "Passkey requested, connection handle = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle);

        if (find_tlv(conn->command, ESL_LIB_CONNECT_DATA_TYPE_PASSKEY, &tlv)) {
          sc = sl_bt_sm_enter_passkey(conn->connection_handle,
                                      *((uint32_t*)tlv->data.data));
          esl_lib_log_connection_debug(CONN_FMT "Entering passkey, connection handle = %u" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       conn->connection_handle);
          if (sc != SL_STATUS_OK) {
            lib_status = ESL_LIB_STATUS_BONDING_FAILED;
            esl_lib_log_connection_error(CONN_FMT "Failed to set passkey, connection handle = %u. Closing." APP_LOG_NL,
                                         ESL_LIB_LOG_PTR(conn),
                                         conn->connection_handle);
            // Close the connection in case of error, save address for later use
            addr = close_broken_connection(&conn, &addr_backup);
          }
        } else {
          esl_lib_log_connection_error(CONN_FMT "No passkey available but requested, connection handle = %u" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       conn->connection_handle);
        }
      } else {
        // Suppress error event for unknown connections
        sc = SL_STATUS_OK;
      }
      break;
    case sl_bt_evt_external_bondingdb_data_ready_id:
      sc = esl_lib_connection_find(evt->data.evt_external_bondingdb_data_ready.connection,
                                   &conn);
      if (sc == SL_STATUS_OK) {
        esl_lib_log_connection_debug(CONN_FMT "Bonding data ready, connection handle = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle);
        sc = sl_bt_sm_increase_security(conn->connection_handle);
        if (sc == SL_STATUS_OK) {
          conn->state = ESL_LIB_CONNECTION_STATE_BONDING;
        } else {
          esl_lib_log_level_t level = conn->established ? ESL_LIB_LOG_LEVEL_ERROR : ESL_LIB_LOG_LEVEL_WARNING;
          // Close the connection in case of error.
          (void)close_connection(conn);
          conn->last_error = sc; // Override last_error from close_connection
          lib_status = ESL_LIB_STATUS_BONDING_FAILED;
          esl_lib_log(level, ESL_LIB_LOG_MODULE_CONNECTION,
                      CONN_FMT "Increase security %s, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                      ESL_LIB_LOG_PTR(conn),
                      conn->established ? "failed" : "is not possible",
                      conn->connection_handle,
                      sc);
        }
      } else {
        // Suppress error event for unknown connections
        sc = SL_STATUS_OK;
      }
      break;
    case sl_bt_evt_external_bondingdb_data_id:
      // Check type of the bonding data first.
      if (evt->data.evt_external_bondingdb_data_request.type
          == sl_bt_external_bondingdb_data_remote_ltk) {
        // Find connection in the list.
        sc = esl_lib_connection_find(evt->data.evt_external_bondingdb_data.connection,
                                     &conn);
        if (sc == SL_STATUS_OK) {
          esl_lib_log_connection_debug(CONN_FMT "Bonding LTK arrived, connection handle = %u" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       conn->connection_handle);

          // Send LTK to the application.
          (void)send_bonding_data(conn,
                                  evt->data.evt_external_bondingdb_data.data.data);
        } else {
          sc = SL_STATUS_OK;
        }
      }
      break;
    case sl_bt_evt_sm_bonding_failed_id:
      sc = esl_lib_connection_find(evt->data.evt_sm_bonding_failed.connection,
                                   &conn);
      if (sc == SL_STATUS_OK) {
        if ((conn->command != NULL && conn->command->data.cmd_connect.retries_left)
            && ((evt->data.evt_sm_bonding_failed.reason == SL_STATUS_BT_CTRL_PIN_OR_KEY_MISSING)
                || (evt->data.evt_sm_bonding_failed.reason == SL_STATUS_BT_SMP_PAIRING_NOT_SUPPORTED))) {
          esl_lib_log_connection_warning(CONN_FMT "Bonding failed, reconnecting, connection handle = %u, reason = 0x%04x" APP_LOG_NL,
                                         ESL_LIB_LOG_PTR(conn),
                                         conn->connection_handle,
                                         evt->data.evt_sm_bonding_failed.reason);
          if (evt->data.evt_sm_bonding_failed.reason == SL_STATUS_BT_CTRL_PIN_OR_KEY_MISSING) {
            // Remove LTK if present
            esl_lib_connect_tlv_t *tlv;
            if (find_tlv(conn->command, ESL_LIB_CONNECT_DATA_TYPE_LTK, &tlv)) {
              tlv->type = ESL_LIB_CONNECT_DATA_TYPE_INVALID_TYPE;
              esl_lib_log_connection_warning(CONN_FMT "LTK dropped, connection handle = %u" APP_LOG_NL,
                                             ESL_LIB_LOG_PTR(conn),
                                             conn->connection_handle);
            }
          }
          // Set connection state to match this
          conn->state = ESL_LIB_CONNECTION_STATE_BONDING_RECOVERY;
          // Bypass event
          sc = SL_STATUS_OK;
          lib_status = ESL_LIB_STATUS_NO_ERROR;
        } else {
          esl_lib_log_connection_error(CONN_FMT "Bonding failed, disconnecting, connection handle = %u, reason = 0x%04x" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       conn->connection_handle,
                                       evt->data.evt_sm_bonding_failed.reason);
          // Set library status accordingly.
          lib_status = ESL_LIB_STATUS_BONDING_FAILED;
          // Save reason to the status variable.
          sc = evt->data.evt_sm_bonding_failed.reason;
        }
        // Try to close connection gently - it should be closing already
        (void)close_connection(conn);
        conn->last_error = sc; // Override last_error from close_connection call
      } else {
        // Suppress error event for unknown connections
        sc = SL_STATUS_OK;
      }
      break;
    // GATT
    case sl_bt_evt_gatt_service_id:
      sc = esl_lib_connection_find(evt->data.evt_gatt_service.connection,
                                   &conn);
      if (sc == SL_STATUS_OK) {
        // Check size for UUID if in proper state - send error otherwise
        if (conn->state != ESL_LIB_CONNECTION_STATE_SERVICE_DISCOVERY) {
          sc = SL_STATUS_INVALID_STATE;
          lib_status = ESL_LIB_STATUS_CONN_DISCOVERY_FAILED;
          esl_lib_log_connection_error(CONN_FMT "Service discovery failed, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       conn->connection_handle,
                                       sc);
          addr = close_broken_connection(&conn, &addr_backup);
        } else if (evt->data.evt_gatt_service.uuid.len == sizeof(sl_bt_uuid_16_t)) {
          // Check for service UUIDs
          if (uuid_16_match(evt->data.evt_gatt_service.uuid.data, (uint8_t *)uuid_map.services.dis.data)) {
            esl_lib_log_connection_debug(CONN_FMT "DIS service found, connection handle = %u" APP_LOG_NL,
                                         ESL_LIB_LOG_PTR(conn),
                                         conn->connection_handle);
            // Copy DIS handle
            conn->gattdb_handles.services.dis = evt->data.evt_gatt_service.service;
          } else if (uuid_16_match(evt->data.evt_gatt_service.uuid.data, (uint8_t *)uuid_map.services.esl.data)) {
            esl_lib_log_connection_debug(CONN_FMT "ESL service found, connection handle = %u" APP_LOG_NL,
                                         ESL_LIB_LOG_PTR(conn),
                                         conn->connection_handle);
            // Copy ESL handle
            conn->gattdb_handles.services.esl = evt->data.evt_gatt_service.service;
          } else if (uuid_16_match(evt->data.evt_gatt_service.uuid.data, (uint8_t *)uuid_map.services.ots.data)) {
            esl_lib_log_connection_debug(CONN_FMT "OTS service found, connection handle = %u" APP_LOG_NL,
                                         ESL_LIB_LOG_PTR(conn),
                                         conn->connection_handle);

            // Copy OTS handle
            conn->gattdb_handles.services.ots = evt->data.evt_gatt_service.service;
          }
        }
      } else {
        sc = SL_STATUS_OK;
      }
      break;
    case sl_bt_evt_gatt_characteristic_id:
      sc = esl_lib_connection_find(evt->data.evt_gatt_characteristic.connection,
                                   &conn);
      if (sc == SL_STATUS_OK) {
        // Check size
        if (evt->data.evt_gatt_characteristic.uuid.len == sizeof(sl_bt_uuid_16_t)) {
          // Iterate over ESL characteristic UUIDs
          // ESL characteristics
          for (uint8_t i = 0; i < ESL_LIB_CHARACTERISTIC_INDEX_SIZE; i++) {
            if (uuid_16_match(evt->data.evt_gatt_characteristic.uuid.data, (uint8_t *)uuid_map.esl_characteristics[i].data)) {
              esl_lib_log_connection_debug(CONN_FMT "ESL characteristic %d found, connection handle = %u" APP_LOG_NL,
                                           ESL_LIB_LOG_PTR(conn),
                                           i,
                                           conn->connection_handle);

              // Assign GATT database handle
              conn->gattdb_handles.esl_characteristics[i] = evt->data.evt_gatt_characteristic.characteristic;
            }
          }
          // DIS characteristic
          for (uint8_t i = 0; i < ESL_LIB_CHARACTERISTIC_INDEX_DIS_SIZE; i++) {
            if (uuid_16_match(evt->data.evt_gatt_characteristic.uuid.data,
                              (uint8_t *)uuid_map.dis_characteristics[i].data)) {
              esl_lib_log_connection_debug(CONN_FMT "DIS characteristic %u found, connection handle = %u" APP_LOG_NL,
                                           ESL_LIB_LOG_PTR(conn),
                                           i,
                                           conn->connection_handle);
              conn->gattdb_handles.dis_characteristics[i] = evt->data.evt_gatt_characteristic.characteristic;
            }
          }
        }
      } else {
        // Suppress error event for unknown connections
        sc = SL_STATUS_OK;
      }
      break;
    case sl_bt_evt_gatt_characteristic_value_id:
      esl_lib_log_connection_debug("Char value, opcode: %u" APP_LOG_NL, evt->data.evt_gatt_characteristic_value.att_opcode);
      sc = esl_lib_connection_find(evt->data.evt_gatt_characteristic_value.connection,
                                   &conn);
      if (sc == SL_STATUS_OK) {
        if (evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_read_response) {
          esl_lib_log_connection_debug(CONN_FMT "Read response, connection handle = %u" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       conn->connection_handle);
          if (conn->state == ESL_LIB_CONNECTION_STATE_GET_TAG_INFO) {
            lib_status = ESL_LIB_STATUS_CONN_READ_FAILED;
            sc = esl_lib_storage_append(conn->tag_info_data,
                                        evt->data.evt_gatt_characteristic_value.value.len,
                                        evt->data.evt_gatt_characteristic_value.value.data);
            if (sc == SL_STATUS_OK) {
              lib_status = ESL_LIB_STATUS_NO_ERROR;
            }
          }
        } else if (evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_handle_value_notification) {
          // Notification arrived
          if (evt->data.evt_gatt_characteristic_value.characteristic
              == conn->gattdb_handles.esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_CONTROL_POINT]) {
            esl_lib_log_connection_debug(CONN_FMT "CP notification received, connection handle = %u" APP_LOG_NL,
                                         ESL_LIB_LOG_PTR(conn),
                                         conn->connection_handle);
            (void)send_cp_notification_event(conn,
                                             evt->data.evt_gatt_characteristic_value.value.len,
                                             evt->data.evt_gatt_characteristic_value.value.data);
          }
        }
      } else {
        // Suppress error event for unknown connections
        sc = SL_STATUS_OK;
      }
      break;
    case sl_bt_evt_gatt_procedure_completed_id:
      sc = esl_lib_connection_find(evt->data.evt_gatt_procedure_completed.connection,
                                   &conn);
      if (sc == SL_STATUS_OK) {
        if (conn->state == ESL_LIB_CONNECTION_STATE_WRITE_CONTROL_POINT) {
          // This state is active only if write with response requested
          // Send response with the result
          (void)send_att_response(conn,
                                  ESL_LIB_EVT_CONTROL_POINT_RESPONSE,
                                  evt->data.evt_gatt_procedure_completed.result);
          conn->command_complete = true;
          esl_lib_log_connection_debug(CONN_FMT "Control point written, result = 0x%04x, connection handle = %u" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       evt->data.evt_gatt_procedure_completed.result,
                                       conn->connection_handle);
        } else if (conn->state == ESL_LIB_CONNECTION_STATE_CONFIGURE_TAG) {
          // This state is active only if write with response requested
          esl_lib_log_connection_debug(CONN_FMT "Configure tag - Writing next value, connection handle = %u" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       conn->connection_handle);
          // Send response with the result
          (void)send_att_response(conn,
                                  ESL_LIB_EVT_CONFIGURE_TAG_RESPONSE,
                                  evt->data.evt_gatt_procedure_completed.result);
          // Write next configuration value
          write_next_config_value(conn);
        } else {
          // Check result first for all other cases
          if (evt->data.evt_gatt_procedure_completed.result == SL_STATUS_OK) {
            esl_lib_log_connection_debug(CONN_FMT "Procedure completed, connection handle = %u" APP_LOG_NL,
                                         ESL_LIB_LOG_PTR(conn),
                                         conn->connection_handle);
            switch (conn->state) {
              case ESL_LIB_CONNECTION_STATE_GET_TAG_INFO:
                (void)app_timer_stop(&conn->gatt_timer);
                lib_status = ESL_LIB_STATUS_CONN_READ_FAILED;
                esl_lib_log_connection_debug(CONN_FMT "Tag info %u arrived, connection handle = %u" APP_LOG_NL,
                                             ESL_LIB_LOG_PTR(conn),
                                             conn->tag_info_type,
                                             conn->connection_handle);
                // Save tag info, completed
                sc = save_tag_info(conn);
                if (sc == SL_STATUS_OK) {
                  esl_lib_log_connection_debug(CONN_FMT "Read next tag info, connection handle = %u" APP_LOG_NL,
                                               ESL_LIB_LOG_PTR(conn),
                                               conn->connection_handle);
                  // Move to next one
                  sc = get_next_tag_info(conn);
                  if (sc == SL_STATUS_OK) {
                    lib_status = ESL_LIB_STATUS_NO_ERROR;
                  }
                } else {
                  sc = get_tag_info_finish(conn, sc);
                }
                break;
              case ESL_LIB_CONNECTION_STATE_SERVICE_DISCOVERY:
                (void)app_timer_stop(&conn->gatt_timer);
                // If DIS found
                if (conn->gattdb_handles.services.dis != ESL_LIB_INVALID_SERVICE_HANDLE) {
                  esl_lib_log_connection_debug(CONN_FMT "Service discovery finished, start DIS discovery, connection handle = %u" APP_LOG_NL,
                                               ESL_LIB_LOG_PTR(conn),
                                               conn->connection_handle);
                  // Discover Device Information Service service characteristics
                  sc = sl_bt_gatt_discover_characteristics(conn->connection_handle,
                                                           conn->gattdb_handles.services.dis);
                  if (sc == SL_STATUS_OK) {
                    conn->state = ESL_LIB_CONNECTION_STATE_DIS_DISCOVERY;
                    sc = app_timer_start(&conn->gatt_timer,
                                         GATT_TIMEOUT_MS,
                                         gatt_timeout,
                                         conn,
                                         false);
                  }
                  if (sc != SL_STATUS_OK) {
                    lib_status = ESL_LIB_STATUS_CONN_DISCOVERY_FAILED;
                    esl_lib_log_connection_error(CONN_FMT "DIS discovery failed, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                                                 ESL_LIB_LOG_PTR(conn),
                                                 conn->connection_handle,
                                                 sc);
                  }
                } else if (conn->gattdb_handles.services.esl != ESL_LIB_INVALID_SERVICE_HANDLE) {
                  esl_lib_log_connection_debug(CONN_FMT "Service discovery finished, start ESL discovery, connection handle = %u" APP_LOG_NL,
                                               ESL_LIB_LOG_PTR(conn),
                                               conn->connection_handle);
                  // Discover ESL service characteristics
                  sc = sl_bt_gatt_discover_characteristics(conn->connection_handle,
                                                           conn->gattdb_handles.services.esl);
                  if (sc == SL_STATUS_OK) {
                    conn->state = ESL_LIB_CONNECTION_STATE_ESL_DISCOVERY;
                    sc = app_timer_start(&conn->gatt_timer,
                                         GATT_TIMEOUT_MS,
                                         gatt_timeout,
                                         conn,
                                         false);
                  }
                } else {
                  // Special internal signal
                  sc = SL_STATUS_BT_ATT_ATT_NOT_FOUND;
                }
                if (sc != SL_STATUS_OK) {
                  lib_status = sc == SL_STATUS_BT_ATT_ATT_NOT_FOUND ? ESL_LIB_STATUS_CONN_ESL_SERVICE_VIOLATION : ESL_LIB_STATUS_CONN_DISCOVERY_FAILED;
                  esl_lib_log_connection_error(CONN_FMT "Discovery failed, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                                               ESL_LIB_LOG_PTR(conn),
                                               conn->connection_handle,
                                               sc);
                  // Close connection
                  addr = close_broken_connection(&conn, &addr_backup);
                }
                break;
              case ESL_LIB_CONNECTION_STATE_DIS_DISCOVERY:
                (void)app_timer_stop(&conn->gatt_timer);
                if (conn->gattdb_handles.services.esl != ESL_LIB_INVALID_SERVICE_HANDLE) {
                  // Discover ESL service characteristics
                  esl_lib_log_connection_debug(CONN_FMT "Device information discovery finished, start ESL discovery, connection handle = %u" APP_LOG_NL,
                                               ESL_LIB_LOG_PTR(conn),
                                               conn->connection_handle);

                  sc = sl_bt_gatt_discover_characteristics(conn->connection_handle,
                                                           conn->gattdb_handles.services.esl);
                  if (sc == SL_STATUS_OK) {
                    conn->state = ESL_LIB_CONNECTION_STATE_ESL_DISCOVERY;
                    sc = app_timer_start(&conn->gatt_timer,
                                         GATT_TIMEOUT_MS,
                                         gatt_timeout,
                                         conn,
                                         false);
                  }
                  if (sc != SL_STATUS_OK) {
                    lib_status = ESL_LIB_STATUS_CONN_DISCOVERY_FAILED;
                    esl_lib_log_connection_error(CONN_FMT "DIS discovery failed, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                                                 ESL_LIB_LOG_PTR(conn),
                                                 conn->connection_handle,
                                                 sc);
                    // Close connection
                    addr = close_broken_connection(&conn, &addr_backup);
                  }
                } else {
                  // Set the cause of the error
                  sc = SL_STATUS_INVALID_HANDLE;
                  lib_status = ESL_LIB_STATUS_CONN_ESL_SERVICE_VIOLATION;
                  esl_lib_log_connection_error(CONN_FMT "ESL Service not found, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                                               ESL_LIB_LOG_PTR(conn),
                                               conn->connection_handle,
                                               sc);
                  // Close connection
                  addr = close_broken_connection(&conn, &addr_backup);
                }
                break;
              case ESL_LIB_CONNECTION_STATE_ESL_DISCOVERY:
                (void)app_timer_stop(&conn->gatt_timer);
                // Subscribe to characteristics
                if (conn->gattdb_handles.esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_CONTROL_POINT]
                    != ESL_LIB_INVALID_CHARACTERISTIC_HANDLE) {
                  esl_lib_log_connection_debug(CONN_FMT "Feature discovery complete, subscribe to ESL CP notifications, connection handle = %u" APP_LOG_NL,
                                               ESL_LIB_LOG_PTR(conn),
                                               conn->connection_handle);
                  // Check validity of the handles.
                  sc = esl_lib_connection_check_gattdb_handles(&(conn->gattdb_handles));
                  if (sc == SL_STATUS_OK) {
                    conn->gattdb_known = ESL_LIB_TRUE;
                    sc = sl_bt_gatt_set_characteristic_notification(conn->connection_handle,
                                                                    conn->gattdb_handles.esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_CONTROL_POINT],
                                                                    sl_bt_gatt_notification);
                    if (sc == SL_STATUS_OK) {
                      conn->state = ESL_LIB_CONNECTION_STATE_ESL_SUBSCRIBE;
                      sc = app_timer_start(&conn->gatt_timer,
                                           GATT_TIMEOUT_MS,
                                           gatt_timeout,
                                           conn,
                                           false);
                    }
                  } else {
                    sc = SL_STATUS_BT_ATT_CLIENT_CHARACTERISTIC_CONFIGURATION_DESCRIPTOR_IMPROPERLY_CONFIGURED; // ESL does not meet Profile / Service specification!
                  }
                  if (sc != SL_STATUS_OK) {
                    lib_status = ESL_LIB_STATUS_CONN_SUBSCRIBE_FAILED;
                    esl_lib_log_connection_error(CONN_FMT "Subscribe failed, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                                                 ESL_LIB_LOG_PTR(conn),
                                                 conn->connection_handle,
                                                 sc);
                    // Close connection
                    addr = close_broken_connection(&conn, &addr_backup);
                  }
                } else {
                  // Set the cause of the error
                  sc = SL_STATUS_INVALID_HANDLE;
                  lib_status = ESL_LIB_STATUS_CONN_ESL_SERVICE_VIOLATION;
                  esl_lib_log_connection_error(CONN_FMT "ESL CP not found, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                                               ESL_LIB_LOG_PTR(conn),
                                               conn->connection_handle,
                                               sc);
                  // Close connection
                  addr = close_broken_connection(&conn, &addr_backup);

                  if (conn) {
                    conn->last_error = sc; // Override last_error -only if the connection could be closed gracefully!
                  }
                }
                break;
              case ESL_LIB_CONNECTION_STATE_ESL_SUBSCRIBE:
                (void)app_timer_stop(&conn->gatt_timer);
                // Check presense of OTS service
                if (conn->gattdb_handles.services.ots != ESL_LIB_INVALID_SERVICE_HANDLE) {
                  esl_lib_log_connection_debug(CONN_FMT "Initializing Image Transfer - OTS, connection handle = %u" APP_LOG_NL,
                                               ESL_LIB_LOG_PTR(conn),
                                               conn->connection_handle);
                  // Initialize Image transfer
                  sc = esl_lib_image_transfer_init(conn->connection_handle,
                                                   conn->gattdb_handles.services.ots,
                                                   on_image_transfer_status,
                                                   on_image_transfer_type_arrived,
                                                   on_image_transfer_finished,
                                                   NULL,
                                                   &conn->ots_handle);
                  if (sc == SL_STATUS_OK) {
                    conn->state = ESL_LIB_CONNECTION_STATE_OTS_INIT;
                  } else {
                    lib_status = ESL_LIB_STATUS_OTS_INIT_FAILED;
                    esl_lib_log_connection_error(CONN_FMT "Image Transfer - OTS init failed, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                                                 ESL_LIB_LOG_PTR(conn),
                                                 conn->connection_handle,
                                                 sc);
                    // Close connection
                    addr = close_broken_connection(&conn, &addr_backup);
                  }
                } else {
                  // If no OTS present, consider complete
                  connection_complete(conn, SL_STATUS_OK);
                }
                break;
              default:
                break;
            }
          } else {
            // If not in image transfer, procedures handled by this module
            if (!check_image_transfer(conn)) {
              lib_status = ESL_LIB_STATUS_CONN_LOST;
              sc = evt->data.evt_gatt_procedure_completed.result;
              esl_lib_log_connection_error(CONN_FMT "Procedure failure, connection handle = %u, result = 0x%04x" APP_LOG_NL,
                                           ESL_LIB_LOG_PTR(conn),
                                           conn->connection_handle,
                                           sc);
              // Close connection on procedure failure
              addr = close_broken_connection(&conn, &addr_backup);
            }
          }
        }
      } else {
        // Suppress error event for unknown connections
        sc = SL_STATUS_OK;
      }
      break;
    default:
      break;
  }
  if (sc != SL_STATUS_OK) {
    esl_lib_status_data_t state;

    if (conn != NULL) {
      state.connection_state = conn->state;
      // Send connection error if connection is present.
      (void)send_connection_error(conn,
                                  lib_status,
                                  sc,
                                  conn->state);
      conn->command_complete = true;
    } else {
      esl_lib_node_id_t node_id;
      state.core_state = ESL_LIB_CORE_STATE_CONNECTING;
      if (addr != NULL) {
        // Send address if present.
        node_id.type = ESL_LIB_NODE_ID_TYPE_ADDRESS;
        node_id.id.address.address_type = addr->address_type;
        memcpy(node_id.id.address.addr, addr->addr, sizeof(node_id.id.address.addr));
      } else {
        node_id.type = ESL_LIB_NODE_ID_TYPE_NONE;
      }

      // Send available data in the error message
      (void)esl_lib_event_push_error(lib_status,
                                     &node_id,
                                     sc,
                                     state);
    }
  }
}

// -----------------------------------------------------------------------------
// Private functions
static void esl_lib_connection_safe_remove_ptr(esl_lib_connection_t *ptr)
{
  if (ptr != NULL) {
    (void)app_timer_stop(&ptr->timer);
    (void)app_timer_stop(&ptr->gatt_timer);
    sl_slist_remove(&connection_list, &ptr->node);
    esl_lib_command_list_cleanup(&ptr->command_list);

    if (ptr->command != NULL) {
      if (ptr->command->cmd_code == ESL_LIB_CMD_WRITE_IMAGE
          && ptr->command->data.cmd_write_image.img_data_copied != NULL) {
        esl_lib_memory_free(ptr->command->data.cmd_write_image.img_data_copied);
      }
      esl_lib_memory_free(ptr->command);
    }
    clean_tag_info(ptr);
    esl_lib_log_connection_debug(CONN_FMT "Removed connection handle = %u, last error: 0x%04x" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(ptr),
                                 ptr->connection_handle,
                                 ptr->last_error);
    // Sanitize handles in memory area to avoid possible corruption later due to garbage
    ptr->connection_handle = SL_BT_INVALID_CONNECTION_HANDLE;
    esl_lib_image_dump_transfer_by_handle(&ptr->ots_handle);
    esl_lib_memory_free(ptr);
  }
}

static void run_command(esl_lib_command_list_cmd_t *cmd)
{
  sl_status_t          sc         = SL_STATUS_DELETED;
  esl_lib_connection_t *conn      = NULL;
  esl_lib_status_t     lib_status = ESL_LIB_STATUS_NO_ERROR;

  switch (cmd->cmd_code) {
    case ESL_LIB_CMD_CLOSE_CONNECTION:
      conn = (esl_lib_connection_t *)cmd->data.cmd_close_connection;
      if (esl_lib_connection_contains(conn)) {
        esl_lib_log_connection_debug(CONN_FMT "Close connection command, connection handle = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle);
        sc = close_connection(conn);
        esl_lib_log_connection_debug(CONN_FMT "After Close connection command, connection handle = %u , sc = 0x%04x" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle,
                                     sc);
      } else {
        lib_status = ESL_LIB_STATUS_CONN_CLOSE_FAILED;
      }
      break;
    case ESL_LIB_CMD_GET_TAG_INFO:
      conn = (esl_lib_connection_t *)cmd->data.cmd_write_image.connection_handle;
      lib_status = ESL_LIB_STATUS_CONN_READ_FAILED;
      if (esl_lib_connection_contains(conn)) {
        esl_lib_log_connection_debug(CONN_FMT "Get tag info command, connection handle = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle);
        // Check OTS features
        if (conn->ots_handle != ESL_LIB_INVALID_HANDLE) {
          conn->tag_info_type = ESL_LIB_DATA_TYPE_GATT_OTS_FEATURE;
          // Allocate an array
          uint8_t data[sizeof(byte_array) + sizeof(esl_lib_ots_features_t)];
          // Refer to the data as an byte array
          byte_array *data_send = (byte_array*)&data;
          // Set pointer to data part of the array as OTS features
          esl_lib_ots_features_t *features = (esl_lib_ots_features_t *)&data_send->data;
          // Set size of the data
          data_send->len = sizeof(esl_lib_ots_features_t);
          // Get data
          sc = esl_lib_image_transfer_get_features(conn->ots_handle, features);
          if (sc == SL_STATUS_OK) {
            // Create storage
            sc = esl_lib_storage_create(&conn->tag_info_data);
            if (sc == SL_STATUS_OK) {
              // Append data
              sc = esl_lib_storage_append(conn->tag_info_data, data_send->len, data_send->data);
              if (sc == SL_STATUS_OK) {
                // Save data to the list
                sc = save_tag_info(conn);
              }
            }
          }
          if (sc != SL_STATUS_OK) {
            esl_lib_log_connection_error(CONN_FMT "Failed to get OTS features %u, connection handle = %u" APP_LOG_NL,
                                         ESL_LIB_LOG_PTR(conn),
                                         conn->tag_info_type,
                                         conn->connection_handle);
            (void)get_tag_info_finish(conn, sc);
          }
        } else {
          sc = SL_STATUS_OK; // Nothing to do so far if there's no OTS service, let's continue the GetInfo procedure
        }
        if (sc == SL_STATUS_OK) {
          sc = get_next_tag_info(conn);
          if (sc == SL_STATUS_OK) {
            conn->state = ESL_LIB_CONNECTION_STATE_GET_TAG_INFO;
            lib_status = ESL_LIB_STATUS_NO_ERROR;
          }
        }
      }
      break;
    case ESL_LIB_CMD_CONFIGURE_TAG:
      conn = (esl_lib_connection_t *)cmd->data.cmd_configure_tag.connection_handle;
      if (esl_lib_connection_contains(conn)) {
        esl_lib_log_connection_debug(CONN_FMT "Configure tag command, connection handle = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle);
        // Clear index and type
        conn->config_index = 0;
        conn->config_type = ESL_LIB_DATA_TYPE_UNINITIALIZED;
        // Start write process using TLV(s)
        sc = write_next_config_value(conn);
      } else {
        lib_status = ESL_LIB_STATUS_CONN_CONFIG_FAILED;
      }
      break;
    case ESL_LIB_CMD_WRITE_CONTROL_POINT:
      conn = (esl_lib_connection_t *)cmd->data.cmd_write_control_point.connection_handle;
      lib_status = ESL_LIB_STATUS_CONN_WRITE_CP_FAILED;
      if (esl_lib_connection_contains(conn) && conn->command != NULL) {
        esl_lib_log_connection_debug(CONN_FMT "Write control point command, connection handle = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle);
        // Write characteristic value (with or without response)
        sc = write_value(conn,
                         conn->command->data.cmd_write_control_point.att_response,
                         ESL_LIB_DATA_TYPE_GATT_CONTROL_POINT,
                         conn->command->data.cmd_write_control_point.data.len,
                         conn->command->data.cmd_write_control_point.data.data);
        if (sc == SL_STATUS_OK) {
          if (conn->command->data.cmd_write_control_point.att_response == ESL_LIB_TRUE) {
            // Set state
            conn->state = ESL_LIB_CONNECTION_STATE_WRITE_CONTROL_POINT;
          } else {
            (void)send_att_response(conn,
                                    ESL_LIB_EVT_CONTROL_POINT_RESPONSE,
                                    sc);
            conn->command_complete = true;
          }
          lib_status = ESL_LIB_STATUS_NO_ERROR;
        } else {
          (void)send_att_response(conn,
                                  ESL_LIB_EVT_CONTROL_POINT_RESPONSE,
                                  sc);
          conn->command_complete = true;
          // Do not generate error event
          sc = SL_STATUS_OK;
        }
      }
      break;
    case ESL_LIB_CMD_WRITE_IMAGE:
      conn = (esl_lib_connection_t *)cmd->data.cmd_write_image.connection_handle;
      lib_status = ESL_LIB_STATUS_OTS_ERROR;
      if (esl_lib_connection_contains(conn)) {
        esl_lib_image_transfer_handle_t it_handle = conn->ots_handle;
        esl_lib_log_connection_debug(CONN_FMT "Write image command, connection handle = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle);
        if (it_handle != ESL_LIB_INVALID_HANDLE) {
          sc = esl_lib_image_transfer_start(it_handle,
                                            cmd->data.cmd_write_image.img_index,
                                            0,
                                            cmd->data.cmd_write_image.img_size,
                                            cmd->data.cmd_write_image.img_data_copied);
          if (sc == SL_STATUS_OK) {
            conn->state = ESL_LIB_CONNECTION_STATE_OTS_IMAGE_TRANSFER;
            lib_status = ESL_LIB_STATUS_NO_ERROR;
          } else {
            conn->command_complete = true;
          }
        } else {
          sc = SL_STATUS_NOT_INITIALIZED;
          conn->command_complete = true;
        }
      }
      break;
    case ESL_LIB_CMD_GET_IMAGE_TYPE:
      conn = (esl_lib_connection_t *)cmd->data.cmd_get_image_type.connection_handle;
      lib_status = ESL_LIB_STATUS_OTS_ERROR;
      if (esl_lib_connection_contains(conn)) {
        esl_lib_log_connection_debug(CONN_FMT "Get image type command, connection handle = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle);
        esl_lib_image_transfer_handle_t it_handle = conn->ots_handle;
        if (it_handle != ESL_LIB_INVALID_HANDLE) {
          sc = esl_lib_image_transfer_get_type(it_handle,
                                               cmd->data.cmd_get_image_type.img_index);
          if (sc == SL_STATUS_OK) {
            conn->state = ESL_LIB_CONNECTION_STATE_OTS_GET_TYPE;
            lib_status = ESL_LIB_STATUS_NO_ERROR;
          } else {
            conn->command_complete = true;
          }
        } else {
          sc = SL_STATUS_NOT_INITIALIZED;
          conn->command_complete = true;
        }
      }
      break;
    case ESL_LIB_CMD_INITIATE_PAST:
      conn = (esl_lib_connection_t *)cmd->data.cmd_init_past.connection_handle;
      if (esl_lib_connection_contains(conn)) {
        uint32_t supervison_timeout = PAST_CONN_DEFAULT_TIMEOUT;
        uint16_t min_interval = 2 * PAST_CONN_INTERVAL_MIN;
        uint16_t max_interval = PAST_CONN_INTERVAL_MAX;
        esl_lib_pawr_t *pawr = NULL;
        esl_lib_log_connection_debug(CONN_FMT "Initiate PAST command, connection handle = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle);
        if (conn->command != NULL) {
          pawr = (esl_lib_pawr_t *)conn->command->data.cmd_init_past.pawr_handle;
        }

        if (esl_lib_pawr_contains(pawr)) {
          supervison_timeout = pawr->config.adv_interval.max + pawr->config.adv_interval.max / 4; // * 1.25ms
          min_interval = pawr->config.adv_interval.min / 32;
          max_interval = pawr->config.adv_interval.max / 16;

          if (max_interval > PAST_CONN_INTERVAL_MAX) {
            max_interval = PAST_CONN_INTERVAL_MAX;
          }

          supervison_timeout *= 2 * (PAST_CONN_PERIPHERAL_LATENCY + 1); // Core spec. 5.4 Vol 4, Part E, 7.8.31.
          supervison_timeout = (supervison_timeout / 10) + 1;
          supervison_timeout += 9; // Round up to next multiple of 10ms

          if (supervison_timeout > PAST_CONN_MAX_TIMEOUT) {
            supervison_timeout = PAST_CONN_MAX_TIMEOUT;
          } else if (supervison_timeout < PAST_CONN_MIN_TIMEOUT) {
            supervison_timeout = PAST_CONN_MIN_TIMEOUT;
          }

          if (min_interval < PAST_CONN_INTERVAL_MIN) {
            min_interval = PAST_CONN_INTERVAL_MIN;
          } else if (min_interval > max_interval / 2) {
            min_interval = max_interval / 2;
          }

          sc = sl_bt_connection_set_parameters(conn->connection_handle,
                                               min_interval,
                                               max_interval,
                                               PAST_CONN_PERIPHERAL_LATENCY,
                                               (uint16_t)supervison_timeout,
                                               PAST_CONN_MIN_CE_LENGTH,
                                               PAST_CONN_MAX_CE_LENGTH);
          if (sc == SL_STATUS_OK) {
            conn->state = ESL_LIB_CONNECTION_STATE_PAST_INIT;
          } else {
            lib_status = ESL_LIB_STATUS_PAST_INIT_FAILED;
            conn->command_complete = true;
            // Ignore the warning if the connection is already closed by a remote node still in sync
            if (sc != SL_STATUS_BT_CTRL_COMMAND_DISALLOWED) {
              esl_lib_log_connection_warning(CONN_FMT "PAST init fail with connection interval[%u - %u] and timeout: %u, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                                             ESL_LIB_LOG_PTR(conn),
                                             min_interval,
                                             max_interval,
                                             supervison_timeout,
                                             conn->connection_handle,
                                             sc);
            }
          }
          // Start timeout anyway: the ESL may close/closing/closed the connection because it can be already in sync
          sc = app_timer_start(&conn->timer,
                               10 * supervison_timeout,
                               connection_timeout,
                               conn,
                               false);

          if (sc == SL_STATUS_OK) {
            esl_lib_log_connection_debug(CONN_FMT "Connection parameters set for PAST, connection handle = %u, PAwR = " ESL_LIB_LOG_HANDLE_FORMAT "timeout = %d ms" APP_LOG_NL,
                                         ESL_LIB_LOG_PTR(conn),
                                         conn->connection_handle,
                                         ESL_LIB_LOG_PTR(pawr),
                                         supervison_timeout);
          }
        }
      } else {
        lib_status = ESL_LIB_STATUS_PAST_INIT_FAILED;
      }
      break;
    default:
      break;
  }

  if (sc == SL_STATUS_DELETED) {
    esl_lib_log_connection_warning(CONN_FMT "Command failure on command %d, sc = 0x%04x" APP_LOG_NL,
                                   ESL_LIB_LOG_PTR(conn),
                                   cmd->cmd_code,
                                   sc);
  } else if (sc != SL_STATUS_OK && conn != NULL) {
    esl_lib_log_connection_error(CONN_FMT "Command failure, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(conn),
                                 conn->connection_handle,
                                 sc);
    // Send connection error if connection is present.
    (void)send_connection_error(conn,
                                lib_status,
                                sc,
                                conn->state);
  }
}

static sl_status_t send_cp_notification_event(esl_lib_connection_t *conn,
                                              size_t               len,
                                              void                 *data)
{
  sl_status_t   sc;
  esl_lib_evt_t *lib_evt;

  if (conn == ESL_LIB_INVALID_HANDLE || data == NULL) {
    return SL_STATUS_NULL_POINTER; // usage error, do nothing
  }

  sc = esl_lib_event_list_allocate(ESL_LIB_EVT_CONTROL_POINT_NOTIFICATION,
                                   len,
                                   &lib_evt);
  if (sc == SL_STATUS_OK) {
    lib_evt->data.evt_control_point_notification.connection_handle
      = (esl_lib_connection_handle_t)conn;
    lib_evt->data.evt_control_point_notification.data.len = len;
    memcpy(lib_evt->data.evt_control_point_notification.data.data,
           data,
           len);
    sc = esl_lib_event_list_push_back(lib_evt);
    if (sc != SL_STATUS_OK) {
      // Free up memory on failure
      esl_lib_memory_free(lib_evt);
    }
  }

  return sc;
}

static sl_status_t send_bonding_finished(esl_lib_connection_t *conn)
{
  sl_status_t   sc;
  esl_lib_evt_t *lib_evt;

  if (conn == ESL_LIB_INVALID_HANDLE) {
    return SL_STATUS_NULL_POINTER; // usage error, no event to send!
  }

  sc = esl_lib_event_list_allocate(ESL_LIB_EVT_BONDING_FINISHED,
                                   0,
                                   &lib_evt);
  if (sc == SL_STATUS_OK) {
    lib_evt->data.evt_bonding_finished.connection_handle
      = (esl_lib_connection_handle_t)conn;
    // Copy address
    lib_evt->data.evt_bonding_finished.address.address_type
      = conn->address_type;
    memcpy(lib_evt->data.evt_bonding_finished.address.addr,
           conn->address.addr,
           sizeof(lib_evt->data.evt_bonding_finished.address.addr));
    sc = esl_lib_event_list_push_back(lib_evt);
    if (sc != SL_STATUS_OK) {
      // Free up memory on failure
      esl_lib_memory_free(lib_evt);
    }
  }
  return sc;
}

static sl_status_t send_bonding_data(esl_lib_connection_t *conn,
                                     uint8_t              *data)
{
  sl_status_t   sc;
  esl_lib_evt_t *lib_evt;

  if (conn == ESL_LIB_INVALID_HANDLE || data == NULL) {
    return SL_STATUS_NULL_POINTER; // usage error, do nothing
  }

  sc = esl_lib_event_list_allocate(ESL_LIB_EVT_BONDING_DATA,
                                   0,
                                   &lib_evt);
  if (sc == SL_STATUS_OK) {
    lib_evt->data.evt_bonding_data.connection_handle
      = (esl_lib_connection_handle_t)conn;
    // Copy address
    lib_evt->data.evt_bonding_data.address.address_type
      = conn->address_type;
    memcpy(lib_evt->data.evt_bonding_data.address.addr,
           conn->address.addr,
           sizeof(lib_evt->data.evt_bonding_data.address.addr));
    memcpy(lib_evt->data.evt_bonding_data.ltk,
           data,
           sizeof(lib_evt->data.evt_bonding_data.ltk));
    sc = esl_lib_event_list_push_back(lib_evt);
    if (sc != SL_STATUS_OK) {
      // Free up memory on failure
      esl_lib_memory_free(lib_evt);
    }
  }
  return sc;
}

static sl_status_t send_retry_event(esl_lib_connection_t *conn,
                                    sl_status_t          reason)
{
  esl_lib_evt_t *lib_evt;
  sl_status_t sc;

  if (conn == ESL_LIB_INVALID_HANDLE) {
    return SL_STATUS_NULL_POINTER; // usage error, no event to send!
  }

  sc = esl_lib_event_list_allocate(ESL_LIB_EVT_CONNECTION_RETRY,
                                   0,
                                   &lib_evt);
  if (sc == SL_STATUS_OK) {
    lib_evt->data.evt_connection_retry.connection_handle = (esl_lib_connection_handle_t)conn;
    // Set last known connection state for the event
    lib_evt->data.evt_connection_retry.connection_state = conn->state;
    // Update the connection state
    conn->state = ESL_LIB_CONNECTION_STATE_OFF;
    // Set reason for disconnection event
    lib_evt->data.evt_connection_retry.reason = reason;
    // Copy address and set its type
    lib_evt->data.evt_connection_retry.address.address_type = conn->address_type;
    // Copy remaining retry count, if known
    if ((conn->command != NULL) && (conn->command->cmd_code == ESL_LIB_CMD_CONNECT)) {
      lib_evt->data.evt_connection_retry.retries_left = conn->command->data.cmd_connect.retries_left;
    }
    memcpy(lib_evt->data.evt_connection_retry.address.addr,
           conn->address.addr,
           sizeof(lib_evt->data.evt_connection_retry.address.addr));
    sc = esl_lib_event_list_push_back(lib_evt);

    if (sc != SL_STATUS_OK) {
      // Free up memory on failure
      esl_lib_memory_free(lib_evt);
    }
  }

  return sc;
}
static sl_status_t send_connection_status(esl_lib_connection_t *conn,
                                          esl_lib_bool_t       status,
                                          sl_status_t          reason)
{
  sl_status_t sc;
  esl_lib_evt_t *lib_evt;
  esl_lib_evt_type_t type = ESL_LIB_EVT_CONNECTION_CLOSED;

  if (conn == ESL_LIB_INVALID_HANDLE) {
    return SL_STATUS_NULL_POINTER; // usage error, no event to send!
  }

  // Detect event type (extended if connected)
  if (status == ESL_LIB_TRUE) {
    type = ESL_LIB_EVT_CONNECTION_OPENED;
  }

  sc = esl_lib_event_list_allocate(type,
                                   0,
                                   &lib_evt);
  if (sc == SL_STATUS_OK) {
    // Get pointer to status data in general
    if (status == ESL_LIB_TRUE) {
      // Set handle
      lib_evt->data.evt_connection_opened.connection_handle
        = (esl_lib_connection_handle_t)conn;
      // Set sl_status
      lib_evt->data.evt_connection_opened.status = reason;
      // Copy address
      lib_evt->data.evt_connection_opened.address.address_type = conn->address_type;
      memcpy(lib_evt->data.evt_connection_opened.address.addr,
             conn->address.addr,
             sizeof(lib_evt->data.evt_connection_opened.address.addr));
      // Copy GATT database handles
      memcpy(&lib_evt->data.evt_connection_opened.gattdb_handles,
             &conn->gattdb_handles,
             sizeof(lib_evt->data.evt_connection_opened.gattdb_handles));
      for (uint8_t i = 0; i < sizeof(conn->gattdb_handles.esl_characteristics) / sizeof(uint16_t); i++) {
        esl_lib_log_connection_debug(CONN_FMT "Sending ESL %u characteristic handle = 0x%02x" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     i,
                                     conn->gattdb_handles.esl_characteristics[i]);
      }
      for (uint8_t i = 0; i < sizeof(conn->gattdb_handles.dis_characteristics) / sizeof(uint16_t); i++) {
        esl_lib_log_connection_debug(CONN_FMT "Sending DIS %u characteristic handle = 0x%02x" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     i,
                                     conn->gattdb_handles.dis_characteristics[i]);
      }
    } else {
      // Set handle
      lib_evt->data.evt_connection_closed.connection_handle
        = (esl_lib_connection_handle_t)conn;
      // Set reason for disconnection event
      lib_evt->data.evt_connection_closed.reason = reason;
      // Copy address
      lib_evt->data.evt_connection_closed.address.address_type = conn->address_type;
      memcpy(lib_evt->data.evt_connection_closed.address.addr,
             conn->address.addr,
             sizeof(lib_evt->data.evt_connection_closed.address.addr));
    }

    sc = esl_lib_event_list_push_back(lib_evt);
    if (sc != SL_STATUS_OK) {
      // Free up memory on failure
      esl_lib_memory_free(lib_evt);
    }
  }
  return sc;
}

static sl_status_t send_att_response(esl_lib_connection_t *conn,
                                     esl_lib_evt_type_t   type,
                                     sl_status_t          status)
{
  sl_status_t sc = SL_STATUS_NULL_POINTER;
  esl_lib_evt_t *lib_evt;

  if (conn == ESL_LIB_INVALID_HANDLE) {
    return sc; // usage error, nothing to do!
  }

  // Stop timer
  (void)app_timer_stop(&conn->gatt_timer);

  if (type == ESL_LIB_EVT_CONFIGURE_TAG_RESPONSE) {
    sc = esl_lib_event_list_allocate(type, 0, &lib_evt);
    if (sc == SL_STATUS_OK) {
      // Tag config response
      lib_evt->data.evt_configure_tag_response.connection_handle
        = (esl_lib_connection_handle_t)conn;
      lib_evt->data.evt_configure_tag_response.status = status;
      // Set current config type
      lib_evt->data.evt_configure_tag_response.type = conn->config_type;
    }
  } else if (conn->command != NULL) {
    sc = esl_lib_event_list_allocate(type,
                                     conn->command->data.cmd_write_control_point.data.len,
                                     &lib_evt);
    if (sc == SL_STATUS_OK) {
      // Control Point response
      lib_evt->data.evt_control_point_response.connection_handle
        = (esl_lib_connection_handle_t)conn;
      lib_evt->data.evt_control_point_response.status = status;
      // Copy original data
      lib_evt->data.evt_control_point_response.data_sent.len
        = conn->command->data.cmd_write_control_point.data.len;
      memcpy(lib_evt->data.evt_control_point_response.data_sent.data,
             conn->command->data.cmd_write_control_point.data.data,
             conn->command->data.cmd_write_control_point.data.len);
    }
  } else {
    (void)send_connection_error(conn,
                                ESL_LIB_STATUS_CONN_WRITE_CP_FAILED,
                                sc,
                                conn->state);
  }

  // Push event
  if (sc == SL_STATUS_OK) {
    sc = esl_lib_event_list_push_back(lib_evt);
  } else {
    esl_lib_memory_free(lib_evt);
  }

  return sc;
}

static sl_status_t send_connection_error(esl_lib_connection_t        *conn,
                                         esl_lib_status_t            lib_status,
                                         sl_status_t                 status,
                                         esl_lib_connection_state_t  data)
{
  sl_status_t sc;
  esl_lib_node_id_t node_id;

  if (conn == ESL_LIB_INVALID_HANDLE) {
    return SL_STATUS_NULL_POINTER; // usage error, no error event to send!
  }

  if (conn->command == NULL || is_eslp_ready(conn)) {
    node_id.type = ESL_LIB_NODE_ID_TYPE_CONNECTION;
    node_id.id.connection_handle = (esl_lib_connection_handle_t)conn;
  } else {
    node_id.type = ESL_LIB_NODE_ID_TYPE_ADDRESS;
    node_id.id.address.address_type = conn->address_type;
    memcpy(node_id.id.address.addr,
           conn->address.addr,
           sizeof(node_id.id.address.addr));
  }

  sc = esl_lib_event_push_error(lib_status,
                                &node_id,
                                status,
                                (esl_lib_status_data_t)data);
  return sc;
}

static sl_status_t close_connection(esl_lib_connection_t *conn)
{
  sl_status_t sc = SL_STATUS_OK;

  if (conn == ESL_LIB_INVALID_HANDLE) {
    return SL_STATUS_NULL_POINTER; // usage error, nothing to do!
  } else if (!esl_lib_connection_contains(conn)) {
    return SL_STATUS_DELETED;
  }

  (void)app_timer_stop(&conn->gatt_timer);
  (void)app_timer_stop(&conn->timer);

  if (conn->last_error != SL_STATUS_BT_CTRL_CONNECTION_TERMINATED_BY_LOCAL_HOST) {
    sc = sl_bt_connection_close(conn->connection_handle);
    // Invalidate to avoid later possible redundant close calls that would fail
    conn->last_error = SL_STATUS_BT_CTRL_CONNECTION_TERMINATED_BY_LOCAL_HOST;
    // Prevent executing any future commands until the connection is closed
    conn->command_complete = false;

    (void)app_timer_start(&conn->timer,
                          CLOSE_TIMEOUT_MS,
                          connection_timeout,
                          conn,
                          false);
  }

  return sc;
}

static bool uuid_16_match(uint8_t *uuid_a, uint8_t *uuid_b)
{
  return (memcmp(uuid_a, uuid_b, sizeof(sl_bt_uuid_16_t)) == 0);
}

static void connection_complete(esl_lib_connection_t *conn, sl_status_t result)
{
  if (conn != NULL) {
    (void)send_connection_status(conn, ESL_LIB_TRUE, result);
    conn->state = ESL_LIB_CONNECTION_STATE_CONNECTED;
    // Open command has been completed.
    if (conn->command && conn->command->cmd_code == ESL_LIB_CMD_CONNECT) {
      esl_lib_memory_free(conn->command);
    }
    conn->command_complete = true;
  }
}

static bool check_image_transfer(esl_lib_connection_t *conn)
{
  switch (conn->state) {
    case ESL_LIB_CONNECTION_STATE_OTS_INIT:
    case ESL_LIB_CONNECTION_STATE_OTS_GET_TYPE:
    case ESL_LIB_CONNECTION_STATE_OTS_IMAGE_TRANSFER:
      return true;
    default:
      // Other cases it is connected
      return false;
  }
}

static bool is_eslp_ready(esl_lib_connection_t *conn)
{
  if (conn == ESL_LIB_INVALID_HANDLE) {
    return false;
  }

  switch (conn->state) {
    // Not ready for Config / Update actions according to ESL Profile in connecting phases
    case ESL_LIB_CONNECTION_STATE_OFF:
    case ESL_LIB_CONNECTION_STATE_CONNECTING:
    case ESL_LIB_CONNECTION_STATE_CONNECTION_OPENED:
    case ESL_LIB_CONNECTION_STATE_APPLYING_LTK:
    case ESL_LIB_CONNECTION_STATE_NEW_BOND_REQUIRED:
    case ESL_LIB_CONNECTION_STATE_BONDING:
    case ESL_LIB_CONNECTION_STATE_BONDING_RECOVERY:
    case ESL_LIB_CONNECTION_STATE_SERVICE_DISCOVERY:
    case ESL_LIB_CONNECTION_STATE_DIS_DISCOVERY:
    case ESL_LIB_CONNECTION_STATE_ESL_DISCOVERY:
    case ESL_LIB_CONNECTION_STATE_ESL_SUBSCRIBE:
    case ESL_LIB_CONNECTION_STATE_OTS_INIT:
      return false;
    default:
      // Other cases it is ready
      return true;
  }
}

static void on_image_transfer_status(esl_lib_image_transfer_handle_t handle,
                                     uint8_t                         connection,
                                     esl_image_transfer_state_t      state,
                                     sl_status_t                     result,
                                     esl_lib_ots_gattdb_handles_t    *gattdb_handles)
{
  (void)handle;
  esl_lib_connection_t *conn = ESL_LIB_INVALID_HANDLE;
  sl_status_t sc = esl_lib_connection_find(connection, &conn);
  if (sc == SL_STATUS_OK) {
    if (conn->state == ESL_LIB_CONNECTION_STATE_OTS_INIT && result == SL_STATUS_OK) {
      // Copy handles if present
      if (gattdb_handles != NULL) {
        memcpy(conn->gattdb_handles.ots_characteristics,
               *gattdb_handles,
               sizeof(conn->gattdb_handles.ots_characteristics));
      }

      if (state == ESL_LIB_IMAGE_TRANSFER_STATE_IDLE) {
        // init succeeded
        connection_complete(conn, result);
      }
    }

    if (state == ESL_LIB_IMAGE_TRANSFER_REMOVED) {
      // Clear local handle reference in case of it's been already deleted by an error
      conn->ots_handle = ESL_LIB_INVALID_HANDLE;
    }

    if (check_image_transfer(conn)) {
      if (state == ESL_LIB_IMAGE_TRANSFER_STATE_NOT_INITIALIZED) {
        // Removed since there were an error during init or transfer
        (void)send_connection_error(conn,
                                    ESL_LIB_STATUS_OTS_INIT_FAILED,
                                    result,
                                    conn->state);
        // Close connection as OTS errors are mostly unrecoverable
        esl_lib_log_connection_debug(CONN_FMT "Close connection due image transfer status 0x%04x, connection handle = %u, state = %d" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     result,
                                     conn->connection_handle,
                                     state);
        // Close the connection and discard the address since it is no longer needed for the function
        (void)close_broken_connection(&conn, NULL);
      } else if (state == ESL_LIB_IMAGE_TRANSFER_STATE_ERROR) {
        (void)send_connection_error(conn,
                                    ESL_LIB_STATUS_OTS_ERROR,
                                    result,
                                    conn->state);
        // Let the AP try to continue in this case
        conn->command_complete = true;
        conn->state = ESL_LIB_CONNECTION_STATE_CONNECTED;
      }
    }
  } else {
    esl_lib_log_connection_warning("[Unknown] Image transfer status changed for a deceised connection, no handle available anymore" APP_LOG_NL);
  }
}

static void on_image_transfer_finished(esl_lib_image_transfer_handle_t handle,
                                       uint8_t                         connection,
                                       sl_status_t                     result,
                                       uint8_t                         image_index)
{
  (void)handle;
  esl_lib_connection_t *conn = ESL_LIB_INVALID_HANDLE;
  esl_lib_evt_t        *lib_evt;

  sl_status_t sc = esl_lib_connection_find(connection, &conn);
  if (sc == SL_STATUS_OK) {
    esl_lib_log_connection_debug(CONN_FMT "Image %u transfer finished, result = %u, connection handle = %u" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(conn),
                                 image_index,
                                 result,
                                 conn->connection_handle);

    // Free up image data if still there after ESL_LIB_CMD_WRITE_IMAGE request
    if (conn->command != NULL && conn->command->cmd_code == ESL_LIB_CMD_WRITE_IMAGE && conn->command->data.cmd_write_image.img_data_copied != NULL) {
      esl_lib_memory_free(conn->command->data.cmd_write_image.img_data_copied);
      conn->command_complete = true; // Also report command complete
      conn->state = ESL_LIB_CONNECTION_STATE_CONNECTED; // And return to connected, idle.
    }

    if (result == SL_STATUS_OK) {
      sc = esl_lib_event_list_allocate(ESL_LIB_EVT_IMAGE_TRANSFER_FINISHED, 0, &lib_evt);
      if (sc == SL_STATUS_OK) {
        lib_evt->data.evt_image_transfer_finished.connection_handle = conn;
        lib_evt->data.evt_image_type.img_index = image_index;
        lib_evt->data.evt_image_transfer_finished.status = result;
        (void)esl_lib_event_list_push_back(lib_evt);
      }
    } else {
      (void)send_connection_error(conn,
                                  ESL_LIB_STATUS_OTS_TRANSFER_FAILED,
                                  result,
                                  conn->state);
      if (result == SL_STATUS_TIMEOUT
          || result == SL_STATUS_FAIL
          || result == SL_STATUS_INITIALIZATION
          || result == SL_STATUS_TRANSMIT_INCOMPLETE
          || result == SL_STATUS_NO_MORE_RESOURCE
          || result == SL_STATUS_BT_CTRL_CONNECTION_REJECTED_DUE_TO_NO_SUITABLE_CHANNEL_FOUND) {
        esl_lib_log_connection_debug(CONN_FMT "Close connection due image transfer finished result: 0x%04x, connection handle = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     result,
                                     conn->connection_handle);
        // Close connection as some OTS errors are unrecoverable
        (void)close_broken_connection(&conn, NULL);
      }
    }
  }
}

static void on_image_transfer_type_arrived(esl_lib_image_transfer_handle_t handle,
                                           uint8_t                         connection,
                                           sl_status_t                     result,
                                           uint8_t                         image_index,
                                           uint8_t                         *object_type,
                                           uint8_t                         len)
{
  (void)handle;
  esl_lib_connection_t *conn = ESL_LIB_INVALID_HANDLE;
  esl_lib_evt_t        *lib_evt;
  sl_status_t sc = esl_lib_connection_find(connection, &conn);
  if (sc == SL_STATUS_OK) {
    esl_lib_log_connection_debug(CONN_FMT "Read image %u type finished, result = %u, connection handle = %u" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(conn),
                                 image_index,
                                 result,
                                 conn->connection_handle);
    if (result == SL_STATUS_OK) {
      conn->command_complete = true;
      conn->state = ESL_LIB_CONNECTION_STATE_CONNECTED;
      sc = esl_lib_event_list_allocate(ESL_LIB_EVT_IMAGE_TYPE, len, &lib_evt);
      if (sc == SL_STATUS_OK) {
        lib_evt->data.evt_image_type.connection_handle = conn;
        lib_evt->data.evt_image_type.img_index = image_index;
        lib_evt->data.evt_image_type.type_data.len = len;
        // Copy data
        memcpy(lib_evt->data.evt_image_type.type_data.data,
               object_type,
               len);
        // Push event
        (void)esl_lib_event_list_push_back(lib_evt);
      }
    } else {
      (void)send_connection_error(conn,
                                  ESL_LIB_STATUS_OTS_META_READ_FAILED,
                                  result,
                                  conn->state);
      esl_lib_log_connection_debug(CONN_FMT "OTS Image get type fail (0x%04x), image index: %u, connection handle = %u" APP_LOG_NL,
                                   ESL_LIB_LOG_PTR(conn),
                                   result,
                                   image_index,
                                   conn->connection_handle);
    }
  }
}

static void gatt_timeout(app_timer_t *timer,
                         void        *data)
{
  (void)timer;
  esl_lib_connection_t *conn = (esl_lib_connection_t *)data;
  // Check if it exists
  if (esl_lib_connection_contains(conn)) {
    esl_lib_log_connection_error(CONN_FMT "GATT timeout, connection handle = %u" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(conn),
                                 conn->connection_handle);
    if (conn->command == NULL) {
      esl_lib_log_connection_debug(CONN_FMT "Close connection due GATT timout during discovery phase!" APP_LOG_NL,
                                   ESL_LIB_LOG_PTR(conn));
    } else if (conn->command->cmd_code == ESL_LIB_CMD_WRITE_CONTROL_POINT) {
      // Send event
      (void)send_att_response(conn,
                              ESL_LIB_EVT_CONTROL_POINT_RESPONSE,
                              SL_STATUS_TIMEOUT);
      conn->command_complete = true;
    } else if (conn->command->cmd_code == ESL_LIB_CMD_CONFIGURE_TAG) {
      // Send event
      (void)send_att_response(conn,
                              ESL_LIB_EVT_CONFIGURE_TAG_RESPONSE,
                              SL_STATUS_TIMEOUT);
      esl_lib_log_connection_debug(CONN_FMT "Close connection due GATT timout during configuring phase!" APP_LOG_NL,
                                   ESL_LIB_LOG_PTR(conn));
    }
    // Close connection as GATT errors during configuration phase are unrecoverable
    (void)close_broken_connection(&conn, NULL);
  } else {
    esl_lib_log_connection_warning(CONN_FMT "GATT timeout for unknown connection!" APP_LOG_NL,
                                   ESL_LIB_LOG_PTR(data));
  }
}

static void reconnect_timeout(app_timer_t *timer,
                              void        *data)
{
  (void)timer;
  sl_status_t sc;
  esl_lib_connection_t *conn = (esl_lib_connection_t *)data;

  esl_lib_log_connection_debug(CONN_FMT "Reconnect timer rised" APP_LOG_NL,
                               ESL_LIB_LOG_PTR(conn));

  if (esl_lib_connection_contains(conn)) {
    esl_lib_log_connection_debug(CONN_FMT "Connection retry to " ESL_LIB_LOG_ADDR_FORMAT ", expired handle = %u" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(conn),
                                 ESL_LIB_LOG_ADDR(conn->command->data.cmd_connect.address),
                                 conn->connection_handle);
    if (conn->command && conn->command->cmd_code == ESL_LIB_CMD_CONNECT) {
      // Resend connect command to core queue
      sc = esl_lib_core_add_command(conn->command);
    } else {
      // It's an error condition if there's no active command for the connection or it's not an ESL_LIB_CMD_CONNECT request!
      sc = SL_STATUS_DELETED;
    }

    if (sc != SL_STATUS_OK) {
      (void)send_connection_error(conn,
                                  ESL_LIB_STATUS_CONN_FAILED,
                                  sc,
                                  conn->state);
      esl_lib_log_connection_error(CONN_FMT "Failed to reopen connection, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                                   ESL_LIB_LOG_PTR(conn),
                                   conn->connection_handle,
                                   sc);
      // And also remove connection from the list.
      (void)esl_lib_connection_remove_ptr(conn);
    } else {
      // Decouple the connect command as it's been successfully queued again - we'll take ownership again, later.
      conn->command = NULL;
      // Invalidate the handle already closed by ble_stack
      conn->connection_handle = SL_BT_INVALID_CONNECTION_HANDLE;
      // Signal command complete for last (whatever) command
      conn->command_complete = true;
      // Update the connection state
      conn->state = ESL_LIB_CONNECTION_STATE_OFF;
    }
  } else {
    esl_lib_log_connection_warning(CONN_FMT "Reconnect handle not found (possibly has already been removed)" APP_LOG_NL,
                                   ESL_LIB_LOG_PTR(conn));
  }
}

static void connection_timeout(app_timer_t *timer,
                               void        *data)
{
  (void)timer;
  sl_status_t sc;
  esl_lib_status_t status = ESL_LIB_STATUS_CONN_TIMEOUT;
  esl_lib_connection_t *conn = (esl_lib_connection_t *)data;
  // Check if it exists
  if (esl_lib_connection_contains(conn)) {
    if (conn->last_error == SL_STATUS_BT_CTRL_CONNECTION_TERMINATED_BY_LOCAL_HOST) {
      // This is the timeout for earlier local close request! Send error for closing because close event didn't arrive.
      status = ESL_LIB_STATUS_CONN_CLOSE_FAILED;
      sc = sl_bt_connection_forcefully_close(conn->connection_handle);
      esl_lib_log_connection_error(CONN_FMT "Timeout for closing the connection handle = %u, forcefully close result: 0x%04x" APP_LOG_NL,
                                   ESL_LIB_LOG_PTR(conn),
                                   conn->connection_handle,
                                   sc);
    } else {
      if (conn->state == ESL_LIB_CONNECTION_STATE_PAST_CLOSE_CONNECTION) {
        esl_lib_log_connection_warning(CONN_FMT "Timeout for PAST close the connection handle = %u" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       conn->connection_handle);
      } else {
        esl_lib_log_connection_warning(CONN_FMT "Timeout for opening the connection handle = %u" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       conn->connection_handle);
      }
      // Try to gentle close the connection in the first place
      sc = close_connection(conn);
      if (sc != SL_STATUS_OK) {
        status = ESL_LIB_STATUS_CONN_CLOSE_FAILED;
        esl_lib_log_connection_error(CONN_FMT "Connection close request failed in timeout handler, connection handle = %u, sc = 0x%04x" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle,
                                     sc);
      }
    }

    (void)send_connection_error(conn,
                                status,
                                SL_STATUS_TIMEOUT,
                                conn->state);

    if (status == ESL_LIB_STATUS_CONN_CLOSE_FAILED) {
      if (conn->state == ESL_LIB_CONNECTION_STATE_CONNECTING) {
        // Report connection complete only if the connection opened event didn't arrive before the timeout
        esl_lib_core_connection_complete();
      }
      // Remove connection
      (void)esl_lib_connection_remove_ptr(conn);
    }
  } else {
    esl_lib_log_connection_warning(CONN_FMT "Connection timeout for unknown connection" APP_LOG_NL,
                                   ESL_LIB_LOG_PTR(conn));
  }
}

static uint16_t get_handle_for_type(esl_lib_connection_t *conn,
                                    esl_lib_data_type_t  tag_info_type)
{
  uint16_t char_handle = ESL_LIB_INVALID_CHARACTERISTIC_HANDLE;
  switch (tag_info_type) {
    case ESL_LIB_DATA_TYPE_GATT_DISPLAY_INFO:
      char_handle = conn->gattdb_handles.esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_DISPLAY_INFORMATION];
      break;
    case ESL_LIB_DATA_TYPE_GATT_IMAGE_INFO:
      char_handle = conn->gattdb_handles.esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_IMAGE_INFORMATION];
      break;
    case ESL_LIB_DATA_TYPE_GATT_SENSOR_INFO:
      char_handle = conn->gattdb_handles.esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_SENSOR_INFORMATION];
      break;
    case ESL_LIB_DATA_TYPE_GATT_LED_INFO:
      char_handle = conn->gattdb_handles.esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_LED_INFORMATION];
      break;
    case ESL_LIB_DATA_TYPE_GATT_PNP_ID:
      char_handle = conn->gattdb_handles.dis_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_DIS_PNP_ID];
      break;
    case ESL_LIB_DATA_TYPE_GATT_SERIAL_NUMBER:
      char_handle = conn->gattdb_handles.dis_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_DIS_SN];
      break;
    default:
      break;
  }
  return char_handle;
}

static esl_lib_data_type_t get_next_type(esl_lib_data_type_t type)
{
  esl_lib_data_type_t next_type = ESL_LIB_DATA_TYPE_UNINITIALIZED;
  switch (type) {
    case ESL_LIB_DATA_TYPE_UNINITIALIZED:
      next_type = ESL_LIB_DATA_TYPE_GATT_DISPLAY_INFO;
      break;
    case ESL_LIB_DATA_TYPE_GATT_DISPLAY_INFO:
      next_type = ESL_LIB_DATA_TYPE_GATT_IMAGE_INFO;
      break;
    case ESL_LIB_DATA_TYPE_GATT_IMAGE_INFO:
      next_type = ESL_LIB_DATA_TYPE_GATT_SENSOR_INFO;
      break;
    case ESL_LIB_DATA_TYPE_GATT_SENSOR_INFO:
      next_type = ESL_LIB_DATA_TYPE_GATT_LED_INFO;
      break;
    case ESL_LIB_DATA_TYPE_GATT_LED_INFO:
      next_type = ESL_LIB_DATA_TYPE_GATT_PNP_ID;
      break;
    case ESL_LIB_DATA_TYPE_GATT_PNP_ID:
      next_type = ESL_LIB_DATA_TYPE_GATT_SERIAL_NUMBER;
      break;
    case ESL_LIB_DATA_TYPE_GATT_SERIAL_NUMBER:
      next_type = ESL_LIB_DATA_TYPE_GATT_CONTROL_POINT;
      break;
    default:
      break;
  }
  return next_type;
}

static sl_status_t get_tag_info_finish(esl_lib_connection_t *conn, sl_status_t status)
{
  uint32_t data_size = 0;
  size_t current_size = 0;
  esl_lib_connection_tag_info_storage_t *current;
  esl_lib_evt_t *event;
  uint8_t *ptr;
  esl_lib_tlv_t *tlv;
  sl_status_t sc = status;

  // Stop GATT watchdog timer
  (void)app_timer_stop(&conn->gatt_timer);
  // Calculate event size from storage list
  SL_SLIST_FOR_EACH_ENTRY(conn->tag_info_list, current, esl_lib_connection_tag_info_storage_t, node) {
    // Get current size
    esl_lib_storage_get_size(current->storage, &current_size);
    // Append required size
    data_size += (current_size + ESL_LIB_TLV_HEADER_LEN);
  }

  esl_lib_log_connection_debug(CONN_FMT "Tag info TLV data size = %u , TLV header size= %u, connection handle = %u" APP_LOG_NL,
                               ESL_LIB_LOG_PTR(conn),
                               data_size,
                               (unsigned int)ESL_LIB_TLV_HEADER_LEN,
                               conn->connection_handle);

  // Create event
  esl_lib_event_list_allocate(ESL_LIB_EVT_TAG_INFO, data_size, &event);

  // Set event code
  event->evt_code = ESL_LIB_EVT_TAG_INFO;
  // Set habdke
  event->data.evt_tag_info.connection_handle = (esl_lib_connection_handle_t)conn;
  // Set size
  event->data.evt_tag_info.tlv_data.len = data_size;

  // Get pointer to the TLV data content
  ptr = event->data.evt_tag_info.tlv_data.data;

  // Compose TLVs from storages in the list
  // Iterate over the list
  while ((current = (esl_lib_connection_tag_info_storage_t *)sl_slist_pop(&conn->tag_info_list)) != NULL) {
    // TLV pointer
    tlv = (esl_lib_tlv_t *)ptr;
    // Set up type for TLV
    tlv->type = current->type;
    // Get current size
    esl_lib_storage_get_size(current->storage, &current_size);
    // Set up length for TLV
    tlv->data.len = (uint32_t)current_size;
    // Get value for TLV
    esl_lib_storage_copy(current->storage, tlv->data.data);
    // Free up storage
    esl_lib_storage_delete(&current->storage);
    // Free list item also
    esl_lib_memory_free(current);
    // Move pointer to the next TLV
    ptr += (current_size + ESL_LIB_TLV_HEADER_LEN);

    esl_lib_log_connection_debug(CONN_FMT "TLV type %u copied with data size = %u, connection handle = %u" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(conn),
                                 tlv->type,
                                 tlv->data.len,
                                 conn->connection_handle);
  }

  if (sc != SL_STATUS_OK) {
    esl_lib_log_connection_error(CONN_FMT "Get tag info finished with error status: 0x%04x, connection handle = %u" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(conn),
                                 sc,
                                 conn->connection_handle);
    // Force close as keeping connected would break the AP procedure flow
    (void)close_broken_connection(&conn, NULL);
  } else {
    // Send event
    sc = esl_lib_event_list_push_back(event);

    if (sc == SL_STATUS_OK) {
      esl_lib_log_connection_debug(CONN_FMT "Get tag info finished, connection handle = %u" APP_LOG_NL,
                                   ESL_LIB_LOG_PTR(conn),
                                   conn->connection_handle);
    } else {
      esl_lib_log_connection_error(CONN_FMT "Get tag info finished but failed to send tag info, connection handle = %u." APP_LOG_NL,
                                   ESL_LIB_LOG_PTR(conn),
                                   conn->connection_handle);
      // Force close as keeping connected would break the AP procedure flow
      (void)close_broken_connection(&conn, NULL);
    }
  }

  if (conn != NULL) {
    // Clear type
    conn->tag_info_type = ESL_LIB_DATA_TYPE_UNINITIALIZED;

    // Set command complete
    conn->command_complete = true;
  } else {
    esl_lib_log_connection_warning("[Unknown] Get tag info finished for a deceised connection, no handle available anymore" APP_LOG_NL);
  }

  return sc;
}

static sl_status_t get_next_tag_info(esl_lib_connection_t *conn)
{
  sl_status_t sc = SL_STATUS_OK;
  uint16_t char_handle = ESL_LIB_INVALID_CHARACTERISTIC_HANDLE;
  esl_lib_data_type_t type = conn->tag_info_type;

  (void)app_timer_stop(&conn->gatt_timer);
  esl_lib_log_connection_debug(CONN_FMT "Get next tag info, connection handle = %u" APP_LOG_NL,
                               ESL_LIB_LOG_PTR(conn),
                               conn->connection_handle);

  // Find a valid handle for next read
  while ( !(char_handle != ESL_LIB_INVALID_CHARACTERISTIC_HANDLE
            || type == ESL_LIB_DATA_TYPE_GATT_CONTROL_POINT)) {
    type = get_next_type(type);
    char_handle = get_handle_for_type(conn, type);
    esl_lib_log_connection_debug(CONN_FMT "Next tag info type is %u (0x%02x), connection handle = %u" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(conn),
                                 type,
                                 char_handle,
                                 conn->connection_handle);
  }

  if (char_handle != ESL_LIB_INVALID_CHARACTERISTIC_HANDLE && type != ESL_LIB_DATA_TYPE_GATT_CONTROL_POINT) {
    // Found a valid handle, create storage
    sc = sl_bt_gatt_read_characteristic_value(conn->connection_handle,
                                              char_handle);
    if (sc == SL_STATUS_OK) {
      esl_lib_log_connection_debug(CONN_FMT "Next tag info type %u read requested, connection handle = %u" APP_LOG_NL,
                                   ESL_LIB_LOG_PTR(conn),
                                   type,
                                   conn->connection_handle);
      // Move to next type
      conn->tag_info_type = type;
      // Create storage for storing data
      sc = esl_lib_storage_create(&conn->tag_info_data);

      if (sc == SL_STATUS_OK) {
        sc = app_timer_start(&conn->gatt_timer,
                             GATT_TIMEOUT_MS,
                             gatt_timeout,
                             conn,
                             false);
      }
    }
    if (sc != SL_STATUS_OK) {
      esl_lib_log_connection_error(CONN_FMT "Failed to read tag info type %u, connection handle = %u" APP_LOG_NL,
                                   ESL_LIB_LOG_PTR(conn),
                                   type,
                                   conn->connection_handle);
      sc = get_tag_info_finish(conn, sc);
    }
  } else {
    // No valid handle found, finish
    sc = get_tag_info_finish(conn, sc);
  }

  return sc;
}

static void clean_tag_info(esl_lib_connection_t *conn)
{
  esl_lib_connection_tag_info_storage_t *current;

  // Delete temporary data
  esl_lib_storage_delete(&conn->tag_info_data);

  // Delete saved data
  while ((current = (esl_lib_connection_tag_info_storage_t *)sl_slist_pop(&conn->tag_info_list)) != NULL) {
    esl_lib_storage_delete(&current->storage);
    esl_lib_memory_free(current);
  }
}

static sl_status_t save_tag_info(esl_lib_connection_t *conn)
{
  sl_status_t sc = SL_STATUS_ALLOCATION_FAILED;
  esl_lib_connection_tag_info_storage_t *tag_info;

  // Allocate memory for the list item
  tag_info = (esl_lib_connection_tag_info_storage_t *)esl_lib_memory_allocate(sizeof(esl_lib_connection_tag_info_storage_t));
  if (tag_info != NULL) {
    // Save type
    tag_info->type = conn->tag_info_type;
    // Save storage handle
    tag_info->storage = conn->tag_info_data;
    // Clear storage handle from connection
    conn->tag_info_data = NULL;
    // Add to the list
    sl_slist_push(&conn->tag_info_list, &tag_info->node);
    // Set positive result
    sc = SL_STATUS_OK;
  }
  return sc;
}

static sl_status_t write_next_config_value(esl_lib_connection_t *conn)
{
  sl_status_t sc = SL_STATUS_NULL_POINTER;

  if (conn != NULL && conn->command != NULL) {
    bool move_to_next = true;

    while (move_to_next) {
      esl_lib_tlv_t *tlv = NULL;
      esl_lib_log_connection_debug(CONN_FMT "Next configure tag TLV data %u / %u, connection handle = %u" APP_LOG_NL,
                                   ESL_LIB_LOG_PTR(conn),
                                   conn->config_index,
                                   conn->command->data.cmd_configure_tag.tlv_data.len,
                                   conn->connection_handle);

      // Check if it was the last TLV
      if (conn->config_index >= conn->command->data.cmd_configure_tag.tlv_data.len) {
        // This was the last TLV
        move_to_next = false;
        // Consider command completed
        conn->command_complete = true;
        // Reset the state
        conn->state = ESL_LIB_CONNECTION_STATE_CONNECTED;
        sc = SL_STATUS_OK;
        esl_lib_log_connection_debug(CONN_FMT "Finished configure tag, connection handle = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle);
      } else {
        // Set TLV
        tlv = (esl_lib_tlv_t *)&(conn->command->data.cmd_configure_tag.tlv_data.data[conn->config_index]);

        // Set current type for the connection
        conn->config_type = tlv->type;

        esl_lib_log_connection_debug(CONN_FMT "Write next value from index %u, type %u, connection handle = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->config_index,
                                     (uint8_t)conn->config_type,
                                     conn->connection_handle);

        // Try to write the current value
        sc = write_value(conn,
                         ESL_LIB_TRUE,
                         tlv->type,
                         tlv->data.len,
                         tlv->data.data);
        if (sc == SL_STATUS_OK) {
          // Set state, check for this later in procedure completed event
          conn->state = ESL_LIB_CONNECTION_STATE_CONFIGURE_TAG;
          // Return and wait for procedure completed event
          move_to_next = false;
          esl_lib_log_connection_debug(CONN_FMT "Wait for response to index %u, type %u, connection handle = %u" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       conn->config_index,
                                       (uint8_t)conn->config_type,
                                       conn->connection_handle);
        } else {
          // Send back an immediate response in case of error
          (void)send_att_response(conn,
                                  ESL_LIB_EVT_CONFIGURE_TAG_RESPONSE,
                                  sc);
        }
      }

      conn->config_index += sizeof(esl_lib_tlv_t);

      if (!conn->command_complete) {
        // Move to next TLV until the very last one reached
        conn->config_index += tlv->data.len;
      }
    }
  }
  return sc;
}

static sl_status_t write_value(esl_lib_connection_t *conn,
                               esl_lib_bool_t       response,
                               esl_lib_data_type_t  type,
                               uint32_t             len,
                               uint8_t              *data)
{
  sl_status_t sc = SL_STATUS_INVALID_TYPE;
  uint16_t characteristic = ESL_LIB_INVALID_CHARACTERISTIC_HANDLE;
  uint16_t sent_len = 0;

  switch (type) {
    case ESL_LIB_DATA_TYPE_GATT_ESL_ADDRESS:
      characteristic = conn->gattdb_handles.esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_ADDRESS];
      break;
    case ESL_LIB_DATA_TYPE_GATT_AP_SYNC_KEY:
      characteristic = conn->gattdb_handles.esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_AP_SYNC_KEY_MATERIAL];
      break;
    case ESL_LIB_DATA_TYPE_GATT_RESPONSE_KEY:
      characteristic = conn->gattdb_handles.esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_RESPONSE_KEY_MATERIAL];
      break;
    case ESL_LIB_DATA_TYPE_GATT_CURRENT_TIME:
      characteristic = conn->gattdb_handles.esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_CURRENT_ABSOLUTE_TIME];
      break;
    case ESL_LIB_DATA_TYPE_GATT_CONTROL_POINT:
      characteristic = conn->gattdb_handles.esl_characteristics[ESL_LIB_CHARACTERISTIC_INDEX_ESL_CONTROL_POINT];
      break;
    default:
      break;
  }
  if (characteristic != ESL_LIB_INVALID_CHARACTERISTIC_HANDLE) {
    esl_lib_log_connection_debug(CONN_FMT "Writing value type %u (0x%02x), connection handle = %u" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(conn),
                                 type,
                                 characteristic,
                                 conn->connection_handle);
    if (response == ESL_LIB_TRUE) {
      (void)app_timer_stop(&conn->gatt_timer);
      sc = app_timer_start(&conn->gatt_timer,
                           GATT_TIMEOUT_MS,
                           gatt_timeout,
                           conn,
                           false);
      if (sc == SL_STATUS_OK) {
        sc = sl_bt_gatt_write_characteristic_value(conn->connection_handle,
                                                   characteristic,
                                                   len,
                                                   data);
        if (sc != SL_STATUS_OK) {
          (void)app_timer_stop(&conn->gatt_timer);
        } else {
          esl_lib_log_connection_debug(CONN_FMT "Writing value type %u (0x%02x) succeeded, waiting for response, connection handle = %u" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       type,
                                       characteristic,
                                       conn->connection_handle);
        }
      }
    } else {
      sc = sl_bt_gatt_write_characteristic_value_without_response(conn->connection_handle,
                                                                  characteristic,
                                                                  len,
                                                                  data,
                                                                  &sent_len);
      if (sc == SL_STATUS_OK && sent_len == len) {
        esl_lib_log_connection_debug(CONN_FMT "Writing value type %u (0x%02x) succeeded, connection handle = %u" APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     type,
                                     characteristic,
                                     conn->connection_handle);
      } else {
        esl_lib_log_connection_warning(CONN_FMT "Writing value type %u (0x%02x) unsuccessful, connection handle = %u, length/sent: %u/%u, sc = 0x%04x" APP_LOG_NL,
                                       ESL_LIB_LOG_PTR(conn),
                                       type,
                                       characteristic,
                                       conn->connection_handle,
                                       len,
                                       sent_len,
                                       sc);
      }
    }
  } else {
    esl_lib_log_connection_error(CONN_FMT "Invalid GATT handle for value type %u, connection handle = %u" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(conn),
                                 type,
                                 conn->connection_handle);
    sc = SL_STATUS_INVALID_HANDLE;
  }
  return sc;
}

static bool find_tlv(esl_lib_command_list_cmd_t  *cmd,
                     esl_lib_connect_data_type_t type,
                     esl_lib_connect_tlv_t       **tlv_out)
{
  uint32_t data_index = 0;
  esl_lib_connect_tlv_t *tlv;

  if (cmd == NULL || tlv_out == NULL) {
    return false;
  }

  while (data_index < cmd->data.cmd_connect.tlv_data.len) {
    // Get TLV at data index
    tlv = (esl_lib_connect_tlv_t *)&cmd->data.cmd_connect.tlv_data.data[data_index];
    // Check type
    if (tlv->type == type) {
      // Set output pointer and return
      *tlv_out = tlv;
      return true;
    }
    // Move data index
    data_index += sizeof(esl_lib_tlv_t) + tlv->data.len;
  }

  return false;
}

static void *close_broken_connection(esl_lib_connection_t **conn, esl_lib_address_t *backup)
{
  if (conn == NULL || *conn == ESL_LIB_INVALID_HANDLE
      || !esl_lib_connection_contains(*conn)
      || (*conn)->connection_handle == SL_BT_INVALID_CONNECTION_HANDLE
      || (*conn)->last_error == SL_STATUS_BT_CTRL_CONNECTION_TERMINATED_BY_LOCAL_HOST) {
    // Nothing left to close (second invocation can happen on the same connection in edge cases, especially in case of various OTS errors)
    return NULL;
  } else {
    (void)app_timer_stop(&(*conn)->timer);
    (void)app_timer_stop(&(*conn)->gatt_timer);
    esl_lib_log_connection_debug(CONN_FMT "Cleanup command list for connection handle = %u during close on error" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(*conn),
                                 (*conn)->connection_handle);
    esl_lib_command_list_cleanup(&(*conn)->command_list);
    // Fill up backup adress if needed
    if (backup != NULL) {
      backup->address_type = (*conn)->address_type;
      memcpy(backup->address.addr, (*conn)->address.addr, sizeof(backup->address.addr));
    }
  }

  sl_status_t sc = close_connection(*conn);
  if (sc != SL_STATUS_OK) {
    esl_lib_log_connection_error(CONN_FMT "Closing request failed with status: 0x%04x on connection handle = %u during close on error" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(*conn),
                                 sc,
                                 (*conn)->connection_handle);
    // Send error event about the deletion of the connection pointer
    send_connection_status(*conn, ESL_LIB_FALSE, SL_STATUS_BT_CTRL_UNKNOWN_CONNECTION_IDENTIFIER);
    // Force removal of connection handle in case of error on sl_bt_connection_close request
    (void)esl_lib_connection_remove_ptr(*conn);
    *conn = ESL_LIB_INVALID_HANDLE;
  } else {
    esl_lib_log_connection_debug(CONN_FMT "Requested closing connection with handle = %u on error" APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(*conn),
                                 (*conn)->connection_handle);
  }

  return (void *)backup;
}

static sl_status_t esl_lib_initiate_auto_connection(esl_lib_connection_t *handle)
{
  sl_status_t               sc = SL_STATUS_OK;
  esl_lib_connection_t      *conn = handle;
  uint8_t                   connection_handle = SL_BT_INVALID_CONNECTION_HANDLE;
  esl_lib_core_state_t      core_status;
  esl_lib_connection_mode_t mode = esl_lib_get_connection_mode_and_status(&core_status, NULL, NULL);
  filter_data_p             command = NULL;

  if (mode == ESL_LIB_CONNECTION_MODE_SINGLE) {
    return SL_STATUS_INVALID_STATE;
  }

  if (core_status == ESL_LIB_CORE_STATE_CONNECTING) {
    // esl_lib_initiate_auto_connection() will be invoked later by the connection open / close event processors.
    return SL_STATUS_OK;
  }

  esl_lib_log_connection_debug("Preparing to activate Initiator Filter Policy" APP_LOG_NL);

  // Check if identity recovery is needed
  if (foreign_initiator_id) {
    bd_addr default_identity;
    uint8_t default_address_type;
    sc = esl_lib_core_get_identity_address(&default_identity, &default_address_type);

    if (sc == SL_STATUS_OK) {
      sc = sl_bt_gap_set_identity_address(default_identity, default_address_type);
    }
    // Re-check status as it may have changed
    if (sc != SL_STATUS_OK) {
      esl_lib_log_connection_error("Identity address recovery failed, sc = 0x%04x" APP_LOG_NL, sc);
    }

    foreign_initiator_id = false;
  }

  if (sc == SL_STATUS_OK && last_io_capabilities != sl_bt_sm_io_capability_noinputnooutput) {
    // Auto connection mode does only support the following SM config
    sc = sl_bt_sm_configure(0, sl_bt_sm_io_capability_noinputnooutput);
    if (sc != SL_STATUS_OK) {
      esl_lib_log_connection_error("Failed to configure SM, sc = 0x%04x" APP_LOG_NL, sc);
      return SL_STATUS_BT_SMP_AUTHENTICATION_REQUIREMENTS;
    } else {
      last_io_capabilities = sl_bt_sm_io_capability_noinputnooutput;
    }
  }

  while (sc == SL_STATUS_OK
         && filter_accept_list_get_size(auto_acceptance_list)) {
    // Get the first node from the overall acceptance list
    command = filter_accept_list_pop_first_node(auto_acceptance_list);
    // Convert between address formats for the stack
    bd_addr *addr = (bd_addr *)&(command->data.cmd_connect.address.addr);
    uint8_t address_type = command->data.cmd_connect.address.address_type;

    // Add address to the link layer's accept list via stack API
    sc = sl_bt_accept_list_add_device_by_address(*addr,
                                                 address_type);
    if (sc != SL_STATUS_OK) {
      esl_lib_log_level_t level = (sc == SL_STATUS_BT_CTRL_MEMORY_CAPACITY_EXCEEDED)
                                  ? ESL_LIB_LOG_LEVEL_DEBUG : ESL_LIB_LOG_LEVEL_WARNING;

      // If we couldn't pass it to the stack for any reason, then put it back to the acceptance list
      (void)filter_accept_list_insert_command(auto_acceptance_list, command);
      esl_lib_log(level, ESL_LIB_LOG_MODULE_CONNECTION,
                  CONN_FMT "Stack busy, sc = 0x%04x - connection request deferred to source accept list." APP_LOG_NL,
                  ESL_LIB_LOG_PTR(conn),
                  sc);
    } else {
      // Otherwise put it on the auto initiator list
      (void)filter_accept_list_insert_command(auto_initiator_list, command);
    }
  }

  // Check if there's anything on the active list
  if (filter_accept_list_get_size(auto_initiator_list) == 0) {
    // Check for state inconsistency: library has no pending connections but NCP reports its list is full
    if (sc == SL_STATUS_BT_CTRL_MEMORY_CAPACITY_EXCEEDED) {
      // Discrepancy between the target NCP state and the library state - try forced recovery
      sc = sl_bt_accept_list_remove_all_devices();
      if (sc != SL_STATUS_OK) {
        esl_lib_log_connection_error("Failed to clear link layer acceptance list during auto connection recovery, sc = 0x%04x" APP_LOG_NL, sc);
      }
      // Emit mass error events for pending connections to prevent them from getting stuck in initiating state in the Access Point
      esl_lib_connection_emit_mass_errors(NULL); // NULL means all connections on the auto acceptance list
    }
    return sc;
  }

  if (sc == SL_STATUS_OK || sc == SL_STATUS_BT_CTRL_MEMORY_CAPACITY_EXCEEDED) {
    // Enable Initiator Filter Policy if everything went OK so far
    sc = sl_bt_connection_open_with_accept_list(sl_bt_gap_phy_1m, &connection_handle);
  }
  // Do final status check to decide whether or not to generate an error event
  if (sc == SL_STATUS_OK) {
    if (conn == ESL_LIB_INVALID_HANDLE) {
      // Allocate and add the connection to the connection list.
      sc = esl_lib_connection_add(connection_handle, &conn);
    } else {
      conn->connection_handle = connection_handle;
      conn->last_error = SL_STATUS_IN_PROGRESS; // Operation is in progress and not yet complete (pass or fail)
      conn->established = false;
      sc = app_timer_start(&conn->timer,
                           CONNECTION_TIMEOUT_MS,
                           connection_timeout,
                           conn,
                           false);
      if (sc != SL_STATUS_OK) {
        esl_lib_log_connection_error(CONN_FMT "Connection timeout reinit failed, handle = %u, sc = 0x%04x. Closing." APP_LOG_NL,
                                     ESL_LIB_LOG_PTR(conn),
                                     conn->connection_handle,
                                     sc);
        (void)close_connection(conn);
        return sc; // Skip the list operations below for this kind of error
      }
    }

    if (sc == SL_STATUS_OK) {
      // Move to connecting state with no error present.
      conn->state = ESL_LIB_CONNECTION_STATE_CONNECTING;
      // Set command initially to NULL for auto conn handles - it will be updated on successful connection open
      conn->command = NULL;
      conn->command_complete = false;
      esl_lib_log_connection_debug(CONN_FMT "Pending new auto connection to next address" APP_LOG_NL,
                                   ESL_LIB_LOG_PTR(conn));
    }
  } else if (sc != SL_STATUS_BT_CTRL_CONNECTION_LIMIT_EXCEEDED && sc != SL_STATUS_NO_MORE_RESOURCE) {
    // Remove all accepted device at Link Layer level on errors that we can't handle otherwise
    (void)sl_bt_accept_list_remove_all_devices();
    esl_lib_log_connection_debug(CONN_FMT "Auto connection deferred, sc = 0x%04x - restoring source acceptance list." APP_LOG_NL,
                                 ESL_LIB_LOG_PTR(conn),
                                 sc);
    // Check if any list recovery is needed
    while ((command = filter_accept_list_pop_first_node(auto_initiator_list)) != NULL ) {
      // Try recover the internal acceptance list at least
      filter_accept_list_insert_command(auto_acceptance_list, command);
    }
  } else {
    // The library automatically handles reaching the connection limit, do not report this as an error in auto mode.
    sc = SL_STATUS_OK;
  }

  return sc;
}

static void esl_lib_connection_emit_mass_errors(esl_lib_connection_t *conn)
{
  filter_list_p target_list;
  esl_lib_connection_t *working_conn;
  esl_lib_connection_t local_conn = { 0 }; // Initialize to zero for local use

  if (conn != NULL) {
    target_list = auto_initiator_list;
    working_conn = conn;
  } else {
    target_list = auto_acceptance_list;
    working_conn = &local_conn;
    // Set default state for error emission
    local_conn.state = ESL_LIB_CONNECTION_STATE_OFF;
  }

  filter_data_p node_data;
  while ((node_data = filter_accept_list_pop_first_node(target_list)) != NULL) {
    working_conn->command = node_data;
    // Set temporary BLE address for the connection handle
    working_conn->address_type = node_data->data.cmd_connect.address.address_type;
    memcpy(working_conn->address.addr,
           node_data->data.cmd_connect.address.addr,
           sizeof(working_conn->address.addr));

    (void)send_connection_error(working_conn,
                                ESL_LIB_STATUS_CONN_FAILED,
                                SL_STATUS_BT_CTRL_CONNECTION_ACCEPT_TIMEOUT_EXCEEDED,
                                working_conn->state);
    // Free node data, i.e. the connect command after it is no longer needed
    esl_lib_memory_free(node_data);
  }

  // Only invalidate address and command fields if conn was provided
  if (conn != NULL) {
    memset(conn->address.addr, 0, sizeof(conn->address.addr));
    conn->address_type = sl_bt_gap_public_address;
    conn->command = NULL;
  }
}
