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

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/// Transmit data length
#define TX_PAYLOAD_LENGTH (16U)
/// RX buffer length
#define RX_BUFFER_LENGTH (256U)

/// States of the Auto-ACK app
typedef enum {
  S_IDLE,             //!< Idle state waiting for TX or RX
  S_TRANSMIT,         //!< CLI or button push request to TX a packet
  S_RECEIVE,          //!< Receiving a packet or an ACK for previous TX
  S_ERROR             //!< An error occurred
} state_t;

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * Starts listening for radio packets.
 *
 * @param[in] rail_handle Handle to the RAIL context
 *****************************************************************************/
static void start_receiving(sl_rail_handle_t rail_handle);

/**************************************************************************//**
 * Transmits the data packet.
 *
 * @param[in] rail_handle Handle to the RAIL context
 *****************************************************************************/
static void handle_packet_transmission(sl_rail_handle_t rail_handle);

/**************************************************************************//**
 * Check the received packet (data or ACK).
 *
 * @param[in] rail_handle Handle to the RAIL context
 *****************************************************************************/
static void handle_received_packet(sl_rail_handle_t rail_handle);

/**************************************************************************//**
 * Handle errors detected in RAIL events.
 *****************************************************************************/
static void handle_error_state(void);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// Flag, indicating transmit request (button was pressed / CLI transmit request has occurred)
/// Also used in the application state machine
volatile bool tx_requested = false;
/// Flag, indicating received packet is forwarded on CLI or not
volatile bool rx_requested = true;

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
/// The variable shows the actual state of the state machine
static volatile state_t state = S_IDLE;

/// Contains the status of RAIL Calibration
static volatile sl_rail_status_t calibration_status = 0;

/// RAIL Rx packet handle
static volatile sl_rail_rx_packet_handle_t rx_packet_handle = SL_RAIL_RX_PACKET_HANDLE_INVALID;

/// Transmit packet
static uint8_t out_packet[TX_PAYLOAD_LENGTH] = {
  0x0F, 0x16, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
  0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE,
};

/// RX buffer
static uint8_t rx_buffer[RX_BUFFER_LENGTH];

/// State machine flags and conditions
/// Notify end of packet transmission
static bool rail_packet_sent = false;

/// Notify reception of packet
static bool rail_packet_received = false;

/// Notify RAIL Tx or Rx error
static bool rail_error = false;

/// Request start receiving
static bool start_rx = true;

/// Copy of last RAIL events to process
static sl_rail_events_t rail_last_state = SL_RAIL_EVENTS_NONE;

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
  // Handle errors if pending
  if (rail_error) {
    rail_error = false;
    state = S_ERROR;
  }

  switch (state) {
    case S_IDLE:
      if (start_rx) {
        // Start receiving upon entering the state
        start_rx = false;
        start_receiving(rail_handle);
      } else if (rail_packet_received) {
        // Go to RECEIVE state
        rail_packet_received = false;
        state = S_RECEIVE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
        app_task_notify();
#endif
      } else if (tx_requested) {
        // Transition to TRANSMIT if requested
        state = S_TRANSMIT;
#if defined(SL_CATALOG_KERNEL_PRESENT)
        app_task_notify();
#endif
      }
      break;
    case S_TRANSMIT:
      // Send packet upon entering state
      if (tx_requested) {
        tx_requested = false;
        handle_packet_transmission(rail_handle);
      } else if (rail_packet_sent) {
        app_log_info("Packet has been sent\n");
        state = S_IDLE;
        rail_packet_sent = false;
#if defined(SL_CATALOG_KERNEL_PRESENT)
        app_task_notify();
#endif
      }
      break;
    case S_RECEIVE:
      handle_received_packet(rail_handle);
      // Request Rx when entering IDLE state
      start_rx = true;
      state = S_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
      app_task_notify();
#endif
      break;
    case S_ERROR:
      handle_error_state();
      break;
    default:
      // Unexpected state
      app_log_error("Unexpected state occurred:%d\n", state);
      break;
  }

  // Reset copy of RAIL events
  rail_last_state = SL_RAIL_EVENTS_NONE;
}

/******************************************************************************
 * RAIL callback, called if a RAIL event occurs.
 *****************************************************************************/
