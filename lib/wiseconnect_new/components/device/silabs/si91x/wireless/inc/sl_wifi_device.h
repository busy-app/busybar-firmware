/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#pragma once

#include "sl_si91x_status.h"
#include "sl_si91x_protocol_types.h"
#include "sl_constants.h"
#include "sl_bit.h"
#include "sl_wifi_types.h"
#include <stdint.h>
#include <stddef.h>

//! @cond Doxygen_Suppress
#ifndef BIT
#define BIT(a) ((uint32_t)1U << a)
#endif

#define NETWORK_INTERFACE_VALID(x)                                                                                    \
  (x == SL_NET_WIFI_CLIENT_1_INTERFACE) || (x == SL_NET_WIFI_CLIENT_2_INTERFACE) || (x == SL_NET_WIFI_AP_1_INTERFACE) \
    || (x == SL_NET_WIFI_AP_2_INTERFACE)

// NOTE: The value for SL_SI91X_SI917_RAM_MEM_CONFIG will be fetched from respective si91x_mem_config_1/2/3.slcc
#ifdef SLI_SI91X_MCU_INTERFACE
#if SL_SI91X_SI917_RAM_MEM_CONFIG == 1
#define MEMORY_CONFIG SL_SI91X_RAM_LEVEL_NWP_ADV_MCU_BASIC
#elif SL_SI91X_SI917_RAM_MEM_CONFIG == 2
#define MEMORY_CONFIG SL_SI91X_RAM_LEVEL_NWP_MEDIUM_MCU_MEDIUM
#elif SL_SI91X_SI917_RAM_MEM_CONFIG == 3
#define MEMORY_CONFIG SL_SI91X_RAM_LEVEL_NWP_BASIC_MCU_ADV
#endif
#else
#define MEMORY_CONFIG (BIT(20) | BIT(21))
#endif

/** \addtogroup WIFI_FEATURE_BITMAP
 * @{ */
/*=========================================================================*/
// Feature bitmap parameters description !//
/*=========================================================================*/
/**
  * @def SL_WIFI_FEAT_SECURITY_OPEN
  * @brief Security type: Open.
  * @details
  * This feature supports open security type in client mode.
  * 
  * @note It is recommended to enable this macro to configure the security type as open mode.
  */
#define SL_WIFI_FEAT_SECURITY_OPEN BIT(0)

/**
   * @def SL_WIFI_FEAT_SECURITY_PSK
   * @brief Security type: WPA/WPA2.
   * @details
   * This feature supports WPA/WPA2 security type in client mode.
   * 
   * @note It is recommended to enable this macro to configure the security type as WPA, WPA2, or any other security modes.
   */
#define SL_WIFI_FEAT_SECURITY_PSK BIT(1)

/**
   * @def SL_WIFI_FEAT_AGGREGATION
   * @brief Aggregation support.
   * @details
   * Enables support for packet aggregation.
   * 
   * @note Supports AMPDU for both TX and RX.
   */
#define SL_WIFI_FEAT_AGGREGATION BIT(2)

/**
   * @def SL_WIFI_FEAT_WPS_DISABLE
   * @brief Disable WPS in AP mode.
   * @details
   * Disables Wi-Fi Protected Setup (WPS) functionality in Client and Access Point (AP) mode.
   */
#define SL_WIFI_FEAT_WPS_DISABLE BIT(7)

/**
   * @def SL_WIFI_FEAT_EAP_LEAP_IN_COEX
   * @brief Enable EAP-LEAP mode.
   * @details
   * Enables Extensible Authentication Protocol - Lightweight Extensible Authentication Protocol (EAP-LEAP) in coexistence mode.
   */
#define SL_WIFI_FEAT_EAP_LEAP_IN_COEX BIT(8)

/**
   * @def SL_WIFI_FEAT_HIDE_PSK_CREDENTIALS
   * @brief Hide sensitive credentials.
   * @details
   * Hides sensitive information such as Pre-Shared Key (PSK), Pairwise Master Key (PMK), and EAP credentials.
   */
#define SL_WIFI_FEAT_HIDE_PSK_CREDENTIALS BIT(9)

/**
   * @def SL_WIFI_FEAT_TRANSCEIVER_MAC_PEER_DS_SUPPORT
   * @brief MAC layer peer information storage.
   * @details
   * Enables support to store peer information in the MAC layer in Transceiver mode of operation.
   */
#define SL_WIFI_FEAT_TRANSCEIVER_MAC_PEER_DS_SUPPORT BIT(13)

/**
   * @def SL_WIFI_FEAT_DISABLE_11AX_SUPPORT
   * @brief Disable 11ax connections.
   * @details
   * Force DUT connection (in station mode) to use 11n, disabling 11ax connections.
   */
#define SL_WIFI_FEAT_DISABLE_11AX_SUPPORT BIT(15)

/**
   * @def SL_WIFI_FEAT_SOCKET_CMDS_ALLOW_BEFORE_WLAN_CONNECTION
   * @brief Allow socket commands before establishing a WLAN connection.
   * @details
   * This feature, when enabled, permits the execution of socket commands even if the Wi-Fi connection has not been established.
   * 
   * @note If this feature is disabled, issuing socket commands before the device obtains an IP address will result in an invalid state error.
   */
#define SL_WIFI_FEAT_SOCKET_CMDS_ALLOW_BEFORE_WLAN_CONNECTION BIT(18)

/**
 * @def SL_WIFI_FEAT_SECURE_ATTESTATION
 * @brief Secure attestation.
 * @details
 * Enables secure attestation functionality.
 * 
 * @note Bit(16) is used internally by SDK. Bits 17-29 and bit 31 are reserved.
 */
#define SL_WIFI_FEAT_SECURE_ATTESTATION BIT(30)

/** @} */

/** \addtogroup WIFI_CUSTOM_FEATURE_BITMAP
    * @{ */
/*=========================================================================*/
// Custom feature bitmap parameters description !//
/*=========================================================================*/
/**
   * @def SL_WIFI_CUSTOM_FEAT_DISABLE_GATEWAY_IN_RSI_AP
   * @brief Disables gateway configuration sent to STA from RSI AP.
   * @details If this bit is set to 1, the DHCP server behavior changes when the device is in Access Point (AP) mode.
   * The DHCP server assigns IP addresses to client nodes without sending out a Gateway address, providing only the assigned IP and Subnet values.
   * It is highly recommended to keep this value at '0' for standard AP functionality,
   * because disabling the gateway address is typically needed only for very specialized use cases. The default value of this bit is '0'.
   * 
   * @note Bits 0 - 1 are reserved.
   */
#define SL_WIFI_CUSTOM_FEAT_DISABLE_GATEWAY_IN_RSI_AP BIT(2)

/**
   * @def SL_WIFI_CUSTOM_FEAT_AP_IN_HIDDEN_MODE
   * @brief Configures the Access Point (AP) to operate in hidden mode.
   * @details If this bit is set, the AP is created in a hidden mode where its SSID is not broadcasted, making the AP less visible to clients.
   * This feature is valid only when the device is in AP mode.
   */
#define SL_WIFI_CUSTOM_FEAT_AP_IN_HIDDEN_MODE BIT(5)

/**
   * @def SL_WIFI_CUSTOM_FEAT_DFS_CHANNEL_SUPPORT
   * @brief Enables scanning of DFS channels in the 5 GHz band.
   * @details This bit enables the support for scanning Dynamic Frequency Selection (DFS) channels in the 5 GHz band.
   * It is applicable in Wi-Fi client mode only. Ensure to set the region configuration before scanning DFS channels.
   * 
   * @note Bit 7 is reserved.
   * @note 5 GHz is not supported in SI917.
   */
#define SL_WIFI_CUSTOM_FEAT_DFS_CHANNEL_SUPPORT BIT(8)

/**
   * @def SL_WIFI_CUSTOM_FEAT_ASYNC_CONNECTION_STATUS
   * @brief Enables asynchronous WLAN connection status indication to the host.
   * @details If this bit is enabled, the module will asynchronously notify the host of WLAN connection status changes.
   * This feature is valid in Wi-Fi client mode only.
   */
#define SL_WIFI_CUSTOM_FEAT_ASYNC_CONNECTION_STATUS BIT(10)

/**
   * @def SL_WIFI_CUSTOM_FEAT_WAKE_ON_WIRELESS
   * @brief Enables wake-on-wireless functionality in UART mode.
   * @details This bit enables the wake-on-wireless feature when operating in UART mode, which allows the module to wake up in response to wireless events.
   * 
   * @note applicable for NCP only.
   */
#define SL_WIFI_CUSTOM_FEAT_WAKE_ON_WIRELESS BIT(11)

/**
   * @def SL_WIFI_CUSTOM_FEAT_ENABLE_AP_BLACKLIST
   * @brief Enables AP blacklisting in Station (STA) mode.
   * @details By default, the client maintains an AP blacklist to avoid specific access points.
   * Enabling this feature allows the client to bypass the AP blacklist during roaming or rejoin, if required.
   */
#define SL_WIFI_CUSTOM_FEAT_ENABLE_AP_BLACKLIST BIT(12)

/**
   * @def SL_WIFI_CUSTOM_FEAT_MAX_NUM_OF_CLIENTS
   * @brief Sets the maximum number of clients supported in AP mode.
   * @details This bit field sets the maximum number of clients that can be supported in Access Point (AP) mode.
   * The value for this field should be provided in the range specified by bits 13 - 16.
   * @param max_num_of_clients Specifies the number of clients supported: AP-only mode supports (1–8) clients, while concurrent mode supports (1–4) clients.
   */
#define SL_WIFI_CUSTOM_FEAT_MAX_NUM_OF_CLIENTS(max_num_of_clients) (max_num_of_clients << 13)

/**
   * @def SL_WIFI_CUSTOM_FEAT_ROAM_WITH_DEAUTH_OR_NULL_DATA
   * @brief Selects between de-authentication or null data (with power management bit set) for roaming.
   * @details If this bit is enabled then roam through DEAUTH, or roam through NULL.
   */
#define SL_WIFI_CUSTOM_FEAT_ROAM_WITH_DEAUTH_OR_NULL_DATA BIT(17)

/**
   * @def SL_WIFI_CUSTOM_FEAT_TRIGGER_AUTO_CONFIG
   * @brief Triggers automatic configuration.
   * @details This bit enables the auto-configuration feature, which allows the module to automatically configure itself based on predefined parameters.
   * 
   * @note Bits 18 - 19 are reserved.
   * @note Not applicable for SIWx91x.
   */
#define SL_WIFI_CUSTOM_FEAT_TRIGGER_AUTO_CONFIG BIT(20)

/**
   * @def SL_WIFI_CUSTOM_FEAT_LIMIT_PACKETS_PER_STA
   * @brief Limits the number of packets buffered per STA in Access Point (AP) mode.
   * @details In AP mode, if the bit is set, only two packets per Station (STA) would be buffered when the STA is in Power Save (PS) mode.
   * This helps manage buffer usage and ensures efficient packet handling.
   * 
   * @note Bit 21 is reserved.
   */
#define SL_WIFI_CUSTOM_FEAT_LIMIT_PACKETS_PER_STA BIT(22)

/**
   * @def SL_WIFI_CUSTOM_FEAT_REJECT_CONNECT_REQ_IMMEDIATELY
   * @brief Rejects new LTCP connection requests immediately when maximum clients are connected.
   * @details When bit is set, any new connection request for an LTCP socket would be rejected immediately if the maximum number of clients are connected already.
   * By default, such requests are maintained in a pending list until an existing client disconnects.
   * 
   * @note When BIT[26] = 0: New connection requests are held in a pending list. When BIT[26] = 1: new connection requests are immediately rejected.
   */
#define SL_WIFI_CUSTOM_FEAT_REJECT_CONNECT_REQ_IMMEDIATELY BIT(26)

/**
   * @def SL_WIFI_CUSTOM_FEAT_DUAL_BAND_ROAM_VCSAFD
   * @brief Enables dual-band roaming and VCSAFD (Virtual Channel Scan and Frequency Avoidance Detection) feature (currently not supported).
   * @details This bit enables support for dual-band roaming and VCSAFD,
   * which enhances the module’s ability to switch between different frequency bands and avoid interference.
   */
#define SL_WIFI_CUSTOM_FEAT_DUAL_BAND_ROAM_VCSAFD BIT(27)

/**
 * @def SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID
 * @brief Validates the use of extended custom feature bitmap.
 * @details The bit indicates the extended custom feature bitmap is valid.
 * If this bit is enabled then only, the features present in the extended custom feature bitmap can be used.
 */
#define SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID BIT(31)
/** @} */

/** \addtogroup WIFI_EXTENDED_CUSTOM_FEATURE_BITMAP
    * @{ */
/*=========================================================================*/

// Extended custom feature bitmap !//
/*=========================================================================*/

/**
   * @def SL_WIFI_EXT_FEAT_AP_BROADCAST_PKT_SND_BEFORE_DTIM
   * @brief Extended custom bitmap for AP Broadcast customization.
   * @details Enabling this bit configures the Access Point to send broadcast packets before the DTIM (Delivery Traffic Indication Message) interval.
   * 
   * @note If this bit is enabled, the clients connected in power save mode might miss the packet.
   */
#define SL_WIFI_EXT_FEAT_AP_BROADCAST_PKT_SND_BEFORE_DTIM BIT(4)

/**
   * @def SL_WIFI_EXT_FEAT_FCC_LOW_PWR
   * @brief Extended custom bitmap to support FCC (currently not supported).
   * @details Enabling this bit allows the device to operate in a mode that complies with FCC (Federal Communications Commission) regulations for low-power operation.
   */
#define SL_WIFI_EXT_FEAT_FCC_LOW_PWR BIT(5)

/**
   * @def SL_WIFI_EXT_FEAT_ENABLE_11R_OTA
   * @brief To enable 802.11R Over The Air Roaming (currently not supported).
   * @details Enabling this bit activates support for 802.11R (Fast BSS Transition) Over The Air Roaming, which improves the handoff experience between access points.
   * 
   * @note Resource Request Support is not present.
   * @note If both BIT[11] and BIT[16] are not enabled, the device would default to Legacy Roaming.
   */
#define SL_WIFI_EXT_FEAT_ENABLE_11R_OTA BIT(11)

/**
   * @def SL_WIFI_EXT_FEAT_IEEE_80211J
   * @brief To enable 802.11J protocol (currently not supported).
   * @details Enabling this bit activates support for the 802.11J protocol, which is used for wireless communication in Japan.
   * 
   * @note If this bit is enabled, the set region command is mandatory with the region set to Japan and the band value must be 1.
   */
#define SL_WIFI_EXT_FEAT_IEEE_80211J BIT(12)

/**
   * @def SL_WIFI_EXT_FEAT_IEEE_80211W
   * @brief To enable 802.11W protocol.
   * @details Enabling this bit activates support for the 802.11W protocol, which provides management frame protection.
   * 
   * @note This bit must be set to enable WPA3 Personal Mode and WPA3 Personal Transition mode.
   */
#define SL_WIFI_EXT_FEAT_IEEE_80211W BIT(13)

/**
   * @def SL_WIFI_EXT_FEAT_16TH_STATION_IN_AP_MODE
   * @brief To enable 16 client support in Access Point (AP) mode.
   * @details Enabling this bit allows up to 16 stations to connect to the device when it is operating in AP mode.
   * 
   * @note If this bit is enabled, up to 16 stations can connect; otherwise, a maximum of 8 stations can connect.
   */
#define SL_WIFI_EXT_FEAT_16TH_STATION_IN_AP_MODE BIT(15)

/**
   * @def SL_WIFI_EXT_FEAT_ENABLE_11R_ODS
   * @brief To enable 802.11R Over the Distribution System Roaming.
   * @details Enabling this bit activates support for 802.11R (Fast BSS Transition) Over the Distribution System Roaming, which enhances roaming performance across different access points in the distribution system.
   * 
   * @note 1. Resource request support is not present.
   * @note 2. If both BIT[11] and BIT[16] are not enabled, the device would default to Legacy Roaming.
   */
#define SL_WIFI_EXT_FEAT_ENABLE_11R_ODS BIT(16)

/**
   * @def SL_WIFI_EXT_FEAT_WOWLAN_DISABLE
   * @brief To disable the WoWLAN (Wake-on-Wireless-LAN) feature.
   * @details Enabling this bit disables the WoWLAN feature, which is used for waking the device from a low-power state through wireless network activity.
   * By default, WOW LAN is enabled to maintain backward compatibility. This provides an option to disable this feature.
   * 
   * @note This only valid in NCP mode.
   */
#define SL_WIFI_EXT_FEAT_WOWLAN_DISABLE BIT(17)

/**
   * @def SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE
   * @brief To enable low power mode in WLAN.
   * @details Enabling this bit activates low-power mode for WLAN, Active current would also be reduced.
   * As most of the code which is needed to maintain connection is kept in RAM,
   * there would be minimal execution of code from flash which in turn results in low average current.
   */
