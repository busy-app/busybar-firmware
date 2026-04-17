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
#include "sl_component_catalog.h"
#include "sl_rail.h"
#include "sl_power_manager.h"
#include "app_process.h"
#include "app_assert.h"
#include "app_log.h"
#include "sl_rail_util_init.h"
#include "sl_rail_sdk_channel_selector.h"
#include "sl_code_classification.h"

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "app_task_init.h"
#endif

#include "cmsis_compiler.h"

#include "sl_rail_sdk_packet_assistant.h"
#include "sl_rail_sdk_fifo_size_config.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/// Transmit data length
#define TX_PAYLOAD_LENGTH (16U)

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
/*******************************************************************************
 * Print to CLI the current value of the TX power
 * @param[in] rail_handle: where to get the TX power levels
 ******************************************************************************/
static void print_current_power_levels(sl_rail_handle_t rail_handle);

/*******************************************************************************
 * Print out the newly selected sleep_mode
 ******************************************************************************/
static void print_new_sleep_mode(void);

/*******************************************************************************
 * Configure and starts RX to make it periodical
 * @param[in] rail_handle: which rail_handle to use for receiving
 ******************************************************************************/
static void handle_periodic_rx(sl_rail_handle_t rail_handle);

/*******************************************************************************
 * Configure and starts TX to make it periodical
 * @param[in] rail_handle: which rail_handle to use for sending
 ******************************************************************************/
static void handle_periodic_tx(sl_rail_handle_t rail_handle);

/*******************************************************************************
 * The API selects the proper sleep level and set it for power_manager
 ******************************************************************************/
static void manage_sleep_levels(void);

/*******************************************************************************
 * The API set the rail back to idle state
 ******************************************************************************/
static void set_radio_to_idle_state(sl_rail_handle_t rail_handle);

/*******************************************************************************
 * Clear flag and allow power manager to go lower then EM1
 ******************************************************************************/
static void clear_em1_mode(void);

/*******************************************************************************
 * Set flag and power manager to EM1 maximum sleep mode
 ******************************************************************************/
static void set_em1_mode(void);

/*******************************************************************************
 * API to handle received packets
 *
 * @param[in] rail_handle: the rail instance that the packet will be read from
 ******************************************************************************/
static void handle_received_packet(sl_rail_handle_t rail_handle);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// Sleep level variable
volatile uint8_t sleep_mode = 1;

/// TX power settings
volatile sli_rail_tx_power_level_t power_raw = 0;
volatile sl_rail_tx_power_t power_deci_dbm = 0;
volatile bool is_raw = false;

/// Scheduled TX/RX variables
volatile uint32_t sleep_period = 0; //sleep period in us for PERIODIC_TX/_RX
volatile uint32_t rx_on_period = 0; //rx period for periodic _RX

/// Schedule state first run
volatile bool init_needed = false;

/// Schedule TX/RX flags to not run again accidentally
volatile bool packet_sending = false;
volatile bool periodic_rx_ended = true;
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// Transmit packet
static uint8_t out_packet[TX_PAYLOAD_LENGTH] = {
  0x0F, 0x16, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
  0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE,
};

/// RAIL Rx packet handle
static volatile sl_rail_rx_packet_handle_t rx_packet_handle;

/// Flags to manage rail sleep levels
static bool em1_is_enabled = false;
static bool allow_to_sleep = true;

/// State machine state variable and buffer
static volatile state_t app_state = S_IDLE;
bool volatile periodic_receive = false;

/// config for the TX schedule option
static sl_rail_scheduled_tx_config_t schedule_tx_config = {
  .mode = SL_RAIL_TIME_DELAY,
  .tx_during_rx = SL_RAIL_SCHEDULED_TX_DURING_RX_ABORT_TX,
  .when = 500000
};

