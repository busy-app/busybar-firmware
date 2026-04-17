/***************************************************************************/ /**
 * @file  sl_wifi_callback_framework.c
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

#include "sl_wifi_callback_framework.h"
#include "sli_wifi_callback_framework.h"
#include "sl_si91x_host_interface.h"
#include "sl_constants.h"
#include "sl_si91x_core_utilities.h"
#include "sl_si91x_driver.h"
#include "sl_wifi.h"
#include "sli_wifi.h"
#include "sli_wifi_constants.h"
#include "sli_wifi_utility.h"

/******************************************************
 *               Function Declarations
 ******************************************************/
static sl_status_t sli_callback_with_status(sli_wifi_callback_entry_t *entry,
                                            sl_wifi_event_t event,
                                            sl_status_t status,
                                            void *data,
                                            uint32_t length);

static sl_status_t sli_handle_command_engine_status_event(sli_wifi_callback_entry_t *entry, sl_wifi_buffer_t *buffer);

static sl_status_t sli_handle_failed_event(sli_wifi_callback_entry_t *entry,
                                           sl_wifi_event_t event,
                                           const sl_wifi_system_packet_t *packet);

static sl_status_t sli_handle_tx_transceiver_event(sli_wifi_callback_entry_t *entry,
                                                   const sl_wifi_system_packet_t *packet);

static sl_status_t sli_handle_rx_transceiver_event(sli_wifi_callback_entry_t *entry, sl_wifi_system_packet_t *packet);

/******************************************************
 *               Function Definitions
 ******************************************************/
sl_status_t sl_wifi_set_callback(sl_wifi_event_group_t group, sl_wifi_callback_function_t function, void *optional_arg)
{
  return sli_wifi_set_callback(group, function, NULL, optional_arg);
}

sl_status_t sl_wifi_set_callback_v2(sl_wifi_event_group_t group,
                                    sl_wifi_callback_function_v2_t function,
                                    void *optional_arg)
{
  return sli_wifi_set_callback(group, NULL, function, optional_arg);
}

