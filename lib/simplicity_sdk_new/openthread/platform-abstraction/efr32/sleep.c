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
 *   This file implements the OpenThread platform abstraction for power (sleep)
 * management.
 *
 */

#define CURRENT_MODULE_NAME "OPENTHREAD"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#include "alarm.h"
#include "sl_core.h"
#include "sleep.h"

#include "platform-efr32.h"
#include <assert.h>
#include <openthread-core-config.h>
#include <openthread/tasklet.h>
#include <openthread/platform/toolchain.h>

#include "radio_instance.h"

#include "utils/code_utils.h"

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif // SL_CATALOG_POWER_MANAGER_PRESENT

//------------------------------------------------------------------------------
// Forward declarations

#if (defined(SL_CATALOG_POWER_MANAGER_PRESENT))

static void setWakeRequirement(bool aShouldWake);
static bool isPlatformEventPending(void);
static bool shouldStayAwake(void);
static bool instanceShouldInterruptSleep(otInstance *aInstance);
static bool shouldInterruptSleep(void);

#endif // SL_CATALOG_POWER_MANAGER_PRESENT

//------------------------------------------------------------------------------
// Static variables

static bool sWakeRequirementSet = false;

//------------------------------------------------------------------------------
// Internal APIs and callbacks

void sl_ot_sleep_init(void)
{
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    setWakeRequirement(true);
#else
    sWakeRequirementSet = true;
#endif // SL_CATALOG_POWER_MANAGER_PRESENT
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
OT_TOOL_WEAK bool efr32AllowSleepCallback(void)
{
    return true;
}

#if (defined(SL_CATALOG_POWER_MANAGER_PRESENT))

// This is invoked only the bare metal case.
bool sl_ot_is_ok_to_sleep(void)
{
    return !(sWakeRequirementSet || shouldInterruptSleep());
}

// This is invoked only the bare metal case.
sl_power_manager_on_isr_exit_t sl_ot_sleep_on_isr_exit(void)
{
    return (isPlatformEventPending() ? SL_POWER_MANAGER_WAKEUP : SL_POWER_MANAGER_IGNORE);
}

void sl_ot_sleep_update(void)
{
    CORE_CRITICAL_SECTION(setWakeRequirement(shouldStayAwake());)
}

//------------------------------------------------------------------------------
// Static functions

/**
 * @brief Set the wake requirement for the system.
 *
 * @param[in] aShouldWake  True if the system should wake up.
 *
 */
static void setWakeRequirement(bool aShouldWake)
{
    otEXPECT(aShouldWake != sWakeRequirementSet);
    void (*em_operation)(sl_power_manager_em_t) =
        aShouldWake ? &sl_power_manager_add_em_requirement : &sl_power_manager_remove_em_requirement;
    (*em_operation)(SL_POWER_MANAGER_EM1);
    sWakeRequirementSet = aShouldWake;
exit:
    return;
}

/**
 * @brief Check if there is any platform event pending.
 *
 * @return True if there is a platform event pending.
 *
 */
static bool isPlatformEventPending(void)
{
    bool isPending = efr32AlarmIsReady();

#if defined(SL_CATALOG_OPENTHREAD_UART_PRESENT) \
    && (defined(SL_CATALOG_IOSTREAM_EUSART_PRESENT) || defined(SL_CATALOG_IOSTREAM_USART_PRESENT))
    isPending = isPending || efr32UartIsDataReady();
#endif

    return isPending;
}

/**
 * @brief Check if the system should stay awake.
 *
 * @return True if the system should stay awake.
 *
 */
static bool shouldStayAwake(void)
{
    bool shouldWake = (!(efr32AllowSleepCallback()) || isPlatformEventPending());

#if defined SL_CATALOG_KERNEL_PRESENT
    shouldWake = shouldWake || shouldInterruptSleep();
#endif

    return shouldWake;
}

/**
 * @brief Check if individual instance should interrupt sleep.
 *
 * @param[in] aInstance  A pointer to an OpenThread instance.
 *
 * @return True if the system should interrupt sleep.
 *
 */
static bool instanceShouldInterruptSleep(otInstance *aInstance)
{
    bool shouldWake = true;

    otEXPECT(aInstance != NULL);

    shouldWake = otTaskletsArePending(aInstance);

    shouldWake = shouldWake
                 || (efr32AlarmIsRunning(aInstance)
                     && efr32AlarmPendingTime(aInstance) < OPENTHREAD_CONFIG_MIN_SLEEP_DURATION_MS);

exit:
    return shouldWake;
}

/**
 * @brief Check if the system should interrupt sleep.
 *
 * @details This function should be used to prevent power manager from entering sleep mode
 *          based on events that happen after the OpenThread power manager module complete.
 *
 * @return True if the system should interrupt sleep.
 *
 */
static bool shouldInterruptSleep(void)
{
    CORE_ATOMIC_IRQ_DISABLE();

    otInstance *instance;
    bool        shouldWake = false;

    uint8_t instanceIndex = 0;

    while ((!shouldWake) && instanceIndex < RADIO_INTERFACE_COUNT)
    {
        // Use instance offset from multipan index for multipan configuration
        instance   = sli_ot_radio_instance_get(instanceIndex);
        shouldWake = instanceShouldInterruptSleep(instance);
        instanceIndex++;
    }

    CORE_ATOMIC_IRQ_ENABLE();

    return shouldWake;
}
#endif // SL_CATALOG_POWER_MANAGER_PRESENT
