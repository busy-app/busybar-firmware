/*******************************************************************************
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// Define module name for Power Manager debuging feature.
#define CURRENT_MODULE_NAME "OPENTHREAD_MULTI_INSTANCE_APP"

#include <assert.h>
#include <openthread-core-config.h>
#include <openthread/config.h>

#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>

#include "app.h"
#include "openthread-system.h"

#include "reset_util.h"

#include "sl_component_catalog.h"
#include "sl_memory_manager.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif

#ifdef SL_CATALOG_KERNEL_PRESENT
#include "sl_ot_rtos_adaptation.h"
#endif // SL_CATALOG_KERNEL_PRESENT

#include "sl_ot_custom_cli.h"
#include <common/code_utils.hpp>
#include <common/debug.hpp>
#include "cli/multi_instance_cli.h"

/**
 * This function initializes the CLI app.
 *
 * @param[in]  aInstance  The OpenThread instance structure.
 *
 */
extern void otAppCliInit(otInstance *aInstance);

// Static instance variables
static otInstance *sInstances[OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_NUM] = {NULL};

/*
 * Provide, if required an "otPlatLog()" function
 */
#if OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_APP
void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, ...)
{
    OT_UNUSED_VARIABLE(aLogLevel);
    OT_UNUSED_VARIABLE(aLogRegion);
    OT_UNUSED_VARIABLE(aFormat);

    va_list ap;
    va_start(ap, aFormat);
    otCliPlatLogv(aLogLevel, aLogRegion, aFormat, ap);
    va_end(ap);
}
#endif

void sl_ot_create_instance(void)
{
    // Create all instances using otInstanceInitMultiple
    for (int i = 0; i < OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_NUM; i++)
    {
        // Initialize OpenThread instance with static buffer
        sInstances[i] = otInstanceInitMultiple(i);
        OT_ASSERT(sInstances[i]);
        OT_ASSERT(otInstanceIsInitialized(sInstances[i]));
    }
}

void sl_ot_cli_init(void)
{
    uint8_t initialInstance = 0;
    otAppCliInit(sInstances[initialInstance]);
    otCliOutputFormat("Initialized CLI for instance %d\n", initialInstance);
    otCliOutputFormat("Use 'instance list' to list all instances\n");
    otCliOutputFormat("Use 'instance set <index>' to change between instances (0-%d)\n",
                      OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_NUM - 1);
}

#ifdef SL_CATALOG_KERNEL_PRESENT
/******************************************************************************
 * RTOS Application Tick.
 * This function is called by the RTOS app task to handle application-level
 * processing, including instance switching.
 *****************************************************************************/
void sl_ot_rtos_application_tick(void)
{
    if (sl_ot_should_change_instance())
    {
        sl_ot_switch_to_instance_index(sl_ot_get_new_instance_index());
    }
}
#endif // SL_CATALOG_KERNEL_PRESENT

/******************************************************************************
 * Application Init.
 *****************************************************************************/

void app_init(void)
{
    OT_SETUP_RESET_JUMP(argv);
}

/******************************************************************************
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void)
{
    if (sl_ot_should_change_instance())
    {
        sl_ot_switch_to_instance_index(sl_ot_get_new_instance_index());
    }

    // Process tasklets for all instances
    for (int i = 0; i < OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_NUM; i++)
    {
        if (sInstances[i] != NULL)
        {
            otTaskletsProcess(sInstances[i]);
            otSysProcessDrivers(sInstances[i]);
        }
    }
}

/******************************************************************************
 * Application Exit.
 *****************************************************************************/
void app_exit(void)
{
    // Finalize all instances
    for (int i = 0; i < OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_NUM; i++)
    {
        if (sInstances[i] != NULL)
        {
            otInstanceFinalize(sInstances[i]);
            sInstances[i] = NULL;
        }
    }
}
