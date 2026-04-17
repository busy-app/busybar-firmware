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
 *   This file defines the radio security interface for the EFR32 platform.
 */

#ifndef RADIO_SECURITY_H
#define RADIO_SECURITY_H

#include <openthread-core-config.h>

#include <openthread/platform/radio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize radio security module
 */
void sli_ot_radio_security_init(void);

/**
 * @brief Deinitialize radio security module
 */
void sli_ot_radio_security_deinit(void);

/**
 * @brief Process transmit security for a frame
 *
 * @param[in] aFrame     The radio frame to process
 * @param[in] aInstance  The OpenThread instance
 *
 * @returns otError  OT_ERROR_NONE on success, error code otherwise
 */
otError sli_ot_radio_security_process_transmit(otRadioFrame *aFrame, otInstance *aInstance);

/**
 * @brief Set MAC key for an instance
 *
 * @param[in] aInstance  The OpenThread instance
 * @param[in] aKeyIdMode Key ID mode
 * @param[in] aKeyId     Key ID
 * @param[in] aPrevKey   Previous key material
 * @param[in] aCurrKey   Current key material
 * @param[in] aNextKey   Next key material
 * @param[in] aKeyType   Key type
 */
void sli_ot_radio_security_set_mac_key(otInstance             *aInstance,
                                       uint8_t                 aKeyIdMode,
                                       uint8_t                 aKeyId,
                                       const otMacKeyMaterial *aPrevKey,
                                       const otMacKeyMaterial *aCurrKey,
                                       const otMacKeyMaterial *aNextKey,
                                       otRadioKeyType          aKeyType);

/**
 * @brief Set MAC frame counter for an instance
 *
 * @param[in] aInstance         The OpenThread instance
 * @param[in] aMacFrameCounter  The MAC frame counter value
 */
void sli_ot_radio_security_set_mac_frame_counter(otInstance *aInstance, uint32_t aMacFrameCounter);

/**
 * @brief Set MAC frame counter for an instance if larger than current value
 *
 * @param[in] aInstance         The OpenThread instance
 * @param[in] aMacFrameCounter  The MAC frame counter value
 */
void sli_ot_radio_security_set_mac_frame_counter_if_larger(otInstance *aInstance, uint32_t aMacFrameCounter);

/**
 * @brief Get ACK key ID for an instance
 *
 * @param[in] aInstance  The OpenThread instance
 *
 * @returns uint8_t  The ACK key ID
 */
uint8_t sli_ot_radio_security_get_ack_key_id(otInstance *aInstance);

/**
 * @brief Get ACK frame counter for an instance
 *
 * @param[in] aInstance  The OpenThread instance
 *
 * @returns uint32_t  The ACK frame counter
 */
uint32_t sli_ot_radio_security_get_ack_frame_counter(otInstance *aInstance);

#ifdef __cplusplus
}
#endif

#endif // RADIO_SECURITY_H
