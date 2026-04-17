/***************************************************************************//**
 * @file
 * @brief Helper functions for BLE interoperability test.
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
#include <stdio.h>
#include "sl_bt_api.h"
#include "gatt_db.h"
#include "sl_component_catalog.h"
#include "app_timer.h"
#include "app_assert.h"
#include "app_log.h"
#include "app_memlcd.h"
#include "app_iop.h"
#include "app.h"

#ifdef SL_CATALOG_APP_OTA_DFU_PRESENT
#include "sl_bt_app_ota_dfu.h"

// Status indication frequency in ms
#define DOWNLOAD_IND_FREQ        1000u
#define VERIFICATION_IND_FREQ    2500u

// Returns a data blocks percentage based on a byte position.
#define GET_DATA_PERCENTAGE(storage_size, actual_byte_pos) \
  (uint8_t) (actual_byte_pos / (storage_size / 100U))      \

// Estimate the actual transfer speed in Kbps based on the current byte
// position in the used storage and the elapsed seconds.
#define GET_TRANSFER_SPEED_KBPS(actual_byte_pos, elapsed_sec) \
  (uint32_t) (actual_byte_pos * 8U / (1024U * elapsed_sec))   \

// OTA Transfer
static sl_bt_app_ota_dfu_status_t app_ota_dfu_status = SL_BT_APP_OTA_DFU_UNINIT;
static uint32_t slot_size = 0u;
static uint16_t datablock_idx = 0u;
static uint32_t write_position = 0u;
static uint32_t verify_position = 0u;
static uint16_t time_elapsed = 0u;

// Private function to handle the Application OTA DFU states.
static void app_ota_dfu_on_status_change(sl_bt_app_ota_dfu_status_t curr_sts,
                                         sl_bt_app_ota_dfu_status_t prev_sts,
                                         sl_bt_app_ota_dfu_error_t app_ota_dfu_error_code,
                                         int32_t boot_api_err_code);
#endif // SL_CATALOG_APP_OTA_DFU_PRESENT

// Size of the arrays for sending and receiving data
#define DATA_SIZE_MAX     255
// Size of the arrays for indication or notification data
#define DATA_SIZE_NOTIFY  250

//--------------------------------
// Data payload to be sent over indications or notifications.
// Characteristic value for gattdb_iop_test_notify_len_1 and
// gattdb_iop_test_indicate_len_1 characteristics. Test cases: 4.7, 4.9.
static uint8_t iop_test_notification_1;
// Characteristic value for gattdb_iop_test_notify_len_mtu_3,
// gattdb_iop_test_indicate_len_mtu_3 and gattdb_iop_test_throughput
// characteristics. Test cases: 4.8, 4.10, 7.1.
static uint8_t iop_test_notification_250_arr[DATA_SIZE_NOTIFY];

//--------------------------------
// Variables for the remote device to write and read back.
// Storage for the value of the gattdb_iop_test_user_len_1 characteristic.
// Test case: 5.6.
static uint8_t iop_test_chr_user_1;
// Storage for the value of the gattdb_iop_test_user_len_255 characteristic.
// Test case: 5.7.
static uint8_t iop_test_chr_user_255_arr[DATA_SIZE_MAX];
static bool prepare_write_request_received = false;
// Storage for the value and the length of the gattdb_iop_test_user_len_var_4
// characteristic. Test case: 5.8.
static uint8_t iop_test_chr_user_var_4_arr[4];
static uint8_t iop_test_chr_user_var_4_len;

//--------------------------------
// Connection parameters
uint16_t connection_interval = DEFAULT_CONNECTION_INTERVAL;
uint16_t responder_latency = DEFAULT_RESPONDER_LATENCY;
uint16_t supv_timeout = DEFAULT_SUPV_TIMEOUT;
uint16_t mtu_size = DEFAULT_PDU - 4;
uint16_t pdu_size = DEFAULT_PDU;
uint8_t phy = DEFAULT_PHY;

// Storage for the connection parameters, defined in iop_test_connection.
static uint8_t iop_connection_arr[11];

//--------------------------------
// Security level request from the tester.
security_level_t security_level = SECURITY_LEVEL_NONE;

// Encryption key for pairing and bonding.
uint32_t passkey = 123456;

//--------------------------------
// Throughput
#define THROUGHPUT_TIMEOUT (10000) // [ms]
static bool throughput_in_progress = false;

//--------------------------------
// Timer
#define TIMER_TRIGGER (100) // [ms]
static app_timer_t timer_iop; // Timer for the IOP test cases
static app_timer_t timer_ota; // Timer for the OTA DFU process
static uint16_t timer_cb_data;

/***************************************************************************//**
 * Timer Callback
 * @param[in] handle pointer to handle instance
 * @param[in] data pointer to input data
 ******************************************************************************/