#define SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE BIT(19)
/** @} */

/** \addtogroup WIFI_JOIN_FEATURE_BIT_MAP
    * @{ */
/*=========================================================================*/
// Join feature bitmap parameters description !//
/*=========================================================================*/
/// To enable b/g only mode in station mode.
#define SL_WIFI_JOIN_FEAT_STA_BG_ONLY_MODE_ENABLE (1 << 0)

/// To take listen interval from join command.
#define SL_WIFI_JOIN_FEAT_LISTEN_INTERVAL_VALID (1 << 1)

/// To enable quick join feature.
#define SL_WIFI_JOIN_FEAT_QUICK_JOIN (1 << 2)

/// To enable CCXV2 feature.
#define SL_WIFI_JOIN_FEAT_CCXV2_FEATURE (1 << 3)

/// To connect to AP based on BSSID together with configured SSID.
#define SL_WIFI_JOIN_FEAT_BSSID_BASED (1 << 4)

/// MFP Capable only.
#define SL_WIFI_JOIN_FEAT_MFP_CAPABLE_ONLY (1 << 5)

/// MFP Capable required
#define SL_WIFI_JOIN_FEAT_MFP_CAPABLE_REQUIRED ((1 << 5) | (1 << 6))

/// Listen interval from power save command.
#define SL_WIFI_JOIN_FEAT_PS_CMD_LISTEN_INTERVAL_VALID (1 << 7)
/** @} */

/** \addtogroup WIFI_CONFIG_FEATURE_BITMAP
    * @{ */
/*=========================================================================*/
// Config feature bitmap parameters description !//
/*=========================================================================*/
/**
   * @def SL_WIFI_FEAT_EAP_TLS_V1P0
   * @brief Enables TLS version 1.0 for enterprise security.
   * 
   * @note Bit 12 -13 are reserved
   */
#define SL_WIFI_FEAT_EAP_TLS_V1P0 BIT(14)

/**
   * @def SL_WIFI_FEAT_EAP_TLS_V1P2
   * @brief Enables TLS version 1.2 for enterprise security.
   */
#define SL_WIFI_FEAT_EAP_TLS_V1P2 BIT(15)

/**
   * @def SL_WIFI_FEAT_CONC_STA_AP_DYN_SWITCH_SEL
   * @brief Configures dynamic switching between STA and AP modes.
   * @details This bit enables or disables dynamic switching between Station (STA) and Access Point (AP) modes.
   * When enabled, the system can dynamically switch between STA and AP modes based on operational requirements or network conditions.
   * This feature is useful for applications requiring flexible mode changes to optimize performance or power consumption.
   * 
   * @note Enabling this feature allows the system to switch modes dynamically, which can improve adaptability in varying network scenarios.
   * Make sure to configure the system appropriately for the desired switching behavior.
   * @note Bit 16 is reserved.
   */
#define SL_WIFI_FEAT_CONC_STA_AP_DYN_SWITCH_SEL BIT(17)

/**
   * @def SL_WIFI_FEAT_DISABLE_MCS_5_6_7_DATARATES
   * @brief Disables MCS-5, 6, and 7 data rates.
   * @details This bit is used to disable the higher MCS data rates (5, 6, and 7) for reduced data rate requirements.
   */
#define SL_WIFI_FEAT_DISABLE_MCS_5_6_7_DATARATES BIT(19)

/**
   * @def SL_WIFI_FEAT_DISABLE_SHORT_GI
   * @brief Disables Short Guard Interval (Short-GI).
   * @details This bit disables the use of Short-GI, which may affect the timing and performance of the system.
   * 
   * @note This bit applies only to 802.11n data rates. For 802.11ax data rates, disabling Short-GI is not supported.
   */
#define SL_WIFI_FEAT_DISABLE_SHORT_GI BIT(20)

/**
 * @def SL_WIFI_ENABLE_ENHANCED_MAX_PSP
 * @brief Enables Enhanced Max PSP.
 * @details Set this bit to enable the Enhanced Max PSP feature for improved performance.
 * 
 * @note Set the performance profile to ASSOCIATED_POWER_SAVE_LOW_LATENCY for the device to enter Enhanced Max PSP mode.
 */
#define SL_WIFI_ENABLE_ENHANCED_MAX_PSP BIT(26)

/** @} */

/** \addtogroup WIFI_BURN_TARGET_OPTIONS
    * @{ */
/*=========================================================================*/
// Burn target options parameters description !//
// This group defines the target options for burning data into different memory types.
//=========================================================================*/

/**
   * @def SL_WIFI_BURN_INTO_EFUSE
   * @brief Option to burn data into EFUSE.
   * 
   * @details This option specifies that the data should be burned into the EFUSE memory.
   * EFUSE memory is used for storing critical calibration or configuration data that should not be modified after programming. 
   * Use this option when permanent storage of data is required.
   */
#define SL_WIFI_BURN_INTO_EFUSE 0

/**
   * @def SL_WIFI_BURN_INTO_FLASH
   * @brief Option to burn data into flash.
   * 
   * @details This option specifies that the data should be burned into flash memory.
   * Flash memory provides non-volatile storage that can be reprogrammed.
   * This is suitable for data that might need to be updated or modified over time.
   */
#define SL_WIFI_BURN_INTO_FLASH 1
/** @} */

/** \addtogroup WIFI_CALIBRATION_FLAGS
    * @{ */
/**
   * @def SL_WIFI_BURN_GAIN_OFFSET
   * @brief Burn gain offset into the device.
   *
   * @details
   * This macro defines the bit for burning the gain offset into the device.
   */
#define SL_WIFI_BURN_GAIN_OFFSET BIT(0)

/**
   * @def SL_WIFI_BURN_FREQ_OFFSET
   * @brief Burn frequency offset into the device.
   *
   * @details
   * This macro defines the bit for burning the frequency offset into the device.
   */
#define SL_WIFI_BURN_FREQ_OFFSET BIT(1)

/**
   * @def SL_WIFI_SW_XO_CTUNE_VALID
   * @brief Indicates if the software XO CTUNE is valid.
   *
   * @details
   * This macro defines the bit to indicate that the software XO CTUNE (crystal tuning) value is valid.
   */
#define SL_WIFI_SW_XO_CTUNE_VALID BIT(2)

/**
   * @def SL_WIFI_BURN_XO_FAST_DISABLE
   * @brief Burn bit to disable XO fast into the device.
   *
   * @details
   * This macro defines the bit for burning a setting to disable the fast XO (crystal oscillator) into the device.
   * 
   * @note Not applicable for SiWx91x.
   */
#define SL_WIFI_BURN_XO_FAST_DISABLE BIT(3)

/** @} */

/** \addtogroup WIFI_DTIM_ALIGNMENT_TYPES
    * @{ */
/// Module wakes up at beacon which is just before or equal to listen_interval
#define SL_WIFI_ALIGN_WITH_BEACON 0
/// Module wakes up at DTIM beacon which is just before or equal to listen_interval
#define SL_WIFI_ALIGN_WITH_DTIM_BEACON 1
/** @} */
//! @endcond

/** \addtogroup SI91X_FEATURE_BITMAP
  * @{ */
/*=========================================================================*/
// feature bitmap parameters description !//
/*=========================================================================*/
/**
 * @def SL_SI91X_FEAT_SECURITY_OPEN
 * @brief Security type: Open.
 * @details
 * This feature supports open security type in client mode.
 * 
 * @note It is recommended to enable this macro to configure the security type as open mode.
 */
#define SL_SI91X_FEAT_SECURITY_OPEN SL_WIFI_FEAT_SECURITY_OPEN

/**
 * @def SL_SI91X_FEAT_SECURITY_PSK
 * @brief Security type: WPA/WPA2.
 * @details
 * This feature supports WPA/WPA2 security type in client mode.
 * 
 * @note It is recommended to enable this macro to configure the security type as WPA, WPA2, or any other security modes.
 */
#define SL_SI91X_FEAT_SECURITY_PSK SL_WIFI_FEAT_SECURITY_PSK

/**
 * @def SL_SI91X_FEAT_AGGREGATION
 * @brief Aggregation support.
 * @details
 * Enables support for packet aggregation.
 * 
 * @note Supports AMPDU for both TX and RX.
 */
#define SL_SI91X_FEAT_AGGREGATION SL_WIFI_FEAT_AGGREGATION

/**
 * @def SL_SI91X_FEAT_LP_GPIO_BASED_HANDSHAKE
 * @brief Low Power (LP) mode GPIO handshake.
 * @details
 * Enables GPIO-based handshake for low-power mode.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_FEAT_LP_GPIO_BASED_HANDSHAKE BIT(3)

/**
 * @def SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
 * @brief Ultra Low Power (ULP) mode GPIO handshake.
 * @details
 * Enables GPIO-based handshake for ultra-low-power mode.
 * 
 * @note Not applicable for SoC
 */
#define SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE BIT(4)

/**
 * @def SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1
 * @brief ULP GPIO 1 wake-up indication.
 * @details
 * Configures ULP GPIO 1 for wake-up indication.
 */
#define SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1 BIT(5)

/**
 * @def SL_SI91X_FEAT_RF_SUPPLY_VOL_3_3_VOLT
 * @brief 3.3V RF supply.
 * @details
 * Configures the device to use a 3.3 V power supply for RF.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_FEAT_RF_SUPPLY_VOL_3_3_VOLT BIT(6)

/**
 * @def SL_SI91X_FEAT_WPS_DISABLE
 * @brief Disable WPS in AP mode.
 * @details
 * Disables Wi-Fi Protected Setup (WPS) functionality in Client and Access Point (AP) mode.
 */
#define SL_SI91X_FEAT_WPS_DISABLE SL_WIFI_FEAT_WPS_DISABLE

/**
 * @def SL_SI91X_FEAT_EAP_LEAP_IN_COEX
 * @brief Enable EAP-LEAP mode.
 * @details
 * Enables Extensible Authentication Protocol - Lightweight Extensible Authentication Protocol (EAP-LEAP) in coexistence mode.
 */
#define SL_SI91X_FEAT_EAP_LEAP_IN_COEX SL_WIFI_FEAT_EAP_LEAP_IN_COEX

/**
 * @def SL_SI91X_FEAT_HIDE_PSK_CREDENTIALS
 * @brief Hide sensitive credentials.
 * @details
 * Hides sensitive information such as Pre-Shared Key (PSK), Pairwise Master Key (PMK), and EAP credentials.
 */
#define SL_SI91X_FEAT_HIDE_PSK_CREDENTIALS SL_WIFI_FEAT_HIDE_PSK_CREDENTIALS

/**
 * @def SL_SI91X_FEAT_SSL_HIGH_STREAMING_BIT
 * @brief High SSL streaming throughput.
 * @details
 * Enables high throughput for Secure Sockets Layer (SSL) streaming.
 */
#define SL_SI91X_FEAT_SSL_HIGH_STREAMING_BIT BIT(10)

/**
 * @def SL_SI91X_FEAT_LONG_ICMP_PACKET
 * @brief Support for long-sized ICMP packets.
 * @details
 * Enables support for long-sized Internet Control Message Protocol (ICMP) packets.
 * Enable this bit to receive and process the ICMP requests of size more than 308 bytes sent from peers.
 * A maximum packet size of 1472 bytes can be processed for ICMP and 1452 bytes for ICMPv6.
 * The size limit for sending ICMP/ICMPv6 packets to peers using the sl_si91x_send_ping API remains restricted to 300 bytes.
 * 
 * @note Bit 11 is reserved.
 */
#define SL_SI91X_FEAT_LONG_ICMP_PACKET BIT(12)

/**
 * @def SL_SI91X_FEAT_TRANSCEIVER_MAC_PEER_DS_SUPPORT
 * @brief MAC layer peer information storage.
 * @details
 * Enables support to store peer information in the MAC layer in Transceiver mode of operation.
 */
#define SL_SI91X_FEAT_TRANSCEIVER_MAC_PEER_DS_SUPPORT SL_WIFI_FEAT_TRANSCEIVER_MAC_PEER_DS_SUPPORT

/**
 * @def SL_SI91X_FEAT_LONG_HTTP_URL
 * @brief Support for long HTTP GET and OTAF URLs.
 * @details
 * Enables support for long HTTP GET and OTAF URLs. The maximum URL length supported is 2048 bytes.
 */
#define SL_SI91X_FEAT_LONG_HTTP_URL BIT(14)

/**
 * @def SL_SI91X_FEAT_DISABLE_11AX_SUPPORT
 * @brief Disable 11ax connections.
 * @details
 * Force DUT connection (in station mode) to use 11n, disabling 11ax connections.
 */
#define SL_SI91X_FEAT_DISABLE_11AX_SUPPORT SL_WIFI_FEAT_DISABLE_11AX_SUPPORT
/** @} */

/**
 * @def SLI_SI91X_FEAT_FW_UPDATE_NEW_CODE
 * @brief Indicates support for a new set of firmware update result codes. This bit is used for internal purpose.
 * @details
 * This bit in the feature bitmap is used to inform the NWP firmware whether
 * the host supports a new set of result codes to differentiate firmware update
 * results from other non-firmware-related results. If this bit is set,
 * the NWP firmware would send result codes from the new set after a firmware update.
 * If the bit is not set, the legacy result codes would be used.
 */
#define SLI_SI91X_FEAT_FW_UPDATE_NEW_CODE BIT(16)

/** \addtogroup SI91X_FEATURE_BITMAP
  * @{ */
/**
 * @def SL_SI91X_FEAT_SECURE_ATTESTATION
 * @brief Secure attestation.
 * @details
 * Enables secure attestation functionality.
 * 
 * @note Bit(16) is used internally by SDK. Bits 17-29 and bit 31 are reserved.
 */
#define SL_SI91X_FEAT_SECURE_ATTESTATION SL_WIFI_FEAT_SECURE_ATTESTATION
/** @} */

/** \addtogroup SI91X_TCP_IP_FEATURE_BITMAP
  * @{ */
/*=========================================================================*/
// TCP/IP feature bitmap parameters description !//
/*=========================================================================*/
/**
 * @def SL_SI91X_TCP_IP_FEAT_BYPASS
 * @brief Enables the TCP/IP bypass feature.
 *
 * @details
 * When this feature is enabled, the TCP/IP stack processing is bypassed,
 * allowing raw Ethernet frames to be sent and received. This is useful
 * for applications that require direct control over Ethernet frames or
 * for implementing custom network protocols.
 *
 * This is defined as a bit flag that can be set in the feature configuration
 * to enable the TCP/IP bypass.
 */

#define SL_SI91X_TCP_IP_FEAT_BYPASS BIT(0)

/// @note Bit 1 is reserved

/**
 * @def SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT
 * @brief Enables the DHCPv4 client feature.
 *
 * @details
 * This feature allows the device to obtain an IPv4 address,
 * subnet mask, default gateway, and DNS server from a DHCP server.
 */
#define SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT BIT(2)

/**
 * @def SL_SI91X_TCP_IP_FEAT_DHCPV6_CLIENT
 * @brief Enables the DHCPv6 client feature.
 *
 * @details
 * This feature allows the device to obtain an IPv6 address
 * and other network configuration details from a DHCPv6 server.
 */
#define SL_SI91X_TCP_IP_FEAT_DHCPV6_CLIENT BIT(3)

/**
 * @def SL_SI91X_TCP_IP_FEAT_DHCPV4_SERVER
 * @brief Enables the DHCPv4 server feature.
 *
 * @details
 * This feature allows the device to act as a DHCPv4 server, which provides IPv4
 * addresses and network configuration to DHCPv4 clients on the network.
 */
#define SL_SI91X_TCP_IP_FEAT_DHCPV4_SERVER BIT(4)

/**
 * @def SL_SI91X_TCP_IP_FEAT_DHCPV6_SERVER
 * @brief Enables the DHCPv6 server feature.
 *
 * @details
 * This feature allows the device to act as a DHCPv6 server, which provides IPv6
 * addresses and network configuration to DHCPv6 clients on the network.
 */
#define SL_SI91X_TCP_IP_FEAT_DHCPV6_SERVER BIT(5)

/**
 * @def SL_SI91X_TCP_IP_FEAT_JSON_OBJECTS
 * @brief Enables support for JSON objects.
 *
 * @details
 * This feature allows the device to handle JSON (JavaScript Object Notation)
 * objects, which can be used for data interchange in web applications.
 */
#define SL_SI91X_TCP_IP_FEAT_JSON_OBJECTS BIT(6)

/**
 * @def SL_SI91X_TCP_IP_FEAT_HTTP_CLIENT
 * @brief Enables the HTTP client feature.
 *
 * @details
 * This feature allows the device to send HTTP requests and receive HTTP
 * responses from web servers, enabling web-based communication.
 */
#define SL_SI91X_TCP_IP_FEAT_HTTP_CLIENT BIT(7)

