/***************************************************************************/
/**
 * @file  sl_si91x_ssi_primary_config.h
 * @brief SL SI91X Primary Config.
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

#ifndef SL_SI91X_SSI_PRIMARY_CONFIG_H
#define SL_SI91X_SSI_PRIMARY_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_ssi.h"
// <<< Use Configuration Wizard in Context Menu >>>

// <e> SSI Primary(Master) Configuration
// <i> Default: 1
#define SSI_PRIMARY_UC 1

// <o SL_SSI_PRIMARY_CLOCK_MODE> Frame Format
//   <SL_SSI_PERIPHERAL_CPOL0_CPHA0=> Mode 0
//   <SL_SSI_PERIPHERAL_CPOL0_CPHA1=> Mode 1
//   <SL_SSI_PERIPHERAL_CPOL1_CPHA0=> Mode 2
//   <SL_SSI_PERIPHERAL_CPOL1_CPHA1=> Mode 3
//   <SL_SSI_PERIPHERAL_TI_SSI=> Texas Instrument
//   <SL_SSI_PERIPHERAL_MICROWIRE=> National Semiconductor Microwire
// <i> Selection of the SSI Master Mode.
#define SL_SSI_PRIMARY_CLOCK_MODE SL_SSI_PERIPHERAL_CPOL0_CPHA0

// <o SL_SSI_PRIMARY_TRANSFER_MODE> Transfer Mode
//   <SPI_TRANSFER_MODE_STANDARD=> Single Line Mode
//   <SPI_TRANSFER_MODE_DUAL=> Dual Line Mode
//   <SPI_TRANSFER_MODE_QUAD=> Quad Line Mode
// <i> Selection of the SSI Master Tranfer Mode.
#define SL_SSI_PRIMARY_TRANSFER_MODE SPI_TRANSFER_MODE_STANDARD

// <o SL_SSI_PRIMARY_BAUD> Bit Rate (Bits/Second) <500000-40000000>
// <i> Default: 10000000
#define SL_SSI_PRIMARY_BAUD 10000000 // SPI Speed ; Max is 40000000

// <o SL_SSI_PRIMARY_BIT_WIDTH> Data Width <4-16>
// <i> Default: 8
#define SL_SSI_PRIMARY_BIT_WIDTH 8

#define SL_SSI_PRIMARY_DEVICE_MODE SL_SSI_PRIMARY_ACTIVE

// <o SL_SSI_PRIMARY_RECEIVE_SAMPLE_DELAY> Rx Sample Delay <0-63>
// <i> Default: 0
#define SL_SSI_PRIMARY_RECEIVE_SAMPLE_DELAY 0

// </e>
// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <ssi_master signal=MOSI_DATA0,MISO_DATA1,(DATA2),(DATA3),SCK_,(CS0_),(CS1_),(CS2_),(CS3_)> SL_SSI_MASTER
// $[SSI_MASTER_SL_SSI_MASTER]
#ifndef SL_SSI_MASTER_PERIPHERAL
#define SL_SSI_MASTER_PERIPHERAL SSI_MASTER
#endif

// SSI_MASTER MOSI_DATA0 on GPIO_26
#ifndef SL_SSI_MASTER_MOSI_DATA0_PORT
#define SL_SSI_MASTER_MOSI_DATA0_PORT HP
#endif
#ifndef SL_SSI_MASTER_MOSI_DATA0_PIN
#define SL_SSI_MASTER_MOSI_DATA0_PIN 26
#endif
#ifndef SL_SSI_MASTER_DATA0_LOC
#define SL_SSI_MASTER_DATA0_LOC 1
#endif

// SSI_MASTER MISO_DATA1 on GPIO_27
#ifndef SL_SSI_MASTER_MISO_DATA1_PORT
#define SL_SSI_MASTER_MISO_DATA1_PORT HP
#endif
#ifndef SL_SSI_MASTER_MISO_DATA1_PIN
#define SL_SSI_MASTER_MISO_DATA1_PIN 27
#endif
#ifndef SL_SSI_MASTER_DATA1_LOC
#define SL_SSI_MASTER_DATA1_LOC 4
#endif

// SSI_MASTER SCK_ on GPIO_25
#ifndef SL_SSI_MASTER_SCK__PORT
#define SL_SSI_MASTER_SCK__PORT HP
#endif
#ifndef SL_SSI_MASTER_SCK__PIN
#define SL_SSI_MASTER_SCK__PIN 25
#endif
#ifndef SL_SSI_MASTER_SCK_LOC
#define SL_SSI_MASTER_SCK_LOC 7
#endif

// SSI_MASTER CS0_ on GPIO_28
#ifndef SL_SSI_MASTER_CS0__PORT
#define SL_SSI_MASTER_CS0__PORT HP
#endif
#ifndef SL_SSI_MASTER_CS0__PIN
#define SL_SSI_MASTER_CS0__PIN 28
#endif
#ifndef SL_SSI_MASTER_CS0_LOC
#define SL_SSI_MASTER_CS0_LOC 10
#endif
// [SSI_MASTER_SL_SSI_MASTER]$
// <<< sl:end pin_tool >>>

#if (defined(SL_SSI_PRIMARY_TRANSFER_MODE) && (SL_SSI_PRIMARY_TRANSFER_MODE == SPI_TRANSFER_MODE_QUAD))
#if (SPI_QUAD_MODE != 1)
#warning \
  "SSI Quad Line Mode is configured, please add Preprocessor symbol SPI_QUAD_MODE with Value 1 in Project properties."
#endif
#if !(defined(SL_SSI_MASTER_DATA2_PORT) && defined(SL_SSI_MASTER_DATA3_PORT))
#warning "DATA2 or DATA3 pin is not configured while SSI Primary Transfer Mode is Quad Line Mode."
#endif
#endif

#if (defined(SL_SSI_PRIMARY_TRANSFER_MODE)                        \
     && ((SL_SSI_PRIMARY_TRANSFER_MODE == SPI_TRANSFER_MODE_DUAL) \
         || (SL_SSI_PRIMARY_TRANSFER_MODE == SPI_TRANSFER_MODE_STANDARD)))
#if (defined(SL_SSI_MASTER_DATA2_PORT) || defined(SL_SSI_MASTER_DATA3_PORT))
#warning "DATA2 or DATA3 pin is configured while SSI Primary Transfer Mode is not Quad Line Mode."
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
