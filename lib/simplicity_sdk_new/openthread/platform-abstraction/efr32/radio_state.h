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
 *   This file defines the radio state management interface for the EFR32 platform.
 */

#ifndef RADIO_STATE_H
#define RADIO_STATE_H

#include <openthread-core-config.h>

#include <stdbool.h>
#include <stdint.h>

#include "sl_rail_types.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

// Note: Internal state flags (FLAG_*) are private implementation details defined in radio_state.cpp
// Note: Radio event types (EVENT_*) are defined in radio_events.h

/**
 * Set or clear an internal radio flag
 *
 * @param[in] aFlag  The flag to set or clear
 * @param[in] aVal   True to set the flag, false to clear it
 */
void sli_ot_radio_state_set_internal_flag(uint32_t aFlag, bool aVal);

/**
 * Check if an internal radio flag is set
 *
 * @param[in] aFlag  The flag to check
 *
 * @returns True if the flag is set, false otherwise
 */
bool sli_ot_radio_state_get_internal_flag(uint32_t aFlag);

/**
 * Set the radio to idle state
 */
void sli_ot_radio_state_set_idle(void);

/**
 * Check if the radio is currently transmitting (data or ACK)
 *
 * @returns True if transmitting, false otherwise
 */
bool sli_ot_radio_state_is_transmitting(void);

/**
 * Check if the radio is currently busy (transmitting, scanning, or scheduled)
 *
 * This checks for:
 * - Energy scan in progress
 * - TX data or ACK ongoing
 * - TX events pending
 * - Scheduled RX pending
 * - Scheduled TX pending
 *
 * @returns True if the radio is busy, false otherwise
 */
bool sli_ot_radio_state_is_transmitting_or_scanning(void);

/**
 * Check if the radio is waiting for an ACK
 *
 * @returns True if waiting for ACK, false otherwise
 */
bool sli_ot_radio_state_is_waiting_for_ack(void);

/**
 * Check if a scheduled TX is pending or has started
 *
 * @returns True if a scheduled TX is pending or started, false otherwise
 */
bool sli_ot_radio_state_is_tx_scheduled(void);

/**
 * Set the scheduled RX pending flag
 *
 * @param[in] aPending  True to set the flag, false to clear it
 */
void sli_ot_radio_state_set_scheduled_rx_pending(bool aPending);

/**
 * Check if a scheduled RX is pending
 *
 * @returns True if a scheduled RX is pending, false otherwise
 */
bool sli_ot_radio_state_is_rx_scheduled(void);

/**
 * Set the scheduled RX started flag
 *
 * @param[in] aStarted  True to set the flag, false to clear it
 */
void sli_ot_radio_state_set_scheduled_rx_started(bool aStarted);

/**
 * Check if the radio is currently receiving a frame
 *
 * @returns True if receiving, false otherwise
 */
bool sli_ot_radio_state_is_receiving_frame(void);

// ============================================================================
// Radio Initialization State
// ============================================================================

/**
 * Check if the radio has been initialized
 *
 * @returns True if initialized, false otherwise
 */
bool sli_ot_radio_state_is_initialized(void);

/**
 * Mark the radio as initialized
 */
void sli_ot_radio_state_mark_initialized(void);

// ============================================================================
// TX Data State
// ============================================================================

/**
 * Check if a TX data operation is ongoing
 *
 * @returns True if TX data is ongoing, false otherwise
 */
bool sli_ot_radio_state_is_tx_data_ongoing(void);

/**
 * Set or clear the TX data ongoing flag
 *
 * @param[in] aOngoing  True to set, false to clear
 */
void sli_ot_radio_state_set_tx_data_ongoing(bool aOngoing);

// ============================================================================
// TX ACK State
// ============================================================================

/**
 * Check if a TX ACK operation is ongoing
 *
 * @returns True if TX ACK is ongoing, false otherwise
 */
bool sli_ot_radio_state_is_tx_ack_ongoing(void);

/**
 * Set or clear the TX ACK ongoing flag
 *
 * @param[in] aOngoing  True to set, false to clear
 */
