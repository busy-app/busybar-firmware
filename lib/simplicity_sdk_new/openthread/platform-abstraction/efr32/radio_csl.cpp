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
 *   This file implements the CSL (Coordinated Sampled Listening) support for the EFR32 radio platform.
 */

#include "radio_csl.h"

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2

#include <string.h>
#include <openthread/platform/alarm-micro.h>
#include <openthread/platform/radio.h>
#include "common/code_utils.hpp"
#include "common/debug.hpp"
#include "utils/code_utils.h"
#include "utils/mac_frame.h"

#include "platform-efr32.h"
#include "radio_instance.h"
#include "radio_interface.h"

// CSL transmitter state (always present in Thread 1.2+)
typedef struct
{
    uint16_t     shortAddress; // CSL peer short address
    otExtAddress extAddress;   // CSL peer extended address
    bool         present;      // CSL present flag
#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
    uint32_t period;     // CSL period in units of 10 symbols (receiver only)
    uint32_t sampleTime; // CSL sample time (receiver only)
#endif
} csl_state_t;

// Per-instance CSL state
static csl_state_t sCslState[RADIO_INTERFACE_COUNT];

// Helper function to get CSL state for an instance
static csl_state_t *getCslState(otInstance *aInstance)
{
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    instanceIndex_t index = sli_ot_radio_instance_get_index(aInstance);
    OT_ASSERT(index < RADIO_INTERFACE_COUNT);
    return &sCslState[index];
#else
    OT_UNUSED_VARIABLE(aInstance);
    return &sCslState[0];
#endif
}

void sli_ot_radio_csl_reset_state(otInstance *aInstance)
{
    csl_state_t *state = getCslState(aInstance);

    memset(state, 0, sizeof(csl_state_t));
    state->shortAddress = OT_RADIO_INVALID_SHORT_ADDR;
}

void sli_ot_radio_csl_init(void)
{
    // Reset CSL state for all instances
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    for (size_t i = 0; i < RADIO_INTERFACE_COUNT; i++)
    {
        otInstance *instance = sli_ot_radio_instance_get(i);
        if (instance != nullptr)
        {
            sli_ot_radio_csl_reset_state(instance);
        }
    }
#else
    sli_ot_radio_csl_reset_state(nullptr);
#endif
}

void sli_ot_radio_csl_deinit(void)
{
    // Reset CSL state for all instances
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    for (size_t i = 0; i < RADIO_INTERFACE_COUNT; i++)
    {
        otInstance *instance = sli_ot_radio_instance_get(i);
        if (instance != nullptr)
        {
            sli_ot_radio_csl_reset_state(instance);
        }
    }
#else
    sli_ot_radio_csl_reset_state(nullptr);
#endif
}

void sli_ot_radio_csl_set_peer_address(otInstance *aInstance, uint16_t aShortAddress, const otExtAddress *aExtAddress)
{
    csl_state_t *state  = getCslState(aInstance);
    state->shortAddress = aShortAddress;
    if (aExtAddress != nullptr)
    {
        memcpy(&state->extAddress, aExtAddress, sizeof(otExtAddress));
    }
}

void sli_ot_radio_csl_set_present(otInstance *aInstance, bool aPresent)
{
    csl_state_t *state = getCslState(aInstance);
    state->present     = aPresent;
}

bool sli_ot_radio_csl_get_present(otInstance *aInstance)
{
    csl_state_t *state = getCslState(aInstance);
    return state->present;
}

bool sli_ot_radio_csl_src_addr_matches_peer(otInstance *aInstance, otRadioFrame *aFrame)
{
    csl_state_t *state = getCslState(aInstance);
    otMacAddress src;
    bool         matches = false;

    otEXPECT(otMacFrameGetSrcAddr(aFrame, &src) == OT_ERROR_NONE);

    switch (src.mType)
    {
    case OT_MAC_ADDRESS_TYPE_SHORT:
        if (state->shortAddress < OT_RADIO_INVALID_SHORT_ADDR && src.mAddress.mShortAddress == state->shortAddress)
        {
            matches = true;
        }
        break;

    case OT_MAC_ADDRESS_TYPE_EXTENDED:
        if (memcmp(&src.mAddress.mExtAddress, &state->extAddress, OT_EXT_ADDRESS_SIZE) == 0)
        {
            matches = true;
        }
        break;

    case OT_MAC_ADDRESS_TYPE_NONE:
    default:
        matches = false;
        break;
    }

exit:
    return matches;
}

#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE

void sli_ot_radio_csl_set_period(otInstance *aInstance, uint32_t aPeriod)
{
    csl_state_t *state = getCslState(aInstance);
    state->period      = aPeriod;
}

