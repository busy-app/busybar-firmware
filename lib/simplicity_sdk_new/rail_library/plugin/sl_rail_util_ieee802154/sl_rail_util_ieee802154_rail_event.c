/***************************************************************************//**
 * @file
 * @brief RAIL event hanlder used to map RAIL events to 802.15.4 stack events
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#include "sl_rail.h"
#include "rail.h"
#include "sl_rail_util_ieee802154_stack_event.h"

static bool ack_waiting = false;

static inline bool is_receiving_frame(sl_rail_handle_t rail_handle)
{
  return (sl_rail_get_radio_state(rail_handle) & SL_RAIL_RF_STATE_RX_ACTIVE)
         == SL_RAIL_RF_STATE_RX_ACTIVE;
}

void sl_rail_util_ieee801254_on_rail_event(sl_rail_handle_t railHandle, sl_rail_events_t events)
{
  if (events & (SL_RAIL_EVENT_RX_SYNC_0_DETECT
                | SL_RAIL_EVENT_RX_SYNC_1_DETECT)) {
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_STARTED,
                                            (uint32_t) is_receiving_frame(railHandle));
  }
  if (events & SL_RAIL_EVENT_RX_FILTER_PASSED) {
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACCEPTED,
                                            (uint32_t) is_receiving_frame(railHandle));
  }
  if (events & SL_RAIL_EVENT_SIGNAL_DETECTED) {
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_SIGNAL_DETECTED, 0U);
  }
  if (events & (SL_RAIL_EVENT_TX_CHANNEL_BUSY | SL_RAIL_EVENT_TX_BLOCKED)) {
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_BLOCKED,
                                            (uint32_t) sl_rail_is_auto_ack_waiting_for_ack(railHandle));
  }
  if (events & (SL_RAIL_EVENT_TX_UNDERFLOW | SL_RAIL_EVENT_TX_ABORTED)) {
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ABORTED,
                                            (uint32_t) sl_rail_is_auto_ack_waiting_for_ack(railHandle));
  }
  if (events & SL_RAIL_EVENT_RX_ACK_TIMEOUT) {
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ACK_TIMEDOUT, 0);
  }
  if ((events & SL_RAIL_EVENT_TX_PACKET_SENT) != SL_RAIL_EVENTS_NONE) {
    ack_waiting = sl_rail_is_auto_ack_waiting_for_ack(railHandle);
    (void) sl_rail_util_ieee802154_on_event((ack_waiting
                                             ? SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ACK_WAITING
                                             : SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ENDED), 0U);
  }
  if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
    if (ack_waiting
        || !sl_rail_is_auto_ack_enabled(railHandle)
        || sl_rail_is_rx_auto_ack_paused(railHandle)) {
      (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ENDED,
                                              (uint32_t) is_receiving_frame(railHandle));
    }
    if (ack_waiting) {
      ack_waiting = false;
      (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ACK_RECEIVED, 0U);
    }
  }
  if (events & SL_RAIL_EVENT_TX_START_CCA) {
    // We are starting RXWARM for a CCA check
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_CCA_SOON, 0U);
  }
  if (events & SL_RAIL_EVENT_TX_CCA_RETRY) {
    // We failed a CCA check and need to retry
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_CCA_BUSY, 0U);
  }
  if (events & SL_RAIL_EVENT_TX_CHANNEL_CLEAR) {
    // We're going on-air
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_STARTED, 0U);
  }
  if (events & SL_RAIL_EVENT_RX_FRAME_ERROR) {
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_CORRUPTED,
                                            (uint32_t) is_receiving_frame(railHandle));
  }
  // The following 3 events cause us to not receive a packet
  if (events & (SL_RAIL_EVENT_RX_PACKET_ABORTED
                | SL_RAIL_EVENT_RX_ADDRESS_FILTERED
                | SL_RAIL_EVENT_RX_FIFO_OVERFLOW)) {
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_FILTERED,
                                            (uint32_t) is_receiving_frame(railHandle));
  }
  if (events & SL_RAIL_EVENT_RX_ACK_TIMEOUT) {
    ack_waiting = false;
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ACK_TIMEDOUT, 0);
  }
  if (events & SL_RAIL_EVENT_TXACK_PACKET_SENT) {
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACK_SENT,
                                            (uint32_t) is_receiving_frame(railHandle));
  }
  if (events & (SL_RAIL_EVENT_TXACK_ABORTED | SL_RAIL_EVENT_TXACK_UNDERFLOW)) {
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACK_ABORTED,
                                            (uint32_t) is_receiving_frame(railHandle));
  }
  if (events & SL_RAIL_EVENT_TXACK_BLOCKED) {
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACK_BLOCKED,
                                            (uint32_t) is_receiving_frame(railHandle));
  }
  if (events & SL_RAIL_EVENT_CONFIG_UNSCHEDULED) {
    (void) sl_rail_util_ieee802154_on_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_IDLED, 0U);
  }
}
