/***************************************************************************//**
 * @file
 * @brief Bluetooth Peer Manager - connections
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
#ifndef BLE_PEER_MANAGER_COMMON_CONNECTIONS_H
#define BLE_PEER_MANAGER_COMMON_CONNECTIONS_H

#include "sl_status.h"
#include "sl_bt_api.h"
#include "ble_peer_manager_common.h"

// Struct for the connection array of the Peer Manager
typedef struct ble_peer_manager_connection_s {
  uint8_t conn_handle;
  sl_bt_connection_role_t conn_type;
  bd_addr peer_address;
} ble_peer_manager_connection_t;

/******************************************************************************
 * Set all connection handles to SL_BT_INVALID_CONNECTION_HANDLE.
 *****************************************************************************/
void ble_peer_manager_clear_all_connections(void);

/******************************************************************************
 * Add a connection to the list of managed connections.
 *
 * @param[in] conn_handle Connection handle to add.
 * @param[in] peer_address BT address of the connection.
 * @param[in] role Role of the connection.
 *
 * @retval SL_STATUS_OK if successful
 * @retval SL_STATUS_NO_MORE_RESOURCE if the new connection cannot be added
 *****************************************************************************/
sl_status_t ble_peer_manager_add_connection(uint8_t conn_handle,
                                            bd_addr peer_address,
                                            sl_bt_connection_role_t role);

/******************************************************************************
 * Delete a connection from the connection array.
 *
 * @param[in] conn_handle Connection handle to delete from the array.
 *
 * @retval SL_STATUS_OK if successful
 * @retval SL_STATUS_NOT_FOUND if the connection handle is not found in the array
 *****************************************************************************/
sl_status_t ble_peer_manager_delete_connection(uint8_t conn_handle);

/******************************************************************************
 * Get the number of active connections.
 *
 * @return Number of active connections
 *****************************************************************************/
uint8_t ble_peer_manager_get_active_conn_number(void);

/******************************************************************************
 * Get the BT address of a connection.
 *
 * @param[in] conn_handle Connection handle to get the BT address for.
 *
 * @return Pointer to the BT address array
 *****************************************************************************/
bd_addr* ble_peer_manager_get_bt_address(uint8_t conn_handle);

/******************************************************************************
 * Check if a connection handle is in the connection array.
 *
 * @param[in] conn_handle Connection handle to check.
 *
 * @retval true if the connection handle is in the array
 * @retval false if the connection handle is not in the array
 *****************************************************************************/
bool ble_peer_manager_is_conn_handle_in_array(uint8_t conn_handle);

/******************************************************************************
 * Check if a device with a given BT address is already connected.
 *
 * @param[in] bt_address BT address to check.
 *
 * @return true if the BT address is already connected, false otherwise
 *****************************************************************************/
bool ble_peer_manager_is_bt_address_already_connected(bd_addr *bt_address);

/******************************************************************************
 * Check if a connection handle is a central connection.
 *
 * @param[in] conn_handle Connection handle to check.
 *
 * @retval true if the connection handle is a central connection
 * @retval false if the connection handle is not a central connection
 *****************************************************************************/
bool ble_peer_manager_is_conn_handle_central(uint8_t conn_handle);

#endif // BLE_PEER_MANAGER_COMMON_CONNECTIONS_H
