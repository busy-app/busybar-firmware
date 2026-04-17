/***************************************************************************//**
 * @file
 * @brief app_process.c
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include "em_device.h"
#if defined _SILICON_LABS_32B_SERIES_2
#include "em_system.h"
#else
#include "sl_hal_system.h"
#endif
#include "sl_component_catalog.h"
#include "app_process.h"
#include "sl_rail_sdk_simple_assistance.h"
#include "sl_rail.h"
#include "sl_simple_button_instances.h"
#include "sl_rail_sdk_packet_asm.h"
#ifdef SL_CATALOG_RAIL_SDK_IEEE802154_SUPPORT_PRESENT
  #include "sl_rail_sdk_util_802154_init.h"
  #include "sl_rail_sdk_util_802154_protocol_types.h"
  #include "sl_rail_sdk_util_802154_init_config.h"
  #include "sl_rail_sdk_ieee802154_config.h"
  #include "sl_rail_sdk_ieee802154_support.h"
#elif defined SL_CATALOG_RAIL_SDK_BLE_SUPPORT_PRESENT
  #include "sl_rail_sdk_util_ble_init.h"
  #include "sl_rail_sdk_util_ble_protocol.h"
  #include "sl_rail_sdk_util_ble_protocol_config.h"
  #include "sl_rail_sdk_util_ble_init_config.h"
#else
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "app_task_init.h"
#endif

#include "cmsis_compiler.h"
#include "sl_rail_sdk_fifo_size_config.h"
#include "sl_code_classification.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/// Size of sending data
#define RAIL_PAYLOAD_DATA_SIZE            (17U)
#ifdef SL_CATALOG_RAIL_SDK_BLE_SUPPORT_PRESENT
/// BLE channel number
  #define BLE_CHANNEL ((uint8_t) 0)
#endif

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * Starts listening for radio packets.
 *
 * @param[in] rail_handle     Handle to the RAIL context
 *****************************************************************************/
static void start_receiving(sl_rail_handle_t rail_handle);

/**************************************************************************//**
 * Transmits the data packet.
 *
 * @param[in] rail_handle Handle to the RAIL context
 * @param[in] packet      The packet that is desired to send
 * @param[in] rail_handle The size of the packet
 *****************************************************************************/
static void handle_transmit(sl_rail_handle_t rail_handle);

/**************************************************************************//**
 * Checks the received packet (data or ACK).
 *
 * @param[in] rail_handle     Handle to the RAIL context
 * @return sl_rail_rx_packet_handle_t
 *****************************************************************************/
static void handle_receive(sl_rail_handle_t rail_handle);

/**************************************************************************//**
 * Handle errors detected in RAIL events.
 *****************************************************************************/
static void handle_error_state(void);

#ifdef SL_CATALOG_RAIL_SDK_BLE_SUPPORT_PRESENT
/**************************************************************************//**
 * Print BLE packet
 *
 * @param[in] sl_rail_sdk_ble_advertising_packet_t  BLE packet pointer
 *****************************************************************************/
static void printf_ble_packet(const sl_rail_sdk_ble_advertising_packet_t *packet);

/**************************************************************************//**
 * Print BLE received data
 *
 * @param[in] rx_buffer  BLE received data pointer
 * @param[in] length  length of BLE received data
 *****************************************************************************/
