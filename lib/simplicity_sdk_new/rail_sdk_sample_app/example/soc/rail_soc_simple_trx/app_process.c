/***************************************************************************//**
 * @file
 * @brief app_tick.c
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
#include "sl_component_catalog.h"
#include "sl_rail_sdk_simple_assistance.h"
#include "sl_rail.h"
#include "app_process.h"
#include "sl_rail_util_init.h"
#include "sl_simple_button_instances.h"
#include "sl_rail_sdk_packet_assistant.h"
#include "sl_rail_sdk_fifo_size_config.h"
#include "sl_rail_sdk_channel_selector.h"
#include "sl_code_classification.h"

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "app_task_init.h"
#endif

#include "cmsis_compiler.h"

#if defined(SL_CATALOG_RAIL_SIMPLE_CPC_PRESENT)
#include "sl_rail_sdk_simple_cpc.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/// Transmit data length
#define TX_PAYLOAD_LENGTH (16U)

/// RX buffer length
#define RX_BUFFER_LENGTH (16U)

/// State machine of simple_trx
typedef enum {
  S_PACKET_RECEIVED,
  S_PACKET_SENT,
  S_RX_PACKET_ERROR,
  S_TX_PACKET_ERROR,
  S_CALIBRATION_ERROR,
  S_IDLE,
} state_t;

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// Flag, indicating transmit request (button was pressed / CLI transmit request has occurred)
volatile bool tx_requested = false;
/// Flag, indicating received packet is forwarded on CLI or not
volatile bool rx_requested = true;

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
/// The variable shows the actual state of the state machine
static volatile state_t state = S_IDLE;

/// Contains the last RAIL Rx/Tx error events
static volatile uint64_t error_code = 0;

/// Contains the status of RAIL Calibration
static volatile sl_rail_status_t calibration_status = 0;

/// Transmit packet
static uint8_t out_packet[TX_PAYLOAD_LENGTH] = {
  0x0F, 0x16, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
  0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE,
};

/// RX buffer
static uint8_t rx_buffer[RX_BUFFER_LENGTH];

/// Flags to update state machine from interrupt
static volatile bool packet_received = false;
static volatile bool packet_sent = false;
static volatile bool rx_error = false;
static volatile bool tx_error = false;
static volatile bool cal_error = false;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/******************************************************************************
 * Application state machine, called infinitely
 *****************************************************************************/
void app_process_action(void)
{
  // Get RAIL handle, used later by the application
  sl_rail_handle_t rail_handle = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);
  // RAIL Rx packet handles
  sl_rail_rx_packet_handle_t rx_packet_handle;
  sl_rail_rx_packet_info_t packet_info;
  // Status indicator of the RAIL API calls
  sl_rail_status_t rail_status = SL_RAIL_STATUS_NO_ERROR;
  #if defined(SL_CATALOG_APP_LOG_PRESENT)
  sl_rail_status_t calibration_status_buff = SL_RAIL_STATUS_NO_ERROR;
  #endif
  #if defined(SL_CATALOG_RAIL_SIMPLE_CPC_PRESENT)
  uint8_t success_sent = 0x01;
  #endif

  if (packet_received) {
    packet_received = false;
    state = S_PACKET_RECEIVED;
  } else if (packet_sent) {
    packet_sent = false;
    state = S_PACKET_SENT;
  } else if (rx_error) {
    rx_error = false;
    state = S_RX_PACKET_ERROR;
  } else if (tx_error) {
    tx_error = false;
    state = S_TX_PACKET_ERROR;
  } else if (cal_error) {
    cal_error = false;
    state = S_CALIBRATION_ERROR;
  }

  switch (state) {
    case S_PACKET_RECEIVED:
      // Packet received:
      //  - Check whether sl_rail_hold_rx_packet() was successful, i.e. packet handle is valid
      //  - Copy it to the application FIFO
      //  - Free up the radio FIFO
      //  - Return to app IDLE state (RAIL will automatically switch back to Rx radio state)
      rx_packet_handle = sl_rail_get_rx_packet_info(rail_handle, SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_info);
      while (rx_packet_handle != SL_RAIL_RX_PACKET_HANDLE_INVALID) {
        uint8_t *start_of_packet = 0;
        uint16_t packet_size = unpack_packet(rail_handle, rx_buffer, &packet_info, &start_of_packet);
        rail_status = sl_rail_release_rx_packet(rail_handle, rx_packet_handle);
        if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
          app_log_warning("sl_rail_release_rx_packet() result: %lu\n", rail_status);
        }
        if (rx_requested) {
          printf_rx_packet(start_of_packet, packet_size);
#if defined(SL_CATALOG_RAIL_SIMPLE_CPC_PRESENT)
          sl_rail_sdk_simple_cpc_transmit(packet_size, start_of_packet);
#endif
        }
        toggle_receive_led();
        rx_packet_handle = sl_rail_get_rx_packet_info(rail_handle, SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_info);
      }
      state = S_IDLE;
      break;
    case S_PACKET_SENT:
      app_log_info("Packet has been sent\n");
