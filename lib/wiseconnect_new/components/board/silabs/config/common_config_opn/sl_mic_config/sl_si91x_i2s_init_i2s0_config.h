/***************************************************************************/ /**
* @file sl_si91x_i2s_i2s0_config.h
* @brief I2S driver configuration file.
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

#ifndef SL_SI91X_I2S_I2S0_CONFIG_H
#define SL_SI91X_I2S_I2S0_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_i2s.h"
/******************************************************************************/
/******************************* I2S Configuration **************************/
// <<< Use Configuration Wizard in Context Menu >>>

//  <e>I2S0 UC Configuration
//  <i> Enable: Peripheral configuration is taken straight from the configuration set in the universal configuration (UC).
//  <i> Disable: If the application demands it to be modified during runtime, use the sl_si91x_i2s_config_transmit_receive API to modify the peripheral configuration.
//  <i> Default: 0
#define I2S0_UC 0

// <o SL_I2S0_RESOLUTION> I2S0 Resolution
//   <16=> 16 bit
//   <24=> 24 bit
//   <32=> 32 bit
// <i> Default: 16
#define SL_I2S0_RESOLUTION 16

// <o SL_I2S0_CHANNEL> I2S0 Channel
//   <0=> Channel 0
//   <1=> Channel 1
// <i> Default: 0
#define SL_I2S0_CHANNEL 0

// <o SL_I2S0_SAMPLING_RATE> I2S0 Sampling Rate
//   <SL_I2S_SAMPLING_RATE_8000=>   8kHz
//   <SL_I2S_SAMPLING_RATE_11025=>  11.025kHz
//   <SL_I2S_SAMPLING_RATE_16000=>  16kHz
//   <SL_I2S_SAMPLING_RATE_22050=>  22.05kHz
//   <SL_I2S_SAMPLING_RATE_24000=>  24kHz
//   <SL_I2S_SAMPLING_RATE_32000=>  32kHz
//   <SL_I2S_SAMPLING_RATE_44100=>  44.1kHz
//   <SL_I2S_SAMPLING_RATE_48000=>  48kHz
//   <SL_I2S_SAMPLING_RATE_88200=>  88.2kHz
//   <SL_I2S_SAMPLING_RATE_96000=>  96kHz
//   <SL_I2S_SAMPLING_RATE_192000=> 192kHz
// <i> Default: SL_I2S_SAMPLING_RATE_8000
#define SL_I2S0_SAMPLING_RATE SL_I2S_SAMPLING_RATE_8000

// </e>

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_I2S_I2S0_CONFIG_H