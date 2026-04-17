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
#include "sl_component_catalog.h"
#include "sl_rail.h"
#include "app_init.h"
#include "app_process.h"
#include "sl_rail_util_init.h"
#include "sl_rail_sdk_packet_assistant.h"
#include "sl_duty_cycle_config.h"
#include "sl_rail_sdk_fifo_size_config.h"
#include "sl_rail_sdk_channel_selector.h"
#include "sl_rail_sdk_simple_assistance.h"
#include "sl_code_classification.h"

#if DUTY_CYCLE_USE_LCD_BUTTON
#include "app_graphics.h"
#include "sl_simple_button_instances.h"
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "app_task_init.h"
#endif

#include "cmsis_compiler.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Application type string
#define APP_TYPE_STRING   "Burst Mode"
/// Required transmission time for a guaranteed reception in Slave Rx mode
#define BURST_TIME        (DUTY_CYCLE_OFF_TIME + (2UL * DUTY_CYCLE_ON_TIME))
/// Transmit data length
#define TX_PAYLOAD_LENGTH (16U)
/// RX buffer length
#define RX_BUFFER_LENGTH (256U)

/// State machine of Duty Cycle
typedef enum {
  S_IDLE,             //!< Idling in default Slave Mode
  S_BURST_RECEIVE,    //!< Burst received in Slave mode
  S_BURST_SENDING,    //!< Burst TX in progress in Master Mode
  S_ERROR             //!< An error occurred
} state_t;

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * This helper function handles the S_IDLE state of the state machine.
 ******************************************************************************/
static void handle_idle_state(sl_rail_handle_t rail_handle);

/*******************************************************************************
 * This helper function handles the S_BURST_RECEIVE state of the state machine.
 ******************************************************************************/
static void handle_receive_state(sl_rail_handle_t rail_handle);

/*******************************************************************************
 * This helper function handles the S_BURST_SENDING state of the state machine.
 ******************************************************************************/
static void handle_send_state(sl_rail_handle_t rail_handle);

/*******************************************************************************
 * This helper function handles the S_ERROR state of the state machine.
 ******************************************************************************/
static void handle_error_state(sl_rail_handle_t rail_handle);

/*******************************************************************************
 * Send a prepared TX packet on selected rail handler
 *
 * @param[in] rail_handle: which rail handler to use for tx function
 * @return rail_status: error code from rail
 ******************************************************************************/
static sl_rail_status_t send_tx_packet(sl_rail_handle_t rail_handle);

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

/// Master mode constants and variables
/// ID for a blast
static volatile uint8_t master_burst_id = 0U;
/// count of packets sent in the current burst
static volatile uint32_t master_burst_packets_count = 0UL;
/// burst ID received by the Slave
static volatile uint8_t slave_rx_burst_id = 0U;
/// The variable shows the actual state of the state machine
static volatile state_t state = S_IDLE;
/// Contains the status of RAIL Calibration, useful for error handling
static volatile sl_rail_status_t calibration_status = 0;
/// RAIL Rx packet handle
static volatile sl_rail_rx_packet_handle_t rx_packet_handle;

/// Transmit packet
static uint8_t out_packet[TX_PAYLOAD_LENGTH] = {
  0x0F, 0x16, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
  0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE,
};

/// RX buffer
static uint8_t rx_buffer[RX_BUFFER_LENGTH];

/// Counter to be displayed on LCD
static uint16_t packet_transmitted = 0U;              // TX packets count
static uint16_t packet_received = 0U;                 // RX packets count

#if DUTY_CYCLE_USE_LCD_BUTTON == 1
/// Flag to refresh LCD with new values
static bool refresh_display = true;
#endif

/// Flag to indicate that app is ready for sleep
static volatile bool app_ready_to_sleep = true;

/// Flags to know is the interrupt was useful
static bool button_interrupt = false;
static bool radio_interrupt = false;
static bool first_run = false;

/// Flags to indicate relevant RAIL events
static bool rail_packet_sent = false;
static bool rail_packet_received = false;
static bool rail_error = false;
/// Timeout calculated for the burst
static sl_rail_time_t timeout = 0UL;
static sl_rail_events_t rail_last_state = SL_RAIL_EVENTS_NONE;

