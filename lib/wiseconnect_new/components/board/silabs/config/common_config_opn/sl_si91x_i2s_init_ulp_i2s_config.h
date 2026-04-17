/***************************************************************************/ /**
 * @file sl_si91x_i2s_init_ulp_i2s_config.h
 * @brief ULP_I2S driver configuration file.
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
 *****************************************************************************/

#ifndef SL_SI91X_I2S_ULP_I2S_CONFIG_H
#define SL_SI91X_I2S_ULP_I2S_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_i2s.h"
/******************************************************************************/
/******************************* I2S1(ULP_I2S) Configuration
 * **************************/
//  <e>ULP I2S UC Configuration
//  <i> Enable: Peripheral configuration is taken straight from the
//  configuration set in the universal configuration (UC). <i> Disable: If the
//  application demands it to be modified during runtime, use the
//  sl_si91x_i2s_config_transmit_receive API to modify the peripheral
//  configuration. <i> Default: 0
#define ULP_I2S_UC 0

// <o SL_ULP_I2S_RESOLUTION> ULP_I2S Resolution
//   <16=> 16 bit
//   <24=> 24 bit
//   <32=> 32 bit
// <i> Default: 16
#define SL_ULP_I2S_RESOLUTION 16

// <o SL_ULP_I2S_SAMPLING_RATE> ULP_I2S Sampling Rate
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
#define SL_ULP_I2S_SAMPLING_RATE SL_I2S_SAMPLING_RATE_8000

// </e>
#if USER_CONFIGURATION_ENABLE
// <<< sl:start pin_tool >>>
// <i2s_ulp signal=(SCLK),(WSCLK),DOUT0,DIN0> SL_ULP_I2S
// $[I2S_ULP_SL_ULP_I2S]
#ifndef SL_ULP_I2S_PERIPHERAL
#define SL_ULP_I2S_PERIPHERAL ULP_I2S
#endif

// ULP_I2S SCLK on ULP_GPIO_7/GPIO_71
#ifndef SL_ULP_I2S_SCLK_PORT
#define SL_ULP_I2S_SCLK_PORT ULP
#endif
#ifndef SL_ULP_I2S_SCLK_PIN
#define SL_ULP_I2S_SCLK_PIN 7
#endif
#ifndef SL_ULP_I2S_SCLK_LOC
#define SL_ULP_I2S_SCLK_LOC 0
#endif

// ULP_I2S WSCLK on ULP_GPIO_10/GPIO_74
#ifndef SL_ULP_I2S_WSCLK_PORT
#define SL_ULP_I2S_WSCLK_PORT ULP
#endif
#ifndef SL_ULP_I2S_WSCLK_PIN
#define SL_ULP_I2S_WSCLK_PIN 10
#endif
#ifndef SL_ULP_I2S_WSCLK_LOC
#define SL_ULP_I2S_WSCLK_LOC 3
#endif

// ULP_I2S DOUT0 on ULP_GPIO_1/GPIO_65
#ifndef SL_ULP_I2S_DOUT0_PORT
#define SL_ULP_I2S_DOUT0_PORT ULP
#endif
#ifndef SL_ULP_I2S_DOUT0_PIN
#define SL_ULP_I2S_DOUT0_PIN 1
#endif
#ifndef SL_ULP_I2S_DOUT0_LOC
#define SL_ULP_I2S_DOUT0_LOC 4
#endif

// ULP_I2S DIN0 on ULP_GPIO_6/GPIO_70
#ifndef SL_ULP_I2S_DIN0_PORT
#define SL_ULP_I2S_DIN0_PORT ULP
#endif
#ifndef SL_ULP_I2S_DIN0_PIN
#define SL_ULP_I2S_DIN0_PIN 6
#endif
#ifndef SL_ULP_I2S_DIN0_LOC
#define SL_ULP_I2S_DIN0_LOC 8
#endif
// [I2S_ULP_SL_ULP_I2S]$
// <<< sl:end pin_tool >>>
#else

#warning \
  "ULP I2S peripheral pins are not configured. To configure, either install [ENABLE USER CONFIGURATION] component or define USER_CONFIGURATION_ENABLE macro to 1, then configure the pins as per the Custom board."

#endif // USER_CONFIGURATION_ENABLE
#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_I2S_ULP_I2S_CONFIG_H