static void timer_cb(app_timer_t *handle, void *data);

//--------------------------------
// Display operations
bd_addr public_address;
#define STATE_STRING "State"
#define PASSKEY_STRING "Passkey"
#define ADDRESS_STRING "Address"
#define INDENT_STRING "  "

// Initialize test data values.
void app_test_data_init(void)
{
  iop_test_notification_1 = 0x55;

  for (int i = 0; i < DATA_SIZE_NOTIFY; i++) {
    iop_test_notification_250_arr[i] = i;
  }

  iop_test_chr_user_1 = 0x00;

  for (int i = 0; i < DATA_SIZE_MAX; i++) {
    iop_test_chr_user_255_arr[i] = 0x00;
  }
  prepare_write_request_received = false;

  for (int i = 0; i < 4; i++) {
    iop_test_chr_user_var_4_arr[i] = 0x00;
  }

  iop_test_chr_user_var_4_len = 1;
}

// Throughput test process step. Sends out a single notification.
void app_throughput_step(void)
{
  sl_status_t sc;
  if (throughput_in_progress) {
    sc = sl_bt_gatt_server_notify_all(gattdb_iop_test_throughput,
                                      pdu_size - 4 - 3,
                                      iop_test_notification_250_arr);
    (void)sc;
    app_proceed();
  }
}

