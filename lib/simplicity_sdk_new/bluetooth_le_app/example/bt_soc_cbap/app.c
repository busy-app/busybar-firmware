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
#include <stdio.h>
#include <stdbool.h>
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app_assert.h"
#include "app_log.h"
#include "app_timer.h"
#include "sl_simple_led_instances.h"
#include "sl_bt_cbap.h"
#include "cbap_config.h"
#include "app.h"
#include "sl_main_init.h"

#if SL_BT_CONFIG_MAX_CONNECTIONS < 1
  #error At least 1 connection has to be enabled!
#endif

// -----------------------------------------------------------------------------
// GATT

// Reference to the CBAP service.
static uint32_t cbap_service_handle = HANDLE_NOT_INITIALIZED;
static const uint8_t cbap_service_uuid[] = { CBAP_SERVICE_UUID };

// Reference to the CBAP characteristics.
static characteristic_128_ref_t cbap_characteristics[] = {
  {
    .handle = HANDLE_NOT_INITIALIZED,
    .uuid = { CENTRAL_CERT_CHAR_UUID }
  },
  {
    .handle = HANDLE_NOT_INITIALIZED,
    .uuid = { PERIPHERAL_CERT_CHAR_UUID }
  },
  {
    .handle = HANDLE_NOT_INITIALIZED,
    .uuid = { CENTRAL_OOB_CHAR_UUID }
  },
  {
    .handle = HANDLE_NOT_INITIALIZED,
    .uuid = { PERIPHERAL_OOB_CHAR_UUID }
  }
};

// Device role
static sl_bt_connection_role_t role = ROLE;

// -----------------------------------------------------------------------------
// States

// State of the peripheral device
static peripheral_state_t peripheral_state = (peripheral_state_t)0;
// State of the central device
static central_state_t central_state = (central_state_t)0;
// Pointing to the characteristic that shall be discovered next
static characteristics_t char_state = (characteristics_t)0;

#define IS_PERIPHERAL_IN_PROGRESS (peripheral_state != (peripheral_state_t)0)
#define IS_CENTRAL_IN_PROGRESS    (central_state != (central_state_t)0)

// -----------------------------------------------------------------------------
// Connections

// The connection handle and the Bluetooth address of the remote device we have CBAP in progress with
static conn_properties_t candidate_device;
// Array for holding properties of the trusted connections
static conn_properties_t trusted_devices[SL_BT_CONFIG_MAX_CONNECTIONS];

// Clears candidate device.
static void clear_connection_info(void);
// Adds the candidate device to the trusted devices array.
static void save_connection_info(void);
// Finds next available connection slot.
static int next_available_connection(void);
// Logs the connection handle and the Bluetooth address of the trusted devices.
static void print_trusted_devices(void);

// -----------------------------------------------------------------------------
// Advertising

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = SL_BT_INVALID_ADVERTISING_SET_HANDLE;

// -----------------------------------------------------------------------------
// Scanning

// Should we search for a specified peripheral device or not
static bool peripheral_target_defined = ADDR_ENABLE;
// Target device Bluetooth address
static bd_addr peripheral_target_addr;

// Convert address string to address data bytes.
static bool decode_address(char *addess_str, bd_addr *address);
// Examine a scan report and decide if a connection should be established.
bool check_scan_report(sl_bt_evt_scanner_legacy_advertisement_report_t *scan_report);
// Search for a Service UUID in scan report.
static bool find_service_in_advertisement(const uint8_t *scan_data,
                                          uint8_t scan_data_len,
                                          const uint8_t *uuid,
                                          uint8_t uuid_len);

// -----------------------------------------------------------------------------
// Certificates

// Device certificate in DER format
static uint8_t device_certificate_der[CHAIN_LINK_DATA_LEN * CHAIN_LINK_DATA_NUM] = { 0 };
static uint32_t device_certificate_der_len = 0;
static uint32_t dev_cert_sending_progression = 0;
static bool device_cert_sent = false;

// Remote certificate which was sent over GATT in DER format
static uint8_t remote_certificate_der[CHAIN_LINK_DATA_LEN * CHAIN_LINK_DATA_NUM] = { 0 };
static uint32_t remote_certificate_der_len = 0;
static bool remote_cert_arrived = false;

// Out-Of-Band data
static uint8_t signed_device_oob_data[SIGNED_OOB_DATA_LEN];
static size_t signed_device_oob_len = 0;

// -----------------------------------------------------------------------------
// Timers
#define STATE_TIMEOUT                 5000 // ms
#define LED_TIMEOUT                   500 // ms
#define CALLBACK_DATA                 (void *)NULL // Callback has no parameters

// Timer handles
static app_timer_t state_timer;
static app_timer_t led_timer;

// Timer Callbacks
static void state_machine_timeout_cb(app_timer_t *handle, void *data);
static void led_timer_cb(app_timer_t *handle, void *data);

// Start timer for state machine timeout checking
#define STATE_TIMEOUT_START()                                                                                            \
  do {                                                                                                                   \
    sl_status_t timeout_start_status;                                                                                    \
    timeout_start_status = app_timer_start(&state_timer, STATE_TIMEOUT, state_machine_timeout_cb, CALLBACK_DATA, false); \
    app_assert_status(timeout_start_status);                                                                             \
  } while (0)
// Stop timer for state machine timeout checking
#define STATE_TIMEOUT_STOP()                            \
  do {                                                  \
    sl_status_t timeout_stop_status;                    \
    timeout_stop_status = app_timer_stop(&state_timer); \
    app_assert_status(timeout_stop_status);             \
  } while (0)

