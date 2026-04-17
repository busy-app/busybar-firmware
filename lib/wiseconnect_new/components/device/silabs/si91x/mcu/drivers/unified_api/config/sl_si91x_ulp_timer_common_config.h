/***************************************************************************/ /**
 * @file sl_si91x_ulp_timer_common_config.h
 * @brief ULP Timer common configuration file.
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

#ifndef SL_SI91X_ULP_TIMER_COMMON_CONFIG_H
#define SL_SI91X_ULP_TIMER_COMMON_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_ulp_timer.h"
/******************************************************************************/
/******************** ADC Peripheral CommonConfiguration **********************/

// <<< Use Configuration Wizard in Context Menu >>>
// ULP Timer Clock Configuration

//  SL_ULP_TIMER_CLK_TYPE> Clock Type
//  SL_ULP_TIMER_CLK_TYPE_STATIC=> Static (auto select)
//  SL_ULP_TIMER_CLK_TYPE_DYNAMIC=> Dynamic
//  Selection of the Timer CLK Type.
#define SL_ULP_TIMER_CLK_TYPE SL_ULP_TIMER_CLK_TYPE_STATIC

// SL_ULP_TIMER_SYNC_TO_ULPSS_PCLK> Sync to ULPSS PCLK
// Enable or disable sync to ULPSS pclock.
// Default: 0
#define SL_ULP_TIMER_SYNC_TO_ULPSS_PCLK 0

//  SL_ULP_TIMER_SKIP_SWITCH_TIME> Wait for switching timer clock
//  1 : Enable waiting for switching timer clk & 0 : Skip waiting for switching timer clk.
//  Default: 0
#define SL_ULP_TIMER_SKIP_SWITCH_TIME 0

//  SL_ULP_TIMER_CLK_INPUT_SOURCE> clock source
//   ULP_TIMER_REF_CLK_SRC=> Ref clock
//   ULP_TIMER_32KHZ_RC_CLK_SRC=> 32KHZ_RC
//   ULP_TIMER_32KHZ_XTAL_CLK_SRC=> 32KHZ_XTAL
//   ULP_TIMER_MHZ_RC_CLK_SRC=> MHZ_RC
// Selection of the Clock source
#define SL_ULP_TIMER_CLK_INPUT_SOURCE ULP_TIMER_REF_CLK_SRC //default timer clock input source is ref clock

// <<< end of configuration section >>>

ulp_timer_clk_src_config_t sl_timer_clk_handle = {
  .ulp_timer_clk_type           = SL_ULP_TIMER_CLK_TYPE,
  .ulp_timer_sync_to_ulpss_pclk = SL_ULP_TIMER_SYNC_TO_ULPSS_PCLK,
  .ulp_timer_clk_input_src      = SL_ULP_TIMER_CLK_INPUT_SOURCE,
  .ulp_timer_skip_switch_time   = SL_ULP_TIMER_SKIP_SWITCH_TIME,
};

#ifdef __cplusplus
}
#endif // SL_ULP_TIMER
#endif /* SL_SI91X_ULP_TIMER_COMMON_CONFIG_H */