/**
 * @def SL_SI91X_TCP_IP_FEAT_DNS_CLIENT
 * @brief Enables the DNS client feature.
 *
 * @details
 * This feature allows the device to resolve domain names to IP addresses
 * using the Domain Name System (DNS), enabling communication with servers
 * by hostname.
 */
#define SL_SI91X_TCP_IP_FEAT_DNS_CLIENT BIT(8)

/**
 * @def SL_SI91X_TCP_IP_FEAT_SNMP_AGENT
 * @brief Enables the SNMP agent feature.
 *
 * @details
 * This feature allows the device to act as an SNMP (Simple Network Management
 * Protocol) agent, which enables network management and monitoring.
 */
#define SL_SI91X_TCP_IP_FEAT_SNMP_AGENT BIT(9)

/**
 * @def SL_SI91X_TCP_IP_FEAT_SSL
 * @brief Enables the SSL feature.
 *
 * @details
 * This feature allows the device to use SSL (Secure Sockets Layer) for secure
 * communication over the network, that provides encryption and authentication.
 */
#define SL_SI91X_TCP_IP_FEAT_SSL BIT(10)

/**
 * @def SL_SI91X_TCP_IP_FEAT_ICMP
 * @brief Enables the ICMP feature (ping).
 *
 * @details
 * This feature allows the device to use ICMP (Internet Control Message
 * Protocol) for network diagnostics, such as sending ping requests.
 */
#define SL_SI91X_TCP_IP_FEAT_ICMP BIT(11)

/// @note Bit 12 and  13 are reserved.

/**
 * @def SL_SI91X_TCP_IP_FEAT_SEND_CONFIGS_TO_HOST
 * @brief Enables sending configuration data to the host.
 *
 * @details
 * This feature allows the device to send web page configuration data to the
 * host system from the wireless configuration page.
 */
#define SL_SI91X_TCP_IP_FEAT_SEND_CONFIGS_TO_HOST BIT(14)

/**
 * @def SL_SI91X_TCP_IP_FEAT_FTP_CLIENT
 * @brief Enables the FTP client feature.
 *
 * @details
 * This feature allows the device to act as an FTP (File Transfer Protocol)
 * client, enabling file transfers to and from FTP servers.
 */
#define SL_SI91X_TCP_IP_FEAT_FTP_CLIENT BIT(15)

/**
 * @def SL_SI91X_TCP_IP_FEAT_SNTP_CLIENT
 * @brief Enables the SNTP client feature.
 *
 * @details
 * This feature allows the device to synchronize its clock with an SNTP
 * (Simple Network Time Protocol) server, which ensures accurate timekeeping.
 */
#define SL_SI91X_TCP_IP_FEAT_SNTP_CLIENT BIT(16)

/**
 * @def SL_SI91X_TCP_IP_FEAT_IPV6
 * @brief Enables IPv6 support.
 *
 * @details
 * This feature allows the device to use IPv6 (Internet Protocol version 6),
 * which provides a larger address space and improved routing capabilities.
 * 
 * @note IPv6 would also be enabled if the DHCPv6 client or DHCPv6 server
 * feature is enabled, regardless of the tcp_ip_feature_bit_map[17] setting.
 */
#define SL_SI91X_TCP_IP_FEAT_IPV6 BIT(17)

/**
 * @def SL_SI91X_TCP_IP_FEAT_RAW_DATA
 * @brief Enables raw data support.
 *
 * @details
 * This feature allows the device to handle raw data frames, which bypass the
 * TCP/IP stack. It is supported only in AP mode and requires the TCP_BYPASS
 * feature to be disabled. If any packet from the host with frame type 0x1
 * is received by the firmware, the packet would be sent on air without
 * TCP/IP stack processing. ARP and broadcast packets (other than DHCP
 * packets) received on air would be sent to the host.
 */
#define SL_SI91X_TCP_IP_FEAT_RAW_DATA BIT(18)

/**
 * @def SL_SI91X_TCP_IP_FEAT_MDNSD
 * @brief Enables the MDNSD feature.
 *
 * @details
 * This feature allows the device to use Multicast DNS (mDNS) for local
 * network service discovery, enabling devices to find each other without
 * a central DNS server.
 * 
 * @note This feature is not supported in AP mode.
 */
#define SL_SI91X_TCP_IP_FEAT_MDNSD BIT(19)

/**
 * @def SL_SI91X_TCP_IP_FEAT_SMTP_CLIENT
 * @brief Enables the SMTP client feature.
 *
 * @details
 * This feature allows the device to act as an SMTP (Simple Mail Transfer
 * Protocol) client, that enables it to send emails.
 */
#define SL_SI91X_TCP_IP_FEAT_SMTP_CLIENT BIT(20)

/**
 * @def SL_SI91X_TCP_IP_TOTAL_SOCKETS
 * @brief Selects the number of sockets.
 *
 * @details
 * This macro allows the configuration of the total number of sockets
 * available. A maximum of 10 sockets are allowed. Bits 21-24 are used
 * to set the TOTAL_SOCKETS.
 * @param total_sockets The total number of sockets to be configured.
 */
#define SL_SI91X_TCP_IP_TOTAL_SOCKETS(total_sockets) (total_sockets << 21)

/**
 * @def SL_SI91X_TCP_IP_FEAT_SINGLE_SSL_SOCKET
 * @brief Enables a single SSL socket.
 *
 * @details
 * This feature allows the device to use a single SSL socket for secure
 * communication.
 */
#define SL_SI91X_TCP_IP_FEAT_SINGLE_SSL_SOCKET BIT(25)

/**
 * @def SL_SI91X_TCP_IP_FEAT_LOAD_PUBLIC_PRIVATE_CERTS
 * @brief Enables loading of public and private keys for TLS/SSL handshake.
 *
 * @details
 * This feature allows the device to load public and private keys for use
 * in TLS/SSL handshakes. 
 * 
 * @note If a secure handshake is to be done using only
 * a CA-certificate, then loading of private and public keys can be
 * disabled, and these certificates can be erased from the flash using
 * the load_cert API. If secure handshake verification of private and
 * public keys is needed, then loading of these keys must be enabled.
 */
#define SL_SI91X_TCP_IP_FEAT_LOAD_PUBLIC_PRIVATE_CERTS BIT(26)

/**
 * @def SL_SI91X_TCP_IP_FEAT_LOAD_CERTS_INTO_RAM
 * @brief Enables loading of SSL certificates into RAM.
 *
 * @details
 * This feature allows the device to load SSL certificates into RAM for
 * faster access during secure communications.
 */
#define SL_SI91X_TCP_IP_FEAT_LOAD_CERTS_INTO_RAM BIT(27)

/// @note Bit 28 is reserved

/**
 * @def SL_SI91X_TCP_IP_FEAT_DTLS_THREE_SOCKETS
 * @brief Enables three DTLS sockets.
 *
 * @details
 * This feature allows the device to support up to three simultaneous
 * DTLS (Datagram Transport Layer Security) connections.
 */
#define SL_SI91X_TCP_IP_FEAT_DTLS_THREE_SOCKETS BIT(29)

/**
 * @def SL_SI91X_TCP_IP_FEAT_POP3_CLIENT
 * @brief Enables the POP3 client feature.
 *
 * @details
 * This feature allows the device to act as a POP3 (Post Office Protocol
 * version 3) client, enabling it to retrieve emails from a POP3 server.
  * @note  The macro SL_SI91X_TCP_IP_FEAT_POP3_CLIENT is being deprecated and will be removed in the future. Please use SL_SI91X_TCP_IP_FEAT_DTLS_THREE_SOCKETS instead.
 */
#define SL_SI91X_TCP_IP_FEAT_POP3_CLIENT SL_SI91X_TCP_IP_FEAT_DTLS_THREE_SOCKETS

/**
 * @def SL_SI91X_TCP_IP_FEAT_OTAF
 * @brief Enables the OTAF client feature.
 *
 * @details
 * This feature allows the device to perform Over-The-Air Firmware (OTAF)
 * updates, enables it to download and install firmware updates remotely.
 */
#define SL_SI91X_TCP_IP_FEAT_OTAF BIT(30)

/**
 * @def SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID
 * @brief Enables TCP/IP extension support.
 *
 * @details
 * This feature allows the device to use extended TCP/IP features,
 * If this bit is enabled only then, the features present in the ext_tcp ip feature bitmap can be used.
 */
#define SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID BIT(31)
/** @} */

/** \addtogroup SI91X_CUSTOM_FEATURE_BITMAP
  * @{ */
/*=========================================================================*/
// Custom feature bitmap parameters description !//
/*=========================================================================*/
/**
 * @def SL_SI91X_CUSTOM_FEAT_DISABLE_GATEWAY_IN_RSI_AP
 * @brief Disables gateway configuration sent to STA from RSI AP.
 * @details If this bit is set to 1, the DHCP server behavior changes when the device is in Access Point (AP) mode.
 * The DHCP server assigns IP addresses to client nodes without sending out a Gateway address, providing only the assigned IP and Subnet values.
 * It is highly recommended to keep this value at '0' for standard AP functionality,
 * as disabling the gateway address is typically needed only for very specialized use cases. The default value of this bit is '0'.
 * 
 * @note Bits 0 - 1 are reserved.
 */
#define SL_SI91X_CUSTOM_FEAT_DISABLE_GATEWAY_IN_RSI_AP SL_WIFI_CUSTOM_FEAT_DISABLE_GATEWAY_IN_RSI_AP

/**
 * @def SL_SI91X_CUSTOM_FEAT_SOC_CLK_CONFIG_160MHZ
 * @brief Configures the clock for NWP SOC to 160 MHz.
 * @details If higher performance, such as increased throughput, is required this configuration sets the System-on-Chip (SoC) clock to 160 MHz.
 * 
 * @note Ensure to set `pll_mode` to 1 in the feature frame command for this configuration to take effect.
 * @note Bit 3 is reserved.
 * @note This configuration is NOT supported in WLAN+BLE coexistence mode.
 */
#define SL_SI91X_CUSTOM_FEAT_SOC_CLK_CONFIG_160MHZ BIT(4)

/**
 * @def SL_SI91X_CUSTOM_FEAT_AP_IN_HIDDEN_MODE
 * @brief Configures the Access Point (AP) to operate in hidden mode.
 * @details If this bit is set, the AP is created in a hidden mode where its SSID is not broadcasted, making the AP less visible to clients.
 * This feature is valid only when the device is in AP mode.
 */
#define SL_SI91X_CUSTOM_FEAT_AP_IN_HIDDEN_MODE SL_WIFI_CUSTOM_FEAT_AP_IN_HIDDEN_MODE

/**
 * @def SL_SI91X_CUSTOM_FEAT_DNS_SERVER_IN_DHCP_OFFER
 * @brief Includes DNS server IP address in DHCP offer response when in AP mode. This feature is currently not supported.
 * @details When this bit is set, the DHCP server runs in AP mode and includes the DNS server IP address in the DHCP offer response sent to clients.
 */
#define SL_SI91X_CUSTOM_FEAT_DNS_SERVER_IN_DHCP_OFFER BIT(6)

/**
 * @def SL_SI91X_CUSTOM_FEAT_DFS_CHANNEL_SUPPORT
 * @brief Enables scanning of DFS channels in the 5 GHz band.
 * @details This bit enables the support for scanning Dynamic Frequency Selection (DFS) channels in the 5 GHz band.
 * It is applicable in Wi-Fi client mode only. Ensure to set the region configuration before scanning DFS channels.
 * 
 * @note Bit 7 is reserved.
 * @note 5 GHz is not supported in SI917.
 */
#define SL_SI91X_CUSTOM_FEAT_DFS_CHANNEL_SUPPORT SL_WIFI_CUSTOM_FEAT_DFS_CHANNEL_SUPPORT

/**
 * @def SL_SI91X_CUSTOM_FEAT_LED_FEATURE
 * @brief Enables LED blinking feature to indicate network activity.
 * @details When this bit is set, the LED (GPIO_16) will blink to indicate network activity.
 * The LED blinks when a TX packet is sent or when a unicast packet addressed to the device’s MAC is received.
 * 
 * @note Not applicable for SIWx91x.
 */
#define SL_SI91X_CUSTOM_FEAT_LED_FEATURE BIT(9)

/**
 * @def SL_SI91X_CUSTOM_FEAT_ASYNC_CONNECTION_STATUS
 * @brief Enables asynchronous WLAN connection status indication to the host.
 * @details If this bit is enabled, the module will asynchronously notify the host of WLAN connection status changes.
 * This feature is valid in Wi-Fi client mode only.
 */
#define SL_SI91X_CUSTOM_FEAT_ASYNC_CONNECTION_STATUS SL_WIFI_CUSTOM_FEAT_ASYNC_CONNECTION_STATUS

/**
 * @def SL_SI91X_CUSTOM_FEAT_WAKE_ON_WIRELESS
 * @brief Enables wake-on-wireless functionality in UART mode.
 * @details This bit enables the wake-on-wireless feature when operates in UART mode, which allows the module to wake up in response to wireless events.
 * 
 * @note applicable for NCP only.
 */
#define SL_SI91X_CUSTOM_FEAT_WAKE_ON_WIRELESS SL_WIFI_CUSTOM_FEAT_WAKE_ON_WIRELESS

/**
 * @def SL_SI91X_CUSTOM_FEAT_ENABLE_AP_BLACKLIST
 * @brief Enables AP blacklisting in Station (STA) mode.
 * @details By default, the client maintains an AP blacklist to avoid specific access points.
 * Enabling this feature allows the client to bypass the AP blacklist during roaming or rejoin, if required.
 */
#define SL_SI91X_CUSTOM_FEAT_ENABLE_AP_BLACKLIST SL_WIFI_CUSTOM_FEAT_ENABLE_AP_BLACKLIST

/**
 * @def SL_SI91X_CUSTOM_FEAT_MAX_NUM_OF_CLIENTS
 * @brief Sets the maximum number of clients supported in AP mode.
 * @details This bit field sets the maximum number of clients that can be supported in Access Point (AP) mode.
 * The value for this field should be provided in the range specified by bits 13 - 16.
 * @param max_num_of_clients Specifies the number of clients supported: AP-only mode supports (1–8) clients, while concurrent mode supports (1–4) clients.
 */
#define SL_SI91X_CUSTOM_FEAT_MAX_NUM_OF_CLIENTS SL_WIFI_CUSTOM_FEAT_MAX_NUM_OF_CLIENTS(max_num_of_clients)

/**
 * @def SL_SI91X_CUSTOM_FEAT_ROAM_WITH_DEAUTH_OR_NULL_DATA
 * @brief Selects between de-authentication or null data (with power management bit set) for roaming.
 * @details If this bit is enabled then roam through DEAUTH, or roam through NULL.
 */
#define SL_SI91X_CUSTOM_FEAT_ROAM_WITH_DEAUTH_OR_NULL_DATA SL_WIFI_CUSTOM_FEAT_ROAM_WITH_DEAUTH_OR_NULL_DATA

/**
 * @def SL_SI91X_CUSTOM_FEAT_TRIGGER_AUTO_CONFIG
 * @brief Triggers automatic configuration.
 * @details This bit enables the auto-configuration feature, which allows the module to automatically configure itself based on predefined parameters.
 * 
 * @note Bits 18 - 19 are reserved.
 * @note Not applicable for SIWx91x.
 */
#define SL_SI91X_CUSTOM_FEAT_TRIGGER_AUTO_CONFIG SL_WIFI_CUSTOM_FEAT_TRIGGER_AUTO_CONFIG

/**
 * @def SL_SI91X_CUSTOM_FEAT_LIMIT_PACKETS_PER_STA
 * @brief Limits the number of packets buffered per STA in Access Point (AP) mode.
 * @details In AP mode, if the bit is set, only two packets per Station (STA) would be buffered when the STA is in Power Save (PS) mode.
 * This helps manage buffer usage and ensures efficient packet handling.
 * 
 * @note Bit 21 is reserved.
 */
#define SL_SI91X_CUSTOM_FEAT_LIMIT_PACKETS_PER_STA SL_WIFI_CUSTOM_FEAT_LIMIT_PACKETS_PER_STA

/**
 * @def SL_SI91X_CUSTOM_FEAT_HTTP_HTTPS_AUTH
 * @brief Enables HTTP/HTTPS authentication (currently not supported).
 * @details This bit enables authentication for HTTP and HTTPS connections, adding an extra layer of security for web-based communications.
 */
#define SL_SI91X_CUSTOM_FEAT_HTTP_HTTPS_AUTH BIT(23)

/**
 * @def SL_SI91X_CUSTOM_FEAT_SOC_CLK_CONFIG_120MHZ
 * @brief Configures the clock for NWP SOC to 120 MHz.
 * @details This configuration sets the System-on-Chip (SoC) clock to 120 MHz. This might requires certain performance needs.
 * Ensure to set `pll_mode` to 1 in the feature frame command for the configuration to be in effect.
 * 
 * @note This configuration is necessary for high throughput scenarios.
 */