void sli_ot_radio_state_set_tx_ack_ongoing(bool aOngoing);

// ============================================================================
// CSMA State
// ============================================================================

/**
 * Check if the current TX is using CSMA
 *
 * @returns True if using CSMA, false otherwise
 */
bool sli_ot_radio_state_is_using_csma(void);

/**
 * Set or clear the CSMA usage flag
 *
 * @param[in] aUseCsma  True to set, false to clear
 */
void sli_ot_radio_state_set_using_csma(bool aUseCsma);

// ============================================================================
// Waiting for ACK State
// ============================================================================

/**
 * Set or clear the waiting for ACK flag
 *
 * @param[in] aWaiting  True to set, false to clear
 */
void sli_ot_radio_state_set_waiting_for_ack(bool aWaiting);

// ============================================================================
// Scheduled TX State
// ============================================================================

/**
 * Set or clear the scheduled TX pending flag
 *
 * @param[in] aPending  True to set, false to clear
 */
void sli_ot_radio_state_set_scheduled_tx_pending(bool aPending);

/**
 * Set or clear the scheduled TX started flag
 *
 * @param[in] aStarted  True to set, false to clear
 */
void sli_ot_radio_state_set_scheduled_tx_started(bool aStarted);

// ============================================================================
// TX Events
// ============================================================================

/**
 * Check if any TX events are pending
 *
 * @returns True if any TX events are set, false otherwise
 */
bool sli_ot_radio_state_has_tx_events(void);

/**
 * Clear all TX event flags
 */
void sli_ot_radio_state_clear_all_tx_events(void);

/**
 * Check if TX success event is set
 *
 * @returns True if TX success event is set, false otherwise
 */
bool sli_ot_radio_state_has_tx_success(void);

/**
 * Set or clear the TX success event
 *
 * @param[in] aSuccess  True to set, false to clear
 */
void sli_ot_radio_state_set_tx_success(bool aSuccess);

/**
 * Check if TX CCA failed event is set
 *
 * @returns True if TX CCA failed event is set, false otherwise
 */
bool sli_ot_radio_state_has_tx_cca_failed(void);

/**
 * Set or clear the TX CCA failed event
 *
 * @param[in] aFailed  True to set, false to clear
 */
void sli_ot_radio_state_set_tx_cca_failed(bool aFailed);

/**
 * Check if TX no ACK event is set
 *
 * @returns True if TX no ACK event is set, false otherwise
 */
bool sli_ot_radio_state_has_tx_no_ack(void);

/**
 * Set or clear the TX no ACK event
 *
 * @param[in] aNoAck  True to set, false to clear
 */
void sli_ot_radio_state_set_tx_no_ack(bool aNoAck);

/**
 * Check if TX failed event is set
 *
 * @returns True if TX failed event is set, false otherwise
 */
bool sli_ot_radio_state_has_tx_failed(void);

/**
 * Set or clear the TX failed event
 *
 * @param[in] aFailed  True to set, false to clear
 */
void sli_ot_radio_state_set_tx_failed(bool aFailed);

// ============================================================================
// Combined Operations
// ============================================================================

/**
 * Clear TX data ongoing, waiting for ACK, and scheduled TX started flags
 */
void sli_ot_radio_state_clear_tx_data_and_wait_for_ack(void);

/**
 * Clear all scheduled RX/TX event flags
 */
void sli_ot_radio_state_clear_all_scheduled_events(void);

/**
 * Set the emPendingData flag
 *
 * @param[in] aPending  True to set pending data flag, false to clear it
 */
void sli_ot_radio_state_set_em_pending_data(bool aPending);

/**
 * Check if emPendingData flag is set
 *
 * @returns True if emPendingData is set, false otherwise
 */
bool sli_ot_radio_state_get_em_pending_data(void);

/**
 * Initialize the radio state module
 */
void sli_ot_radio_state_init(void);

/**
 * Deinitialize the radio state module
 */
void sli_ot_radio_state_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // RADIO_STATE_H
