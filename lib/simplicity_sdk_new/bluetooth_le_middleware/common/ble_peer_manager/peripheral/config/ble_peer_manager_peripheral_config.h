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
#ifndef BLE_PEER_MANAGER_PERIPHERAL_CONFIG_H
#define BLE_PEER_MANAGER_PERIPHERAL_CONFIG_H

/***********************************************************************************************//**
 * @addtogroup ble_peer_manager_peripheral
 * @{
 **************************************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <q BLE_PEER_MANAGER_PERIPHERAL_CONFIG_DEFAULT_ADV_DISCOVERY_MODE> Advertising discovery mode
// <i> Discovery mode for advertiser.
// <sl_bt_advertiser_non_discoverable=> Not discoverable
// <sl_bt_advertiser_limited_discoverable=> Discoverable by both limited and general discovery procedures
// <sl_bt_advertiser_general_discoverable=> Discoverable by the general discovery procedure
// <i> Default: Discoverable by the general discovery procedure
#ifndef BLE_PEER_MANAGER_PERIPHERAL_CONFIG_DEFAULT_ADV_DISCOVERY_MODE
#define BLE_PEER_MANAGER_PERIPHERAL_CONFIG_DEFAULT_ADV_DISCOVERY_MODE         sl_bt_advertiser_general_discoverable
#endif

// <q BLE_PEER_MANAGER_PERIPHERAL_CONFIG_DEFAULT_ADV_CONNECTION_MODE> Advertiser connection mode
// <i> Connection mode for advertiser.
// <sl_bt_legacy_advertiser_non_connectable=> Undirected non-connectable and non-scannable legacy advertising
// <sl_bt_legacy_advertiser_connectable=> Undirected connectable and scannable legacy advertising
// <sl_bt_legacy_advertiser_scannable=> Undirected scannable and non-connectable legacy advertising
// <i> Default: Undirected connectable and scannable legacy advertising
#ifndef BLE_PEER_MANAGER_PERIPHERAL_CONFIG_DEFAULT_ADV_CONNECTION_MODE
#define BLE_PEER_MANAGER_PERIPHERAL_CONFIG_DEFAULT_ADV_CONNECTION_MODE        sl_bt_legacy_advertiser_connectable
#endif

// <o BLE_PEER_MANAGER_PERIPHERAL_CONFIG_ADV_TIMING_INTERVAL_MIN>  Minimum advertising interval <32..16777>
// <i> Value in units of 0.625 ms
// <i> Range: 0x20 to 0xFFFFFF
// <i> Time range: 20 ms to 10485.759375 s
// <i> Default value: 100 ms
#ifndef BLE_PEER_MANAGER_PERIPHERAL_CONFIG_ADV_TIMING_INTERVAL_MIN
#define BLE_PEER_MANAGER_PERIPHERAL_CONFIG_ADV_TIMING_INTERVAL_MIN            160
#endif

// <o BLE_PEER_MANAGER_PERIPHERAL_CONFIG_ADV_TIMING_INTERVAL_MAX> Maximum advertising interval <32..16777>
// <i> Value in units of 0.625 ms
// <i> Range: 0x20 to 0xFFFFFF
// <i> Time range: 20 ms to 10485.759375 s
// <i> Default value: 100 ms
#ifndef BLE_PEER_MANAGER_PERIPHERAL_CONFIG_ADV_TIMING_INTERVAL_MAX
#define BLE_PEER_MANAGER_PERIPHERAL_CONFIG_ADV_TIMING_INTERVAL_MAX            160
#endif

// <o BLE_PEER_MANAGER_PERIPHERAL_CONFIG_ADV_TIMING_DURATION> Advertising duration <0..65535>
// <i> Advertising duration for this advertising set. Value 0 indicates no
// <i> advertising duration limit and advertising continues until it is disabled.
// <i> A non-zero value sets the duration in units of 10 ms. The duration begins
// <i> at the start of the first advertising event of this advertising set.
// <i> Range: 0x0001 to 0xFFFF
// <i> Time range: 10 ms to 655.35 s
// <i> Default value: 0
#ifndef BLE_PEER_MANAGER_PERIPHERAL_CONFIG_ADV_TIMING_DURATION
#define BLE_PEER_MANAGER_PERIPHERAL_CONFIG_ADV_TIMING_DURATION                0
#endif

// <o BLE_PEER_MANAGER_PERIPHERAL_CONFIG_ADV_TIMING_MAX_EVENTS> Maximum number of advertising events <0..100>
// <i> If non-zero, indicates the maximum number of advertising events to send
// <i> before the advertiser is stopped. Value 0 indicates no maximum number limit.
// <i> Default value: 0
#ifndef BLE_PEER_MANAGER_PERIPHERAL_CONFIG_ADV_TIMING_MAX_EVENTS
#define BLE_PEER_MANAGER_PERIPHERAL_CONFIG_ADV_TIMING_MAX_EVENTS              0
#endif

// <q BLE_PEER_MANAGER_PERIPHERAL_CONFIG_ADV_PHY> Advertiser PHY
// <sl_bt_gap_phy_1m=> 1Mbps PHY
// <sl_bt_gap_phy_2m=> 2Mbps PHY
// <i> Default: 1Mbps PHY
#ifndef BLE_PEER_MANAGER_PERIPHERAL_CONFIG_ADV_PHY
#define BLE_PEER_MANAGER_PERIPHERAL_CONFIG_ADV_PHY                            sl_bt_gap_phy_1m
#endif

// <<< end of configuration section >>>

/** @} (end addtogroup ble_peer_manager_peripheral) */
#endif // BLE_PEER_MANAGER_PERIPHERAL_CONFIG_H
