/*
 *  Copyright (c) 2024, The OpenThread Authors.
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
 *   This file implements the OpenThread platform abstraction for radio coex metrics
 *   collection.
 *
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "platform-efr32.h"
#include "radio_coex.h"
#include "radio_interface.h"
#include "common/logging.hpp"

#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT
#include "coexistence-802154.h"
#include "coexistence-ot.h"
#endif

#if OPENTHREAD_CONFIG_PLATFORM_RADIO_COEX_ENABLE

static sl_ot_coex_counter_t sCoexCounter;

#if SL_OPENTHREAD_COEX_COUNTER_ENABLE
// RAIL-level coexistence event counters
uint32_t efr32RadioCoexCounters[SL_RAIL_UTIL_COEX_EVENT_COUNT] = {0};

// For multiplexer builds, this is called through the function pointer
void sli_ot_radio_coex_counter_on_event(sl_rail_util_coex_event_t event)
{
    otEXPECT(event < SL_RAIL_UTIL_COEX_EVENT_COUNT);
    efr32RadioCoexCounters[event] += 1;
exit:
    return;
}

// For non-multiplexer builds, RAIL calls this directly
#ifndef SL_CATALOG_RAIL_MULTIPLEXER_PRESENT
void sl_rail_util_coex_counter_on_event(sl_rail_util_coex_event_t event)
{
    sli_ot_radio_coex_counter_on_event(event);
}
#endif

void sli_ot_radio_coex_clear_counters(void)
{
    memset((void *)efr32RadioCoexCounters, 0, sizeof(efr32RadioCoexCounters));
}
#endif // SL_OPENTHREAD_COEX_COUNTER_ENABLE

static inline void sl_increment_if_no_overflow(uint32_t *aVar, uint32_t aIncr, bool *aStopped)
{
    uint32_t old  = *aVar;
    uint32_t temp = old + aIncr;
    if (temp < old)
    {
        if (aStopped)
            *aStopped = true;
    }
    else
    {
        *aVar = temp;
    }
}

void sl_rail_util_coex_ot_events(sl_rail_util_coex_ot_event_t aEvent)
{
    bool                         isTxEvent = (aEvent & SL_RAIL_UTIL_COEX_OT_TX_REQUEST);
    sl_rail_util_coex_ot_event_t coexEvent =
        (aEvent & ~(SL_RAIL_UTIL_COEX_OT_TX_REQUEST | SL_RAIL_UTIL_COEX_OT_RX_REQUEST));
    uint32_t *metrics = isTxEvent ? &sCoexCounter.metrics.mNumTxRequest : &sCoexCounter.metrics.mNumRxRequest;
    uint64_t *totalReqToGrantDuration =
        isTxEvent ? &sCoexCounter.totalTxReqToGrantDuration : &sCoexCounter.totalRxReqToGrantDuration;

    /* clang-format off */

    // uint32_t mNumGrantGlitch;                       ///< Not available.

    // mNumTxRequest    = mNumTxGrantImmediate + mNumTxGrantWait
    // mNumTxGrantWait  = mNumTxGrantWaitActivated + mNumTxGrantWaitTimeout
    // Same applies for Rx counters.
    
    /* Tx Events*/
    // uint32_t mNumTxRequest;                         ///< Number of Tx Requested = mNumTxGrantImmediate + mNumTxGrantWait.
    // uint32_t mNumTxGrantImmediate;                  ///< Not Available.
    // uint32_t mNumTxGrantWait;                       ///< Number of tx requests while grant was inactive.
    // uint32_t mNumTxGrantWaitActivated;              ///< Number of tx requests while grant was inactive that were ultimately granted.
    // uint32_t mNumTxGrantWaitTimeout;                ///< Number of tx requests while grant was inactive that timed out.
    // uint32_t mNumTxGrantDeactivatedDuringRequest;   ///< Number of tx that were in progress when grant was deactivated.
    // uint32_t mNumTxDelayedGrant;                    ///< Number of tx requests that were not granted within 50us.
    // uint32_t mAvgTxRequestToGrantTime;              ///< Average time in usec from tx request to grant.
    
    /* Rx Events*/
    // uint32_t mNumRxRequest;                         ///< Number of rx requests.
    // uint32_t mNumRxGrantImmediate;                  ///< Number of rx requests while grant was active.
    // uint32_t mNumRxGrantWait;                       ///< Number of rx requests while grant was inactive.
    // uint32_t mNumRxGrantWaitActivated;              ///< Number of rx requests while grant was inactive that were ultimately granted.
    // uint32_t mNumRxGrantWaitTimeout;                ///< Number of rx requests while grant was inactive that timed out.
    // uint32_t mNumRxGrantDeactivatedDuringRequest;   ///< Number of rx that were in progress when grant was deactivated.
    // uint32_t mNumRxDelayedGrant;                    ///< Number of rx requests that were not granted within 50us.
    // uint32_t mAvgRxRequestToGrantTime;              ///< Average time in usec from rx request to grant.
    
    // uint32_t mNumRxGrantNone;                       ///< Number of rx requests that completed without receiving grant.
    // bool     mStopped;

    /* clang-format on */

    otEXPECT(sCoexCounter.metrics.mStopped == false);

    switch (coexEvent)
    {
    case SL_RAIL_UTIL_COEX_OT_EVENT_GRANTED_IMMEDIATE:
        sl_increment_if_no_overflow(metrics, 1, &sCoexCounter.metrics.mStopped);
        break;

    case SL_RAIL_UTIL_COEX_OT_EVENT_REQUESTED:
        sCoexCounter.timestamp = otPlatAlarmMicroGetNow();
        sl_increment_if_no_overflow(metrics, 1, &sCoexCounter.metrics.mStopped);
        break;

    case SL_RAIL_UTIL_COEX_OT_EVENT_GRANTED:
    {
        uint64_t reqToGrantDuration = otPlatAlarmMicroGetNow() - sCoexCounter.timestamp;
        sl_increment_if_no_overflow(metrics, 1, &sCoexCounter.metrics.mStopped);

        if (reqToGrantDuration > 50)
        {
            sl_increment_if_no_overflow(metrics, 1, &sCoexCounter.metrics.mStopped);
        }

        *totalReqToGrantDuration += reqToGrantDuration;
    }
    break;

    case SL_RAIL_UTIL_COEX_OT_EVENT_DENIED:
        sl_increment_if_no_overflow(metrics, 1, &sCoexCounter.metrics.mStopped);
        break;

    case SL_RAIL_UTIL_COEX_OT_EVENT_GRANT_ABORTED:
        sl_increment_if_no_overflow(metrics, 1, &sCoexCounter.metrics.mStopped);
        break;

    default:
        break;
    }

    sl_increment_if_no_overflow(metrics,
                                sCoexCounter.metrics.mNumTxGrantWaitTimeout
                                    + sCoexCounter.metrics.mNumTxGrantWaitActivated,
                                &sCoexCounter.metrics.mStopped);
    if (*metrics != 0)
    {
        sCoexCounter.metrics.mAvgTxRequestToGrantTime = (uint32_t)(*totalReqToGrantDuration / *metrics);
    }
    else
    {
        sCoexCounter.metrics.mAvgTxRequestToGrantTime = 0;
    }