static void printf_ble_recv_payload(const uint8_t * const rx_buffer, uint16_t length);
#endif

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// Flag, indicating transmit request (button was pressed
/// / CLI transmit request has occurred)
volatile bool tx_requested = false;
/// Flag, indicating received packet is forwarded on CLI or not
volatile bool rx_requested = true;
/// CLI requests comes via CLI
#ifdef SL_CATALOG_RAIL_SDK_IEEE802154_SUPPORT_PRESENT
volatile sl_rail_sdk_ieee802154_cli_requests cli_requests = { 0 };
/// CLI requests' desired settings/values for changing the application
volatile sl_rail_sdk_ieee802154_cli_data cli_desired_settings;
/// Status of the application
sl_rail_sdk_ieee802154_status_t comm_status = {
  .ack = false,
  .auto_ack = false,
  .crc_length = 0,
  .channel = 0,
  .data_whitening = false,
  .destination_address = 0,
  .destination_pan_id = 0,
  .source_address = 0,
  .std = (sl_rail_sdk_ieee802154_std_t)SL_RAIL_SDK_UTIL_INIT_PROTOCOL_INSTANCE_DEFAULT
};
#endif
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
#ifdef SL_CATALOG_RAIL_SDK_IEEE802154_SUPPORT_PRESENT
/// IEEE 802.15.4 frame structure for RX direction
static sl_rail_sdk_ieee802154_frame_t rx_frame = { 0 };
/// IEEE 802.15.4 frame structure for TX direction
static sl_rail_sdk_ieee802154_frame_t tx_frame = {
  .mhr_config = {
    .frame_control          = DEFAULT_FCF_FIELD,
    .sequence_number        = 0U,
    .destination_pan_id     = DEFAULT_BROADCAST_PAN_ID,
    .destination_address    = DEFAULT_BROADCAST_DEST_ADDR,
    .source_address         = DEFAULT_BROADCAST_SRC_ADDR
  },
  .phr_config = SL_RAIL_SDK_IEEE802154G_PHR_MODE_SWITCH_OFF
                | SL_RAIL_SDK_IEEE802154G_PHR_CRC_2_BYTE
                | APP_WHITENING_ON_OFF,
};
#endif

/// The variable shows the actual state of the state machine
static volatile state_t state = S_IDLE;
/// Contains the status of RAIL Calibration
static volatile sl_rail_status_t calibration_status = 0;
/// RAIL Rx packet handle
// static volatile sl_rail_rx_packet_handle_t rx_packet_handle;

/// Receive app buffer
static __ALIGNED(RAIL_FIFO_ALIGNMENT) uint8_t rx_app_buff[SL_RAIL_SDK_RX_FIFO_SIZE];
/// Transmit app buffer
static __ALIGNED(RAIL_FIFO_ALIGNMENT) uint8_t tx_app_buff[SL_RAIL_SDK_TX_FIFO_SIZE];

/// Sending data (payload)
static const __ALIGNED(RAIL_FIFO_ALIGNMENT) uint8_t tx_payload_data[RAIL_PAYLOAD_DATA_SIZE] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
};
/// Notify RAIL Tx, Rx calibration error
static bool rail_error = false;
/// Copy of last RAIL events to process
static sl_rail_events_t rail_last_state = SL_RAIL_EVENTS_NONE;
/// Notify end of packet transmission
static bool rail_packet_sent = false;
/// Notify reception of packet
static bool rail_packet_received = false;
/// Request start receiving
static bool start_rx = true;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/******************************************************************************
 * Application state machine, called infinitely
 *****************************************************************************/
void app_process_action(void)
{
  // Get RAIL handle, used later by the application
  sl_rail_handle_t rail_handle = sl_rail_sdk_util_get_handle();

  // Handle errors if pending
  if (rail_error) {
    rail_error = false;
    state = S_ERROR;
  }

  switch (state) {
    case S_INIT:
      /* IEEE 802.15.4 Application init*/
#ifdef SL_CATALOG_RAIL_SDK_IEEE802154_SUPPORT_PRESENT
      // initializes the IEEE 802154 according to desired changes;
      // updates the status
      sl_rail_sdk_ieee802154_request_manager(rail_handle, &tx_frame, &cli_requests,
                                             &cli_desired_settings);
      // updates the status for changes
      sl_rail_sdk_ieee802154_update_status(&comm_status, &tx_frame);
#endif
      state = S_IDLE;
      start_rx = true;
#if defined(SL_CATALOG_KERNEL_PRESENT)
      app_task_notify();
#endif
      break;

    case S_IDLE:
      if (start_rx) {
        start_receiving(rail_handle);
        start_rx = false;
      }

      if (tx_requested) {
        state = S_TRANSMIT;
        tx_requested = false;
#if defined(SL_CATALOG_KERNEL_PRESENT)
        app_task_notify();
#endif
      }

      if (rail_packet_sent) {
        rail_packet_sent = false;
        app_log_info("Packet has been sent\n");
        // toggle when the TX packet sent
        toggle_send_led();
      }

      if (rail_packet_received) {
        rail_packet_received = false;
        state = S_RECEIVE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
        app_task_notify();
#endif
      }

#ifdef SL_CATALOG_RAIL_SDK_IEEE802154_SUPPORT_PRESENT
      // if any change comes, it goes into S_INIT state and performs the changes
      if (sl_rail_sdk_ieee802154_is_change_requested(&cli_requests)) {
        sl_rail_idle(rail_handle, SL_RAIL_IDLE_ABORT, true);
        state = S_INIT;
#if defined(SL_CATALOG_KERNEL_PRESENT)
        app_task_notify();
#endif
      }
#endif
      break;

    case S_TRANSMIT:
      // sends the packet for IEEE 802.15.4 and BLE standards
      handle_transmit(rail_handle);
      tx_requested = false;
      state = S_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
      app_task_notify();
#endif
      break;

    case S_RECEIVE:
      // receive the packet for IEEE 802.15.4 and BLE standards
      handle_receive(rail_handle);
      toggle_receive_led();
      start_rx = true;
      state = S_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
      app_task_notify();
#endif
      break;

    case S_ERROR:
      // calls the error handle to print the reason and id of the error.
      handle_error_state();
      state = S_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
      app_task_notify();
#endif
      break;

    default:
      // Unexpected state
      app_log_error("Unexpected Simple TRX state occurred:%d\n", state);
      break;
  }
}

