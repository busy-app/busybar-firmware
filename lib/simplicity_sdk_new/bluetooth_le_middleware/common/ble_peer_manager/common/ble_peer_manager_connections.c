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
#include "ble_peer_manager_connections.h"
#include "ble_peer_manager_common_config.h"

// -----------------------------------------------------------------------------
// Static variables
static uint8_t active_conn_num = 0;
static ble_peer_manager_connection_t pm_connections[BLE_PEER_MANAGER_COMMON_MAX_ALLOWED_CONN_COUNT];

// -----------------------------------------------------------------------------
// Public functions
void ble_peer_manager_clear_all_connections(void)
{
  for (uint8_t i = 0; i < BLE_PEER_MANAGER_COMMON_MAX_ALLOWED_CONN_COUNT; i++) {
    pm_connections[i].conn_handle = SL_BT_INVALID_CONNECTION_HANDLE;
  }
  active_conn_num = 0;
}

sl_status_t ble_peer_manager_add_connection(uint8_t conn_handle,
                                            bd_addr peer_address,
                                            sl_bt_connection_role_t role)
{
  for (uint8_t i = 0; i < BLE_PEER_MANAGER_COMMON_MAX_ALLOWED_CONN_COUNT; i++) {
    if (pm_connections[i].conn_handle == SL_BT_INVALID_CONNECTION_HANDLE) {
      pm_connections[i].conn_handle = conn_handle;
      pm_connections[i].conn_type = role;
      memcpy(pm_connections[i].peer_address.addr, peer_address.addr, sizeof(bd_addr));
      active_conn_num++;
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NO_MORE_RESOURCE;
}

sl_status_t ble_peer_manager_delete_connection(uint8_t conn_handle)
{
  for (uint8_t i = 0; i < BLE_PEER_MANAGER_COMMON_MAX_ALLOWED_CONN_COUNT; i++) {
    if (pm_connections[i].conn_handle == conn_handle) {
      pm_connections[i].conn_handle = SL_BT_INVALID_CONNECTION_HANDLE;
      active_conn_num--;
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

uint8_t ble_peer_manager_get_active_conn_number()
{
  return active_conn_num;
}

bd_addr* ble_peer_manager_get_bt_address(uint8_t conn_handle)
{
  for (uint8_t i = 0; i < BLE_PEER_MANAGER_COMMON_MAX_ALLOWED_CONN_COUNT; i++) {
    if (pm_connections[i].conn_handle == conn_handle) {
      return &pm_connections[i].peer_address;
    }
  }
  return NULL;
}

bool ble_peer_manager_is_conn_handle_in_array(uint8_t conn_handle)
{
  for (uint8_t  i = 0; i < BLE_PEER_MANAGER_COMMON_MAX_ALLOWED_CONN_COUNT; i++) {
    if (pm_connections[i].conn_handle == conn_handle) {
      return true;
    }
  }
  return false;
}

bool ble_peer_manager_is_bt_address_already_connected(bd_addr *bt_address)
{
  if (bt_address == NULL) {
    return false;
  }
  for (uint8_t i = 0; i < BLE_PEER_MANAGER_COMMON_MAX_ALLOWED_CONN_COUNT; i++) {
    if (pm_connections[i].conn_handle == SL_BT_INVALID_CONNECTION_HANDLE) {
      continue;
    }
    if (memcmp(pm_connections[i].peer_address.addr, bt_address->addr, sizeof(bd_addr)) == 0) {
      return true;
    }
  }
  return false;
}

bool ble_peer_manager_is_conn_handle_central(uint8_t conn_handle)
{
  for (uint8_t  i = 0; i < BLE_PEER_MANAGER_COMMON_MAX_ALLOWED_CONN_COUNT; i++) {
    if ((pm_connections[i].conn_handle == conn_handle)
        && (pm_connections[i].conn_type == sl_bt_connection_role_central)) {
      return true;
    }
  }
  return false;
}