// -----------------------------------------------------------------------------
// Error handling

// Handle CBAP process errors
static void on_error(void);
// Reset CBAP process states, flags and timers
static void app_reset(void);

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
void app_init(void)
{
  // Initialize CBAP component and dependencies
  sl_status_t sc;
  sc = sl_bt_cbap_init(device_certificate_der, &device_certificate_der_len);
  app_assert_status(sc);
  app_log_info("CBAP initialized. Device certificate verified." APP_LOG_NL);

  // Initialize candidate device data
  clear_connection_info();

  // Initialize connection array
  for (uint8_t i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
    trusted_devices[i].connection_handle = SL_BT_INVALID_CONNECTION_HANDLE;
    for (uint8_t j = 0; j < sizeof(bd_addr); j++) {
      trusted_devices[i].address.addr[j] = 0xFF;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the default weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      switch (role) {
        case sl_bt_connection_role_peripheral:
          app_log_info("Peripheral role selected." APP_LOG_NL);
          // Create an advertising set.
          sc = sl_bt_advertiser_create_set(&advertising_set_handle);
          app_assert_status(sc);

          // Generate data for advertising
          sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle, sl_bt_advertiser_general_discoverable);
          app_assert_status(sc);

          // Set advertising interval to 100ms.
          sc = sl_bt_advertiser_set_timing(
            advertising_set_handle,
            160, // min. adv. interval (milliseconds * 1.6)
            160, // max. adv. interval (milliseconds * 1.6)
            0,   // adv. duration
            0);  // max. num. adv. events
          app_assert_status(sc);

          // Request OOB data from both device
          sc = sl_bt_sm_configure(SL_BT_SM_CONFIGURATION_OOB_FROM_BOTH_DEVICES_REQUIRED,
                                  sl_bt_sm_io_capability_noinputnooutput);
          app_assert_status(sc);

          // Start advertising and enable connections.
          sc = sl_bt_legacy_advertiser_start(advertising_set_handle, sl_bt_legacy_advertiser_connectable);
          app_assert_status(sc);
          app_log_info("Advertising started." APP_LOG_NL);
          break;

        case sl_bt_connection_role_central:
          app_log_info("Central role selected." APP_LOG_NL);

          // If defined, get target address
          if (peripheral_target_defined) {
            if (decode_address(ADDR, &peripheral_target_addr)) {
              app_log_info("Searching for %02X:%02X:%02X:%02X:%02X:%02X. " APP_LOG_NL,
                           peripheral_target_addr.addr[5],
                           peripheral_target_addr.addr[4],
                           peripheral_target_addr.addr[3],
                           peripheral_target_addr.addr[2],
                           peripheral_target_addr.addr[1],
                           peripheral_target_addr.addr[0]);
            } else {
              peripheral_target_defined = false;
              app_log_error("Reading target address failed. Searching for any " \
                            "device advertising the CBAP Service." APP_LOG_NL);
            }
          } else {
            app_log_info("Searching for any device advertising the CBAP Service." APP_LOG_NL);
          }

          // Start scanning
          sc = sl_bt_scanner_start(sl_bt_scanner_scan_phy_1m, sl_bt_scanner_discover_generic);
          app_assert_status(sc);
          app_log_info("Scanning started." APP_LOG_NL);
          break;

        default:
          app_assert_status_f(SL_STATUS_INVALID_STATE, "Invalid role!");
          break;
      }
      break;

    // -------------------------------
    // This event is generated when an advertisement packet or a scan response is received from a responder
    case sl_bt_evt_scanner_legacy_advertisement_report_id:
      if (role == sl_bt_connection_role_central) {
        // Filter for connectable scannable undirected advertisements
        if ((evt->data.evt_scanner_legacy_advertisement_report.event_flags
             == (SL_BT_SCANNER_EVENT_FLAG_CONNECTABLE | SL_BT_SCANNER_EVENT_FLAG_SCANNABLE))
            && check_scan_report(&evt->data.evt_scanner_legacy_advertisement_report)) {
          // Target device found. Stop scanning.
          sc = sl_bt_scanner_stop();
          app_assert_status(sc);

          // Connect to device
          sc = sl_bt_connection_open(evt->data.evt_scanner_legacy_advertisement_report.address,
                                     evt->data.evt_scanner_legacy_advertisement_report.address_type,
                                     sl_bt_gap_phy_1m,
                                     NULL);
          app_assert_status(sc);
        }
      }
      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      app_log_info("Connection opened." APP_LOG_NL);

      // Store data of the candidate device
      candidate_device.connection_handle = evt->data.evt_connection_opened.connection;
      candidate_device.address = evt->data.evt_connection_opened.address;

      // Check if there is a connection with this device already
      for (int i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
        if (memcmp(evt->data.evt_connection_opened.address.addr, trusted_devices[i].address.addr, sizeof(bd_addr)) == 0) {
          on_error();
          break;
        }
      }

      if (IS_PERIPHERAL_IN_PROGRESS || IS_CENTRAL_IN_PROGRESS) {
        on_error();
        break;
      }

      if (evt->data.evt_connection_opened.bonding != SL_BT_INVALID_BONDING_HANDLE) {
        app_log_warning("Devices are already bonded." APP_LOG_NL);
      }

      if (role == sl_bt_connection_role_central) {
        // Discover CBAP service on the peripheral device
        sc = sl_bt_gatt_discover_primary_services_by_uuid(candidate_device.connection_handle,
                                                          sizeof(cbap_service_uuid),
                                                          (const uint8_t *)cbap_service_uuid);
        app_assert_status(sc);
        central_state = CENTRAL_DISCOVER_SERVICES;
        app_log_info("Discovering services." APP_LOG_NL);
      }

      STATE_TIMEOUT_START();
      app_log_info("CBAP procedure start." APP_LOG_NL);
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      app_log_info("Connection closed." APP_LOG_NL);

      // Remove connection from the connection array if present
      for (int i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
        if (trusted_devices[i].connection_handle == evt->data.evt_connection_closed.connection) {
          trusted_devices[i].connection_handle = SL_BT_INVALID_CONNECTION_HANDLE;
          for (uint8_t j = 0; j < sizeof(bd_addr); j++) {
            trusted_devices[i].address.addr[j] = 0xff;
          }
          app_log_info("Trusted device [%d] removed." APP_LOG_NL, evt->data.evt_connection_closed.connection);
        }
      }

      // Clean up if CBAP procedure was aborted
      if (IS_PERIPHERAL_IN_PROGRESS || IS_CENTRAL_IN_PROGRESS) {
        on_error();
      } else {
        app_reset();
      }
      break;

    //--------------------------------
    // Triggered whenever the connection parameters are changed
    case sl_bt_evt_connection_parameters_id:
      if (evt->data.evt_connection_parameters.connection != candidate_device.connection_handle) {
        break;
      }

      app_log_debug("Security mode: %d" APP_LOG_NL, evt->data.evt_connection_parameters.security_mode);

      if (role == sl_bt_connection_role_peripheral) {
        if (evt->data.evt_connection_parameters.security_mode > sl_bt_connection_mode1_level1
            && peripheral_state != PERIPHERAL_CENTRAL_OOB_OK) {
          app_log_error("The central device increased the security level with no CBAP." APP_LOG_NL);
          on_error();
          break;
        }

        if (evt->data.evt_connection_parameters.security_mode == sl_bt_connection_mode1_level4) {
          peripheral_state = PERIPHERAL_DONE;

          app_log_info("CBAP procedure complete." APP_LOG_NL);
          save_connection_info();
          clear_connection_info();
          print_trusted_devices();

          app_reset();
        }
      } else if (role == sl_bt_connection_role_central) {
        if (evt->data.evt_connection_parameters.security_mode > sl_bt_connection_mode1_level1
            && central_state != CENTRAL_INCREASE_SECURITY) {
          app_log_error("Security level has been increased with no CBAP." APP_LOG_NL);
          on_error();
          break;
        }

        if (evt->data.evt_connection_parameters.security_mode == sl_bt_connection_mode1_level4) {
          central_state = CENTRAL_DONE;
          app_log_info("CBAP procedure complete." APP_LOG_NL);

          // Blink LED on peripheral
          uint8_t data_sent = 0x01;
          sc = sl_bt_gatt_write_characteristic_value(candidate_device.connection_handle,
                                                     gattdb_aio_digital_out,
                                                     sizeof(data_sent),
                                                     &data_sent);
          app_assert_status(sc);

          save_connection_info();
          clear_connection_info();
          print_trusted_devices();

          app_reset();
        }
      }
      break;

    // -------------------------------
    // This event is generated when a new service is discovered
    case sl_bt_evt_gatt_service_id:
      if (evt->data.evt_gatt_characteristic.connection != candidate_device.connection_handle) {
        break;
      }

      if (role == sl_bt_connection_role_central) {
        if (cbap_service_handle == HANDLE_NOT_INITIALIZED) {
          // Save service handle for future reference
          cbap_service_handle = evt->data.evt_gatt_service.service;
          app_log_debug("Service handle found: %d" APP_LOG_NL, (int)cbap_service_handle);
        }
      }
      break;

    // -------------------------------
    // This event is generated when a new characteristic is discovered
    case sl_bt_evt_gatt_characteristic_id:
      if (evt->data.evt_gatt_characteristic.connection != candidate_device.connection_handle) {
        break;
      }

      if (role == sl_bt_connection_role_central) {
        if (cbap_characteristics[char_state].handle == HANDLE_NOT_INITIALIZED) {
          // Save characteristic handle for future reference
          cbap_characteristics[char_state].handle = evt->data.evt_gatt_characteristic.characteristic;
          app_log_debug("Characteristic handle found: %d" APP_LOG_NL, cbap_characteristics[char_state].handle);
        }
      }
      break;

    // -------------------------------
    // This event is generated when a characteristic value was received
    case sl_bt_evt_gatt_characteristic_value_id:
      if (evt->data.evt_gatt_characteristic.connection != candidate_device.connection_handle) {
        break;
      }

      if (role == sl_bt_connection_role_central) {
        if (central_state == CENTRAL_GET_PERIPHERAL_CERT) {
          memcpy(&remote_certificate_der[remote_certificate_der_len],
                 &evt->data.evt_gatt_characteristic_value.value.data[1],
                 evt->data.evt_gatt_characteristic_value.value.len - 1);
          remote_certificate_der_len += evt->data.evt_gatt_characteristic_value.value.len - 1;
          sc = sl_bt_gatt_send_characteristic_confirmation(evt->data.evt_gatt_characteristic_value.connection);
          app_assert_status(sc);
          if (evt->data.evt_gatt_characteristic_value.value.data[0] == 0) {
            // Last chunk stop indication
            sc = sl_bt_gatt_set_characteristic_notification(candidate_device.connection_handle,
                                                            cbap_characteristics[CHAR_PERIPHERAL_CERT].handle,
                                                            sl_bt_gatt_disable);
            app_assert_status(sc);

            remote_cert_arrived = true;
            central_state = CENTRAL_SEND_CENTRAL_CERT;
            app_log_info("Sending certificate." APP_LOG_NL);
            STATE_TIMEOUT_START();

            sc = sl_bt_cbap_process_remote_cert(remote_certificate_der, remote_certificate_der_len);
            if (sc == SL_STATUS_OK) {
              app_log_info("Remote certificate verified." APP_LOG_NL);
            } else {
              app_log_error("Remote certificate verification failed." APP_LOG_NL);
              on_error();
              break;
            }
          }
        } else if (central_state == CENTRAL_GET_PERIPHERAL_OOB) {
          aes_key_128 remote_random;
          aes_key_128 remote_confirm;
          uint8_t remote_oob_signature[OOB_SIGNATURE_LEN];
          memcpy(&remote_random, &evt->data.evt_gatt_characteristic_value.value.data[0], sizeof(aes_key_128));
          memcpy(&remote_confirm, &evt->data.evt_gatt_characteristic_value.value.data[16], sizeof(aes_key_128));
          memcpy(&remote_oob_signature, &evt->data.evt_gatt_server_user_write_request.value.data[32], OOB_SIGNATURE_LEN);
          sc = sl_bt_gatt_send_characteristic_confirmation(evt->data.evt_gatt_characteristic_value.connection);
          app_assert_status(sc);
          central_state = CENTRAL_SEND_OOB;
          app_log_info("Sending OOB data." APP_LOG_NL);
          STATE_TIMEOUT_START();
          sc = sl_bt_gatt_set_characteristic_notification(candidate_device.connection_handle,
                                                          cbap_characteristics[CHAR_PERIPHERAL_OOB].handle,
                                                          sl_bt_gatt_disable);
          app_assert_status(sc);

          app_log_debug("Remote OOB data:" APP_LOG_NL);
          app_log_hexdump_debug(&remote_random, sizeof(aes_key_128));
          app_log_debug(APP_LOG_NL);
          app_log_hexdump_debug(&remote_confirm, sizeof(aes_key_128));
          app_log_debug(APP_LOG_NL);
          app_log_debug("Remote OOB signature:" APP_LOG_NL);
          app_log_hexdump_debug(&remote_oob_signature, OOB_SIGNATURE_LEN);
          app_log_debug(APP_LOG_NL);

          sc = sl_bt_cbap_verify_remote_oob_data(remote_random.data, remote_confirm.data, remote_oob_signature);
          app_assert_status(sc);
          app_log_info("Remote OOB data verified." APP_LOG_NL);
          sc = sl_bt_sm_set_remote_oob(1, remote_random, remote_confirm);
          app_assert_status(sc);
          sc = sl_bt_cbap_destroy_key();
          app_assert_status(sc);
        }
      }
      break;

    // -------------------------------
    // This event indicates that the value of an attribute in the local GATT
    // database was changed by a remote GATT client.
    case sl_bt_evt_gatt_server_attribute_value_id:
      if (role == sl_bt_connection_role_peripheral
          && gattdb_aio_digital_out == evt->data.evt_gatt_server_characteristic_status.characteristic) {
        // The value of the gattdb_aio_digital_out characteristic was changed.
        uint8_t data_recv;
        size_t data_recv_len;

        // Read characteristic value.
        sc = sl_bt_gatt_server_read_attribute_value(gattdb_aio_digital_out,
                                                    0,
                                                    sizeof(data_recv),
                                                    &data_recv_len,
                                                    &data_recv);
        (void)data_recv_len;
        app_log_status_error(sc);

        if (sc != SL_STATUS_OK) {
          break;
        }

        if (data_recv == 0x00) {
          // Turn off LED.
          sl_led_turn_off(SL_SIMPLE_LED_INSTANCE(0));
          app_log_info("LED off." APP_LOG_NL);
        } else {
          // Blink LED.
          sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(0));
          app_log_info("LED on." APP_LOG_NL);
          sc = app_timer_start(&led_timer, LED_TIMEOUT, led_timer_cb, CALLBACK_DATA, false);
          app_assert_status(sc);
        }
      }
      break;

    //--------------------------------
    // Indicates that a remote GATT client is attempting to write a value of an attribute into the local GATT database
    case sl_bt_evt_gatt_server_user_write_request_id:
      if (evt->data.evt_connection_parameters.connection != candidate_device.connection_handle) {
        break;
      }

      if (role == sl_bt_connection_role_peripheral) {
        // Set default response parameters.
        sc = SL_STATUS_BT_ATT_WRITE_REQUEST_REJECTED;
        // Receiving Certificate from central device
        if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_central_cert) {
          if (remote_cert_arrived == false) {
            // First byte indicates that it is a last packet or not
            memcpy(&remote_certificate_der[remote_certificate_der_len],
                   &evt->data.evt_gatt_server_user_write_request.value.data[1],
                   evt->data.evt_gatt_server_user_write_request.value.len - 1);
            remote_certificate_der_len += evt->data.evt_gatt_server_user_write_request.value.len - 1;
            sc = SL_STATUS_OK;
            if (evt->data.evt_gatt_server_user_write_request.value.data[0] == 0) {
              // Last packet of the remote cert arrived
              app_log_info("Getting certificate from central." APP_LOG_NL);
              remote_cert_arrived = true;
              sc = sl_bt_cbap_process_remote_cert(remote_certificate_der, remote_certificate_der_len);
              if (sc == SL_STATUS_OK) {
                app_assert(peripheral_state == PERIPHERAL_IDLE, "Unexpected peripheral state.");
                peripheral_state = PERIPHERAL_CENTRAL_CERT_OK;
                app_log_info("Remote certificate verified." APP_LOG_NL);
                STATE_TIMEOUT_START();
              } else {
                app_log_error("Remote certificate verification failed." APP_LOG_NL);
                on_error();
                break;
              }
            }
            // Map status code to a valid attribute error.
            if (SL_STATUS_OK != sc) {
              sc = SL_STATUS_BT_ATT_WRITE_REQUEST_REJECTED;
            }
          } else {
            sc = SL_STATUS_BT_ATT_PROCEDURE_ALREADY_IN_PROGRESS;
          }
          sl_bt_gatt_server_send_user_write_response(evt->data.evt_gatt_server_user_write_request.connection,
                                                     evt->data.evt_gatt_server_user_write_request.characteristic,
                                                     (uint8_t)sc);
        }
        // Receiving OOB data from central device
        else if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_central_oob ) {
          app_log_info("Getting OOB data from central." APP_LOG_NL);
          aes_key_128 remote_random;
          aes_key_128 remote_confirm;
          uint8_t remote_oob_signature[OOB_SIGNATURE_LEN];
          memcpy(&remote_random, &evt->data.evt_gatt_server_user_write_request.value.data[0], sizeof(remote_random));
          memcpy(&remote_confirm, &evt->data.evt_gatt_server_user_write_request.value.data[16], sizeof(remote_confirm));
          memcpy(&remote_oob_signature, &evt->data.evt_gatt_server_user_write_request.value.data[32], OOB_SIGNATURE_LEN);

          sc = sl_bt_gatt_server_send_user_write_response(evt->data.evt_gatt_server_user_write_request.connection,
                                                          evt->data.evt_gatt_server_user_write_request.characteristic,
                                                          SL_STATUS_OK);
          app_assert_status(sc);

          app_log_debug("Remote OOB data:" APP_LOG_NL);
          app_log_hexdump_debug(&remote_random, sizeof(aes_key_128));
          app_log_debug(APP_LOG_NL);
          app_log_hexdump_debug(&remote_confirm, sizeof(aes_key_128));
          app_log_debug(APP_LOG_NL);
          app_log_debug("Remote OOB signature:" APP_LOG_NL);
          app_log_hexdump_debug(&remote_oob_signature, OOB_SIGNATURE_LEN);
          app_log_debug(APP_LOG_NL);

          sc = sl_bt_cbap_verify_remote_oob_data(remote_random.data, remote_confirm.data, remote_oob_signature);
          app_assert_status(sc);
          app_log_info("Remote OOB data verified." APP_LOG_NL);
          sc = sl_bt_sm_set_remote_oob(1, remote_random, remote_confirm);
          app_assert_status(sc);
          sc = sl_bt_cbap_destroy_key();
          app_assert_status(sc);

          app_assert(peripheral_state == PERIPHERAL_CENTRAL_CERT_OK, "Unexpected peripheral state.");
          peripheral_state = PERIPHERAL_CENTRAL_OOB_OK;
          app_log_info("Remote OOB data verified." APP_LOG_NL);
          STATE_TIMEOUT_START();
        }
      }
      break;

    //--------------------------------
    // Indicates either that a local Client Characteristic Configuration descriptor was changed by the remote GATT
    // client, or that a confirmation from the remote GATT client was received upon a successful reception of the
    // indication
    case sl_bt_evt_gatt_server_characteristic_status_id:
      if (evt->data.evt_connection_parameters.connection != candidate_device.connection_handle) {
        break;
      }

      if (role == sl_bt_connection_role_peripheral) {
        if (gattdb_peripheral_cert == evt->data.evt_gatt_server_characteristic_status.characteristic) {
          if (sl_bt_gatt_server_client_config == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags
              && sl_bt_gatt_indication == (sl_bt_gatt_client_config_flag_t)evt->data.evt_gatt_server_characteristic_status.client_config_flags
              && device_cert_sent == false) {
            uint8_t buff[CERT_IND_CHUNK_LEN + 1];
            buff[0] = 1;
            memcpy(&buff[1], device_certificate_der, CERT_IND_CHUNK_LEN);
            sc = sl_bt_gatt_server_send_indication(candidate_device.connection_handle,
                                                   gattdb_peripheral_cert,
                                                   CERT_IND_CHUNK_LEN + 1,
                                                   buff);
            app_assert_status(sc);
            dev_cert_sending_progression += CERT_IND_CHUNK_LEN;
          }
          // Sending Peripheral certificate to Central device
          else if (sl_bt_gatt_server_confirmation == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags
                   && device_cert_sent == false) {
            uint32_t remaining = device_certificate_der_len - dev_cert_sending_progression;
            uint8_t buff[CERT_IND_CHUNK_LEN + 1];
            uint8_t len = 0;
            if (remaining > CERT_IND_CHUNK_LEN) {
              buff[0] = 1;
              len = CERT_IND_CHUNK_LEN + 1;
            } else {
              // Send last chunk
              buff[0] = 0;
              len = remaining + 1;
              device_cert_sent = true;
            }
            memcpy(&buff[1], &device_certificate_der[dev_cert_sending_progression], len - 1);
            dev_cert_sending_progression += len - 1;
            sc = sl_bt_gatt_server_send_indication(candidate_device.connection_handle,
                                                   gattdb_peripheral_cert,
                                                   len,
                                                   buff);
            app_assert_status(sc);
          }
        }
        // Sending Peripheral OOB data to Central device
        else if (gattdb_peripheral_oob == evt->data.evt_gatt_server_characteristic_status.characteristic
                 && sl_bt_gatt_server_client_config == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags
                 && sl_bt_gatt_indication == (sl_bt_gatt_client_config_flag_t)evt->data.evt_gatt_server_characteristic_status.client_config_flags) {
          aes_key_128 device_random;
          aes_key_128 device_confirm;
          // Generate device oob data and send over GATT
          sc = sl_bt_sm_set_oob(1, &device_random, &device_confirm);
          app_assert_status(sc);

          app_log_debug("Device OOB Data:" APP_LOG_NL);
          app_log_hexdump_debug(&device_random, OOB_RANDOM_LEN);
          app_log_debug(APP_LOG_NL);
          app_log_hexdump_debug(&device_confirm, OOB_RANDOM_LEN);
          app_log_debug(APP_LOG_NL);

          sc = sl_bt_cbap_sign_device_oob_data(device_random.data,
                                               device_confirm.data,
                                               signed_device_oob_data,
                                               &signed_device_oob_len);
          app_assert_status(sc);

          app_log_debug("Device OOB Signature:" APP_LOG_NL);
          app_log_hexdump_debug(&signed_device_oob_data[OOB_DATA_LEN], OOB_SIGNATURE_LEN);
          app_log_debug(APP_LOG_NL);

          sc = sl_bt_gatt_server_send_indication(candidate_device.connection_handle,
                                                 gattdb_peripheral_oob,
                                                 signed_device_oob_len,
                                                 signed_device_oob_data);
          app_assert_status(sc);
        }
      }
      break;

    // -------------------------------
    // This event is generated for various procedure completions, e.g. when a
    // write procedure is completed, or service discovery is completed
    case sl_bt_evt_gatt_procedure_completed_id:
      if (evt->data.evt_connection_parameters.connection != candidate_device.connection_handle) {
        break;
      }

      if (role == sl_bt_connection_role_central) {
        // Check result
        if (evt->data.evt_gatt_procedure_completed.result != 0) {
          app_log_error("GATT procedure completed error. Connection: %d. Error: 0x%04x." APP_LOG_NL,
                        evt->data.evt_gatt_procedure_completed.connection,
                        evt->data.evt_gatt_procedure_completed.result);
          on_error();
          break;
        }

        switch (central_state) {
          case CENTRAL_DISCOVER_SERVICES: {
            // Continue by finding the characteristics under the CBAP service.
            char_state = (characteristics_t)0; // Start with the first characteristic
            sc = sl_bt_gatt_discover_characteristics_by_uuid(evt->data.evt_gatt_procedure_completed.connection,
                                                             cbap_service_handle,
                                                             sizeof(cbap_characteristics[char_state].uuid),
                                                             (const uint8_t *)cbap_characteristics[char_state].uuid);
            app_assert_status(sc);
            central_state = CENTRAL_DISCOVER_CHARACTERISTICS;
            app_log_info("Discovering characteristics." APP_LOG_NL);
            STATE_TIMEOUT_START();
            break;
          }

          case CENTRAL_DISCOVER_CHARACTERISTICS: {
            char_state++;
            if (char_state < CHAR_NUM) {
              // Find the next characteristic
              sc = sl_bt_gatt_discover_characteristics_by_uuid(evt->data.evt_gatt_procedure_completed.connection,
                                                               cbap_service_handle,
                                                               sizeof(cbap_characteristics[char_state].uuid),
                                                               (const uint8_t *)cbap_characteristics[char_state].uuid);
              app_assert_status(sc);
            } else {
              // Get Peripheral certificates
              sc = sl_bt_gatt_set_characteristic_notification(evt->data.evt_gatt_procedure_completed.connection,
                                                              cbap_characteristics[CHAR_PERIPHERAL_CERT].handle,
                                                              sl_bt_gatt_indication);
              app_assert_status(sc);
              central_state = CENTRAL_GET_PERIPHERAL_CERT;
              app_log_info("Getting certificate from peripheral." APP_LOG_NL);
              STATE_TIMEOUT_START();
            }
            break;
          }

          case CENTRAL_SEND_CENTRAL_CERT: {
            if (!device_cert_sent) {
              uint32_t remaining = device_certificate_der_len - dev_cert_sending_progression;
              uint8_t buff[CERT_IND_CHUNK_LEN + 1];
              uint8_t len = 0;
              if (remaining > CERT_IND_CHUNK_LEN) {
                buff[0] = 1;
                memcpy(&buff[1], &device_certificate_der[dev_cert_sending_progression], CERT_IND_CHUNK_LEN);
                dev_cert_sending_progression += CERT_IND_CHUNK_LEN;
                len = CERT_IND_CHUNK_LEN + 1;
              } else {
                // Last chunk
                buff[0] = 0;
                memcpy(&buff[1], &device_certificate_der[dev_cert_sending_progression], remaining);
                len = remaining + 1;
                device_cert_sent = true;
              }
              sc = sl_bt_gatt_write_characteristic_value(candidate_device.connection_handle,
                                                         cbap_characteristics[CHAR_CENTRAL_CERT].handle,
                                                         len,
                                                         buff);
              app_assert_status(sc);
            } else {
              // If certificate exchange completed get OOB data. Enable indication.
              sc = sl_bt_gatt_set_characteristic_notification(evt->data.evt_gatt_procedure_completed.connection,
                                                              cbap_characteristics[CHAR_PERIPHERAL_OOB].handle,
                                                              sl_bt_gatt_indication);
              app_assert_status(sc);
              aes_key_128 device_random;
              aes_key_128 device_confirm;
              // Generate device oob data and send over GATT
              sc = sl_bt_sm_set_oob(1, &device_random, &device_confirm);
              app_assert_status(sc);

              app_log_debug("Device OOB Data:" APP_LOG_NL);
              app_log_hexdump_debug(&device_random, OOB_RANDOM_LEN);
              app_log_debug(APP_LOG_NL);
              app_log_hexdump_debug(&device_confirm, OOB_RANDOM_LEN);
              app_log_debug(APP_LOG_NL);

              sc = sl_bt_cbap_sign_device_oob_data(device_random.data,
                                                   device_confirm.data,
                                                   signed_device_oob_data,
                                                   &signed_device_oob_len);
              app_assert_status(sc);

              app_log_debug("Device OOB Signature:" APP_LOG_NL);
              app_log_hexdump_debug(&signed_device_oob_data[OOB_DATA_LEN], OOB_SIGNATURE_LEN);
              app_log_debug(APP_LOG_NL);

              central_state = CENTRAL_GET_PERIPHERAL_OOB;
              app_log_info("Getting OOB data from peripheral." APP_LOG_NL);
              STATE_TIMEOUT_START();
            }
            break;
          }

          case CENTRAL_SEND_OOB: {
            sc = sl_bt_gatt_write_characteristic_value(candidate_device.connection_handle,
                                                       cbap_characteristics[CHAR_CENTRAL_OOB].handle,
                                                       signed_device_oob_len,
                                                       signed_device_oob_data);
            app_assert_status(sc);

            // Request OOB data from both device
            sc = sl_bt_sm_configure(SL_BT_SM_CONFIGURATION_OOB_FROM_BOTH_DEVICES_REQUIRED,
                                    sl_bt_sm_io_capability_noinputnooutput);
            app_assert_status(sc);
            sc = sl_bt_sm_increase_security(candidate_device.connection_handle);
            app_assert_status(sc);

            central_state = CENTRAL_INCREASE_SECURITY;
            app_log_info("Increasing security level." APP_LOG_NL);
            STATE_TIMEOUT_START();
            break;
          }

          default: {
            break;
          }
        }
      }
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

