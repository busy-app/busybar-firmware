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
 *   This file defines the fast channel switching interface for the EFR32 radio platform.
 */

#ifndef RADIO_CHANNEL_SWITCHING_H_
#define RADIO_CHANNEL_SWITCHING_H_

#include <openthread-core-config.h>

#include <stdbool.h>
#include <stdint.h>

#include <openthread/error.h>
#include <openthread/instance.h>

#include "radio_multi_channel.h"
#include "sl_rail_ieee802154.h"
#include "sl_rail_types.h"

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT
#include "sl_rail_util_ieee802154_fast_channel_switching_config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Check if multi-channel support is enabled
 *
 * @returns true if multi-channel is enabled, false otherwise
 */
bool sli_ot_radio_channel_switching_is_multi_channel_enabled(void);

/**
 * @brief Get the channel switching configuration
 *
 * @param[out] channelSwitchingCfg  Pointer to store the configuration
 *
 * @returns OT_ERROR_NONE on success, error code on failure
 */
otError sli_ot_radio_channel_switching_get_config(sl_rail_ieee802154_rx_channel_switching_cfg_t *channelSwitchingCfg);

/**
 * @brief Get the channel index for a given channel
 *
 * @param[in] aChannel  The channel number
 *
 * @returns Channel index if found, INVALID_INTERFACE_INDEX if not found
 */
uint8_t sli_ot_radio_channel_switching_get_channel_index(uint8_t aChannel);

/**
 * @brief Get the channel for a specific instance
 *
 * @param[in] aInstance  The OpenThread instance
 *
 * @returns The channel number for the instance
 */
uint8_t sli_ot_radio_channel_switching_get_channel(otInstance *aInstance);

/**
 * @brief Configure channel switching for a specific instance
 *
 * @param[in] aInstance  The OpenThread instance
 * @param[in] aChannel   The channel to configure
 */
void sli_ot_radio_channel_switching_configure(otInstance *aInstance, uint8_t aChannel);

/**
 * @brief Initialize the channel switching module
 *
 * This function initializes the channel switching configuration and buffer.
 * Must be called during radio initialization.
 */
void sli_ot_radio_channel_switching_init(void);

/**
 * @brief Get the channel switching configuration pointer
 *
 * @returns Pointer to the channel switching configuration
 */
sl_rail_ieee802154_rx_channel_switching_cfg_t *sli_ot_radio_channel_switching_get_config_ptr(void);

/**
 * @brief Get the channel switching buffer pointer
 *
 * @returns Pointer to the channel switching buffer
 */
void *sli_ot_radio_channel_switching_get_buffer_ptr(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // RADIO_CHANNEL_SWITCHING_H_
