/***************************************************************************//**
 * @file
 * @brief app_tick.c
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "app_process.h"
#include "sl_rail_util_init.h"
#include "sl_simple_button_instances.h"
#include "sl_rail_sdk_simple_assistance.h"
#include "rail_config.h"
#include "sl_rail_sdk_packet_asm.h"
#include "cmsis_compiler.h"
#include "sl_rail_sdk_mode_switch.h"
#include "sl_rail_sdk_fifo_size_config.h"
#include "sl_power_manager.h"
#include "app_log.h"
#include "app_assert.h"
#include "sl_code_classification.h"

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "app_task_init.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/// Transmit data length
#define TX_PAYLOAD_LENGTH (16U)
/// Indicator of a mode switch end packet
#define MS_END_PACKET_INDICATOR (0xF0U)

/// State machine
typedef enum {
  S_PACKET_RECEIVED,
  S_PACKET_SENT,
  S_RX_PACKET_ERROR,
  S_TX_PACKET_ERROR,
  S_CALIBRATION_ERROR,
  S_IDLE
} state_t;

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * Function to select current state.
 *****************************************************************************/
static void select_state(void);

/**************************************************************************//**
 * Function to handle the S_PACKET_RECEIVED state.
 *
 * @param[in] rail_handle A RAIL instance handle
 *****************************************************************************/
static void handle_state_packet_received(sl_rail_handle_t rail_handle);

/**************************************************************************//**
 * Function to handle the S_PACKET_SENT state.
 *
 * @param[in] rail_handle A RAIL instance handle
 *****************************************************************************/
static void handle_state_packet_sent(sl_rail_handle_t rail_handle);

/**************************************************************************//**
 * Function to handle the S_RX_PACKET_ERROR state.
 *
 * @param[in] rail_handle A RAIL instance handle
 *****************************************************************************/
static void handle_state_rx_packet_error(sl_rail_handle_t rail_handle);

/**************************************************************************//**
 * Function to handle the S_TX_PACKET_ERROR state.
 *
 * @param[in] rail_handle A RAIL instance handle
 *****************************************************************************/
static void handle_state_tx_packet_error(sl_rail_handle_t rail_handle);

/**************************************************************************//**
 * Function to handle the S_CALIBRATION_ERROR state.
 *
 * @param[in] rail_handle A RAIL instance handle
 *****************************************************************************/
static void handle_state_calibration_error(sl_rail_handle_t rail_handle);

/**************************************************************************//**
 * Function to handle the S_IDLE state.
 *
 * @param[in] rail_handle A RAIL instance handle
 *****************************************************************************/
static void handle_state_idle(sl_rail_handle_t rail_handle);

/**************************************************************************//**
 * The function printfs the received rx message.
 *
 * @param[in] rx_buffer Msg buffer
 * @param[in] length How many bytes should be printed out
 *****************************************************************************/
static void printf_rx_packet(const uint8_t * const rx_buffer, uint16_t length);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// Flag that indicates that send is requested
volatile bool tx_requested = false;
/// Flag, indicating received packet is forwarded to CLI or not
volatile bool rx_requested = true;
/// Struct that holds all the available channels and their properties
extern radio_info_t radio_info;

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
  0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE
};
/// Mode switch end packet
static uint8_t ms_end_packet[TX_PAYLOAD_LENGTH] = {
  MS_END_PACKET_INDICATOR, 0x16, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
  0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE
};

/// Flags to update state machine from interrupt
static volatile bool packet_received = false;
static volatile bool packet_sent = false;
static volatile bool rx_error = false;
static volatile bool tx_error = false;
static volatile bool cal_error = false;