#define SL_SI91X_CUSTOM_FEAT_SOC_CLK_CONFIG_120MHZ BIT(24)

/**
 * @def SL_SI91X_CUSTOM_FEAT_REJECT_CONNECT_REQ_IMMEDIATELY
 * @brief Rejects new LTCP connection requests immediately when maximum clients are connected.
 * @details When bit is set, any new connection request for an LTCP socket would be rejected immediately if the maximum number of clients are connected already.
 * By default, such requests are maintained in a pending list until an existing client disconnects.
 * 
 * @note When BIT[26] = 0: New connection requests are held in a pending list. When BIT[26] = 1: New connection requests are immediately rejected.
 */
#define SL_SI91X_CUSTOM_FEAT_REJECT_CONNECT_REQ_IMMEDIATELY SL_WIFI_CUSTOM_FEAT_REJECT_CONNECT_REQ_IMMEDIATELY

/**
 * @def SL_SI91X_CUSTOM_FEAT_DUAL_BAND_ROAM_VCSAFD
 * @brief Enables dual-band roaming and VCSAFD (Virtual Channel Scan and Frequency Avoidance Detection) feature (currently not supported).
 * @details This bit enables support for dual-band roaming and VCSAFD,
 * which enhances the module’s ability to switch between different frequency bands and avoid interference.
 */
#define SL_SI91X_CUSTOM_FEAT_DUAL_BAND_ROAM_VCSAFD SL_WIFI_CUSTOM_FEAT_DUAL_BAND_ROAM_VCSAFD

/**
 * @def SL_SI91X_CUSTOM_FEAT_RTC_FROM_HOST
 * @brief Enables Real-Time Clock (RTC) synchronization from the host.
 * @details When bit is set, the module uses the Real-Time Clock (RTC) provided by the host system for timekeeping.
 * 
 * @note
 *   Ensures the Real-Time Clock (RTC) timer is configured to enable certificate validation.
 */
#define SL_SI91X_CUSTOM_FEAT_RTC_FROM_HOST BIT(28)

/**
 * @def SL_SI91X_CUSTOM_FEAT_BT_IAP
 * @brief Enables Bluetooth In-App Programming (IAP) feature.
 * @details This bit enables the Bluetooth In-App Programming (IAP) feature, which allows the module to support Bluetooth-related IAP functionalities.
 */
#define SL_SI91X_CUSTOM_FEAT_BT_IAP BIT(29)

/**
 * @def SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID
 * @brief Validates the use of extended custom feature bitmap.
 * @details The bit indicates the extended custom feature bitmap is valid.
 * If this bit is enabled then only, the features present in the extended custom feature bitmap can be used.
 */
#ifndef __ZEPHYR__
#define SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID
#else
#define SL_SI91X_CUSTOM_FEAT_EXTENSION_VALID SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID
#endif
/** @} */

/** \addtogroup SI91X_EXTENDED_CUSTOM_FEATURE_BITMAP
  * @{ */
/*=========================================================================*/

// Extended custom feature bitmap !//
/*=========================================================================*/

/**
 * @def SL_SI91X_EXT_FEAT_RSA_KEY_WITH_4096_SUPPORT
 * @brief Supports 4096 size RSA KEY certificate.
 * @details Enabling this bit allows the device to support 4096-bit RSA keys. Recommended only if 4096-bit keys are required.
 * 
 * @note Bit 0 is reserved.
 */
#define SL_SI91X_EXT_FEAT_RSA_KEY_WITH_4096_SUPPORT BIT(1)

/**
 * @def SL_SI91X_EXT_FEAT_SSL_CERT_WITH_4096_KEY_SUPPORT
 * @brief Supports 4096 size KEY SSL certificate.
 * @details Enabling this bit allows the device to support SSL certificates with 4096-bit keys. Recommended only if 4096-bit keys are required.
 * 
 * @note Bit 2 is reserved.
 */
#define SL_SI91X_EXT_FEAT_SSL_CERT_WITH_4096_KEY_SUPPORT BIT(3)

/**
 * @def SL_SI91X_EXT_FEAT_AP_BROADCAST_PKT_SND_B4_DTIM
 * @brief Extended custom bitmap for AP broadcast customization.
 * @details Enabling this bit configures the Access Point to send broadcast packets before the DTIM (Delivery Traffic Indication Message) interval.
 * 
 * @note If this bit is enabled, the clients connected in power save mode might miss the packets.
 */
#define SL_SI91X_EXT_FEAT_AP_BROADCAST_PKT_SND_B4_DTIM SL_WIFI_EXT_FEAT_AP_BROADCAST_PKT_SND_BEFORE_DTIM

/**
 * @def SL_SI91X_EXT_FEAT_FCC_LOW_PWR
 * @brief Extended custom bitmap to support FCC (currently not supported).
 * @details Enabling this bit allows the device to operate in a mode that complies with Federal Communications Commission (FCC) regulations for low power operation.
 */
#define SL_SI91X_EXT_FEAT_FCC_LOW_PWR SL_WIFI_EXT_FEAT_FCC_LOW_PWR

/**
 * @def SL_SI91X_EXT_FEAT_NWP_QSPI_80MHZ_CLK_ENABLE
 * @brief To switch NWP QSPI clock to 80 MHz.
 * @details Enabling this bit switches the NWP QSPI clock from the default 40 MHz to 80 MHz. By increasing the QSPI clock speed, flash execution accelerates, resulting in improved throughput (in cases where flash execution is involved).
 */
#define SL_SI91X_EXT_FEAT_NWP_QSPI_80MHZ_CLK_ENABLE BIT(8)

/**
 * @def SL_SI91X_EXT_HTTP_SKIP_DEFAULT_LEADING_CHARACTER
 * @brief Extended feature bitmap to skip default leading character '\' in HTTP header.
 * @details Enabling this bit configures the device to omit the default leading character '\' in HTTP headers, allows for custom header formatting.
 */
#define SL_SI91X_EXT_HTTP_SKIP_DEFAULT_LEADING_CHARACTER BIT(9)

/**
 * @def SL_SI91X_EXT_FEAT_ENABLE_11R_OTA
 * @brief To enable 802.11R Over The Air Roaming (currently not supported).
 * @details Enabling this bit activates support for 802.11R (Fast BSS Transition) Over The Air Roaming, which improves the handoff experience between access points.
 * 
 * @note Resource Request Support is not present.
 * @note If both BIT[11] and BIT[16] are not enabled, the device would default to Legacy Roaming.
 */
#define SL_SI91X_EXT_FEAT_ENABLE_11R_OTA SL_WIFI_EXT_FEAT_ENABLE_11R_OTA

/**
 * @def SL_SI91X_EXT_FEAT_IEEE_80211J
 * @brief To enable 802.11J protocol (currently not supported).
 * @details Enabling this bit activates support for the 802.11J protocol, which is used for wireless communication in Japan.
 * 
 * @note If this bit is enabled, the set region command is mandatory with the region set to Japan and the band value must be 1.
 */
#define SL_SI91X_EXT_FEAT_IEEE_80211J SL_WIFI_EXT_FEAT_IEEE_80211J

/**
 * @def SL_SI91X_EXT_FEAT_IEEE_80211W
 * @brief To enable 802.11W protocol.
 * @details Enabling this bit activates support for the 802.11W protocol, which provides management frame protection.
 * 
 * @note This bit must be set to enable WPA3 Personal Mode and WPA3 Personal Transition mode.
 */
#define SL_SI91X_EXT_FEAT_IEEE_80211W SL_WIFI_EXT_FEAT_IEEE_80211W

/**
 * @def SL_SI91X_EXT_FEAT_SSL_VERSIONS_SUPPORT
 * @brief To enable the Multi-version TCP over SSL support.
 * @details Enabling this bit allows the device to support multiple versions of SSL/TLS over TCP, providing flexibility in handling different SSL/TLS versions.
 */
#define SL_SI91X_EXT_FEAT_SSL_VERSIONS_SUPPORT BIT(14)

/**
 * @def SL_SI91X_EXT_FEAT_16th_STATION_IN_AP_MODE
 * @brief To enable 16 client support in Access Point (AP) mode.
 * @details Enabling this bit allows up to 16 stations to connect to the device when it is operating in AP mode.
 * 
 * @note If this bit is enabled, up to 16 stations can connect; otherwise, a maximum of 8 stations can connect.
 */
#define SL_SI91X_EXT_FEAT_16th_STATION_IN_AP_MODE SL_WIFI_EXT_FEAT_16TH_STATION_IN_AP_MODE

/**
 * @def SL_SI91X_EXT_FEAT_ENABLE_11R_ODS
 * @brief To enable 802.11R Over the Distribution System Roaming.
 * @details Enabling this bit activates support for 802.11R (Fast BSS Transition) Over the Distribution System Roaming, which enhances roaming performance across different access points in the distribution system.
 * 
 * @note 1. Resource Request Support is not present.
 * @note 2. If both BIT[11] and BIT[16] are not enabled, the device would default to Legacy Roaming.
 */
#define SL_SI91X_EXT_FEAT_ENABLE_11R_ODS SL_WIFI_EXT_FEAT_ENABLE_11R_ODS

/**
 * @def SL_SI91X_EXT_FEAT_WOWLAN_DISABLE
 * @brief To disable the WoWLAN (Wake-on-Wireless-LAN) feature.
 * @details Enabling this bit disables the WoWLAN feature, which is used for waking the device from a low-power state through wireless network activity.
 * By default WOW LAN Is enabled to maintain backward compatibility. So given option to disable this feature.
 * 
 * @note This only valid in NCP mode.
 */
#define SL_SI91X_EXT_FEAT_WOWLAN_DISABLE SL_WIFI_EXT_FEAT_WOWLAN_DISABLE

/**
 * @def SL_SI91X_EXT_FEAT_DISABLE_XTAL_CORRECTION
 * @brief To disable auto correction of XTAL (40 MHz crystal)
 * @details Enabling this bit would disable the automatic compensation for frequency offsets, ensuring error-free calibration.
 * 
 * @note This bit should be enabled in the following cases:
 * @note 1. Always enable it in the Calibration application.
 * @note 2. Enable it for all applications for the customer hardware with an XTAL part number other than 8Y40070013.
 */
#define SL_SI91X_EXT_FEAT_DISABLE_XTAL_CORRECTION BIT(18)

/**
 * @def SL_SI91X_EXT_FEAT_LOW_POWER_MODE
 * @brief To enable low power mode in WLAN.
 * @details Enabling this bit activates low power mode for WLAN, Active current would also be reduced.
 * As most of the code which is needed to maintain connection is kept in RAM.
 * There would be minimal execution of code from flash which in turn results in low average current.
 */
#define SL_SI91X_EXT_FEAT_LOW_POWER_MODE SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE

#if defined(SLI_SI917) || defined(DOXYGEN)

// For SoC
#if defined(SLI_SI91X_MCU_INTERFACE) || defined(DOXYGEN)
/**
 * @def SL_SI91X_EXT_FEAT_352K_M4SS_320K
 * @brief To enable 352K memory for NWP and 320K memory for M4.
 * @details This configuration allocates 352K memory to the Network Processor (NWP) and 320K memory to the M4 core.
 */
#define SL_SI91X_EXT_FEAT_352K_M4SS_320K 0

/**
 * @def SL_SI91X_RAM_LEVEL_NWP_BASIC_MCU_ADV
 * @brief To enable basic NWP and advanced MCU RAM level configuration.
 * @details This setting configures the NWP with a basic memory level while providing the MCU with an advanced memory configuration.
 * 
 * @note This configuration uses SL_SI91X_EXT_FEAT_352K_M4SS_320K.
 */
#define SL_SI91X_RAM_LEVEL_NWP_BASIC_MCU_ADV SL_SI91X_EXT_FEAT_352K_M4SS_320K

/**
 * @def SL_SI91X_EXT_FEAT_416K_M4SS_256K
 * @brief To enable 416K memory for NWP and 256K memory for M4. This macro is applicable only for SoC mode.
 * @details This configuration allocates 416K memory to the Network Processor (NWP) and 256K memory to the M4 core.
 * 
 * @note Ensure that RAM bank 9 remains powered ON in the M4 for low power application use cases.
 */
#define SL_SI91X_EXT_FEAT_416K_M4SS_256K BIT(21)

/**
 * @def SL_SI91X_RAM_LEVEL_NWP_MEDIUM_MCU_MEDIUM
 * @brief To enable medium NWP and medium MCU RAM level configuration.
 * @details This setting configures both the NWP and the MCU with medium memory levels.
 * 
 * @note This configuration uses SL_SI91X_EXT_FEAT_416K_M4SS_256K.
 */
#define SL_SI91X_RAM_LEVEL_NWP_MEDIUM_MCU_MEDIUM SL_SI91X_EXT_FEAT_416K_M4SS_256K

/**
 * @def SL_SI91X_EXT_FEAT_480K_M4SS_192K
 * @brief To enable 480K memory for NWP and 192K memory for M4. This macro is applicable only for SoC mode.
 * @details This configuration allocates 480K memory to the Network Processor (NWP) and 192K memory to the M4 core.
 * 
 * @note Ensure that RAM banks 8, and 9 remain powered ON in the M4 for low power application use cases.
 */
#define SL_SI91X_EXT_FEAT_480K_M4SS_192K BIT(20)

/**
 * @def SL_SI91X_RAM_LEVEL_NWP_ADV_MCU_BASIC
 * @brief To enable advanced NWP and basic MCU RAM level configuration.
 * @details This setting configures the NWP with an advanced memory level while providing the MCU with a basic memory configuration.
 * 
 * @note This configuration uses SL_SI91X_EXT_FEAT_480K_M4SS_192K.
 */
#define SL_SI91X_RAM_LEVEL_NWP_ADV_MCU_BASIC SL_SI91X_EXT_FEAT_480K_M4SS_192K
#endif

// For NCP
#if (!defined(SLI_SI91X_MCU_INTERFACE)) || defined(DOXYGEN)
/**
 * @def SL_SI91X_EXT_FEAT_352K
 * @brief To enable 352K memory for NWP.
 */
#define SL_SI91X_EXT_FEAT_352K 0

/**
 * @def SL_SI91X_RAM_LEVEL_NWP_BASIC
 * @brief To enable basic NWP RAM level configuration.
 * @details This setting configures the Network Processor (NWP) with 352K of memory.
 */
#define SL_SI91X_RAM_LEVEL_NWP_BASIC SL_SI91X_EXT_FEAT_352K

/**
 * @def SL_SI91X_EXT_FEAT_672K
 * @brief To enable 672K memory for NWP. This macro is applicable for NCP mode only.
 */
#define SL_SI91X_EXT_FEAT_672K (BIT(20) | BIT(21))

/**
 * @def SL_SI91X_RAM_LEVEL_NWP_ALL_AVAILABLE
 * @brief To enable full NWP RAM level configuration.
 * @details This setting configures the NWP with 672K of memory.
 */
#define SL_SI91X_RAM_LEVEL_NWP_ALL_AVAILABLE SL_SI91X_EXT_FEAT_672K

/**
 * @def SL_SI91X_EXT_FEAT_352K_M4SS_320K
 * @brief Enable 352K memory for NWP. The remaining memory will be unused. This macro is applicable for NCP mode only.
 */
#define SL_SI91X_EXT_FEAT_352K_M4SS_320K SL_SI91X_EXT_FEAT_352K

/**
 * @def SL_SI91X_RAM_LEVEL_NWP_BASIC_MCU_ADV
 * @brief To enable basic NWP RAM level configuration. This macro is applicable for NCP mode only.
 * @details This setting configures the Network Processor (NWP) with 352K of memory in NCP mode.
 * 
 * @note This setting is suitable for low-power examples that do not require significant memory for the NWP.
 */
#define SL_SI91X_RAM_LEVEL_NWP_BASIC_MCU_ADV SL_SI91X_EXT_FEAT_352K

/**
 * @def SL_SI91X_EXT_FEAT_672K_M4SS_0K
 * @brief To enable 672K memory for NWP and 0K memory for M4. This macro is applicable for NCP mode only.
 * @details This setting configures the Network Processor (NWP) with 672K of memory and allocates no memory to the M4 core in NCP mode.
 */
#define SL_SI91X_EXT_FEAT_672K_M4SS_0K SL_SI91X_EXT_FEAT_672K

/**
 * @def SL_SI91X_RAM_LEVEL_NWP_ALL_MCU_ZERO
 * @brief To enable full NWP RAM level configuration. This macro is applicable for NCP mode only.
 * @details This setting configures the NWP with 672K of memory and allocates no memory to the M4 core in NCP mode.
 */
#define SL_SI91X_RAM_LEVEL_NWP_ALL_MCU_ZERO SL_SI91X_EXT_FEAT_672K

#endif

