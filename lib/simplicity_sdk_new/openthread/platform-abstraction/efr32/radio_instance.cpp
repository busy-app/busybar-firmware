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
 *   This file implements instance management for single and multi-instance OpenThread configurations,
 *   including instance accessors, transmit queueing, and energy scan deferral.
 */

#include "radio_instance.h"
#include <openthread-core-config.h>

#include <assert.h>
#include "common/code_utils.hpp"
#include "common/debug.hpp"
#include "utils/code_utils.h"

#include <openthread/platform/radio.h>
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
#include <openthread/platform/multipan.h>
#endif

extern "C" {
#include "circular_queue.h"
#include "sl_core.h"
#include "sl_rail.h"
#include "sl_rail_ieee802154.h"
}

#include "radio_channel_switching.h"
#include "radio_energy_scan.hpp"
#include "radio_interface.h"
#include "radio_state.h"
#include "sl_memory_manager.h"

//------------------------------------------------------------------------------
// Type Definitions

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE

typedef struct energyScanParams
{
    uint8_t  scanChannel;  ///< Energy scan channel
    uint16_t scanDuration; ///< Energy scan duration
} energyScanParams;

enum class PendingCommandType : uint8_t
{
    Transmit   = 0,
    EnergyScan = 1
};

struct PendingCommandEntry
{
    otInstance        *instance;
    PendingCommandType cmdType;
    union
    {
        otRadioFrame    *txFrame;
        energyScanParams energyScan;
    } request;
};

#endif // OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE

//------------------------------------------------------------------------------
// Static Variables

// Global state for filter mask (used in multi-instance mode)
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
static uint8_t sRailFilterMask = RADIO_BCAST_PANID_FILTER_MASK;

static Queue_t sPendingCommandQueue;

static volatile bool tx_busy = false;

#endif // OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE

//------------------------------------------------------------------------------
// Instance Accessor Functions

#if OPENTHREAD_CONFIG_MULTIPLE_STATIC_INSTANCE_ENABLE
instanceIndex_t sli_ot_radio_instance_get_index(otInstance *aInstance)
{
    return otInstanceGetIndex(aInstance);
}
#else  // !OPENTHREAD_CONFIG_MULTIPLE_STATIC_INSTANCE_ENABLE
instanceIndex_t sli_ot_radio_instance_get_index(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return 0;
}
#endif // OPENTHREAD_CONFIG_MULTIPLE_STATIC_INSTANCE_ENABLE

panIndex_t sli_ot_radio_instance_get_pan_index(otInstance *aInstance)
{
    return static_cast<panIndex_t>(sli_ot_radio_instance_get_index(aInstance));
}

otInstance *sli_ot_radio_instance_get(uint8_t aIndex)
{
#if OPENTHREAD_CONFIG_MULTIPLE_STATIC_INSTANCE_ENABLE
    return otInstanceGetInstance(aIndex);
#else
    OT_UNUSED_VARIABLE(aIndex);
    return otInstanceGetSingle();
#endif
}

bool sli_ot_radio_instance_is_filter_mask_broadcast_pan(uint8_t aFilterMask)
{
    // Check if broadcast PAN ID bit (bit 0) is set
    // This indicates the packet should be delivered to all instances
    return ((aFilterMask & RADIO_BCAST_PANID_FILTER_MASK) != 0);
}

