/***************************************************************************//**
 * @file
 * @brief Voice NCP-host Example.
 *
 * This is the client of Voice over Bluetooth Low Energy (VoBLE) application
 * that demonstrates Bluetooth connectivity using BGAPI and handling audio
 * streaming. The example enables Bluetooth advertisements and configures the
 * following VoBLE parameters:
 * 1. Sample Rate,
 * 2. Enable notification for Audio Data Characteristic
 * 3. Enable/Disable filtering
 * 4. Enable/Disable encoding
 * 5. Enable/Disable audio data streaming status
 * After initialization and establishing connection application waits for audio
 * data transmission and finally stores received data into the file or sends it
 * to stdout.
 * Most of the functionality in BGAPI uses a request-response-event pattern
 * where the module responds to a command with a command response indicating
 * it has processed the request and then later sending an event indicating
 * the requested operation has been completed.
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
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include "sl_bt_api.h"
#include "app_log.h"
#include "app_assert.h"
#include "app.h"
#include "ncp_host.h"

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "config.h"
#include "parse.h"
#include "ble_peer_manager_common.h"
#include "ble_peer_manager_central.h"
#include "ble_peer_manager_filter.h"

// connection parameters
#define CONNECTION_HANDLE_INVALID     SL_BT_INVALID_CONNECTION_HANDLE
#define SERVICE_HANDLE_INVALID        ((uint32_t)0xFFFFFFFFu)
#define CHARACTERISTIC_HANDLE_INVALID ((uint16_t)0xFFFFu)

//  Setup state machine
typedef enum {
  initializing,
  scanning,
  opening,
  discover_services,
  discover_characteristics,
  enable_indication,
  set_adc_sample_rate,
  set_filtering,
  set_encoding,
  transfer_status,
  running
} conn_state_t;

// Characteristic UUID iterator
typedef enum {
  char_audio_data = 0,
  char_sample_rate,
  char_filter,
  char_encoding,
  char_transfer,
  char_nonexistent
} char_db_index_t;

#define CHAR_UUID_LEN (16)
#define CHAR_N (16)

// Characteristic handles
static uint16_t char_handle[CHAR_N];

// Local function declarations
static void initalise_characteristic_handles(void);
static float timediff_sec_msec(struct timeval t0, struct timeval t1);

// Bluetooth connection
static uint8_t ble_connection;
static uint16_t audio_packetCnt = 0;
static bool transfer_started = false;
struct timeval current_time, start_time;

/***************************************************************************//**
 *  \brief  Save audio data to file
 *  \param[in] evt Event pointer.
 *  \return None.
 ******************************************************************************/
void save_audio_data_to_file(sl_bt_msg_t *evt)
{
  if (!transfer_started) {
    transfer_started = true;
    gettimeofday(&start_time, NULL);
  } else {
    gettimeofday(&current_time, NULL);
  }

  app_log_info("%.4f[s] - %d packet. Data length: %d [bytes]" APP_LOG_NL,
               timediff_sec_msec(start_time, current_time),
               audio_packetCnt++,
               evt->data.evt_gatt_characteristic_value.value.len);

  FILE *fd;
  if ((fd = fopen(get_config()->out_file_name, "ab+")) != NULL) {
    fwrite(evt->data.evt_gatt_characteristic_value.value.data,
           sizeof(uint8_t),
           evt->data.evt_gatt_characteristic_value.value.len,
           fd);
    fclose(fd);
  } else {
    app_log_error("File %s can not be open." APP_LOG_NL, get_config()->out_file_name);
    sl_status_t sc = ble_peer_manager_central_close_connection(ble_connection);
    app_assert(sc == SL_STATUS_OK,
               "[E: 0x%04x] Failed to close connection" APP_LOG_NL,
               (int)sc);
  }
}

/***************************************************************************//**
 *  \brief  Save audio data to standard output
 *  \param[in] evt Event pointer.
 *  \return None.
 ******************************************************************************/
void save_audio_data_to_stdout(sl_bt_msg_t *evt)
{
  fwrite(evt->data.evt_gatt_characteristic_value.value.data,
         sizeof(uint8_t),
         evt->data.evt_gatt_characteristic_value.value.len,
         stdout);
}