/**
 * 
 * | Configuration                         | BIT(20) | BIT(21) | NCP Mode | SOC Mode |
 * |---------------------------------------|---------|---------|----------|----------|
 * | SL_SI91X_EXT_FEAT_352K_M4SS_320K      | 0       | 0       | YES      | YES      |
 * | SL_SI91X_EXT_FEAT_416K_M4SS_256K      | 0       | 1       | NO       | YES      |
 * | SL_SI91X_EXT_FEAT_480K_M4SS_192K      | 1       | 0       | NO       | YES      |
 * | SL_SI91X_EXT_FEAT_672K_M4SS_0K        | 1       | 1       | YES      | NO       |
 * 
 */

#endif // SLI_SI917

/**
 * @def SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE
 * @brief To enable crystal clock for NWP.
 * @details This macro configures the sleep clock source selection for the NWP. Following are the options:
 * 
 * | Selection                                     | BIT[23] | BIT[22] |
 * |:----------------------------------------------|:--------|:--------|
 * | Use RC clock as sleep clock                   | 0       | 0       |
 * | Use 32KHz clock from external XTAL OSCILLATOR | 0       | 1       |
 * | Use 32KHz bypass clock on UULP_GPIO_3         | 1       | 0       |
 * | Use 32KHz bypass clock on UULP_GPIO_4         | 1       | 1       |
 * 
 * @note For 917 radio boards, set `SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE` to 1. For other variants, a value of 2 is recommended.
 */

#ifdef SI91X_32kHz_EXTERNAL_OSCILLATOR
#define SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE(xtal_clk_enable) (xtal_clk_enable << 23)
#else
#define SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE(xtal_clk_enable) (xtal_clk_enable << 22)
#endif

// Determine the XTAL clock enable value
#if defined(SLI_SI917) && defined(SLI_SI91X_MCU_CONFIG_RADIO_BOARD_VER2)
/// To enable crystal clock for NWP
#define SL_SI91X_EXT_FEAT_XTAL_CLK SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE(1)
#else
/**
 * @def SL_SI91X_EXT_FEAT_XTAL_CLK
 * @brief Define to enable 32 KHz crystal clock using the external XTAL OSCILLATOR.
 * @details This macro sets the `SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE` with a value of 2, which configures the sleep clock source to use the 32 KHz clock from the external XTAL OSCILLATOR.
 */
#define SL_SI91X_EXT_FEAT_XTAL_CLK SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE(2)
#endif

/**
 * @def SL_SI91X_EXT_FEAT_HOMEKIT_WAC_ENABLED
 * @brief To inform firmware not to modify mDNS text records.
 * @details Enabling this bit indicates that the firmware should not alter mDNS (Multicast DNS) text records.
 */
#define SL_SI91X_EXT_FEAT_HOMEKIT_WAC_ENABLED BIT(24)

/**
 * @def SL_SI91X_EXT_FEAT_1P8V_SUPPORT
 * @brief To enable 1.8V support for NWP.
 * @details Enabling this bit activates support for 1.8 V operation.
 */
#define SL_SI91X_EXT_FEAT_1P8V_SUPPORT BIT(25)

/**
 * @def SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS
 * @brief To select UART for debug prints pin selection.
 * @details If BIT(27) is enabled, debug prints are supported on UART1. If BIT(27) is disabled, debug prints are not available on UART2 unless SL_SI91X_ULP_GPIO9_FOR_UART2_TX is enabled.
 * 
 * @note Bit 26 is reserved.
 * @note The baud rate for capturing UART debug prints is 460800.
 * @note UART1 and UART2 pins are mapped as follows:
 * - UART1-TX: GPIO_9
 * - UART1-RX: GPIO_8
 * - UART2-TX: ULP_GPIO_9
 *
 * Ensure these pins are not used in MCU applications in SoC mode to avoid pin usage conflicts. This bit is valid only if BIT[28] in the ext_custom_feature_bit_map is set to 0. There is no functionality on RX pins for debug prints.
 */
#define SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS BIT(27)

/**
 * @def SL_SI91X_EXT_FEAT_DISABLE_DEBUG_PRINTS
 * @brief To disable debug prints support in NWP.
 * @details By default the prints would be coming on UART2. If this bit is enabled, disable debug prints.
 * To enable prints on UART 1 @ref SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS  bit needs to set.
 */
#define SL_SI91X_EXT_FEAT_DISABLE_DEBUG_PRINTS BIT(28)

#if defined(SLI_SI917) || defined(DOXYGEN)
/**
 * @def SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
 * @brief To configure frontend with selection BIT[30:29] for 917B0.
 * @details This bit configures the frontend switch pins based on the following table:
 * 
 * | Bit[30] | BIT[29] | ANT_SEL_1 (VC3)  | ANT_SEL_2 (VC2)  | ANT_SEL_3 (VC1)  |                        
 * |:--------|:--------|:-----------------|:-----------------|:-----------------|
 * | 0       | 0       | Reserved         | Reserved         | Reserved         |
 * | 0       | 1       | ULP_GPIO 4       | ULP_GPIO 5       | ULP_GPIO 0       |
 * | 1       | 0       | Internal Switch  | Internal Switch  | Internal Switch  |                                
 * | 1       | 1       | Reserved         | Reserved         | Reserved         |
 * 
 * @note SiWx917 has an integrated on-chip Transmit/Receive (T/R) switch. This internal RF switch configuration uses internal logic present in the IC, and GPIOs are not needed. RF_BLE_TX (8 dBm) mode is not supported in this configuration.
 * @note VC1, VC2, and VC3 are control voltage pins of the RF switch.
 * @note This configuration is not applicable for devices with internal antennas.
 */
#if defined(SLI_SI91X_DEVICE_WITH_INT_RF_SWITCH)
#define SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0 BIT(30)
#else
#define SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0 BIT(29)
#endif

/**
 * @def SL_SI91X_EXT_FEAT_FRONT_END_INTERNAL_SWITCH
 * @brief To enable the internal front-end switch configuration.
 * @details Enabling this bit selects the internal front-end switch configuration for the frontend. This configuration uses internal logic present in the IC, eliminating the need for external GPIOs.
 */
#define SL_SI91X_EXT_FEAT_FRONT_END_INTERNAL_SWITCH BIT(30)

#else
/** 
 * @brief For 917A0
 * 
 * | Bit[30] | BIT[29] | ANT_SEL_1(VC3) | ANT_SEL_2(VC2) | ANT_SEL_3(VC1) |                        
 * |:--------|:--------|:---------------|:---------------|:---------------|
 * | 0       |   0     | GPIO 46        | GPIO 47        | GPIO 48        |
 * | 0       |   1     | Reserved       | Reserved       | Reserved       |
 * | 1       |   0     | UILP_GPIO 4    | ULP_GPIO 5     | ULP_GPIO 0     |
 * | 1       |   1     | UILP_GPIO 4    | ULP_GPIO 5     | ULP_GPIO 7     |
 */
#define SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_GPIO_46_47_48  0
#define SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0 BIT(30)
#define SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_7 (BIT(30) | BIT(29))
#endif

/**
 * @def SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE
 * @brief To enable Bluetooth custom features.
 * @details Enabling this bit activates Bluetooth custom features.
 * If this bit is enabled then only, the features present in the Bluetooth custom feature can be used.
 */
#define SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE BIT(31)
/** @} */

/** \addtogroup SI91X_EXTENDED_TCP_IP_FEATURE_BITMAP
  * @{ */
/*=========================================================================*/
// Extended TCP/IP feature bitmap parameters description !//
/*=========================================================================*/
/**
 * @def SL_SI91X_EXT_TCP_FEAT_DHCP_OPT77
 * @brief DHCP USER CLASS.
 * @details
 * This feature enables DHCP option 77, which allows the device to specify user class information in DHCP requests.
 * 
 * @note Bit 0 is reserved.
 */
#define SL_SI91X_EXT_TCP_FEAT_DHCP_OPT77 BIT(1)

/**
 * @def SL_SI91X_EXT_TCP_IP_BI_DIR_ACK_UPDATE
 * @brief TCP bi-directional acknowledgment update.
 * @details
 * This feature enables bi-directional data transfer by updating TCP acknowledgement handling.
 * 
 * @note Need to enable this bit if you want to run the bi-directional data transfer.
 * @note Bit 2 is reserved.
 */
#define SL_SI91X_EXT_TCP_IP_BI_DIR_ACK_UPDATE BIT(3)

/**
 * @def SL_SI91X_EXT_TCP_IP_WINDOW_DIV
 * @brief TCP RX window division.
 * @details
 * This feature allows the division of the TCP receive window, enables
 * more granular control over the window size.
 */
#define SL_SI91X_EXT_TCP_IP_WINDOW_DIV BIT(4)

/**
 * @def SL_SI91X_EXT_TCP_IP_CERT_BYPASS
 * @brief SSL server certificate bypass.
 * @details
 * This feature allows the device to bypass SSL server certificate
 * validation, with validation being performed by the host instead.
 */
#define SL_SI91X_EXT_TCP_IP_CERT_BYPASS BIT(5)

/**
 * @def SL_SI91X_EXT_TCP_IP_SSL_16K_RECORD
 * @brief SSL 16K record size support.
 * @details
 * This feature enables support for 16K SSL record sizes, which improves
 * performance for SSL connections that use larger record sizes.
 */
#define SL_SI91X_EXT_TCP_IP_SSL_16K_RECORD BIT(6)

/**
 * @def SL_SI91X_EXT_TCP_IP_DNS_CLIENT_BYPASS
 * @brief Enable DNS client bypass.
 * @details
 * This feature allows the device to bypass the internal DNS client,
 * using the host for DNS resolution instead.
 */
#define SL_SI91X_EXT_TCP_IP_DNS_CLIENT_BYPASS BIT(7)

/**
 * @def SL_SI91X_EXT_TCP_IP_WINDOW_SCALING
 * @brief Enable TCP window scaling feature.
 * @details
 * This feature enables TCP window scaling, which allows the device to use
 * receive window sizes larger than 64 KB.
 * 
 * @note If this feature is not enabled, then the maximum possible RX window size is 64 KB.
 * If you want to use more than 64 KB window size, tcp_rx_window_size_cap in socket configuration is used to increase the window size.
 */
#define SL_SI91X_EXT_TCP_IP_WINDOW_SCALING BIT(8)

/**
 * @def SL_SI91X_EXT_TCP_IP_DUAL_MODE_ENABLE
 * @brief Enables support for dual network stack mode.
 * 
 * @details
 * This feature enables support for dual network stack mode, which allows two different network
 * stacks to run on the NWP and the external host simultaneously, which provides flexibility in
 * network communication.
 *
 * @note @ref SL_SI91X_TCP_IP_FEAT_BYPASS bit should not be set to 1 when this bit is enabled.
 * @note Bit 9 is reserved.
 */
#define SL_SI91X_EXT_TCP_IP_DUAL_MODE_ENABLE BIT(10)

/**
 * @def SL_SI91X_EXT_DYNAMIC_COEX_MEMORY
 * @brief Enable dynamic coexistence memory.
 * @details
 * This feature dynamically adjusts the TCP receive window size based on
 * coexistence requirements, that improves network performance in coexistence
 * scenarios.
 * 
 * @note To enable or disable the coexistence and update TCP RX window accordingly.
 */
#define SL_SI91X_EXT_DYNAMIC_COEX_MEMORY BIT(11)

/**
 * @def SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS
 * @brief Configure the number of selects.
 * @details
 * This feature configures the number of select operations the device can
 * handle, with a maximum value of 10.
 * 
 * @note Bits 12-15 are used for TOTAL_SELECTS.
 */
#define SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(total_selects) (total_selects << 12)

/**
 * @def SL_SI91X_EXT_TCP_IP_WAIT_FOR_SOCKET_CLOSE
 * @brief Enable socket wait close.
 * @details
 * This feature ensures that a socket is not closed until close() is called
 * from the host, which is recommended for use with TCP sockets.
 * 
 * @note If it is set socket would not be closed until close() is called from host. It is recommended to enable this bit when using TCP sockets.
 * @note If this bit is enabled, closing the socket will flush any pending data to be sent to remote peer.
 * @note If this bit is disabled, closing the socket will first send any pending data to remote peer.
 * @note This is always set internally for Si91x chips.
 */
#define SL_SI91X_EXT_TCP_IP_WAIT_FOR_SOCKET_CLOSE BIT(16)

/**
 * @def SL_SI91X_EXT_EMB_MQTT_ENABLE
 * @brief Enable embedded/internal MQTT.
 * @details
 * This feature enables support for embedded MQTT (Message Queuing
 * Telemetry Transport) functionality, allowing the device to use MQTT
 * without external libraries.
 * 
 * @note If user wants to use AT command for MQTT, enable this bit in the Opermode Command.
 */
#define SL_SI91X_EXT_EMB_MQTT_ENABLE BIT(17)

/**
 * @def SL_SI91X_EXT_FEAT_HTTP_OTAF_SUPPORT
 * @brief Enable HTTP OTAF support.
 * @details
 * This feature enables support for HTTP-based Over-The-Air Firmware (OTAF)
 * updates, that allows the device to download and install firmware updates
 * via HTTP.
 * 
 * @note To do firmware upgrade with HTTP this bit should be enabled.
 */
#define SL_SI91X_EXT_FEAT_HTTP_OTAF_SUPPORT BIT(18)

/**
 * @def SL_SI91X_EXT_TCP_DYNAMIC_WINDOW_UPDATE_FROM_HOST
 * @brief Enable to update TCP window from host.
 * @details
 * This feature allows the TCP window size to be dynamically updated from
 * the host, that provides more control over TCP flow management.
 */
#define SL_SI91X_EXT_TCP_DYNAMIC_WINDOW_UPDATE_FROM_HOST BIT(19)

/**
 * @def SL_SI91X_EXT_TCP_MAX_RECV_LENGTH
 * @brief Enable to update max receive length for TCP.
 * @details
 * This feature allows the maximum receive length for TCP connections to be
 * updated, accommodating different application requirements.
 */
#define SL_SI91X_EXT_TCP_MAX_RECV_LENGTH BIT(20)

/**
 * @def SL_SI91X_EXT_TCP_NAT_SUPPORT
 * @brief Enable NAT support.
 * @details
 * This feature enables Network Address Translation (NAT) support, allowing
 * stations connected to SiWx91x to access the internet through a third-party AP.
 */
#define SL_SI91X_EXT_TCP_NAT_SUPPORT BIT(21)

/**
 * @def SL_SI91X_EXT_TCP_IP_FEAT_SSL_THREE_SOCKETS
 * @brief Enable three SSL/TLS sockets.
 * @details
 * This feature allows the device to support up to three simultaneous
 * SSL/TLS connections.
 * 
 * @note Set tcp_ip_feature_bit_map[31] and ext_tcp_ip_feature_bit_map[29] to open three TLS sockets.
 * @note Bits 21-28 are reserved.
 */
#define SL_SI91X_EXT_TCP_IP_FEAT_SSL_THREE_SOCKETS BIT(29)

/**
 * @def SL_SI91X_EXT_TCP_IP_FEAT_SSL_MEMORY_CLOUD
 * @brief Configure additional memory for SSL/TLS connections to cloud servers.
 * @details
 * This feature allocates additional memory for SSL/TLS connections,
 * typically required for connections to cloud servers, to avoid 0xD2 error.
 * 
 * @note If user connects to a cloud server using two SSL/TLS connections then it is required to set this bit to avoid 0xD2 error.
 */
#define SL_SI91X_EXT_TCP_IP_FEAT_SSL_MEMORY_CLOUD BIT(30)

/**
 * @def SL_SI91X_CONFIG_FEAT_EXTENTION_VALID
 * @brief Config feature bitmap validity.
 * @details
 * This feature validates the configuration feature bitmap.
 * If this bit is enabled then only, the features present in the configuration feature bitmap can be used.
 */
#ifndef __ZEPHYR__
#define SL_SI91X_CONFIG_FEAT_EXTENTION_VALID BIT(31)
#else
#define SL_SI91X_CONFIG_FEAT_EXTENSION_VALID BIT(31)
#endif
/** @} */

/** \addtogroup SI91X_BT_FEATURE_BITMAP
  * @{ */
/*=========================================================================*/
// BT feature bitmap parameters description !//
/*=========================================================================*/
/**
 * @def SL_SI91X_BT_RF_TYPE
 * @brief Macro to specify the BT RF type for the SI91X wireless device.
 *
 * This macro is used to specify the BT RF type for the SI91X wireless device.
 * It is defined as BIT(30).
 * 0 - RF_TYPE_EXTERNAL
 * 1 - RF_TYPE_INTERNAL
 */
#define SL_SI91X_BT_RF_TYPE BIT(30)
/**
 * @def SL_SI91X_ENABLE_BLE_PROTOCOL
 * @brief Macro to enable the BLE protocol.
 *
 * This macro is used to enable the BLE (Bluetooth Low Energy) protocol.
 * The BLE protocol can be enabled by setting the bit 31 of the corresponding register.
 *
 */
