/***************************************************************************/ /**
 * @file uart_at_commands_config.h
 * @brief UART AT commands Config.
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

#ifndef UART_AT_COMMANDS_CONFIG_H
#define UART_AT_COMMANDS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// <<< Use Configuration Wizard in Context Menu >>>

// <h>UART Configuration

// <o UART_BAUD_RATE_DEFAULT> Baud Rate (Baud/Second) <300-921600>
// <i> Default: 115200
#ifndef UART_BAUD_RATE_DEFAULT
#define UART_BAUD_RATE_DEFAULT 115200
#endif

// <o UART_DATA_BITS_DEFAULT> Data Width
// <SL_USART_DATA_BITS_5=> 5
// <SL_USART_DATA_BITS_6=> 6
// <SL_USART_DATA_BITS_7=> 7
// <SL_USART_DATA_BITS_8=> 8
// <i> Default: SL_USART_DATA_BITS_8
#ifndef UART_DATA_BITS_DEFAULT
#define UART_DATA_BITS_DEFAULT SL_USART_DATA_BITS_8
#endif

// <o UART_STOP_BITS_DEFAULT> Stop Bits
// <SL_USART_STOP_BITS_1=> 1
// <SL_USART_STOP_BITS_1_5=> 1.5
// <SL_USART_STOP_BITS_2=> 2
// <i> Default: SL_USART_STOP_BITS_1
#ifndef UART_STOP_BITS_DEFAULT
#define UART_STOP_BITS_DEFAULT SL_USART_STOP_BITS_1
#endif

// <o UART_PARITY_DEFAULT> Parity
// <SL_USART_NO_PARITY=> No Parity
// <SL_USART_EVEN_PARITY=> Even
// <SL_USART_ODD_PARITY=> Odd
// <i> Default: SL_USART_NO_PARITY
#ifndef UART_PARITY_DEFAULT
#define UART_PARITY_DEFAULT SL_USART_NO_PARITY
#endif

// <o UART_HW_FLOW_CTRL_DEFAULT> Hardware Flow Control
// <SL_USART_FLOW_CONTROL_NONE=> None
// <SL_USART_FLOW_CONTROL_CTS=> CTS
// <SL_USART_FLOW_CONTROL_RTS=> RTS
// <SL_USART_FLOW_CONTROL_RTS_CTS=> RTS/CTS
// <i> Default: SL_USART_FLOW_CONTROL_NONE
#ifndef UART_HW_FLOW_CTRL_DEFAULT
#define UART_HW_FLOW_CTRL_DEFAULT SL_USART_FLOW_CONTROL_NONE
#endif

// </h> UART Configuration
// <<< end of configuration section >>>

#ifdef __cplusplus
}
#endif

#endif //UART_AT_COMMANDS_CONFIG_H