#if defined(SL_CATALOG_RAIL_SIMPLE_CPC_PRESENT)
      sl_rail_sdk_simple_cpc_transmit(1, &success_sent);
#endif

      toggle_send_led();
      state = S_IDLE;
      break;
    case S_RX_PACKET_ERROR:
      // Handle Rx error
      app_log_error("Radio RX Error occurred\nEvents: %llX\n", error_code);
      state = S_IDLE;
      break;
    case S_TX_PACKET_ERROR:
      // Handle Tx error
      app_log_error("Radio TX Error occurred\nEvents: %llX\n", error_code);
      state = S_IDLE;
      break;
    case S_IDLE:
      if (tx_requested) {
        prepare_packet(rail_handle, out_packet, sizeof(out_packet));
        rail_status = sl_rail_start_tx(rail_handle, get_selected_channel(), SL_RAIL_TX_OPTIONS_DEFAULT, NULL);
        if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
          app_log_warning("sl_rail_start_tx() result: %lu\n ", rail_status);
        }
        tx_requested = false;
      }
      break;
    case S_CALIBRATION_ERROR:
      #if defined(SL_CATALOG_APP_LOG_PRESENT)
      calibration_status_buff = calibration_status;
      app_log_error("Radio Calibration Error occurred\nEvents: %llX\nRAIL_Calibrate() result:%ld\n",
                    error_code,
                    calibration_status_buff);
      #endif
      state = S_IDLE;
      break;
    default:
      // Unexpected state
      app_log_error("Unexpected Simple TRX state occurred:%d\n", state);
      break;
  }
}

/******************************************************************************
 * RAIL callback, called if a RAIL event occurs.
 *****************************************************************************/
SL_CODE_RAM void sl_rail_util_on_event(sl_rail_handle_t rail_handle, sl_rail_events_t events)
{
  error_code = events;
  // Handle Rx events
  if ( events & SL_RAIL_EVENTS_RX_COMPLETION ) {
    if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
      // Keep the packet in the radio buffer, download it later at the state machine
      sl_rail_hold_rx_packet(rail_handle);
      packet_received = true;
    } else {
      // Handle Rx error
      rx_error = true;
    }
  }
  // Handle Tx events
  if ( events & SL_RAIL_EVENTS_TX_COMPLETION) {
    if (events & SL_RAIL_EVENT_TX_PACKET_SENT) {
      packet_sent = true;
    } else {
      // Handle Tx error
      tx_error = true;
    }
  }

  // Perform all calibrations when needed
  if ( events & SL_RAIL_EVENT_CAL_NEEDED ) {
    calibration_status = sl_rail_calibrate(rail_handle, NULL, SL_RAIL_CAL_ALL_PENDING);
    if (calibration_status != SL_RAIL_STATUS_NO_ERROR) {
      cal_error = true;
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

#if defined(SL_CATALOG_RAIL_SIMPLE_CPC_PRESENT)
void sl_rail_sdk_simple_cpc_receive_cb(sl_status_t status, uint32_t len, uint8_t *data)
{
  //SL_STATUS_OK
  if (status == SL_STATUS_OK) {
    if (len == 1) {
      if (data[0] == 0x01 || data[0] == '1') {
        tx_requested = true;
      }
      if (data[0] == 0x00 || data[0] == '0') {
        rx_requested = !rx_requested;
      }
    }
  }
}
#endif
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