#define SL_SI91X_ENABLE_BLE_PROTOCOL BIT(31)
/** @} */

/** \addtogroup SI91X_BLE_FEATURE_BITMAP
  * @{ */
/*=========================================================================*/
// BLE feature bitmap
/*=========================================================================*/
/**
 * @def SL_SI91X_BLE_MAX_NBR_ATT_REC
 * @brief BLE number of attributes.
 * @details
 * Sets the maximum number of BLE attributes.
 
 * @note Maximum number of BLE attributes is 124.
 * @note Bits 0 - 7 are used to set MAX_NBR_ATT_REC.
 */
#define SL_SI91X_BLE_MAX_NBR_ATT_REC(max_num_of_att_rec) (max_num_of_att_rec << 0)
/**
 * @def SL_SI91X_BLE_MAX_NBR_ATT_SERV
 * @brief BLE number of GATT services.
 * @details
 * Sets the maximum number of BLE GATT services.
 
 * @note Maximum number of services is 10.
 * @note Bits 8-11 are used to set MAX_NBR_ATT_SERV.
 */
#define SL_SI91X_BLE_MAX_NBR_ATT_SERV(max_num_of_att_serv) (max_num_of_att_serv << 8)
/**
 * @def SL_SI91X_BLE_MAX_NBR_PERIPHERALS
 * @brief BLE number of peripherals.
 * @details
 * Sets the maximum number of BLE peripherals.
 
 * @note Maximum number of BLE peripherals is 8 and minimum number of BLE peripheral is 1. 
 * @note Bits 12-15 are used to set MAX_NBR_PERIPHERALS.
 */
#define SL_SI91X_BLE_MAX_NBR_PERIPHERALS(max_num_of_peripherals) (max_num_of_peripherals << 12)
/**
 * @def SL_SI91X_BLE_PWR_INX
 * @brief BLE Tx power index.
 * @details
 * Sets the BLE Tx power index value.
 * - Default value for BLE Tx Power Index is 31.
 * - Range for the BLE Tx Power Index is 1 to 127 (0, 32 index is invalid).
 *   -  1 - 31: BLE - 0 dBm Mode
 *   - 33 - 63: BLE - 10 dBm Mode
 *   - 64 - 127: BLE - HP Mode
 
 * @note Bits 16-23 are used to set PWR_INX.
 */
#define SL_SI91X_BLE_PWR_INX(power_index) (power_index << 16)
/**
 * @def SL_SI91X_BLE_PWR_SAVE_OPTIONS
 * @brief BLE power save options.
 * @details
 * Configures BLE power save options.
 * - Bit 24: BLE_DUTY_CYCLING
 * - Bit 25: BLR_DUTY_CYCLING
 * - Bit 26: BLE_4X_PWR_SAVE_MODE
 * - For BLE_DISABLE_DUTY_CYCLING, bits 24-26 are set to zero.
 
 * @note This feature is not supported in the current release.
 */
#define SL_SI91X_BLE_PWR_SAVE_OPTIONS(duty_cycle) (duty_cycle << 24)
/**
 * @def SL_SI91X_BLE_MAX_NBR_CENTRALS
 * @brief Number of Centrals.
 * @details
 * Sets the maximum number of BLE Central devices.
 
 * @note Maximum number of BLE Centrals is 2 and minimum number of BLE Central is 1.
 * @note Bits 27-28 are used to set BLE_PWR_INX.
 */
#define SL_SI91X_BLE_MAX_NBR_CENTRALS(max_num_of_centrals) (max_num_of_centrals << 27)
/**
 * @def SL_SI91X_BLE_GATT_ASYNC_ENABLE
 * @brief GATT ASYNC BIT.
 * @details
 * Enables asynchronous GATT operations.
 
 * @note Default is disabled. When enabled, the response structure would be filled in the Event, which would come later, not in sync with the response for the query command.
 */
#define SL_SI91X_BLE_GATT_ASYNC_ENABLE BIT(29)
/**
 * @def SL_SI91X_916_BLE_COMPATIBLE_FEAT_ENABLE
 * @brief BLE feature compatibility.
 * @details
 * Enables new feature compatible.
 *  
 * @note This bit should be enable to get the set of events from controller for the new features.
 */
#define SL_SI91X_916_BLE_COMPATIBLE_FEAT_ENABLE BIT(30)
/**
 * @def SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENTION_VALID
 * @brief Extension validity for custom feature bitmap.
 * @details
 * Validates the use of an extended custom feature bitmap for BLE.
 */
#ifndef __ZEPHYR__
#define SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENTION_VALID BIT(31)
#else
#define SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENSION_VALID BIT(31)
#endif
/** @} */

/** \addtogroup SI91X_EXTENDED_BLE_CUSTOM_FEATURE_BITMAP
  * @{ */
/*=========================================================================*/
// Extended BLE custom feature bitmap parameters description !//
/*=========================================================================*/
/**
 * @def SL_SI91X_BLE_NUM_CONN_EVENTS
 * @brief BLE number of connection events.
 * @details
 * Describes the number of buffers that need to be allocated for BLE on the opermode.
 * - By default, each role (central/peripheral) will be allocated with 1 buffer for the notify/write command.
 * - Increasing the buffer capacity for the notify/write commands helps achieve the best throughput.
 * - See rsi_ble_set_wo_resp_notify_buf_info() to set more buffers for the notify/write commands.
 
 * @note Bits 0-4 are used to set NUM_CONN_EVENTS.
 */
#define SL_SI91X_BLE_NUM_CONN_EVENTS(num_conn_events) (num_conn_events << 0)
/**
 * @def SL_SI91X_BLE_NUM_REC_BYTES
 * @brief BLE number of record size in bytes.
 * @details
 * Specifies the number of Attribute 'Value' fields maintained in TA in multiples of 16 bytes.
 * It includes following Attribute types :
 * - All Include Declaration Attribute(0x2802) 'Value' fields (Start Handle, End Handle, UUID)
 * - All Charateristic declaration Attribute(0x2803) 'Value' fields (Properties + Value Handle + UUID)
 * - All Charateristic Value Attribute(0x2A00 - 0xFFFF) 'Value' fields (Value) that are maintained in TA.
 * - n*16 : (n=64, Default 1024 bytes (1K)).
 
 * @note Bits 5-12 are used to set NUM_REC_BYTES.
 */
#define SL_SI91X_BLE_NUM_REC_BYTES(num_rec_bytes) (num_rec_bytes << 5)
/**
 * @def SL_SI91X_BLE_GATT_INIT
 * @brief GATT initialization mode.
 * @details
 * Specifies whether the GATT is initialized in firmware or by the host.
 * - 0: GATT Init in Firmware. Both the GAP service and GATT service would be maintained by the firmware.
 * - 1: GATT Init in Host. The GAP service and GATT service should be created by the APP/Host/User, and the ATT transactions like read, write, notify, and indicates to be handled by the APP/Host/User.
 * - Default: GATT Init in Firmware.
 */
#define SL_SI91X_BLE_GATT_INIT BIT(13)
/**
 * @def SL_SI91X_BLE_INDICATE_CONFIRMATION_FROM_HOST
 * @brief Acknowlegment of the indication from the client.
 * @details
 * As per the ATT protocol, every indication received from the server should be acknowledged (indication response) by the client.
 * - If this bit is disabled, the firmware sends the acknowledgment (indication response).
 * - If this bit is enabled, the APP/Host/User needs to send the acknowledgement (indication response).
 */
#define SL_SI91X_BLE_INDICATE_CONFIRMATION_FROM_HOST BIT(14)
/**
 * @def SL_SI91X_BLE_MTU_EXCHANGE_FROM_HOST
 * @brief MTU exchange request initiation from APP.
 * @details
 * - If this bit is disabled, the firmware initiates the MTU request to the remote device on a successful connection.
 *  - If the peer initiates an MTU exchange request, the firmware will send an Exchange MTU Response in reply to the received Exchange MTU Request.
 * - If this bit is enabled, the APP/Host/User needs to initiate the MTU request using the rsi_ble_mtu_exchange_event API.
 *  - If the peer initiates an MTU exchange request, the APP/Host/User shall send an Exchange MTU Response using the rsi_ble_mtu_exchange_resp API.
 */
#define SL_SI91X_BLE_MTU_EXCHANGE_FROM_HOST BIT(15)
/**
 * @def SL_SI91X_BLE_SET_SCAN_RESP_DATA_FROM_HOST
 * @brief Set scan response data from host.
 * @details
 * The device would maintain some default scan response data to be used in the scan_response controller frame.
 * - Enabling this bit would make the default data Null (empty).
 */
#define SL_SI91X_BLE_SET_SCAN_RESP_DATA_FROM_HOST BIT(16)
/**
 * @def SL_SI91X_BLE_DISABLE_CODED_PHY_FROM_HOST
 * @brief Disable coded PHY from APP.
 * @details
 * The device supports the LE-coded PHY feature (that is, LR - 125 kbps, and 500 kbps) by default.
 * - If this bit is enabled, the device would not support the LE-coded PHY rates.
 */
#define SL_SI91X_BLE_DISABLE_CODED_PHY_FROM_HOST BIT(17)
/**
 * @def SL_SI91X_BLE_ENABLE_ADV_EXTN
 * @brief Enable advertising extensions.
 * @details
 * Enables or disables advertising extensions.
 
 * @note Bit 19 enables advertising extensions.
 */
#define SL_SI91X_BLE_ENABLE_ADV_EXTN BIT(19)
/**
 * @def SL_SI91X_BLE_AE_MAX_ADV_SETS
 * @brief Maximum number of AE advertising sets.
 * @details
 * Configures the maximum number of AE advertising sets.
 * - Maximum number of AE advertising sets is 2. 
 
 * @note Bits 20-23 are used to set the number of AE advertising sets.
 */
#define SL_SI91X_BLE_AE_MAX_ADV_SETS(num_adv_sets) (num_adv_sets << 20)

/**
 * @def SL_SI91X_BT_BLE_STACK_BYPASS_ENABLE
 * @brief Enable BT and BLE stack bypass.
 * @details
 * Enables or disables the BT and BLE stack bypass mode.
 * @note This bit is applicable only for Zephyr.
 * @note Bit 24 enables the BT and BLE stack bypass mode.
 */
#define SL_SI91X_BT_BLE_STACK_BYPASS_ENABLE BIT(24)
/// @note Bits 25-31 are reserved
/** @} */

/** \addtogroup SI91X_CONFIG_FEATURE_BITMAP
  * @{ */
/*=========================================================================*/
// Config feature bitmap parameters description !//
/*=========================================================================*/
/**
 * @def SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP
 * @brief Selects the GPIO for wakeup indication to the host.
 * @details When this bit is disabled, UULP_GPIO_3 is used as the wakeup indication. When enabled, UULP_GPIO_0 is used instead.
 * 
 * @note Bit 1 is reserved and should not be used.
 */
#define SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP BIT(0)

/**
 * @def SL_SI91X_FEAT_DVS_SEL_CONFIG_1
 * @brief Enables Dynamic Voltage Selection (DVS) Configuration 1.
 * @details This bit configures the dynamic voltage selection for the system.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_FEAT_DVS_SEL_CONFIG_1 BIT(2)

/**
 * @def SL_SI91X_FEAT_DVS_SEL_CONFIG_2
 * @brief Enables Dynamic Voltage Selection (DVS) Configuration 2.
 * @details This bit configures the dynamic voltage selection for the system.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_FEAT_DVS_SEL_CONFIG_2 BIT(3)

/**
 * @def SL_SI91X_FEAT_DVS_SEL_CONFIG_3
 * @brief Enables Dynamic Voltage Selection (DVS) Configuration 3.
 * @details This bit configures the dynamic voltage selection for the system.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_FEAT_DVS_SEL_CONFIG_3 BIT(4)

/**
 * @def SL_SI91X_FEAT_DVS_SEL_CONFIG_4
 * @brief Enables Dynamic Voltage Selection (DVS) Configuration 4.
 * @details This bit configures the dynamic voltage selection for the system.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_FEAT_DVS_SEL_CONFIG_4 BIT(5)

/**
 * @def SL_SI91X_EXTERNAL_PMU_GOOD_TIME_100us
 * @brief Configures External PMU good time to 100 µs.
 * @details This bit selects an external PMU good time of 100 microseconds.
 * 
 * @note Not applicable for SI917.
 * @note These bits are used to select external PMU good time. 1 to 15 means 100 µsec to 1500 µsec (in 100 µsec granularity).
 */
#define SL_SI91X_EXTERNAL_PMU_GOOD_TIME_100us BIT(6)

/**
 * @def SL_SI91X_EXTERNAL_PMU_GOOD_TIME_200us
 * @brief Configures External PMU good time to 200 µs.
 * @details This bit selects an external PMU good time of 200 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_EXTERNAL_PMU_GOOD_TIME_200us BIT(7)

/**
 * @def SL_SI91X_EXTERNAL_PMU_GOOD_TIME_300us
 * @brief Configures External PMU good time to 300 µs.
 * @details This is a combination of 100 µs and 200 µs good times, totaling 300 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_EXTERNAL_PMU_GOOD_TIME_300us (BIT(6) | BIT(7))

/**
 * @def SL_SI91X_EXTERNAL_PMU_GOOD_TIME_400us
 * @brief Configures External PMU good time to 400 µs.
 * @details This bit selects an external PMU good time of 400 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_EXTERNAL_PMU_GOOD_TIME_400us BIT(8)

/**
 * @def SL_SI91X_EXTERNAL_PMU_GOOD_TIME_500us
 * @brief Configures External PMU good time to 500 µs.
 * @details This is a combination of 100 µs and 400 µs good times, totaling 500 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_EXTERNAL_PMU_GOOD_TIME_500us (BIT(6) | BIT(8))

/**
 * @def SL_SI91X_EXTERNAL_PMU_GOOD_TIME_600us
 * @brief Configures External PMU good time to 600 µs.
 * @details This is a combination of 200 µs and 400 µs good times, totaling 600 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_EXTERNAL_PMU_GOOD_TIME_600us (BIT(7) | BIT(8))

/**
 * @def SL_SI91X_EXTERNAL_PMU_GOOD_TIME_700us
 * @brief Configures External PMU good time to 700 µs.
 * @details This is a combination of 100 µs, 200 µs, and 400 µs good times, totaling 700 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_EXTERNAL_PMU_GOOD_TIME_700us (BIT(6) | BIT(7) | BIT(8))

/**
 * @def SL_SI91X_EXTERNAL_PMU_GOOD_TIME_800us
 * @brief Configures External PMU good time to 800 µs.
 * @details This bit selects an external PMU good time of 800 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_EXTERNAL_PMU_GOOD_TIME_800us BIT(9)

/**
 * @def SL_SI91X_EXTERNAL_PMU_GOOD_TIME_900us
 * @brief Configures External PMU good time to 900 µs.
 * @details This is a combination of 100 µs and 800 µs good times, totaling 900 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_EXTERNAL_PMU_GOOD_TIME_900us (BIT(6) | BIT(9))

/**
 * @def SL_SI91X_EXTERNAL_PMU_GOOD_TIME_1000us
 * @brief Configures External PMU good time to 1000 µs.
 * @details This is a combination of 200 µs and 800 µs good times, totaling 1000 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_EXTERNAL_PMU_GOOD_TIME_1000us (BIT(7) | BIT(9))

/**
 * @def SL_SI91X_EXTERNAL_PMU_GOOD_TIME_1100us
 * @brief Configures External PMU good time to 1100 µs.
 * @details This is a combination of 100 µs, 200 µs, and 800 µs good times, totaling 1100 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_EXTERNAL_PMU_GOOD_TIME_1100us (BIT(6) | BIT(7) | BIT(9))

/**
 * @def SL_SI91X_EXTERNAL_PMU_GOOD_TIME_1200us
 * @brief Configures External PMU good time to 1200 µs.
 * @details This is a combination of 400 µs and 800 µs good times, totaling 1200 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_EXTERNAL_PMU_GOOD_TIME_1200us (BIT(8) | BIT(9))

/**
 * @def SL_SI91X_EXTERNAL_PMU_GOOD_TIME_1300us
 * @brief Configures External PMU good time to 1300 µs.
 * @details This is a combination of 100 µs, 400 µs, and 800 µs good times, totaling 1300 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_EXTERNAL_PMU_GOOD_TIME_1300us (BIT(6) | BIT(8) | BIT(9))

/**
 * @def SL_SI91X_EXTERNAL_PMU_GOOD_TIME_1400us
 * @brief Configures External PMU good time to 1400 µs.
 * @details This is a combination of 200 µs, 400 µs, and 800 µs good times, totaling 1400 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_EXTERNAL_PMU_GOOD_TIME_1400us (BIT(7) | BIT(8) | BIT(9))

/**
 * @def SL_SI91X_EXTERNAL_PMU_GOOD_TIME_1500us
 * @brief Configures External PMU good time to 1500 µs.
 * @details This is a combination of 100 µs, 200 µs, 400 µs, and 800 µs good times, totaling 1500 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_EXTERNAL_PMU_GOOD_TIME_1500us (BIT(6) | BIT(7) | BIT(8) | BIT(9))

/**
 * @def SL_SI91X_FEAT_EXTERNAL_LDO_SEL
 * @brief Enables selection of external LDO voltage.
 * @details When enabled, this bit allows the selection between using an external LDO or an internal PMU.
 * If both the external LDO selection and the LDO voltage are configured, the system uses the external LDO.
 * If this bit is cleared, the internal PMU is used.
 * 
 * @note These bits are used for External LDO selection External PMU:
 * 1. Incase of External PMU, User has to set EXTERNAL_PMU_GOOD_TIME_CONFIGURATION value to external PMU good time, If this is zero then it indicates using Internal PMU.
 * 2. Incase of External PMU 1.0 or 1.05 V, User has to set both the bits config_feature_bit_map[11] and config_feature_bit_map[10].
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_FEAT_EXTERNAL_LDO_SEL BIT(10)

/**
 * @def SL_SI91X_FEAT_EXTERNAL_LDO_VOL
 * @brief Selects the external LDO voltage.
 * @details This field is relevant only if SL_SI91X_FEAT_EXTERNAL_LDO_SEL is enabled (that is, BIT(10) is set).
 * If this bit is set, the LDO voltage is configured to 1.0 V; if cleared, it is set to 1.1 V.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_FEAT_EXTERNAL_LDO_VOL BIT(11)

/**
 * @def SL_SI91X_ENABLE_NWP_LOGGING
 * @brief Enables NWP logging feature.
 * @details This bit enables the logging feature for the Network Processor (NWP), allowing for better debugging and monitoring of network activities.
 * 
 */
