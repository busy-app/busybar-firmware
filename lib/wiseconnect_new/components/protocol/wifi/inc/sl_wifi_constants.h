/***************************************************************************/ /**
 * @file  sl_wifi_constants.h
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

#ifndef _SL_WIFI_CONSTANTS_H_
#define _SL_WIFI_CONSTANTS_H_

#include <stdint.h>

/** \addtogroup SL_WIFI_CONSTANTS Constants
   * @{ */

/// Maximum number of Access Points are scanned in response to a normal scan request.
/// @note This is not a configurable value.
#define SL_WIFI_MAX_SCANNED_AP 11

/**
  * @def SL_WIFI_MAX_SSID_LENGTH
  * @brief Defines the maximum length of a Wi-Fi SSID.
  *
  * @details This macro specifies the maximum number of characters (including the null terminator) 
  *          that a Wi-Fi SSID can have. It is used to ensure proper memory allocation and validation 
  *          of SSID strings in Wi-Fi operations.
  *
  * @note The maximum SSID length is 34 characters, which includes the null terminator.
  */
#define SL_WIFI_MAX_SSID_LENGTH 34

/// Maximum number of clients supported when module is running in Access Point mode.
#define SL_WIFI_MAX_CLIENT_COUNT 16

/// Maximum length of the Wi-Fi Pre-Shared Key (PSK) credential.
#define SL_WIFI_MAX_PSK_LENGTH 64

/// Minimum length of the Wi-Fi Pre-Shared Key (PSK) credential.
#define SL_WIFI_MIN_PSK_LENGTH 8

/// Maximum length of the Wi-Fi Pairwise Master Key (PMK) credential.
#define SL_WIFI_MAX_PMK_LENGTH 32

/// Maximum length of the key in WEP security.
#define SL_WIFI_WEP_KEY_LENGTH 32

/// Maximum number of keys supported for WEP security.
#define SL_WIFI_WEP_KEY_COUNT 4

/// Maximum length of the username in enterprise security.
#define SL_WIFI_EAP_USER_NAME_LENGTH 64

/// Maximum length of the password in enterprise security.
#define SL_WIFI_EAP_PASSWORD_LENGTH 128

/// Maximum length of the certificate key in enterprise security.
#define SL_WIFI_EAP_CERTIFICATE_KEY_LENGTH 80

/// Select Internal Antenna for Wi-Fi.
#define SL_WIFI_SELECT_INTERNAL_ANTENNA 0

/// Select External Antenna for Wi-Fi.
#define SL_WIFI_SELECT_EXTERNAL_ANTENNA 1

/// Macro to get the default Wi-Fi interface.
#define SL_WIFI_DEFAULT_INTERFACE sl_wifi_get_default_interface()

/// Maximum interval for Wi-Fi roaming trigger in milliseconds.
#define SL_WIFI_NEVER_ROAM 0x7FFFFFFF

/// To take listen interval from join command.
#define SL_WIFI_JOIN_FEAT_LISTEN_INTERVAL_VALID (1 << 1)

/// Length of the BSSID (MAC address) for Wi-Fi devices.
#define SL_WIFI_BSSID_LENGTH 6

/// Length of the Wi-Fi MAC address.
#define SL_WIFI_MAC_ADDRESS_LENGTH 6

/// No Wi-Fi credential ID.
#define SL_WIFI_NO_CREDENTIAL_ID 0

/**
  * @enum sl_wifi_security_t
  * @brief Enumeration for Wi-Fi security types.
  *
  * @note WPA3 Transition security type is not currently supported while running as an Access Point (AP).
  * @note To enable any WPA3 mode, the bit represented by the macro [SL_WIFI_EXT_FEAT_IEEE_80211W](../wiseconnect-api-reference-guide-si91x-driver/si91-x-extended-custom-feature-bitmap#sl-si91-x-ext-feat-ieee-80211-w) must be set in the [ext_custom_feature_bit_map](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-boot-configuration-t#ext-custom-feature-bit-map).
  */
typedef enum {
  SL_WIFI_OPEN                       = 0,  ///< Wi-Fi Open security type
  SL_WIFI_WPA                        = 1,  ///< Wi-Fi WPA security type
  SL_WIFI_WPA2                       = 2,  ///< Wi-Fi WPA2 security type
  SL_WIFI_WEP                        = 3,  ///< Wi-Fi WEP security type
  SL_WIFI_WPA_ENTERPRISE             = 4,  ///< Wi-Fi WPA enterprise security type
  SL_WIFI_WPA2_ENTERPRISE            = 5,  ///< Wi-Fi WPA2 enterprise security type
  SL_WIFI_WPA_WPA2_MIXED             = 6,  ///< Wi-Fi WPA/WPA2 mixed security type that supports both WPA and WPA2
  SL_WIFI_WPA3                       = 7,  ///< Wi-Fi WPA3 security type
  SL_WIFI_WPA3_TRANSITION            = 8,  ///< Wi-Fi WPA3 Transition security type (not currently supported in AP mode)
  SL_WIFI_WPA3_ENTERPRISE            = 9,  ///< Wi-Fi WPA3 enterprise security type
  SL_WIFI_WPA3_TRANSITION_ENTERPRISE = 10, ///< Wi-Fi WPA3 Transition enterprise security type

  SL_WIFI_SECURITY_UNKNOWN = 0xFFFF, ///< Wi-Fi Unknown Security type
} sl_wifi_security_t;

/**
  * @enum sl_wifi_encryption_t
  * @brief Enumeration for Wi-Fi encryption methods.
  *
  * @note Some encryption types are not currently supported in station (STA) mode.
  * @note If encryption type is configured anything other than SL_WIFI_DEFAULT_ENCRYPTION, then make sure the AP (third party) supports the configured encryption type. If not, there might be a possibility of getting join failure due to the encryption type mismatch between AP (third party) and STA.
  * @note PEAP encryption supports three security levels through different enum values:
  * - SL_WIFI_PEAP_MSCHAPV2_ENCRYPTION: The EAP server may bypass Phase2 authentication (less secure).
  * - SL_WIFI_PEAP_SAFE1_ENCRYPTION: If a client certificate (private_key/client_cert) is not used and TLS session resumption is not used, then Phase2 authentication is mandatory.
  * - SL_WIFI_PEAP_SAFE2_ENCRYPTION: Requires Phase2 authentication in all cases (most secure).
  * Users can select the desired PEAP security level by setting the appropriate encryption type in their Wi-Fi profile.
  */
typedef enum {
  SL_WIFI_DEFAULT_ENCRYPTION,       ///< Default Wi-Fi encryption
  SL_WIFI_NO_ENCRYPTION,            ///< Wi-Fi with no Encryption (not currently supported in STA mode)
  SL_WIFI_WEP_ENCRYPTION,           ///< Wi-Fi with WEP Encryption (not currently supported in STA mode)
  SL_WIFI_TKIP_ENCRYPTION,          ///< Wi-Fi with TKIP Encryption (not currently supported in STA mode)
  SL_WIFI_CCMP_ENCRYPTION,          ///< Wi-Fi with CCMP Encryption
  SL_WIFI_EAP_TLS_ENCRYPTION,       ///< Wi-Fi with Enterprise TLS Encryption
  SL_WIFI_EAP_TTLS_ENCRYPTION,      ///< Wi-Fi with Enterprise TTLS Encryption
  SL_WIFI_EAP_FAST_ENCRYPTION,      ///< Wi-Fi with Enterprise FAST Encryption
  SL_WIFI_PEAP_MSCHAPV2_ENCRYPTION, ///< Wi-Fi with Enterprise PEAP Encryption (default, less secure)
  SL_WIFI_PEAP_SAFE1_ENCRYPTION,    ///< Wi-Fi with Enterprise PEAP Encryption (SAFE1, medium security)
  SL_WIFI_PEAP_SAFE2_ENCRYPTION,    ///< Wi-Fi with Enterprise PEAP Encryption (SAFE2, most secure)
  SL_WIFI_EAP_LEAP_ENCRYPTION,      ///< Wi-Fi with Enterprise LEAP Encryption
} sl_wifi_encryption_t;

