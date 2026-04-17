/*
 *  Copyright (c) 2023, The OpenThread Authors.
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
 *   This file defines the frequency band configuration structure for efr32.
 *
 */

#ifndef PLATFORM_BAND_H_
#define PLATFORM_BAND_H_

#include <openthread/platform/radio.h>

#include "radio_counters.h"
#include "rail_config.h"
#include "sl_802154_radio_priority_config.h"
#include "sl_rail.h"
#include "sl_rail_ieee802154.h"

#define RAIL_TX_FIFO_SIZE (OT_RADIO_FRAME_MAX_SIZE + 1)

#define RADIO_SCHEDULER_CHANNEL_SLIP_TIME 500000UL

#define RADIO_TIMING_CSMA_OVERHEAD_US 500
#define RADIO_TIMING_DEFAULT_BYTETIME_US 32   // only used if sl_rail_get_bit_rate returns 0
#define RADIO_TIMING_DEFAULT_SYMBOLTIME_US 16 // only used if sl_rail_get_symbol_rate returns 0
typedef struct efr32CommonConfig
{
    sl_rail_config_t mRailConfig;
    union
    {
        // Used to align this buffer as needed
        SL_RAIL_DECLARE_FIFO_BUFFER(align, RAIL_TX_FIFO_SIZE);
        uint8_t fifo[RAIL_TX_FIFO_SIZE]; // must be 2 power between 64 and 4096, and bigger than OT_RADIO_FRAME_MAX_SIZE
    } mRailTxFifo;
} efr32CommonConfig;

typedef struct efr32BandConfig
{
    const sl_rail_channel_config_t *mChannelConfig;
    uint8_t                         mChannelMin;
    uint8_t                         mChannelMax;
} efr32BandConfig;

#endif // PLATFORM_BAND_H_
