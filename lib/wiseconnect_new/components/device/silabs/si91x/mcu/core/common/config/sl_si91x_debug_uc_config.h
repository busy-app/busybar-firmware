/***************************************************************************/ /**
 * @file sl_si91x_debug_uc_config.h
 * @brief Debug configuration file.
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

#ifndef SL_SI91X_DEBUG_UC_CONFIG_H
#define SL_SI91X_DEBUG_UC_CONFIG_H

/******************************* Debug UART Configuration **************************/
// <<< Use Configuration Wizard in Context Menu >>>
// <h> Debug Configuration

//  <e>Enable/Disable Debug Configuration
//  <i> Enable: Enables the Debug logging
//  <i> Disable: Disables the Debug logging
#define DEBUG_UART_ENABLE 1
#if (DEBUG_UART_ENABLE == 1)
#define DEBUG_UART 1
#else
#undef DEBUG_UART
#endif

#define SL_M4_USART0_INSTANCE 0
#define SL_M4_UART1_INSTANCE  1
#define SL_ULP_UART_INSTANCE  2

// <o SL_DEBUG_INSTANCE> Instance (Select TX/RX pins from chosen component)
//   <SL_M4_USART0_INSTANCE=> UART0/USART0
//   <SL_M4_UART1_INSTANCE=> UART1
//   <SL_ULP_UART_INSTANCE=> ULP UART
// <i> Default: ULP UART
#define SL_DEBUG_INSTANCE SL_ULP_UART_INSTANCE

// <o SL_DEBUG_BAUD_RATE> Baud Rate (Baud/Second) <300-7372800>
// <i> Default: 115200
#define SL_DEBUG_BAUD_RATE 115200

// </e>
// </h>
// <<< end of configuration section >>>

// If Debug UC configuration is enabled, the following dependency checks are
// performed
#if (DEBUG_UART_ENABLE == 1) && (SL_DEBUG_INSTANCE == SL_ULP_UART_INSTANCE) && !defined(ULP_UART_MODULE)
#warning \
  "Configuration Error: ULP_UART component is required when Debug Configuration is set to ULP_UART. Please ensure ULP_UART is installed."
#elif (DEBUG_UART_ENABLE == 1) && (SL_DEBUG_INSTANCE == SL_M4_USART0_INSTANCE) && !defined(USART_MODULE)
#warning \
  "Configuration Error: USART component is required when Debug Configuration is set to UART0/USART0. Please ensure USART is installed."
#elif (DEBUG_UART_ENABLE == 1) && (SL_DEBUG_INSTANCE == SL_M4_UART1_INSTANCE) && !defined(UART_MODULE)
#warning \
  "Configuration Error: UART component is required when Debug Configuration is set to UART1. Please ensure UART1 is installed."
#endif

#endif /* SL_SI91X_DEBUG_UC_CONFIG_H */
