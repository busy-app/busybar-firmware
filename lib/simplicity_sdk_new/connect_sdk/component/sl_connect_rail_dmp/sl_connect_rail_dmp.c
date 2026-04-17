/***************************************************************************//**
 * @file sl_connect_rail_dmp.c
 * @brief Source file for the proprietary task initialization and handling in
 *        the Silicon Labs Connect RAIL DMP application.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_component_catalog.h"
#include "app_assert.h"
#include "app_log.h"
#include "rail_config.h"
#include "sl_connect_rail_dmp.h"
#include "sl_rail.h"
#include "sl_rail_util_init.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// Tx and Rx FIFO sizes
#define SL_RAIL_SDK_TX_FIFO_SIZE  256
#define SL_RAIL_SDK_RX_FIFO_SIZE  256

/// Transmit data length
#define TX_PAYLOAD_LENGTH (16U)

// Redefining DEF_TRUE in order to be able to test the infinite loop
#if defined(__SL_UNIT_TEST)
#if defined(DEF_TRUE)
#undef DEF_TRUE
#endif // DEF_TRUE
extern int num_loops;
#define DEF_TRUE num_loops--
#else
#define DEF_TRUE 1U
#endif // __SL_UNIT_TEST
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// Flags to update state machine from interrupt
static volatile bool packet_received = false;
static volatile bool packet_sent = false;
static volatile bool rx_error = false;
static volatile bool tx_error = false;

/// Scheduler status
static volatile sl_rail_scheduler_status_t scheduler_status;

/// Transmit packet
static uint8_t out_packet[TX_PAYLOAD_LENGTH] = {
  0x0F, 0x16, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
  0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE,
};

uint8_t rx_packet[16U] = { 0 };
uint16_t packet_size = 0;

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// Flag, indicating transmit request (button was pressed / CLI transmit request has occurred)
volatile bool tx_requested = false;
volatile bool start_rx = false;
volatile bool stop_rx = false;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/**
 * @brief Calculate the transaction time for a given RAIL handle.
 *
 * This function computes the transaction time based on the provided RAIL handle.
 *
 * @param railHandle The handle to the RAIL instance.
 * @return The calculated transaction time as a RAIL_Time_t value.
 */
sl_rail_time_t calculate_transaction_time(sl_rail_handle_t rail_handle)
{
  // Get current bitrate
  uint32_t bitrate = sl_rail_get_bit_rate(rail_handle);

  // Calculate packet size (adjust these values based on your configuration)
  uint32_t payloadLengthByte = 16;
  uint32_t preambleBit = 40;
  uint32_t syncWordBit = 16;
  uint32_t crcBit = 16;
  uint32_t totalPacketBit = preambleBit + syncWordBit + crcBit + (payloadLengthByte * 8);

  // Calculate transmission time in microseconds
  sl_rail_time_t txTime = (totalPacketBit * 1000000) / bitrate;

  return txTime;
}

#ifndef __SL_UNIT_TEST
/**
 * @brief Callback function triggered when the RAIL (Radio Abstraction Interface Layer) is ready.
 *
 * This function is invoked to perform additional setup or configuration when the RAIL is ready.
 * If the Bluetooth stack is present, it attempts to add a third state buffer to the RAIL multiprotocol library.
 * As Connect Stack uses the 3rd state buffer.
 *
 * @param rail_handle The handle to the RAIL instance.
 *
 * @note This function includes conditional compilation for Bluetooth presence. If the third state buffer
 *       cannot be added (e.g., it was already added or the RAIL multiprotocol library is not being used),
 *       an error is logged.
 *
 * @error Logs an error message if adding the third state buffer fails, including the error code.
 */
void sl_rail_util_on_rf_ready(sl_rail_handle_t rail_handle)
{
  #ifdef SL_CATALOG_BLUETOOTH_PRESENT
  sl_rail_status_t rail_status = SL_RAIL_STATUS_NO_ERROR;
  // Added 3rd buffer as only 2 buffer is supported by default
  rail_status = sl_rail_add_state_buffer_3(SL_RAIL_EFR32_HANDLE);
  if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_error("sl_rail_add_state_buffer_3 error, 3rd state buffer was previously added or this isn't the RAIL multiprotocol library. Error code: %lX", rail_status);
  }
  #endif
}
#endif

/**
 * @brief Task function for the rail application.
 *
 * This function serves as the entry point for the rail application task.
 *
 * @param p_arg Pointer to the argument passed to the task function.
 */
