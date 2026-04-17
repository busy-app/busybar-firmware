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
#include "sl_rail.h"
#include "sl_component_catalog.h"
#include "app_init.h"
#include "app_process.h"
#include "sl_rail_util_init.h"
#include "sl_rail_sdk_simple_assistance.h"
#include "sl_rail_sdk_wmbus_support.h"
#include "sl_rail_sdk_wmbus_packet_assembler.h"
#include "em_emu.h"

#if defined(SL_CATALOG_WMBUS_SENSOR_CORE_PRESENT)
#include "sl_rail_sdk_wmbus_sensor_core.h"
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

/// TX buffer length
#define TX_BUFFER_LENGTH (256U)

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
/*******************************************************************************
 * Prepare and send the Wireless M-Bus packet
 *
 * @param[in] rail_handle: which rail handle to use for sending
 * @param[in] length: length of the packet
 * @param[in] send_at: absolute time when to send it
 * @return sl_rail_status_t: status of the transmission
 ******************************************************************************/
sl_rail_status_t send_packet_at(sl_rail_handle_t rail_handle, uint16_t length, uint64_t send_at);

/*******************************************************************************
 * @brief Handles the scheduling of a transmission.
 *
 * This function is responsible for managing the scheduling of a transmission
 * using the provided RAIL handle. It ensures that the transmission is properly
 * scheduled according to the application's requirements.
 *
 * @param[in] rail_handle The RAIL handle used to manage the transmission.
 ******************************************************************************/
void handle_schedule_tx(sl_rail_handle_t rail_handle);

/*******************************************************************************
 * @brief Handles the completion of a transmission.
 *
 * This function is called when a transmission is completed. It performs
 * necessary actions to process the completion of the transmission.
 ******************************************************************************/
void handle_tx_done(void);

/*******************************************************************************
 * @brief Handle the response delay .
 *
 * This function is responsible for managing the delay in responses for the
 * specified RAIL handle.
 *
 * @param[in] rail_handle: The RAIL handle for which the response delay
 * is to be handled.
 ******************************************************************************/
void handle_response_delay(sl_rail_handle_t rail_handle);

/**************************************************************************//**
 * @brief Handle unlimited access.
 *
 * This function configures and schedules an RX window for unlimited access
 * using the RAIL library. It sets the start and end times for the RX window,
 * and enables sleep mode if the gap between RX and TX is sufficient.
 *
 * @param[in] rail_handle The RAIL handle.
 *****************************************************************************/
void handle_unlimited_access(sl_rail_handle_t rail_handle);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// Variables from app_init.c
extern uint16_t rx_channel;
extern uint8_t access_number;

/// Time for calculation for the proper sending timing
sl_rail_time_t last_tx_start_time = 0;
sl_rail_time_t last_tx_end_time = 0;

// TX Buffer
static __ALIGNED(RAIL_FIFO_ALIGNMENT) uint8_t tx_buffer[SL_RAIL_SDK_TX_FIFO_SIZE];

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
/// State machine state
static volatile state_t state = S_SCHEDULE_TX;

/// Contains the last RAIL Rx/Tx error events
static volatile uint64_t current_rail_err = 0U;

/// Contains the status of RAIL Calibration
static volatile sl_rail_status_t calibration_status = SL_RAIL_STATUS_NO_ERROR;

/// Variable to allow to go to sleep
static volatile bool ok_to_sleep = true;

/// Last sent packet length, need for calculating the next one
static uint16_t last_tx_length = 0U;

/// Wireless M-Bus specific parameters
static uint64_t wmbus_app_period_acc = 500e3;
static const uint32_t wmbus_app_period_nom = 10e6; //10s

/// Start the rx window earlier and close it later by this amount
static const uint32_t response_delay_safety_margin = 200U;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/*******************************************************************************
 * Set the next state for the process state machine
 * @param[in] next_state: Which state will the sate machine enter next time
 ******************************************************************************/
void set_next_state(state_t next_state)
{
  state = next_state;
}

/******************************************************************************
 * Application state machine, called infinitely
 *****************************************************************************/
void app_process_action(void)
{
  // Get RAIL handle, used later by the application
  sl_rail_handle_t rail_handle = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);

  switch (state) {
    case S_SCHEDULE_TX:
      handle_schedule_tx(rail_handle);
      break;
    case S_TX_DONE:
      handle_tx_done();
      break;
    case S_RESPONSE_DELAY:
      handle_response_delay(rail_handle);
      break;
    case S_UNLIMITED_ACCESS:
      handle_unlimited_access(rail_handle);
      break;
    //in idle states (like S_RX, we need the main oscillator, but waiting for an interrupt. EM1 is safe)
    case S_IDLE_OR_RX:
      // go to sleep or back to rx listening
      break;
    default:
      // unknown state
      break;
  }
}