/**************************************************************************//**
 * @brief
 *    Application Init.
 * @param argc [in]
 *    Argument count
 * @param argv [in]
 *    Argument vector
 * @return
 *   None
 *****************************************************************************/
void app_init(int argc, char* argv[])
{
  // Parsing list of arguments
  parse_arguments(argc, argv);

  // Initialize NCP connection.
  sl_status_t sc = ncp_host_init();
  if (sc == SL_STATUS_INVALID_PARAMETER) {
    app_log(USAGE, argv[0]);
    exit(EXIT_FAILURE);
  }

  print_configuration();

  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////

  initalise_characteristic_handles();
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
void sl_bt_on_event(sl_bt_msg_t* evt)
{
  sl_status_t sc;
  static volatile uint32_t service_handle = SERVICE_HANDLE_INVALID;

  uuid_128 service_uuid = { .data = SERVICE_VOICE_OVER_BLE_UUID };
  const uint8_t service_uuid_len = sizeof(service_uuid) / sizeof(uint8_t);
  uint8_t char_iterator, uuid_iterator;
  uint8_t uuid_len;
  uint16_t result;

  // UUID database of characteristic
  static const uint8_t char_uuid_db[CHAR_N][CHAR_UUID_LEN] = { CHAR_AUDIO_DATA_UUID,
                                                               CHAR_SAMPLE_RATE_UUID,
                                                               CHAR_FILTER_ENABLE_UUID,
                                                               CHAR_ENCODING_ENABLE_UUID,
                                                               CHAR_TRANSFER_STATUS_UUID };

  // State of the connection under establishment
  static conn_state_t conn_state = initializing;

  bd_addr address;
  uint8_t address_type;

  // Handle stack events
  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Reset to initial state
      ble_peer_manager_central_init();
      ble_peer_manager_filter_init();

      // Filter for device
      if (get_config()->remote_address_set) {
        ble_peer_manager_set_filter_bt_address(true);
        sc = ble_peer_manager_add_allowed_bt_address(&get_config()->remote_address);
      } else {
        sc = ble_peer_manager_set_filter_service_uuid128(&service_uuid);
      }
      app_assert_status(sc);

      // Print bluetooth address.
      sc = sl_bt_system_get_identity_address(&address, &address_type);
      app_assert_status(sc);
      app_log_debug("Bluetooth %s address: ",
                    address_type ? "static random" : "public device");
      print_address(&address);

      uint16_t max_mtu_out = 0;
      sc = sl_bt_gatt_set_max_mtu(250, &max_mtu_out);
      app_assert(sc == SL_STATUS_OK,
                 "[E: 0x%04x] Failed to set max mtu" APP_LOG_NL,
                 (int)sc);
      app_log("System booted." APP_LOG_NL);

      app_log("Scanning for VoBLE devices..." APP_LOG_NL);
      sc = ble_peer_manager_central_create_connection();
      app_assert_status(sc);
      conn_state = scanning;
      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      app_log("Connected." APP_LOG_NL);
      //  Discover BT service. This will trigger a gatt service event
      sc = sl_bt_gatt_discover_primary_services_by_uuid(evt->data.evt_connection_opened.connection,
                                                        service_uuid_len,
                                                        service_uuid.data);
      app_assert(sc == SL_STATUS_OK,
                 "[E: 0x%04x] Failed to discover primary services" APP_LOG_NL,
                 (int)sc);
      conn_state = discover_services;
      break;

    // -------------------------------
    // This event indicates that a GATT procedure completed.
    case sl_bt_evt_gatt_procedure_completed_id:
      result =  evt->data.evt_gatt_procedure_completed.result;

      if (!result) {
        app_log_debug("Success." APP_LOG_NL);
      } else {
        app_log_error("Error: %d" APP_LOG_NL, result);
      }

      // Discover VoBLE characteristic
      if (conn_state == discover_services && service_handle != SERVICE_HANDLE_INVALID) {
        app_log("Discovering characteristic..." APP_LOG_NL);
        sc = sl_bt_gatt_discover_characteristics(evt->data.evt_gatt_procedure_completed.connection,
                                                 service_handle);

        app_assert(sc == SL_STATUS_OK,
                   "[E: 0x%04x] Failed to discover characteristics" APP_LOG_NL,
                   (int)sc);
        conn_state = discover_characteristics;
        break;
      }

      // If characteristic discovery finished. Enable audio data notifications
      if (conn_state == discover_characteristics && char_handle[char_audio_data] != CHARACTERISTIC_HANDLE_INVALID) {
        app_log("Enabling voice data flow." APP_LOG_NL);
        // stop discovering
        sl_bt_scanner_stop();
        sc = sl_bt_gatt_set_characteristic_notification(evt->data.evt_gatt_procedure_completed.connection,
                                                        char_handle[char_audio_data],
                                                        sl_bt_gatt_notification);
        app_assert(sc == SL_STATUS_OK,
                   "[E: 0x%04x] Failed to set notification for characteristic" APP_LOG_NL,
                   (int)sc);
        conn_state = enable_indication;
        break;
      }

      //  Set ADC sample rate
      if (conn_state == enable_indication && char_handle[char_sample_rate] != CHARACTERISTIC_HANDLE_INVALID) {
        app_log("Setting ADC sample rate." APP_LOG_NL);
        uint8_t sr = get_config()->adc_sample_rate;
        sc = sl_bt_gatt_write_characteristic_value(evt->data.evt_gatt_procedure_completed.connection,
                                                   char_handle[char_sample_rate],
                                                   1,
                                                   &sr);
        app_assert(sc == SL_STATUS_OK,
                   "[E: 0x%04x] Failed to set sample rate" APP_LOG_NL,
                   (int)sc);
        conn_state = set_adc_sample_rate;
        break;
      }

      //  Set filtering
      if (conn_state == set_adc_sample_rate && char_handle[char_filter] != CHARACTERISTIC_HANDLE_INVALID) {
        app_log("Setting filtering." APP_LOG_NL);
        uint8_t filt = get_config()->filter_enabled;
        sc = sl_bt_gatt_write_characteristic_value(evt->data.evt_gatt_procedure_completed.connection,
                                                   char_handle[char_filter],
                                                   1, // This going to be changed
                                                   &filt);
        app_assert(sc == SL_STATUS_OK,
                   "[E: 0x%04x] Failed to set filtering" APP_LOG_NL,
                   (int)sc);
        conn_state = set_filtering;
        break;
      }

      //  Enable encoding
      if (conn_state == set_filtering && char_handle[char_encoding] != CHARACTERISTIC_HANDLE_INVALID) {
        app_log("Setting encoding." APP_LOG_NL);
        uint8_t filt = get_config()->encoding_enabled;
        sc = sl_bt_gatt_write_characteristic_value(evt->data.evt_gatt_procedure_completed.connection,
                                                   char_handle[char_encoding],
                                                   1, // This going to be changed
                                                   &filt);
        app_assert(sc == SL_STATUS_OK,
                   "[E: 0x%04x] Failed to set encoding" APP_LOG_NL,
                   (int)sc);
        conn_state = set_encoding;
        break;
      }

      //  Transfer status
      if (conn_state == set_encoding && char_handle[char_transfer] != CHARACTERISTIC_HANDLE_INVALID) {
        app_log("Setting transfer status." APP_LOG_NL);
        uint8_t txfer_flags = sl_bt_gatt_disable;
        if (get_config()->transfer_status) {
          txfer_flags = sl_bt_gatt_notification;
        }
        sc = sl_bt_gatt_set_characteristic_notification(evt->data.evt_gatt_procedure_completed.connection,
                                                        char_handle[char_transfer],
                                                        txfer_flags);
        app_assert(sc == SL_STATUS_OK,
                   "[E: 0x%04x] Failed to set transfer status" APP_LOG_NL,
                   (int)sc);
        conn_state = transfer_status;
        app_log_info("Ready!" APP_LOG_NL);
        break;
      }

      break;

    // -------------------------------
    // This event is triggered when a new service is discovered
    case sl_bt_evt_gatt_service_id:
      app_log("New Service discovered." APP_LOG_NL);
      //  Save the service handle
      service_handle = evt->data.evt_gatt_service.service;
      conn_state = discover_services;
      break;

    // -------------------------------
    // This event is triggered when a new characteristic is discovered
    case sl_bt_evt_gatt_characteristic_id:
      // Compare the UUID with the local data

      // Select the smallest UUID length
      if (evt->data.evt_gatt_characteristic.uuid.len > CHAR_UUID_LEN) {
        uuid_len = CHAR_UUID_LEN;
      } else {
        uuid_len = evt->data.evt_gatt_characteristic.uuid.len;
      }

      // Look up the UUID
      for (char_iterator = 0; char_iterator < char_nonexistent; char_iterator++) {
        for (uuid_iterator = 0; uuid_iterator < uuid_len; uuid_iterator++ ) {
          if (evt->data.evt_gatt_characteristic.uuid.data[uuid_iterator] != char_uuid_db[char_iterator][uuid_iterator]) {
            break;
          }
        }
        if (uuid_iterator == uuid_len ) {
          // Found it!
          char_handle[char_iterator] = evt->data.evt_gatt_characteristic.characteristic;
          break;
        }
      }

      break;

    // -------------------------------
    // This event is triggered when a new characteristic value received
    case sl_bt_evt_gatt_characteristic_value_id:
      if (evt->data.evt_gatt_characteristic_value.characteristic == char_handle[char_audio_data]) {
        if ( get_config()->output_to_stdout == true ) {
          save_audio_data_to_stdout(evt);
        } else {
          save_audio_data_to_file(evt);
        }
      } else if (evt->data.evt_gatt_characteristic_value.characteristic == char_handle[char_transfer]) {
        FILE *fd;
        if ( (fd = fopen(get_config()->transfer_status_file_name, "w")) != NULL ) {
          fwrite(evt->data.evt_gatt_characteristic_value.value.data,
                 sizeof(uint8_t),
                 evt->data.evt_gatt_characteristic_value.value.len,
                 fd);
          fclose(fd);
        } else {
          app_log_error("File %s can not be opened." APP_LOG_NL, get_config()->transfer_status_file_name);
          sc = ble_peer_manager_central_close_connection(ble_connection);
          app_assert(sc == SL_STATUS_OK,
                     "[E: 0x%04x] Failed to close connection" APP_LOG_NL,
                     (int)sc);
        }
      }
      break;

    // -------------------------------
    // This event is triggered when a scan report received
    case sl_bt_evt_scanner_legacy_advertisement_report_id:
      conn_state = opening;
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

void app_ble_peer_manager_on_event(ble_peer_manager_evt_type_t *event)
{
  switch (event->evt_id) {
    case BLE_PEER_MANAGER_ON_CONN_OPENED_CENTRAL:
      app_log_info("Connection opened as central" APP_LOG_NL);
      break;

    case BLE_PEER_MANAGER_ON_CONN_OPENED_PERIPHERAL:
      app_log_info("Connection opened as peripheral" APP_LOG_NL);
      break;

    case BLE_PEER_MANAGER_ON_CONN_CLOSED:
      app_log_info("Connection closed" APP_LOG_NL);
      exit(EXIT_SUCCESS);
      break;

    case BLE_PEER_MANAGER_ON_ADV_STOPPED:
      app_log_info("Advertisement stopped" APP_LOG_NL);
      break;

    case BLE_PEER_MANAGER_ERROR:
      app_log_info("Error" APP_LOG_NL);
      break;

    default:
      app_log_info("Unhandled Peer Manager event (%u)" APP_LOG_NL, event->evt_id);
      break;
  }
}

/***************************************************************************//**
 *  \brief  Calculate difference between two time stamps and return elapsed time.
 *  \param[in]  t0 - first time stamp
 *  \param[in]  t1 - second time stamp
 *  \return elapsed time or 0 if elapsed time is negative
 ******************************************************************************/
static float timediff_sec_msec(struct timeval t0, struct timeval t1)
{
  float elapsed_time = ((t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f) / 1000;
  return (elapsed_time < 0) ? (float)0 : elapsed_time;
}

/**************************************************************************//**
 * Initialise characteristic handles.
 *****************************************************************************/
static void initalise_characteristic_handles(void)
{
  uint8_t characteristic_iterator;

  for (characteristic_iterator = 0; characteristic_iterator < CHAR_N; characteristic_iterator++ ) {
    char_handle[characteristic_iterator] = CHARACTERISTIC_HANDLE_INVALID;
  }
}
