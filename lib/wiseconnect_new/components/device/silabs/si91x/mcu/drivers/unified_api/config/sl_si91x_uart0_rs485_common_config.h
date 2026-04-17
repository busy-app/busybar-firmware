/***************************************************************************/ /**
 * @file sl_si91x_uart0_rs485_common_config.h
 * @brief SL SI91X UART0 RS485 Common Config.
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

#ifndef SL_SI91X_UART0_RS485_COMMON_CONFIG_H
#define SL_SI91X_UART0_RS485_COMMON_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
// <h>DMA Configuration

// <q SL_USART0_DMA_CONFIG_ENABLE> DMA
// <i> Default: 1
#define SL_USART0_DMA_CONFIG_ENABLE 1
// </h>
// <<< end of configuration section >>>
// <<< sl:start pin_tool >>>
// <usart0 signal=TX,RX,(RS485_RE),(RS485_DE),(RS485_EN)> SL_USART0
// $[USART0_SL_USART0]
#ifndef SL_USART0_PERIPHERAL
#define SL_USART0_PERIPHERAL USART0
#endif

// USART0 TX on GPIO_30
#ifndef SL_USART0_TX_PORT
#define SL_USART0_TX_PORT HP
#endif
#ifndef SL_USART0_TX_PIN
#define SL_USART0_TX_PIN 30
#endif
#ifndef SL_USART0_TX_LOC
#define SL_USART0_TX_LOC 5
#endif

// USART0 RX on GPIO_29
#ifndef SL_USART0_RX_PORT
#define SL_USART0_RX_PORT HP
#endif
#ifndef SL_USART0_RX_PIN
#define SL_USART0_RX_PIN 29
#endif
#ifndef SL_USART0_RX_LOC
#define SL_USART0_RX_LOC 10
#endif

// USART0 RS485_RE on GPIO_28
#ifndef SL_USART0_RS485_RE_PORT
#define SL_USART0_RS485_RE_PORT HP
#endif
#ifndef SL_USART0_RS485_RE_PIN
#define SL_USART0_RS485_RE_PIN 28
#endif
#ifndef SL_USART0_RE_LOC
#define SL_USART0_RE_LOC 38
#endif

// USART0 RS485_DE on GPIO_51
#ifndef SL_USART0_RS485_DE_PORT
#define SL_USART0_RS485_DE_PORT HP
#endif
#ifndef SL_USART0_RS485_DE_PIN
#define SL_USART0_RS485_DE_PIN 51
#endif
#ifndef SL_USART0_DE_LOC
#define SL_USART0_DE_LOC 43
#endif

// USART0 RS485_EN on GPIO_27
#ifndef SL_USART0_RS485_EN_PORT
#define SL_USART0_RS485_EN_PORT HP
#endif
#ifndef SL_USART0_RS485_EN_PIN
#define SL_USART0_RS485_EN_PIN 27
#endif
#ifndef SL_USART0_EN_LOC
#define SL_USART0_EN_LOC 46
#endif
// [USART0_SL_USART0]$
// <<< sl:end pin_tool >>>
#endif // SL_SI91X_UART0_RS485_COMMON_CONFIG_H