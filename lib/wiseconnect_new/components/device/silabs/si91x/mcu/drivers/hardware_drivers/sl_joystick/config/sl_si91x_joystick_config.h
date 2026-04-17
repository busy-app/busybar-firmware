/***************************************************************************/ /**
 * @file sl_si91x_joystick_config.h
 * @brief Joystick Driver User Config
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_JOYSTICK_CONFIG_H
#define SL_JOYSTICK_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <e>Channel selection

// <o JOYSTICK_ADC_CHANNEL> Joystick ADC channel (make sure to install selected ADC channel/instance)
//   <SL_ADC_CHANNEL_1=>  channel_1
//   <SL_ADC_CHANNEL_2=>  channel_2
//   <SL_ADC_CHANNEL_3=>  channel_3
//   <SL_ADC_CHANNEL_4=>  channel_4
//   <SL_ADC_CHANNEL_5=>  channel_5
//   <SL_ADC_CHANNEL_6=>  channel_6
//   <SL_ADC_CHANNEL_7=>  channel_7
//   <SL_ADC_CHANNEL_8=>  channel_8
//   <SL_ADC_CHANNEL_9=>  channel_9
//   <SL_ADC_CHANNEL_10=>  channel_10
//   <SL_ADC_CHANNEL_11=>  channel_11
//   <SL_ADC_CHANNEL_12=>  channel_12
//   <SL_ADC_CHANNEL_13=>  channel_13
//   <SL_ADC_CHANNEL_14=>  channel_14
//   <SL_ADC_CHANNEL_15=>  channel_15
//   <SL_ADC_CHANNEL_16=>  channel_16

#define JOYSTICK_ADC_CHANNEL SL_ADC_CHANNEL_1

//</e>

// <h>Joystick Voltage value Configuration

// <o REFERENCE_VOLTAGE> Reference voltage <f.d>
// <i> Vref magnitude expressed in millivolts. As per Joystick Hardware on Wireless Pro Kit, Vref = AVDD = 3300 mV
// <i> Default: 3300
#define REFERENCE_VOLTAGE 3300

// <o JOYSTICK_MV_C> Center position(mV) <f.d>
// <i> Default: 30
#define JOYSTICK_MV_C 30

// <o JOYSTICK_MV_N> North position(mV) <f.d>
// <i> Default: 2850
#define JOYSTICK_MV_N 2850

// <o JOYSTICK_MV_S> South position(mV) <f.d>
// <i> Default: 1850
#define JOYSTICK_MV_S 1850

// <o JOYSTICK_MV_E> East position(mV) <f.d>
// <i> Default: 2650
#define JOYSTICK_MV_E 2650

// <o JOYSTICK_MV_W> West position(mV) <f.d>
// <i> Default: 2150
#define JOYSTICK_MV_W 2150

// <o JOYSTICK_MV_ERR_CARDINAL_ONLY> Joystick error mV value for Cardinal Directions only <f.d>
// <i> This value will not be used when joystick is enabled for secondary directions
// <i> Default: 150
#define JOYSTICK_MV_ERR_CARDINAL_ONLY 150

// </h> end Joystick Voltage value Configuration

#define SL_JOYSTICK_ADC_CHANNEL JOYSTICK_ADC_CHANNEL

// <<< end of configuration section >>>

#endif // SL_JOYSTICK_CONFIG_H