/**************************************************************************//**
 * Handle CBAP process errors.
 *****************************************************************************/
static void on_error(void)
{
  if (candidate_device.connection_handle != SL_BT_INVALID_CONNECTION_HANDLE) {
    app_log_error("CBAP procedure was aborted for connection %d." APP_LOG_NL, candidate_device.connection_handle);

    (void)sl_bt_connection_close(candidate_device.connection_handle);
    clear_connection_info();
  }

  app_reset();
}

/***************************************************************************//**
 * Reset CBAP process states, flags and timers.
 ******************************************************************************/
static void app_reset(void)
{
  sl_status_t sc;
  STATE_TIMEOUT_STOP(); // Make sure timer is stopped
  candidate_device.connection_handle = SL_BT_INVALID_CONNECTION_HANDLE; // Clear connection handle

  // Reset states
  peripheral_state = (peripheral_state_t)0;
  central_state = (central_state_t)0;
  char_state = (characteristics_t)0;

  // Reset flags
  remote_cert_arrived = false;
  device_cert_sent = false;
  remote_certificate_der_len = 0;
  dev_cert_sending_progression = 0;

  if (next_available_connection() != -1) {
    switch (role) {
      case sl_bt_connection_role_peripheral:
        // Restart advertising and enable connections
        sc = sl_bt_legacy_advertiser_start(advertising_set_handle, sl_bt_legacy_advertiser_connectable);
        if (sc == SL_STATUS_OK) {
          app_log_info("Advertising started." APP_LOG_NL);
        }
        break;

      case sl_bt_connection_role_central:
        // Start scanning
        sc = sl_bt_scanner_start(sl_bt_scanner_scan_phy_1m, sl_bt_scanner_discover_generic);
        if (sc == SL_STATUS_OK) {
          app_log_info("Scanning started." APP_LOG_NL);
        }
        break;

      default:
        app_assert_status_f(SL_STATUS_INVALID_STATE, "Invalid role!");
        break;
    }
  } else {
    app_log_warning("Maximum number of connections reached (SL_BT_CONFIG_MAX_CONNECTIONS: %d)." APP_LOG_NL,
                    SL_BT_CONFIG_MAX_CONNECTIONS);
  }
}

