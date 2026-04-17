/***************************************************************************//**
 * @file sl_wisun_br_config.h
 * @brief Wi-SUN Border Router configuration
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
#ifndef SL_WISUN_BR_CONFIG_H
#define SL_WISUN_BR_CONFIG_H

#include "sl_component_catalog.h"

#if defined(SL_CATALOG_WISUN_BR_WIFI_PRESENT)
#include "sl_wisun_br_wifi.h"
#endif

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Wi-SUN BR configuration

// <o SL_WISUN_BR_CONFIG_UC_DWELL_INTERVAL> Unicast Dwell Interval
// <i> Default: 255
// <i> Unicast dwell interval in milliseconds.
#define SL_WISUN_BR_CONFIG_UC_DWELL_INTERVAL  255

// <o SL_WISUN_BR_CONFIG_BC_INTERVAL> Broadcast Channel Interval
// <i> Default: 1020
// <i> Broadcast interval in milliseconds.
#define SL_WISUN_BR_CONFIG_BC_INTERVAL  1020

// <o SL_WISUN_BR_CONFIG_BC_DWELL_INTERVAL> Broadcast Dwell Interval
// <i> Default: 255
// <i> Broadcast dwell interval in milliseconds.
#define SL_WISUN_BR_CONFIG_BC_DWELL_INTERVAL  255

// <o SL_WISUN_BR_CONFIG_MAX_NEIGHBOR_COUNT> Max Neighbor Count
// <i> Default: 32
// <i> Size of the neighbor table, including temporary entries and RPL parents.
#define SL_WISUN_BR_CONFIG_MAX_NEIGHBOR_COUNT 32

// <o SL_WISUN_BR_CONFIG_MAX_CHILD_COUNT> Max Child Count
// <i> Default: 22
// <i> Maximum number of RPL children.
#define SL_WISUN_BR_CONFIG_MAX_CHILD_COUNT 22

// <o SL_WISUN_BR_CONFIG_MAX_SECURITY_NEIGHBOR_COUNT> Max Security Neighbor Count
// <i> Default: 300
// <i> Size of the security neighbor table.
#define SL_WISUN_BR_CONFIG_MAX_SECURITY_NEIGHBOR_COUNT 300

// <o SL_WISUN_BR_CONFIG_KEYCHAIN> Keychain
// <SL_WISUN_KEYCHAIN_AUTOMATIC=> Automatic
// <SL_WISUN_KEYCHAIN_BUILTIN=> Built-in
// <SL_WISUN_KEYCHAIN_NVM=> NVM
// <i> Default: SL_WISUN_KEYCHAIN_AUTOMATIC
// <i> Keychain to use for device credentials.
#define SL_WISUN_BR_CONFIG_KEYCHAIN SL_WISUN_KEYCHAIN_AUTOMATIC

// <o SL_WISUN_BR_CONFIG_KEYCHAIN_INDEX> Keychain Index
// <i> Default: 0
// <i> Device credential index to use for built-in keychain.
#define SL_WISUN_BR_CONFIG_KEYCHAIN_INDEX 0

// <o SL_WISUN_BR_CONFIG_SOCKET_RX_BUFFER_SIZE> Socket RX Buffer Size
// <i> Default: 2048
// <i> Socket receiver buffer size for ICMP echo requests.
#define SL_WISUN_BR_CONFIG_SOCKET_RX_BUFFER_SIZE 2048

// <s SL_WISUN_BR_CONFIG_IPV6_PREFIX> IPv6 Prefix
// <i> Default: "fd12:3456::/64"
// <i> IPv6 prefix for DODAG.
#define SL_WISUN_BR_CONFIG_IPV6_PREFIX "fd12:3456::/64"

#if defined(SL_CATALOG_WISUN_BR_WIFI_PRESENT)
// <s SL_WISUN_BR_CONFIG_WIFI_DEFAULT_SSID> Wi-Fi SSID
// <i> Default: "DEFAULT_SSID"
// <i> Wi-Fi SSID for the border router.
#define SL_WISUN_BR_CONFIG_WIFI_DEFAULT_SSID "DEFAULT_SSID"

// <s SL_WISUN_BR_CONFIG_WIFI_DEFAULT_PASSPHRASE> Wi-Fi Passphrase
// <i> Default: "DEFAULT_PASSPHRASE"
// <i> Wi-Fi passphrase for the border router.
#define SL_WISUN_BR_CONFIG_WIFI_DEFAULT_PASSPHRASE "DEFAULT_PASSPHRASE"

// <s SL_WISUN_BR_CONFIG_WIFI_DEFAULT_SECURITY_TYPE> Wi-Fi Security Type
// <i> Default: "WPA2-CCMP"
// <i> Wi-Fi security type for the border router.
#define SL_WISUN_BR_CONFIG_WIFI_DEFAULT_SECURITY_TYPE WIFI_SECURITY_TYPE_WPA2_CCMP
#endif

// </h>

// <<< end of configuration section >>>

#endif // SL_WISUN_BR_CONFIG_H