/******************************************************************************
 * Manages the power_manager_sleep in main.c
 *****************************************************************************/
bool app_is_ok_to_sleep(void)
{
  return ok_to_sleep;
}

/******************************************************************************
 * RAIL callback, called if a RAIL event occurs
 *****************************************************************************/
SL_CODE_RAM void sl_rail_util_on_event(sl_rail_handle_t rail_handle, sl_rail_events_t events)
{
  if ( events & SL_RAIL_EVENT_TX_STARTED) {
    sl_rail_tx_packet_details_t packet_details;
    sl_rail_get_tx_packet_details(rail_handle, &packet_details);
    packet_details.time_sent.total_packet_bytes = RAIL_TX_STARTED_BYTES;
    sl_rail_get_tx_time_preamble_start(rail_handle, &packet_details);
    last_tx_start_time = packet_details.time_sent.packet_time;
  }

  if ( events & SL_RAIL_EVENTS_TX_COMPLETION ) {
    if ( events & SL_RAIL_EVENT_TX_PACKET_SENT ) {
      sl_rail_tx_packet_details_t packet_details;
      sl_rail_get_tx_packet_details(rail_handle, &packet_details);
      packet_details.time_sent.total_packet_bytes = last_tx_length;
      sl_rail_get_tx_time_frame_end(rail_handle, &packet_details);
      last_tx_end_time = packet_details.time_sent.packet_time;
    }
    state = S_TX_DONE;
  }

  if ( events & SL_RAIL_EVENTS_RX_COMPLETION ) {
    if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
      toggle_receive_led();
    }
    switch (sl_rail_sdk_wmbus_get_accessibility()) {
      case WMBUS_ACCESSIBILITY_LIMITED_ACCESS:
        state = S_SCHEDULE_TX;
        break;
      case WMBUS_ACCESSIBILITY_UNLIMITED_ACCESS:
        state = S_IDLE_OR_RX;
        break;
      default:
        state = S_IDLE_OR_RX;
    }
  }

  if ( events & SL_RAIL_EVENT_RX_SCHEDULED_RX_END ) {
    state = S_SCHEDULE_TX;
  }

  // Perform all calibrations when needed
  if ( events & SL_RAIL_EVENT_CAL_NEEDED ) {
    calibration_status = sl_rail_calibrate(rail_handle, NULL, SL_RAIL_CAL_ALL_PENDING);
    if (calibration_status != SL_RAIL_STATUS_NO_ERROR) {
      current_rail_err = (events & SL_RAIL_EVENT_CAL_NEEDED);
    }
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

/*******************************************************************************
 * @brief Handles the scheduling of a transmission.
 *
 * This function is responsible for managing the scheduling of a transmission
 * using the provided RAIL handle. It ensures that the transmission is properly
 * scheduled according to the application's requirements.
 *
 * @param[in] rail_handle The RAIL handle used to manage the transmission.
 ******************************************************************************/
void handle_schedule_tx(sl_rail_handle_t rail_handle)
{
  uint16_t length = 0;
  sl_rail_status_t rail_status;
  memset(tx_buffer, 0, SL_RAIL_SDK_TX_FIFO_SIZE);

#if defined(SL_CATALOG_WMBUS_SENSOR_CORE_PRESENT)
  length = sl_rail_sdk_wmbus_sensor_core_process(tx_buffer, &access_number);
#else
  uint32_t data = 1234;
  sl_rail_sdk_wmbus_sensor_data_t sensors_data = {
    .data = 1234,
    .vif = 0x17,
    .next = NULL
  };
  length = sl_rail_sdk_wmbus_setup_frame(tx_buffer, *access_number, sl_rail_sdk_wmbus_get_accessibility(), WMBUS_DEVICE_WATER_METER, &sensors_data, true, true);
#endif
  // TX is scheduled, sleep can be enabled as RAIL deals with power manager
  rail_status = send_packet_at(rail_handle, length, last_tx_start_time + wmbus_app_period_acc);

  if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
    // TX failed, schedule the next TX immediately
    last_tx_start_time = sl_rail_get_time(rail_handle);
    state = S_SCHEDULE_TX;
  } else {
    // TX is scheduled, sleep can be enabled as RAIL deals with power manager
    ok_to_sleep = true;
    state = S_IDLE_OR_RX;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

/*******************************************************************************
 * @brief Handles the completion of a transmission.
 *
 * This function is called when a transmission is completed. It performs
 * necessary actions to process the completion of the transmission.
 ******************************************************************************/
void handle_tx_done(void)
{
  toggle_send_led();
  //let's setup the time for the next tx.
  /*
   * from EN13757-4:
   * tACC=1+(|nACC-128|-64)/2048*tNOM
   * a bit easier to calculate:
   * tACC=(2048+|nACC-128|-64)*tNOM/2048
   */
  access_number++;
  wmbus_app_period_acc = access_number > 128 ? access_number - 128 : 128 - access_number;
  wmbus_app_period_acc = (uint64_t)(2048 + wmbus_app_period_acc - 64) * wmbus_app_period_nom / 2048;
  switch (sl_rail_sdk_wmbus_get_accessibility()) {
    case WMBUS_ACCESSIBILITY_LIMITED_ACCESS:
      state = S_RESPONSE_DELAY;
      break;
    case WMBUS_ACCESSIBILITY_UNLIMITED_ACCESS:
      state = S_UNLIMITED_ACCESS;
      break;
    // no RX between TXes, schedule the next TX immediately
    default:
      state = S_SCHEDULE_TX;
      break;
  }
  ok_to_sleep = false;
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

/*******************************************************************************
 * @brief Handle the response delay .
 *
 * This function is responsible for managing the delay in responses for the specified RAIL handle.
 *
 * @param[in] rail_handle: The RAIL handle for which the response delay is to be handled.
 ******************************************************************************/
void handle_response_delay(sl_rail_handle_t rail_handle)
{
  sl_rail_status_t rail_status;
  sl_rail_scheduled_rx_config_t schedule = {
    .start = last_tx_end_time + sl_rail_sdk_wmbus_get_meter_limited_acc_rx_start(false) - response_delay_safety_margin,
    .start_mode = SL_RAIL_TIME_ABSOLUTE,
    .end = last_tx_end_time + sl_rail_sdk_wmbus_get_meter_limited_acc_rx_stop(false) + response_delay_safety_margin,
    .end_mode = SL_RAIL_TIME_ABSOLUTE,
    .rx_transition_end_schedule = 0U,
    .hard_window_end = 0,
  };
  rail_status = sl_rail_start_scheduled_rx(rail_handle, rx_channel, &schedule, NULL);

  if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
    // RX failed, schedule the next TX immediately
    state = S_SCHEDULE_TX;
  } else {
    // RX is scheduled, sleep can be enabled as RAIL deals with power manager
    ok_to_sleep = true;
    state = S_IDLE_OR_RX;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

/**************************************************************************//**
 * @brief Handle unlimited access.
 *
 * This function configures and schedules an RX window for unlimited access
 * using the RAIL library. It sets the start and end times for the RX window,
 * and enables sleep mode if the gap between RX and TX is sufficient.
 *
 * @param[in] rail_handle The RAIL handle.
 *****************************************************************************/
void handle_unlimited_access(sl_rail_handle_t rail_handle)
{
  sl_rail_status_t rail_status;
  sl_rail_scheduled_rx_config_t schedule = {
    .start =  last_tx_end_time + sl_rail_sdk_wmbus_get_meter_limited_acc_rx_start(false) - response_delay_safety_margin,
    .start_mode = SL_RAIL_TIME_ABSOLUTE,
    .end = (uint32_t)(last_tx_end_time + wmbus_app_period_acc - 2000),
    .end_mode = SL_RAIL_TIME_ABSOLUTE,
    .rx_transition_end_schedule = 0,
    .hard_window_end = 0,
  };
  // RX is scheduled, sleep can be enabled as RAIL deals with power manager
  // although, if the gap between RX and TX is short enough, the device will
  // not have time to go to sleep
  rail_status = sl_rail_start_scheduled_rx(rail_handle, rx_channel, &schedule, NULL);

  if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
    // RX failed, schedule the next TX immediately
    state = S_SCHEDULE_TX;
  } else {
    // RX is scheduled, sleep can be enabled as RAIL deals with power manager
    ok_to_sleep = true;
    state = S_IDLE_OR_RX;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

/*******************************************************************************
 * Prepare and send the Wireless M-Bus packet
 *
 * @param[in] rail_handle: which rail handle to use for sending
 * @param[in] length: length of the packet
 * @param[in] send_at: absolute time when to send it
 * @return sl_rail_status_t: status of the transmission
 ******************************************************************************/
sl_rail_status_t send_packet_at(sl_rail_handle_t rail_handle, uint16_t length, uint64_t send_at)
{
  last_tx_length = length;
  sl_rail_write_tx_fifo(rail_handle, tx_buffer, last_tx_length, true);

  sl_rail_scheduled_tx_config_t schedule = {
    .when = send_at,
    .mode = SL_RAIL_TIME_ABSOLUTE,
  };
  return sl_rail_start_scheduled_tx(rail_handle, DEFAULT_CHANNEL, SL_RAIL_TX_OPTIONS_DEFAULT, &schedule, NULL);
}
