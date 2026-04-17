/*******************************************************************************
 * @file
 * @brief Multi-instance CLI support
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

#if SL_OPENTHREAD_MULTI_INSTANCE_CLI_ENABLE

#include "multi_instance_cli.h"
#include "sl_ot_custom_cli.h"
#include <openthread/cli.h>
#include <openthread/instance.h>
#include "common/code_utils.hpp"
#include "common/debug.hpp"

#ifdef SL_CATALOG_KERNEL_PRESENT
#include "sl_ot_rtos_adaptation.h"
#endif // SL_CATALOG_KERNEL_PRESENT

/**
 * This function initializes the CLI app.
 *
 * @param[in]  aInstance  The OpenThread instance structure.
 *
 */
extern void otAppCliInit(otInstance *aInstance);

// Static state variables
static uint8_t sCurrentInstanceIndex = 0;
static uint8_t sNewInstanceIndex     = 0;

uint8_t sl_ot_get_current_instance_index(void)
{
    return sCurrentInstanceIndex;
}

void sl_ot_set_new_instance_index(uint8_t aInstanceIndex)
{
    sNewInstanceIndex = aInstanceIndex;
}

uint8_t sl_ot_get_new_instance_index(void)
{
    return sNewInstanceIndex;
}

void sl_ot_set_current_instance_index(uint8_t aInstanceIndex)
{
    sCurrentInstanceIndex = aInstanceIndex;
}

void sl_ot_switch_to_instance_index(uint8_t aInstanceIndex)
{
    OT_ASSERT(aInstanceIndex < OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_NUM);

    otInstance *instance = otInstanceGetInstance(sCurrentInstanceIndex);

    // Deinitialize CLI for current instance
    if (instance != NULL)
    {
        otCliOutputFormat("Switching from instance %d\r\n", sCurrentInstanceIndex);
    }

    // Move to given instance
    sCurrentInstanceIndex = aInstanceIndex;

    // Initialize CLI for new instance
    instance = otInstanceGetInstance(aInstanceIndex);
    if (instance != NULL)
    {
        otAppCliInit(instance);
        sl_ot_custom_cli_init();
        otCliOutputFormat("Switched to instance %d\r\n", aInstanceIndex);
    }
}

bool sl_ot_should_change_instance(void)
{
    return sCurrentInstanceIndex != sNewInstanceIndex;
}

static otError instanceListCommand(void *aContext, uint8_t argc, char *argv[])
{
    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);

    uint8_t currentIndex = sl_ot_get_current_instance_index();

    for (int i = 0; i < OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_NUM; i++)
    {
        otInstance *instance = otInstanceGetInstance(i);
        if (instance != NULL && otInstanceIsInitialized(instance))
        {
            otCliOutputFormat("%c Index: %d, Id: %lu\r\n",
                              (i == currentIndex) ? '*' : ' ',
                              i,
                              otInstanceGetId(instance));
        }
    }

    return OT_ERROR_NONE;
}

static otError instanceGetCommand(void *aContext, uint8_t argc, char *argv[])
{
    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);

    otError error = OT_ERROR_NONE;

    uint8_t instanceIndex = sl_ot_get_current_instance_index();
    otCliOutputFormat("Current instance index: %d\r\n", instanceIndex);

    return error;
}

static otError instanceSetCommand(void *aContext, uint8_t argc, char *argv[])
{
    OT_UNUSED_VARIABLE(aContext);

    otError error = OT_ERROR_NONE;
    VerifyOrExit(argc == 1, error = OT_ERROR_INVALID_ARGS);

    uint8_t instanceIndex = (uint8_t)strtoul(argv[0], NULL, 10);
    VerifyOrExit(instanceIndex < OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_NUM, error = OT_ERROR_INVALID_ARGS);

    sl_ot_set_new_instance_index(instanceIndex);

#ifdef SL_CATALOG_KERNEL_PRESENT
    // Signal the app task to wake up and process the instance switch in RTOS mode
    sl_ot_rtos_set_pending_event(SL_OT_RTOS_EVENT_APP);
#endif // SL_CATALOG_KERNEL_PRESENT

exit:
    return error;
}

static otError helpCommand(void *aContext, uint8_t argc, char *argv[]);

static otCliCommand instanceCommands[] = {
    {"help", helpCommand},
    {"get", instanceGetCommand},
    {"list", instanceListCommand},
    {"set", instanceSetCommand},
};

otError instanceCommandHandler(void *context, uint8_t argc, char *argv[])
{
    otError error = processCommand(context, argc, argv, OT_ARRAY_LENGTH(instanceCommands), instanceCommands);

    if (error == OT_ERROR_INVALID_COMMAND)
    {
        (void)helpCommand(NULL, 0, NULL);
    }

    return error;
}

static otError helpCommand(void *context, uint8_t argc, char *argv[])
{
    OT_UNUSED_VARIABLE(context);
    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);

    printCommands(instanceCommands, OT_ARRAY_LENGTH(instanceCommands));

    return OT_ERROR_NONE;
}

#endif // SL_OPENTHREAD_MULTI_INSTANCE_CLI_ENABLE