// Helper function to call callback with status
static sl_status_t sli_callback_with_status(sli_wifi_callback_entry_t *entry,
                                            sl_wifi_event_t event,
                                            sl_status_t status,
                                            void *data,
                                            uint32_t length)
{
  if (entry->function_v2) {
    return entry->function_v2(event, status, data, length, entry->arg);
  }

  sl_status_t *status_pointer = (sl_status_t *)malloc(sizeof(status));
  if (status_pointer == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  *status_pointer = status;

  sl_status_t result = entry->function(event, status_pointer, length, entry->arg);
  free(status_pointer);
  return result;
}

// Helper function to handle command engine status events
static sl_status_t sli_handle_command_engine_status_event(sli_wifi_callback_entry_t *entry, sl_wifi_buffer_t *buffer)
{
  sl_status_t status = *(sl_status_t *)buffer;
  return sli_callback_with_status(entry, SL_WIFI_COMMAND_ENGINE_STATUS_EVENT, status, NULL, 0);
}

// Helper function to handle failed events
static sl_status_t sli_handle_failed_event(sli_wifi_callback_entry_t *entry,
                                           sl_wifi_event_t event,
                                           const sl_wifi_system_packet_t *packet)
{
  sl_status_t status = sli_convert_and_save_firmware_status(sli_get_si91x_frame_status(packet));

  if (packet->command == SLI_WIFI_RSP_JOIN) {
    sl_status_t temp_status = sli_si91x_driver_send_command(SLI_WIFI_REQ_INIT,
                                                            SLI_WIFI_WLAN_CMD,
                                                            NULL,
                                                            0,
                                                            SLI_WIFI_RSP_INIT_WAIT_TIME,
                                                            NULL,
                                                            NULL);
    VERIFY_STATUS_AND_RETURN(temp_status);
  }

  return sli_callback_with_status(entry, event, status, NULL, 0);
}

// Helper function to handle TX transceiver events
static sl_status_t sli_handle_tx_transceiver_event(sli_wifi_callback_entry_t *entry,
                                                   const sl_wifi_system_packet_t *packet)
{
  sl_wifi_transceiver_tx_data_confirmation_t *tx_cfm_cb_data =
    (sl_wifi_transceiver_tx_data_confirmation_t *)malloc(sizeof(sl_wifi_transceiver_tx_data_confirmation_t));
  if (tx_cfm_cb_data == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  memset(tx_cfm_cb_data, 0, sizeof(sl_wifi_transceiver_tx_data_confirmation_t));
  tx_cfm_cb_data->status   = packet->desc[15];
  tx_cfm_cb_data->rate     = packet->data[0];          //Extended descriptor in data[] for rate
  tx_cfm_cb_data->priority = packet->data[4];          //Extended descriptor in data[] for priority
  memcpy(&tx_cfm_cb_data->token, &packet->data[8], 4); //Extended descriptor in data[] for token

  if (entry->function_v2) {
    return entry->function_v2(SL_WIFI_TRANSCEIVER_TX_DATA_STATUS_CB, SL_STATUS_OK, tx_cfm_cb_data, 0, entry->arg);
  }
  return entry->function(SL_WIFI_TRANSCEIVER_TX_DATA_STATUS_CB, tx_cfm_cb_data, 0, entry->arg);
}

// Helper function to handle RX transceiver events
static sl_status_t sli_handle_rx_transceiver_event(sli_wifi_callback_entry_t *entry, sl_wifi_system_packet_t *packet)
{
  sl_wifi_transceiver_rx_data_t *rx_cb_data =
    (sl_wifi_transceiver_rx_data_t *)malloc(sizeof(sl_wifi_transceiver_rx_data_t));
  if (rx_cb_data == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  memset(rx_cb_data, 0, sizeof(sl_wifi_transceiver_rx_data_t));
  uint16_t payload_offset = packet->desc[4];
  uint16_t payload_length = packet->length & 0xFFF;
  uint32_t status         = *(uint32_t *)(&packet->data[12]);

  rx_cb_data->length = payload_length - payload_offset;
  rx_cb_data->buffer = packet->data + payload_offset;
  rx_cb_data->rssi   = *(int8_t *)(&packet->data[0]);   //Extended descriptor in data[] for rssi
  rx_cb_data->rate   = *(uint16_t *)(&packet->data[2]); //Extended descriptor in data[] for rate

  /*
   * SL_STATUS_UNKNOWN_PEER - If SL_WIFI_FEAT_TRANSCEIVER_MAC_PEER_DS_SUPPORT is enabled but
   * data packet is received from a peer not present in MAC layer.
   */
  rx_cb_data->status = ((status & TRANSCEIVER_RX_PKT_TA_MATCH_BIT) || IS_BCAST_MCAST_MAC(rx_cb_data->buffer[4]))
                         ? SL_STATUS_OK
                         : SL_STATUS_UNKNOWN_PEER;

  if (entry->function_v2) {
    return entry->function_v2(SL_WIFI_TRANSCEIVER_RX_DATA_RECEIVE_CB, SL_STATUS_OK, rx_cb_data, 0, entry->arg);
  }
  return entry->function(SL_WIFI_TRANSCEIVER_RX_DATA_RECEIVE_CB, rx_cb_data, 0, entry->arg);
}

sl_status_t sl_wifi_default_event_handler(sl_wifi_event_t event, sl_wifi_buffer_t *buffer)
{
  sli_wifi_callback_entry_t *entry = sli_get_callback_entry((sl_wifi_event_group_t)event);

  // Verify there is a entry registered, if not return immediately
  if (entry == NULL) {
    return SL_STATUS_OK;
  }

  // Verify there is a callback registered, if not return immediately
  if (entry->function == NULL && entry->function_v2 == NULL) {
    return SL_STATUS_OK;
  }

  if (event == SL_WIFI_COMMAND_ENGINE_STATUS_EVENT) {
    return sli_handle_command_engine_status_event(entry, buffer);
  }

  // Start processing the event
  sl_wifi_system_packet_t *packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return sli_handle_failed_event(entry, event, packet);
  }

  // Handle TX transceiver events
  if (event == SL_WIFI_TRANSCEIVER_TX_DATA_STATUS_CB) {
    return sli_handle_tx_transceiver_event(entry, packet);
  }

  // Handle RX transceiver events
  if (event == SL_WIFI_TRANSCEIVER_RX_DATA_RECEIVE_CB) {
    return sli_handle_rx_transceiver_event(entry, packet);
  }

  // Handle other events
  void *data      = packet->length ? packet->data : NULL;
  uint32_t length = packet->length;

  // For extended scan completion, payload is empty; pass complete data size in bytes (count * sizeof)
  if ((event & ~SL_WIFI_EVENT_FAIL_INDICATION) == SL_WIFI_SCAN_RESULT_EVENT && length == 0) {
    uint16_t scan_count = 0;
    if (sli_wifi_get_stored_scan_result_count(sli_wifi_get_default_interface(), &scan_count) == SL_STATUS_OK) {
      length = (uint32_t)scan_count * sizeof(sl_wifi_extended_scan_result_t);
    }
  }

  if (entry->function_v2) {
    return entry->function_v2(event, SL_STATUS_OK, data, length, entry->arg);
  }
  return entry->function(event, data, length, entry->arg);
}
