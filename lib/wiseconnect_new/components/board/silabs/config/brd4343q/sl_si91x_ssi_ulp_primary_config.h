/***************************************************************************/
/**
 * @file  sl_si91x_ssi_ulp_primary_config.h
 * @brief SL SI91X Ulp_Primary Config.
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

#ifndef SL_SI91X_SSI_ULP_PRIMARY_CONFIG_H
#define SL_SI91X_SSI_ULP_PRIMARY_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_ssi.h"
// <<< Use Configuration Wizard in Context Menu >>>

// <e> SSI ULP Master Configuration
// <i> Default: 1
#define SSI_ULP_PRIMARY_UC 1

// <o SL_SSI_ULP_PRIMARY_CLOCK_MODE> Frame Format
//   <SL_SSI_PERIPHERAL_CPOL0_CPHA0=> Mode 0
//   <SL_SSI_PERIPHERAL_CPOL0_CPHA1=> Mode 1
//   <SL_SSI_PERIPHERAL_CPOL1_CPHA0=> Mode 2
//   <SL_SSI_PERIPHERAL_CPOL1_CPHA1=> Mode 3
//   <SL_SSI_PERIPHERAL_TI_SSI=> Texas Instrument
//   <SL_SSI_PERIPHERAL_MICROWIRE=> National Semiconductor Microwire
// <i> Selection of the SSI Master Mode.
#define SL_SSI_ULP_PRIMARY_CLOCK_MODE SL_SSI_PERIPHERAL_CPOL0_CPHA0

// <o SL_SSI_ULP_PRIMARY_BAUD> Bit Rate (Bits/Second) <500000-5000000>
// <i> Default: 5000000
#define SL_SSI_ULP_PRIMARY_BAUD 5000000 // SSI Speed ; Max is 5000000

#define SL_SSI_ULP_PRIMARY_TRANSFER_MODE 0 // SSI ULP Primary is not NA

// <o SL_SSI_ULP_PRIMARY_BIT_WIDTH> Data Width <4-16>
// <i> Default: 8
#define SL_SSI_ULP_PRIMARY_BIT_WIDTH 8

#define SL_SSI_ULP_PRIMARY_DEVICE_MODE SL_SSI_ULP_PRIMARY_ACTIVE

// <o SL_SSI_ULP_PRIMARY_RECEIVE_SAMPLE_DELAY> Rx Sample Delay <0-63>
// <i> Default: 0
#define SL_SSI_ULP_PRIMARY_RECEIVE_SAMPLE_DELAY 0

// </e>
// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <ulp_ssi signal=MOSI_,MISO_,SCK_,(CS0_),(CS1_),(CS2_)> SL_ULP_SSI
// $[ULP_SSI_SL_ULP_SSI]

#ifndef SL_ULP_SSI_PERIPHERAL
#define SL_ULP_SSI_PERIPHERAL ULP_SSI
#endif

// ULP_SSI MOSI_ on ULP_GPIO_1/GPIO_65
#ifndef SL_ULP_SSI_MOSI__PORT
#define SL_ULP_SSI_MOSI__PORT ULP
#endif
#ifndef SL_ULP_SSI_MOSI__PIN
#define SL_ULP_SSI_MOSI__PIN 1
#endif
#ifndef SL_ULP_SSI_MOSI_LOC
#define SL_ULP_SSI_MOSI_LOC 0
#endif

// ULP_SSI MISO_ on ULP_GPIO_2/GPIO_66
#ifndef SL_ULP_SSI_MISO__PORT
#define SL_ULP_SSI_MISO__PORT ULP
#endif
#ifndef SL_ULP_SSI_MISO__PIN
#define SL_ULP_SSI_MISO__PIN 2
#endif
#ifndef SL_ULP_SSI_MISO_LOC
#define SL_ULP_SSI_MISO_LOC 12
#endif

// ULP_SSI SCK_ on ULP_GPIO_8/GPIO_72
#ifndef SL_ULP_SSI_SCK__PORT
#define SL_ULP_SSI_SCK__PORT ULP
#endif
#ifndef SL_ULP_SSI_SCK__PIN
#define SL_ULP_SSI_SCK__PIN 8
#endif
#ifndef SL_ULP_SSI_SCK_LOC
#define SL_ULP_SSI_SCK_LOC 7
#endif

// ULP_SSI CS1_ on ULP_GPIO_4/GPIO_68
#ifndef SL_ULP_SSI_CS1__PORT
#define SL_ULP_SSI_CS1__PORT ULP
#endif
#ifndef SL_ULP_SSI_CS1__PIN
#define SL_ULP_SSI_CS1__PIN 4
#endif
#ifndef SL_ULP_SSI_CS1_LOC
#define SL_ULP_SSI_CS1_LOC 10
#endif

// [ULP_SSI_SL_ULP_SSI]$
// <<< sl:end pin_tool >>>

#ifdef __cplusplus
}
#endif

#endif