/**
  * @enum sl_wifi_tdi_t
  * @brief Enumeration for Wi-Fi Transition Disable Indication (TDI).
  *
  * @note TDI is supported only in WPA3 (Personal or Personal Transition) security in Access Point (AP) mode.
  */
typedef enum {
  SL_WIFI_TDI_NONE = 0, ///< Allows stations to transition within AP network.
  SL_WIFI_TDI_SAE =
    (1
     << 4), ///< Disallows stations from transitioning within the AP network and only allows connections with WPA3-SAE security mode.
} sl_wifi_tdi_t;

/**
  * @enum sl_wifi_credential_type_t
  * @brief Enumeration for Wi-Fi Credential Types.
  */
typedef enum {
  SL_WIFI_PSK_CREDENTIAL  = 0,        ///< Wi-Fi Personal Credential
  SL_WIFI_PMK_CREDENTIAL  = 1,        ///< Wi-Fi Pairwise Master Key
  SL_WIFI_WEP_CREDENTIAL  = 2,        ///< Wi-Fi WEP Credential
  SL_WIFI_EAP_CREDENTIAL  = 3,        ///< Wi-Fi Enterprise Client Credential
  SL_WIFI_USER_CREDENTIAL = (1 << 31) ///< Wi-Fi User Credential
} sl_wifi_credential_type_t;

/**
  * @enum sl_wifi_antenna_t
  * @brief Enumeration of Wi-Fi antenna selections.
  *
  * @note Only the internal antenna is currently supported.
  */
typedef enum {
  SL_WIFI_ANTENNA_1,        ///< Wi-Fi Radio Antenna 1 (not currently supported)
  SL_WIFI_ANTENNA_2,        ///< Wi-Fi Radio Antenna 2 (not currently supported)
  SL_WIFI_ANTENNA_AUTO,     ///< Wi-Fi Radio Antenna Auto Selection (not currently supported)
  SL_WIFI_ANTENNA_EXTERNAL, ///< Wi-Fi Radio External Antenna (not currently supported)
  SL_WIFI_ANTENNA_INTERNAL, ///< Wi-Fi Radio Internal Antenna
} sl_wifi_antenna_t;

/**
  * @enum sl_wifi_interface_index_t
  * @brief Enumeration of Wi-Fi interface indices.
  *
  * @note 5 GHz interfaces are not currently supported.
  */
typedef enum {
  SL_WIFI_CLIENT_2_4GHZ_INTERFACE_INDEX = 0, ///< Wi-Fi client on 2.4 GHz interface
  SL_WIFI_AP_2_4GHZ_INTERFACE_INDEX,         ///< Wi-Fi access point on 2.4 GHz interface
  SL_WIFI_CLIENT_5GHZ_INTERFACE_INDEX,       ///< Wi-Fi client on 5 GHz interface (Supported in Series-3)
  SL_WIFI_AP_5GHZ_INTERFACE_INDEX,           ///< Wi-Fi access point on 5 GHz interface (Supported in Series-3)
  SL_WIFI_TRANSCEIVER_INTERFACE_INDEX,       ///< Wi-Fi Transceiver Mode
  SL_WIFI_CLIENT_DUAL_INTERFACE_INDEX,       ///< Wi-Fi client on Dual interface (Supported in Series-3)
  SL_WIFI_AP_DUAL_INTERFACE_INDEX,           ///< Wi-Fi access point on Dual interface (Supported in Series-3)
  SL_WIFI_MAX_INTERFACE_INDEX                ///< Used for internally by SDK
} sl_wifi_interface_index_t;

/**
  * @enum sl_wifi_interface_t
  * @brief Enumeration of Wi-Fi interfaces.
  *
  * @note 5 GHz radio interfaces are not currently supported.
  */
typedef enum {
  SL_WIFI_INVALID_INTERFACE = 0, ///< Invalid interface

  SL_WIFI_CLIENT_INTERFACE = (1 << 0), ///< Wi-Fi client interface
  SL_WIFI_AP_INTERFACE     = (1 << 1), ///< Wi-Fi access point interface

  SL_WIFI_2_4GHZ_INTERFACE = (1 << 2),                                          ///<  2.4 GHz radio interface
  SL_WIFI_5GHZ_INTERFACE   = (1 << 3),                                          ///< 5 GHz radio interface
  SL_WIFI_DUAL_INTERFACE   = SL_WIFI_2_4GHZ_INTERFACE | SL_WIFI_5GHZ_INTERFACE, ///< Dual radio interface

  // BIT(4), BIT(5) - Reserved for 6 GHz and Sub-GHz

  SL_WIFI_TRANSCEIVER_INTERFACE = (1 << 7), ///< Wi-Fi Transceiver mode interface

  SL_WIFI_CLIENT_2_4GHZ_INTERFACE = SL_WIFI_CLIENT_INTERFACE
                                    | SL_WIFI_2_4GHZ_INTERFACE, ///< Wi-Fi client interface on 2.4 GHz radio
  SL_WIFI_AP_2_4GHZ_INTERFACE = SL_WIFI_AP_INTERFACE
                                | SL_WIFI_2_4GHZ_INTERFACE, ///< Wi-Fi access point interface on 2.4 GHz radio

  SL_WIFI_CLIENT_5GHZ_INTERFACE = SL_WIFI_CLIENT_INTERFACE
                                  | SL_WIFI_5GHZ_INTERFACE, ///< Wi-Fi client interface on 5 GHz radio
  SL_WIFI_AP_5GHZ_INTERFACE = SL_WIFI_AP_INTERFACE
                              | SL_WIFI_5GHZ_INTERFACE, ///< Wi-Fi access point interface on 5 GHz radio
  SL_WIFI_CLIENT_DUAL_INTERFACE = SL_WIFI_CLIENT_INTERFACE
                                  | SL_WIFI_DUAL_INTERFACE, ///< Wi-Fi client interface on Dual radio
  SL_WIFI_AP_DUAL_INTERFACE = SL_WIFI_AP_INTERFACE
                              | SL_WIFI_DUAL_INTERFACE, ///<  Wi-Fi access point interface on Dual radio
  SL_WIFI_ALL_INTERFACES = SL_WIFI_CLIENT_INTERFACE | SL_WIFI_AP_INTERFACE | SL_WIFI_2_4GHZ_INTERFACE
                           | SL_WIFI_5GHZ_INTERFACE, ///< All available Wi-Fi interfaces

} sl_wifi_interface_t;

