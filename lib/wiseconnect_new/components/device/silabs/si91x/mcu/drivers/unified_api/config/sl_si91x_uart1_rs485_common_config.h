/***************************************************************************/ /**
 * @file sl_si91x_uart1_rs485_common_config.h
 * @brief SL SI91X UART1 RS485 Common Config.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SI91X_UART1_RS485_COMMON_CONFIG_H
#define SL_SI91X_UART1_RS485_COMMON_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
// <h>DMA Configuration

// <q SL_UART1_DMA_CONFIG_ENABLE> DMA
// <i> Default: 1
#define SL_UART1_DMA_CONFIG_ENABLE 1
// </h>
// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <uart1 signal=TX,RX,(RS485_RE),(RS485_DE),(RS485_EN)> SL_UART1
// $[UART1_SL_UART1]
#ifndef SL_UART1_PERIPHERAL
#define SL_UART1_PERIPHERAL UART1
#endif
#ifndef SL_UART1_PERIPHERAL_NO
#define SL_UART1_PERIPHERAL_NO 1
#endif

// UART1 TX on GPIO_7
#ifndef SL_UART1_TX_PORT
#define SL_UART1_TX_PORT HP
#endif
#ifndef SL_UART1_TX_PIN
#define SL_UART1_TX_PIN 7
#endif
#ifndef SL_UART1_TX_LOC
#define SL_UART1_TX_LOC 0
#endif

// UART1 RX on GPIO_6
#ifndef SL_UART1_RX_PORT
#define SL_UART1_RX_PORT HP
#endif
#ifndef SL_UART1_RX_PIN
#define SL_UART1_RX_PIN 6
#endif
#ifndef SL_UART1_RX_LOC
#define SL_UART1_RX_LOC 5
#endif

// UART1 RS485_RE on GPIO_8
#ifndef SL_UART1_RS485_RE_PORT
#define SL_UART1_RS485_RE_PORT HP
#endif
#ifndef SL_UART1_RS485_RE_PIN
#define SL_UART1_RS485_RE_PIN 8
#endif
#ifndef SL_UART1_RE_LOC
#define SL_UART1_RE_LOC 26
#endif

// UART1 RS485_DE on GPIO_9
#ifndef SL_UART1_RS485_DE_PORT
#define SL_UART1_RS485_DE_PORT HP
#endif
#ifndef SL_UART1_RS485_DE_PIN
#define SL_UART1_RS485_DE_PIN 9
#endif
#ifndef SL_UART1_DE_LOC
#define SL_UART1_DE_LOC 29
#endif

// UART1 RS485_EN on GPIO_12
#ifndef SL_UART1_RS485_EN_PORT
#define SL_UART1_RS485_EN_PORT HP
#endif
#ifndef SL_UART1_RS485_EN_PIN
#define SL_UART1_RS485_EN_PIN 12
#endif
#ifndef SL_UART1_EN_LOC
#define SL_UART1_EN_LOC 31
#endif
// [UART1_SL_UART1]$
// <<< sl:end pin_tool >>>

#endif // SL_SI91X_UART1_RS485_COMMON_CONFIG_H