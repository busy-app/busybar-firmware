/***************************************************************************//**
 * @file
 * @brief Bluetooth Peer Manager - central
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
#ifndef BLE_PEER_MANAGER_CENTRAL_H
#define BLE_PEER_MANAGER_CENTRAL_H

/***********************************************************************************************//**
 * @addtogroup ble_peer_manager_central
 * @{
 **************************************************************************************************/

#include "sl_status.h"
#include "sl_bt_api.h"

/**************************************************************************//**
 * Initialize the Peer Manager.
 *
 * Set default values for scanner, reset filters, set state to IDLE
 * and create an emtpy connection array.
 *****************************************************************************/
void ble_peer_manager_central_init(void);

/**************************************************************************//**
 * Set scanner parameters.
 *
 * Set the scanner parameters if the Peer Manager is in IDLE state.
 * Call this function before starting the scanner.
 *
 * @param[in] scan_phy PHY to use for scanning.
 * @param[in] discovery_mode Discovery mode to use for scanning.
 * @param[in] scan_mode Scan mode to use for scanning (active or passive).
 * @param[in] scan_interval Scan interval to use for scanning.
 * @param[in] scan_window Scan window to use for scanning.
 *
 * @retval SL_STATUS_OK if successful otherwise error code.
 * @retval SL_STATUS_INVALID_STATE if the Peer Manager is in SCANNING state.
 *****************************************************************************/
sl_status_t ble_peer_manager_central_set_scanner(sl_bt_gap_phy_coding_t scanning_phy,
                                                 sl_bt_scanner_discover_mode_t scan_discovery_mode,
                                                 sl_bt_scanner_scan_mode_t scan_mode,
                                                 uint8_t scan_interval,
                                                 uint8_t scan_window);

/**************************************************************************//**
 * Create a central connection.
 *
 * The Peer Manager will start the scanner. If a connection opened
 * event is received, and the connection handle is the same as
 * in the sl_bt_connection_open() function, the Peer Manager will stop the
 * scanner and save the connection.
 *
 * @retval SL_STATUS_OK if successful otherwise error code.
 *****************************************************************************/
sl_status_t ble_peer_manager_central_create_connection(void);

/**************************************************************************//**
 * Open a connection as a central.
 *
 * @param[in] address Address of the peripheral device.
 * @param[in] address_type Address type of the peripheral device.
 *
 * @retval SL_STATUS_OK if successful otherwise error code.
 *****************************************************************************/
sl_status_t ble_peer_manager_central_open_connection(bd_addr *address,
                                                     uint8_t address_type);

/**************************************************************************//**
 * Close connection.
 *
 * @param[in] conn_handle Connection handle of the connection
 *            which needs to be closed.
 *
 * @retval SL_STATUS_OK if successful otherwise error code.
 * @retval SL_STATUS_NOT_FOUND if the connection handle is not found.
 *****************************************************************************/
sl_status_t ble_peer_manager_central_close_connection(uint8_t conn_handle);
/** @} (end addtogroup ble_peer_manager_central) */
#endif // BLE_PEER_MANAGER_CENTRAL_H
