/***************************************************************************/ /**
 * @file
 * @brief IOSTREAM_USART Config.
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

#ifndef SL_IOSTREAM_USART_VCOM_CONFIG_H
#define SL_IOSTREAM_USART_VCOM_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h>USART settings

#define ENABLE  1
#define DISABLE 0

// <o SL_IOSTREAM_USART_VCOM_BAUDRATE> Baud rate <9600-921600>
// <i> Default: 115200
#define SL_IOSTREAM_USART_VCOM_BAUDRATE 115200

// <o SL_IOSTREAM_USART_VCOM_PARITY> Parity
// <SL_USART_NO_PARITY=> No Parity
// <SL_USART_EVEN_PARITY=> Even parity
// <SL_USART_ODD_PARITY=> Odd parity
// <i> Default: SL_USART_NO_PARITY
#define SL_IOSTREAM_USART_VCOM_PARITY SL_USART_NO_PARITY

// <o SL_IOSTREAM_USART_VCOM_STOP_BITS> Stop bits
// <SL_USART_STOP_BITS_0_5=> 0.5 stop bits
// <SL_USART_STOP_BITS_1=> 1 stop bits
// <SL_USART_STOP_BITS_1_5=> 1.5 stop bits
// <SL_USART_STOP_BITS_2=> 2 stop bits
// <i> Default: SL_USART_STOP_BITS_1
#define SL_IOSTREAM_USART_VCOM_STOP_BITS SL_USART_STOP_BITS_1

#define SL_IOSTREAM_USART_VCOM_MODE SL_USART_MODE_ASYNCHRONOUS

// <o SL_IOSTREAM_USART_VCOM_DATA_BITS> Data Width
// <SL_USART_DATA_BITS_5=> 5 data bits
// <SL_USART_DATA_BITS_6=> 6 data bits
// <SL_USART_DATA_BITS_7=> 7 data bits
// <SL_USART_DATA_BITS_8=> 8 data bits
// <i> Default: SL_USART_DATA_BITS_8
#define SL_IOSTREAM_USART_VCOM_DATA_BITS SL_USART_DATA_BITS_8

// <o SL_IOSTREAM_USART_VCOM_FLOW_CONTROL_TYPE> Flow control
// <SL_USART_FLOW_CONTROL_NONE=> None
// <SL_USART_FLOW_CONTROL_CTS=> CTS
// <SL_USART_FLOW_CONTROL_RTS=> RTS
// <SL_USART_FLOW_CONTROL_RTS_CTS=> CTS/RTS
// <i> Default: SL_USART_FLOW_CONTROL_NONE
#define SL_IOSTREAM_USART_VCOM_FLOW_CONTROL_TYPE SL_USART_FLOW_CONTROL_NONE

// <q SL_SI91X_VCOM_ENABLE> VCOM enable
// <i> Default: 1
#define SL_SI91X_VCOM_ENABLE 1

#define USART0_MODULE   0
#define UART1_MODULE    1
#define ULP_UART_MODULE 2

#if SL_SI91X_VCOM_ENABLE
#define SL_USART_MODULE ULP_UART_MODULE
#else
#define SL_USART_MODULE USART0_MODULE
#endif

// <o SL_IOSTREAM_USART_VCOM_RX_BUFFER_SIZE> Receive buffer size
// <i> Default: 32
#define SL_IOSTREAM_USART_VCOM_RX_BUFFER_SIZE 32

// <q SL_IOSTREAM_USART_VCOM_CONVERT_BY_DEFAULT_LF_TO_CRLF> Convert \n to \r\n
// <i> It can be changed at runtime using the C API.
// <i> Default: 0
#define SL_IOSTREAM_USART_VCOM_CONVERT_BY_DEFAULT_LF_TO_CRLF 0
// </h>

// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <ulp_uart signal=TX,RX> SL_SI91X_IOSTREAM
// $[ULP_UART_SL_SI91X_IOSTREAM]
#ifndef SL_SI91X_IOSTREAM_PERIPHERAL
#define SL_SI91X_IOSTREAM_PERIPHERAL ULP_UART
#endif

// ULP_UART TX on ULP_GPIO_11/GPIO_75
#ifndef SL_SI91X_IOSTREAM_TX_PORT
#define SL_SI91X_IOSTREAM_TX_PORT ULP
#endif
#ifndef SL_SI91X_IOSTREAM_TX_PIN
#define SL_SI91X_IOSTREAM_TX_PIN 11
#endif
#ifndef SL_SI91X_IOSTREAM_TX_LOC
#define SL_SI91X_IOSTREAM_TX_LOC 1
#endif

// ULP_UART RX on ULP_GPIO_9/GPIO_73
#ifndef SL_SI91X_IOSTREAM_RX_PORT
#define SL_SI91X_IOSTREAM_RX_PORT ULP
#endif
#ifndef SL_SI91X_IOSTREAM_RX_PIN
#define SL_SI91X_IOSTREAM_RX_PIN 9
#endif
#ifndef SL_SI91X_IOSTREAM_RX_LOC
#define SL_SI91X_IOSTREAM_RX_LOC 3
#endif
// [ULP_UART_SL_SI91X_IOSTREAM]$
// <<< sl:end pin_tool >>>

#endif