/**************************************************************************//**
 * Clears candidate device.
 *****************************************************************************/
static void clear_connection_info(void)
{
  candidate_device.connection_handle = SL_BT_INVALID_CONNECTION_HANDLE;
  for (uint8_t i = 0; i < sizeof(bd_addr); i++) {
    candidate_device.address.addr[i] = 0xff;
  }
}

/**************************************************************************//**
 * Adds the candidate device to the trusted devices array.
 *****************************************************************************/
static void save_connection_info(void)
{
  int index = next_available_connection();
  if (index == -1) {
    app_log_error("Maximum number of connections reached (SL_BT_CONFIG_MAX_CONNECTIONS: %d)." APP_LOG_NL,
                  SL_BT_CONFIG_MAX_CONNECTIONS);
    return;
  }

  // Save connection parameters
  trusted_devices[index].connection_handle = candidate_device.connection_handle;
  trusted_devices[index].address = candidate_device.address;
  app_log_info("Trusted device [%d] added." APP_LOG_NL, trusted_devices[index].connection_handle);
}

/**************************************************************************//**
 * Finds next available connection slot.
 *****************************************************************************/
static int next_available_connection(void)
{
  for (int i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
    if (trusted_devices[i].connection_handle == SL_BT_INVALID_CONNECTION_HANDLE) {
      return i;
    }
  }
  return -1;
}

