/***************************************************************************/ /**
 * @file sl_si91x_opamp_opamp1_config.h
 * @brief OPAMP opamp1 configuration file.
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

#ifndef SL_SI91X_OPAMP_OPAMP1_CONFIG_H
#define SL_SI91X_OPAMP_OPAMP1_CONFIG_H

#include "sl_si91x_opamp.h"
// <<< Use Configuration Wizard in Context Menu >>>
#ifdef __cplusplus
extern "C" {
#endif
/******************** OPAMP Configuration ******************************/

#define SL_OPAMP_OPAMP1 SL_OPAMP_1

//  <e> OPAMP1 Input/Output Configuration Selection
#define OPAMP1_UC 1

// <o SL_OPAMP_OPAMP1_CONFIGURATION_SELECTION> OPAMP Feature Selection
//   <SL_OPAMP_UNITY_GAIN=> Unity Gain
//   <SL_OPAMP_INVERTING_PROGRAMMABLE_GAIN_AMPLIFIER=> Inverting PGA
//   <SL_OPAMP_NON_INVERTING_PROGRAMMABLE_GAIN_AMPLIFIER=> Non-Inverting PGA
//   <SL_OPAMP_INVERTING_PROGRAMMABLE_HYST_COMP=> Inverting Hysteresis Comparator
//   <SL_OPAMP_NON_INVERTING_PROGRAMMABLE_HYST_COMP=> Non-Inverting Hysteresis Comparator
//   <SL_OPAMP_CASCADED_INVERTING_PROGRAMMABLE_GAIN_AMPLIFIER=>  Cascaded Inverting PGA
//   <SL_OPAMP_CASCADED_NON_INVERTING_PROGRAMMABLE_GAIN_AMPLIFIER=> Cascaded Non-inverting PGA
//   <SL_OPAMP_TWO_OPAMPS_DIFFERENTIAL_AMPLIFIER=> Two OPAMPs Differential Amplifier
//   <SL_OPAMP_TRANS_IMPEDANCE_AMPLIFIER=> Trans-Impedance Amplifier
//   <SL_OPAMP_INSTRUMENTATION_AMPLIFIER=> Instrumentation Amplifier
// <i> Selection of OPAMP Feature

#define SL_OPAMP_OPAMP1_CONFIGURATION_SELECTION SL_OPAMP_UNITY_GAIN

// OPAMP1 VinP
// <o SL_OPAMP1_VINP_SEL> VinP Mux Selection
//   <0=> P0 GPIO_27
//   <1=>  P1 ULP_GPIO_7
//   <2=>  P2 ULP_GPIO_0
//   <3=>  P3 ULP_GPIO_2
//   <4=>  P4 ULP_GPIO_6
//   <5=>  P5 ULP_GPIO_8
//   <6=>  P6 DAC
//   <7=>  P7 Resistor_tap
// <i> Selection of OPAMP1 VinP
#define SL_OPAMP1_VINP_SEL 0

// OPAMP1 VinN
// <o SL_OPAMP1_VINN_SEL> VinN Mux Selection
//   <0=> N0 GPIO_27
//   <1=> N1 ULP_GPIO_7
//   <2=> N2 DAC
//   <3=> N3 Resistor_tap
//   <4=> N4 Out
// <i> Selection of OPAMP1 VinN
#define SL_OPAMP1_VINN_SEL 4

// OPAMP1 Resistor
// <o SL_OPAMP1_VREF_SEL> Resistor Mux Selection
//   <10=> None
//   <0=>  Res0 GPIO_27
//   <1=>  Res1 ULP_GPIO_7
//   <2=>  Res2 ULP_GPIO_0
//   <3=>  Res3 ULP_GPIO_2
//   <4=>  Res4 ULP_GPIO_6
//   <5=>  Res5 ULP_GPIO_8
//   <6=>  Res6 DAC
// <i> Selection of OPAMP1 Resistor
#define SL_OPAMP1_VREF_SEL 10

// <o SL_OPAMP1_LP_MODE> OPAMP_1 Power mode
//   <0=> HP mode
//   <1=> LP mode
// <i> Selection of OPAMP_1 Power Mode
#define SL_OPAMP1_LP_MODE 0

// OPAMP1 R1 Resistor
// <o SL_OPAMP1_R1_SELECT> R1 Resistor (kΩ)
//   <0=> None
//   <1=> 20
//   <2=> 60
//   <3=> 140
// <i> Selection of OPAMP1 R1
#define SL_OPAMP1_R1_SELECT 0

// OPAMP1 R2 Resistor
// <o SL_OPAMP1_R2_SELECT> R2 Resistor (kΩ)
//   <8=> None
//   <0=> 20
//   <1=> 30
//   <2=> 40
//   <3=> 60
//   <4=> 120
//   <5=> 250
//   <6=> 500
//   <7=> 1000
// <i> Selection of OPAMP1 R2
#define SL_OPAMP1_R2_SELECT 8

// </e> OPAMP1
/******************************************************************************/
// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <opamp1 signal=(P),(N),(RES),(OUT)> SL_OPAMP1
// $[OPAMP1_SL_OPAMP1]
#ifndef SL_OPAMP1_PERIPHERAL
#define SL_OPAMP1_PERIPHERAL OPAMP1
#endif
#ifndef SL_OPAMP1_PERIPHERAL_NO
#define SL_OPAMP1_PERIPHERAL_NO 1
#endif

// [OPAMP1_SL_OPAMP1]$
// <<< sl:end pin_tool >>>

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_OPAMP_OPAMP1_CONFIG_H