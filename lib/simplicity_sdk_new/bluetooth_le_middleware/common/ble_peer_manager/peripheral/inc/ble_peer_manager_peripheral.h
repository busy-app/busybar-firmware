/***************************************************************************//**
 * @file
 * @brief Bluetooth Peer Manager - peripheral
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
#ifndef BLE_PEER_MANAGER_PERIPHERAL_H
#define BLE_PEER_MANAGER_PERIPHERAL_H

/***********************************************************************************************//**
 * @addtogroup ble_peer_manager_peripheral
 * @{
 **************************************************************************************************/

#include "sl_status.h"
#include "sl_bt_api.h"

/**************************************************************************//**
 * Initialize the Peer Manager.
 *
 * Set default values for advertiser, set state to IDLE
 * and create an emtpy connection array.
 *****************************************************************************/
void ble_peer_manager_peripheral_init(void);

/**************************************************************************//**
 * Set advertiser parameters.
 *
 * Set the advertiser parameters if the Peer Manager is in IDLE state.
 * Call this function before starting the advertiser.
 *
 * @param[in] adv_discovery_mode Advertising discovery mode. Should be one of
 * the following:
 * - sl_bt_advertiser_non_discoverable
 * - sl_bt_advertiser_limited_discoverable
 * - sl_bt_advertiser_general_discoverable
 * - sl_bt_advertiser_broadcast
 * - sl_bt_advertiser_user_data
 *
 * @retval SL_STATUS_OK if successful otherwise error code.
 * @retval SL_STATUS_INVALID_STATE if the Peer Manager is in
 *         BLE_PEER_MANAGER_ADVERTISING state.
 *****************************************************************************/
sl_status_t ble_peer_manager_peripheral_set_advertiser_discovery_mode(sl_bt_advertiser_discovery_mode_t adv_discovery_mode);

/**************************************************************************//**
 * Set advertiser parameters.
 *
 * Set the advertiser PHY if the Peer Manager is in IDLE state.
 * Call this function before starting the advertiser.
 *
 * @param[in] adv_phy Advertising PHY. Should be one of
 * the following:
 * - sl_bt_gap_phy_1m
 * - sl_bt_gap_phy_2m
 *
 * @retval SL_STATUS_OK if successful otherwise error code.
 * @retval SL_STATUS_INVALID_STATE if the Peer Manager is in
 *         BLE_PEER_MANAGER_ADVERTISING state.
 *****************************************************************************/
sl_status_t ble_peer_manager_peripheral_set_advertiser_phy(sl_bt_gap_phy_t adv_phy);

/**************************************************************************//**
 * Create a central connection.
 *
 * The Peer Manager will start the advertiser. If a connection opened
 * event is received, and the advertising handle is the same as
 * in the sl_bt_legacy_advertiser_start() function, the Peer Manager
 * will stop the advertiser and save the connection.
 *
 * @retval SL_STATUS_OK if successful otherwise error code.
 *****************************************************************************/
sl_status_t ble_peer_manager_peripheral_create_connection(void);

/**************************************************************************//**
 * Start advertising.
 *
 * If handle is SL_BT_INVALID_ADVERTISING_SET_HANDLE the peer manager will
 * create an advertising handle and generate data using the default value
 * for discovery mode, or the value that was set using the
 * ble_peer_manager_peripheral_set_advertiser_discovery_mode() function and
 * the default value for advertiser timing. If the handle is not invalid,
 * it will use the given handle to start advertising.
 *
 * @param[in] advertising_handle Advertising handle to use for advertising.
 *
 * @retval SL_STATUS_OK if successful otherwise error code.
 *****************************************************************************/
sl_status_t ble_peer_manager_peripheral_start_advertising(uint8_t advertising_handle);

/**************************************************************************//**
 * Stop advertising.
 *
 * @param[in] advertising_handle Advertising handle to stop.
 *
 * @retval SL_STATUS_OK if successful otherwise error code.
 *****************************************************************************/
sl_status_t ble_peer_manager_peripheral_stop_advertising(uint8_t advertising_handle);

/**************************************************************************//**
 * Close connection.
 *
 * @param[in] conn_handle Connection handle of the connection
 *            which needs to be closed.
 *
 * @retval SL_STATUS_OK if successful otherwise error code.
 * @retval SL_STATUS_NOT_FOUND if the connection handle is not found.
 *****************************************************************************/
sl_status_t ble_peer_manager_peripheral_close_connection(uint8_t conn_handle);

/** @} (end addtogroup ble_peer_manager_peripheral) */
#endif // BLE_PEER_MANAGER_PERIPHERAL_H