/// Enumeration of de-authentication reasons from an access point.
typedef enum {
  SL_WIFI_DEAUTH,             ///< De-Authentication from radius server
  SL_WIFI_DEAUTH_UNSPECIFIED, ///< Unspecified de-authentication reason
} sl_wifi_deauth_reason_t;

/**
  * @enum sl_wifi_regulatory_region_t
  * @brief Enumeration of Wi-Fi regulatory regions.
  *
  * @note Australia and France regions are not currently supported.
  */
typedef enum {
  SL_WIFI_REGION_AUSTRALIA,      ///< Wi-Fi Region Australia (not currently supported)
  SL_WIFI_REGION_FRANCE,         ///< Wi-Fi Region France (not currently supported)
  SL_WIFI_REGION_EUROPEAN_UNION, ///< Wi-Fi Region European Union
  SL_WIFI_REGION_JAPAN,          ///< Wi-Fi Region Japan
  SL_WIFI_REGION_UNITED_STATES,  ///< Wi-Fi Region United States
} sl_wifi_regulatory_region_t;

/**
  * @brief Wi-Fi rate protocols.
  * @note Recommended value for default behavior is SL_WIFI_RATE_PROTOCOL_AUTO.
  * @note 802.11ac not currently supported.
  */
typedef enum {
  SL_WIFI_RATE_PROTOCOL_B_ONLY,  ///< 802.11b rates only (rates go here)
  SL_WIFI_RATE_PROTOCOL_G_ONLY,  ///< 802.11g rates only (rates go here)
  SL_WIFI_RATE_PROTOCOL_N_ONLY,  ///< 802.11n rates only (rates go here)
  SL_WIFI_RATE_PROTOCOL_AC_ONLY, ///< 802.11ac rates only (rates go here) (not currently supported)
  SL_WIFI_RATE_PROTOCOL_AX_ONLY, ///< 802.11ax rates only (rates go here)
  SL_WIFI_RATE_PROTOCOL_AUTO,    ///< Automatic rate selection
} sl_wifi_rate_protocol_t;

/// Wi-Fi scan types.
typedef enum {
  SL_WIFI_SCAN_TYPE_ACTIVE = 0x00, ///< Active scan: Transmit probe requests and listen for responses
  SL_WIFI_SCAN_TYPE_PASSIVE =
    0x01, ///< Passive scan: No active transmissions, listen for AP beacons and broadcast probe responses
  SL_WIFI_SCAN_TYPE_EXTENDED =
    0x02, ///< Extended Active scan. Transmit probe requests and listen for responses to get more than SL_WIFI_MAX_SCANNED_AP number of results
  SL_WIFI_SCAN_TYPE_PROHIBITED_CHANNELS = 0x04, ///< Scan channels prohibited by regulatory region
  SL_WIFI_SCAN_TYPE_ADV_SCAN = 0X08 ///< Advanced scan: Scan for Access Points while the module is in connected state
} sl_wifi_scan_type_t;

/**
  * @enum sl_wifi_rate_t
  * @brief Enumeration of Wi-Fi transfer rates.
  */
typedef enum {
  SL_WIFI_AUTO_RATE = 0, ///< Wi-Fi Auto transfer rate

  SL_WIFI_RATE_11B_1   = 1,                   ///< Wi-Fi 1 Mbps transfer rate for 802.11b
  SL_WIFI_RATE_11B_MIN = SL_WIFI_RATE_11B_1,  ///< Wi-Fi Minimum transfer rate for 802.11b
  SL_WIFI_RATE_11B_2   = 2,                   ///< Wi-Fi 2 Mbps transfer rate for 802.11b
  SL_WIFI_RATE_11B_5_5 = 3,                   ///< Wi-Fi 5.5 Mbps transfer rate for 802.11b
  SL_WIFI_RATE_11B_11  = 4,                   ///< Wi-Fi 11 Mbps transfer rate for 802.11b
  SL_WIFI_RATE_11B_MAX = SL_WIFI_RATE_11B_11, ///< Wi-Fi Maximum transfer rate for 802.11b

  SL_WIFI_RATE_11G_6   = 5,                   ///< Wi-Fi 6 Mbps transfer rate for 802.11g
  SL_WIFI_RATE_11G_MIN = SL_WIFI_RATE_11G_6,  ///< Wi-Fi Minimum transfer rate for 802.11g
  SL_WIFI_RATE_11G_9   = 6,                   ///< Wi-Fi 9 Mbps transfer rate for 802.11g
  SL_WIFI_RATE_11G_12  = 7,                   ///< Wi-Fi 12 Mbps transfer rate for 802.11g
  SL_WIFI_RATE_11G_18  = 8,                   ///< Wi-Fi 18 Mbps transfer rate for 802.11g
  SL_WIFI_RATE_11G_24  = 9,                   ///< Wi-Fi 24 Mbps transfer rate for 802.11g
  SL_WIFI_RATE_11G_36  = 10,                  ///< Wi-Fi 36 Mbps transfer rate for 802.11g
  SL_WIFI_RATE_11G_48  = 11,                  ///< Wi-Fi 48 Mbps transfer rate for 802.11g
  SL_WIFI_RATE_11G_54  = 12,                  ///< Wi-Fi 54 Mbps transfer rate for 802.11g
  SL_WIFI_RATE_11G_MAX = SL_WIFI_RATE_11G_54, ///< Wi-Fi Maximum transfer rate for 802.11g

  SL_WIFI_RATE_11N_MCS0 = 13,                    ///< Wi-Fi MCS index 0 transfer rate for 802.11n
  SL_WIFI_RATE_11N_MIN  = SL_WIFI_RATE_11N_MCS0, ///< Wi-Fi Minimum transfer rate for 802.11n
  SL_WIFI_RATE_11N_MCS1 = 14,                    ///< Wi-Fi MCS index 1 transfer rate for 802.11n
  SL_WIFI_RATE_11N_MCS2 = 15,                    ///< Wi-Fi MCS index 2 transfer rate for 802.11n
  SL_WIFI_RATE_11N_MCS3 = 16,                    ///< Wi-Fi MCS index 3 transfer rate for 802.11n
  SL_WIFI_RATE_11N_MCS4 = 17,                    ///< Wi-Fi MCS index 4 transfer rate for 802.11n
  SL_WIFI_RATE_11N_MCS5 = 18,                    ///< Wi-Fi MCS index 5 transfer rate for 802.11n
  SL_WIFI_RATE_11N_MCS6 = 19,                    ///< Wi-Fi MCS index 6 transfer rate for 802.11n
  SL_WIFI_RATE_11N_MCS7 = 20,                    ///< Wi-Fi MCS index 7 transfer rate for 802.11n
  SL_WIFI_RATE_11N_MAX  = SL_WIFI_RATE_11N_MCS7, ///< Wi-Fi Maximum transfer rate for 802.11n

  SL_WIFI_RATE_11AX_MCS0 = 21,                     ///< Wi-Fi MCS index 0 transfer rate for 802.11ax
  SL_WIFI_RATE_11AX_MIN  = SL_WIFI_RATE_11AX_MCS0, ///< Wi-Fi Minimum transfer rate for 802.11ax
  SL_WIFI_RATE_11AX_MCS1 = 22,                     ///< Wi-Fi MCS index 1 transfer rate for 802.11ax
  SL_WIFI_RATE_11AX_MCS2 = 23,                     ///< Wi-Fi MCS index 2 transfer rate for 802.11ax
  SL_WIFI_RATE_11AX_MCS3 = 24,                     ///< Wi-Fi MCS index 3 transfer rate for 802.11ax
  SL_WIFI_RATE_11AX_MCS4 = 25,                     ///< Wi-Fi MCS index 4 transfer rate for 802.11ax
  SL_WIFI_RATE_11AX_MCS5 = 26,                     ///< Wi-Fi MCS index 5 transfer rate for 802.11ax
  SL_WIFI_RATE_11AX_MCS6 = 27,                     ///< Wi-Fi MCS index 6 transfer rate for 802.11ax
  SL_WIFI_RATE_11AX_MCS7 = 28,                     ///< Wi-Fi MCS index 7 transfer rate for 802.11ax
  SL_WIFI_RATE_11AX_MAX  = SL_WIFI_RATE_11AX_MCS7, ///< Wi-Fi Maximum transfer rate for 802.11ax

  SL_WIFI_RATE_INVALID = 0xFF ///< Wi-Fi Invalid transfer rate
} sl_wifi_rate_t;

