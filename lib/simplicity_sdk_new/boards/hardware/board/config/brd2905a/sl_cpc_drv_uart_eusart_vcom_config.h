/***************************************************************************//**
 * @file
 * @brief CPC EUSART driver configuration file.
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

#include "sl_cpc_config.h"

// <<< Use Configuration Wizard in Context Menu >>>

#ifndef SL_CPC_DRV_UART_EUSART_VCOM_CONFIG_H
#define SL_CPC_DRV_UART_EUSART_VCOM_CONFIG_H

// <h> CPC - EUSART Driver Configuration

// <o SL_CPC_DRV_UART_VCOM_RX_PAYLOAD_MAX_LENGTH>Max Rx Payload Length<1-4087>
// <i> Default: SL_CPC_RX_PAYLOAD_MAX_LENGTH
// <i> Maximum size of the payload in bytes of each RX buffer
#define SL_CPC_DRV_UART_VCOM_RX_PAYLOAD_MAX_LENGTH      SL_CPC_RX_PAYLOAD_MAX_LENGTH

// <o SL_CPC_DRV_UART_VCOM_TX_QUEUE_ITEM_MAX_COUNT> Tx Queue Size
// <i> Default: SL_CPC_TX_QUEUE_ITEM_MAX_COUNT
// <i> The maximum number of outgoing messages capable of being queued for transmission.
#define SL_CPC_DRV_UART_VCOM_TX_QUEUE_ITEM_MAX_COUNT    SL_CPC_TX_QUEUE_ITEM_MAX_COUNT

// <o SL_CPC_DRV_UART_VCOM_RX_BUFFER_MAX_COUNT> Total Number of Rx Buffers
// <i> Default: SL_CPC_RX_BUFFER_MAX_COUNT
// <i> Total number of CPC RX buffers available across all endpoints
// <i> Multiple RX buffers can be associated with a single endpoint
#define SL_CPC_DRV_UART_VCOM_RX_BUFFER_MAX_COUNT        SL_CPC_RX_BUFFER_MAX_COUNT

// <o SL_CPC_DRV_UART_VCOM_RX_QUEUE_SIZE> Number of frame that can be queued in the driver receive queue
// <i> Default: 10
#define SL_CPC_DRV_UART_VCOM_RX_QUEUE_SIZE            10

// <o SL_CPC_DRV_UART_VCOM_TX_QUEUE_SIZE> Number of frame that can be queued in the driver transmit queue
// <i> Default: 10
#define SL_CPC_DRV_UART_VCOM_TX_QUEUE_SIZE            10

// <o SL_CPC_DRV_UART_VCOM_BAUDRATE> EUSART Baudrate
// <i> Default: 115200
#define SL_CPC_DRV_UART_VCOM_BAUDRATE              115200

// <o SL_CPC_DRV_UART_VCOM_FLOW_CONTROL_TYPE> Flow control
// <eusartHwFlowControlNone=> None
// <eusartHwFlowControlCtsAndRts=> CTS/RTS
// <i> Default: eusartHwFlowControlCtsAndRts
#define SL_CPC_DRV_UART_VCOM_FLOW_CONTROL_TYPE eusartHwFlowControlCtsAndRts
// </h>

// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <eusart signal=TX,RX,(CTS),(RTS)> SL_CPC_DRV_UART_VCOM
// $[EUSART_SL_CPC_DRV_UART_VCOM]
#define SL_CPC_DRV_UART_VCOM_PERIPHERAL          EUSART0
#define SL_CPC_DRV_UART_VCOM_PERIPHERAL_NO       0

// EUSART0 TX on PA00
#define SL_CPC_DRV_UART_VCOM_TX_PORT             SL_GPIO_PORT_A
#define SL_CPC_DRV_UART_VCOM_TX_PIN              0

// EUSART0 RX on PA07
#define SL_CPC_DRV_UART_VCOM_RX_PORT             SL_GPIO_PORT_A
#define SL_CPC_DRV_UART_VCOM_RX_PIN              7

// EUSART0 CTS on PA05
#define SL_CPC_DRV_UART_VCOM_CTS_PORT            SL_GPIO_PORT_A
#define SL_CPC_DRV_UART_VCOM_CTS_PIN             5

// EUSART0 RTS on PA04
#define SL_CPC_DRV_UART_VCOM_RTS_PORT            SL_GPIO_PORT_A
#define SL_CPC_DRV_UART_VCOM_RTS_PIN             4

// [EUSART_SL_CPC_DRV_UART_VCOM]$
// <<< sl:end pin_tool >>>

#endif /* SL_CPC_DRV_UART_VCOM_CONFIG_H */
