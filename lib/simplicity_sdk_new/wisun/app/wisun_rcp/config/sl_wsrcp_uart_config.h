/***************************************************************************//**
 * Copyright 2021 Silicon Laboratories Inc. www.silabs.com
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available here[1]. This software is distributed to you in
 * Source Code format and is governed by the sections of the MSLA applicable to
 * Source Code.
 *
 * [1] www.silabs.com/about-us/legal/master-software-license-agreement
 *
 ******************************************************************************/
#ifndef SL_WSRCP_UART_CONFIG_H
#define SL_WSRCP_UART_CONFIG_H

#include <em_device.h>
#include <sl_device_peripheral.h>

// <<< Use Configuration Wizard in Context Menu >>>

// <h> UART settings
// <o UART_BAUDRATE> Baud rate
// <i> Default: 115200
#define UART_BAUDRATE           115200
// </h>

// <<< end of configuration section >>>
#include <em_eusart.h>
#define UART_PERIPHERAL_VAL     SL_PERIPHERAL_EUSART0
#define UART_PERIPHERAL         EUSART0
#define UART_RX_IRQ             EUSART0_RX_IRQn
#define UART_LDMA_SIGNAL_RX     ldmaPeripheralSignal_EUSART0_RXFL
#define UART_LDMA_SIGNAL_TX     ldmaPeripheralSignal_EUSART0_TXFL

#define UART_PORT_TX            gpioPortA
#define UART_PIN_TX             8

#define UART_PORT_RX            gpioPortA
#define UART_PIN_RX             9

#endif