/// config for the RX schedule option
static sl_rail_scheduled_rx_config_t schedule_rx_config = {
  .start_mode = SL_RAIL_TIME_DELAY,
  .start =  500000,
  .end_mode = SL_RAIL_TIME_DELAY,
  .end = 500000,
  .rx_transition_end_schedule = 0,
  .hard_window_end = 0
};

static uint8_t rx_buffer[SL_RAIL_SDK_RX_FIFO_SIZE];

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
  // Status indicator of the RAIL API calls
  sl_rail_status_t rail_status;

  switch (app_state) {
    case S_IDLE:
      break;
    case S_SET_IDLE:
      print_new_sleep_mode();
      set_radio_to_idle_state(rail_handle);
      app_state = S_IDLE;
      break;
    case S_CW:
      app_log_info("Tx CW mode; EM%d\n", sleep_mode);
      set_radio_to_idle_state(rail_handle);
      rail_status = sl_rail_start_tx_stream(rail_handle, get_selected_channel(), SL_RAIL_STREAM_CARRIER_WAVE, SL_RAIL_TX_OPTIONS_DEFAULT);
      if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
        app_log_warning("sl_rail_start_tx_stream() result: %lu", rail_status);
      }
      app_state = S_IDLE;
      break;
    case S_RX:
      app_log_info("Rx mode; EM%d\n", sleep_mode);
      set_radio_to_idle_state(rail_handle);
      rail_status = sl_rail_start_rx(rail_handle, get_selected_channel(), NULL);
      if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
        app_log_warning("sl_rail_start_rx() result: %lu", rail_status);
      }
      app_state = S_IDLE;
      break;
    case S_PERIODIC_RX:
      handle_periodic_rx(rail_handle);
      break;
    case S_PERIODIC_TX:
      handle_periodic_tx(rail_handle);
      break;
    case S_SET_POWER_LEVEL:
      set_radio_to_idle_state(rail_handle);
      if (is_raw) {
        rail_status = sli_rail_set_tx_power(rail_handle, power_raw);
        power_deci_dbm = sl_rail_get_tx_power_dbm(rail_handle);
        if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
          app_log_warning("sli_rail_set_tx_power() result: %lu", rail_status);
        }
      } else {
        rail_status = sl_rail_set_tx_power_dbm(rail_handle, power_deci_dbm);
        power_raw = sli_rail_get_tx_power(rail_handle);
        if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
          app_log_warning("sl_rail_set_tx_power_dbm() result: %lu", rail_status);
        }
      }
      print_current_power_levels(rail_handle);
      app_state = S_IDLE;
      break;
    case S_GET_POWER_LEVEL:
      print_current_power_levels(rail_handle);
      app_state = S_IDLE;
      break;
    case S_PACKET_RECEIVED:
      handle_received_packet(rail_handle);
      break;
    case S_RX_ERROR:
      app_log_error("RX error\n");
      if (periodic_receive) {
        periodic_rx_ended = false;
        rail_status = sl_rail_start_scheduled_rx(rail_handle, get_selected_channel(), &schedule_rx_config, NULL);
        if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
          app_log_warning("sl_rail_start_scheduled_rx() result: %lu", rail_status);
        }
        app_state = S_PERIODIC_RX;
      } else {
        rail_status = sl_rail_start_rx(rail_handle, get_selected_channel(), NULL);
        if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
          app_log_warning("sl_rail_start_rx() result: %lu", rail_status);
        }
        app_state = S_IDLE;
      }
      break;
    default:
      break;
  }
  manage_sleep_levels();
}

/*******************************************************************************
 * Set state machine state out of this file
 *
 * @param[in] next_state: the next state the machine will be in
 ******************************************************************************/
void set_next_state(state_t next_state)
{
  app_state = next_state;
}

/*******************************************************************************
 * Set that the app can go to sleep
 * @return bool: if true app can go to sleep mode
 ******************************************************************************/
bool app_is_ok_to_sleep(void)
{
  return allow_to_sleep;
}