/**************************************************************************//**
 * Logs the connection handle and the Bluetooth address of the trusted devices.
 *****************************************************************************/
static void print_trusted_devices(void)
{
  bool found = false;
  app_log_info("List of trusted connections:" APP_LOG_NL);

  for (int i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
    if (trusted_devices[i].connection_handle != SL_BT_INVALID_CONNECTION_HANDLE) {
      found = true;
      app_log_info("  Connection handle: %d  Address: %02X:%02X:%02X:%02X:%02X:%02X" APP_LOG_NL,
                   trusted_devices[i].connection_handle,
                   trusted_devices[i].address.addr[5],
                   trusted_devices[i].address.addr[4],
                   trusted_devices[i].address.addr[3],
                   trusted_devices[i].address.addr[2],
                   trusted_devices[i].address.addr[1],
                   trusted_devices[i].address.addr[0]);
    }
  }

  if (!found) {
    app_log_info("  None." APP_LOG_NL);
  }
}

/**************************************************************************//**
 * Convert address string to address data bytes.
 * @param[in] addess_str Address string
 * @param[out] address Bluetooth address byte array
 * @return true if operation was successful
 *****************************************************************************/
static bool decode_address(char *addess_str, bd_addr *address)
{
  uint8_t retval;
  unsigned int address_cache[sizeof(bd_addr)];

  retval = sscanf(addess_str, "%02X:%02X:%02X:%02X:%02X:%02X",
                  &address_cache[5],
                  &address_cache[4],
                  &address_cache[3],
                  &address_cache[2],
                  &address_cache[1],
                  &address_cache[0]);

  if (retval != sizeof(bd_addr)) {
    app_log_error("Invalid Bluetooth address." APP_LOG_NL);
    return false;
  }

  for (uint8_t i = 0; i < sizeof(bd_addr); i++) {
    address->addr[i] = (uint8_t)(address_cache[i]);
  }
  return true;
}

