/***************************************************************************//**
 * @file sl_rail_sdk_statistic.h
 * @brief RAIL SDK - RAIL Event Statistics Component
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

#ifndef SL_RAIL_SDK_STATISTIC_H
#define SL_RAIL_SDK_STATISTIC_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include "sl_component_catalog.h"
#include "sl_rail_types.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/// Max number of the known RAIL events
#define NUMBER_OF_EVENTS (42U)

/// Statistic value and name for gathering and printing
typedef struct statistic {
  uint32_t value;
#if defined(SL_CATALOG_CLI_PRESENT)
  const char* name;
#endif
}statistic_t;

/// The known RAIL events
typedef union rail_event_statistic{
  struct {
    struct rx {
      statistic_t rssi_average_done;
      statistic_t fifo_almost_full;
      statistic_t packet_received;
      statistic_t preamble_lost;
      statistic_t preamble_detect;
      statistic_t sync1_detect;
      statistic_t sync2_detect;
      statistic_t frame_error;
      statistic_t fifo_full;
      statistic_t fifo_overflow;
      statistic_t address_filtered;
      statistic_t timeout;
      statistic_t scheduled_rx_end;
      statistic_t scheduled_rx_missed;
      statistic_t packet_aborted;
      statistic_t filter_passed;
      statistic_t timing_lost;
      statistic_t timing_detect;
      statistic_t channel_hopping_complete;
      statistic_t duty_cycle_rx_end;
      statistic_t ack_timeout;
    } rx;
    struct tx {
      statistic_t fifo_almost_empty;
      statistic_t packet_sent;
      statistic_t aborted;
      statistic_t blocked;
      statistic_t underflow;
      statistic_t channel_clear;
      statistic_t channel_busy;
      statistic_t cca_retry;
      statistic_t start_cca;
      statistic_t started;
      statistic_t scheduled_tx_missed;
      statistic_t ack_packet_sent;
      statistic_t ack_aborted;
      statistic_t ack_blocked;
      statistic_t ack_underflow;
    } tx;
    struct protocol_spec {
      statistic_t ieee802154_data_request_command;
      statistic_t zwave_beam;
    } protocol_spec;
    struct multi_protocol {
      statistic_t unscheduled;
      statistic_t scheduled;
      statistic_t scheduler_status;
    } multi_protocol;
    struct maintenance {
      statistic_t cal_needed;
    } maintenance;
  } event;
  statistic_t events[NUMBER_OF_EVENTS];
} rail_event_statistic_t;

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// Variable to read the statistic
extern rail_event_statistic_t rail_event_statistic;

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/*******************************************************************************
 * Set all events statistic to 0
 ******************************************************************************/
void clear_statistic(void);

/*******************************************************************************
 * Collects the statistic form RAIL event
 * @param[in] rail_handle: not used
 * @param[in] events: the events what happened
 ******************************************************************************/
void sl_rail_sdk_statistic_event(sl_rail_handle_t rail_handle, sl_rail_events_t events);

#if defined(SL_CATALOG_CLI_PRESENT)
/*******************************************************************************
 * Print statistic to USART
 ******************************************************************************/
void print_statistic(void);
#endif

#endif // SL_RAIL_SDK_STATISTIC_H