/// Flag, indicating mode switch request
volatile bool ms_requested = false;

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

  select_state();
  switch (state) {
    case S_PACKET_RECEIVED:
      handle_state_packet_received(rail_handle);
      break;

    case S_PACKET_SENT:
      handle_state_packet_sent(rail_handle);
      break;

    case S_RX_PACKET_ERROR:
      handle_state_rx_packet_error(rail_handle);
      break;

    case S_TX_PACKET_ERROR:
      handle_state_tx_packet_error(rail_handle);
      break;

    case S_IDLE:
      handle_state_idle(rail_handle);
      break;

    case S_CALIBRATION_ERROR:
      handle_state_calibration_error(rail_handle);
      break;

    default:
      app_log_error("Unexpected state occurred: %d\n", state);
      break;
  }
}

/******************************************************************************
 * RAIL callback, called if a RAIL event occurs.
 *****************************************************************************/
SL_CODE_RAM void sl_rail_util_on_event(sl_rail_handle_t rail_handle, sl_rail_events_t events)
{
  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;
  mode_switch_state_t ms_state = get_ms_state();

  error_code = events;

  // Handle Rx events
  if (events & SL_RAIL_EVENTS_RX_COMPLETION) {
    if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
      // Keep the packet in the radio buffer,
      // download it later at the state machine
      sl_rail_hold_rx_packet(rail_handle);
      packet_received = true;
    } else {
      // Handle Rx error
      rx_error = true;
    }
  }
  // Handle Tx events
  if (events & SL_RAIL_EVENTS_TX_COMPLETION) {
    if (events & SL_RAIL_EVENT_TX_PACKET_SENT) {
      packet_sent = true;
    } else {
      // Handle Tx error
      tx_error = true;
    }
  }

  if (events & SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_START) {
    if (ms_state == MS_IDLE) {
      status = switch_to_ms_channel(rail_handle);
      if (status == SL_RAIL_STATUS_NO_ERROR) {
        app_log_info("SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_START happened\n");
        set_ms_state(MS_ON_NEW_PHY);
      }
    } else {
      set_ms_state(MS_IDLE);
    }
  }

  if (events & SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_END) {
    status = return_to_base_channel();
    if (status == SL_RAIL_STATUS_NO_ERROR) {
      app_log_info("SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_END happened\n");
    }
  }

  // Perform all calibrations when needed
  if (events & SL_RAIL_EVENT_CAL_NEEDED) {
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
    if (handle == &sl_button_btn0) {
      tx_requested = true;
    } else if (handle == &sl_button_btn1) {
      set_mode_switch_duration(10);
      uint16_t selected_ofdm_id = 0;
      for (uint8_t i = 0; i < radio_info.mode_switch_capable_channels; i++) {
        if (radio_info.channel_list[i].modulation == M_OFDM) {
          selected_ofdm_id = radio_info.channel_list[i].phy_mode_id;
        }
      }
      set_new_phy_mode_id(selected_ofdm_id);
      request_mode_switch();
    }
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

/******************************************************************************
 * API to request a radio transmission.
 *****************************************************************************/
void request_tx(void)
{
  tx_requested = true;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/******************************************************************************
 * Function to select current state.
 *****************************************************************************/
static void select_state(void)
{
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
}

/******************************************************************************
 * Function to handle the S_PACKET_RECEIVED state.
 *****************************************************************************/
static void handle_state_packet_received(sl_rail_handle_t rail_handle)
{
  // Packet received:
  //  - Check whether sl_rail_hold_rx_packet() was successful, i.e. packet handle is valid
  //  - Copy it to the application FIFO
  //  - Free up the radio FIFO
  //  - Return to app IDLE state (RAIL will automatically switch back to Rx radio state)
  sl_rail_status_t rail_status = SL_RAIL_STATUS_NO_ERROR;
  sl_rail_rx_packet_handle_t rx_packet_handle;
  sl_rail_rx_packet_info_t packet_info;
  uint8_t *start_of_packet = 0U;
  uint16_t packet_size = 0U;

  rx_packet_handle = sl_rail_get_rx_packet_info(rail_handle,
                                                SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE,
                                                &packet_info);
  while (rx_packet_handle != SL_RAIL_RX_PACKET_HANDLE_INVALID) {
    start_of_packet = 0;
    if (packet_info.packet_bytes <= SL_RAIL_SDK_RX_FIFO_SIZE) {
      packet_size = unpack_packet(rail_handle, rx_buffer,
                                  &packet_info,
                                  &start_of_packet,
                                  get_phy_modulation_from_channel(get_channel()));
      rail_status = sl_rail_release_rx_packet(rail_handle, rx_packet_handle);
      if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
        app_log_warning("ERROR sl_rail_release_rx_packet() result: %lu\n", rail_status);
      }
      if (rx_requested) {
        printf_rx_packet(start_of_packet, packet_size);
      }
      if (start_of_packet[0] == MS_END_PACKET_INDICATOR) {
        rail_status = return_to_base_channel();
        if (rail_status == SL_RAIL_STATUS_NO_ERROR) {
          app_log_info("Mode switch end, returned to the base channel\n");
        } else {
          app_log_warning("Error during returning to the base channel: %d\n",
                          (uint8_t)rail_status);
        }
      }
      toggle_receive_led();
    }
    rx_packet_handle = sl_rail_get_rx_packet_info(rail_handle,
                                                  SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE,
                                                  &packet_info);
  }
  state = S_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

/******************************************************************************
 * Function to handle the S_PACKET_SENT state.
 *****************************************************************************/
static void handle_state_packet_sent(sl_rail_handle_t rail_handle)
{
  (void) rail_handle;
  sl_rail_status_t rail_status = SL_RAIL_STATUS_NO_ERROR;
  mode_switch_state_t ms_state = get_ms_state();
  uint16_t ms_new_channel = get_ms_new_channel();

  app_log_info("Packet has been sent\n");
  if (ms_state == MS_SENDING_MS_START_PACKET) {
    set_ms_state(MS_ON_NEW_PHY);
    set_channel(ms_new_channel);
    app_log_info("Mode switch completed\n");
  }

  if (ms_state == MS_SENDING_MS_END_PACKET) {
    rail_status = return_to_base_channel();
    if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
      app_log_warning("ERROR return_to_base_channel: %lu\n",
                      rail_status);
    }
  }
  if (get_print_packet_details()) {
    if (get_phy_modulation_from_channel(get_channel()) == M_OFDM) {
      app_log_info("With rate: %d and scramble: %d\n",
                   get_ofdm_rate(),
                   get_ofdm_scrambler());
    } else {
      app_log_info("With fcs type: %d and whitening: %d\n",
                   get_fsk_fcs_type(),
                   get_fsk_whitening());
    }
  }
  toggle_send_led();
  state = S_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

/******************************************************************************
 * Function to handle the S_RX_PACKET_ERROR state.
 *****************************************************************************/
static void handle_state_rx_packet_error(sl_rail_handle_t rail_handle)
{
  (void) rail_handle;

  app_log_error("Radio RX Error occurred\nEvents: 0x%llX\n", error_code);
  state = S_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

/******************************************************************************
 * Function to handle the S_TX_PACKET_ERROR state.
 *****************************************************************************/
static void handle_state_tx_packet_error(sl_rail_handle_t rail_handle)
{
  (void) rail_handle;

  app_log_error("Radio TX Error occurred\nEvents: 0x%llX\n", error_code);
  state = S_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

/******************************************************************************
 * Function to handle the S_CALIBRATION_ERROR state.
 *****************************************************************************/
static void handle_state_calibration_error(sl_rail_handle_t rail_handle)
{
  (void) rail_handle;
  sl_rail_status_t calibration_status_buff = SL_RAIL_STATUS_NO_ERROR;

  calibration_status_buff = calibration_status;
  app_log_error("Radio Calibration Error occurred\nEvents: 0x%llX\n"
                "sl_rail_calibrate() result: %lu\n",
                error_code,
                calibration_status_buff);
  state = S_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

/******************************************************************************
 * Function to handle the S_IDLE state.
 *****************************************************************************/
static void handle_state_idle(sl_rail_handle_t rail_handle)
{
  sl_rail_status_t rail_status = SL_RAIL_STATUS_NO_ERROR;
  sl_status_t status = SL_STATUS_OK;
  mode_switch_state_t ms_state = get_ms_state();

  if (ms_state == MS_RETURN_TO_BASE_PHY) {
    tx_requested = true;
  }

  if (ms_state == MS_REQUESTED) {
    status = trig_mode_switch_tx(rail_handle);
    if (rail_status != SL_STATUS_OK) {
      app_log_warning("ERROR sl_rail_ieee802154_compute_channel_from_phy_mode_id: %d\n",
                      (uint16_t)status);
    }
    // ms_state is changed on success
    ms_state = get_ms_state();
  }

  if (ms_state == MS_INITIATED) {
    tx_requested = true;
  }

  if (tx_requested) {
    if (ms_state == MS_RETURN_TO_BASE_PHY) {
      prepare_packet(rail_handle,
                     ms_end_packet,
                     sizeof(ms_end_packet),
                     get_phy_modulation_from_channel(get_channel()));
      set_ms_state(MS_SENDING_MS_END_PACKET);
    } else {
      prepare_packet(rail_handle,
                     out_packet,
                     sizeof(out_packet),
                     get_phy_modulation_from_channel(get_channel()));
    }
    rail_status = sl_rail_start_tx(rail_handle,
                                   get_channel(),
                                   SL_RAIL_TX_OPTIONS_DEFAULT,
                                   NULL);
    if (rail_status == SL_RAIL_STATUS_NO_ERROR) {
      app_log_info("sl_rail_start_tx() ok\n");
      ms_state = get_ms_state();
      if (ms_state == MS_INITIATED) {
        set_ms_state(MS_SENDING_MS_START_PACKET);
        if (!packet_sent) {
          uint16_t i = 0;
          for (i = 0; i < 20000; i++) {
            sl_power_manager_sleep();
            if (packet_sent) {
              break;
            }
          }
          if (i >= 20000) {
            app_log_info("Timeout happened\n");
          }
        }
        prepare_packet(rail_handle,
                       out_packet,
                       sizeof(out_packet),
                       get_phy_modulation_from_channel(get_ms_new_channel()));
        rail_status = sl_rail_start_tx(rail_handle,
                                       get_ms_new_channel(),
                                       SL_RAIL_TX_OPTIONS_DEFAULT,
                                       NULL);
        if (rail_status == SL_RAIL_STATUS_NO_ERROR) {
          app_log_info("sl_rail_start_tx() ok\n");
        } else {
          app_log_warning("ERROR sl_rail_start_tx() result: %lu\n", rail_status);
        }
      }
    } else {
      app_log_warning("ERROR sl_rail_start_tx() result: %lu\n", rail_status);
    }
    tx_requested = false;
#if defined(SL_CATALOG_KERNEL_PRESENT)
    app_task_notify();
#endif
  }
}

/******************************************************************************
 * The API forwards the received rx packet on CLI
 *****************************************************************************/
static void printf_rx_packet(const uint8_t * const rx_buffer, uint16_t length)
{
  uint16_t current_channel = get_channel();

  app_log_info("Packet has been received on channel %d:\n", current_channel);
  for (uint16_t i = 0; i < length; i++) {
    app_log_info("0x%02X", rx_buffer[i]);
    if (i < length - 1) {
      app_log_info(", ");
    } else {
      app_log_info("\n");
    }
  }
  if (get_print_packet_details()) {
    if (get_phy_modulation_from_channel(get_channel()) == M_OFDM) {
      app_log_info("With rate: %d and scramble: %d\n",
                   get_ofdm_rate(),
                   get_ofdm_scrambler());
    } else {
      app_log_info("With fcs type: %d and whitening: %d\n",
                   get_fsk_fcs_type(),
                   get_fsk_whitening());
    }
  }
}