/*******************************************************************************
 * Set the flag for proper sleep level.
 ******************************************************************************/
void init_em1_mode(void)
{
  set_em1_mode();
}

/******************************************************************************
 * RAIL callback, called if a RAIL event occurs
 *****************************************************************************/
SL_CODE_RAM void sl_rail_util_on_event(sl_rail_handle_t rail_handle, sl_rail_events_t events)
{
  // Handle Rx events
  if ( events & SL_RAIL_EVENTS_RX_COMPLETION ) {
    if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
      // Keep the packet in the radio buffer, download it later at the state machine
      rx_packet_handle = sl_rail_hold_rx_packet(rail_handle);
      app_state = S_PACKET_RECEIVED;
    } else {
      app_state = S_RX_ERROR;
    }
  }
  // Handle Tx events
  if ( events & SL_RAIL_EVENTS_TX_COMPLETION) {
    if (events & SL_RAIL_EVENT_TX_PACKET_SENT) {
      packet_sending = false;
    }
  }

  if (events & SL_RAIL_EVENT_RX_SCHEDULED_RX_END) {
    periodic_rx_ended = true;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/*******************************************************************************
 * Print to CLI the current value of the TX power
 * @param[in] rail_handle: where to get the TX power levels
 ******************************************************************************/
static void print_current_power_levels(sl_rail_handle_t rail_handle)
{
  if (is_raw) {
    app_log_info("Power            %d/%d\n",
                 power_raw,
                 (sli_rail_get_tx_power(rail_handle)));
  } else {
#if defined(__IAR_SYSTEMS_ICC__)
  #pragma diag_suppress=Pa205
#endif
    app_log_info("Power:            %.1f/%.1fdBm\n",
                 (float)(power_deci_dbm) / 10.0,
                 (float)(sl_rail_get_tx_power_dbm(rail_handle)) / 10.0);
  }
#if defined(__IAR_SYSTEMS_ICC__)
  #pragma diag_default=Pa205
#endif
}

/*******************************************************************************
 * Print out the newly selected sleep_mode
 ******************************************************************************/
static void print_new_sleep_mode(void)
{
  app_log_info("Idle mode; EM%d\n", sleep_mode);
}

/*******************************************************************************
 * Configure and starts RX to make it periodical
 * @param[in] rail_handle: which rail_handle to use for receiving
 ******************************************************************************/
static void handle_periodic_rx(sl_rail_handle_t rail_handle)
{
  // Status indicator of the RAIL API calls
  sl_rail_status_t rail_status;
  if (init_needed) {
    init_needed = false;
    set_radio_to_idle_state(rail_handle);
    schedule_rx_config.start = sleep_period;
    schedule_rx_config.end = rx_on_period;
    app_log_info(
      "Periodic Rx mode, sleepPeriod=%lu, rxPeriod=%lu; EM%d (sleep); EM%d (active)\n",
      schedule_rx_config.start, schedule_rx_config.end, sleep_mode,
      0);
  }
  if (periodic_rx_ended) {
    periodic_rx_ended = false;
    rail_status = sl_rail_start_scheduled_rx(rail_handle, get_selected_channel(), &schedule_rx_config, NULL);
    if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
      app_log_warning("sl_rail_start_scheduled_rx() result: %lu", rail_status);
    }
  }
}

/*******************************************************************************
 * Configure and starts TX to make it periodical
 * @param[in] rail_handle: which rail_handle to use for sending
 ******************************************************************************/
