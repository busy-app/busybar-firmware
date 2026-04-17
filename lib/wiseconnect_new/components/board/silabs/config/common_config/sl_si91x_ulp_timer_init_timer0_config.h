/***************************************************************************/ /**
 * @file sl_si91x_ulp_timer_init_timer0_config.h
 * @brief ULP Timer timer0 configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SI91X_ULP_TIMER_TIMER0_CONFIG_H
#define SL_SI91X_ULP_TIMER_TIMER0_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_ulp_timer.h"
/******************************************************************************/
/******************************* ULP-Timer clock Configuration **************************/
// <h> timer0 Configuration

// <o SL_ULP_TIMER_TIMER0_TYPE> Timer type
//   <ULP_TIMER_TYP_DEFAULT=> Down Counter (auto select)
//   <ULP_TIMER_TYP_1US=> 1 Micro-Sec
//   <ULP_TIMER_TYP_256US=> 256-Micro-Sec
// <i> Selection of the Timer type
#define SL_ULP_TIMER_TIMER0_TYPE ULP_TIMER_TYP_DEFAULT

// <o SL_ULP_TIMER_TIMER0_MODE> Timer mode
//   <ULP_TIMER_MODE_ONESHOT=> One-shot
//   <ULP_TIMER_MODE_PERIODIC=> Periodic (auto select)
// <i> Selection of the Timer mode
#define SL_ULP_TIMER_TIMER0_MODE ULP_TIMER_MODE_PERIODIC

// <o SL_ULP_TIMER_TIMER0_DIRECTION> Timer direction
//   <DOWN_COUNTER=> Down (auto select)
//   <UP_COUNTER=> Up
// <i> Selection of the Timer direction
#define SL_ULP_TIMER_TIMER0_DIRECTION DOWN_COUNTER

// </h> End timer0 Configuration
/******************************************************************************/
// <<< end of configuration section >>>

#endif /* SL_SI91X_ULP_TIMER_TIMER0_CONFIG_H */
