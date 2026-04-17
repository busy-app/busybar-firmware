/***************************************************************************//**
 * @file
 * @brief Bluetooth NCP Transport over IO Stream configuration header
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_H
#define SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_H

#include "sl_iostream.h"

/***********************************************************************************************//**
 * @addtogroup bluetooth_ncp_transport
 * @{
 **************************************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <h> IO Stream settings

// <o SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_STREAM_TYPE> Stream type
// <SL_IOSTREAM_TYPE_RTT=> RTT
// <SL_IOSTREAM_TYPE_UART=> UART
// <SL_IOSTREAM_TYPE_VUART=> VUART
// <i> Default: UART
#define SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_STREAM_TYPE        SL_IOSTREAM_TYPE_UART

// <s SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_STREAM_INSTANCE> Preferred instance
// <i> Preferred IOStream instance name
#define SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_STREAM_INSTANCE    "vcom"

// </h>

// <h> Buffers

// <o SL_BT_NCP_TRANSPORT_CONFIG_RX_BUF_SIZE> Receive buffer size (bytes) <260-1024>
// <i> Default: 520
// <i> Define the size of the receive buffer in bytes.
#define SL_BT_NCP_TRANSPORT_CONFIG_RX_BUF_SIZE        520

// <o SL_BT_NCP_TRANSPORT_CONFIG_TX_BUF_SIZE> Transmit buffer size (bytes) <260-4096>
// <i> Default: 260
// <i> Define the size of the transmit buffer in bytes.
#define SL_BT_NCP_TRANSPORT_CONFIG_TX_BUF_SIZE        260

// </h>

// <h> Runtime settings

// <q SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_SEPARATE_TX> Separate runtime for TX
// <i> Create separate runtime for transmission.
// <i> Default: Off
#define SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_SEPARATE_TX        0

// <o SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_TX_TASK_PRIO> Runtime context priority for TX
// <APP_RTA_PRIORITY_LOW=> Low
// <APP_RTA_PRIORITY_BELOW_NORMAL=> Below normal
// <APP_RTA_PRIORITY_NORMAL=> Normal
// <APP_RTA_PRIORITY_ABOVE_NORMAL=> Above normal
// <APP_RTA_PRIORITY_HIGH=> High
// <i> Default: Normal
// <i> Only applicable for RTOS
#define SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_TX_TASK_PRIO       APP_RTA_PRIORITY_NORMAL

// <o SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_TX_TASK_STACK> Stack size (in bytes) for TX
// <i> Default: 1024
// <i> Only applicable for RTOS
#define SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_TX_TASK_STACK      1024

// <o SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_TX_WAIT_FOR_GUARD> Timeout for guard (in ticks) for TX
// <i> Default: 10
// <i> Only applicable for RTOS
#define SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_TX_WAIT_FOR_GUARD  10

// <o SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_RX_TASK_PRIO> Runtime context priority for RX
// <APP_RTA_PRIORITY_LOW=> Low
// <APP_RTA_PRIORITY_BELOW_NORMAL=> Below normal
// <APP_RTA_PRIORITY_NORMAL=> Normal
// <APP_RTA_PRIORITY_ABOVE_NORMAL=> Above normal
// <APP_RTA_PRIORITY_HIGH=> High
// <i> Default: Normal
// <i> Only applicable for RTOS
#define SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_RX_TASK_PRIO       APP_RTA_PRIORITY_NORMAL

// <o SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_RX_TASK_STACK> Stack size (in bytes) for RX
// <i> Default: 1024
// <i> Only applicable for RTOS
#define SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_RX_TASK_STACK      1024

// <o SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_RX_WAIT_FOR_GUARD> Timeout for guard (in ticks) for RX
// <i> Default: 10
// <i> Only applicable for RTOS
#define SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_RX_WAIT_FOR_GUARD  10

// </h>

// <<< end of configuration section >>>

/** @} (end addtogroup bluetooth_ncp_transport) */

#endif // SL_BT_NCP_TRANSPORT_IOSTREAM_CONFIG_H