/**
  * @enum sl_wifi_bss_type_t
  * @brief Enumeration of Wi-Fi Basic Service Set (BSS) types.
  */
typedef enum {
  SL_WIFI_BSS_TYPE_INFRASTRUCTURE =
    0, ///< Infrastructure BSS: Standard Wi-Fi Infrastructure network with an access point
  SL_WIFI_BSS_TYPE_ADHOC =
    1, ///< Ad-hoc BSS: Peer-to-peer Wi-Fi network without an access point. (Currently not supported)
  SL_WIFI_BSS_TYPE_ANY     = 2,   ///< Any Wi-Fi BSS type. (Currently not supported)
  SL_WIFI_BSS_TYPE_UNKNOWN = 0xFF ///< Unknown Wi-Fi BSS type
} sl_wifi_bss_type_t;

/**
  * @enum sl_wifi_band_t
  * @brief Enumeration of Wi-Fi radio bands.
  *
  * @note Only the 2.4 GHz band is currently supported.
  */
typedef enum {
  SL_WIFI_AUTO_BAND   = 0, ///< Wi-Fi Band Auto
  SL_WIFI_BAND_900MHZ = 1, ///< Wi-Fi Band 900 MHz (not currently supported)
  SL_WIFI_BAND_2_4GHZ = 2, ///< Wi-Fi Band 2.4 GHz
  SL_WIFI_BAND_5GHZ   = 3, ///< Wi-Fi Band 5 GHz (Supported in Series-3)
  SL_WIFI_BAND_DUAL   = 4, ///< Wi-Fi Band Dual (Supported in Series-3)
  SL_WIFI_BAND_6GHZ   = 5, ///< Wi-Fi Band 6 GHz (not currently supported)
  SL_WIFI_BAND_60GHZ  = 6, ///< Wi-Fi Band 60 GHz (not currently supported)
} sl_wifi_band_t;

/**
  * @enum sl_wifi_bandwidth_t
  * @brief Enumeration of Wi-Fi bandwidth options.
  *
  * @note Only 20 MHz bandwidth is currently supported.
  */
typedef enum {
  SL_WIFI_AUTO_BANDWIDTH   = 0, ///< Wi-Fi Bandwidth Auto
  SL_WIFI_BANDWIDTH_10MHz  = 0, ///< Wi-Fi Bandwidth 10 MHz (not currently supported)
  SL_WIFI_BANDWIDTH_20MHz  = 1, ///< Wi-Fi Bandwidth 20 MHz
  SL_WIFI_BANDWIDTH_40MHz  = 2, ///< Wi-Fi Bandwidth 40 MHz (not currently supported)
  SL_WIFI_BANDWIDTH_80MHz  = 3, ///< Wi-Fi Bandwidth 80 MHz (not currently supported)
  SL_WIFI_BANDWIDTH_160MHz = 4, ///< Wi-Fi Bandwidth 160 MHz (not currently supported)
} sl_wifi_bandwidth_t;

/**
  * @enum sl_wifi_client_flag_t
  * @brief Option flags for Wi-Fi client interfaces.
  */
typedef enum {
  SL_WIFI_NO_JOIN_OPTION   = 0,        ///< Wi-Fi Client Join with no flags
  SL_WIFI_JOIN_WITH_NO_CSA = (1 << 0), ///< Wi-Fi Client Join without Channel Switch Announcement (CSA)
  SL_WIFI_JOIN_WITH_SCAN   = (1 << 1)  ///< Wi-Fi Client Join with Scan
} sl_wifi_client_flag_t;

/**
  * @enum sl_wifi_ap_flag_t
  * @brief Option flags for Access Point (AP) interfaces.
  * 
  * @note When SL_WIFI_HIDDEN_SSID is enabled, the AP's SSID will not be broadcast in beacon frames.
  *       Clients must manually enter the exact SSID name to connect to the hidden AP.
  * @note Hidden SSID dynamic configurability is only available in APCONF when it is disabled in opermode.
  */
typedef enum {
  SL_WIFI_HIDDEN_SSID = (1 << 0), ///< Hide the SSID of the AP
} sl_wifi_ap_flag_t;

/**
  * @enum sl_wifi_listen_interval_time_unit_t
  * @brief Enumeration of listen interval time units.
  */
typedef enum {
  SL_WIFI_LISTEN_INTERVAL_TIME_UNIT_BEACON, ///< Time units specified in beacon periods
  SL_WIFI_LISTEN_INTERVAL_TIME_UNIT_DTIM ///< Time units specified in Delivery Traffic Indication Message (DTIM) periods
} sl_wifi_listen_interval_time_unit_t;

/**
  * @enum sl_wifi_wps_mode_t
  * @brief Enumeration of Wi-Fi Protected Setup (WPS) modes.
  */
typedef enum {
  SL_WIFI_WPS_PUSH_BUTTON_MODE, ///< WPS push button mode: Requires the user to press a physical or virtual button on both the AP and the client device.
  SL_WIFI_WPS_PIN_MODE, ///< WPS pin mode: Requires a PIN to be entered on the client device.
} sl_wifi_wps_mode_t;

/**
  * @enum sl_wifi_wps_role_t
  * @brief Enumeration of Wi-Fi WPS (Wi-Fi Protected Setup) modes.
  * @note The SL_WIFI_WPS_REGISTRAR_ROLE is currently not supported.
  */
typedef enum {
  SL_WIFI_WPS_ENROLLEE_ROLE, ///< With enrollee role configured, the device will join the network.
  SL_WIFI_WPS_REGISTRAR_ROLE, ///< With registrar role configured, the device will create a Wi-Fi network for other devices to join.
} sl_wifi_wps_role_t;

/**
  * @enum sl_wifi_wps_resp_status_error_code_t
  * @brief Provides definitions for error codes updated in the status field of the sl_wifi_wps_response_t structure.
  */
typedef enum {
  SL_WIFI_WPS_RESP_STATUS_NO_ERROR = 0, ///< Success case - Used in all successful M1, M2 messages
  SL_WIFI_WPS_RESP_STATUS_MULTIPLE_PBC_DETECTED =
    12, ///< PBC overlap detected - Multiple devices attempting PBC at the same time
  SL_WIFI_WPS_RESP_STATUS_SETUP_LOCKED = 15, ///< AP setup locked - AP refuses new registrar enrollment when locked
  SL_WIFI_WPS_RESP_STATUS_MSG_TIMEOUT  = 16, ///< WPS timeout - Operation timed out (2 min PBC_WALK_TIME expired)
} sl_wifi_wps_resp_status_error_code_t;

