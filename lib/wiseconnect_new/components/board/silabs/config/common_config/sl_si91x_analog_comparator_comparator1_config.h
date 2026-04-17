/***************************************************************************/ /**
* @file sl_si91x_analog_comparator_comparator1_config.h
* @brief Analog Comparator comparator1 configuration file.
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

#ifndef SL_SI91X_ANALOG_COMPARATOR_COMPARATOR1_CONFIG_H
#define SL_SI91X_ANALOG_COMPARATOR_COMPARATOR1_CONFIG_H

#include "sl_si91x_analog_comparator.h"

// <<< Use Configuration Wizard in Context Menu >>>
// <h>Analog Comparator Configuration

#define SL_ANALOG_COMPARATOR_COMPARATOR1 1
#define SL_ANALOG_COMPARATOR1_PIN_INPUT  1

// <o SL_ANALOG_COMPARATOR1_NON_INVERTING_INPUT> Non-inverting Input
//   <SL_COMPARATOR_EXTERNAL_GPIO_INPUT_0=> External Input-0
//   <SL_COMPARATOR_EXTERNAL_GPIO_INPUT_1=> External Input-1
//   <SL_COMPARATOR_INPUT_FROM_DAC_OUTPUT=> DAC OUTPUT as comparator input
//   <SL_COMPARATOR_INPUT_FROM_REF_BUFFER_OUTPUT=> Reference buffer output as comparator input
//   <SL_COMPARATOR_INPUT_FROM_REF_SCALER_OUTPUT=> Reference scaler output as comparator input
//   <SL_COMPARATOR_INPUT_FROM_RES_BANK_OUTPUT=> Resistor bank output as comparator input
//   <SL_COMPARATOR_INPUT_FROM_OPAMP1_OUTPUT=> OPAMP1 output as comparator input
// <i> Selection of Comparator Non-inverting input
#define SL_ANALOG_COMPARATOR1_NON_INVERTING_INPUT SL_COMPARATOR_EXTERNAL_GPIO_INPUT_1

// <o SL_ANALOG_COMPARATOR1_INVERTING_INPUT> Inverting Input
//   <SL_COMPARATOR_EXTERNAL_GPIO_INPUT_0=> External Input-0
//   <SL_COMPARATOR_EXTERNAL_GPIO_INPUT_1=> External Input-1
//   <SL_COMPARATOR_INPUT_FROM_DAC_OUTPUT=> DAC OUTPUT as comparator input
//   <SL_COMPARATOR_INPUT_FROM_REF_BUFFER_OUTPUT=> Reference buffer output as comparator input
//   <SL_COMPARATOR_INPUT_FROM_REF_SCALER_OUTPUT=> Reference scaler output as comparator input
//   <SL_COMPARATOR_INPUT_FROM_RES_BANK_OUTPUT=> Resistor bank output as comparator input
//   <SL_COMPARATOR_INPUT_FROM_OPAMP1_OUTPUT=> OPAMP1 output as comparator input
// <i> Selection of Comparator Inverting input
#define SL_ANALOG_COMPARATOR1_INVERTING_INPUT SL_COMPARATOR_EXTERNAL_GPIO_INPUT_1

#define SL_ANALOG_COMPARATOR1_HYSTERSIS_VALUE SL_COMPARATOR_HYSTERSIS_VALUE_0

#define SL_ANALOG_COMPARATOR1_FILTER_ENABLED 0

// </h>
// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <comp1 signal=COMP_P1,COMP_N1> SL_COMP1
// $[COMP1_SL_COMP1]
#ifndef SL_COMP1_PERIPHERAL
#define SL_COMP1_PERIPHERAL COMP1
#endif

// COMP1 COMP_P1 on ULP_GPIO_5/GPIO_69
#ifndef SL_COMP1_COMP_P1_PORT
#define SL_COMP1_COMP_P1_PORT ULP
#endif
#ifndef SL_COMP1_COMP_P1_PIN
#define SL_COMP1_COMP_P1_PIN 5
#endif
#ifndef SL_COMP1_P1_LOC
#define SL_COMP1_P1_LOC 2
#endif

// COMP1 COMP_N1 on ULP_GPIO_4/GPIO_68
#ifndef SL_COMP1_COMP_N1_PORT
#define SL_COMP1_COMP_N1_PORT ULP
#endif
#ifndef SL_COMP1_COMP_N1_PIN
#define SL_COMP1_COMP_N1_PIN 4
#endif
#ifndef SL_COMP1_N1_LOC
#define SL_COMP1_N1_LOC 3
#endif
// [COMP1_SL_COMP1]$
// <<< sl:end pin_tool >>>

#if (SL_ANALOG_COMPARATOR1_NON_INVERTING_INPUT == SL_COMPARATOR_EXTERNAL_GPIO_INPUT_1)
#define SL_ANALOG_COMPARATOR1_NON_INVERTING_PIN_INPUT SL_ANALOG_COMPARATOR1_PIN_INPUT
#endif
#if (SL_ANALOG_COMPARATOR1_INVERTING_INPUT == SL_COMPARATOR_EXTERNAL_GPIO_INPUT_1)
#define SL_ANALOG_COMPARATOR1_INVERTING_PIN_INPUT SL_ANALOG_COMPARATOR1_PIN_INPUT
#endif

#endif // SL_SI91X_ANALOG_COMPARATOR_COMPARATOR1_CONFIG_H