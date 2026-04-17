/***************************************************************************//**
 * @file
 * @brief Bluetooth Peer Manager - common
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
#ifndef BLE_PEER_MANAGER_COMMON_H
#define BLE_PEER_MANAGER_COMMON_H

/***********************************************************************************************//**
 * @addtogroup ble_peer_manager_common
 * @{
 **************************************************************************************************/

#include <stdint.h>

typedef enum {
  BLE_PEER_MANAGER_ON_CONN_OPENED_CENTRAL = 0,
  BLE_PEER_MANAGER_ON_CONN_OPENED_PERIPHERAL,
  BLE_PEER_MANAGER_ON_CONN_CLOSED,
  BLE_PEER_MANAGER_ON_ADV_STOPPED,
  BLE_PEER_MANAGER_ERROR,
} ble_peer_manager_evt_id;

typedef struct ble_peer_manager_evt_type_s {
  ble_peer_manager_evt_id evt_id;
  uint8_t connection_id;
} ble_peer_manager_evt_type_t;

void ble_peer_manager_on_event(ble_peer_manager_evt_type_t *event);

/** @} (end addtogroup ble_peer_manager_common) */
#endif // BLE_PEER_MANAGER_COMMON_H
