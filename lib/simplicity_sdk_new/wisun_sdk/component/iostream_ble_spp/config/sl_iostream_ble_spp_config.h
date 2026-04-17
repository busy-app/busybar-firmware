/***************************************************************************//**
 * @file sl_iostream_ble_spp_config.h
 * @brief I/O Stream BLE Serial Port Profile component configuration
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
#ifndef SL_IOSTREAM_BLE_SPP_CONFIG_H
#define SL_IOSTREAM_BLE_SPP_CONFIG_H

/**************************************************************************//**
 * @defgroup SL_IOSTREAM_BLE_SPP_CONFIG Configurations
 * @ingroup SL_IOSTREAM_BLE_SPP_API
 * @{
 *****************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <h>I/O Stream BLE Serial Port Profile configuration

// <o SL_IOSTREAM_BLE_SPP_FIFO_TX_SIZE> I/O Stream BLE SPP FIFO TX buffer size
// <i> Default: 4096
#define SL_IOSTREAM_BLE_SPP_FIFO_TX_SIZE                     4096UL

// <o SL_IOSTREAM_BLE_SPP_FIFO_RX_SIZE> I/O Stream BLE SPP FIFO RX buffer size
// <i> Default: 128
#define SL_IOSTREAM_BLE_SPP_FIFO_RX_SIZE                     128UL

// <o SL_IOSTREAM_BLE_SPP_MAX_NOTIFICATION_SIZE> I/O Stream BLE SPP max notification size
// <i> At most, ATT_MTU - 3 number of bytes can be sent in a notification, (ATT_MTU = 250 bytes)
// <i> See: sl_bt_gatt_server_send_notification API documentation
// <i> Default: 247
#define SL_IOSTREAM_BLE_SPP_MAX_NOTIFICATION_SIZE            247U

// </h>

// <<< end of configuration section >>>

/** @}*/

#endif // SL_IOSTREAM_BLE_SPP_CONFIG_H
