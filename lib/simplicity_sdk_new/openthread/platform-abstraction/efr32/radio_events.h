/*
 *  Copyright (c) 2025, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file defines the radio events module interface for handling RAIL
 *   event processing and callbacks.
 */

#ifndef RADIO_EVENTS_H
#define RADIO_EVENTS_H

#include <openthread-core-config.h>

#include "sl_rail.h"
#include "sl_rail_types.h"
#include "openthread/instance.h"
#include "openthread/platform/radio.h"
#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
#include "sl_rail_util_ieee802154_stack_event.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Internal state flags (needed for event processing)
#define FLAG_WAITING_FOR_ACK 0x00000008
#define FLAG_SCHEDULED_RX_PENDING 0x00000020
#define FLAG_SCHEDULED_TX_PENDING 0x00000040

// Radio event type definitions
// These represent different types of events that can occur during radio operations
#define EVENT_TX_SUCCESS 0x00000100
#define EVENT_TX_CCA_FAILED 0x00000200
#define EVENT_TX_NO_ACK 0x00000400
#define EVENT_TX_SCHEDULER_ERROR 0x00000800
#define EVENT_TX_FAILED 0x00001000
#define EVENT_ACK_SENT_WITH_FP_SET 0x00002000
#define EVENT_SECURED_ACK_SENT 0x00004000
#define EVENT_SCHEDULED_RX_STARTED 0x00008000
#define EVENT_SCHEDULED_TX_STARTED 0x00010000

#define TX_WAITING_FOR_ACK 0x00
#define TX_NO_ACK 0x01

#define RADIO_TX_EVENTS \
    (EVENT_TX_SUCCESS | EVENT_TX_CCA_FAILED | EVENT_TX_NO_ACK | EVENT_TX_SCHEDULER_ERROR | EVENT_TX_FAILED)

/**
 * @brief Initialize the radio events module
 *
 * Sets up event configuration and initializes internal state.
 * Must be called during radio initialization.
 */
void sli_ot_radio_events_init(void);

/**
 * @brief Deinitialize the radio events module
 *
 * Cleans up event configuration and resets internal state.
 * Must be called during radio deinitialization.
 */
void sli_ot_radio_events_deinit(void);

/**
 * @brief Main RAIL event callback function
 *
 * This is the primary callback function that processes all RAIL events.
 * It routes events to appropriate handlers and manages event state.
 *
 * @param aRailHandle RAIL handle
 * @param aEvents Event mask indicating which events occurred
 */
void sli_ot_radio_events_process_callback(sl_rail_handle_t aRailHandle, sl_rail_events_t aEvents);

/**
 * @brief Update RAIL event configuration
 *
 * Modifies the current event configuration by applying a mask and values.
 *
 * @param mask Bit mask indicating which events to modify
 * @param values New values for the masked events
 */
void sli_ot_radio_events_update_config(sl_rail_events_t mask, sl_rail_events_t values);

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
/**
 * @brief Handle PHY stack events
 *
 * Processes IEEE 802.15.4 stack events and routes them to appropriate handlers.
 *
 * @param stackEvent The stack event that occurred
 * @param supplement Additional event data
 * @return The processed stack event
 */
void sli_ot_radio_events_handle_phy_stack_event(sl_rail_util_ieee802154_stack_event_t stackEvent, uint32_t supplement);

// Special version that returns status - used only for PTA grant checking
sl_rail_util_ieee802154_stack_event_t sli_ot_radio_events_handle_phy_stack_event_with_status(
    sl_rail_util_ieee802154_stack_event_t stackEvent,
    uint32_t                              supplement);
#else
#define sli_ot_radio_events_handle_phy_stack_event(event, supplement) ((void)0)
#define sli_ot_radio_events_handle_phy_stack_event_with_status(event, supplement) (0)
#endif

/**
 * @brief Process TX-related events
 *
 * Handles transmission events including packet sent, channel busy, blocked, etc.
 *
 * @param aEvents Event mask containing TX events
 */
void sli_ot_radio_events_process_tx_events(sl_rail_events_t aEvents);

/**
 * @brief Process RX-related events
 *
 * Handles reception events including packet received, sync detected, etc.
 *
 * @param aEvents Event mask containing RX events
 */
void sli_ot_radio_events_process_rx_events(sl_rail_events_t aEvents);

/**
 * @brief Process scheduled TX events
 *
 * Handles scheduled transmission events for Thread 1.2+.
 *
 * @param aEvents Event mask containing scheduled TX events
 */
void sli_ot_radio_events_process_scheduled_tx_events(sl_rail_events_t aEvents);

/**
 * @brief Process scheduled RX events
 *
 * Handles scheduled reception events for Thread 1.2+.
 *
 * @param aEvents Event mask containing scheduled RX events
 */
void sli_ot_radio_events_process_scheduled_rx_events(sl_rail_events_t aEvents);

/**
 * @brief Process ACK-related events
 *
 * Handles ACK transmission and reception events.
 *
 * @param aEvents Event mask containing ACK events
 */
void sli_ot_radio_events_process_ack_events(sl_rail_events_t aEvents);

/**
 * @brief Process coexistence events
 *
 * Handles coexistence-related events if coexistence is enabled.
 *
 * @param aEvents Event mask containing coexistence events
 */
void sli_ot_radio_events_process_coex_events(sl_rail_events_t aEvents);

/**
 * @brief Process data request command events
 *
 * Handles IEEE 802.15.4 data request command events.
 *
 * @param aRailHandle RAIL handle
 * @param aEvents Event mask containing data request events
 */
void sli_ot_radio_events_process_data_request_events(sl_rail_handle_t aRailHandle, sl_rail_events_t aEvents);

/**
 * @brief Process error events
 *
 * Handles various error conditions and frame corruption events.
 *
 * @param aEvents Event mask containing error events
 */
void sli_ot_radio_events_process_error_events(sl_rail_events_t aEvents);

#ifdef __cplusplus
}
#endif

#endif // RADIO_EVENTS_H