exit:
    return;
}

otError otPlatRadioGetCoexMetrics(otInstance *aInstance, otRadioCoexMetrics *aCoexMetrics)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError error = OT_ERROR_NONE;

    otEXPECT_ACTION(aCoexMetrics != NULL, error = OT_ERROR_INVALID_ARGS);

    memcpy(aCoexMetrics, &sCoexCounter.metrics, sizeof(otRadioCoexMetrics));

exit:
    return error;
}

void sli_radio_coex_reset(void)
{
    memset(&sCoexCounter, 0, sizeof(sCoexCounter));
}

otError otPlatRadioSetCoexEnabled(otInstance *aInstance, bool aEnabled)
{
    otError     error;
    sl_status_t status;

    OT_UNUSED_VARIABLE(aInstance);
    otEXPECT_ACTION(sl_ot_rtos_task_can_access_pal(), error = OT_ERROR_REJECTED);

    status = sl_rail_util_coex_set_enable(aEnabled);
    if (aEnabled && !sl_rail_util_coex_is_enabled())
    {
        otLogInfoPlat("Coexistence GPIO configurations not set");
        return OT_ERROR_FAILED;
    }
    sli_ot_radio_interface_set_coex_enabled(aEnabled);

    error = (status != SL_STATUS_OK) ? OT_ERROR_FAILED : OT_ERROR_NONE;

exit:
    return error;
}

#else

otError otPlatRadioGetCoexMetrics(otInstance *aInstance, otRadioCoexMetrics *aCoexMetrics)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aCoexMetrics);
    return OT_ERROR_NOT_IMPLEMENTED;
}

otError otPlatRadioSetCoexEnabled(otInstance *aInstance, bool aEnabled)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aEnabled);
    return OT_ERROR_NOT_IMPLEMENTED;
}
#endif // OPENTHREAD_CONFIG_PLATFORM_RADIO_COEX_ENABLE