static void handle_periodic_tx(sl_rail_handle_t rail_handle)
{
  // Status indicator of the RAIL API calls
  sl_rail_status_t rail_status;

  if (init_needed) {
    init_needed = false;
    set_radio_to_idle_state(rail_handle);
    schedule_tx_config.when = sleep_period;
    app_log_info("Periodic Tx mode, period=%lu; EM%d (sleep); EM%d (active)\n",
                 schedule_tx_config.when, sleep_mode, 0);
  }

  if (!packet_sending) {
    prepare_packet(rail_handle, out_packet, sizeof(out_packet));
    rail_status = sl_rail_start_scheduled_tx(rail_handle, get_selected_channel(), SL_RAIL_TX_OPTIONS_DEFAULT, &schedule_tx_config, NULL);
    if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
      app_log_warning("sl_rail_start_scheduled_tx() result: %lu", rail_status);
    }
    packet_sending = true;
  }
}

/*******************************************************************************
 * API to handle received packets
 *
 * @param[in] rail_handle: the rail instance that the packet will be read from
 ******************************************************************************/
static void handle_received_packet(sl_rail_handle_t rail_handle)
{
  // Status indicator of the RAIL API calls
  sl_rail_status_t rail_status;
  // for received packets
  sl_rail_rx_packet_info_t packet_info;

  if (rx_packet_handle == SL_RAIL_RX_PACKET_HANDLE_INVALID) {
    app_log_error("sl_rail_hold_rx_packet() error: SL_RAIL_RX_PACKET_HANDLE_INVALID\n"
                  "No such RAIL rx packet yet exists or rail_handle is not active");
  }
  rx_packet_handle = sl_rail_get_rx_packet_info(rail_handle, SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_info);
  while (rx_packet_handle != SL_RAIL_RX_PACKET_HANDLE_INVALID) {
    uint8_t *start_of_packet = 0;
    if (packet_info.packet_bytes <= SL_RAIL_SDK_RX_FIFO_SIZE) {
      uint16_t packet_size = unpack_packet(rail_handle, rx_buffer, &packet_info, &start_of_packet);
      rail_status = sl_rail_release_rx_packet(rail_handle, rx_packet_handle);
      if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
        app_log_warning("sl_rail_release_rx_packet() result: %lu", rail_status);
      }
      printf_rx_packet(start_of_packet, packet_size);
    }
    rx_packet_handle = sl_rail_get_rx_packet_info(rail_handle, SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_info);
  }

  if (periodic_receive) {
    periodic_rx_ended = false;
    rail_status = sl_rail_start_scheduled_rx(rail_handle, get_selected_channel(), &schedule_rx_config, NULL);
    if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
      app_log_warning("sl_rail_start_scheduled_rx() result: %lu", rail_status);
    }
    app_state = S_PERIODIC_RX;
  } else {
    rail_status = sl_rail_start_rx(rail_handle, get_selected_channel(), NULL);
    if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
      app_log_warning("sl_rail_start_rx() result: %lu", rail_status);
    }
    app_state = S_IDLE;
  }
}

/*******************************************************************************
 * The API selects the proper sleep level and set it for power_manager
 ******************************************************************************/
static void manage_sleep_levels(void)
{
  allow_to_sleep = true;
  switch (sleep_mode) {
    case 0:
      allow_to_sleep = false;
      break;
    case 1:
      set_em1_mode();
      break;
    case 2:
      clear_em1_mode();
      break;
    default:
      allow_to_sleep = false;
      break;
  }
}

/*******************************************************************************
 * The API set the rail back to idle state
 ******************************************************************************/
static void set_radio_to_idle_state(sl_rail_handle_t rail_handle)
{
  sl_rail_stop_tx_stream(rail_handle);
  sl_rail_idle(rail_handle, SL_RAIL_IDLE, true);
}

/*******************************************************************************
 * Set flag and power manager to EM1 maximum sleep mode
 ******************************************************************************/
static void set_em1_mode(void)
{
  if (!em1_is_enabled) {
    em1_is_enabled = true;
    sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
  }
}

/*******************************************************************************
 * Clear flag and allow power manager to go lower then EM1
 ******************************************************************************/
static void clear_em1_mode(void)
{
  if (em1_is_enabled) {
    em1_is_enabled = false;
    sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
  }
}