/// Flag, indicating duty cycle cycle was completed
static volatile bool duty_cycle_end = false;

#if DUTY_CYCLE_ALLOW_EM2 == 1
static sl_rail_scheduled_rx_config_t rx_schedule_config = {
  .start = DUTY_CYCLE_OFF_TIME,
  .start_mode = SL_RAIL_TIME_DELAY,
  .end = 0U,
  .end_mode = SL_RAIL_TIME_DISABLED,
  .rx_transition_end_schedule = 0U,
  .hard_window_end = 0U
};
#endif

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Setter function for flag which allows to run state machine without interrupt
 *
 * @param[in] is_first_run: Set the first run flag
 ******************************************************************************/
void set_first_run(bool is_first_run)
{
  first_run = is_first_run;
}

/*******************************************************************************
 * Application state machine, called infinitely
 *
 * @param[in] rail_handle: which rail handler to use for rx and tx
 ******************************************************************************/
void app_process_action(void)
{
  // Get RAIL handle, used later by the application
  sl_rail_handle_t rail_handle = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);
  // To make sure to only enter in state machine if useful interrupt happened
  if (!(button_interrupt || radio_interrupt || first_run || rail_error || tx_requested)) {
    return;
  } else {
    button_interrupt = false;
    radio_interrupt = false;
    first_run = false;
  }

  // Handle errors if pending
  if (rail_error) {
    rail_error = false;
    state = S_ERROR;
  }

  switch (state) {
    case S_IDLE:
      handle_idle_state(rail_handle);
      break;
    case S_BURST_SENDING:
      handle_send_state(rail_handle);
      break;
    case S_BURST_RECEIVE:
      handle_receive_state(rail_handle);
      break;
    case S_ERROR:
      handle_error_state(rail_handle);
      break;
    default:
      // Unexpected state
      app_log_error("Unexpected state occurred:%d\n", state);
#if DUTY_CYCLE_USE_LCD_BUTTON == 1
      display_error_on_lcd(INVALID_APP_STATE);
      refresh_display = false;
#endif
      break;
  }

  // Reset copy of RAIL events
  rail_last_state = SL_RAIL_EVENTS_NONE;

  // After packet received or DutyCycle end RAIL RX needs to be restarted
  if (duty_cycle_end) {
    duty_cycle_end = false;
#if DUTY_CYCLE_ALLOW_EM2 == 0
    sl_rail_start_rx(rail_handle, get_selected_channel(), NULL);
#else
    rx_schedule_config.start = duty_cycle_config.delay_us;
    sl_rail_idle(rail_handle, SL_RAIL_IDLE_ABORT, true);
    sl_rail_start_scheduled_rx(rail_handle, get_selected_channel(), &rx_schedule_config, NULL);
#endif
  }

#if DUTY_CYCLE_USE_LCD_BUTTON == 1
  if (refresh_display) {
    refresh_display = false;
    update_display(APP_TYPE_STRING, packet_transmitted, packet_received);
  }
#endif
}

/*******************************************************************************
 * RAIL callback, called if a RAIL event occurs.
 ******************************************************************************/
SL_CODE_RAM void sl_rail_util_on_event(sl_rail_handle_t rail_handle, sl_rail_events_t events)
{
  // Make a copy of the events
  rail_last_state = events;
  // something RAIL happened
  radio_interrupt = true;

  // Handle Tx events
  if (events & SL_RAIL_EVENTS_TX_COMPLETION) {
    if (events & SL_RAIL_EVENT_TX_PACKET_SENT) {
      rail_packet_sent = true;
    } else {
      rail_error = true;
    }
  }

  // Handle Rx events
  if (events & SL_RAIL_EVENTS_RX_COMPLETION) {
    if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
      // Keep the packet in the radio buffer, download it later at the state machine
      rx_packet_handle = sl_rail_hold_rx_packet(rail_handle);
      // Prevent app from sleeping before processing received packet
      app_ready_to_sleep = false;
      rail_packet_received = true;
    } else {
      rail_error = true;
    }
  }

#if DUTY_CYCLE_ALLOW_EM2 == 1
  if (events & SL_RAIL_EVENT_RX_DUTY_CYCLE_RX_END) {
    duty_cycle_end = true;
  }