/**
  * @enum sl_wifi_event_group_t
  * @brief Enumeration of Wi-Fi event groups.
  */
typedef enum {
  SL_WIFI_SCAN_RESULT_EVENTS = 0, ///< Event group for Wi-Fi scan results
  SL_WIFI_JOIN_EVENTS        = 1, ///< Event group for Wi-Fi join status
  SL_WIFI_RX_PACKET_EVENTS =
    2, ///< Event group for Wi-Fi received packet. This feature is not supported in current release
  SL_WIFI_COMMAND_RESPONSE_EVENTS =
    3, ///< Event group for Wi-Fi command response. This feature is not supported in current release
  SL_WIFI_STATS_RESPONSE_EVENTS     = 4, ///< Event group for Wi-Fi statistics response
  SL_WIFI_HTTP_OTA_FW_UPDATE_EVENTS = 5, ///< Event group for Wi-Fi OTA firmware update status via HTTP
  SL_WIFI_NETWORK_DOWN_EVENTS =
    6, ///< Event group for Wi-Fi network down. This feature is not supported in current release
  SL_WIFI_NETWORK_UP_EVENTS = 7, ///< Event group for Wi-Fi network up. This feature is not supported in current release
  SL_WIFI_CLIENT_CONNECTED_EVENTS      = 8,  ///< Event group for Wi-Fi client connected status
  SL_WIFI_TWT_RESPONSE_EVENTS          = 9,  ///< Event group for Wi-Fi TWT response
  SL_WIFI_CLIENT_DISCONNECTED_EVENTS   = 10, ///< Event group for Wi-Fi client disconnection status
  SL_WIFI_TRANSCEIVER_EVENTS           = 11, ///< Event group for Wi-Fi transceiver events
  SL_WIFI_COMMAND_ENGINE_STATUS_EVENTS = 12, ///< Event group for Wi-Fi command engine status
  SL_WIFI_EVENT_GROUP_COUNT = 13, ///< Event group for Wi-Fi maximum default group count. Used internally by SDK
  SL_WIFI_EVENT_FAIL_INDICATION_EVENTS = (1 << 31), ///< Event group for Wi-Fi fail indication
} sl_wifi_event_group_t;

/**
  * @enum sl_wifi_event_t
  * @brief Enumeration of Wi-Fi events.
  *
  * @note Each event group has a matching event.
  * @note Each event group may be a source of multiple different events.
  */
typedef enum {
  SL_WIFI_SCAN_RESULT_EVENT =
    SL_WIFI_SCAN_RESULT_EVENTS, ///< Event for Wi-Fi scan result. Data would be type of @ref sl_wifi_scan_result_t
  SL_WIFI_JOIN_EVENT = SL_WIFI_JOIN_EVENTS, ///< Event for Wi-Fi join status. Data would be of type string
  SL_WIFI_RX_PACKET_EVENT =
    SL_WIFI_RX_PACKET_EVENTS, ///< Event for Wi-Fi received packet. This feature is not supported in current release
  SL_WIFI_COMMAND_RESPONSE_EVENT =
    SL_WIFI_COMMAND_RESPONSE_EVENTS, ///< Event for Wi-Fi command response. This feature is not supported in current release
  SL_WIFI_STATS_RESPONSE_EVENT =
    SL_WIFI_STATS_RESPONSE_EVENTS, ///< Event for Wi-Fi statistics response. Data would be NULL
  SL_WIFI_HTTP_OTA_FW_UPDATE_EVENT =
    SL_WIFI_HTTP_OTA_FW_UPDATE_EVENTS, ///< Event for Wi-Fi OTA firmware update status via HTTP. Data would be NULL
  SL_WIFI_NETWORK_DOWN_EVENT =
    SL_WIFI_NETWORK_DOWN_EVENTS, ///< Event for Wi-Fi network down. This feature is not supported in current release
  SL_WIFI_NETWORK_UP_EVENT =
    SL_WIFI_NETWORK_UP_EVENTS, ///< Event for Wi-Fi network up. This feature is not supported in current release
  SL_WIFI_CLIENT_CONNECTED_EVENT =
    SL_WIFI_CLIENT_CONNECTED_EVENTS, ///< Event for Wi-Fi client connected status in Access Point Mode. Data would be of type [sl_mac_address_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-net-types#sl-mac-address-t).
  SL_WIFI_TWT_RESPONSE_EVENT =
    SL_WIFI_TWT_RESPONSE_EVENTS, ///< Event for Wi-Fi TWT response in WiFi Client mode. Data would be NULL
  SL_WIFI_CLIENT_DISCONNECTED_EVENT =
    SL_WIFI_CLIENT_DISCONNECTED_EVENTS, ///< Event for Wi-Fi client disconnection status in Access Point Mode. Data would of type [sl_mac_address_t](../wiseconnect-api-reference-guide-common/ieee802#sl-mac-address-t).
  SL_WIFI_TRANSCEIVER_EVENT =
    SL_WIFI_TRANSCEIVER_EVENTS, ///< Event for Wi-Fi transceiver TX/RX events in WiFi Client mode.
  SL_WIFI_COMMAND_ENGINE_STATUS_EVENT =
    SL_WIFI_COMMAND_ENGINE_STATUS_EVENTS, ///< Event for Wi-Fi command engine status.
  // TWT specific events
  SL_WIFI_TWT_UNSOLICITED_SESSION_SUCCESS_EVENT =
    SL_WIFI_TWT_RESPONSE_EVENTS
    | (1
       << 16), ///< Event for TWT unsolicited session success. Data would be of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).
  SL_WIFI_TWT_AP_REJECTED_EVENT =
    SL_WIFI_TWT_RESPONSE_EVENTS
    | (4
       << 16), ///< Event for TWT AP rejection. Data would be of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).
  SL_WIFI_TWT_OUT_OF_TOLERANCE_EVENT =
    SL_WIFI_TWT_RESPONSE_EVENTS
    | (5
       << 16), ///< Event for TWT out of tolerance. Data would be of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).
  SL_WIFI_TWT_RESPONSE_NOT_MATCHED_EVENT =
    SL_WIFI_TWT_RESPONSE_EVENTS
    | (6
       << 16), ///<  Event for TWT response not matched. Data would be of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).
  SL_WIFI_TWT_UNSUPPORTED_RESPONSE_EVENT =
    SL_WIFI_TWT_RESPONSE_EVENTS
    | (10
       << 16), ///< Event for TWT unsupported response. Data would be of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).
  SL_WIFI_TWT_TEARDOWN_SUCCESS_EVENT =
    SL_WIFI_TWT_RESPONSE_EVENTS
    | (11
       << 16), ///< Event for TWT teardown success. Data would be of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).
  SL_WIFI_TWT_AP_TEARDOWN_SUCCESS_EVENT =
    SL_WIFI_TWT_RESPONSE_EVENTS
    | (12
       << 16), ///< Event for TWT AP teardown success. Data would be of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).
  SL_WIFI_TWT_FAIL_MAX_RETRIES_REACHED_EVENT =
    SL_WIFI_TWT_RESPONSE_EVENTS
    | (15
       << 16), ///< Event for TWT maximum retries reached. Data would be of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).
  SL_WIFI_TWT_INACTIVE_DUE_TO_ROAMING_EVENT =
    SL_WIFI_TWT_RESPONSE_EVENTS
    | (16
       << 16), ///< Event for TWT inactive due to roaming. Data would be of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).
  SL_WIFI_TWT_INACTIVE_DUE_TO_DISCONNECT_EVENT =
    SL_WIFI_TWT_RESPONSE_EVENTS
    | (17
       << 16), ///< Event for TWT inactive due to disconnect. Data would be of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).
  SL_WIFI_TWT_INACTIVE_NO_AP_SUPPORT_EVENT =
    SL_WIFI_TWT_RESPONSE_EVENTS
    | (18
       << 16), ///< Event for TWT inactive due to no AP support. Data would be of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).
  SL_WIFI_RESCHEDULE_TWT_SUCCESS_EVENT =
    SL_WIFI_TWT_RESPONSE_EVENTS
    | (19
       << 16), ///<  Event for TWT suspend resume success. Data would be of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).
  SL_WIFI_TWT_INFO_FRAME_EXCHANGE_FAILED_EVENT =
    SL_WIFI_TWT_RESPONSE_EVENTS
    | (20
       << 16), ///< Event for TWT info frame exchange failure. Data would be of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).
  SL_WIFI_TWT_EVENTS_END =
    SL_WIFI_TWT_RESPONSE_EVENTS
    | (21
       << 16), ///< Event for TWT event end.  Data would be of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).

  // Stats specific events
  SL_WIFI_STATS_EVENT = SL_WIFI_STATS_RESPONSE_EVENTS
                        | (1 << 16), ///< Event for Wi-Fi statistics. This feature is not supported in current release
  SL_WIFI_STATS_ASYNC_EVENT =
    SL_WIFI_STATS_RESPONSE_EVENTS
    | (2
       << 16), ///< Event for Wi-Fi asynchronous statistics. Data would be of type [sl_wifi_async_stats_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-async-stats-response-t)
  SL_WIFI_STATS_ADVANCE_EVENT =
    SL_WIFI_STATS_RESPONSE_EVENTS
    | (3
       << 16), ///< Event for Wi-Fi advance statistics. Data would be of type [sl_wifi_advanced_stats_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-advance-stats-response-t)
  SL_WIFI_STATS_TEST_MODE_EVENT =
    SL_WIFI_STATS_RESPONSE_EVENTS
    | (4 << 16), ///< Event for Wi-Fi test mode statistics. This feature is not supported in current release
  SL_WIFI_STATS_MODULE_STATE_EVENT =
    SL_WIFI_STATS_RESPONSE_EVENTS
    | (5
       << 16), ///< Event for Wi-Fi module state statistics. Data would be of type [sl_wifi_module_state_stats_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-module-state-stats-response-t)

  SL_WIFI_TRANSCEIVER_RX_DATA_RECEIVE_CB = SL_WIFI_TRANSCEIVER_EVENTS | (1 << 16),
  SL_WIFI_TRANSCEIVER_TX_DATA_STATUS_CB  = SL_WIFI_TRANSCEIVER_EVENTS | (2 << 16),

  // Single bit to indicate relevant event is related to a failure condition
  SL_WIFI_EVENT_FAIL_INDICATION = (1 << 31),  ///< Event for Wi-Fi event failure indication
  SL_WIFI_INVALID_EVENT         = 0xFFFFFFFF, ///< Invalid Wi-Fi event.  Data would be NULL
} sl_wifi_event_t;

