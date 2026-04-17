/***************************************************************************//**
 * @file
 * @brief Application Runtime Adaptor (RTA) - Internal header for bare-metal.
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
#ifndef APP_RTA_INTERNAL_BM_H
#define APP_RTA_INTERNAL_BM_H

#include <stdbool.h>
#include <stdint.h>
#include "sl_power_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * Start Application Runtime Adaptor (RTA) module.
 * @note Called by the system.
 *****************************************************************************/
void app_rta_internal_init(void);

/**************************************************************************//**
 * Initialize components that was template contributed to RTA init.
 * @note implementation is automatically generated.
 *****************************************************************************/
void app_rta_init_contributors(void);

/**************************************************************************//**
 * Finalize initialization for components that were template contributed to
 * RTA ready.
 * @note implementation is automatically generated.
 *****************************************************************************/
void app_rta_ready(void);

/**************************************************************************//**
 * Application Runtime Adaptor (RTA) step function.
 * @note Called by the system.
 *****************************************************************************/
void app_rta_step(void);

/**************************************************************************//**
 * Check if it is ok to sleep now / power manager
 * @return true - if ready to sleep, false otherwise
 *****************************************************************************/
bool app_rta_is_ok_to_sleep(void);

/**************************************************************************//**
 * Routine for power manager handler
 * @return sl_power_manager_on_isr_exit_t
 * @retval SL_POWER_MANAGER_IGNORE = (1UL << 0UL), < The module did not trigger
 *         an ISR and it doesn't want to contribute to the decision
 * @retval SL_POWER_MANAGER_SLEEP  = (1UL << 1UL), < The module was the one that
 *         caused the system wakeup and the system SHOULD go back to sleep
 * @retval SL_POWER_MANAGER_WAKEUP = (1UL << 2UL), < The module was the one that
 *         caused the system wakeup and the system MUST NOT go back to sleep
 *****************************************************************************/
sl_power_manager_on_isr_exit_t app_rta_on_isr_exit(void);

#ifdef __cplusplus
};
#endif

#endif // APP_RTA_INTERNAL_BM_H