#endif

  // Perform all calibrations when needed or indicate error if failed
  if (events & SL_RAIL_EVENT_CAL_NEEDED) {
    calibration_status = sl_rail_calibrate(rail_handle, NULL, SL_RAIL_CAL_ALL_PENDING);
    if (calibration_status != SL_RAIL_STATUS_NO_ERROR) {
      rail_error = true;
    }
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

#if DUTY_CYCLE_USE_LCD_BUTTON == 1
/*******************************************************************************
 * Button callback, called if any button is pressed or released.
 ******************************************************************************/
SL_CODE_RAM void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    button_interrupt = true;
    tx_requested = true;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}
#endif

/*******************************************************************************
* This app-level function overrides a weak power manager implementation and
* gets called in sl_power_manager_handler.c preventing the MCU from going
* to sleep if app is not ready for it.
*******************************************************************************/
bool app_is_ok_to_sleep(void)
{
  return app_ready_to_sleep;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/*******************************************************************************
 * This helper function handles the S_IDLE state of the state machine.
 ******************************************************************************/
static void handle_idle_state(sl_rail_handle_t rail_handle)
{
  // Wait for a received packet
  if (rail_packet_received) {
    // Don't clear the RX flag, keep coming back to state machine
    radio_interrupt = true;
    // Switch to handling of the received packet
    state = S_BURST_RECEIVE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
    app_task_notify();
#endif
  } else if (tx_requested) {
    // Clear request and start a new burst
    tx_requested = false;
    master_burst_id++;
    // Disable duty cycle,then schedule the burst period and start sending
    sl_rail_idle(rail_handle, SL_RAIL_IDLE, true);
    sl_rail_enable_rx_duty_cycle(rail_handle, false);
    // Time the stop of burst to time when the Rx side is guaranteed to have listened.
    timeout = sl_rail_get_time(rail_handle) + BURST_TIME;
    // Kickstart the burst
    radio_interrupt = true;
    rail_packet_sent = true;
    state = S_BURST_SENDING;
    // Prevent app from going to sleep until the burst tx is over
    app_ready_to_sleep = false;
#if defined(SL_CATALOG_KERNEL_PRESENT)
    app_task_notify();
#endif
  }
}

/*******************************************************************************
 * This helper function handles the S_BURST_RECEIVE state of the state machine.
 ******************************************************************************/
static void handle_receive_state(sl_rail_handle_t rail_handle)
{
  // Used for accessing the packet data
  sl_rail_rx_packet_info_t packet_info;
  // Status indicator of the RAIL API calls
  sl_rail_status_t rail_status;
  // Packet size for the RX packet
  uint16_t packet_size;

  // Incoming packet?
  if (rail_packet_received) {
    rail_packet_received = false;
    // Get the RX packet data
    if (rx_packet_handle == SL_RAIL_RX_PACKET_HANDLE_INVALID) {
      app_log_error("sl_rail_hold_rx_packet() error: SL_RAIL_RX_PACKET_HANDLE_INVALID\n"
                    "No such RAIL rx packet yet exists or rail_handle is not active");
    }
    rx_packet_handle = sl_rail_get_rx_packet_info(rail_handle,
                                                  SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE,
                                                  &packet_info);
    if (rx_packet_handle == SL_RAIL_RX_PACKET_HANDLE_INVALID) {
      app_log_error("sl_rail_get_rx_packet_info() error: SL_RAIL_RX_PACKET_HANDLE_INVALID\n");
    }
    uint8_t *start_of_packet = rx_buffer;
    if (packet_info.packet_bytes > RX_BUFFER_LENGTH) {
      app_log_error("sl_rail_get_rx_packet_info() error: packet too long\n");
      packet_size = RX_BUFFER_LENGTH;
    } else {
      packet_size = unpack_packet(rail_handle, rx_buffer, &packet_info, &start_of_packet);
    }
    // Packet processed, app is now ready to sleep
    app_ready_to_sleep = true;
    rail_status = sl_rail_release_rx_packet(rail_handle, rx_packet_handle);
    if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
      app_log_warning("sl_rail_release_rx_packet() result: %lu", rail_status);
    }
    // Check if this is a new burst
    if (slave_rx_burst_id != start_of_packet[0]) {
      slave_rx_burst_id = start_of_packet[0];
      // Print packet if requested
      if (rx_requested) {
        printf_rx_packet(start_of_packet, packet_size);
      }
      // RX bookkeeping & update LCD
      toggle_receive_led();
      packet_received++;
#if DUTY_CYCLE_USE_LCD_BUTTON == 1
      refresh_display = true;
#endif
      // Rx of one packet done, going back to listen
      duty_cycle_end = true;
      state = S_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
      app_task_notify();
#endif
    } else {
      duty_cycle_end = true;
      state = S_IDLE;
    }
  }
}

