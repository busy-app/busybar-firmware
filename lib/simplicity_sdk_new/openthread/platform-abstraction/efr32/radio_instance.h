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
 *   This file defines the instance management interface for the EFR32 radio platform.
 */

#ifndef RADIO_INSTANCE_H
#define RADIO_INSTANCE_H

#include <openthread-core-config.h>

#include "sl_rail.h"
#include "sl_rail_ieee802154.h"
#include <openthread/instance.h>
#include <openthread/platform/radio.h>

#ifdef __cplusplus
extern "C" {
#endif

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
#define RADIO_INTERFACE_COUNT OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_NUM
#else
#define RADIO_INTERFACE_COUNT 1
#endif

typedef uint8_t instanceIndex_t;
typedef uint8_t panIndex_t;

// Instance accessors
instanceIndex_t sli_ot_radio_instance_get_index(otInstance *aInstance);
panIndex_t      sli_ot_radio_instance_get_pan_index(otInstance *aInstance);
otInstance     *sli_ot_radio_instance_get(uint8_t aIndex);
otInstance     *sli_ot_radio_instance_from_filter_mask(uint8_t aFilterMask);
bool            sli_ot_radio_instance_is_filter_mask_broadcast_pan(uint8_t aFilterMask);

#ifndef RADIO_REQUEST_BUFFER_COUNT
#define RADIO_REQUEST_BUFFER_COUNT RADIO_INTERFACE_COUNT
#endif

#define RADIO_EXT_ADDR_COUNT RADIO_REQUEST_BUFFER_COUNT

// Common instance constants
#define INVALID_INTERFACE_INDEX 0xFF

// Filter mask macros for multi-instance support
#define RADIO_BCAST_IID (0)
#define RADIO_GET_FILTER_MASK(index) (1 << (index))
#define RADIO_PANID_FILTER_SHIFT (0)
#define RADIO_ADDR_FILTER_SHIFT (4)

#define RADIO_BCAST_PANID_FILTER_MASK RADIO_GET_FILTER_MASK(0)
#define RADIO_INDEX0_PANID_FILTER_MASK RADIO_GET_FILTER_MASK(1)
#define RADIO_INDEX1_PANID_FILTER_MASK RADIO_GET_FILTER_MASK(2)
#define RADIO_INDEX2_PANID_FILTER_MASK RADIO_GET_FILTER_MASK(3)

#define RADIO_GET_PANID_FILTER_MASK(filter)                                                             \
    (filter                                                                                             \
     & (RADIO_BCAST_PANID_FILTER_MASK | RADIO_INDEX0_PANID_FILTER_MASK | RADIO_INDEX1_PANID_FILTER_MASK \
        | RADIO_INDEX2_PANID_FILTER_MASK))

#define RADIO_BCAST_ADDR_FILTER_MASK (RADIO_GET_FILTER_MASK(0) << RADIO_ADDR_FILTER_SHIFT)
#define RADIO_INDEX0_ADDR_FILTER_MASK (RADIO_GET_FILTER_MASK(1) << RADIO_ADDR_FILTER_SHIFT)
#define RADIO_INDEX1_ADDR_FILTER_MASK (RADIO_GET_FILTER_MASK(2) << RADIO_ADDR_FILTER_SHIFT)
#define RADIO_INDEX2_ADDR_FILTER_MASK (RADIO_GET_FILTER_MASK(3) << RADIO_ADDR_FILTER_SHIFT)

#define RADIO_GET_ADDR_FILTER_MASK(filter)                                                           \
    (filter                                                                                          \
     & (RADIO_BCAST_ADDR_FILTER_MASK | RADIO_INDEX0_ADDR_FILTER_MASK | RADIO_INDEX1_ADDR_FILTER_MASK \
        | RADIO_INDEX2_ADDR_FILTER_MASK))

#define RADIO_BCAST_PANID (0xFFFF)

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE

// Command queue management
void sli_ot_radio_instance_init_command_queue(void);
void sli_ot_radio_instance_process_commands(void);
void sli_ot_radio_instance_queue_transmit(otInstance *aInstance, otRadioFrame *aFrame);
void sli_ot_radio_instance_queue_energy_scan(otInstance *aInstance, uint8_t aScanChannel, uint16_t aScanDuration);

// State management
bool sli_ot_radio_instance_is_busy(void);

// Queue management
bool sli_ot_radio_instance_is_queue_empty(void);
void sli_ot_radio_instance_clear_queue(void);

void    sli_ot_radio_instance_set_tx_busy(bool busy);
bool    sli_ot_radio_instance_get_tx_busy(void);
uint8_t sli_ot_radio_instance_get_rail_filter_mask(void);
void    sli_ot_radio_instance_set_rail_filter_mask(uint8_t mask);
void    sli_ot_radio_instance_update_rail_filter_mask_for_pan_id(uint16_t aPanId, uint8_t aPanIndex);

#if FAST_CHANNEL_SWITCHING_SUPPORT && OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
otError sl_get_channel_switching_cfg(sl_rail_ieee802154_rx_channel_switching_cfg_t *channelSwitchingCfg);
uint8_t fastChannelSwitchingChannel(otInstance *aInstance);
#endif // FAST_CHANNEL_SWITCHING_SUPPORT && OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE

#endif // OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE

// Energy scan defer functions (available in both single and multi-instance builds)
bool sli_ot_radio_instance_energy_scan_should_defer(void);
void sli_ot_radio_instance_energy_scan_defer(otInstance *aInstance, uint8_t aScanChannel, uint16_t aScanDuration);

#ifdef __cplusplus
}
#endif

#endif // RADIO_INSTANCE_H