void rail_app_task(void *p_arg)
{
  (void)(p_arg);
  sl_rail_status_t rail_status = SL_RAIL_STATUS_NO_ERROR;

  static sl_rail_scheduler_info_t scheduler_info = {
    .priority = 100,
    .slip_time = 100000,
    .transaction_time = 2500,
  };

#ifndef __SL_UNIT_TEST
  for (uint8_t i = 0; i < 255; i++) {
    if (channelConfigs[i] == NULL) {
      if (i < 2) {
        app_assert(i > 1, "Create a rail protocol in radio configuration!\n");
      }
      break;
    }
  }
#endif

  sl_rail_handle_t rail_handle = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);

  // Set transaction time
  scheduler_info.transaction_time = calculate_transaction_time(rail_handle);

  while (DEF_TRUE) {
    if (tx_requested) {
      tx_requested = false;
      app_log_info("Send Packet\n");
      sl_rail_write_tx_fifo(rail_handle, out_packet, TX_PAYLOAD_LENGTH, true);
      scheduler_info.priority = SL_RAIL_SCHEDULER_PRIORITY_SEND;
      rail_status = sl_rail_start_tx(rail_handle, 0, SL_RAIL_TX_OPTIONS_DEFAULT, &scheduler_info);
      if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
        app_log_warning("TX error %lu\n", rail_status);
      }
    }
    if (packet_sent) {
      packet_sent = false;
      app_log_info("Packet sent\n");
    }
    if (tx_error) {
      tx_error = false;
      app_log_info("TX error\n");
    }
    if (rx_error) {
      rx_error = false;
      app_log_info("RX error\n");
    }
    if (start_rx) {
      start_rx = false;
      app_log_info("Start RAIL RX\n");
      scheduler_info.priority = SL_RAIL_SCHEDULER_PRIORITY_RECEIVE;
      rail_status = sl_rail_start_rx(rail_handle, 0, &scheduler_info);
      if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
        app_log_warning("sl_rail_start_rx() result: %lu\n", rail_status);
      }
    }
    if (stop_rx) {
      stop_rx = false;
      app_log_info("Stop RAIL RX\n");
      rail_status = sl_rail_idle(rail_handle, RAIL_IDLE, true);
      if (rail_status != RAIL_STATUS_NO_ERROR) {
        app_log_warning("sl_rail_idle() result: %lu\n", rail_status);
      }
    }
    if (packet_received) {
      packet_received = false;
      app_log_hexdump_info(rx_packet, packet_size);
      app_log_info("\n");
    }

    uint8_t masked_status = scheduler_status & SL_RAIL_SCHEDULER_STATUS_MASK;
    uint8_t masked_task = scheduler_status & SL_RAIL_SCHEDULER_TASK_MASK;

    if (masked_status != SL_RAIL_SCHEDULER_STATUS_NO_ERROR) {
      if (masked_status == SL_RAIL_SCHEDULER_STATUS_EVENT_INTERRUPTED) {
        start_rx = true;
      }
      if (masked_task == SL_RAIL_SCHEDULER_TASK_SINGLE_TX) {
        tx_requested = true;
      }
      app_log_info("Scheduler status: %d, task: %d\n", masked_status, masked_task);
    }
    handle_os_pending();
  }
}

/*******************************************************************************
 * RAIL callback, called if a RAIL event occurs.
 ******************************************************************************/
void sl_rail_util_on_event(sl_rail_handle_t rail_handle, sl_rail_events_t events)
{
  sl_rail_handle_t rail_handle_rail = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);
  // RAIL Rx packet handles
  sl_rail_rx_packet_handle_t rx_packet_handle;
  sl_rail_rx_packet_info_t packet_info;

  if (rail_handle != rail_handle_rail) {
    return;
  }
  // Handle Rx events
  if (events & SL_RAIL_EVENTS_RX_COMPLETION) {
    if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
      // Download the latest packet
      rx_packet_handle = sl_rail_get_rx_packet_info(rail_handle, SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_info);
      if (rx_packet_handle != SL_RAIL_RX_PACKET_HANDLE_INVALID) {
        sl_rail_copy_rx_packet(rail_handle, rx_packet, &packet_info);
        packet_size = packet_info.packet_bytes;
        packet_received = true;
      } else {
        rx_error = true;
      }
    } else {
      // Handle Rx error
      rx_error = true;
    }
    post_os_flag();
    start_rx = true;
  }
  // Handle Tx events
  if (events & SL_RAIL_EVENTS_TX_COMPLETION) {
    if (events & SL_RAIL_EVENT_TX_PACKET_SENT) {
      packet_sent = true;
    } else {
      // Handle Tx error
      tx_error = true;
    }
    post_os_flag();
    sl_rail_yield_radio(rail_handle);
  }
  if (events & SL_RAIL_EVENT_SCHEDULER_STATUS) {
    // Copy the scheduler status to a non-volatile variable
    sl_rail_scheduler_status_t scheduler_status_buffer;
    sl_rail_get_scheduler_status(rail_handle, &scheduler_status_buffer, NULL);
    scheduler_status = scheduler_status_buffer;
    uint8_t status = scheduler_status & SL_RAIL_SCHEDULER_STATUS_MASK;
    if (status != SL_RAIL_SCHEDULER_STATUS_NO_ERROR) {
      post_os_flag();
    }
    sl_rail_yield_radio(rail_handle);
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