void app_process_init(sl_rail_handle_t rail_handle)
{
#ifdef SL_CATALOG_RAIL_SDK_IEEE802154_SUPPORT_PRESENT
  sl_rail_sdk_ieee802154_protocol_init(rail_handle, SL_RAIL_SDK_UTIL_INIT_PROTOCOL_INSTANCE_DEFAULT);
  // updates the status for changes
  sl_rail_sdk_ieee802154_update_status(&comm_status, &tx_frame);
#else
  (void)rail_handle;
#endif
}

/******************************************************************************
 * RAIL callback, called if any RAIL event occurs.
 *****************************************************************************/
SL_CODE_RAM void sl_rail_util_on_event(sl_rail_handle_t rail_handle, sl_rail_events_t events)
{
  // Make a copy of the events
  rail_last_state = events;

  // Handle Rx events
  if ( events & SL_RAIL_EVENTS_RX_COMPLETION ) {
    if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
      // Keep the packet in the radio buffer,
      // download it later at the state machine
      (void)sl_rail_hold_rx_packet(rail_handle);
      rail_packet_received = true;
    } else {
      rail_error = true;
    }
  }
  // Handle Tx events
  if ( events & SL_RAIL_EVENTS_TX_COMPLETION) {
    if (events & SL_RAIL_EVENT_TX_PACKET_SENT) {
      rail_packet_sent = true;
    } else {
      rail_error = true;
    }
  }

  if (events & SL_RAIL_EVENTS_TXACK_COMPLETION ) {
    if (events & SL_RAIL_EVENT_TXACK_PACKET_SENT) {
    } else {
      rail_error = true;
    }
  }

  // Perform all calibrations when needed
  if ( events & SL_RAIL_EVENT_CAL_NEEDED ) {
    calibration_status = sl_rail_calibrate(rail_handle, NULL, SL_RAIL_CAL_ALL_PENDING);
    if (calibration_status != SL_RAIL_STATUS_NO_ERROR) {
      rail_error = true;
    }
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

/******************************************************************************
 * Button callback, called if any button is pressed or released.
 *****************************************************************************/
SL_CODE_RAM void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    tx_requested = true;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/*******************************************************************************
 * Use RAIL to transmit a data packet
 ******************************************************************************/
static void handle_transmit(sl_rail_handle_t rail_handle)
{
  sl_rail_status_t status;
#ifdef SL_CATALOG_RAIL_SDK_BLE_SUPPORT_PRESENT
  sl_rail_sdk_ble_advertising_packet_t *ble_send_packet;
#endif
  uint16_t packet_size;

#ifdef SL_CATALOG_RAIL_SDK_IEEE802154_SUPPORT_PRESENT
  // Prepare packet
  sl_rail_sdk_ieee802154_prepare_sending(&tx_frame,
                                         (uint8_t *)tx_payload_data,
                                         sizeof(tx_payload_data));
  // packs the data frame using the parameter information and the packed
  // frame is copied into the fifo
  (void)sl_rail_sdk_ieee802154_pack_data_frame(sl_rail_sdk_ieee802154_get_std(),
                                               &tx_frame,
                                               &packet_size,
                                               tx_app_buff);

  // Send packet
  // sets the tx options based on the current ACK settings (auto-ACK enabled?)
  status = sl_rail_sdk_ieee802154_transmission(rail_handle, tx_app_buff, packet_size);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_warning("sl_rail_sdk_ieee802154_transmission() status: %lu\n", status);
  }

#elif defined SL_CATALOG_RAIL_SDK_BLE_SUPPORT_PRESENT
  // Prepare packet
  // set ble_send_packet pointer to tx app buff
  ble_send_packet = sl_rail_sdk_ble_get_packet(tx_app_buff);
  sl_rail_sdk_ble_prepare_packet(ble_send_packet, tx_payload_data, sizeof(tx_payload_data));
  packet_size = sl_rail_sdk_ble_get_packet_size(ble_send_packet);
  printf_ble_packet(ble_send_packet);

  // Send Packet
  status = sl_rail_write_tx_fifo(rail_handle, tx_app_buff, packet_size, true);
  if (status != packet_size) {
    app_log_warning("BLE sl_rail_write_tx_fifo status: %lu\n", status);
  }
  status = sl_rail_start_tx(rail_handle, BLE_CHANNEL, SL_RAIL_TX_OPTIONS_DEFAULT, NULL);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_warning("BLE sl_rail_start_tx status: %lu\n", status);
  }
  #else