otInstance *sli_ot_radio_instance_from_filter_mask(uint8_t aFilterMask)
{
    otInstance *instance      = nullptr;
    uint8_t     instanceIndex = 0;
    bool        foundInstance = false;
    uint8_t     panFilterMask;
    uint8_t     addressFiltermask;

#if !OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    // Single-instance mode: Always return the single instance for security processing
    // This matches the old radio.c behavior where getIidFromFilterMask() always returned
    // a valid IID (0) in single-instance mode
    return sli_ot_radio_instance_get(0);
#endif

    // Check if this is a broadcast packet
    otEXPECT(!sli_ot_radio_instance_is_filter_mask_broadcast_pan(aFilterMask));

    // Extract the PAN ID (bits 0-3)and Address filter bits (bits 4-7) from the filter mask.
    // The filter mask structure:
    // | Bit:7 | Bit:6 | Bit:5 | Bit:4 | Bit:3 | Bit:2 | Bit:1 | Bit:0 |
    // | Addr2 | Addr1 | Addr0 | Bcast | Pan2  | Pan1  | Pan0  | Bcast |
    panFilterMask = (uint8_t)(RADIO_GET_PANID_FILTER_MASK(aFilterMask) >> RADIO_PANID_FILTER_SHIFT);
    // For packets that have the address bit set but do not include a PAN ID.
    // Typically, these packets have the PAN ID compression bit set.
    addressFiltermask = (uint8_t)(RADIO_GET_ADDR_FILTER_MASK(aFilterMask) >> RADIO_ADDR_FILTER_SHIFT);

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    // Mask all the unused indices.
    panFilterMask &= sRailFilterMask;
    addressFiltermask &= sRailFilterMask;
#endif

    // Find the first set bit in the PAN or address filter mask,
    // Skip bit 0 (broadcast) and look for instance-specific matches
    for (uint8_t i = 1; i <= RADIO_INTERFACE_COUNT && i < 8; i++)
    {
        if ((panFilterMask & (1 << i)) || (addressFiltermask & (1 << i)))
        {
            instanceIndex = i - 1; // Convert to 0-based index
            foundInstance = true;
            break;
        }
    }

#if OPENTHREAD_CONFIG_MULTIPLE_STATIC_INSTANCE_ENABLE
    // Multi-instance: If no valid instance found or index is out of bounds, exit with nullptr
    otEXPECT(foundInstance && instanceIndex < RADIO_INTERFACE_COUNT);
#else
    // Single-instance: Always return the single instance (backward compatibility)
    OT_UNUSED_VARIABLE(foundInstance);
#endif

    instance = sli_ot_radio_instance_get(instanceIndex);

exit:
    return instance;
}

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
//------------------------------------------------------------------------------
// Overflow callback for pending command queue
static bool pendingCommandQueueOverflowCallback(const Queue_t *queue, void *data)
{
    OT_UNUSED_VARIABLE(queue);
    OT_UNUSED_VARIABLE(data);

    // We should never hit this callback because a host can only request
    // one command at a time. Hence, added an assert if it happens.
    OT_ASSERT(false);

    return false;
}

//------------------------------------------------------------------------------
// Push a command to pending queue
static void pushPendingCommand(PendingCommandType aCmdType, otInstance *aInstance, void *aCmdParams)
{
    PendingCommandEntry *pendingCommand = (PendingCommandEntry *)sl_malloc(sizeof(PendingCommandEntry));
    OT_ASSERT(pendingCommand != NULL);

    pendingCommand->instance = aInstance;
    pendingCommand->cmdType  = aCmdType;

    if (aCmdType == PendingCommandType::Transmit)
    {
        otRadioFrame *txFrame           = (otRadioFrame *)aCmdParams;
        pendingCommand->request.txFrame = txFrame;
    }
    else if (aCmdType == PendingCommandType::EnergyScan)
    {
        const energyScanParams *energyScanReq           = (energyScanParams *)aCmdParams;
        pendingCommand->request.energyScan.scanChannel  = energyScanReq->scanChannel;
        pendingCommand->request.energyScan.scanDuration = energyScanReq->scanDuration;
    }

    if (!queueAdd(&sPendingCommandQueue, (void *)pendingCommand))
    {
        sl_free(pendingCommand);
    }
}

//------------------------------------------------------------------------------
// Init pending command queue
void sli_ot_radio_instance_init_command_queue(void)
{
    bool queueStatus;

    // Initialize the queue for pending commands.
    queueStatus = queueInit(&sPendingCommandQueue, RADIO_REQUEST_BUFFER_COUNT);
    OT_ASSERT(queueStatus);

    // Specify a callback to be called upon queue overflow.
    queueStatus = queueOverflow(&sPendingCommandQueue, &pendingCommandQueueOverflowCallback);
    OT_ASSERT(queueStatus);
}