/**
  * @enum sl_wifi_reschedule_twt_action_t
  * @brief Enumeration defining actions related to Target Wake Time (TWT).
  */
typedef enum {
  SL_WIFI_SUSPEND_INDEFINITELY, ///< Suspend TWT indefinitely, effectively disabling TWT functionality until explicitly resumed.
  SL_WIFI_SUSPEND_FOR_DURATION, ///< Suspend TWT for a specified duration, after which it can automatically resume.
  SL_WIFI_RESUME_IMMEDIATELY    ///< Resume TWT immediately, allowing devices to continue adhering to TWT schedules.
} sl_wifi_reschedule_twt_action_t;

/**
  * @enum sl_wifi_data_rate_t
  * @brief Enumeration of Wi-Fi data rates.
  */
typedef enum {
  SL_WIFI_DATA_RATE_1       = 0,   ///< Wi-Fi 1 Mbps transfer rate
  SL_WIFI_DATA_RATE_2       = 2,   ///< Wi-Fi 2 Mbps transfer rate
  SL_WIFI_DATA_RATE_5_5     = 4,   ///< Wi-Fi 5.5 Mbps transfer rate
  SL_WIFI_DATA_RATE_11      = 6,   ///< Wi-Fi 11 Mbps transfer rate
  SL_WIFI_DATA_RATE_6       = 139, ///< Wi-Fi 6 Mbps transfer rate
  SL_WIFI_DATA_RATE_9       = 143, ///< Wi-Fi 9 Mbps transfer rate
  SL_WIFI_DATA_RATE_12      = 138, ///< Wi-Fi 12 Mbps transfer rate
  SL_WIFI_DATA_RATE_18      = 142, ///< Wi-Fi 18 Mbps transfer rate
  SL_WIFI_DATA_RATE_24      = 137, ///< Wi-Fi 24 Mbps transfer rate
  SL_WIFI_DATA_RATE_36      = 141, ///< Wi-Fi 36 Mbps transfer rate
  SL_WIFI_DATA_RATE_48      = 136, ///< Wif-Fi 48 Mbps transfer rate
  SL_WIFI_DATA_RATE_54      = 140, ///< Wi-Fi 54 Mbps transfer rate
  SL_WIFI_DATA_RATE_MCS0    = 256, ///< Wi-Fi MCS index 0 transfer rate
  SL_WIFI_DATA_RATE_MCS1    = 257, ///< Wi-Fi MCS index 1 transfer rate
  SL_WIFI_DATA_RATE_MCS2    = 258, ///< Wi-Fi MCS index 2 transfer rate
  SL_WIFI_DATA_RATE_MCS3    = 259, ///< Wi-Fi MCS index 3 transfer rate
  SL_WIFI_DATA_RATE_MCS4    = 260, ///< Wi-Fi MCS index 4 transfer rate
  SL_WIFI_DATA_RATE_MCS5    = 261, ///< Wi-Fi MCS index 5 transfer rate
  SL_WIFI_DATA_RATE_MCS6    = 262, ///< Wi-Fi MCS index 6 transfer rate
  SL_WIFI_DATA_RATE_MCS7    = 263, ///< Wi-Fi MCS index 7 transfer rate
  SL_WIFI_DATA_RATE_MCS7_SG = 775,
} sl_wifi_data_rate_t;

/**
  * @enum sl_wifi_tx_test_mode_t
  * @brief Enumeration of Wi-Fi TX test modes.
  */
typedef enum {
  SL_WIFI_TEST_BURST_MODE                          = 0, ///< Burst Mode
  SL_WIFI_TEST_CONTINOUS_MODE                      = 1, ///< Continuous Mode
  SL_WIFI_TEST_CONTINOUS_WAVE_MODE                 = 2, ///< CW Mode
  SL_WIFI_TEST_CONTINOUS_WAVE_MODE_OFF_CENTER_LOW  = 3, ///< CW Mode center frequency - 2.5 MHZ Mode
  SL_WIFI_TEST_CONTINOUS_WAVE_MODE_OFF_CENTER_HIGH = 4, ///< CW Mode center frequency + 5 MHZ Mode
} sl_wifi_tx_test_mode_t;

