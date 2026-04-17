/***************************************************************************/ /**
* @file sl_si91x_ssi_config.h
* * @brief SSI Master/Slave API configuration
* *******************************************************************************
* * # License
* * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SI91X_SSI_CONFIG_H
#define SL_SI91X_SSI_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// <<< Use Configuration Wizard in Context Menu >>>
//<h> SSI Configuration

//  <e>SSI UC Configuration
//  <i> Enable: Peripheral configuration is taken straight from the configuration set in the universal configuration (UC).
//  <i> Disable: If the application demands it to be modified during runtime, use the sl_si91x_ssi_set_configuration API to modify the peripheral configuration.
//  <i> Default: 1
#define SSI_UC 1

// <o SL_SSI_CLOCK_MODE> Frame Format
//   <SL_SSI_PERIPHERAL_CPOL0_CPHA0=> Mode 0
//   <SL_SSI_PERIPHERAL_CPOL0_CPHA1=> Mode 1
//   <SL_SSI_PERIPHERAL_CPOL1_CPHA0=> Mode 2
//   <SL_SSI_PERIPHERAL_CPOL1_CPHA1=> Mode 3
//   <SL_SSI_PERIPHERAL_TI_SSI=> Texas Instrument
//   <SL_SSI_PERIPHERAL_MICROWIRE=> National Semiconductor Microwire
// <i> Selection of the SSI Master Mode.
#define SL_SSI_CLOCK_MODE SL_SSI_PERIPHERAL_CPOL0_CPHA0

// <o SL_SSI_PRIMARY_TRANSFER_MODE> Transfer Mode
//   <SPI_TRANSFER_MODE_STANDARD=> Single Line Mode
//   <SPI_TRANSFER_MODE_DUAL=> Dual Line Mode
//   <SPI_TRANSFER_MODE_QUAD=> Quad Line Mode
// <i> Selection of the SSI Master Tranfer Mode.
#define SL_SSI_PRIMARY_TRANSFER_MODE SL_SSI_PRIMARY_SINGLE_LINE_MODE

// <o SL_SPI_BAUD> Bit Rate (Bits/Second) <500000-40000000>
// <i> Default: 10000000
#define SL_SPI_BAUD 10000000 // speed at which data transmitted through SPI ; Max is 10000000

// <o SL_SSI_BIT_WIDTH> Data Width <4-16>
// <i> Default: 8
#define SL_SSI_BIT_WIDTH 8

// <o SL_SSI_DEVICE_MODE> Mode
//   <SL_SSI_PRIMARY_ACTIVE=> Master
//   <SL_SSI_SECONDARY_ACTIVE=> Slave
//   <SL_SSI_ULP_PRIMARY_ACTIVE=> ULP Master
// <i> Selection of SSI Device Mode.
#define SL_SSI_DEVICE_MODE SL_SSI_PRIMARY_ACTIVE

// <o SL_SSI_RECEIVE_SAMPLE_DELAY> Rx Sample Delay <0-63>
// <i> Default: 0
#define SL_SSI_RECEIVE_SAMPLE_DELAY 0

// </e>

// </h>
// <<< end of configuration section >>>

#if defined(SL_SSI_CLOCK_MODE)
sl_ssi_control_config_t ssi_configuration = { .bit_width            = SL_SSI_BIT_WIDTH,
                                              .device_mode          = SL_SSI_DEVICE_MODE,
                                              .clock_mode           = SL_SSI_CLOCK_MODE,
                                              .receive_sample_delay = SL_SSI_RECEIVE_SAMPLE_DELAY,
                                              .baud_rate            = SL_SPI_BAUD,
                                              .transfer_mode        = SL_SSI_PRIMARY_TRANSFER_MODE };
#endif

#ifdef __cplusplus
}
#endif
#endif /* SL_SI91X_SSI_CONFIG_H */