/*******************************************************************************
 * This helper function handles the S_BURST_SENDING state of the state machine.
 ******************************************************************************/
static void handle_send_state(sl_rail_handle_t rail_handle)
{
  // Check if burst time elapsed
  if (sl_rail_get_time(rail_handle) >= timeout) {
    // Burst completed, clear any Tx flags and update UI
    rail_packet_sent = false;
    toggle_send_led();
    packet_transmitted++;
#if DUTY_CYCLE_USE_LCD_BUTTON == 1
    refresh_display = true;
#endif
    app_log_info("Burst of %lu packets sent.\n", master_burst_packets_count);
    // Run-time check if the listener had no chance to receive the burst
    if ((BURST_TIME / master_burst_packets_count) > DUTY_CYCLE_ON_TIME) {
      app_log_info("WARNING! Packet time of %lu is longer than the ON time of %lu!\n",
                   (BURST_TIME / master_burst_packets_count),
                   (uint32_t)DUTY_CYCLE_ON_TIME);
    }
    master_burst_packets_count = 0UL;
    // Go back to Slave Idle state in Duty Cycle
    sl_rail_idle(rail_handle, SL_RAIL_IDLE, true);
    sl_rail_enable_rx_duty_cycle(rail_handle, true);
    duty_cycle_end = true;
    state = S_IDLE;
    // Burst tx is done, app is now ready to sleep
    app_ready_to_sleep = true;
#if defined(SL_CATALOG_KERNEL_PRESENT)
    app_task_notify();
#endif
  } else {
    // Otherwise, do the burst
    if (rail_packet_sent) {
      rail_packet_sent = false;
      master_burst_packets_count++;
      out_packet[0] = master_burst_id;
      send_tx_packet(rail_handle);
    }
  }
}

/*******************************************************************************
 * This helper function handles the S_ERROR state of the state machine.
 ******************************************************************************/
static void handle_error_state(sl_rail_handle_t rail_handle)
{
  (void)rail_handle;
  // Handle Rx error
  if (rail_last_state & SL_RAIL_EVENTS_RX_COMPLETION) {
    app_log_error("Radio RX Error occurred\nEvents: %lld\n", rail_last_state);
    // Handle Tx error
  } else if (rail_last_state & SL_RAIL_EVENTS_TX_COMPLETION) {
    app_log_error("Radio TX Error occurred\nEvents: %lld\n", rail_last_state);
    // Handle calibration error
  } else if (rail_last_state & SL_RAIL_EVENT_CAL_NEEDED) {
    app_log_error("Radio Calibration Error occurred\nEvents: %lld\nsl_rail_calibrate() result:%ld\n",
                  rail_last_state,
                  calibration_status);
  }
  // Restart duty cycle
  duty_cycle_end = true;

  state = S_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

/*******************************************************************************
 * Send a prepared TX packet on selected rail handler
 *
 * @param[in] rail_handle: which rail handler to use for tx function
 * @return rail_status: error code from rail
 ******************************************************************************/
static sl_rail_status_t send_tx_packet(sl_rail_handle_t rail_handle)
{
  // Status indicator of the RAIL API calls
  sl_rail_status_t rail_status;
  prepare_packet(rail_handle, out_packet, sizeof(out_packet));

  rail_status = sl_rail_start_tx(rail_handle, get_selected_channel(), SL_RAIL_TX_OPTIONS_DEFAULT, NULL);

  if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_warning("sl_rail_start_tx() result: %lu ", rail_status);
  }

  return rail_status;
}
