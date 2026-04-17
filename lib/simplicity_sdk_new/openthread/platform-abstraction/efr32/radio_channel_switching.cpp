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

#include <openthread-core-config.h>
/**
 * @file
 *   This file implements fast channel switching support for multi-instance configurations,
 *   enabling concurrent operation on different channels using RAIL's channel switching features.
 */

#include "radio_channel_switching.h"
#include "radio_multi_channel.h"

#include <openthread/platform/radio.h>
#include <openthread/platform/time.h>

#include "common/code_utils.hpp"
#include "common/debug.hpp"
#include "utils/code_utils.h"

#include "platform-efr32.h"
#include "radio_instance.h"
#include "radio_interface.h"

extern "C" {
#include "sl_rail.h"
#include "sl_rail_ieee802154.h"
#include "sl_status.h"
}

// Channel value used to indicate an uninitialized channel slot
#define UNINITIALIZED_CHANNEL 0xFF

// Channel switching configuration and buffer
#if FAST_CHANNEL_SWITCHING_SUPPORT
static sl_rail_ieee802154_rx_channel_switching_cfg_t sChannelSwitchingCfg;
static SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_ALIGNMENT_TYPE
    sChannelSwitchingBuffer[SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_BYTES
                            / SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_ALIGNMENT];
#endif

bool sli_ot_radio_channel_switching_is_multi_channel_enabled(void)
{
#if FAST_CHANNEL_SWITCHING_SUPPORT
    uint8_t firstChannel = UNINITIALIZED_CHANNEL;

    for (uint8_t i = 0U; i < SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_NUM_CHANNELS; i++)
    {
        if (sChannelSwitchingCfg.channels[i] != UNINITIALIZED_CHANNEL)
        {
            if (firstChannel == UNINITIALIZED_CHANNEL)
            {
                firstChannel = sChannelSwitchingCfg.channels[i];
            }
            else if (firstChannel != sChannelSwitchingCfg.channels[i])
            {
                return true;
            }
        }
    }
#endif
    return false;
}

otError sli_ot_radio_channel_switching_get_config(sl_rail_ieee802154_rx_channel_switching_cfg_t *channelSwitchingCfg)
{
#if FAST_CHANNEL_SWITCHING_SUPPORT
    otError error = OT_ERROR_NONE;

    otEXPECT_ACTION(channelSwitchingCfg != nullptr, error = OT_ERROR_INVALID_ARGS);

    memcpy(channelSwitchingCfg, &sChannelSwitchingCfg, sizeof(sChannelSwitchingCfg));

exit:
    return error;
#else
    OT_UNUSED_VARIABLE(channelSwitchingCfg);
    return OT_ERROR_NOT_IMPLEMENTED;
#endif
}

uint8_t sli_ot_radio_channel_switching_get_channel_index(uint8_t aChannel)
{
#if FAST_CHANNEL_SWITCHING_SUPPORT
    for (uint8_t i = 0U; i < SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_NUM_CHANNELS; i++)
    {
        if (sChannelSwitchingCfg.channels[i] == aChannel)
        {
            return i;
        }
    }
#else
    OT_UNUSED_VARIABLE(aChannel);
#endif
    return INVALID_INTERFACE_INDEX;
}

uint8_t sli_ot_radio_channel_switching_get_channel(otInstance *aInstance)
{
#if (FAST_CHANNEL_SWITCHING_SUPPORT && OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE)
    panIndex_t panIndex = sli_ot_radio_instance_get_pan_index(aInstance);
    uint8_t    channel  = sChannelSwitchingCfg.channels[panIndex];
    return channel;
#else
    OT_UNUSED_VARIABLE(aInstance);
    return 0; // Default channel when not supported
#endif
}

void sli_ot_radio_channel_switching_configure(otInstance *aInstance, uint8_t aChannel)
{
#if (FAST_CHANNEL_SWITCHING_SUPPORT && OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE)
    panIndex_t panIndex = sli_ot_radio_instance_get_pan_index(aInstance);
    OT_ASSERT(panIndex < SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_NUM_CHANNELS);
    sChannelSwitchingCfg.channels[panIndex] = aChannel;
#else
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aChannel);
#endif
}

void sli_ot_radio_channel_switching_init(void)
{
#if (FAST_CHANNEL_SWITCHING_SUPPORT && OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE)
    sChannelSwitchingCfg.buffer_bytes = SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_BYTES;
    sChannelSwitchingCfg.p_buffer     = sChannelSwitchingBuffer;
    for (uint8_t i = 0U; i < SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_NUM_CHANNELS; i++)
    {
        sChannelSwitchingCfg.channels[i] = UNINITIALIZED_CHANNEL;
    }
#endif
}

sl_rail_ieee802154_rx_channel_switching_cfg_t *sli_ot_radio_channel_switching_get_config_ptr(void)
{
#if FAST_CHANNEL_SWITCHING_SUPPORT
    return &sChannelSwitchingCfg;
#else
    return nullptr;
#endif
}

void *sli_ot_radio_channel_switching_get_buffer_ptr(void)
{
#if FAST_CHANNEL_SWITCHING_SUPPORT
    return sChannelSwitchingBuffer;
#else
    return nullptr;
#endif
}