SL_CODE_RAM void sl_rail_util_on_event(sl_rail_handle_t rail_handle, sl_rail_events_t events)
{
  // Make a copy of the events
  rail_last_state = events;

  // Handle Tx events
  if ( events & SL_RAIL_EVENTS_TX_COMPLETION) {
    if (events & SL_RAIL_EVENT_TX_PACKET_SENT) {
      rail_packet_sent = true;
    } else {
      rail_error = true;
    }
  }

  // Handle Rx events
  if ( events & SL_RAIL_EVENTS_RX_COMPLETION ) {
    if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
      // Keep the packet in the radio buffer, download it later at the state machine
      rx_packet_handle = sl_rail_hold_rx_packet(rail_handle);
      rail_packet_received = true;
    } else {
      rail_error = true;
    }
  }

  // Perform all calibrations when needed or indicate error if failed
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
static void handle_packet_transmission(sl_rail_handle_t rail_handle)
{
  /// Status indicator of the RAIL API calls
  sl_rail_status_t rail_status;

  prepare_packet(rail_handle, out_packet, sizeof(out_packet));
  rail_status = sl_rail_start_tx(rail_handle, get_selected_channel(), SL_RAIL_TX_OPTION_WAIT_FOR_ACK, NULL);
  if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_warning("sl_rail_start_tx() result: %lu\n ", rail_status);
  }
}

/*******************************************************************************
 * Use RAIL to start listening for radio packets
 ******************************************************************************/
static void start_receiving(sl_rail_handle_t rail_handle)
{
  /// Status indicator of the RAIL API calls
  sl_rail_status_t rail_status;

  rail_status = sl_rail_start_rx(rail_handle, get_selected_channel(), NULL);
  if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_warning("sl_rail_start_rx() result: %lu\n", rail_status);
  }
}

/*******************************************************************************
 * Process the received packet (print data packet or indicate ACK)
 ******************************************************************************/
static void handle_received_packet(sl_rail_handle_t rail_handle)
{
  sl_rail_rx_packet_info_t packet_info;
  sl_rail_rx_packet_details_t packet_details;
  sl_rail_status_t packet_status;
  /// Status indicator of the RAIL API calls
  sl_rail_status_t rail_status;

  //  - Check whether sl_rail_hold_rx_packet() was successful, i.e. packet handle is valid
  //  - Copy it to the application FIFO
  //  - Free up the radio FIFO
  //  - Return to IDLE state i.e. RAIL Rx
  if (rx_packet_handle == SL_RAIL_RX_PACKET_HANDLE_INVALID) {
    app_log_error("sl_rail_hold_rx_packet() error: SL_RAIL_RX_PACKET_HANDLE_INVALID\n"
                  "No such RAIL rx packet yet exists or rail_handle is not active");
  }
  rx_packet_handle = sl_rail_get_rx_packet_info(rail_handle, SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_info);
  if (rx_packet_handle == SL_RAIL_RX_PACKET_HANDLE_INVALID) {
    app_log_error("sl_rail_get_rx_packet_info() error: SL_RAIL_RX_PACKET_HANDLE_INVALID\n");
  }
  if (rx_packet_handle != SL_RAIL_RX_PACKET_HANDLE_INVALID) {
    // Get packet details to identify ACK of last Tx
    packet_status =  sl_rail_get_rx_packet_details(rail_handle, rx_packet_handle, &packet_details);
    if (packet_status != SL_RAIL_STATUS_NO_ERROR) {
      app_log_error("sl_rail_get_rx_packet_details() error: %lu\n", packet_status);
    }

    uint8_t *start_of_packet = 0;
    uint16_t packet_size = 0;
    // Check the packet status if this RX is an ACK for our last TX
    if (packet_details.is_ack) {
      toggle_send_led();
    } else {
      toggle_receive_led();
      if (packet_info.packet_bytes > RX_BUFFER_LENGTH) {
        app_log_error("sl_rail_get_rx_packet_info() error: packet too long\n");
      } else {
        packet_size = unpack_packet(rail_handle, rx_buffer, &packet_info, &start_of_packet);
      }
    }
    rail_status = sl_rail_release_rx_packet(rail_handle, rx_packet_handle);
    if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
      app_log_warning("sl_rail_release_rx_packet() result: %lu\n", rail_status);
    }
    if (packet_details.is_ack) {
      app_log_info("ACK was received\n");
    } else if (rx_requested) {
      printf_rx_packet(start_of_packet, packet_size);
    }
  }
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
  } else if (rail_last_state & SL_RAIL_EVENT_CAL_NEEDED) {
    app_log_warning("Radio Calibration Error occurred\nEvents: %lld\nsl_rail_calibrate() result:%ld\n",
                    rail_last_state,
                    calibration_status);
  }
  start_rx = true;
  state = S_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}