//------------------------------------------------------------------------------
// Process pending commands
void sli_ot_radio_instance_process_commands(void)
{
    // Check and process pending transmit and energy scan commands if radio is not busy.
    if (!queueIsEmpty(&sPendingCommandQueue) && !sli_ot_radio_instance_is_busy())
    {
        // Dequeue the pending command
        PendingCommandEntry *pendingCommand = (PendingCommandEntry *)queueRemove(&sPendingCommandQueue);
        OT_ASSERT(pendingCommand != NULL);

        switch (pendingCommand->cmdType)
        {
        case PendingCommandType::Transmit:
            otPlatRadioTransmit(pendingCommand->instance, pendingCommand->request.txFrame);
            break;

        case PendingCommandType::EnergyScan:
            sli_ot_energy_scan_async(pendingCommand->instance,
                                     pendingCommand->request.energyScan.scanChannel,
                                     pendingCommand->request.energyScan.scanDuration);
            break;

        default:
            OT_ASSERT(false);
            break;
        }

        // Free the allocated memory.
        sl_free(pendingCommand);
    }
}

// State management functions
bool sli_ot_radio_instance_is_busy(void)
{
    return sli_ot_radio_state_is_transmitting_or_scanning();
}

// Queue management functions
bool sli_ot_radio_instance_is_queue_empty(void)
{
    return queueIsEmpty(&sPendingCommandQueue);
}

void sli_ot_radio_instance_clear_queue(void)
{
    while (!queueIsEmpty(&sPendingCommandQueue))
    {
        PendingCommandEntry *entry = (PendingCommandEntry *)queueRemove(&sPendingCommandQueue);
        sl_free(entry);
    }
}

// Multi-instance state management functions
void sli_ot_radio_instance_set_tx_busy(bool busy)
{
    tx_busy = busy;
}

bool sli_ot_radio_instance_get_tx_busy(void)
{
    return tx_busy;
}

uint8_t sli_ot_radio_instance_get_rail_filter_mask(void)
{
    return sRailFilterMask;
}

void sli_ot_radio_instance_set_rail_filter_mask(uint8_t mask)
{
    sRailFilterMask = mask;
}

extern "C" {
void sli_ot_radio_instance_update_rail_filter_mask_for_pan_id(uint16_t aPanId, uint8_t aPanIndex)
{
    // We already have bit 0 enabled in filtermask to track BCAST Packets, so
    // track only unique PanIds.
    // Filter mask mapping: bit 0 = broadcast, bit 1 = instance 0, bit 2 = instance 1, bit 3 = instance 2
    if (aPanId != RADIO_BCAST_PANID)
    {
        sRailFilterMask |= RADIO_GET_FILTER_MASK(aPanIndex + 1);
    }
}
} // extern "C"

#if FAST_CHANNEL_SWITCHING_SUPPORT

otError sl_get_channel_switching_cfg(sl_rail_ieee802154_rx_channel_switching_cfg_t *channelSwitchingCfg)
{
    return sli_ot_radio_channel_switching_get_config(channelSwitchingCfg);
}

#endif // FAST_CHANNEL_SWITCHING_SUPPORT
#endif // OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE

void sli_ot_radio_instance_queue_transmit(otInstance *aInstance, otRadioFrame *aFrame)
{
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    pushPendingCommand(PendingCommandType::Transmit, aInstance, aFrame);
#else
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aFrame);
#endif
}

void sli_ot_radio_instance_queue_energy_scan(otInstance *aInstance, uint8_t aScanChannel, uint16_t aScanDuration)
{
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    energyScanParams params = {aScanChannel, aScanDuration};
    pushPendingCommand(PendingCommandType::EnergyScan, aInstance, &params);
#else
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aScanChannel);
    OT_UNUSED_VARIABLE(aScanDuration);
#endif
}

bool sli_ot_radio_instance_energy_scan_should_defer(void)
{
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    return sli_ot_radio_instance_is_busy();
#else
    return false; // Single instance: never defer
#endif
}

void sli_ot_radio_instance_energy_scan_defer(otInstance *aInstance, uint8_t aScanChannel, uint16_t aScanDuration)
{
    sli_ot_radio_instance_queue_energy_scan(aInstance, aScanChannel, aScanDuration);
}

//------------------------------------------------------------------------------
// OpenThread Multipan API Functions

otError otPlatMultipanGetActiveInstance(otInstance **aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return OT_ERROR_INVALID_COMMAND;
}

otError otPlatMultipanSetActiveInstance(otInstance *aInstance, bool aCompletePending)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aCompletePending);

    return OT_ERROR_INVALID_COMMAND;
}