uint32_t sli_ot_radio_csl_get_period(otInstance *aInstance)
{
    csl_state_t *state = getCslState(aInstance);
    return state->period;
}

void sli_ot_radio_csl_set_sample_time(otInstance *aInstance, uint32_t aSampleTime)
{
    csl_state_t *state = getCslState(aInstance);
    state->sampleTime  = aSampleTime;
}

uint16_t sli_ot_radio_csl_get_phase(otInstance *aInstance, uint32_t aShrTxTime)
{
    csl_state_t *state         = getCslState(aInstance);
    uint32_t     cslPeriodInUs = state->period * OT_US_PER_TEN_SYMBOLS;
    uint32_t     diff;

    if (aShrTxTime == 0U)
    {
        aShrTxTime = otPlatAlarmMicroGetNow();
    }

    diff = ((state->sampleTime % cslPeriodInUs) - (aShrTxTime % cslPeriodInUs) + cslPeriodInUs) % cslPeriodInUs;

    return (uint16_t)(diff / OT_US_PER_TEN_SYMBOLS);
}

uint8_t sli_ot_radio_csl_generate_ack_ie_data(otInstance *aInstance, otRadioFrame *aReceivedFrame, uint8_t *aIeData)
{
    csl_state_t *state  = getCslState(aInstance);
    uint8_t      offset = 0;

    // Set present flag based on period and whether source address matches CSL peer
    state->present = (state->period > 0) && sli_ot_radio_csl_src_addr_matches_peer(aInstance, aReceivedFrame);

    if (state->present)
    {
        offset += otMacFrameGenerateCslIeTemplate(aIeData);
    }

    return offset;
}

void sli_ot_radio_csl_update_enh_ack_ie(otInstance   *aInstance,
                                        otRadioFrame *aEnhAckFrame,
                                        uint32_t      aRxTimestamp,
                                        uint16_t      aPacketBytes,
                                        uint8_t       aReceivedFrameLength)
{
    csl_state_t *state = getCslState(aInstance);

    if (state->present)
    {
        // Calculate time in the future where the SHR is done being sent out
        uint32_t ackShrDoneTime = // Currently partially received packet's SHR time
            (aRxTimestamp
             - (aPacketBytes * OT_RADIO_SYMBOL_TIME * 2)
             // PHR of this packet
             + (PHY_HEADER_SIZE * OT_RADIO_SYMBOL_TIME * 2)
             // Received frame's expected time in the PHR
             + (aReceivedFrameLength * OT_RADIO_SYMBOL_TIME * 2)
             // rxToTx turnaround time
             + sli_ot_radio_interface_rail_get_rx_to_tx_timing()
             // PHR time of the ACK
             + (PHY_HEADER_SIZE * OT_RADIO_SYMBOL_TIME * 2)
             // SHR time of the ACK
             + (SHR_SIZE * OT_RADIO_SYMBOL_TIME * 2));

        // Update IE data in the 802.15.4 header with the newest CSL period / phase
        otMacFrameSetCslIe(aEnhAckFrame,
                           (uint16_t)state->period,
                           sli_ot_radio_csl_get_phase(aInstance, ackShrDoneTime));
    }
}

//------------------------------------------------------------------------------
// OpenThread Platform Radio API Functions

otError otPlatRadioEnableCsl(otInstance         *aInstance,
                             uint32_t            aCslPeriod,
                             otShortAddress      aShortAddr,
                             const otExtAddress *aExtAddr)
{
    otError error = OT_ERROR_NONE;

    otEXPECT_ACTION(sl_ot_rtos_task_can_access_pal(), error = OT_ERROR_REJECTED);

    OT_ASSERT(aCslPeriod < UINT16_MAX);
    otEXPECT_ACTION((aShortAddr != OT_RADIO_BROADCAST_SHORT_ADDR) && (aShortAddr != OT_RADIO_INVALID_SHORT_ADDR),
                    error = OT_ERROR_FAILED);
    otEXPECT_ACTION(aExtAddr != nullptr, error = OT_ERROR_FAILED);

    sli_ot_radio_csl_set_period(aInstance, aCslPeriod);
    sli_ot_radio_csl_set_peer_address(aInstance, aShortAddr, aExtAddr);

exit:
    return error;
}

void otPlatRadioUpdateCslSampleTime(otInstance *aInstance, uint32_t aCslSampleTime)
{
    otEXPECT(sl_ot_rtos_task_can_access_pal());
    sli_ot_radio_csl_set_sample_time(aInstance, aCslSampleTime);
exit:
    return;
}

otError otPlatRadioResetCsl(otInstance *aInstance)
{
    sli_ot_radio_csl_reset_state(aInstance);
    return OT_ERROR_NONE;
}

#endif // OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE

#endif // OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
