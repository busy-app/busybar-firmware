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
 *   This file defines the CSL (Coordinated Sampled Listening) interface for the EFR32 radio platform.
 */

#ifndef RADIO_CSL_H
#define RADIO_CSL_H

#include <openthread-core-config.h>

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2

#include <stdbool.h>
#include <stdint.h>
#include <openthread/error.h>
#include <openthread/instance.h>
#include <openthread/platform/radio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the CSL module.
 * Resets CSL state for all instances.
 */
void sli_ot_radio_csl_init(void);

/**
 * Deinitialize the CSL module.
 * Resets CSL state for all instances.
 */
void sli_ot_radio_csl_deinit(void);

/**
 * Reset CSL state for a specific instance.
 *
 * @param[in] aInstance  The OpenThread instance.
 */
void sli_ot_radio_csl_reset_state(otInstance *aInstance);

/**
 * Set the CSL peer address for the given instance.
 *
 * @param[in] aInstance      The OpenThread instance.
 * @param[in] aShortAddress  The short address of the CSL peer.
 * @param[in] aExtAddress    The extended address of the CSL peer.
 */
void sli_ot_radio_csl_set_peer_address(otInstance *aInstance, uint16_t aShortAddress, const otExtAddress *aExtAddress);

/**
 * Set the CSL present flag for the given instance.
 *
 * @param[in] aInstance  The OpenThread instance.
 * @param[in] aPresent   True if CSL is present, false otherwise.
 */
void sli_ot_radio_csl_set_present(otInstance *aInstance, bool aPresent);

/**
 * Get the CSL present flag for the given instance.
 *
 * @param[in] aInstance  The OpenThread instance.
 *
 * @return True if CSL is present, false otherwise.
 */
bool sli_ot_radio_csl_get_present(otInstance *aInstance);

/**
 * Check if the source address in a frame matches the CSL receiver peer.
 *
 * @param[in] aInstance  The OpenThread instance.
 * @param[in] aFrame     The radio frame to check.
 *
 * @return True if the source address matches the CSL peer, false otherwise.
 */
bool sli_ot_radio_csl_src_addr_matches_peer(otInstance *aInstance, otRadioFrame *aFrame);

#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE

/**
 * Set the CSL period for the given instance.
 *
 * @param[in] aInstance  The OpenThread instance.
 * @param[in] aPeriod    The CSL period in units of 10 symbols.
 */
void sli_ot_radio_csl_set_period(otInstance *aInstance, uint32_t aPeriod);

/**
 * Get the CSL period for the given instance.
 *
 * @param[in] aInstance  The OpenThread instance.
 *
 * @return The CSL period in units of 10 symbols.
 */
uint32_t sli_ot_radio_csl_get_period(otInstance *aInstance);

/**
 * Set the CSL sample time for the given instance.
 *
 * @param[in] aInstance      The OpenThread instance.
 * @param[in] aSampleTime    The CSL sample time.
 */
void sli_ot_radio_csl_set_sample_time(otInstance *aInstance, uint32_t aSampleTime);

/**
 * Calculate the CSL phase for the given instance.
 *
 * @param[in] aInstance    The OpenThread instance.
 * @param[in] aShrTxTime   The SHR transmission time.
 *
 * @return The CSL phase in units of 10 symbols.
 */
uint16_t sli_ot_radio_csl_get_phase(otInstance *aInstance, uint32_t aShrTxTime);

/**
 * Generate CSL IE data for enhanced ACK.
 * This function also sets the CSL present flag based on period and source address match.
 *
 * @param[in]  aInstance        The OpenThread instance.
 * @param[in]  aReceivedFrame   The received frame to check for address match.
 * @param[out] aIeData          Buffer to store the generated IE data.
 *
 * @return The length of the generated IE data.
 */
uint8_t sli_ot_radio_csl_generate_ack_ie_data(otInstance *aInstance, otRadioFrame *aReceivedFrame, uint8_t *aIeData);

/**
 * Update the CSL IE in an enhanced ACK frame.
 *
 * @param[in]    aInstance              The OpenThread instance.
 * @param[inout] aEnhAckFrame           The enhanced ACK frame to update.
 * @param[in]    aRxTimestamp           The RX timestamp.
 * @param[in]    aPacketBytes           The number of received packet bytes.
 * @param[in]    aReceivedFrameLength   The received frame length.
 */
void sli_ot_radio_csl_update_enh_ack_ie(otInstance   *aInstance,
                                        otRadioFrame *aEnhAckFrame,
                                        uint32_t      aRxTimestamp,
                                        uint16_t      aPacketBytes,
                                        uint8_t       aReceivedFrameLength);

#endif // OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE

#ifdef __cplusplus
}
#endif

#endif // OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2

#endif // RADIO_CSL_H
