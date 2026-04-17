/***************************************************************************/ /**
 * @file sl_si91x_uart0_rs485_config.h
 * @brief UART0 RS485 Config.
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

#ifndef SL_SI91X_UART0_RS485_CONFIG_H
#define SL_SI91X_UART0_RS485_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_usart.h"

// <<< Use Configuration Wizard in Context Menu >>>

//  <e>UART0 UC Configuration
//  <i> Enable: Peripheral configuration is taken straight from the configuration set in the universal configuration (UC).
//  <i> Disable: If the application demands it to be modified during runtime, use the sl_si91x_usart_set_configuration API to modify the peripheral configuration.
//  <i> Default: 1
#define USART_UC 1

// <h>UART0 Configuration
#define ENABLE  1
#define DISABLE 0

#define USART0_MODULE 0

#if (USART_UC)
// <o SL_USART_BAUDRATE> Baud Rate (Baud/Second)
// <BAUD_RATE_300=> 300
// <BAUD_RATE_600=> 600
// <BAUD_RATE_1200=> 1200
// <BAUD_RATE_2400=> 2400
// <BAUD_RATE_4800=> 4800
// <BAUD_RATE_9600=> 9600
// <BAUD_RATE_19200=> 19200
// <BAUD_RATE_38400=> 38400
// <BAUD_RATE_57600=> 57600
// <BAUD_RATE_115200=> 115200
// <BAUD_RATE_230400=> 230400
// <BAUD_RATE_460800=> 460800
// <BAUD_RATE_921600=> 921600
// <i> Default: BAUD_RATE_115200
#define SL_USART_BAUDRATE BAUD_RATE_115200

// <o SL_USART_PARITY> Parity
// <SL_USART_NO_PARITY=> No Parity
// <SL_USART_EVEN_PARITY=> Even
// <SL_USART_ODD_PARITY=> Odd
// <i> Default: SL_USART_NO_PARITY
#define SL_USART_PARITY SL_USART_NO_PARITY

// <o SL_USART_STOP_BITS> Stop Bits
// <SL_USART_STOP_BITS_1=> 1
// <SL_USART_STOP_BITS_1_5=> 1.5
// <SL_USART_STOP_BITS_2=> 2
// <i> Default: USART_STOP_BITS_1
#define SL_USART_STOP_BITS SL_USART_STOP_BITS_1

// <o SL_USART_DATA_BITS> Data Width
// <SL_USART_DATA_BITS_5=> 5
// <SL_USART_DATA_BITS_6=> 6
// <SL_USART_DATA_BITS_7=> 7
// <SL_USART_DATA_BITS_8=> 8
// <i> Default: USART_DATA_BITS_8
#define SL_USART_DATA_BITS SL_USART_DATA_BITS_8

#endif

// </h>  UART0 Configuration
// </e>

//  <h>UART0_RS485 UC Configuration
#define SL_UART0_RS485_MULTIDROP_ENABLE 1
#define RS485_USART_UC                  1
#define SL_UART0_RS485_TRANSMIT_MODE    0

// <o SL_UART0_RS485_TRANSFER_MODE> Transfer Mode
// <SL_UART_SW_CTRL_HALF_DUPLEX_MODE=> SW_CTRL_HALF_DUPLEX
// <SL_UART_HW_CTRL_HALF_DUPLEX_MODE=> HW_CTRL_HALF_DUPLEX
// <SL_UART_FULL_DUPLEX_MODE=> FULL_DUPLEX
// <i> Default: HW_CTRL_HALF_DUPLEX
#define SL_UART0_RS485_TRANSFER_MODE SL_UART_HW_CTRL_HALF_DUPLEX_MODE

// <o SL_UART0_RS485_DE_ASSERT_TIME> DE Assertion Time (no of serial clock period) <0 - 255>
// <i> Default: 10
#define SL_UART0_RS485_DE_ASSERT_TIME 10

// <o SL_UART0_RS485_DE_DEASSERT_TIME> DE De-Assertion Time (no of serial clock period) <0 - 255>
// <i> Default: 10
#define SL_UART0_RS485_DE_DEASSERT_TIME 10

// <o SL_UART0_RS485_DE_POL> DE Polarity
// <SL_UART_DRIVER_EN_POL_HIGH=> High
// <SL_UART_DRIVER_EN_POL_LOW=> Low
// <i> Default: SL_UART_DRIVER_EN_POL_HIGH
#define SL_UART0_RS485_DE_POL SL_UART_DRIVER_EN_POL_HIGH

// <o SL_UART0_RS485_RE_POL> RE Polarity
// <SL_UART_RECEIVER_EN_POL_HIGH=> High
// <SL_UART_RECEIVER_EN_POL_LOW=> Low
// <i> Default: SL_UART_RECEIVER_EN_POL_LOW
#define SL_UART0_RS485_RE_POL SL_UART_RECEIVER_EN_POL_LOW

// <o SL_UART0_RS485_DE_RE_TAT_SET> DE-RE Turnaround Time (no of serial clocks) <0 - 65535>
// <i> Default: 10
#define SL_UART0_RS485_DE_RE_TAT_SET 10

// <o SL_UART0_RS485_RE_DE_TAT_SET> RE-DE Turnaround Time (no of serial clocks) <0 - 65535>
// <i> Default: 10
#define SL_UART0_RS485_RE_DE_TAT_SET 10

// <q SL_UART0_RS485_ADDR_MATCH> RECEIVE Enable(ADDR Match Enable)
// <i> Default: 1
#define SL_UART0_RS485_ADDR_MATCH 0

// <q SL_UART0_RS485_SEND_ADDR> SEND Enable(SEND Addr Enable)
// <i> Default: 1
#define SL_UART0_RS485_SEND_ADDR 1
// </h>  UART0_RS485 UC Configuration

#ifdef __cplusplus
}
#endif
// <<< end of configuration section >>>

#if (USART_UC)
sl_si91x_usart_control_config_t usart_configuration = { .baudrate     = SL_USART_BAUDRATE,
                                                        .mode         = SL_USART_MODE_ASYNCHRONOUS,
                                                        .parity       = SL_USART_PARITY,
                                                        .stopbits     = SL_USART_STOP_BITS,
                                                        .databits     = SL_USART_DATA_BITS,
                                                        .usart_module = USART0_MODULE };
#endif

usart_rs485_config_t uart0_rs485_configuration = { .transfer_mode           = SL_UART0_RS485_TRANSFER_MODE,
                                                   .de_pol                  = SL_UART0_RS485_DE_POL,
                                                   .re_pol                  = SL_UART0_RS485_RE_POL,
                                                   .de_assert_time          = SL_UART0_RS485_DE_ASSERT_TIME,
                                                   .de_deassert_time        = SL_UART0_RS485_DE_DEASSERT_TIME,
                                                   .multidrop_enable        = SL_UART0_RS485_MULTIDROP_ENABLE,
                                                   .tat_set.de_re_tat_set   = SL_UART0_RS485_DE_RE_TAT_SET,
                                                   .tat_set.re_de_tat_set   = SL_UART0_RS485_RE_DE_TAT_SET,
                                                   .multidrop.addr_match    = SL_UART0_RS485_ADDR_MATCH,
                                                   .multidrop.send_addr     = SL_UART0_RS485_SEND_ADDR,
                                                   .multidrop.transmit_mode = SL_UART0_RS485_TRANSMIT_MODE };

#define SL_USART0_RS485_RE_PIN  SL_RS485_UART0_RE_PIN
#define SL_USART0_RS485_RE_PORT SL_RS485_UART0_RE_PORT
#define SL_USART0_RS485_RE_MUX  SL_RS485_UART0_RE_MUX
#define SL_USART0_RS485_RE_PAD  SL_RS485_UART0_RE_PAD

#define SL_USART0_RS485_DE_PIN  SL_RS485_UART0_DE_PIN
#define SL_USART0_RS485_DE_PORT SL_RS485_UART0_DE_PORT
#define SL_USART0_RS485_DE_MUX  SL_RS485_UART0_DE_MUX
#define SL_USART0_RS485_DE_PAD  SL_RS485_UART0_DE_PAD

#define SL_USART0_RS485_EN_PIN  SL_RS485_UART0_EN_PIN
#define SL_USART0_RS485_EN_PORT SL_RS485_UART0_EN_PORT
#define SL_USART0_RS485_EN_MUX  SL_RS485_UART0_EN_MUX
#define SL_USART0_RS485_EN_PAD  SL_RS485_UART0_EN_PAD

sl_uart_rs485_pin_init_t uart0_pin_init = {
  .re.port       = SL_USART0_RS485_RE_PORT,
  .re.pin        = SL_USART0_RS485_RE_PIN,
  .re.pad        = SL_USART0_RS485_RE_PAD,
  .re.mux        = SL_USART0_RS485_RE_MUX,
  .de.port       = SL_USART0_RS485_DE_PORT,
  .de.pin        = SL_USART0_RS485_DE_PIN,
  .de.pad        = SL_USART0_RS485_DE_PAD,
  .de.mux        = SL_USART0_RS485_DE_MUX,
  .rs485_en.port = SL_USART0_RS485_EN_PORT,
  .rs485_en.pin  = SL_USART0_RS485_EN_PIN,
  .rs485_en.pad  = SL_USART0_RS485_EN_PAD,
  .rs485_en.mux  = SL_USART0_RS485_EN_MUX,
};

#endif //SL_SI91X_UART0_RS485_CONFIG_H