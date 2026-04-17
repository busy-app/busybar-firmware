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
 *   This file implements the OpenThread platform abstraction for the alarm.
 *
 */

#include <assert.h>
#include <openthread-core-config.h>
#include <openthread-system.h>
#include <stdbool.h>
#include <stdint.h>
#include <openthread/platform/alarm-micro.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/diag.h>
#include "common/debug.hpp"
#include "common/logging.hpp"

#include "alarm.h"
#include "platform-efr32.h"
#include "utils/code_utils.h"

#include "radio_instance.h"
#include "sl_core.h"
#include "sl_rail.h"
#include "sl_sleeptimer.h"

#ifndef TESTING
#define STATIC static
#else
#define STATIC
#endif

// timer data for handling wrapping
typedef struct wrap_timer_data wrap_timer_data_t;
struct wrap_timer_data
{
    uint16_t overflow_counter;
    uint16_t overflow_max;
};

// millisecond timer (sleeptimer)
static sl_sleeptimer_timer_handle_t sl_handle[RADIO_INTERFACE_COUNT];

// microsecond timer (RAIL timer)
static sl_rail_multi_timer_t rail_timer[RADIO_INTERFACE_COUNT];

// forward declare generic alarm handle
struct AlarmHandle;

// function pointers for timer operations
typedef void (*timerStartFunc)(struct AlarmHandle *, uint32_t);
typedef uint32_t (*timerMaxFunc)(void);
typedef uint32_t (*timerGetTimeFunc)(void);
typedef void (*timerStopFunc)(struct AlarmHandle *);

// alarm handle structure
typedef struct AlarmHandle AlarmHandle;
struct AlarmHandle
{
    otInstance       *mThreadInstance;
    void             *mTimerHandle;
    timerStartFunc    mTimerStart;
    timerMaxFunc      mTimerGetMax;
    timerGetTimeFunc  mTimerGetNow;
    timerStopFunc     mTimerStop;
    wrap_timer_data_t mWrapData;
    volatile bool     mIsRunning;
    volatile int      mFiredCount;
};

// callback function for the stack
typedef void (*StackAlarmCallback)(otInstance *);

// alarm handle instances
static AlarmHandle sMsAlarmHandles[RADIO_INTERFACE_COUNT];
static AlarmHandle sUsAlarmHandles[RADIO_INTERFACE_COUNT];

static uint64_t sPendingTimeMs[RADIO_INTERFACE_COUNT];

// millisecond-alarm callback
STATIC void msAlarmCallback(sl_sleeptimer_timer_handle_t *aHandle, void *aData);

// microsecond-alarm callback
STATIC void usAlarmCallback(struct sl_rail_multi_timer *tmr, sl_rail_time_t expectedTimeOfEvent, void *cbArg);

// timer specific operations
static void     msTimerStart(AlarmHandle *aMsAlarm, uint32_t aAlarmDuration);
static uint32_t msTimerGetMaxTime(void);
static uint32_t msTimerGetNow(void);
static void     msTimerStop(AlarmHandle *aMsAlarm);

static void     usTimerStart(AlarmHandle *aUsAlarm, uint32_t aAlarmDuration);
static uint32_t usTimerGetMaxTime(void);
static uint32_t usTimerGetNow(void);
static void     usTimerStop(AlarmHandle *aUsAlarm);

// overflow utility functions
static inline bool isAlarmOverflowInProgress(const AlarmHandle *aAlarm);

// common timer operations
static void FireAlarm(AlarmHandle *aAlarm);
static void StartAlarmAt(AlarmHandle *aAlarm, uint32_t aT0, uint32_t aDt);
static void StopActiveAlarm(AlarmHandle *aAlarm);
static void AlarmCallback(AlarmHandle *aAlarm);

// pending time utility functions
static inline uint64_t GetPendingTime(otInstance *aInstance);
static inline void     SetPendingTime(otInstance *aInstance, uint64_t aPendingTime);

