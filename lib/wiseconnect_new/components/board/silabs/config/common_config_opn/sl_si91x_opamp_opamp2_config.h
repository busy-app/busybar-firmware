/***************************************************************************/ /**
 * @file sl_si91x_opamp_opamp2_config.h
 * @brief OPAMP opamp2 configuration file.
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

#ifndef SL_SI91X_OPAMP_OPAMP2_CONFIG_H
#define SL_SI91X_OPAMP_OPAMP2_CONFIG_H

#include "sl_si91x_opamp.h"
// <<< Use Configuration Wizard in Context Menu >>>
#ifdef __cplusplus
extern "C" {
#endif
/******************** OPAMP Configuration ******************************/

#define SL_OPAMP_OPAMP2 SL_OPAMP_2

//  <e> OPAMP2 Input/Output Configuration Selection
#define OPAMP2_UC 1

// <o SL_OPAMP_OPAMP2_CONFIGURATION_SELECTION> OPAMP Feature Selection
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

#define SL_OPAMP_OPAMP2_CONFIGURATION_SELECTION SL_OPAMP_UNITY_GAIN

// OPAMP2 VinP
// <o SL_OPAMP2_VINP_SEL> VinP Mux Selection
//   <0=>  P0 ULP_GPIO_11
//   <1=>  P1 ULP_GPIO_5
//   <3=>  P3 DAC
//   <4=>  P4 Resistor_tap
//   <6=>  P6 OPAMP1_OUT
// <i> Selection of OPAMP2 VinP
#define SL_OPAMP2_VINP_SEL 0

// OPAMP2 VinN
// <o SL_OPAMP2_VINN_SEL> VinN Mux Selection
//   <0=>  N0 ULP_GPIO_11
//   <1=>  N1 DAC
//   <2=>  N2 Resistor_tap
//   <3=>  N3 Out
// <i> Selection of OPAMP2 VinN
#define SL_OPAMP2_VINN_SEL 3

// OPAMP2 Resistor
// <o SL_OPAMP2_VREF_SEL> Resistor Mux Selection
//   <10=> None
//   <0=>  Res0 ULP_GPIO_11
//   <1=>  Res1 ULP_GPIO_5
//   <3=>  Res3 DAC
//   <5=>  Res5 OPAMP1_OUT
// <i> Selection of OPAMP2 Resistor
#define SL_OPAMP2_VREF_SEL 10

// <o SL_OPAMP2_LP_MODE> OPAMP_2 Power mode
//   <0=> HP mode
//   <1=> LP mode
// <i> Selection of OPAMP_2 Power Mode
#define SL_OPAMP2_LP_MODE 0

// OPAMP2 R1 Resistor
// <o SL_OPAMP2_R1_SELECT> R1 Resistor (kΩ)
//   <0=> None
//   <1=> 20
//   <2=> 60
//   <3=> 140
// <i> Selection of OPAMP2 R1
#define SL_OPAMP2_R1_SELECT 0

// OPAMP2 R2 Resistor
// <o SL_OPAMP2_R2_SELECT> R2 Resistor (kΩ)
//   <8=> None
//   <0=> 20
//   <1=> 30
//   <2=> 40
//   <3=> 60
//   <4=> 120
//   <5=> 250
//   <6=> 500
//   <7=> 1000
// <i> Selection of OPAMP2 R2
#define SL_OPAMP2_R2_SELECT 8

// </e> OPAMP2
/******************************************************************************/
// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <opamp2 signal=(P),(N),(RES),(OUT)> SL_OPAMP2
// $[OPAMP2_SL_OPAMP2]
#ifndef SL_OPAMP2_PERIPHERAL
#define SL_OPAMP2_PERIPHERAL OPAMP2
#endif
#ifndef SL_OPAMP2_PERIPHERAL_NO
#define SL_OPAMP2_PERIPHERAL_NO 2
#endif

// [OPAMP2_SL_OPAMP2]$
// <<< sl:end pin_tool >>>

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_OPAMP_OPAMP2_CONFIG_H