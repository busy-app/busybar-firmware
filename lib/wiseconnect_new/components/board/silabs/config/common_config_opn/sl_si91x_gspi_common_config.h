/***************************************************************************/ /**
 * @file sl_si91x_gspi_common_config.h
 * @brief SL SI91X Common Config.
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

#ifndef SL_COMMON_CONFIG_H
#define SL_COMMON_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// <<< Use Configuration Wizard in Context Menu >>>
// <h>DMA Configuration

// <e SL_GSPI_DMA_CONFIG_ENABLE> DMA
// <i> Default: 0
#define SL_GSPI_DMA_CONFIG_ENABLE 0

// <o SL_GSPI_AFULL_THRESHOLD> FIFO Threshold - Almost Full <0-14>
// <i> Default: 5
#define SL_GSPI_AFULL_THRESHOLD 5

// <o SL_GSPI_AEMPTY_THRESHOLD> FIFO Threshold - Almost Empty <0-14>
// <i> Default: 5
#define SL_GSPI_AEMPTY_THRESHOLD 5

// </e>

// </h>
// <<< end of configuration section >>>
#if USER_CONFIGURATION_ENABLE
// <<< sl:start pin_tool >>>
// <gspi_master signal=SCK_,CS0_,MOSI_,MISO_> SL_GSPI_MASTER
// $[GSPI_MASTER_SL_GSPI_MASTER]
#ifndef SL_GSPI_MASTER_PERIPHERAL
#define SL_GSPI_MASTER_PERIPHERAL GSPI_MASTER
#endif

// GSPI_MASTER SCK_ on GPIO_25
#ifndef SL_GSPI_MASTER_SCK__PORT
#define SL_GSPI_MASTER_SCK__PORT HP
#endif
#ifndef SL_GSPI_MASTER_SCK__PIN
#define SL_GSPI_MASTER_SCK__PIN 25
#endif
#ifndef SL_GSPI_MASTER_SCK_LOC
#define SL_GSPI_MASTER_SCK_LOC 1
#endif

// GSPI_MASTER CS0_ on GPIO_28
#ifndef SL_GSPI_MASTER_CS0__PORT
#define SL_GSPI_MASTER_CS0__PORT HP
#endif
#ifndef SL_GSPI_MASTER_CS0__PIN
#define SL_GSPI_MASTER_CS0__PIN 28
#endif
#ifndef SL_GSPI_MASTER_CS0_LOC
#define SL_GSPI_MASTER_CS0_LOC 5
#endif

// GSPI_MASTER MOSI_ on GPIO_27
#ifndef SL_GSPI_MASTER_MOSI__PORT
#define SL_GSPI_MASTER_MOSI__PORT HP
#endif
#ifndef SL_GSPI_MASTER_MOSI__PIN
#define SL_GSPI_MASTER_MOSI__PIN 27
#endif
#ifndef SL_GSPI_MASTER_MOSI_LOC
#define SL_GSPI_MASTER_MOSI_LOC 17
#endif

// GSPI_MASTER MISO_ on GPIO_26
#ifndef SL_GSPI_MASTER_MISO__PORT
#define SL_GSPI_MASTER_MISO__PORT HP
#endif
#ifndef SL_GSPI_MASTER_MISO__PIN
#define SL_GSPI_MASTER_MISO__PIN 26
#endif
#ifndef SL_GSPI_MASTER_MISO_LOC
#define SL_GSPI_MASTER_MISO_LOC 22
#endif
// [GSPI_MASTER_SL_GSPI_MASTER]$
// <<< sl:end pin_tool >>>
#else

#warning \
  "GSPI pins are not configured. To configure, either install [ENABLE USER CONFIGURATION] component or define USER_CONFIGURATION_ENABLE macro to 1, then configure the pins as per the Custom board."

#endif // USER_CONFIGURATION_ENABLE
#ifdef __cplusplus
}
#endif

#endif