// alarm handle utility functions
static inline AlarmHandle *GetAlarmHandle(AlarmHandle *aHandleList, otInstance *aInstance);
static AlarmHandle        *GetFirstFiredAlarm(AlarmHandle *aAlarm);
static AlarmHandle        *GetNextFiredAlarm(AlarmHandle *aAlarm, const AlarmHandle *aAlarmEnd);
static inline bool         HasAnyAlarmFired(void);
static inline uint32_t     SetAlarmWrappedDuration(AlarmHandle *aAlarm, uint64_t aRemainingTime);

static void msTimerStart(AlarmHandle *aMsAlarm, uint32_t aAlarmDuration)
{
    OT_ASSERT(aMsAlarm != NULL);
    OT_ASSERT(aMsAlarm->mIsRunning == false);
    sl_status_t status = sl_sleeptimer_start_timer_ms(aMsAlarm->mTimerHandle,
                                                      aAlarmDuration,
                                                      &msAlarmCallback,
                                                      (void *)aMsAlarm,
                                                      0,
                                                      SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
#if OPENTHREAD_CONFIG_ASSERT_ENABLE
    OT_ASSERT(status == SL_STATUS_OK);
#else
    OT_UNUSED_VARIABLE(status);
#endif
}

static uint32_t msTimerGetMaxTime(void)
{
    return sl_sleeptimer_get_max_ms32_conversion();
}

static uint32_t msTimerGetNow(void)
{
    uint64_t    ticks;
    uint64_t    now;
    sl_status_t status;

    ticks  = sl_sleeptimer_get_tick_count64();
    status = sl_sleeptimer_tick64_to_ms(ticks, &now);

#if OPENTHREAD_CONFIG_ASSERT_ENABLE
    OT_ASSERT(status == SL_STATUS_OK);
#else
    OT_UNUSED_VARIABLE(status);
#endif

    return (uint32_t)now;
}

static void msTimerStop(AlarmHandle *aMsAlarm)
{
    OT_ASSERT(aMsAlarm != NULL);
    sl_sleeptimer_stop_timer((sl_sleeptimer_timer_handle_t *)aMsAlarm->mTimerHandle);
}

static void usTimerStart(AlarmHandle *aUsAlarm, uint32_t aAlarmDuration)
{
    OT_ASSERT(aUsAlarm != NULL);
    OT_ASSERT(aUsAlarm->mIsRunning == false);
    sl_rail_status_t status = sl_rail_set_multi_timer(SL_RAIL_EFR32_HANDLE,
                                                      aUsAlarm->mTimerHandle,
                                                      aAlarmDuration,
                                                      SL_RAIL_TIME_DELAY,
                                                      &usAlarmCallback,
                                                      (void *)aUsAlarm);
#if OPENTHREAD_CONFIG_ASSERT_ENABLE
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
#else
    OT_UNUSED_VARIABLE(status);
#endif
}

static uint32_t usTimerGetMaxTime(void)
{
    return UINT32_MAX;
}

static uint32_t usTimerGetNow(void)
{
    return sl_rail_get_time(SL_RAIL_EFR32_HANDLE);
}

static void usTimerStop(AlarmHandle *aUsAlarm)
{
    OT_ASSERT(aUsAlarm != NULL);
    sl_rail_cancel_multi_timer(SL_RAIL_EFR32_HANDLE, (struct sl_rail_multi_timer *)aUsAlarm->mTimerHandle);
}

static inline bool isAlarmOverflowInProgress(const AlarmHandle *aAlarm)
{
    OT_ASSERT(aAlarm != NULL);
    return aAlarm->mWrapData.overflow_counter < aAlarm->mWrapData.overflow_max;
}

static void FireAlarm(AlarmHandle *aAlarm)
{
    OT_ASSERT(aAlarm != NULL);
    aAlarm->mFiredCount++;
    StopActiveAlarm(aAlarm);

    otSysEventSignalPending();
}

static void ProcessAlarm(AlarmHandle *aAlarm, StackAlarmCallback aCallback)
{
    OT_ASSERT(aAlarm != NULL);
    otInstance *instance = aAlarm->mThreadInstance;

    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_ATOMIC();

    int numCallbacks    = aAlarm->mFiredCount;
    aAlarm->mFiredCount = 0;

    CORE_EXIT_ATOMIC();

    while (numCallbacks > 0)
    {
        numCallbacks--;
        aCallback(instance);
    }
}

static inline uint32_t SetAlarmWrappedDuration(AlarmHandle *aAlarm, uint64_t aRemainingTime)
{
    OT_ASSERT(aAlarm != NULL);
    uint64_t          initial_wrap_time = aRemainingTime;
    wrap_timer_data_t wrapData          = {0};

    if (initial_wrap_time > aAlarm->mTimerGetMax())
    {
        initial_wrap_time %= aAlarm->mTimerGetMax();
        wrapData.overflow_max     = (uint16_t)(aRemainingTime / aAlarm->mTimerGetMax());
        wrapData.overflow_counter = 0;
    }
    aAlarm->mWrapData = wrapData;
    return (uint32_t)initial_wrap_time;
}

static void StartAlarmAt(AlarmHandle *aAlarm, uint32_t aT0, uint32_t aDt)
{
    OT_ASSERT(aAlarm != NULL);
    otEXPECT(sl_ot_rtos_task_can_access_pal());
    StopActiveAlarm(aAlarm);

    uint64_t requested_time = (uint64_t)aT0 + (uint64_t)aDt;
    int64_t  remaining      = (int64_t)requested_time - (int64_t)aAlarm->mTimerGetNow();

    if (remaining <= 0)
    {
        FireAlarm(aAlarm);
    }
    else
    {
        aAlarm->mTimerStart(aAlarm, SetAlarmWrappedDuration(aAlarm, (uint64_t)remaining));
        aAlarm->mIsRunning = true;
    }

exit:
    return;
}

static void StopActiveAlarm(AlarmHandle *aAlarm)
{
    OT_ASSERT(aAlarm != NULL);
    otEXPECT(aAlarm->mIsRunning);
    otEXPECT(sl_ot_rtos_task_can_access_pal());
    aAlarm->mTimerStop(aAlarm);
    aAlarm->mIsRunning = false;
exit:
    return;
}

static void AlarmCallback(AlarmHandle *aAlarm)
{
    OT_ASSERT(aAlarm != NULL);
    if (isAlarmOverflowInProgress(aAlarm))
    {
        aAlarm->mIsRunning = false;
        aAlarm->mWrapData.overflow_counter++;
        aAlarm->mTimerStart(aAlarm, aAlarm->mTimerGetMax());
        aAlarm->mIsRunning = true;
    }
    else
    {
        FireAlarm(aAlarm);
    }
}

static inline uint64_t GetPendingTime(otInstance *aInstance)
{
    return sPendingTimeMs[sli_ot_radio_instance_get_index(aInstance)];
}

static inline void SetPendingTime(otInstance *aInstance, uint64_t aPendingTime)
{
    sPendingTimeMs[sli_ot_radio_instance_get_index(aInstance)] = aPendingTime;
}

static inline AlarmHandle *GetAlarmHandle(AlarmHandle *aHandleList, otInstance *aInstance)
{
    instanceIndex_t index = sli_ot_radio_instance_get_index(aInstance);
    return aHandleList + index;
}

static AlarmHandle *GetFirstFiredAlarm(AlarmHandle *aHandleList)
{
    return GetNextFiredAlarm(aHandleList, aHandleList + RADIO_INTERFACE_COUNT);
}

static AlarmHandle *GetNextFiredAlarm(AlarmHandle *aAlarm, const AlarmHandle *aAlarmEnd)
{
    AlarmHandle *nextAlarm = aAlarm;
    size_t       maxCount  = (size_t)(aAlarmEnd - aAlarm);
    size_t       i         = 0;
    while (i < maxCount && aAlarm && (nextAlarm->mFiredCount == 0))
    {
        ++nextAlarm;
        ++i;
    }
    if (i == maxCount)
    {
        nextAlarm = NULL;
    }
    return nextAlarm;
}

static inline bool HasAnyAlarmFired(void)
{
    return (GetFirstFiredAlarm(sMsAlarmHandles) != NULL) || (GetFirstFiredAlarm(sUsAlarmHandles) != NULL);
}

// millisecond-alarm callback
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
STATIC void msAlarmCallback(sl_sleeptimer_timer_handle_t *aHandle, void *aData)
{
    OT_UNUSED_VARIABLE(aHandle);
    AlarmCallback((AlarmHandle *)aData);
}

// microsecond-alarm callback
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
STATIC void usAlarmCallback(struct sl_rail_multi_timer *tmr, sl_rail_time_t expectedTimeOfEvent, void *cbArg)
{
    OT_UNUSED_VARIABLE(tmr);
    OT_UNUSED_VARIABLE(expectedTimeOfEvent);
    AlarmCallback((AlarmHandle *)cbArg);
}

void efr32AlarmInit(void)
{
    memset(&sl_handle, 0, sizeof sl_handle);
    memset(&rail_timer, 0, sizeof rail_timer);

    for (uint8_t i = 0; i < RADIO_INTERFACE_COUNT; i++)
    {
        sPendingTimeMs[i] = 0;

        sMsAlarmHandles[i].mThreadInstance = NULL;
        sMsAlarmHandles[i].mTimerHandle    = &sl_handle[i];
        sMsAlarmHandles[i].mTimerStart     = msTimerStart;
        sMsAlarmHandles[i].mTimerGetMax    = msTimerGetMaxTime;
        sMsAlarmHandles[i].mTimerGetNow    = msTimerGetNow;
        sMsAlarmHandles[i].mTimerStop      = msTimerStop;
        sMsAlarmHandles[i].mIsRunning      = false;
        sMsAlarmHandles[i].mFiredCount     = 0;
        memset(&sMsAlarmHandles[i].mWrapData, 0, sizeof(wrap_timer_data_t));

        sUsAlarmHandles[i].mThreadInstance = NULL;
        sUsAlarmHandles[i].mTimerHandle    = &rail_timer[i];
        sUsAlarmHandles[i].mTimerStart     = usTimerStart;
        sUsAlarmHandles[i].mTimerGetMax    = usTimerGetMaxTime;
        sUsAlarmHandles[i].mTimerGetNow    = usTimerGetNow;
        sUsAlarmHandles[i].mTimerStop      = usTimerStop;
        sUsAlarmHandles[i].mIsRunning      = false;
        sUsAlarmHandles[i].mFiredCount     = 0;
        memset(&sUsAlarmHandles[i].mWrapData, 0, sizeof(wrap_timer_data_t));
    }
}

void efr32AlarmProcess(const otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    otEXPECT(HasAnyAlarmFired());

    AlarmHandle       *msAlarm    = GetFirstFiredAlarm(sMsAlarmHandles);
    const AlarmHandle *msAlarmEnd = sMsAlarmHandles + RADIO_INTERFACE_COUNT;

    StackAlarmCallback alarmCb;

    while (msAlarm != NULL)
    {
#if OPENTHREAD_CONFIG_DIAG_ENABLE
        if (otPlatDiagModeGet())
        {
            alarmCb = otPlatDiagAlarmFired;
        }
        else
#endif
        {
            alarmCb = otPlatAlarmMilliFired;
        }
        ProcessAlarm(msAlarm, alarmCb);
        msAlarm = GetNextFiredAlarm(msAlarm, msAlarmEnd);
    }
#if OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE
    AlarmHandle       *usAlarm    = GetFirstFiredAlarm(sUsAlarmHandles);
    const AlarmHandle *usAlarmEnd = sUsAlarmHandles + RADIO_INTERFACE_COUNT;

    while (usAlarm != NULL)
    {
        alarmCb = otPlatAlarmMicroFired;
        ProcessAlarm(usAlarm, alarmCb);
        usAlarm = GetNextFiredAlarm(usAlarm, usAlarmEnd);
    }
#endif
exit:
    return;
}

uint64_t efr32AlarmPendingTime(otInstance *aInstance)
{
    uint64_t remaining = GetPendingTime(aInstance);
    uint32_t now       = otPlatAlarmMilliGetNow();

    otEXPECT_ACTION(GetAlarmHandle(sMsAlarmHandles, aInstance)->mIsRunning, remaining = 0);

    if (remaining > now)
    {
        remaining -= (uint64_t)now;
    }

exit:
    return remaining;
}

bool efr32AlarmIsRunning(otInstance *aInstance)
{
    return (otInstanceIsInitialized(aInstance) ? GetAlarmHandle(sMsAlarmHandles, aInstance)->mIsRunning : false);
}

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
// Callback to determine if the system can sleep after an interrupt has fired
bool efr32AlarmIsReady(void)
{
    return HasAnyAlarmFired();
}
#endif // SL_CATALOG_POWER_MANAGER_PRESENT

uint32_t otPlatAlarmMilliGetNow(void)
{
    return sMsAlarmHandles[0].mTimerGetNow();
}

uint16_t otPlatTimeGetXtalAccuracy(void)
{
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // For sleepies, we need to account for the low-frequency crystal
    // accuracy when they go to sleep.  Accounting for that as well,
    // for the worst case.
    if (efr32AllowSleepCallback())
    {
        return SL_OPENTHREAD_HFXO_ACCURACY + SL_OPENTHREAD_LFXO_ACCURACY;
    }
#endif
    return SL_OPENTHREAD_HFXO_ACCURACY;
}

void otPlatAlarmMilliStartAt(otInstance *aInstance, uint32_t aT0, uint32_t aDt)
{
    AlarmHandle *alarm = GetAlarmHandle(sMsAlarmHandles, aInstance);
    if (alarm->mThreadInstance == NULL)
    {
        alarm->mThreadInstance = aInstance;
    }

    SetPendingTime(aInstance, (uint64_t)aT0 + (uint64_t)aDt);
    CORE_ATOMIC_SECTION(StartAlarmAt(alarm, aT0, aDt);)
}

void otPlatAlarmMilliStop(otInstance *aInstance)
{
    CORE_ATOMIC_SECTION(StopActiveAlarm(GetAlarmHandle(sMsAlarmHandles, aInstance));)
}

uint32_t otPlatAlarmMicroGetNow(void)
{
    return sUsAlarmHandles[0].mTimerGetNow();
}

// Note: This function should be called at least once per wrap
// period for the wrap-around logic to work below
uint64_t otPlatTimeGet(void)
{
    static uint32_t timerWraps   = 0U;
    static uint32_t prev32TimeUs = 0U;
    uint32_t        now32TimeUs;
    uint64_t        now64TimeUs;
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_CRITICAL();
    now32TimeUs = sl_rail_get_time(SL_RAIL_EFR32_HANDLE);
    if (now32TimeUs < prev32TimeUs)
    {
        timerWraps += 1U;
    }
    prev32TimeUs = now32TimeUs;
    now64TimeUs  = ((uint64_t)timerWraps << 32) + now32TimeUs;
    CORE_EXIT_CRITICAL();
    return now64TimeUs;
}

void otPlatAlarmMicroStartAt(otInstance *aInstance, uint32_t aT0, uint32_t aDt)
{
    AlarmHandle *alarm = GetAlarmHandle(sUsAlarmHandles, aInstance);
    if (alarm->mThreadInstance == NULL)
    {
        alarm->mThreadInstance = aInstance;
    }

    CORE_ATOMIC_SECTION(StartAlarmAt(alarm, aT0, aDt);)
}

void otPlatAlarmMicroStop(otInstance *aInstance)
{
    CORE_ATOMIC_SECTION(StopActiveAlarm(GetAlarmHandle(sUsAlarmHandles, aInstance));)
}