#define SL_SI91X_ENABLE_NWP_LOGGING BIT(12)

/**
 * @def SL_SI91X_FEAT_EAP_TLS_V1P0
 * @brief Enables TLS version 1.0 for enterprise security.
 *
 * @note Bit 13 is reserved.
 */
#define SL_SI91X_FEAT_EAP_TLS_V1P0 SL_WIFI_FEAT_EAP_TLS_V1P0

/**
 * @def SL_SI91X_FEAT_EAP_TLS_V1P2
 * @brief Enables TLS version 1.2 for enterprise security.
 */
#define SL_SI91X_FEAT_EAP_TLS_V1P2 SL_WIFI_FEAT_EAP_TLS_V1P2

/**
 * @def SL_SI91X_FEAT_CONC_STA_AP_DYN_SWITCH_SEL
 * @brief Configures dynamic switching between STA and AP modes.
 * @details This bit enables or disables dynamic switching between Station (STA) and Access Point (AP) modes.
 * When enabled, the system can dynamically switch between STA and AP modes based on operational requirements or network conditions.
 * This feature is useful for applications requiring flexible mode changes to optimize performance or power consumption.
 * 
 * @note Enabling this feature allows the system to switch modes dynamically, which can improve adaptability in varying network scenarios.
 * Make sure to configure the system appropriately for the desired switching behavior.
 * @note Bit 16 is reserved.
 */
#define SL_SI91X_FEAT_CONC_STA_AP_DYN_SWITCH_SEL SL_WIFI_FEAT_CONC_STA_AP_DYN_SWITCH_SEL

/**
 * @def SL_SI91X_ULP_GPIO9_FOR_UART2_TX
 * @brief Selects ULP_GPIO_9 to enable firmware debug prints.
 * @details If BIT(18) is enabled, debug prints are supported on UART2. 
 * 
 * @note The baud rate for capturing UART2 debug prints is 460800.
 * @note UART2 pin is mapped as follows:
 * - UART2-TX: ULP_GPIO_9
 */
#define SL_SI91X_ULP_GPIO9_FOR_UART2_TX BIT(18)

/**
 * @def SL_SI91X_FEAT_DISABLE_MCS_5_6_7_DATARATES
 * @brief Disables MCS-5, 6, and 7 data rates.
 * @details This bit is used to disable the higher MCS data rates (5, 6, and 7) for reduced data rate requirements.
 */
#define SL_SI91X_FEAT_DISABLE_MCS_5_6_7_DATARATES SL_WIFI_FEAT_DISABLE_MCS_5_6_7_DATARATES

/**
 * @def SL_SI91X_FEAT_DISABLE_SHORT_GI
 * @brief Disables Short Guard Interval (Short-GI).
 * @details This bit disables the use of Short-GI, which may affect the timing and performance of the system.
 * 
 * @note This bit applies only to 802.11n data rates. For 802.11ax data rates, disabling Short-GI is not supported.
 */
#define SL_SI91X_FEAT_DISABLE_SHORT_GI SL_WIFI_FEAT_DISABLE_SHORT_GI

/**
 * @def SL_SI91X_PTA_3WIRE_EN
 * @brief Enable PTA 3-Wire feature.
 * @details It has three different configurations, which can be chosen in SL_SI91X_PTA_3WIRE_CONFIG_SEL(config_sel)).
 */
#define SL_SI91X_PTA_3WIRE_EN BIT(21)

/** 
 * @def SL_SI91X_PTA_3WIRE_CONFIG_SEL
 * @brief Option to choose PTA 3-Wire configuration.
 * @details It has three different configurations, which can be chosen by enabling or disabling the Bit [23:22].
 * Each of these configurations changes the behavior of how GRANT is asserted in response to REQUEST and PRIORITY signals.
 * 
 * | Configuration | BIT[23] | BIT[22] |
 * |:--------------|:--------|:--------|
 * | Reserved      | 0       | 0       |
 * | config1       | 0       | 1       |
 * | config2       | 1       | 0       |
 * | config3       | 1       | 1       |
 *
 * | Configuration            |          Description                |
 * |:-------------------------|:------------------------------------|
 * | Configuration 1          | PTA Main would aggressively assert GRANT if the REQUEST is asserted irrespective of PRIORITY being asserted or not. This would mean any ongoing Wi-Fi transmission would be aborted, and GRANT would be provided to the PTA secondary. |
 * | Configuration 2          | PTA Main would aggressively assert GRANT if the REQUEST is asserted irrespective of PRIORITY being asserted or not, with only one exception of an ongoing ACK/Block ACK Transmission in response to a Wi-Fi reception. If there is an ongoing ACK/Block ACK transmission in response to a Wi-Fi Reception, PTA MAIN would GRANT access if PRIORITY is asserted along with REQUEST. |
 * | Configuration 3          | If there is an ongoing Wi-Fi Transmission (whih includes ACK/BLOCK ACK), then PTA MAIN would not assert GRANT to an asserted REQUEST. However, if PRIORITY and REQUEST are asserted, PTA MAIN would assert GRANT. |
 * 
 * The below configuration describes the pin connections between the EFR32MG21 and the SiW91x device that involves the GRANT, REQUEST, and PRIORITY signal.
 * 0 kept reserved for future. 3-Wire used at DUT are GPIO_7(Grant pin driven by DUT), ULP_GPIO_1(Request i/p pin for DUT) and ULP_GPIO_6(Priority i/p pin for DUT).
 *
 * | Pin Description | GPIO         | 4338A Radio board  | 4002A EFR board |
 * |:----------------|:-------------|:-------------------|:----------------|
 * | Request         | ULP_GPIO_1   | WSTK_P16           | Pin7            |
 * | Priority        | ULP_GPIO_6   | EXP_HEADER16       | Pin11           |
 * | Grant           | GPIO_7       | WSTK_P20           | Pin9            |
 */
#define SL_SI91X_PTA_3WIRE_CONFIG_SEL(config_sel) (config_sel << 22)

/**
 * @def SL_SI91X_XTAL_GOODTIME_1000us
 * @brief Configures XTAL good time to 1000 µs.
 * @details This bit selects a default XTAL good time of 1000 microseconds. This setting is applicable from Release 2.3.0 onward.
 * Prior releases have reserved config_feature_bitmap[31:17]. This setting is intended for chip users and not applicable for device users.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_XTAL_GOODTIME_1000us 0

/**
 * @def SL_SI91X_XTAL_GOODTIME_2000us
 * @brief Configures XTAL good time to 2000 µs.
 * @details This bit selects an XTAL good time of 2000 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_XTAL_GOODTIME_2000us BIT(24)

/**
 * @def SL_SI91X_XTAL_GOODTIME_3000us
 * @brief Configures XTAL good time to 3000 µs.
 * @details This bit selects an XTAL good time of 3000 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_XTAL_GOODTIME_3000us BIT(25)

/**
 * @def SL_SI91X_XTAL_GOODTIME_600us
 * @brief Configures XTAL good time to 600 µs.
 * @details This is a combination of 2000 µs and 3000 µs XTAL good times, totaling 600 microseconds.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_XTAL_GOODTIME_600us (BIT(24) | BIT(25))

/**
 * @def SL_SI91X_ENABLE_ENHANCED_MAX_PSP
 * @brief Enables Enhanced Max PSP.
 * @details Set this bit to enable the Enhanced Max PSP feature for improved performance.
 */
#define SL_SI91X_ENABLE_ENHANCED_MAX_PSP SL_WIFI_ENABLE_ENHANCED_MAX_PSP

/**
 * @def SL_SI91X_ENABLE_DEBUG_BBP_TEST_PINS
 * @brief Enables BBP Test Pins.
 * @details Set this bit to enable the use of BBP test pins for debugging and testing purposes.
 * 
 * @note Bits 28 - 31 are reserved for future use.
 */
#define SL_SI91X_ENABLE_DEBUG_BBP_TEST_PINS BIT(27)
/** @} */

/** \addtogroup SL_SI91X_CONSTANTS
  * @{ */
/// Si91x performance profile
#define HIGH_PERFORMANCE SL_WIFI_SYSTEM_HIGH_PERFORMANCE ///< Power save is disabled and throughput is maximum.
#define ASSOCIATED_POWER_SAVE \
  SL_WIFI_SYSTEM_ASSOCIATED_POWER_SAVE ///< Low power profile when the device is associated with an AP (MAX PSP).
#define ASSOCIATED_POWER_SAVE_LOW_LATENCY \
  SL_WIFI_SYSTEM_ASSOCIATED_POWER_SAVE_LOW_LATENCY ///< Low power profile when the device is associated with an AP (FAST PSP). If SL_WIFI_ENABLE_ENHANCED_MAX_PSP bit is set in config_feature_bit_map, then this mode enables the Enhanced Max PSP feature.
#define DEEP_SLEEP_WITHOUT_RAM_RETENTION \
  SL_WIFI_SYSTEM_DEEP_SLEEP_WITHOUT_RAM_RETENTION ///< Deep Sleep without RAM retention when the device is not associated with AP.
#define DEEP_SLEEP_WITH_RAM_RETENTION \
  SL_WIFI_SYSTEM_DEEP_SLEEP_WITH_RAM_RETENTION ///< Deep Sleep with RAM retention when the device is not associated with AP.
/// Si91x performance profile
typedef sl_wifi_system_performance_profile_t SL_DEPRECATED_API_WISECONNECT_4_0 sl_si91x_performance_profile_t;

#define SL_SI91X_CLIENT_MODE            SL_WIFI_CLIENT_MODE                  ///< Wi-Fi personal client mode
#define SL_SI91X_ENTERPRISE_CLIENT_MODE SL_WIFI_ENTERPRISE_CLIENT_MODE       ///< Wi-Fi enterprise client mode
#define SL_SI91X_ACCESS_POINT_MODE      SL_WIFI_ACCESS_POINT_MODE            ///< Wi-Fi access point mode
#define SL_SI91X_TRANSCEIVER_MODE       SL_WIFI_TRANSCEIVER_MODE             ///< Wi-Fi transceiver mode
#define SL_SI91X_TRANSMIT_TEST_MODE     SL_WIFI_TRANSMIT_TEST_MODE           ///< Wi-Fi transmit test mode
#define SL_SI91X_CONCURRENT_MODE        SL_WIFI_CONCURRENT_MODE              ///< Wi-Fi concurrent mode
#define __FORCE_OPERATION_ENUM_16BIT    __SL_WIFI_FORCE_OPERATION_ENUM_16BIT ///< Force the enumeration to be 16-bit

/**
 * @typedef sl_si91x_operation_mode_t
 * @brief Enumeration of Wi-Fi operation modes for the Si91x wireless device.
 *  
 * @details This enumeration defines the various operation modes supported by the Si91x wireless device.
 * Each mode configures the device for a specific use case, such as client mode, access point mode, or transceiver mode.
 */
typedef sl_wifi_operation_mode_t SL_DEPRECATED_API_WISECONNECT_4_0
  sl_si91x_operation_mode_t; ///< Enumeration of Wi-Fi operation modes for the Si91x wireless device.

#define SL_SI91X_WLAN_ONLY_MODE SL_WIFI_SYSTEM_WLAN_ONLY_MODE ///< Wireless local area network (WLAN) only mode
#define SL_SI91X_WLAN_MODE      SL_WIFI_SYSTEM_WLAN_MODE      ///< WLAN mode (not currently supported)
#define SL_SI91X_BLUETOOTH_MODE SL_WIFI_SYSTEM_BLUETOOTH_MODE ///< Bluetooth only mode (not currently supported)
#define SL_SI91X_WLAN_BLUETOOTH_MODE \
  SL_WIFI_SYSTEM_WLAN_BLUETOOTH_MODE                          ///< WLAN and Bluetooth mode (not currently supported)
#define SL_SI91X_DUAL_MODE      SL_WIFI_SYSTEM_DUAL_MODE      ///< Dual mode (not currently supported)
#define SL_SI91X_WLAN_DUAL_MODE SL_WIFI_SYSTEM_WLAN_DUAL_MODE ///< WLAN dual mode (not currently supported)
#define SL_SI91X_BLE_MODE \
  SL_WIFI_SYSTEM_BLE_MODE ///< Bluetooth Low Energy (BLE) only mode, used when power save mode is not needed.
#define SL_SI91X_WLAN_BLE_MODE  SL_WIFI_SYSTEM_WLAN_BLE_MODE    ///< WLAN and BLE mode
#define __FORCE_COEX_ENUM_16BIT __SL_WIFI_FORCE_COEX_ENUM_16BIT ///< Force the enumeration to be 16-bit

/// Si91x wireless co-existence mode
/// @note Only BLE, WLAN, and WLAN + BLE modes are supported.
typedef sl_wifi_system_coex_mode_t SL_DEPRECATED_API_WISECONNECT_4_0 sl_si91x_coex_mode_t;

/// Si91x efuse data index
typedef enum {
  SL_SI91X_EFUSE_MFG_SW_VERSION = 0, ///< Efuse data index for manufacturing software version
  SL_SI91X_EFUSE_PTE_CRC        = 1, ///< Efuse data index for PTE CRC
} sl_si91x_efuse_data_type_t;
/** @} */

/** \addtogroup SI91X_BURN_TARGET_OPTIONS
  * @{ */
/*=========================================================================*/
// Burn target options parameters description !//
// This group defines the target options for burning data into different memory types.
//=========================================================================*/

/**
 * @def SL_SI91X_BURN_INTO_EFUSE
 * @brief Option to burn data into EFUSE.
 * 
 * @details This option specifies that the data should be burned into the EFUSE memory.
 * EFUSE memory is used for storing critical calibration or configuration data that should not be modified after programming. 
 * Use this option when permanent storage of data is required.
 */
#define SL_SI91X_BURN_INTO_EFUSE SL_WIFI_BURN_INTO_EFUSE

/**
 * @def SL_SI91X_BURN_INTO_FLASH
 * @brief Option to burn data into flash.
 * 
 * @details This option specifies that the data should be burned into flash memory.
 * Flash memory provides non-volatile storage that can be reprogrammed.
 * This is suitable for data that might need to be updated or modified over time.
 */
#define SL_SI91X_BURN_INTO_FLASH SL_WIFI_BURN_INTO_FLASH
/** @} */

/** \addtogroup SI91X_CALIBRATION_FLAGS
  * @{ */
/**
 * @def SL_SI91X_BURN_GAIN_OFFSET
 * @brief Burn gain offset into the device.
 *
 * @details
 * This macro defines the bit for burning the gain offset into the device.
 */
#define SL_SI91X_BURN_GAIN_OFFSET SL_WIFI_BURN_GAIN_OFFSET

/**
 * @def SL_SI91X_BURN_FREQ_OFFSET
 * @brief Burn frequency offset into the device.
 *
 * @details
 * This macro defines the bit for burning the frequency offset into the device.
 */
#define SL_SI91X_BURN_FREQ_OFFSET SL_WIFI_BURN_FREQ_OFFSET