#endif
}

/*******************************************************************************
 * Use RAIL to start listening for radio packets
 ******************************************************************************/
static void start_receiving(sl_rail_handle_t rail_handle)
{
  // Status indicator of the RAIL API calls
  sl_rail_status_t rail_status = SL_RAIL_STATUS_NO_ERROR;

#ifdef SL_CATALOG_RAIL_SDK_IEEE802154_SUPPORT_PRESENT
  rail_status = sl_rail_start_rx(rail_handle, sl_rail_sdk_ieee802154_get_channel(), NULL);
#elif defined SL_CATALOG_RAIL_SDK_BLE_SUPPORT_PRESENT
  rail_status = sl_rail_start_rx(rail_handle, BLE_CHANNEL, NULL);
#else
#endif
  if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_warning("sl_rail_start_rx() result: %lu\n",
                    rail_status);
  }
}

/*******************************************************************************
 * Handle received packets in loop
 ******************************************************************************/
static void handle_receive(sl_rail_handle_t rail_handle)
{
  sl_rail_rx_packet_handle_t rx_packet_handle;
  sl_rail_rx_packet_details_t packet_details;
  sl_rail_rx_packet_info_t packet_info;
  sl_rail_status_t rail_status;
#ifdef SL_CATALOG_RAIL_SDK_IEEE802154_SUPPORT_PRESENT
  int16_t res = SL_RAIL_SDK_IEEE802154_ERROR;
#endif
#ifdef SL_CATALOG_RAIL_SDK_BLE_SUPPORT_PRESENT
  sl_rail_sdk_ble_advertising_packet_t *ble_recv_packet = NULL;
#endif

  do {
    rx_packet_handle = sl_rail_get_rx_packet_info(rail_handle, SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_info);

    if (rx_packet_handle == SL_RAIL_RX_PACKET_HANDLE_INVALID) {
      break;
    }

    (void)sl_rail_get_rx_packet_details(rail_handle, rx_packet_handle, &packet_details);
    // copies the data in the RX Buffer
    rail_status = sl_rail_copy_rx_packet(rail_handle, rx_app_buff, &packet_info);
    // after the copy of the packet, the RX packet can be release for RAIL
    rail_status = sl_rail_release_rx_packet(rail_handle, rx_packet_handle);
    if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
      app_log_warning("sl_rail_release_rx_packet() result: %lu\n", rail_status);
    }

#ifdef SL_CATALOG_RAIL_SDK_IEEE802154_SUPPORT_PRESENT
    // Is it a ACK or a data frame
    if (packet_details.is_ack) {
      app_log_info("ACK received\n");
      // prints the received ACK frame
      sl_rail_sdk_ieee802154_print_ack(sl_rail_sdk_ieee802154_get_std(), rx_app_buff);
    } else {
      // unpack the IEEE802154 frame
      res = sl_rail_sdk_ieee802154_unpack_data_frame(sl_rail_sdk_ieee802154_get_std(),
                                                     &rx_frame,
                                                     rx_app_buff,
                                                     SL_RAIL_SDK_RX_FIFO_SIZE);
      // prints the received data based on the standard
      if (rx_requested && (res == SL_RAIL_SDK_IEEE802154_OK)) {
        sl_rail_sdk_ieee802154_print_frame(sl_rail_sdk_ieee802154_get_std(), &rx_frame);
        app_log_info("\n");
      }
    }
#elif defined SL_CATALOG_RAIL_SDK_BLE_SUPPORT_PRESENT

    ble_recv_packet = sl_rail_sdk_ble_get_packet(rx_app_buff);
    if (ble_recv_packet == NULL) {
      if (ble_recv_packet == NULL) {
        app_log_warning("BLE received packet is NULL pointer");
      }
    } else {
      if (rx_requested) {
        printf_ble_packet(ble_recv_packet);
      }
    }

    if (rx_requested) {
      // Getting payload and print it
      printf_ble_recv_payload(sl_rail_sdk_ble_get_payload(ble_recv_packet),
                              sl_rail_sdk_ble_get_payload_len(ble_recv_packet));
    }
#endif
  } while (rx_packet_handle != SL_RAIL_RX_PACKET_HANDLE_INVALID);
}