// Handle read operation for user-type characteristics.
sl_status_t handle_user_read(sl_bt_evt_gatt_server_user_read_request_t *user_read_req)
{
  if (user_read_req == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sl_status_t sc = SL_STATUS_OK;
  uint8_t interval_whole;
  uint8_t interval_partial;
  uint16_t bytes_to_send;

  switch (user_read_req->characteristic) {
    case gattdb_iop_test_connection:
      // Packing MTU Size.
      iop_connection_arr[0] = mtu_size & 0xff;
      iop_connection_arr[1] = mtu_size >> 8;
      // Packing PDU Size.
      iop_connection_arr[2] = pdu_size & 0xff;
      iop_connection_arr[3] = pdu_size >> 8;
      // Packing Connection Interval.
      iop_connection_arr[4] = connection_interval & 0xff;
      iop_connection_arr[5] = connection_interval >> 8;
      // Packing Responder Latency.
      iop_connection_arr[6] = responder_latency & 0xff;
      iop_connection_arr[7] = responder_latency >> 8;
      // Packing Supervision Timeout.
      iop_connection_arr[8] = supv_timeout & 0xff;
      iop_connection_arr[9] = supv_timeout >> 8;
      // Packing PHY
      iop_connection_arr[10] = phy;

      interval_whole = (uint8_t)(connection_interval * 1.25f);
      interval_partial = (uint8_t)(100 * (connection_interval * 1.25f)) - 100 * interval_whole;

      app_log_debug("Sending connection parameters: "
                    "MTU = %u, PDU = %u, Connection Interval = %u.%2u[ms], "
                    "Responder Latency = %u, Supervision Timeout = %u[ms] PHY = %u" APP_LOG_NL,
                    mtu_size,
                    pdu_size,
                    interval_whole,
                    interval_partial,
                    responder_latency,
                    supv_timeout * 10,
                    phy);

      sc = sl_bt_gatt_server_send_user_read_response(user_read_req->connection,
                                                     user_read_req->characteristic,
                                                     (uint8_t)SL_STATUS_OK,
                                                     sizeof(iop_connection_arr),
                                                     (uint8_t *)&iop_connection_arr,
                                                     NULL);
      break;

    //--------------------------------
    case gattdb_iop_test_user_len_1:
      // Test 5.6.2 (iop_test_user_len_1) takes place now.
      app_log_info("Read user-type bytes for gattdb_iop_test_user_len_1." APP_LOG_NL);
      sc = sl_bt_gatt_server_send_user_read_response(user_read_req->connection,
                                                     user_read_req->characteristic,
                                                     (uint8_t)SL_STATUS_OK,
                                                     sizeof(iop_test_chr_user_1),
                                                     (uint8_t *)&iop_test_chr_user_1,
                                                     NULL);
      break;

    case gattdb_iop_test_user_len_255:
      // Test 5.7.2 (iop_test_user_len_255) takes place now.
      app_log_info("Read user-type bytes for gattdb_iop_test_user_len_255. "
                   "offset:%d." APP_LOG_NL,
                   user_read_req->offset);
      bytes_to_send = sizeof(iop_test_chr_user_255_arr) - user_read_req->offset;
      if (bytes_to_send > mtu_size - 1) {
        bytes_to_send = mtu_size - 1; // Clamp to MTU - 1.
      }

      if (bytes_to_send > 0) {
        uint16_t sent_len;
        sc = sl_bt_gatt_server_send_user_read_response(user_read_req->connection,
                                                       user_read_req->characteristic,
                                                       (uint8_t)SL_STATUS_OK,
                                                       bytes_to_send,
                                                       (uint8_t *)&iop_test_chr_user_255_arr[user_read_req->offset],
                                                       &sent_len);
        if (sc == SL_STATUS_OK) {
          app_log_info("Length of data:%d; mtu:%u; result:%x; req len:%d; "
                       "sent len:%d." APP_LOG_NL,
                       sizeof(iop_test_chr_user_255_arr),
                       mtu_size,
                       (int)sc,
                       bytes_to_send,
                       sent_len);
        }
      } else {
        // Invalid value length because of the offset.
        app_log_error("Reading iop_test_user_len_255 with invalid length "
                      "because of offset: 0x%02x." APP_LOG_NL,
                      user_read_req->offset);
        sc = sl_bt_gatt_server_send_user_read_response(user_read_req->connection,
                                                       user_read_req->characteristic,
                                                       (uint8_t)SL_STATUS_BT_ATT_INVALID_ATT_LENGTH,
                                                       (size_t)0,
                                                       NULL,
                                                       NULL);
      }
      break;

    case gattdb_iop_test_user_len_var_4:
      // Test 5.8.1.2 (iop_test_user_len_var_4) takes place now.
      // Test 5.8.2.2 (iop_test_user_len_var_4) takes place now.
      app_log_info("Read user-type bytes for "
                   "gattdb_iop_test_user_len_var_4." APP_LOG_NL);
      sc = sl_bt_gatt_server_send_user_read_response(user_read_req->connection,
                                                     user_read_req->characteristic,
                                                     (uint8_t)SL_STATUS_OK,
                                                     iop_test_chr_user_var_4_len,
                                                     (uint8_t *)&iop_test_chr_user_var_4_arr,
                                                     NULL);
      break;

    default:
      // Not IOP-related characteristic, no need to handle.
      break;
  }

  return sc;
}

// Handle write operation for user-type characteristics.
sl_status_t handle_user_write(sl_bt_evt_gatt_server_user_write_request_t *user_write_req)
{
  sl_status_t sc = SL_STATUS_OK;
  uint8_t ec = (uint8_t)SL_STATUS_OK;

  if (user_write_req == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (prepare_write_request_received
      && (user_write_req->att_opcode == sl_bt_gatt_execute_write_request)) {
    // Upon sl_bt_gatt_execute_write_request the GATT server is processing the
    // execute write, the characteristic is set to 0 and should be ignored.
    // End of Test 5.7.1, every data piece arrived.
    prepare_write_request_received = false;
    sc = sl_bt_gatt_server_send_user_write_response(user_write_req->connection,
                                                    user_write_req->characteristic,
                                                    (uint8_t)SL_STATUS_OK);
    if (sc == SL_STATUS_OK) {
      app_log_info("Execute write user-type bytes for "
                   "gattdb_iop_test_user_len_255." APP_LOG_NL);
    }

    return sc;
  }

  switch (user_write_req->characteristic) {
    case gattdb_iop_test_user_len_1:
      // Test 5.6.1 (iop_test_user_len_1) takes place now.
      app_log_info("Write user-type bytes for gattdb_iop_test_user_len_1. "
                   "length: %d offset: %d." APP_LOG_NL,
                   user_write_req->value.len,
                   user_write_req->offset);
      if (user_write_req->value.len <= sizeof(iop_test_chr_user_1)) {
        memcpy(&iop_test_chr_user_1,
               user_write_req->value.data,
               user_write_req->value.len);
        sc = sl_bt_gatt_server_send_user_write_response(user_write_req->connection,
                                                        user_write_req->characteristic,
                                                        SL_STATUS_OK);
      } else {
        ec = (uint8_t)SL_STATUS_BT_ATT_INVALID_ATT_LENGTH;
      }
      break;

    case gattdb_iop_test_user_len_255:
      // Test 5.7.1 (iop_test_user_len_255) takes place now.
      if ((user_write_req->value.len + user_write_req->offset) <= sizeof(iop_test_chr_user_255_arr)) {
        if (user_write_req->att_opcode == sl_bt_gatt_prepare_write_request) {
          // Store the arriving data chunk
          memcpy(&iop_test_chr_user_255_arr[user_write_req->offset],
                 user_write_req->value.data,
                 user_write_req->value.len);
          // The sl_bt_gatt_prepare_write_request attribute was used to write.
          // Set flag to expect sl_bt_gatt_execute_write_request
          prepare_write_request_received = true;
          // The application needs to respond to this request by using the
          // sl_bt_gatt_server_send_prepare_user_write_response.
          sc = sl_bt_gatt_server_send_user_prepare_write_response(user_write_req->connection,
                                                                  user_write_req->characteristic,
                                                                  SL_STATUS_OK,
                                                                  user_write_req->offset,
                                                                  (size_t)user_write_req->value.len,
                                                                  user_write_req->value.data);

          if (sc == SL_STATUS_OK) {
            app_log_info("Prepare write user-type bytes for gattdb_iop_test_user_len_255. "
                         "length: %d offset: %d." APP_LOG_NL,
                         user_write_req->value.len,
                         user_write_req->offset);
          }
        } else {
          // The sl_bt_gatt_write_request attribute was used to write. The
          // application needs to respond to this request by using the
          // sl_bt_gatt_server_send_user_write_response.
          sc = sl_bt_gatt_server_send_user_write_response(user_write_req->connection,
                                                          user_write_req->characteristic,
                                                          SL_STATUS_OK);
          if (sc == SL_STATUS_OK) {
            app_log_info("Write / Execute write user-type bytes for gattdb_iop_test_user_len_255. "
                         "length: %d offset: %d." APP_LOG_NL,
                         user_write_req->value.len,
                         user_write_req->offset);
          }
        }
      } else {
        app_log_error("Fail to write user-type bytes for gattdb_iop_test_user_len_255. "
                      "length: %d offset: %d." APP_LOG_NL,
                      user_write_req->value.len,
                      user_write_req->offset);
        ec = (uint8_t)SL_STATUS_BT_ATT_INVALID_ATT_LENGTH;
      }
      break;

    case gattdb_iop_test_user_len_var_4:
      // Test 5.8.1.1 (iop_test_user_len_var_4) takes place now.
      // Test 5.8.2.1 (iop_test_user_len_var_4) takes place now.
      app_log_info("Write user-type bytes for "
                   "gattdb_iop_test_user_len_var_4. length: %d." APP_LOG_NL,
                   user_write_req->value.len);

      if (user_write_req->value.len <= sizeof(iop_test_chr_user_var_4_arr)) {
        // Save length for next reading
        iop_test_chr_user_var_4_len = user_write_req->value.len;
        memcpy(&iop_test_chr_user_var_4_arr,
               user_write_req->value.data,
               iop_test_chr_user_var_4_len);
        sc = sl_bt_gatt_server_send_user_write_response(user_write_req->connection,
                                                        user_write_req->characteristic,
                                                        SL_STATUS_OK);
      } else {
        ec = (uint8_t)SL_STATUS_BT_ATT_INVALID_ATT_LENGTH;
      }
      break;

    case gattdb_ota_control:
      // The value of OTA Control characteristic was changed. This indicates there is a request to perform OTA-DFU.
      // The request is handled in either the "In-Place OTA DFU" or in the "Application OTA DFU" component.
      // Test 6.1 (IOP Test OTA update with ACK) takes place now.
      // Test 6.2 (IOP Test OTA update without ACK) takes place now.
      app_log_info("OTA-DFU." APP_LOG_NL);
      set_display(DISPLAY_STATE_OTA_DFU, NULL);
      break;

    case gattdb_iop_test_phase3_control:
      app_log_info("Write user-type bytes for gattdb_iop_test_phase3_control. "
                   "length: %d." APP_LOG_NL,
                   user_write_req->value.len);
      // Byte 0: OS type, older versions used it in Test 7.1 (Throughput), currently unused
      // Byte 1: Security level
      // Byte 2: Reserved
      if ((user_write_req->value.len < 2) || (user_write_req->value.len > 3)) {
        app_log_error("Invalid length for gattdb_iop_test_phase3_control" APP_LOG_NL);
        ec = (SL_STATUS_BT_ATT_INVALID_ATT_LENGTH & ~SL_STATUS_SPACE_MASK);
      }

      sc = sl_bt_gatt_server_send_user_write_response(user_write_req->connection,
                                                      user_write_req->characteristic,
                                                      ec);
      app_log_status_error(sc);
      if (ec != SL_STATUS_OK) {
        // Stop processing the value
        break;
      }

      security_level = (security_level_t)user_write_req->value.data[1];
      app_log_info("Mobile OS: [0x%02x], Requesting security level: [0x%02x]." APP_LOG_NL,
                   user_write_req->value.data[0],
                   security_level);
      if (security_level > SECURITY_LEVEL_PRIVACY) {
        // Map invalid values to NONE
        security_level = SECURITY_LEVEL_NONE;
      }

      if (security_level != SECURITY_LEVEL_NONE) {
        sc = sl_bt_connection_close(user_write_req->connection);
        if (sc == SL_STATUS_IDLE) {
          app_log_info("Connection kept open." APP_LOG_NL);
        } else {
          app_log_info("Closing connection." APP_LOG_NL);
        }
      }
      break;

    default:
      // Not IOP-related characteristic, no need to handle.
      break;
  }

  if (ec != (uint8_t)SL_STATUS_OK) {
    // It is either an invalid characteristic or invalid value length.
    app_log_error("[E: 0x%04x] sl_bt_evt_gatt_server_user_write_request_id: "
                  "invalid characteristic [%04d] or invalid value "
                  "length [%04d]." APP_LOG_NL,
                  (int)ec,
                  (int)user_write_req->characteristic,
                  (int)user_write_req->value.len);

    sc = sl_bt_gatt_server_send_user_write_response(user_write_req->connection,
                                                    user_write_req->characteristic,
                                                    ec);
  }

  return sc;
}

// Set and start timer for corresponding test case.
sl_status_t handle_timer_start(sl_bt_evt_gatt_server_characteristic_status_t *char_stat)
{
  sl_status_t sc = SL_STATUS_OK;
  timer_cb_data = char_stat->characteristic;

  switch (char_stat->characteristic) {
    case gattdb_iop_test_notify_len_1:
      if (char_stat->client_config_flags == sl_bt_gatt_notification) {
        // Test 4.7 (iop_test_notify_len_1) takes place now.
        sc = app_timer_start(&timer_iop,
                             TIMER_TRIGGER,
                             timer_cb,
                             (void *)&timer_cb_data,
                             false);
      }
      break;

    case gattdb_iop_test_notify_len_mtu_3:
      if (char_stat->client_config_flags == sl_bt_gatt_notification) {
        // Test 4.8 (iop_test_notify_len_mtu_3) takes place now.
        sc = app_timer_start(&timer_iop,
                             TIMER_TRIGGER,
                             timer_cb,
                             (void *)&timer_cb_data,
                             false);
      }
      break;

    case gattdb_iop_test_indicate_len_1:
      if (char_stat->client_config_flags == sl_bt_gatt_indication) {
        // Test 4.9 (iop_test_indicate_len_1) takes place now.
        sc = app_timer_start(&timer_iop,
                             TIMER_TRIGGER,
                             timer_cb,
                             (void *)&timer_cb_data,
                             false);
      }
      break;

    case gattdb_iop_test_indicate_len_mtu_3:
      if (char_stat->client_config_flags == sl_bt_gatt_indication) {
        // Test 4.10 (iop_test_indicate_len_mtu_3) takes place now.
        sc = app_timer_start(&timer_iop,
                             TIMER_TRIGGER,
                             timer_cb,
                             (void *)&timer_cb_data,
                             false);
      }
      break;

    //--------------------------------
    case gattdb_iop_test_throughput: {
      if (char_stat->client_config_flags == sl_bt_gatt_notification) {
        // Test 7.1 (Throughput) takes place now.
        uint8_t interval_whole = (uint8_t)(connection_interval * 1.25f);
        uint8_t interval_partial = (uint8_t)(100 * (connection_interval * 1.25f)) - 100 * interval_whole;

        app_log_info("Throughput start. Connection parameters: "
                     "MTU = %u, PDU = %u, Connection Interval = %u.%2u[ms], PHY = %u" APP_LOG_NL,
                     mtu_size,
                     pdu_size,
                     interval_whole,
                     interval_partial,
                     phy);

        throughput_in_progress = true;
        app_proceed();

        // Start timer for timeout checking
        sc = app_timer_start(&timer_iop,
                             THROUGHPUT_TIMEOUT,
                             timer_cb,
                             (void *)&timer_cb_data,
                             false);
      } else if (char_stat->client_config_flags == sl_bt_gatt_disable) {
        sc = app_timer_stop(&timer_iop);

        throughput_in_progress = false;
        app_log_info("Throughput stop." APP_LOG_NL);
      }
      break;
    }

    case gattdb_iop_test_bonded:
      app_log_info("Test bonded CCC status: %d" APP_LOG_NL,
                   char_stat->client_config_flags);
      break;

    default:
      break;
  }
  return sc;
}

// Timer Callback
static void timer_cb(app_timer_t *handle, void *data)
{
  if (handle == NULL) {
    app_assert_status(SL_STATUS_NULL_POINTER);
  }

  if (handle == &timer_iop) {
    sl_status_t sc = SL_STATUS_OK;

    if (data == NULL) {
      app_assert_status(SL_STATUS_NULL_POINTER);
    }

    switch (*(uint16_t *)data) {
      case gattdb_iop_test_notify_len_1:
        sc = sl_bt_gatt_server_notify_all(gattdb_iop_test_notify_len_1,
                                          sizeof(iop_test_notification_1),
                                          &iop_test_notification_1);
        break;

      case gattdb_iop_test_notify_len_mtu_3:
        sc = sl_bt_gatt_server_notify_all(gattdb_iop_test_notify_len_mtu_3,
                                          mtu_size - 3,
                                          iop_test_notification_250_arr);
        break;

      case gattdb_iop_test_indicate_len_1:
        sc = sl_bt_gatt_server_notify_all(gattdb_iop_test_indicate_len_1,
                                          sizeof(iop_test_notification_1),
                                          &iop_test_notification_1);
        break;

      case gattdb_iop_test_indicate_len_mtu_3:
        sc = sl_bt_gatt_server_notify_all(gattdb_iop_test_indicate_len_mtu_3,
                                          mtu_size - 3,
                                          iop_test_notification_250_arr);
        break;

      case gattdb_iop_test_throughput:
        // Throughput test timeout
        throughput_in_progress = false;
        app_log_error("Throughput test timeout." APP_LOG_NL);
        break;

      default:
        break;
    }

    app_log_status_error(sc);
  } else if (handle == &timer_ota) {
#ifdef SL_CATALOG_APP_OTA_DFU_PRESENT
    // Indicate App OTA process
    if (app_ota_dfu_status == SL_BT_APP_OTA_DFU_DOWNLOAD_BEGIN) {
      time_elapsed++;
      uint8_t percentage = GET_DATA_PERCENTAGE(slot_size, write_position);
      app_log_info("Received packets: %u, storage used: %u%%.(%lu kbps)" APP_LOG_NL,
                   datablock_idx,
                   percentage,
                   GET_TRANSFER_SPEED_KBPS(write_position, time_elapsed));
      set_display(DISPLAY_STATE_OTA_DFU, &percentage);
    } else {
      app_log_info("Verified %u%% of the new image.(%u block)" APP_LOG_NL,
                   GET_DATA_PERCENTAGE(write_position, verify_position),
                   datablock_idx);
    }
#endif // SL_CATALOG_APP_OTA_DFU_PRESENT
  } else {
    app_log_error("Invalid timer handle." APP_LOG_NL);
  }
}

// Handle display update.
void set_display(display_state_t state, void *data)
{
  sl_status_t sc;

  // Clear display
  app_memlcd_clear();

  // Read device name from the local GATT table
  size_t name_len;
  uint8_t name[MAX_CHARS_PER_LINE + 1];
  sc = sl_bt_gatt_server_read_attribute_value(gattdb_device_name,
                                              0,
                                              sizeof(name) - 1,
                                              &name_len,
                                              name);
  app_log_status_error(sc);

  if (sc == SL_STATUS_OK) {
    // Make sure that the attribute value is null terminated.
    name[name_len] = '\0';

    app_memlcd_draw_string(FONT_NORMAL, "%s", (char *)name);
    app_memlcd_newline(1);
  } else {
    app_memlcd_newline(2);
  }

  // Display bluetooth address
  app_memlcd_draw_string(FONT_NORMAL, ADDRESS_STRING);
  app_memlcd_draw_string(FONT_NARROW,
                         INDENT_STRING "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
                         public_address.addr[5],
                         public_address.addr[4],
                         public_address.addr[3],
                         public_address.addr[2],
                         public_address.addr[1],
                         public_address.addr[0]);
  app_memlcd_newline(1);

  // Display state
  switch (state) {
    case DISPLAY_STATE_IDLE:
      app_memlcd_draw_string(FONT_NORMAL, STATE_STRING);
      app_memlcd_draw_string(FONT_NARROW, INDENT_STRING "Advertising");
      break;

    case DISPLAY_STATE_CONNECTED:
      app_memlcd_draw_string(FONT_NORMAL, STATE_STRING);
      app_memlcd_draw_string(FONT_NARROW, INDENT_STRING "Connected");
      break;

    case DISPLAY_STATE_PASSKEY:
      // Display key for inputting on other device
      if (data == NULL) {
        app_assert_status(SL_STATUS_NULL_POINTER);
      }
      app_memlcd_draw_string(FONT_NORMAL, PASSKEY_STRING);
      app_memlcd_draw_string(FONT_NARROW, INDENT_STRING "%06lu", *(uint32_t *)data);
      break;

    case DISPLAY_STATE_BONDING:
      app_memlcd_draw_string(FONT_NORMAL, STATE_STRING);
      app_memlcd_draw_string(FONT_NARROW, INDENT_STRING "Bonded");
      break;

    case DISPLAY_STATE_OTA_DFU:
      app_memlcd_draw_string(FONT_NORMAL, STATE_STRING);
      if (data == NULL) {
        app_memlcd_draw_string(FONT_NARROW, INDENT_STRING "OTA DFU");
      } else {
        app_memlcd_draw_string(FONT_NARROW, INDENT_STRING "OTA DFU [%u%%]", *(uint8_t *)data);
      }
      break;

    default:
      break;
  }
  app_memlcd_newline(1);

  // Update screen
  app_memlcd_update();
}

#ifdef SL_CATALOG_APP_OTA_DFU_PRESENT
/**************************************************************************//**
 * Strong implementation of the callback for application OTA DFU status change.
 * The whole process can be tracked with this function from the storage erase
 * until the verification process.
 *****************************************************************************/
void sl_bt_app_ota_dfu_on_status_event(sl_bt_app_ota_dfu_status_evt_t* evt)
{
  // Bootloader information
  BootloaderType_t btl_type = NO_BOOTLOADER;
  uint32_t btl_version = 0;

  switch (evt->event_id) {
    case SL_BT_APP_OTA_DFU_EVT_BTL_STORAGE_INFO_ID:
      // Get bootloader and storage information.
      btl_type = evt->evt_info.btl_storage.bootloader_type;
      btl_version = evt->evt_info.btl_storage.bootloader_ver;
      slot_size = evt->evt_info.btl_storage.storage_size_bytes;

      app_log_info("Checking bootloader type:" APP_LOG_NL);

      switch (btl_type) {
        case NO_BOOTLOADER:
          app_log_error("Error: no bootloader detected!" APP_LOG_NL);
          app_log_error("Application OTA will not work without an OTA-capable bootloader!" APP_LOG_NL);
          break;

        case SL_BOOTLOADER:
          app_log_info("Silicon Labs bootloader detected." APP_LOG_NL);
          app_log_info("Version: v%lu.%lu." APP_LOG_NL,
                       (btl_version & 0xFF000000) >> 24,
                       (btl_version & 0x00FF0000) >> 16);
          app_log_info("Slot %d starts @ 0x%8.8lx, size %lu bytes ." APP_LOG_NL,
                       SL_BT_APP_OTA_DFU_USED_SLOT,
                       evt->evt_info.btl_storage.storage_start_addr,
                       slot_size);
          break;

        default:
          app_log_error("Unknown bootloader! Raw ID: 0x%x." APP_LOG_NL, btl_type);
          app_log_error("Application OTA function may not work. " \
                        "Please use a Silicon Labs Gecko bootloader." APP_LOG_NL);
          break;
      }
      break;

    case SL_BT_APP_OTA_DFU_EVT_STATE_CHANGE_ID:
      // Get application OTA DFU state machine status.
      app_ota_dfu_status = evt->evt_info.sts.status;
      // Process state transitions, display information to the user or start/stop timers for download, etc.
      app_ota_dfu_on_status_change(app_ota_dfu_status,
                                   evt->evt_info.sts.prev_status,
                                   evt->ota_error_code,
                                   evt->btl_api_retval);
      break;

    case SL_BT_APP_OTA_DFU_EVT_DOWNLOAD_PACKET_ID:
      // Get download information.
      datablock_idx += 1;
      write_position = evt->evt_info.download_packet.write_image_position;
      break;

    case SL_BT_APP_OTA_DFU_EVT_VERIFY_IMAGE_ID:
      // Get verification information.
      datablock_idx += 1;
      verify_position = evt->evt_info.verified_bytes;
      break;
  }
}

/**************************************************************************//**
 * Private function to handle the Application OTA DFU states.
 * @param[in] curr_sts Current state.
 * @param[in] prev_sts Previous state for error handling.
 * @param[in] app_ota_dfu_error_code Application OTA DFU error code.
 * @param[in] boot_api_err_code Bootloader error code.
 *****************************************************************************/
static void app_ota_dfu_on_status_change(sl_bt_app_ota_dfu_status_t curr_sts,
                                         sl_bt_app_ota_dfu_status_t prev_sts,
                                         sl_bt_app_ota_dfu_error_t app_ota_dfu_error_code,
                                         int32_t boot_api_err_code)
{
  sl_status_t sc;

  switch (curr_sts) {
    case SL_BT_APP_OTA_DFU_DISCONNECT:
      // Indicates that disconnect from current device initiated.
      // Happens in case of write request or transfer issues.
      app_log_error("Disconnected by the target device." APP_LOG_NL);
      break;

    case SL_BT_APP_OTA_DFU_DOWNLOAD_BEGIN:
      datablock_idx = 0;
      time_elapsed = 0;
      app_log_info("Download started." APP_LOG_NL);
      sc = app_timer_start(&timer_ota,
                           DOWNLOAD_IND_FREQ,
                           timer_cb,
                           NULL,
                           true);
      app_assert_status(sc);
      break;

    case SL_BT_APP_OTA_DFU_DOWNLOAD_END:
      sc = app_timer_stop(&timer_ota);
      time_elapsed = 0;
      app_assert_status(sc);
      app_log_info("Download finished. Received %lu bytes." APP_LOG_NL, write_position);
      app_log_info("Press END button in Simplicity Connect app!" APP_LOG_NL);
      break;

    case SL_BT_APP_OTA_DFU_VERIFY:
      datablock_idx = 0u;
      time_elapsed = 0;
      app_log_info("Verify downloaded image..." APP_LOG_NL);
      sc = app_timer_start(&timer_ota,
                           VERIFICATION_IND_FREQ,
                           timer_cb,
                           NULL,
                           true);
      break;

    case SL_BT_APP_OTA_DFU_FINALIZE:
      sc = app_timer_stop(&timer_ota);
      app_log_info("Verified %u%% of the new image." APP_LOG_NL,
                   GET_DATA_PERCENTAGE(write_position, verify_position));
      app_log_info("Set image to bootload." APP_LOG_NL);
      break;

    case SL_BT_APP_OTA_DFU_WAIT_FOR_REBOOT:
      app_log_info("Rebooting..." APP_LOG_NL);
      sl_bt_app_ota_dfu_reboot();
      break;

    case SL_BT_APP_OTA_DFU_ERROR:
      // In case the error handler reached, progress timer has to be stopped.
      sc = app_timer_stop(&timer_ota);
      time_elapsed = 0;
      app_log_error("App OTA DFU Error. Last state before error: [%d]. " \
                    "Application OTA error code: [%d]. Bootloader error code: [%ld]. " APP_LOG_NL,
                    prev_sts,
                    app_ota_dfu_error_code,
                    boot_api_err_code);
      break;

    default:
      app_log_info("app_ota_dfu status [%d] -> [%d]" APP_LOG_NL, prev_sts, curr_sts);
      break;
  }
}
#endif // SL_CATALOG_APP_OTA_DFU_PRESENT