/**
 * @def SL_SI91X_SW_XO_CTUNE_VALID
 * @brief Indicates if the software XO CTUNE is valid.
 *
 * @details
 * This macro defines the bit to indicate that the software XO CTUNE (crystal tuning) value is valid.
 */
#define SL_SI91X_SW_XO_CTUNE_VALID SL_WIFI_SW_XO_CTUNE_VALID

/**
 * @def SL_SI91X_BURN_XO_FAST_DISABLE
 * @brief Burn bit to disable XO fast into the device.
 *
 * @details
 * This macro defines the bit for burning a setting to disable the fast XO (crystal oscillator) into the device.
 * 
 * @note Not applicable for SiWx91x.
 */
#define SL_SI91X_BURN_XO_FAST_DISABLE SL_WIFI_BURN_XO_FAST_DISABLE

/** @} */

/** \addtogroup SI91X_DTIM_ALIGNMENT_TYPES
  * @{ */
/// Module wakes up at beacon which is just before or equal to listen_interval
#define SL_SI91X_ALIGN_WITH_BEACON SL_WIFI_ALIGN_WITH_BEACON
/// Module wakes up at DTIM beacon which is just before or equal to listen_interval
#define SL_SI91X_ALIGN_WITH_DTIM_BEACON SL_WIFI_ALIGN_WITH_DTIM_BEACON
/** @} */

/** \addtogroup SL_SI91X_TYPES Types
 * @{
 * */

/**
 * @struct sl_wifi_system_boot_configuration_t
 * @brief Boot configuration structure.
 * @note: Refer sl_wifi_device.h for complete bitmap details.
 */
typedef struct {
  uint16_t
    oper_mode; ///< Operation mode, one of the values from [sl_wifi_operation_mode_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-types#sl-wifi-operation-mode-t).
  uint16_t
    coex_mode; ///< Coexistence mode, one of the values from [sl_wifi_system_coex_mode_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-types#sl-wifi-coex-mode-t).
  uint32_t feature_bit_map;            ///< Feature bitmap, @ref SI91X_FEATURE_BITMAP
  uint32_t tcp_ip_feature_bit_map;     ///< TCP/IP feature bitmap, @ref SI91X_TCP_IP_FEATURE_BITMAP
  uint32_t custom_feature_bit_map;     ///< Custom feature bitmap, @ref SI91X_CUSTOM_FEATURE_BITMAP
  uint32_t ext_custom_feature_bit_map; ///< Extended custom feature bitmap, @ref SI91X_EXTENDED_CUSTOM_FEATURE_BITMAP
  uint32_t bt_feature_bit_map;         ///< BT featured bitmap, @ref SI91X_BT_FEATURE_BITMAP
  uint32_t ext_tcp_ip_feature_bit_map; ///< Extended TCP/IP feature bitmap, @ref SI91X_EXTENDED_TCP_IP_FEATURE_BITMAP
  uint32_t ble_feature_bit_map;        ///< BLE feature bitmap, @ref SI91X_BLE_FEATURE_BITMAP
  uint32_t ble_ext_feature_bit_map;    ///< BLE extended feature bitmap, @ref SI91X_EXTENDED_BLE_CUSTOM_FEATURE_BITMAP
  uint32_t config_feature_bit_map;     ///< Config feature bitmap, @ref SI91X_CONFIG_FEATURE_BITMAP
} sl_wifi_system_boot_configuration_t;

/// NWP buffer allocation command parameters
/// The summation of all three ratios should max 10 and the ratio should be in decimal value.
typedef struct {
  uint8_t tx_ratio_in_buffer_pool;     ///< tx ratio
  uint8_t rx_ratio_in_buffer_pool;     ///< rx ratio
  uint8_t global_ratio_in_buffer_pool; ///< global ratio
} sl_wifi_system_dynamic_pool_t;

// Device configuration for 911x. This should be in the 911x driver folder
/// Device configuration for Si91x device
typedef struct {
  uint8_t boot_option; ///< Boot option. One of the values from @ref SI91X_LOAD_IMAGE_TYPES
  sl_mac_address_t *
    mac_address; ///< MAC address of type [sl_mac_address_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-net-types#sl-mac-address-t).
  sl_wifi_band_mode_t
    band; ///< Wi-Fi band of type [sl_wifi_band_mode_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-constants#sl-wifi-band-mode-t).
  sl_wifi_region_code_t
    region_code; ///< Region code of type [sl_wifi_region_code_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-constants#sl-wifi-regulatory-region-t).
  sl_wifi_system_boot_configuration_t
    boot_config; ///< Boot configuration. [sl_wifi_system_boot_configuration_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-types#sl-wifi-system-boot-configuration-t).
  sl_wifi_system_dynamic_pool_t
    ta_pool;               ///< TA buffer allocation command parameters of type @ref sl_wifi_system_dynamic_pool_t.
  uint8_t efuse_data_type; ///<Type of eFuse data need to be read from flash. Refer to @ref sl_si91x_efuse_data_type_t.
  uint8_t
    nwp_fw_image_number; ///< Image number for the NWP firmware, used to specify which firmware image to load @ref SI91X_NWP_FW_IMAGE_NUMBERS.
} sl_wifi_device_configuration_t;

/// Wi-Fi device context
typedef struct {
  void *device_context; ///< Reserved for future use
} sl_wifi_device_context_t;
/** @} */

/// Si91x calibration data structure
typedef struct {
  uint8_t burn_target;  ///< Burn into efuse or flash, @ref SI91X_BURN_TARGET_OPTIONS
  uint8_t reserved0[3]; ///< Reserved bits
  uint32_t flags;       ///< Calibration Flags, @ref SI91X_CALIBRATION_FLAGS
  int8_t gain_offset;   ///< gain offset
  int8_t xo_ctune;      ///< xo ctune
  uint8_t reserved1[2]; ///< Reserved bits
} si91x_calibration_data_t;

/** \addtogroup SL_SI91X_TYPES
 * @{
 * */
/// Bluetooth performance profile
typedef struct {
  sl_wifi_system_performance_profile_t
    profile; ///< Performance profile of type [sl_wifi_system_performance_profile_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-types#sl-wifi-system-performance-profile-t).
} sl_bt_performance_profile_t;

/**
* Wi-Fi performance profile
* 
* Moving forward, this structure will be deprecated. Instead, use the [sl_wifi_performance_profile_v2_t](../wiseconnect-api-reference-guide-si91x-driver/sl-wifi-performance-profile-v2-t) structure. This is retained for backward compatibility.
*/
typedef struct {
  sl_wifi_system_performance_profile_t
    profile; ///< Performance profile of type [sl_wifi_system_performance_profile_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-types#sl-wifi-system-performance-profile-t).
  uint8_t dtim_aligned_type; ///< Set DTIM alignment required. One of the values from @ref SI91X_DTIM_ALIGNMENT_TYPES.
  uint8_t num_of_dtim_skip;  ///< Number of DTIM intervals to skip. Default value is 0.
  uint16_t listen_interval;  ///< Listen interval in milliseconds.
  uint16_t
    monitor_interval; ///< Monitor interval in milliseconds. Default interval 50 milliseconds is used if monitor_interval is set to 0. This is only valid when performance profile is set to ASSOCIATED_POWER_SAVE_LOW_LATENCY.
  sl_wifi_twt_request_t twt_request;     ///< Target Wake Time (TWT) request settings.
  sl_wifi_twt_selection_t twt_selection; ///< Target Wake Time (TWT) selection request settings.
} sl_wifi_performance_profile_t;

/// Wi-Fi performance profile v2
typedef struct {
  sl_wifi_system_performance_profile_t
    profile; ///< Performance profile of type [sl_wifi_system_performance_profile_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-types#sl-wifi-system-performance-profile-t).
  uint8_t dtim_aligned_type; ///< Set DTIM alignment required. One of the values from @ref SI91X_DTIM_ALIGNMENT_TYPES.
  uint8_t num_of_dtim_skip;  ///< Number of DTIM intervals to skip. Default value is 0.
  uint32_t listen_interval;  ///< Listen interval in milliseconds.
  uint16_t
    monitor_interval; ///< Monitor interval in milliseconds. Default interval 50 milliseconds is used if monitor_interval is set to 0. This is only valid when performance profile is set to ASSOCIATED_POWER_SAVE_LOW_LATENCY.
  sl_wifi_twt_request_t twt_request;     ///< Target Wake Time (TWT) request settings.
  sl_wifi_twt_selection_t twt_selection; ///< Target Wake Time (TWT) selection request settings.
  uint8_t
    beacon_miss_ignore_limit; ///< Number of consecutive missed beacons that can be ignored while the device remains in sleep mode. If the number of beacon misses exceeds this limit and the beacon is still not received, the device will wake up to listen for the beacon. The default value is 1. Recommended range: 1 - 10. Values beyond 10 might lead to interoperability issues.
} sl_wifi_performance_profile_v2_t;

/** @} */
// driver TX/RX packet structure
/// Wi-Fi packet structure
typedef struct {
  union {
    struct {
      uint16_t length;  ///< Length of data
      uint16_t command; ///< command type
      uint8_t unused
        [12]; ///< Contains command status and other additional information. Unused for TX and only used for RX packets.
    };
    uint8_t desc[16]; ///< packet header
  };                  ///< Command header

  uint8_t data[]; ///< Data to be transmitted or received
} sl_wifi_system_packet_t;

/** \addtogroup SL_SI91X_TYPES
 * @{
 * */
/**
 * @struct sl_si91x_rsp_wireless_info_t
 * @brief si91x Specific Wireless information.
 * @note This structure is deprecated. Use [sl_wifi_interface_info_t](../wiseconnect-api-reference-guide-si91x-driver/sl-wifi-interface-info-t) for new implementations.
 */
typedef struct {

  uint16_t
    wlan_state; ///< WLAN state: 1 = connected, 0 = disconnected in station mode; number of stations connected in AP mode.

  uint16_t channel_number; ///< Channel number of connected AP in station mode; channel number of the module in AP mode
                           ///<
                           ///< **Wi-Fi Channel Number Range (1–14):**
  ///< - **Channels 1–11:** Commonly used in most countries, including the United States (default range).
  ///< - **Channels 12–13:** Permitted in Europe and several other regions.
  ///< - **Channel 14:** Exclusively allowed in Japan and rarely used elsewhere.

  uint8_t ssid[SL_WIFI_SSID_LEN]; ///< SSID of connected AP in station mode; SSID of the module in AP mode

  uint8_t mac_address[6]; ///< MAC address of the module

  uint8_t sec_type; ///< Security type of connected AP is supported in station mode, but not in AP mode.
                    ///< Refer sl_wifi_security_t for possible values of Security type

  uint8_t psk_pmk[64]; ///< PSK for AP mode, PMK for station mode

  uint8_t ipv4_address[4]; ///< Module IPv4 Address

  uint8_t ipv6_address[16]; ///< Module IPv6 Address

  uint8_t bssid[6]; ///< BSSID address of connected AP in station mode; not supported in AP mode

  uint8_t
    wireless_mode; ///< Wireless mode used in connected AP (6 - AX, 4 - N, 3 - G, 1 - B) in station mode, not supported in AP mode

} sl_si91x_rsp_wireless_info_t;
/** @} */

#ifndef __ZEPHYR__
/** \addtogroup SL_SI91X_DEFAULT_DEVICE_CONFIGURATION 
  * @{ */
/// Default Wi-Fi client configuration
static const sl_wifi_device_configuration_t sl_wifi_default_client_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_WIFI_BAND_MODE_2_4GHZ,
  .region_code = SL_WIFI_REGION_US,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
#ifdef SLI_SI91X_MCU_INTERFACE
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_WPS_DISABLE),
#else
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION),
#endif
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_SSL | SL_SI91X_TCP_IP_FEAT_MDNSD
#ifdef SLI_SI91X_ENABLE_IPV6
                                              | SL_SI91X_TCP_IP_FEAT_DHCPV6_CLIENT | SL_SI91X_TCP_IP_FEAT_IPV6
#endif
                                              | SL_SI91X_TCP_IP_FEAT_ICMP | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
#if defined(SLI_SI917)
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_WINDOW_SCALING | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(10)),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = 0 }
};
#endif

#ifndef __ZEPHYR__
/// Default Wi-Fi enterprise client configuration
static const sl_wifi_device_configuration_t sl_wifi_default_enterprise_client_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_WIFI_BAND_MODE_2_4GHZ,
  .region_code = SL_WIFI_REGION_US,
  .boot_config = { .oper_mode              = SL_SI91X_ENTERPRISE_CLIENT_MODE,
                   .coex_mode              = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map        = (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_ICMP
                                              | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
#if defined(SLI_SI917)
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_WINDOW_SCALING | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(10)),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = 0 }
};
#endif

#ifndef __ZEPHYR__
/// Default Wi-Fi ap configuration
static const sl_wifi_device_configuration_t sl_wifi_default_ap_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_WIFI_BAND_MODE_2_4GHZ,
  .region_code = SL_WIFI_REGION_US,
  .boot_config = { .oper_mode       = SL_SI91X_ACCESS_POINT_MODE,
                   .coex_mode       = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map = SL_WIFI_FEAT_SECURITY_OPEN,
                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_SERVER | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map     = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map = (SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#if defined(SLI_SI917)
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = 0,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = 0 }
};
#endif

#ifndef __ZEPHYR__
/// Default Wi-Fi concurrent (AP + STATION) configuration
static const sl_wifi_device_configuration_t sl_wifi_default_concurrent_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_WIFI_BAND_MODE_2_4GHZ,
  .region_code = SL_WIFI_REGION_US,
  .boot_config = { .oper_mode              = SL_SI91X_CONCURRENT_MODE,
                   .coex_mode              = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map        = SL_WIFI_FEAT_AGGREGATION,
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DHCPV4_SERVER
                                              | SL_SI91X_TCP_IP_FEAT_ICMP | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map = (SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#if defined(SLI_SI917)
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
};
#endif

#ifndef __ZEPHYR__
/// Default Wi-Fi concurrent (AP + STATION) configuration
static const sl_wifi_device_configuration_t sl_wifi_default_concurrent_v6_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_WIFI_BAND_MODE_2_4GHZ,
  .region_code = SL_WIFI_REGION_US,
  .boot_config = { .oper_mode       = SL_SI91X_CONCURRENT_MODE,
                   .coex_mode       = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map = (SL_WIFI_FEAT_AGGREGATION | SL_WIFI_FEAT_DISABLE_11AX_SUPPORT),
                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DHCPV4_SERVER
                      | SL_SI91X_TCP_IP_FEAT_DHCPV6_CLIENT | SL_SI91X_TCP_IP_FEAT_DHCPV6_SERVER
                      | SL_SI91X_TCP_IP_FEAT_IPV6 | SL_SI91X_TCP_IP_FEAT_ICMP | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID
                      | SL_SI91X_TCP_IP_FEAT_HTTP_CLIENT),
                   .custom_feature_bit_map     = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map = (SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#if defined(SLI_SI917)
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
};
#endif

#ifndef __ZEPHYR__
/// Default Wi-Fi transmit configuration
static const sl_wifi_device_configuration_t sl_wifi_default_transmit_test_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_WIFI_BAND_MODE_2_4GHZ,
  .region_code = SL_WIFI_REGION_WORLD_DOMAIN,
  .boot_config = { .oper_mode = SL_SI91X_TRANSMIT_TEST_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
#ifdef SLI_SI91X_MCU_INTERFACE
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_WPS_DISABLE),
#else
                     (SL_WIFI_FEAT_SECURITY_OPEN),
#endif
                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map     = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map = (MEMORY_CONFIG
#if defined(SLI_SI917)
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map         = SL_SI91X_BT_RF_TYPE,
                   .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
};
#endif

#ifndef __ZEPHYR__
/// Default Wi-Fi transceiver mode configuration
static const sl_wifi_device_configuration_t sl_wifi_default_transceiver_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_WIFI_BAND_MODE_2_4GHZ,
  .region_code = SL_WIFI_REGION_JP,
  .boot_config = { .oper_mode = SL_SI91X_TRANSCEIVER_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
#ifdef TRANSCEIVER_MAC_PEER_DS_SUPPORT
                   .feature_bit_map = (SL_WIFI_FEAT_TRANSCEIVER_MAC_PEER_DS_SUPPORT | SL_WIFI_FEAT_SECURITY_OPEN),
#else
                   .feature_bit_map = SL_WIFI_FEAT_SECURITY_OPEN,
#endif
                   .tcp_ip_feature_bit_map = SL_SI91X_TCP_IP_FEAT_BYPASS,
                   .custom_feature_bit_map = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS
#if defined(SLI_SI917)
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = 0 }
};
#endif

/// The typedefs in the below header depends on the structs defination in this .h
#include "sl_si91x_types.h"
/** @} */

void print_firmware_version(const sl_wifi_firmware_version_t *firmware_version);
