/***************************************************************************//**
 * @file sl_rail_sdk_statistic.c
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include "sl_component_catalog.h"
#include "sl_rail_types.h"
#include "sl_rail_sdk_statistic.h"
#if defined(SL_CATALOG_APP_LOG_PRESENT)
#include "app_log.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/// Macro to help collecting the event happening number
#define SHIFT_AND_ADD(DEST, VALUE, SHIFT) DEST += (uint32_t)((VALUE & (1ULL << SHIFT)) >> SHIFT)

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// Global variable to read out happened events counts
#if defined(SL_CATALOG_CLI_PRESENT)
rail_event_statistic_t rail_event_statistic = {
  .event = {
    .rx = {
      .rssi_average_done.name = "RX_RSSI_AVERAGE_DONE",
      .fifo_almost_full.name = "RX_FIFO_ALMOST_FULL",
      .packet_received.name = "RX_PACKET_RECEIVED",
      .preamble_lost.name = "RX_PREAMBLE_LOST",
      .preamble_detect.name = "RX_PREAMBLE_DETECT",
      .sync1_detect.name = "RX_SYNC1_DETECT",
      .sync2_detect.name = "RX_SYNC2_DETECT",
      .frame_error.name = "RX_FRAME_ERROR",
      .fifo_full.name = "RX_FIFO_FULL",
      .fifo_overflow.name = "RX_FIFO_OVERFLOW",
      .address_filtered.name = "RX_ADDRESS_FILTERED",
      .timeout.name = "RX_TIMEOUT",
      .scheduled_rx_end.name = "RX_SCHEDULED_RX_END",
      .scheduled_rx_missed.name = "RX_SCHEDULED_RX_MISSED",
      .packet_aborted.name = "RX_PACKET_ABORTED",
      .filter_passed.name = "RX_FILTER_PASSED",
      .timing_lost.name = "RX_TIMING_LOST",
      .timing_detect.name = "RX_TIMING_DETECT",
      .channel_hopping_complete.name = "RX_CHANNEL_HOPPING_COMPLETE",
      .duty_cycle_rx_end.name = "RX_DUTY_CYCLE_RX_END",
      .ack_timeout.name = "RX_ACK_TIMEOUT"
    },
    .tx = {
      .fifo_almost_empty.name = "TX_FIFO_ALMOST_EMPTY",
      .packet_sent.name = "TX_PACKET_SENT",
      .aborted.name = "TX_ABORTED",
      .blocked.name = "TX_BLOCKED",
      .underflow.name = "TX_UNDERFLOW",
      .channel_clear.name = "TX_CHANNEL_CLEAR",
      .channel_busy.name = "TX_CHANNEL_BUSY",
      .cca_retry.name = "TX_CCA_RETRY",
      .start_cca.name = "TX_START_CCA",
      .started.name = "TX_STARTED",
      .scheduled_tx_missed.name = "TX_SCHEDULED_TX_MISSED",
      .ack_packet_sent.name = "TX_ACK_PACKET_SENT",
      .ack_aborted.name = "TX_ACK_ABORTED",
      .ack_blocked.name = "TX_ACK_BLOCKED",
      .ack_underflow.name = "TX_ACK_UNDERFLOW"
    },
    .protocol_spec = {
      .ieee802154_data_request_command.name = "IEEE802154_DATA_REQUEST_COMMAND",
      .zwave_beam.name = "ZWAVE_BEAM",
    },
    .multi_protocol = {
      .unscheduled.name = "UNSCHEDULED",
      .scheduled.name = "SCHEDULED",
      .scheduler_status.name = "SCHEDULER_STATUS"
    },
    .maintenance = {
      .cal_needed.name = "CAL_NEEDED",
    }
  }
};
#else
rail_event_statistic_t rail_event_statistic;
#endif

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/*******************************************************************************
 * Set all events statistic to 0
 ******************************************************************************/
void clear_statistic(void)
{
  uint8_t index = 0;
  for (index = 0; index < NUMBER_OF_EVENTS; index++) {
    rail_event_statistic.events[index].value = 0;
  }
}

#if defined(SL_CATALOG_CLI_PRESENT)
/*******************************************************************************
 * Print statistic to USART
 ******************************************************************************/
void print_statistic(void)
{
  uint8_t index = 0;
  app_log_info("RAIL Event Statistics\n");
  for (index = 0; index < NUMBER_OF_EVENTS; index++) {
    if (rail_event_statistic.events[index].value != 0) {
      app_log_info("%s: %d\n", rail_event_statistic.events[index].name, rail_event_statistic.events[index].value);
    }
  }
}
#endif

