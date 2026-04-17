/***************************************************************************//**
 * @file
 * @brief Core application logic.
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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sl_bt_api.h"
#include "gatt_db.h"
#include "app_assert.h"
#include "app_log.h"
#include "app_memlcd.h"
#include "app_iop.h"
#include "app.h"
#include "sl_main_init.h"

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = SL_BT_INVALID_ADVERTISING_SET_HANDLE;

// Security
static uint32_t bonding_handle = SL_BT_INVALID_BONDING_HANDLE;
static bool increase_security = false;

// Readable strings for sl_bt_connection_security_t
static const char *connection_security_str[] = {
  "(0x00) No security",
  "(0x01) Unauthenticated pairing",
  "(0x02) Authenticated pairing",
  "(0x03) Bonding"
};

/***************************************************************************//**
 * Application Init.
 ******************************************************************************/
void app_init(void)
{
  app_test_data_init();
  app_memlcd_init();
}

/***************************************************************************//**
 * Application Process Action.
 ******************************************************************************/
void app_process_action(void)
{
  if (app_is_process_required()) {
    app_throughput_step();
  }
}

/***************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the default weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 ******************************************************************************/
void sl_bt_on_event(sl_bt_msg_t* evt)
{
  sl_status_t sc;
  switch (SL_BT_MSG_ID(evt->header)) {
    // This boot event is generated when the system boots up after reset.
    // Do not call any stack commands before receiving the boot event.
    case sl_bt_evt_system_boot_id: {
      // Log device name so firmware upgrades can be checked right after boot
      size_t name_len;
      uint8_t name[MAX_CHARS_PER_LINE + 1];
      sc = sl_bt_gatt_server_read_attribute_value(gattdb_device_name,
                                                  0,
                                                  sizeof(name) - 1,
                                                  &name_len,
                                                  name);
      app_assert_status(sc);
      name[name_len] = '\0'; // Add null character
      app_log_info("Device name: %s" APP_LOG_NL, name);

      // Write BLE stack version into GATT. The first 8 bytes of sl_bt_evt_system_boot_s
      // structure corresponds to major, minor, patch and build versions.
      sc = sl_bt_gatt_server_write_attribute_value(gattdb_iop_test_stack_version,
                                                   0,
                                                   8,
                                                   (uint8_t *)&evt->data.evt_system_boot);
      app_assert_status(sc);
      app_log_info("Bluetooth stack booted: v%d.%d.%d+%08lx" APP_LOG_NL,
                   evt->data.evt_system_boot.major,
                   evt->data.evt_system_boot.minor,
                   evt->data.evt_system_boot.patch,
                   evt->data.evt_system_boot.hash);

      // Read address.
      sc = sl_bt_gap_get_identity_address(&public_address, 0);
      app_assert_status(sc);

      app_log_info("Public device address: ");
      for (int i = 0; i < 5; i++) {
        app_log_append("%2.2x:", public_address.addr[5 - i]);
      }
      app_log_append("%2.2x." APP_LOG_NL, public_address.addr[0]);

      set_display(DISPLAY_STATE_IDLE, NULL);

      // Set default connection parameters
      sc = sl_bt_connection_set_default_parameters(DEFAULT_CONNECTION_INTERVAL,
                                                   DEFAULT_CONNECTION_INTERVAL,
                                                   DEFAULT_RESPONDER_LATENCY,
                                                   DEFAULT_SUPV_TIMEOUT,
                                                   DEFAULT_CE_LENGTH_MIN,
                                                   DEFAULT_CE_LENGTH_MAX);
      app_log_status_error(sc);

      sc = sl_bt_connection_set_default_preferred_phy(DEFAULT_PHY,
                                                      0xff); // Any PHYs
      app_log_status_error(sc);

      sc = sl_bt_connection_set_default_data_length(DEFAULT_PDU);
      app_log_status_error(sc);

      // Delete bondings.
      sc = sl_bt_sm_delete_bondings();
      app_log_status_error(sc);

      if (sc == SL_STATUS_OK) {
        app_log_info("Bondings deleted." APP_LOG_NL);
      }

      // Set Security Manager to debug mode, so the encrypted BLE packets can be
      // read in Network Analyzer.
      sc = sl_bt_sm_set_debug_mode();
      app_log_status_error(sc);

      // Advertising
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert_status(sc);

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_log_status_error(sc);

      // Default advertisement parameters: 32 (20 ms) interval. Other optional
      // parameters for phones: 244 (152.5 ms), 338 (211.25 ms) 160 (100 ms).
      sc = sl_bt_advertiser_set_timing(advertising_set_handle,
                                       DEFAULT_ADVERTISING_INTERVAL,
                                       DEFAULT_ADVERTISING_INTERVAL,
                                       0,   // adv. duration
                                       0);  // max. num. adv. events
      app_log_status_error(sc);

      // Start advertising and enable connections.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_legacy_advertiser_connectable);
      app_assert_status(sc);

      if (sc == SL_STATUS_OK) {
        app_log_info("Advertising started." APP_LOG_NL);
      }

      // Test 1 (Scan device) takes place now.
      // Test 2 (Connect to device) takes place now.
      break;
    }

    //--------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id: {
      uint8_t connection_handle = evt->data.evt_connection_opened.connection;
      app_log_info("Connection opened. Handle: [%d]. "
                   "Address: [%02X:%02X:%02X:%02X:%02X:%02X]. Address type: [%d]. "
                   "Bonded: [%d]" APP_LOG_NL,
                   connection_handle,
                   evt->data.evt_connection_opened.address.addr[5],
                   evt->data.evt_connection_opened.address.addr[4],
                   evt->data.evt_connection_opened.address.addr[3],
                   evt->data.evt_connection_opened.address.addr[2],
                   evt->data.evt_connection_opened.address.addr[1],
                   evt->data.evt_connection_opened.address.addr[0],
                   evt->data.evt_connection_opened.address_type,
                   evt->data.evt_connection_opened.bonding != SL_BT_INVALID_BONDING_HANDLE);

      set_display(DISPLAY_STATE_CONNECTED, NULL);

      // Test 3 (MA discovers the GATT) takes place now.

      // Increase the security of the connection if requested by the tester.
      if (increase_security) {
        sl_bt_sm_increase_security(connection_handle);
      }
      break;
    }

    //--------------------------------
    // This event indicates there is a request to display the passkey to the user.
    case sl_bt_evt_sm_passkey_display_id: {
      passkey = evt->data.evt_sm_passkey_display.passkey; // Store passkey.
      app_log_info("Passkey: %06lu." APP_LOG_NL, passkey); // Make it appear on Console.
      set_display(DISPLAY_STATE_PASSKEY, &passkey); // Make it appear on LCD.
      // Note: It is also displayed in the Simplicity Connect app.
      break;
    }

    // Triggered after the pairing or bonding procedure is successfully completed.
    case sl_bt_evt_sm_bonded_id: {
      app_log_info("Security increased to [%s]" APP_LOG_NL,
                   connection_security_str[evt->data.evt_sm_bonded.security_mode]);

      if (evt->data.evt_sm_bonded.security_mode == sl_bt_connection_mode1_level4) {
        if (evt->data.evt_sm_bonded.bonding != SL_BT_INVALID_BONDING_HANDLE) {
          bonding_handle = evt->data.evt_sm_bonded.bonding;
          app_log_info("Bonding handle: %lu" APP_LOG_NL, bonding_handle);
          set_display(DISPLAY_STATE_BONDING, NULL);
        } else {
          app_log_error("Invalid bonding handle." APP_LOG_NL);
        }
      }
      break;
    }

    // This event is triggered if the pairing or bonding procedure fails.
    case sl_bt_evt_sm_bonding_failed_id: {
      app_log_error("Pairing/Bonding failed. Reason: [0x%04x]." APP_LOG_NL,
                    evt->data.evt_sm_bonding_failed.reason);
      // Closing and reopening connection to retry.
      sc = sl_bt_connection_close(evt->data.evt_sm_bonding_failed.connection);
      app_log_status_error(sc);
      break;
    }

    //--------------------------------
    // Triggered whenever the connection parameters are changed and at any time a connection is established
    case sl_bt_evt_connection_parameters_id: {
      connection_interval = evt->data.evt_connection_parameters.interval;
      responder_latency = evt->data.evt_connection_parameters.latency;
      supv_timeout = (uint16_t)((evt->data.evt_connection_parameters.timeout));

      uint8_t interval_whole = (uint8_t)(connection_interval * 1.25f);
      uint8_t interval_partial = (uint8_t)(100 * (connection_interval * 1.25f)) - 100 * interval_whole;

      app_log_debug("Connection parameters: Connection Interval = %u.%2u[ms], "
                    "Responder Latency = %u, Supervision Timeout = %u[ms] "
                    "Security = [%s]" APP_LOG_NL,
                    interval_whole,
                    interval_partial,
                    responder_latency,
                    supv_timeout * 10,
                    connection_security_str[evt->data.evt_connection_parameters.security_mode]);
      break;
    }

    //--------------------------------
    // Reports a change to the maximum payload length or maximum TX time in
    // either direction of a connection.
    case sl_bt_evt_connection_data_length_id: {
      pdu_size = evt->data.evt_connection_data_length.tx_data_len;
      app_log_debug("Connection parameters are changed: PDU = %u." APP_LOG_NL, pdu_size);
      break;
    }

    //--------------------------------
    // Indicates that an ATT_MTU exchange procedure is completed.
    case sl_bt_evt_gatt_mtu_exchanged_id: {
      mtu_size = evt->data.evt_gatt_mtu_exchanged.mtu;
      app_log_debug("Connection parameters are changed: MTU = %u." APP_LOG_NL, mtu_size);
      break;
    }

    //--------------------------------
    // Indicates that PHY update procedure is completed.
    case sl_bt_evt_connection_phy_status_id: {
      phy = evt->data.evt_connection_phy_status.phy;
      app_log_debug("Connection parameters are changed: PHY = %u." APP_LOG_NL, phy);
      break;
    }

    //--------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id: {
      app_log_info("Connection closed. Handle: [%d]." APP_LOG_NL,
                   evt->data.evt_connection_closed.connection);
      set_display(DISPLAY_STATE_IDLE, NULL);

      // Configure security manager for the next connection.
      switch (security_level) {
        // Unauthenticated pairing with encryption
        case SECURITY_LEVEL_PAIRING: {
          // Preparing for test 7.2 (Security/Pairing).
          increase_security = true;
          sc = sl_bt_sm_delete_bondings();
          app_log_status_error(sc);

          if (sc == SL_STATUS_OK) {
            app_log_info("Bondings deleted. Preparing for security level: [%s]." APP_LOG_NL,
                         connection_security_str[(sl_bt_connection_security_t)security_level]);
          }

          sc = sl_bt_sm_configure(BONDING_WITHOUT_MITM, sl_bt_sm_io_capability_noinputnooutput);
          app_log_status_error_f(sc, "SM configure failure." APP_LOG_NL);

          sc = sl_bt_sm_set_bondable_mode(BONDING_DISABLED);
          app_log_status_error_f(sc, "SM set bondable mode failure." APP_LOG_NL);
          break;
        }

        // Authenticated pairing with encryption
        case SECURITY_LEVEL_AUTHENTICATION: {
          // Preparing for test 7.3 (Security/Authentication).
          increase_security = true;
          sc = sl_bt_sm_delete_bondings();
          app_log_status_error(sc);

          if (sc == SL_STATUS_OK) {
            app_log_info("Bondings deleted. Preparing for security level: [%s]." APP_LOG_NL,
                         connection_security_str[(sl_bt_connection_security_t)security_level]);
          }

          sc = sl_bt_sm_configure(BONDING_WITH_MITM, sl_bt_sm_io_capability_displayonly);
          app_log_status_error_f(sc, "SM configure failure." APP_LOG_NL);

          sc = sl_bt_sm_set_passkey(passkey);
          app_log_status_error_f(sc, "SM set passkey failure." APP_LOG_NL);

          sc = sl_bt_sm_set_bondable_mode(BONDING_DISABLED);
          app_log_status_error_f(sc, "SM set bondable mode failure." APP_LOG_NL);
          break;
        }

        // Authenticated Secure Connections pairing with encryption using a
        // 128-bit strength encryption key
        case SECURITY_LEVEL_BONDING: {
          // Preparing for test 7.4 (Security/Bonding).
          increase_security = true;
          sc = sl_bt_sm_delete_bondings();
          app_log_status_error(sc);

          if (sc == SL_STATUS_OK) {
            app_log_info("Bondings deleted. Preparing for security level: [%s]." APP_LOG_NL,
                         connection_security_str[(sl_bt_connection_security_t)security_level]);
          }

          sc = sl_bt_sm_configure(BONDING_WITH_MITM, sl_bt_sm_io_capability_displayonly);
          app_log_status_error_f(sc, "SM configure failure." APP_LOG_NL);

          sc = sl_bt_sm_set_passkey(passkey);
          app_log_status_error_f(sc, "SM set passkey failure." APP_LOG_NL);

          sc = sl_bt_sm_set_bondable_mode(BONDING_ENABLED);
          app_log_status_error_f(sc, "SM set bondable mode failure." APP_LOG_NL);
          break;
        }

        // LE Privacy 1.2 test based on an existing bonding
        case SECURITY_LEVEL_PRIVACY: {
          sc = sl_bt_resolving_list_add_device_by_bonding(bonding_handle, sl_bt_resolving_list_privacy_mode_network);
          app_log_status_error_f(sc, "Failed to add bonding handle %ld to the resolving list." APP_LOG_NL, bonding_handle);

          sc = sl_bt_accept_list_add_device_by_bonding(bonding_handle);
          app_log_status_error_f(sc, "Failed to add bonding handle %ld to the accept list." APP_LOG_NL, bonding_handle);

          sc = sl_bt_advertiser_configure(advertising_set_handle, SL_BT_ADVERTISER_USE_FILTER_FOR_CONNECTION_REQUESTS);
          app_log_status_error_f(sc, "Failed to configure advertising set." APP_LOG_NL);
          break;
        }

        default: {
          break;
        }
      }
      // Evaluate security request only once.
      security_level = SECURITY_LEVEL_NONE;

      // Restart advertising.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_legacy_advertiser_connectable);
      app_log_status_error(sc);

      if (sc == SL_STATUS_OK) {
        app_log_info("Advertising started." APP_LOG_NL);
      }

      break;
    }

    //--------------------------------
    // This event indicates that a remote GATT is attempting to write a
    // value of an attribute into the local GATT database.
    // Test 4 and 5 (MA performs all GATT) partly takes place here.
    case sl_bt_evt_gatt_server_user_write_request_id: {
      // Test 4.3 (iop_test_writeonly_len_1) takes place now.
      // Test 4.4 (iop_test_writeonly_len_255) takes place now.
      // Test 4.5 (iop_test_writewoutresp_len_1) takes place now.
      // Test 4.6 (iop_test_writewoutresp_len_255) takes place now.
      // Test 5.1.1 (iop_test_len_1) takes place now.
      // Test 5.2.1 (iop_test_len_255) takes place now.
      // Test 5.3.1.1 (iop_test_len_variable_4) takes place now.
      // Test 5.3.2.1 (iop_test_len_variable_4) takes place now.
      // Test 5.4.2 (iop_test_const_len_1) takes place now.
      // Test 5.5.2 (iop_test_const_len_255) takes place now.

      sc = handle_user_write(&(evt->data.evt_gatt_server_user_write_request));
      app_log_status_error(sc);
      break;
    }

    //--------------------------------
    // This event indicates that a remote GATT is attempting to read a
    // value of an attribute from the local GATT database.
    // Test 4 and 5 (MA performs all GATT) partly takes place here.
    case sl_bt_evt_gatt_server_user_read_request_id: {
      // Test 4.1 (iop_test_readonly_len_1) takes place now.
      // Test 4.2 (iop_test_readonly_len_255) takes place now.
      // Test 5.1.2 (iop_test_len_1) takes place now.
      // Test 5.2.2 (iop_test_len_255) takes place now.
      // Test 5.3.1.2 (iop_test_len_variable_4) takes place now.
      // Test 5.3.2.2 (iop_test_len_variable_4) takes place now.
      // Test 5.4.1 (iop_test_const_len_1) takes place now.
      // Test 5.5.1 (iop_test_const_len_255) takes place now.
      // Test 7.2 (Security/Pairing) takes place now.
      // Test 7.3 (Security/Authentication) takes place now.
      // Test 7.4 (Security/Bonding) takes place now.

      sc = handle_user_read(&(evt->data.evt_gatt_server_user_read_request));
      app_log_status_error(sc);
      break;
    }

    // Execute write completed event indicates that the execute write command
    // from a remote GATT has completed with the given result.
    case sl_bt_evt_gatt_server_execute_write_completed_id: {
      // End of Test 5.7.1.
      app_log_info("GATT server execute write completed." APP_LOG_NL);
      break;
    }

    //--------------------------------
    // This event occurs when the remote GATT enabled or disabled a
    // notification or indication.
    case sl_bt_evt_gatt_server_characteristic_status_id: {
      if (evt->data.evt_gatt_server_characteristic_status.status_flags == sl_bt_gatt_server_client_config) {
        sc = handle_timer_start(&(evt->data.evt_gatt_server_characteristic_status));
        app_log_status_error(sc);
      }
      break;
    }

    default: {
      break;
    }
  }
}
