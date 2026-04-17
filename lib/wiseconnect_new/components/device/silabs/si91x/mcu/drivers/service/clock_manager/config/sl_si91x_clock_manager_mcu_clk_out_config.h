/***************************************************************************/ /**
 * @file sl_si91x_clock_manager_mcu_clk_out_config.h
 * @brief MCU Clock Out configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
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
#ifndef SL_SI91X_CLOCK_MANAGER_MCU_CLK_OUT_CONFIG_H
#define SL_SI91X_CLOCK_MANAGER_MCU_CLK_OUT_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Clock Manager Configuration **************************/
// <<< Use Configuration Wizard in Context Menu >>>
// <e> MCU Clock Out Configuration

//  <i> Enable: Peripheral configuration is taken straight from the configuration set in the universal configuration (UC).
//  <i> Disable: If the application demands it to be modified during runtime, use the sl_si91x_clock_manager API to modify the peripheral configuration.
// <i> Default: 1
#define SL_SI91X_MCU_CLK_OUT_EN 1

// <o SL_CLOCK_MANAGER_MCU_CLK_OUT_SOURCE> MCU Clock Source
//   <SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_RC_32MHZ=> High Frequency RC Clock source
//   <SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_XTAL=> XTAL Clock source
//   <SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_RC_32KHZ=> Low Frequency RC Clock source
//   <SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_XTAL_32KHZ=> Low Frequency XTAL Clock source
//   <SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_INTF_PLL=> Interface PLL Clock
//   <SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_SOC_PLL=> SoC PLL Clock
//   <SL_CLOCK_MANAGER_I2S_PLL_CLK=> I2S PLL Clock
// <i> Selection of MCU Clock Output Source
#define SL_CLOCK_MANAGER_MCU_CLK_OUT_SOURCE SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_SOC_PLL

// <o SL_CLOCK_MANAGER_MCU_CLK_OUT_DIV_FACTOR> MCU Clock Out Division Factor <0-63>
// <i> Division factor for mcu clock output. Default: 0
#define SL_CLOCK_MANAGER_MCU_CLK_OUT_DIV_FACTOR 0

// </e>
// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <mcu_clk_out signal=GPIO> SL_SI91X_MCU_CLK_OUT

// $[MCU_CLK_OUT_SL_SI91X_MCU_CLK_OUT]
#ifndef SL_SI91X_MCU_CLK_OUT_PERIPHERAL
#define SL_SI91X_MCU_CLK_OUT_PERIPHERAL MCU_CLK_OUT
#endif

// MCU_CLK_OUT GPIO on GPIO_11
#ifndef SL_SI91X_MCU_CLK_OUT_GPIO_PORT
#define SL_SI91X_MCU_CLK_OUT_GPIO_PORT HP
#endif
#ifndef SL_SI91X_MCU_CLK_OUT_GPIO_PIN
#define SL_SI91X_MCU_CLK_OUT_GPIO_PIN 11
#endif
#ifndef SL_SI91X_MCU_CLK_OUT_GPIO_LOC
#define SL_SI91X_MCU_CLK_OUT_GPIO_LOC 0
#endif
// [MCU_CLK_OUT_SL_SI91X_MCU_CLK_OUT]$
// <<< sl:end pin_tool >>>

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SL_SI91X_CLOCK_MANAGER_MCU_CLK_OUT_CONFIG_H
