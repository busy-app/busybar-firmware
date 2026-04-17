/***************************************************************************//**
 * @file
 * @brief Bluetooth Peer Manager - common config
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
#ifndef BLE_PEER_MANAGER_COMMON_CONFIG_H
#define BLE_PEER_MANAGER_COMMON_CONFIG_H

/***********************************************************************************************//**
 * @addtogroup ble_peer_manager_common
 * @{
 **************************************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <e BLE_PEER_MANAGER_COMMON_LOG> Log
// <i> Should the module log or not.
// <i> Default: 0
#ifndef BLE_PEER_MANAGER_COMMON_LOG
#define BLE_PEER_MANAGER_COMMON_LOG                          0
#endif

// <s BLE_PEER_MANAGER_COMMON_LOG_PREFIX> Log prefix
// <i> Default: "[PM] "
#ifndef BLE_PEER_MANAGER_COMMON_LOG_PREFIX
#define BLE_PEER_MANAGER_COMMON_LOG_PREFIX                   "[PM]"
#endif

// </e>

// <o BLE_PEER_MANAGER_COMMON_MAX_ALLOWED_CONN_COUNT> Max connections <0..32>
// <i> This value should be less than or equal to SL_BT_CONFIG_MAX_CONNECTIONS
// <i> Default: 4
#ifndef BLE_PEER_MANAGER_COMMON_MAX_ALLOWED_CONN_COUNT
#define BLE_PEER_MANAGER_COMMON_MAX_ALLOWED_CONN_COUNT       4
#endif

// <o BLE_PEER_MANAGER_COMMON_TIMEOUT_GATT_MS> GATT timeout <0..10000>
// <i> Default: 10000
#ifndef BLE_PEER_MANAGER_COMMON_TIMEOUT_GATT_MS
#define BLE_PEER_MANAGER_COMMON_TIMEOUT_GATT_MS              10000
#endif

// <<< end of configuration section >>>

/** @} (end addtogroup ble_peer_manager_common) */
#endif // BLE_PEER_MANAGER_COMMON_CONFIG_H
