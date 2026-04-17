/*******************************************************************************
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <assert.h>
#include <openthread-core-config.h>
#include <openthread/config.h>

#include <openthread/diag.h>
#include <openthread/ncp.h>
#include <openthread/tasklet.h>

#include "app.h"
#include "openthread-system.h"

#include "reset_util.h"

#include "sl_component_catalog.h"
#include "sl_memory_manager.h"

#if SL_OPENTHREAD_ENABLE_HOST_WAKE_GPIO
#include "sl_gpio.h"
sl_gpio_t host_wakeup_gpio;
#endif

/**
 * This function initializes the NCP app.
 *
 * @param[in]  aInstance  The OpenThread instance structure.
 *
 */
extern void otAppNcpInit(otInstance *aInstance);

static otInstance *sInstance = NULL;

void sl_ot_create_instance(void)
{
    sInstance = otInstanceInitSingle();
    assert(sInstance);
}

void sl_ot_ncp_init(void)
{
    otAppNcpInit(sInstance);
}

/******************************************************************************
 * Application Init.
 *****************************************************************************/
OT_TOOL_WEAK void sl_host_wakeup_init(void)
{
#if SL_OPENTHREAD_ENABLE_HOST_WAKE_GPIO
    host_wakeup_gpio.port = (uint8_t)SL_OPENTHREAD_HOST_WAKEUP_GPIO_PORT;
    host_wakeup_gpio.pin  = (uint8_t)SL_OPENTHREAD_HOST_WAKEUP_GPIO_PIN;
    sl_gpio_set_pin_mode(&host_wakeup_gpio, SL_GPIO_MODE_PUSH_PULL, 0);
#endif
}

void app_init(void)
{
    OT_SETUP_RESET_JUMP(argv);
    sl_host_wakeup_init();
}

/******************************************************************************
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void)
{
    otTaskletsProcess(sInstance);
    otSysProcessDrivers(sInstance);
}

/******************************************************************************
 * Application Exit.
 *****************************************************************************/
void app_exit(void)
{
    otInstanceFinalize(sInstance);
    // TO DO : pseudo reset?
}
