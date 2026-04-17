/***************************************************************************//**
 * @file
 * @brief Timer service internal header
 *
 * Timer service for applications with less strict timing requirements.
 * This module is based on the sleeptimer, but runs the timer callbacks in
 * non-interrupt context. This behavior gives more flexibility for the callback
 * implementation but causes a less precise timing.
 *
 * @note If your application requires precise timing, please use the sleeptimer
 *       directly.
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
#ifndef APP_TIMER_INTERNAL_H
#define APP_TIMER_INTERNAL_H

#include <stdbool.h>
#include "sl_component_catalog.h"
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif // SL_CATALOG_POWER_MANAGER_PRESENT

/***************************************************************************//**
 * Execute timer callback functions.
 *
 ******************************************************************************/
void sli_app_timer_step(void);

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
/***************************************************************************//**
 * Routine for power manager handler
 *
 * @return SL_POWER_MANAGER_WAKEUP if there is an unhandled timer event
 ******************************************************************************/
sl_power_manager_on_isr_exit_t sli_app_timer_sleep_on_isr_exit(void);

/***************************************************************************//**
 * Checks if it is ok to sleep now
 *
 * @return false if there is an unhandled timer event
 ******************************************************************************/
bool sli_app_timer_is_ok_to_sleep(void);
#endif // SL_CATALOG_POWER_MANAGER_PRESENT

#endif // APP_TIMER_INTERNAL_H