/*******************************************************************************
 * This helper function handles the S_ERROR state of the state machine.
 ******************************************************************************/
static void handle_error_state(void)
{
  // Handle Rx error
  if (rail_last_state & SL_RAIL_EVENTS_RX_COMPLETION) {
    app_log_error("Radio RX Error occurred\nEvents: %lld\n", rail_last_state);
    // Handle Tx error
  } else if (rail_last_state & SL_RAIL_EVENTS_TX_COMPLETION) {
    app_log_error("Radio TX Error occurred\nEvents: %lld\n", rail_last_state);
    // Handle calibration error
  } else if (rail_last_state & RAIL_EVENT_CAL_NEEDED) {
    app_log_warning("Radio Calibr. Error occurred\nEvents: %lld\nsl_rail_calibrate() result:%ld\n",
                    rail_last_state,
                    calibration_status);
  } else if (rail_last_state & SL_RAIL_EVENTS_TXACK_COMPLETION) {
    app_log_error("ACK TX Error occurred\nEvents: %lld\n", rail_last_state);
  }
  start_rx = true;
}

#ifdef SL_CATALOG_RAIL_SDK_BLE_SUPPORT_PRESENT
/*******************************************************************************
 * Print BLE packet
 ******************************************************************************/
static void printf_ble_packet(const sl_rail_sdk_ble_advertising_packet_t *packet)
{
  uint32_t packet_size = sl_rail_sdk_ble_get_packet_size(packet);
  uint32_t payload_len = sl_rail_sdk_ble_get_payload_len(packet);
  uint8_t *packet_byte;
  app_log_info("BLE Packet: (size = %ld) (payload_len = %ld) {\n", packet_size, payload_len);
  for (uint32_t i = 0; i < packet_size; ++i) {
    packet_byte = (uint8_t *)packet + i;
    app_log_info("[%ld] -> 0x%02X\n", i, *packet_byte);
  }
  app_log_info("}\n");
}

/*******************************************************************************
 * Print BLE packet
 ******************************************************************************/
static void printf_ble_recv_payload(const uint8_t * const rx_buffer, uint16_t length)
{
  uint8_t i = 0;
  app_log_info("BLE Packet has been received. Payload (%d): ", length);
  for (i = 0; i < length; i++) {
    app_log_info("0x%02X, ", rx_buffer[i]);
  }
  app_log_info("\n");
}
#endif
