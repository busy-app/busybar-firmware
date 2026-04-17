/***************************************************************************/
/**
 * @file  sl_si91x_ssi_secondary_config.h
 * @brief SL SI91X Secondary Config.
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

#ifndef SL_SI91X_SSI_SECONDARY_CONFIG_H
#define SL_SI91X_SSI_SECONDARY_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_ssi.h"
// <<< Use Configuration Wizard in Context Menu >>>

// <e> SSI Slave Configuration
// <i> Default: 1
#define SSI_SECONDARY_UC 1

// <o SL_SSI_SECONDARY_CLOCK_MODE> Frame Format
//   <SL_SSI_PERIPHERAL_CPOL0_CPHA0=> Mode 0
//   <SL_SSI_PERIPHERAL_CPOL0_CPHA1=> Mode 1
//   <SL_SSI_PERIPHERAL_CPOL1_CPHA0=> Mode 2
//   <SL_SSI_PERIPHERAL_CPOL1_CPHA1=> Mode 3
//   <SL_SSI_PERIPHERAL_TI_SSI=> Texas Instrument
//   <SL_SSI_PERIPHERAL_MICROWIRE=> National Semiconductor Microwire
// <i> Selection of the SSI Master Mode.
#define SL_SSI_SECONDARY_CLOCK_MODE SL_SSI_PERIPHERAL_CPOL0_CPHA0

#define SL_SSI_SECONDARY_BAUD 10000000 // SPI Speed is 0

#define SL_SSI_SECONDARY_TRANSFER_MODE 0 // SPI Secondary transfer mode is NA

// <o SL_SSI_SECONDARY_BIT_WIDTH> Data Width <4-16>
// <i> Default: 8
#define SL_SSI_SECONDARY_BIT_WIDTH 8

#define SL_SSI_SECONDARY_DEVICE_MODE SL_SSI_SECONDARY_ACTIVE

// <o SL_SSI_SECONDARY_RECEIVE_SAMPLE_DELAY> Rx Sample Delay <0-63>
// <i> Default: 0
#define SL_SSI_SECONDARY_RECEIVE_SAMPLE_DELAY 0

//</e>
// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <ssi_slave signal=MOSI_,MISO_,SCK_,CS0_> SL_SSI_SLAVE
// $[SSI_SLAVE_SL_SSI_SLAVE]

#ifndef SL_SSI_SLAVE_PERIPHERAL
#define SL_SSI_SLAVE_PERIPHERAL SSI_SLAVE
#endif

// SSI_SLAVE MOSI_ on GPIO_27
#ifndef SL_SSI_SLAVE_MOSI__PORT
#define SL_SSI_SLAVE_MOSI__PORT HP
#endif
#ifndef SL_SSI_SLAVE_MOSI__PIN
#define SL_SSI_SLAVE_MOSI__PIN 27
#endif
#ifndef SL_SSI_SLAVE_MOSI_LOC
#define SL_SSI_SLAVE_MOSI_LOC 2
#endif

// SSI_SLAVE MISO_ on GPIO_28
#ifndef SL_SSI_SLAVE_MISO__PORT
#define SL_SSI_SLAVE_MISO__PORT HP
#endif
#ifndef SL_SSI_SLAVE_MISO__PIN
#define SL_SSI_SLAVE_MISO__PIN 28
#endif
#ifndef SL_SSI_SLAVE_MISO_LOC
#define SL_SSI_SLAVE_MISO_LOC 6
#endif

// SSI_SLAVE SCK_ on GPIO_26
#ifndef SL_SSI_SLAVE_SCK__PORT
#define SL_SSI_SLAVE_SCK__PORT HP
#endif
#ifndef SL_SSI_SLAVE_SCK__PIN
#define SL_SSI_SLAVE_SCK__PIN 26
#endif
#ifndef SL_SSI_SLAVE_SCK_LOC
#define SL_SSI_SLAVE_SCK_LOC 10
#endif

// SSI_SLAVE CS0_ on GPIO_25
#ifndef SL_SSI_SLAVE_CS0__PORT
#define SL_SSI_SLAVE_CS0__PORT HP
#endif
#ifndef SL_SSI_SLAVE_CS0__PIN
#define SL_SSI_SLAVE_CS0__PIN 25
#endif
#ifndef SL_SSI_SLAVE_CS0_LOC
#define SL_SSI_SLAVE_CS0_LOC 14
#endif
// [SSI_SLAVE_SL_SSI_SLAVE]$
// <<< sl:end pin_tool >>>

#ifdef __cplusplus
}
#endif

#endif