/**
  * @enum sl_wifi_high_throughput_capability_types_t
  * @brief Enumeration of High Throughput (HT) capabilities.
  */
typedef enum {
  SL_WIFI_HT_CAPS_NUM_RX_STBC    = (1 << 8), ///< Number of RX Space-Time Block Coding (STBC) streams supported
  SL_WIFI_HT_CAPS_SHORT_GI_20MHZ = (1 << 5), ///< Support for Short Guard Interval (GI) in 20 MHz channels
  SL_WIFI_HT_CAPS_GREENFIELD_EN =
    (1 << 4), ///< Support for Greenfield mode, which improves efficiency by eliminating legacy preambles
  SL_WIFI_HT_CAPS_SUPPORT_CH_WIDTH = (1 << 1), ///< Support for wider channel bandwidth (Not supported for SiWx917)
} sl_wifi_high_throughput_capability_types_t;

/**
  * @enum sl_wifi_multicast_filter_command_t
  * @brief Enumeration of multicast filter command types.
  */
typedef enum {
  SL_WIFI_MULTICAST_MAC_ADD_BIT   = 0, ///< To set particular bit in multicast bitmap
  SL_WIFI_MULTICAST_MAC_CLEAR_BIT = 1, ///< To reset particular bit in multicast bitmap
  SL_WIFI_MULTICAST_MAC_CLEAR_ALL = 2, ///< To clear all the bits in multicast bitmap
  SL_WIFI_MULTICAST_MAC_SET_ALL   = 3, ///< To set all the bits in multicast bitmap
} sl_wifi_multicast_filter_command_t;

/**
  * @enum sl_wifi_eap_client_flag_t
  * @brief Option flags for EAP (Extensible Authentication Protocol) client interfaces.
  */
typedef enum {
  SL_WIFI_EAP_ENABLE_OKC = (1 << 0), ///< Wi-Fi EAP Client flag to enable Opportunistic Key Caching (OKC)
  SL_WIFI_EAP_ENABLE_PEAP_CA =
    (1 << 1), ///< Wi-Fi EAP Client flag to enable CA certificate requirement for PEAP connection
  SL_WIFI_EAP_DHE_RSA_AES256_SHA256 =
    (1 << 2), ///< Wi-Fi EAP Client flag to use DHE-RSA-AES256-SHA256 Cipher for EAP connection
  SL_WIFI_EAP_DHE_RSA_AES128_SHA256 =
    (1 << 3), ///< Wi-Fi EAP Client flag to use DHE-RSA-AES128-SHA256 Cipher for EAP connection
  SL_WIFI_EAP_DHE_RSA_AES256_SHA =
    (1 << 4), ///< Wi-Fi EAP Client flag to use DHE-RSA-AES256-SHA Cipher for EAP connection
  SL_WIFI_EAP_DHE_RSA_AES128_SHA =
    (1 << 5),                            ///< Wi-Fi EAP Client flag to use DHE-RSA-AES128-SHA Cipher for EAP connection
  SL_WIFI_EAP_AES256_SHA256 = (1 << 6),  ///< Wi-Fi EAP Client flag to use AES256-SHA256 Cipher for EAP connection
  SL_WIFI_EAP_AES128_SHA256 = (1 << 7),  ///< Wi-Fi EAP Client flag to use AES128-SHA256 Cipher for EAP connection
  SL_WIFI_EAP_AES256_SHA    = (1 << 8),  ///< Wi-Fi EAP Client flag to use AES256-SHA Cipher for EAP connection
  SL_WIFI_EAP_AES128_SHA    = (1 << 9),  ///< Wi-Fi EAP Client flag to use AES128-SHA Cipher for EAP connection
  SL_WIFI_EAP_RC4_SHA       = (1 << 10), ///< Wi-Fi EAP Client flag to use RC4-SHA Cipher for EAP connection
  SL_WIFI_EAP_DES_CBC3_SHA  = (1 << 11), ///< Wi-Fi EAP Client flag to use DES-CBC3-SHA Cipher for EAP connection
  SL_WIFI_EAP_RC4_MD5       = (1 << 12)  ///< Wi-Fi EAP Client flag to use RC4-MD5 Cipher for EAP connection
} sl_wifi_eap_client_flag_t;

//! @cond Doxygen_Suppress
/**
  * @enum sli_wifi_statistics_report_t
  * @brief Enumeration for Wi-Fi statistics report commands.
  *
  * This enumeration defines the commands to start or stop the collection of Wi-Fi statistics.
  */
typedef enum {
  SLI_WIFI_START_STATISTICS_REPORT, ///< Start statistics report
  SLI_WIFI_STOP_STATISTICS_REPORT,  ///< Stop statistics report
} sli_wifi_statistics_report_t;
//! @endcond

/**
  * @enum sl_wifi_band_mode_t
  * @brief Band mode.
  * @note Only 2.4 GHz is currently supported.
  */
typedef enum {
  SL_WIFI_BAND_MODE_2_4GHZ = 0, ///< 2.4 GHz Wi-Fi band
  SL_WIFI_BAND_MODE_5GHZ   = 1, ///< 5 GHz Wi-Fi band (not currently supported)
  SL_WIFI_DUAL_BAND_MODE   = 2  ///< Both 2.4 GHz and 5 GHZ WiFi band (not currently supported)
} sl_wifi_band_mode_t;

/**
  * @enum sl_wifi_region_code_t
  * 
  * @brief
  * Enumeration of Wi-Fi region codes.
  * 
  * @details
  * Guidance for Region code Mapping for Different Countries
  * | Country         | Country Code  |  Max power (Based on Regulatory domain)   | Frequency Range (Based on Regulatory Domain) | Suggested Region Code Mapping |
  * |:----------------|:--------------|:------------------------------------------|:---------------------------------------------|:------------------------------|
  * | Korea           | KR            | 23 dBm                                    | 2400 - 2483.5                                | SL_WIFI_REGION_KR             |
  * | Hong Kong       | HK            | 36 dBm                                    | 2400 - 2483.5                                | SL_WIFI_REGION_EU             |
  * | Singapore       | SG            | 200 mW (23 dBm)                           | 2400 - 2483.5                                | SL_WIFI_REGION_EU             |
  * | Malaysia        | MY            | 500 mW (27 dBm)                           | 2402 - 2482                                  | SL_WIFI_REGION_EU             |
  * | Australia       | AU            | 4000 mW (36 dBm)                          | 2400 - 2483.5                                | SL_WIFI_REGION_EU             |
  * | Taiwan          | TW            | 30 dBm                                    | 2400 - 2483.5                                | SL_WIFI_REGION_EU             |
  * | Thailand        | TH            | 20 dBm                                    | 2402 - 2482                                  | SL_WIFI_REGION_EU             |
  * | Mexico          | MX            | 20 dBm                                    | 2402 - 2482                                  | SL_WIFI_REGION_EU             |
  * | Vietnam         | VN            | 20 dBm                                    | 2402 - 2482                                  | SL_WIFI_REGION_EU             |
  * | Indonesia       | ID            | 500mW (27 dBm)                            | 2400 - 2483.5                                | SL_WIFI_REGION_EU             |
  * | China           | CN            | 20 dBm                                    | 2400 - 2483.5                                | SL_WIFI_REGION_CN             |
  *
  * @note `SL_WIFI_IGNORE_REGION` This option will be deprecated in future releases.
  **/
