/***************************************************************************/ /**
* @file sl_si91x_i2s_init_inst_config.h
* @brief I2S driver configuration file.
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

#ifndef SL_SI91X_I2S_INSTANCE_CONFIG_H
#define SL_SI91X_I2S_INSTANCE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* NOTE: User should configure all macros defined below, while creating an
 * instance other than pre-defined one */
#warning Invalid I2S instance present. Create I2S instance for high power i2s or ulp_i2s for low power I2S

#if USER_CONFIGURATION_ENABLE
#include "sl_si91x_i2s.h"
/******************************************************************************/
/******************************* I2S Configuration **************************/
// <<< Use Configuration Wizard in Context Menu >>>

//  <e>INSTANCE UC Configuration
//  <i> Enable: Peripheral configuration is taken straight from the configuration set in the universal configuration (UC).
//  <i> Disable: If the application demands it to be modified during runtime, use the sl_si91x_i2s_config_transmit_receive API to modify the peripheral configuration.
//  <i> Default: 0
#define INSTANCE_UC 0

// <o SL_INSTANCE_RESOLUTION> INSTANCE Resolution
//   <16=> 16 bit
//   <24=> 24 bit
//   <32=> 32 bit
// <i> Default: 16
#define SL_INSTANCE_RESOLUTION 16

// <o SL_INSTANCE_CHANNEL> INSTANCE Channel
//   <0=> Channel 0
//   <1=> Channel 1
// <i> Default: 0
#define SL_INSTANCE_CHANNEL 0

// <o SL_INSTANCE_SAMPLING_RATE> INSTANCE Sampling Rate
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
#define SL_INSTANCE_SAMPLING_RATE SL_I2S_SAMPLING_RATE_8000

// </e>
#else

#warning \
  "The sl_si91x_i2s_init_INSTANCE_config.h file requires specifying the INSTANCE name for custom I2S configurations. To properly configure I2S either enable user configuration by installing the [ENABLE USER CONFIGURATION] component or set USER_CONFIGURATION_ENABLE to 1, and then define all required MACROS in sl_si91x_i2s_init_INSTANCE_config.h according to your board specifications."

#endif // USER_CONFIGURATION_ENABLE
#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_I2S_INIT_INST_CONFIG_H