/**************************************************************************//**
 * Examine a scan report and decide if a connection should be established.
 * @param[in] scan_report Scan report coming from the Bluetooth stack event.
 * return true if a connection should be established with the device.
 *****************************************************************************/
bool check_scan_report(sl_bt_evt_scanner_legacy_advertisement_report_t *scan_report)
{
  // Check if there is a connection with this device already
  for (int i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
    if (memcmp(scan_report->address.addr, trusted_devices[i].address.addr, sizeof(bd_addr)) == 0) {
      return false;
    }
  }

  // If target defined, check the address
  if (peripheral_target_defined
      && memcmp(scan_report->address.addr, peripheral_target_addr.addr, sizeof(bd_addr)) != 0) {
    return false; // Target device is defined but with different address.
  }

  // Look for CBAP service in advertisement packets
  return find_service_in_advertisement(scan_report->data.data,
                                       scan_report->data.len,
                                       cbap_service_uuid,
                                       sizeof(cbap_service_uuid));
}

/**************************************************************************//**
 * Search for a Service UUID in scan report.
 *
 * @param[in] scan_data Data received in scanner advertisement report event
 * @param[in] scan_data_len Length of the scan data
 * @param[in] uuid Service UUID to search for
 * @param[in] uuid_len Service UUID length
 * @return true if the service is found
 *****************************************************************************/
