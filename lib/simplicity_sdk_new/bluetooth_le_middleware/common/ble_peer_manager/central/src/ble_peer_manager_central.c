/***************************************************************************//**
 * @file
 * @brief Bluetooth Peer Manager - central
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
#include "sl_bt_api.h"
#include "app_timer.h"
#include "sl_component_catalog.h"
#include "ble_peer_manager_central.h"
#include "ble_peer_manager_central_internal.h"
#include "ble_peer_manager_central_config.h"
#include "ble_peer_manager_common.h"
#include "ble_peer_manager_common_config.h"
#include "ble_peer_manager_common_internal.h"
#include "ble_peer_manager_filter.h"
#include "ble_peer_manager_connections.h"

// -----------------------------------------------------------------------------
// Macros and Typedefs
typedef struct ble_peer_manager_scanner_s {
  sl_bt_gap_phy_coding_t scanning_phy;
  sl_bt_scanner_discover_mode_t scan_discovery_mode;
  sl_bt_scanner_scan_mode_t scan_mode;
  uint8_t scan_interval;
  uint8_t scan_window;
} ble_peer_manager_scanner_t;

// -----------------------------------------------------------------------------
// Forward declaration of private functions
static void on_connection_timeout(app_timer_t *timer, void *data);
static sl_status_t process_scan_response(bd_addr *address,
                                         sl_bt_gap_address_type_t address_type,
                                         int8_t rssi,
                                         const uint8array *adv_data);

// -----------------------------------------------------------------------------
// Static variables
static ble_peer_manager_scanner_t scanner;
static uint8_t central_active_conn_handle = SL_BT_INVALID_CONNECTION_HANDLE;
static bool scanning = false;
static app_timer_t timer;

// -----------------------------------------------------------------------------
// Public functions
void ble_peer_manager_central_init(void)
{
  // Set initial state
  (void)app_timer_stop(&timer);
  if (scanning) {
    (void)sl_bt_scanner_stop();
  }
  central_active_conn_handle = SL_BT_INVALID_CONNECTION_HANDLE;
  scanning = false;
  scanner.scanning_phy = BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCANNING_PHY;
  scanner.scan_discovery_mode = BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_DISCOVERY_MODE;
  scanner.scan_mode = BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_MODE;
  scanner.scan_interval = BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_INTERVAL;
  scanner.scan_window = BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_WINDOW;
  (void)ble_peer_manager_reset_filter();
  ble_peer_manager_clear_all_connections();
}

void ble_peer_manager_central_on_bt_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  ble_peer_manager_evt_type_t peer_evt;

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id:
    {
      ble_peer_manager_central_init();
      break;
    }
    case sl_bt_evt_connection_opened_id:
      // Handle the requested central connection handle only
      if ((evt->data.evt_connection_opened.connection != central_active_conn_handle)
          || (evt->data.evt_connection_opened.role != sl_bt_connection_role_central)) {
        break;
      }
      // Remove active connecting handle
      central_active_conn_handle = SL_BT_INVALID_CONNECTION_HANDLE;
      // Stop the timer of the connection establishment
      (void)app_timer_stop(&timer);
      if (scanning) {
        // Try to stop scanning
        sc = sl_bt_scanner_stop();
        if (sc != SL_STATUS_OK) {
          ble_peer_manager_log_error("Failed to stop scanner: 0x%04lx" APP_LOG_NL,
                                     (unsigned long)sc);
          // Emit an error event
          peer_evt.evt_id = BLE_PEER_MANAGER_ERROR;
          peer_evt.connection_id = SL_BT_INVALID_CONNECTION_HANDLE;
          ble_peer_manager_on_event(&peer_evt);
        } else {
          scanning = false;
        }
      }
      // Add connection to the database
      sc = ble_peer_manager_add_connection(evt->data.evt_connection_opened.connection,
                                           evt->data.evt_connection_opened.address,
                                           sl_bt_connection_role_central);
      if (sc != SL_STATUS_OK) {
        ble_peer_manager_log_error("Failed to add connection: 0x%04lx" APP_LOG_NL,
                                   (unsigned long)sc);
        // Emit an error event
        peer_evt.evt_id = BLE_PEER_MANAGER_ERROR;
        peer_evt.connection_id = SL_BT_INVALID_CONNECTION_HANDLE;
        ble_peer_manager_on_event(&peer_evt);
      } else {
        ble_peer_manager_log_info("Connection opened, device is central" APP_LOG_NL);
      }
      // Emit the connection opened event
      peer_evt.evt_id = BLE_PEER_MANAGER_ON_CONN_OPENED_CENTRAL;
      peer_evt.connection_id = evt->data.evt_connection_opened.connection;
      ble_peer_manager_on_event(&peer_evt);
      break;

    case sl_bt_evt_connection_closed_id:
      // The active connecting handle is closed
      if (evt->data.evt_connection_closed.connection == central_active_conn_handle) {
        ble_peer_manager_log_info("Central active connection closed, reason 0x%04lx." APP_LOG_NL,
                                  (unsigned long)evt->data.evt_connection_closed.reason);
        // Stop the timer of the connection establishment
        (void)app_timer_stop(&timer);
        // Remove active connecting handle
        central_active_conn_handle = SL_BT_INVALID_CONNECTION_HANDLE;
        // Since the connection is not reported earlier there is no need to send
        // any events
        break;
      }
      // Not the active connecting handle is closed
      // Check if it should be handled (it is in the database and central)
      if (!ble_peer_manager_is_conn_handle_in_array(evt->data.evt_connection_closed.connection)
          || !ble_peer_manager_is_conn_handle_central(evt->data.evt_connection_closed.connection)) {
        break;
      }
      ble_peer_manager_log_info("Central device connection closed, reason 0x%04lx." APP_LOG_NL,
                                (unsigned long)evt->data.evt_connection_closed.reason);
      // Delete the connection from the database
      sc = ble_peer_manager_delete_connection(evt->data.evt_connection_closed.connection);
      if (sc != SL_STATUS_OK) {
        // Emit an error event
        peer_evt.evt_id = BLE_PEER_MANAGER_ERROR;
        peer_evt.connection_id = SL_BT_INVALID_CONNECTION_HANDLE;
        ble_peer_manager_on_event(&peer_evt);
      }
      // Emit the connection closed event
      peer_evt.evt_id = BLE_PEER_MANAGER_ON_CONN_CLOSED;
      peer_evt.connection_id = evt->data.evt_connection_closed.connection;
      ble_peer_manager_on_event(&peer_evt);

      break;

    case sl_bt_evt_scanner_legacy_advertisement_report_id:
      // Check the scan response only if requested
      if (!scanning
          || (central_active_conn_handle != SL_BT_INVALID_CONNECTION_HANDLE)) {
        break;
      }
      // Process scan response and open connection if a match is found
      sc = process_scan_response(&(evt->data.evt_scanner_legacy_advertisement_report.address),
                                 evt->data.evt_scanner_legacy_advertisement_report.address_type,
                                 evt->data.evt_scanner_legacy_advertisement_report.rssi,
                                 &(evt->data.evt_scanner_legacy_advertisement_report.data));
      if (sc != SL_STATUS_OK
          && sc != SL_STATUS_NOT_INITIALIZED
          && sc != SL_STATUS_NOT_FOUND
          && sc != SL_STATUS_ALREADY_EXISTS) {
        // Emit an error event
        peer_evt.evt_id = BLE_PEER_MANAGER_ERROR;
        peer_evt.connection_id = SL_BT_INVALID_CONNECTION_HANDLE;
        ble_peer_manager_on_event(&peer_evt);
      }
      break;

    case sl_bt_evt_scanner_extended_advertisement_report_id:
      // Check the scan response only if requested
      if (!scanning
          || (central_active_conn_handle != SL_BT_INVALID_CONNECTION_HANDLE)) {
        break;
      }
      // Process scan response and open connection if a match is found
      sc = process_scan_response(&(evt->data.evt_scanner_extended_advertisement_report.address),
                                 evt->data.evt_scanner_extended_advertisement_report.address_type,
                                 evt->data.evt_scanner_extended_advertisement_report.rssi,
                                 &(evt->data.evt_scanner_extended_advertisement_report.data));
      if (sc != SL_STATUS_OK
          && sc != SL_STATUS_NOT_INITIALIZED
          && sc != SL_STATUS_NOT_FOUND
          && sc != SL_STATUS_ALREADY_EXISTS) {
        // Emit an error event
        peer_evt.evt_id = BLE_PEER_MANAGER_ERROR;
        peer_evt.connection_id = SL_BT_INVALID_CONNECTION_HANDLE;
        ble_peer_manager_on_event(&peer_evt);
      }
      break;
  }
}

sl_status_t ble_peer_manager_central_set_scanner(sl_bt_gap_phy_coding_t scanning_phy,
                                                 sl_bt_scanner_discover_mode_t scan_discovery_mode,
                                                 sl_bt_scanner_scan_mode_t scan_mode,
                                                 uint8_t scan_interval,
                                                 uint8_t scan_window)
{
  if (scanning) {
    ble_peer_manager_log_info("Already scanning. \
                              New settings will take effect the next time scanning is started." APP_LOG_NL);
  }
  scanner.scanning_phy = scanning_phy;
  scanner.scan_discovery_mode = scan_discovery_mode;
  scanner.scan_mode = scan_mode;
  scanner.scan_interval = scan_interval;
  scanner.scan_window = scan_window;
  return SL_STATUS_OK;
}

sl_status_t ble_peer_manager_central_create_connection(void)
{
  sl_status_t sc;
  uint8_t conn_count = ble_peer_manager_get_active_conn_number();

  if (!ble_peer_manager_is_filter_set()) {
    return SL_STATUS_INVALID_STATE;
  }

  if (scanning) {
    ble_peer_manager_log_info("Already scanning" APP_LOG_NL);
    return SL_STATUS_OK;
  }

  // Check for active connecting handle
  if (central_active_conn_handle != SL_BT_INVALID_CONNECTION_HANDLE) {
    ble_peer_manager_log_error("Connection establishment already in progress" APP_LOG_NL);
    return SL_STATUS_INVALID_STATE;
  }

  // Check if we reached the maximum allowed connection count
  if (conn_count >= BLE_PEER_MANAGER_COMMON_MAX_ALLOWED_CONN_COUNT) {
    ble_peer_manager_log_error("Max allowed connection count reached, cannot open new connection" APP_LOG_NL);
    return SL_STATUS_WOULD_OVERFLOW;
  }

  // Set scanner parameters
  sc = sl_bt_scanner_set_parameters(scanner.scan_mode,
                                    scanner.scan_interval,
                                    scanner.scan_window);
  if (sc != SL_STATUS_OK) {
    ble_peer_manager_log_error("Failed to set scanner parameters..." APP_LOG_NL);
    return sc;
  }
  // Start scanning, based on filtering it will open connection as central
  sc = sl_bt_scanner_start(scanner.scanning_phy, sl_bt_scanner_discover_generic);
  ble_peer_manager_log_info("Started scanning..." APP_LOG_NL);
  if (sc == SL_STATUS_OK) {
    // Register scanning state
    scanning = true;
  }
  return sc;
}

sl_status_t ble_peer_manager_central_open_connection(bd_addr *address, uint8_t address_type)
{
  sl_status_t sc;
  uint8_t new_connection_handle;
  uint8_t conn_count = ble_peer_manager_get_active_conn_number();

  if (address == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // Check for active connecting handle
  if (central_active_conn_handle != SL_BT_INVALID_CONNECTION_HANDLE) {
    ble_peer_manager_log_error("Connection establishment already in progress" APP_LOG_NL);
    return SL_STATUS_INVALID_STATE;
  }

  // Check if we reached the maximum allowed connection count
  if (conn_count >= BLE_PEER_MANAGER_COMMON_MAX_ALLOWED_CONN_COUNT) {
    ble_peer_manager_log_error("Max allowed connection count reached, cannot open new connection" APP_LOG_NL);
    return SL_STATUS_WOULD_OVERFLOW;
  }
  // Start the timer of the connection establishment
  sc = app_timer_start(&timer,
                       BLE_PEER_MANAGER_COMMON_TIMEOUT_GATT_MS,
                       on_connection_timeout,
                       NULL,
                       false);
  if (sc != SL_STATUS_OK) {
    return sc;
  }
  // Start connection establishment
  sc = sl_bt_connection_open(*address,
                             address_type,
                             scanner.scanning_phy,
                             &new_connection_handle);
  if (sc != SL_STATUS_OK) {
    (void)app_timer_stop(&timer);
    return sc;
  }
  // Register active connecting handle
  central_active_conn_handle = new_connection_handle;
  return sc;
}

sl_status_t ble_peer_manager_central_close_connection(uint8_t conn_handle)
{
  sl_status_t sc = SL_STATUS_OK;
  if (conn_handle == SL_BT_INVALID_CONNECTION_HANDLE) {
    return SL_STATUS_INVALID_HANDLE;
  }
  // Check if the handle is in the central connection database
  if (!ble_peer_manager_is_conn_handle_in_array(conn_handle)
      || !ble_peer_manager_is_conn_handle_central(conn_handle)) {
    ble_peer_manager_log_error("Connection handle not found." APP_LOG_NL);
    return SL_STATUS_NOT_FOUND;
  }
  // Handle is in the central connection database
  ble_peer_manager_log_info("Closing connection." APP_LOG_NL);
  // Start connection close, an event will notify the result
  sc = sl_bt_connection_close(conn_handle);
  return sc;
}

bool ble_peer_manager_is_filter_set_allowed(void)
{
  return !scanning;
}

// -----------------------------------------------------------------------------
// Private functions

void on_connection_timeout(app_timer_t *timer, void *data)
{
  (void)timer;
  (void)data;
  sl_status_t sc;
  ble_peer_manager_evt_type_t peer_evt;
  // Check for validity of requested handle
  if (central_active_conn_handle != SL_BT_INVALID_CONNECTION_HANDLE) {
    // Close connection
    ble_peer_manager_log_info("Connection timeout. Closing connection." APP_LOG_NL);
    sc = sl_bt_connection_close(central_active_conn_handle);
    if (sc == SL_STATUS_OK) {
      return;
    } else {
      ble_peer_manager_log_error("Failed to close connection in on_connection_timeout" APP_LOG_NL);
    }
  } else {
    ble_peer_manager_log_error("Invalid connection handle in on_connection_timeout" APP_LOG_NL);
  }
  // Emit an error event
  peer_evt.evt_id = BLE_PEER_MANAGER_ERROR;
  peer_evt.connection_id = SL_BT_INVALID_CONNECTION_HANDLE;
  ble_peer_manager_on_event(&peer_evt);
}

// Process scan response
sl_status_t process_scan_response(bd_addr *address,
                                  sl_bt_gap_address_type_t address_type,
                                  int8_t rssi,
                                  const uint8array *adv_data)
{
  sl_status_t sc;

  if (!ble_peer_manager_is_filter_set()) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  bool match = ble_peer_manager_find_match(address, address_type, rssi, adv_data);
  if (!match) {
    return SL_STATUS_NOT_FOUND;
  }

  // Check if we are already connected to the device
  if (ble_peer_manager_is_bt_address_already_connected(address)) {
    ble_peer_manager_log_debug("Device already connected, won't open new connection" APP_LOG_NL);
    return SL_STATUS_ALREADY_EXISTS;
  }

  // If there was a match or there is no filtering active,
  // open connection as central
  ble_peer_manager_log_info("Opening connection as central to '%02x:%02x:%02x:%02x:%02x:%02x'" APP_LOG_NL,
                            address->addr[5],
                            address->addr[4],
                            address->addr[3],
                            address->addr[2],
                            address->addr[1],
                            address->addr[0]);
  sc = ble_peer_manager_central_open_connection(address, address_type);
  return sc;
}
