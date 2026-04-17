/***************************************************************************/ /**
 * @file sl_si91x_uart_common_config.h
 * @brief SL SI91X UART Common Config.
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

#ifndef SL_SI91X_UART_COMMON_CONFIG_H
#define SL_SI91X_UART_COMMON_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
// <h>UART1 Configuration

// <q SL_UART1_DMA_CONFIG_ENABLE> UART1 DMA
// <i> Default: 1
#define SL_UART1_DMA_CONFIG_ENABLE 1

// <o SL_UART1_CLOCK_SRC> UART1 Clock Source
// <USART_ULPREFCLK=> ULP REF CLK
// <USART_SOCPLLCLK=> SOC PLL CLK
// <USART_INTFPLLCLK=> INTF PLL CLK
// <i> Default: USART_ULPREFCLK
#define SL_UART1_CLOCK_SRC USART_ULPREFCLK

// </h>
// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <uart1 signal=TX,RX,(CTS),(RTS)> SL_UART1
// $[UART1_SL_UART1]
#ifndef SL_UART1_PERIPHERAL
#define SL_UART1_PERIPHERAL UART1
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

// [UART1_SL_UART1]$
// <<< sl:end pin_tool >>>

#endif // SL_SI91X_UART_COMMON_CONFIG_H