/*******************************************************************************
 * Collects the statistic form RAIL event
 * @param[in] rail_handle: not used
 * @param[in] events: the events what happened
 ******************************************************************************/
void sl_rail_sdk_statistic_event(sl_rail_handle_t rail_handle, sl_rail_events_t events)
{
  (void)rail_handle;
  SHIFT_AND_ADD(rail_event_statistic.event.rx.rssi_average_done.value, events, SL_RAIL_EVENT_RSSI_AVERAGE_DONE_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.ack_timeout.value, events, SL_RAIL_EVENT_RX_ACK_TIMEOUT_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.fifo_almost_full.value, events, SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.packet_received.value, events, SL_RAIL_EVENT_RX_PACKET_RECEIVED_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.preamble_lost.value, events, SL_RAIL_EVENT_RX_PREAMBLE_LOST_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.preamble_detect.value, events, SL_RAIL_EVENT_RX_PREAMBLE_DETECT_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.sync1_detect.value, events, SL_RAIL_EVENT_RX_SYNC_0_DETECT_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.sync2_detect.value, events, SL_RAIL_EVENT_RX_SYNC_1_DETECT_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.frame_error.value, events, SL_RAIL_EVENT_RX_FRAME_ERROR_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.fifo_full.value, events, SL_RAIL_EVENT_RX_FIFO_FULL_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.fifo_overflow.value, events, SL_RAIL_EVENT_RX_FIFO_OVERFLOW_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.address_filtered.value, events, SL_RAIL_EVENT_RX_ADDRESS_FILTERED_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.timeout.value, events, SL_RAIL_EVENT_RX_TIMEOUT_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.scheduled_rx_end.value, events, SL_RAIL_EVENT_RX_SCHEDULED_RX_END_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.scheduled_rx_missed.value, events, SL_RAIL_EVENT_RX_SCHEDULED_RX_MISSED_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.packet_aborted.value, events, SL_RAIL_EVENT_RX_PACKET_ABORTED_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.filter_passed.value, events, SL_RAIL_EVENT_RX_FILTER_PASSED_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.timing_lost.value, events, SL_RAIL_EVENT_RX_TIMING_LOST_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.timing_detect.value, events, SL_RAIL_EVENT_RX_TIMING_DETECT_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.channel_hopping_complete.value, events, SL_RAIL_EVENT_RX_CHANNEL_HOPPING_COMPLETE_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.rx.duty_cycle_rx_end.value, events, SL_RAIL_EVENT_RX_DUTY_CYCLE_RX_END_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.protocol_spec.ieee802154_data_request_command.value, events, SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.protocol_spec.zwave_beam.value, events, SL_RAIL_EVENT_ZWAVE_BEAM_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.tx.fifo_almost_empty.value, events, SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.tx.packet_sent.value, events, SL_RAIL_EVENT_TX_PACKET_SENT_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.tx.ack_packet_sent.value, events, SL_RAIL_EVENT_TXACK_PACKET_SENT_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.tx.aborted.value, events, SL_RAIL_EVENT_TX_ABORTED_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.tx.ack_aborted.value, events, SL_RAIL_EVENT_TXACK_ABORTED_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.tx.blocked.value, events, SL_RAIL_EVENT_TX_BLOCKED_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.tx.ack_blocked.value, events, SL_RAIL_EVENT_TXACK_BLOCKED_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.tx.underflow.value, events, SL_RAIL_EVENT_TX_UNDERFLOW_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.tx.ack_underflow.value, events, SL_RAIL_EVENT_TXACK_UNDERFLOW_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.tx.channel_clear.value, events, SL_RAIL_EVENT_TX_CHANNEL_CLEAR_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.tx.channel_busy.value, events, SL_RAIL_EVENT_TX_CHANNEL_BUSY_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.tx.cca_retry.value, events, SL_RAIL_EVENT_TX_CCA_RETRY_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.tx.start_cca.value, events, SL_RAIL_EVENT_TX_START_CCA_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.tx.started.value, events, SL_RAIL_EVENT_TX_STARTED_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.tx.scheduled_tx_missed.value, events, SL_RAIL_EVENT_TX_SCHEDULED_TX_MISSED_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.multi_protocol.unscheduled.value, events, SL_RAIL_EVENT_CONFIG_UNSCHEDULED_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.multi_protocol.scheduled.value, events, SL_RAIL_EVENT_CONFIG_SCHEDULED_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.multi_protocol.scheduler_status.value, events, SL_RAIL_EVENT_SCHEDULER_STATUS_SHIFT);
  SHIFT_AND_ADD(rail_event_statistic.event.maintenance.cal_needed.value, events, SL_RAIL_EVENT_CAL_NEEDED_SHIFT);
}
