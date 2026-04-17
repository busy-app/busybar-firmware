/***************************************************************************/ /**
 * @file sl_si91x_ulp_timer_init_inst_config.h
 * @brief ULP Timer configuration file.
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

#ifndef SL_SI91X_ULP_TIMER_INSTANCE_CONFIG_H
#define SL_SI91X_ULP_TIMER_INSTANCE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* NOTE: User should configure all macros defined below, while creating an
 * instance other than pre-defined one */

#if USER_CONFIGURATION_ENABLE

#include "sl_si91x_ulp_timer.h"
#define SL_ULP_TIMER_TIMER0 0
#define SL_ULP_TIMER_TIMER1 1
#define SL_ULP_TIMER_TIMER2 2
#define SL_ULP_TIMER_TIMER3 3
/******************************************************************************/
/******************************* ULP-timer Configuration **************************/
// <<< Use Configuration Wizard in Context Menu >>>
// <h>ULP Timer INSTANCE Configuration

// <o SL_ULP_TIMER_INSTANCE_TYPE> Timer type
//   <ULP_TIMER_TYP_DEFAULT=> Down Counter (auto select)
//   <ULP_TIMER_TYP_1US=> 1 Micro-Sec
//   <ULP_TIMER_TYP_256US=> 256-Micro-Sec
// <i> Selection of the Timer type
#define SL_ULP_TIMER_INSTANCE_TYPE ULP_TIMER_TYP_DEFAULT

// <o SL_ULP_TIMER_INSTANCE_MODE> Timer mode
//   <ULP_TIMER_MODE_ONESHOT=> One-shot
//   <ULP_TIMER_MODE_PERIODIC=> Periodic (auto select)
// <i> Selection of the Timer mode
#define SL_ULP_TIMER_INSTANCE_MODE ULP_TIMER_MODE_PERIODIC

// <o SL_ULP_TIMER_INSTANCE_DIRECTION> Timer direction
//   <DOWN_COUNTER=> Down (auto select)
//   <UP_COUNTER=> Up
// <i> Selection of the Timer direction
#define SL_ULP_TIMER_INSTANCE_DIRECTION DOWN_COUNTER

// </h>
/******************************************************************************/
// <<< end of configuration section >>>

#else

#warning \
  "The sl_si91x_ulp_timer_init_INSTANCE_config.h file requires specifying the INSTANCE name for custom ULP-timer configurations. To properly configure ULP-timer either enable user configuration by installing the [ENABLE USER CONFIGURATION] component or set USER_CONFIGURATION_ENABLE to 1, and then define all required MACROS in sl_si91x_ulp_timer_init_INSTANCE_config.h according to your board specifications."

#endif

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_ULP_TIMER_INSTANCE_CONFIG_H */