typedef enum {
  SL_WIFI_DEFAULT_REGION,      ///< Factory default region
  SL_WIFI_REGION_US,           ///< United States
  SL_WIFI_REGION_EU,           ///< European Union
  SL_WIFI_REGION_JP,           ///< Japan
  SL_WIFI_REGION_WORLD_DOMAIN, ///< Worldwide domain
  SL_WIFI_REGION_KR,           ///< Korea
  SL_WIFI_REGION_SG,           ///< Singapore (not currently supported)
  SL_WIFI_REGION_CN,           ///< China
  SL_WIFI_IGNORE_REGION        ///< @deprecated This option will be deprecated in future releases.
} sl_wifi_region_code_t;

/**
  * @enum sl_wifi_vap_id_t
  * @brief Wi-Fi VAP ID
  */
typedef enum {
  SL_WIFI_CLIENT_VAP_ID,   ///< Wi-Fi Client VAP ID
  SL_WIFI_AP_VAP_ID,       ///< Wi-Fi Access point VAP ID
  SL_WIFI_CLIENT_VAP_ID_1, ///< Wi-Fi Client 1 VAP ID
  SL_WIFI_AP_VAP_ID_1,     ///< Wi-Fi Access point 1 VAP ID
} sl_wifi_vap_id_t;

/**
  * @enum sl_wifi_ap_keepalive_type_t
  * @brief Keepalive types
  */
typedef enum {
  SL_WIFI_AP_KEEP_ALIVE_DISABLE = 0, ///< Disable keepalive functionality.
  SL_WIFI_AP_DEAUTH_BASED_KEEP_ALIVE =
    1, ///< AP performs keepalive functionality based on the RX packets received from its stations.
  ///< If no packet is received from the station within the AP keep alive timeout period, the AP disconnects the station.
  SL_WIFI_AP_NULL_BASED_KEEP_ALIVE =
    3 ///< AP performs keepalive functionality by sending a NULL DATA packet to the station.
      ///< If no ACK is received from the station after a specific number of retries, the AP discards the station.
} sl_wifi_ap_keepalive_type_t;

/**
  * @def SL_WIFI_AUTO_CHANNEL
  * @brief Macro to enable Auto Channel Selection (ACS).
  *
  * This macro defines the value to enable the Auto Channel Selection (ACS) feature, which automatically determines the operating channel for the Access Point (AP).
  *
  * @note
  * - The channel in which the AP operates. A value of zero enables the ACS feature.
  * - If ACS is enabled, AP start may take approximately 9 seconds as the device scans all channels to select the best channel.
  */
#define SL_WIFI_AUTO_CHANNEL 0

/**
  * @def SL_WIFI_DEFAULT_CHANNEL_BITMAP
  * @brief Macro to define the default channel bitmap for scanning.
  *
  * @note 
  * - The selected or preferred channels to be scanned before the client connects to the AP. A value of zero means to scan on all available channels.
  */
#define SL_WIFI_DEFAULT_CHANNEL_BITMAP 0

/**
  * @def SL_WIFI_ARGS_CHECK_NULL_POINTER(ptr)
  * @brief Macro to check for null pointers in API inputs.
  */
#define SL_WIFI_ARGS_CHECK_NULL_POINTER(ptr) \
  do {                                       \
    if (ptr == NULL) {                       \
      return SL_STATUS_NULL_POINTER;         \
    }                                        \
  } while (0)

/**
  * @def SL_WIFI_ARGS_CHECK_INVALID_INTERFACE(interface)
  * @brief Macro to check for invalid Wi-Fi interfaces in API inputs.
  */
#define SL_WIFI_ARGS_CHECK_INVALID_INTERFACE(interface)                                                   \
  {                                                                                                       \
    if (!((interface == SL_WIFI_CLIENT_INTERFACE) || (interface == SL_WIFI_AP_INTERFACE)                  \
          || (interface == SL_WIFI_CLIENT_2_4GHZ_INTERFACE) || (interface == SL_WIFI_AP_2_4GHZ_INTERFACE) \
          || (interface == SL_WIFI_2_4GHZ_INTERFACE))) {                                                  \
      return SL_STATUS_WIFI_UNKNOWN_INTERFACE;                                                            \
    }                                                                                                     \
  }

/**
  * @enum sl_wifi_cw_tone_frequency_t
  * @brief CW Tone Frequency.
  *
  * Frequency used for Continuous Wave (CW) Tone transmission in Wi-Fi test modes.
  */
typedef enum __attribute__((packed)) {
  SL_WIFI_CW_TONE_FREQUENCY_312500HZ = 0,    ///< CW Tone Frequency 312500Hz
  SL_WIFI_CW_TONE_FREQUENCY_612KHZ   = 1,    ///< CW Tone Frequency 612KHz
  SL_WIFI_CW_TONE_FREQUENCY_1250KHZ  = 2,    ///< CW Tone Frequency 1250KHz
  SL_WIFI_CW_TONE_FREQUENCY_2500KHZ  = 3,    ///< CW Tone Frequency 2500KHz
  SL_WIFI_CW_TONE_FREQUENCY_5000KHZ  = 4,    ///< CW Tone Frequency 5000KHz
  SL_WIFI_CW_TONE_FREQUENCY_INVALID  = 0xFF, ///< Invalid CW Tone Frequency
} sl_wifi_cw_tone_frequency_t;

/**
  * @enum sl_wifi_dc_val_iq_t
  * @brief DC value for I/Q signals.
  *
  * DC value for I/Q signals.
  *
  */
typedef enum __attribute__((packed)) {
  SL_WIFI_DC_VAL_IQ_NODATA  = 0x0,    ///< DC Value IQ No Data
  SL_WIFI_DC_VAL_IQ_DEFAULT = 0x80,   ///< DC Value IQ Default
  SL_WIFI_DC_VAL_IQ_INVALID = 0xFFFF, ///< Invalid DC Value IQ
} sl_wifi_dc_val_iq_t;

/**
  * @enum sl_wifi_cw_tone_amplitude_t
  * @brief CW Tone Amplitude.
  *
  * Amplitude used for Continuous Wave (CW) Tone transmission in Wi-Fi test modes.
  *
  */
typedef enum __attribute__((packed)) {
  SL_WIFI_CW_TONE_AMPLITUDE_1000MV  = 0x0,  ///< Tone Scale Value 1000MV peak amplitude
  SL_WIFI_CW_TONE_AMPLITUDE_500MV   = 0x1,  ///< Tone Scale Value 500MV peak amplitude
  SL_WIFI_CW_TONE_AMPLITUDE_250MV   = 0x2,  ///< Tone Scale Value 250MV peak amplitude
  SL_WIFI_CW_TONE_AMPLITUDE_125MV   = 0x3,  ///< Tone Scale Value 125MV peak amplitude
  SL_WIFI_CW_TONE_AMPLITUDE_INVALID = 0xFF, ///<  Invalid Tone scale value
} sl_wifi_cw_tone_amplitude_t;

/** @} */

#endif // _SL_WIFI_CONSTANTS_H_