static bool find_service_in_advertisement(const uint8_t *scan_data,
                                          uint8_t scan_data_len,
                                          const uint8_t *uuid,
                                          uint8_t uuid_len)
{
  uint8_t ad_field_length;
  uint8_t ad_field_type;
  uint8_t i = 0;

  while (i < scan_data_len) {
    // Parse advertisement packet
    ad_field_length = scan_data[i];  // Not counting the length byte itself
    ad_field_type = scan_data[i + 1];
    if ((uuid_len == UUID_16_LEN && (ad_field_type == GAP_INCOMPLETE_16B_UUID
                                     || ad_field_type == GAP_COMPLETE_16B_UUID))
        || (uuid_len == UUID_128_LEN && (ad_field_type == GAP_INCOMPLETE_128B_UUID
                                         || ad_field_type == GAP_COMPLETE_128B_UUID))) {
      // Packet containing the list of complete/incomplete 16/128-bit services found.
      // Loop through the UUID list
      uint8_t j = 2;
      while (j < ad_field_length + 1) {
        // Compare payload.
        if (memcmp(&scan_data[i + j], uuid, uuid_len) == 0) {
          return true;
        }
        // Advance to the next UUID
        j += uuid_len;
      }
    }
    // Advance to the next packet
    i += ad_field_length + 1;
  }
  return false;
}

/**************************************************************************//**
 * Timer Callback.
 * @param[in] handle pointer to handle instance
 * @param[in] data pointer to input data
 *****************************************************************************/
static void state_machine_timeout_cb(app_timer_t *handle, void *data)
{
  (void)handle;
  (void)data;

  app_log_error("State machine timeout." APP_LOG_NL);
  on_error();
}

/**************************************************************************//**
 * Timer Callback.
 * @param[in] handle pointer to handle instance
 * @param[in] data pointer to input data
 *****************************************************************************/
static void led_timer_cb(app_timer_t *handle, void *data)
{
  (void)handle;
  (void)data;

  // Update local GATT
  sl_status_t sc;
  uint8_t data_sent = 0x00;
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_aio_digital_out,
                                               0,
                                               sizeof(data_sent),
                                               &data_sent);
  app_assert_status(sc);

  sl_led_turn_off(SL_SIMPLE_LED_INSTANCE(0));
  app_log_info("LED off." APP_LOG_NL);
}
