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
#ifndef BLE_PEER_MANAGER_CENTRAL_CONFIG_H
#define BLE_PEER_MANAGER_CENTRAL_CONFIG_H

/***********************************************************************************************//**
 * @addtogroup ble_peer_manager_central
 * @{
 **************************************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <q BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCANNING_PHY> Scanning PHY
// <sl_bt_gap_phy_coding_1m_uncoded=> 1M PHY
// <sl_bt_gap_phy_coding_2m_uncoded=> 2M PHY
// <sl_bt_gap_phy_coding_125k_coded=> 125k Coded PHY
// <sl_bt_gap_phy_coding_500k_coded=> 500k Coded PHY
// <i> Default value: 1M PHY
#ifndef BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCANNING_PHY
#define BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCANNING_PHY            sl_bt_gap_phy_coding_1m_uncoded
#endif

// <q BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_DISCOVERY_MODE> Discovery mode
// <sl_bt_scanner_discover_limited=> Discover only limited discoverable devices.
// <sl_bt_scanner_discover_generic=> Discover limited and general discoverable devices.
// <sl_bt_scanner_discover_observation=> Discover non-discoverable, limited and general discoverable devices
// <i> Default value: Discover limited and general discoverable devices.
#ifndef BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_DISCOVERY_MODE
#define BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_DISCOVERY_MODE     sl_bt_scanner_discover_generic
#endif

// <q BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_MODE> Passive or active scan
// <sl_bt_scanner_scan_mode_passive=> Passive scanning mode
// <sl_bt_scanner_scan_mode_active=> Active scanning mode
// <i> Default value: Passive scanning mode.
#ifndef BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_MODE
#define BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_MODE               sl_bt_scanner_scan_mode_passive
#endif

// <o BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_INTERVAL> Scan interval <4..65>
// <i> Time = Value x 0.625 ms
// <i> Range: 0x0004 to 0xFFFF
// <i> Time Range: 2.5 ms to 40.96 s
// <i> Default value: 10 ms
#ifndef BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_INTERVAL
#define BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_INTERVAL           16
#endif

// <o BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_WINDOW> Scan window <4..65>
// <i> Time = Value x 0.625 ms
// <i> Range: 0x0004 to 0xFFFF
// <i> Time Range: 2.5 ms to 40.96 s
// <i> Default value: 10 ms
#ifndef BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_WINDOW
#define BLE_PEER_MANAGER_CENTRAL_CONFIG_DEFAULT_SCAN_WINDOW             16
#endif

// <<< end of configuration section >>>

/** @} (end addtogroup ble_peer_manager_central) */
#endif // BLE_PEER_MANAGER_CENTRAL_CONFIG_H
