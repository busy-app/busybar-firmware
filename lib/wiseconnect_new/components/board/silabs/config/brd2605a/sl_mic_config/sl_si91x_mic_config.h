/***************************************************************************/ /**
* @file sl_si91x_mic_config.h
* * @brief I2S Mic configuration
* *******************************************************************************
* * # License
* * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
* *******************************************************************************
* *
* * SPDX-License-Identifier: Zlib
* *
* * The licensor of this software is Silicon Laboratories Inc.
* *
* * This software is provided 'as-is', without any express or implied
* * warranty. In no event will the authors be held liable for any damages
* * arising from the use of this software.
* *
* * Permission is granted to anyone to use this software for any purpose,
* * including commercial applications, and to alter it and redistribute it
* * freely, subject to the following restrictions:
* *
* * 1. The origin of this software must not be misrepresented; you must not
* *    claim that you wrote the original software. If you use this software
* *    in a product, an acknowledgment in the product documentation would be
* *    appreciated but is not required.
* * 2. Altered source versions must be plainly marked as such, and must not be
* *    misrepresented as being the original software.
* * 3. This notice may not be removed or altered from any source distribution.
* *
* ******************************************************************************/

#ifndef SL_SI91X_MIC_CONFIG_H
#define SL_SI91X_MIC_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// <<< sl:start pin_tool >>>
// <i2s0 signal=SCLK,WSCLK,DIN0> SL_SI91X_MIC
// $[I2S0_SL_SI91X_MIC]
#ifndef SL_SI91X_MIC_PERIPHERAL
#define SL_SI91X_MIC_PERIPHERAL I2S0
#endif
// I2S0 SCLK on GPIO_46
#ifndef SL_SI91X_MIC_SCLK_PORT
#define SL_SI91X_MIC_SCLK_PORT HP
#endif
#ifndef SL_SI91X_MIC_SCLK_PIN
#define SL_SI91X_MIC_SCLK_PIN 46
#endif
#ifndef SL_SI91X_MIC_SCLK_LOC
#define SL_SI91X_MIC_SCLK_LOC 2
#endif

// I2S0 WSCLK on GPIO_47
#ifndef SL_SI91X_MIC_WSCLK_PORT
#define SL_SI91X_MIC_WSCLK_PORT HP
#endif
#ifndef SL_SI91X_MIC_WSCLK_PIN
#define SL_SI91X_MIC_WSCLK_PIN 47
#endif
#ifndef SL_SI91X_MIC_WSCLK_LOC
#define SL_SI91X_MIC_WSCLK_LOC 6
#endif

// I2S0 DIN0 on GPIO_48
#ifndef SL_SI91X_MIC_DIN0_PORT
#define SL_SI91X_MIC_DIN0_PORT HP
#endif
#ifndef SL_SI91X_MIC_DIN0_PIN
#define SL_SI91X_MIC_DIN0_PIN 48
#endif
#ifndef SL_SI91X_MIC_DIN0_LOC
#define SL_SI91X_MIC_DIN0_LOC 14
#endif
// [I2S0_SL_SI91X_MIC]$
// <<< sl:end pin_tool >>>

// <<< sl:start pin_tool >>>
// <gpio> SL_MIC_ENABLE
// $[GPIO_SL_MIC_ENABLE]
#ifndef SL_MIC_ENABLE_PORT
#define SL_MIC_ENABLE_PORT UULP_VBAT
#endif
#ifndef SL_MIC_ENABLE_PIN
#define SL_MIC_ENABLE_PIN 0
#endif
// [GPIO_SL_MIC_ENABLE]$
// <<< sl:end pin_tool >>>

#define SL_SI91X_MIC_ENABLE_PORT RTE_MIC_ENABLE_PORT
#define SL_SI91X_MIC_ENABLE_PIN  SL_MIC_ENABLE_PIN

// <<< Use Configuration Wizard in Context Menu >>>
// <e>MIC UC Configuration
// <i> Default: 0
#define MIC_UC 0

// <o SL_MIC_DATA_SIZE> MIC Data size
//   <16=> 16 bit
//   <32=> 32 bit
// <i> Default: 16
#define SL_MIC_DATA_SIZE 16

// </e>

#ifdef __cplusplus
}
#endif
#endif /* SL_SI91X_MIC_CONFIG_H */