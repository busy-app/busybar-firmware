/********************************************************************************
 * @file  sl_wifi_types.h
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

#pragma once

#include "sl_wifi_constants.h"
#include "sl_ieee802_types.h"
#include "sl_constants.h"
#include "sl_status.h"
#include "sl_ip_types.h"
#include "sl_slist.h"
#include <stdint.h>

// Default Timeout Configuration
#define SL_WIFI_DEFAULT_AUTH_ASSOCIATION_TIMEOUT  0xFFFF
#define SL_WIFI_DEFAULT_ACTIVE_CHANNEL_SCAN_TIME  0xFFFF
#define SL_WIFI_DEFAULT_KEEP_ALIVE_TIMEOUT        0xFFFF
#define SL_WIFI_DEFAULT_PASSIVE_CHANNEL_SCAN_TIME 0

/// Wi-Fi transceiver mode configurations
#define MAX_PAYLOAD_LEN                     2020
#define MAC_INFO_ENABLE                     BIT(0)
#define BCAST_INDICATION                    BIT(1)
#define CONFIRM_REQUIRED_TO_HOST            BIT(2)
#define QOS_ENABLE                          BIT(4)
#define MAC80211_HDR_MIN_LEN                24
#define MAC80211_HDR_QOS_CTRL_LEN           2
#define MAC80211_HDR_ADDR4_LEN              6
#define WME_AC_BE                           0 /* best effort */
#define WME_AC_BK                           1 /* background */
#define WME_AC_VI                           2 /* video */
#define WME_AC_VO                           3 /* voice */
#define WME_AC_TO_TID(_ac)                  (((_ac) == WME_AC_VO) ? 6 : ((_ac) == WME_AC_VI) ? 5 : ((_ac) == WME_AC_BK) ? 1 : 0)
#define WME_AC_TO_QNUM(_ac)                 (((_ac) == WME_AC_BK) ? 0 : ((_ac) == WME_AC_BE) ? 1 : ((_ac) == WME_AC_VI) ? 2 : 3)
#define FC_TYPE_DATA                        BIT(3)
#define FC_SUBTYPE_QOS_DATA                 BIT(7)
#define FC_TO_DS                            BIT(8)
#define FC_FROM_DS                          BIT(9)
#define TX_DATA_CTRL_FLAG_QOS_BIT           BIT(1)
#define IS_QOS_PKT(ctrl_flags)              (ctrl_flags & TX_DATA_CTRL_FLAG_QOS_BIT)
#define IS_PEER_DS_SUPPORT_ENABLED(bitmap)  (bitmap & SL_WIFI_FEAT_TRANSCEIVER_MAC_PEER_DS_SUPPORT)
#define IS_4ADDR(ctrl_flags)                (ctrl_flags & BIT(0))
#define IS_FIXED_DATA_RATE(ctrl_flags)      (ctrl_flags & BIT(2))
#define IS_TODS(ctrl_flags)                 (ctrl_flags & BIT(3))
#define IS_FROMDS(ctrl_flags)               (ctrl_flags & BIT(4))
#define IS_CFM_TO_HOST_SET(ctrl_flags)      (ctrl_flags & BIT(5))
#define IS_BCAST_MCAST_MAC(addr)            (addr & BIT(0))
#define IS_MAC_ZERO(mac)                    (!(mac[0] | mac[1] | mac[2] | mac[3] | mac[4] | mac[5]))
#define MAX_RETRANSMIT_COUNT                15
#define MAX_CW_EXPN_COUNT                   15
#define MAX_AIFSN                           15
#define TRANSCEIVER_PEER_ADD_FLAG           BIT(0)
#define TRANSCEIVER_PEER_AUTO_RATE_FLAG     BIT(1)
#define TRANSCEIVER_MCAST_FILTER_EN         BIT(0)
#define TRANSCEIVER_MCAST_FILTER_ADDR_LIMIT 2
#define TRANSCEIVER_TX_DATA_EXT_DESC_SIZE   4
#define SL_STATUS_ACK_ERR                   0x1
#define SL_STATUS_CS_BUSY                   0x2
#define SL_STATUS_UNKNOWN_PEER              0x3
#define TRANSCEIVER_RX_PKT_TA_MATCH_BIT     BIT(20)
#define SL_WIFI_SSID_LEN                    34

/** @addtogroup SL_WIFI_CONSTANTS
  * @{ */
#define SL_WIFI_TRANSCEIVER_CHANNEL_NO               14 ///< Wi-Fi transceiver default channel
#define SL_WIFI_TRANSCEIVER_TX_POWER                 20 ///< Wi-Fi transceiver default TX power
#define SL_WIFI_TRANSCEIVER_DEFAULT_RETRANSMIT_COUNT 15 ///< Wi-Fi transceiver default retransmit count
#define SL_WIFI_TRANSCEIVER_DEFAULT_QOS_BE_CWMIN     4  ///< Wi-Fi transceiver default BE cwmin contention param value
#define SL_WIFI_TRANSCEIVER_DEFAULT_QOS_BE_CWMAX     6  ///< Wi-Fi transceiver default BE cwmax contention param value
#define SL_WIFI_TRANSCEIVER_DEFAULT_QOS_BE_AIFSN     3  ///< Wi-Fi transceiver default BE aifsn contention param value
#define SL_WIFI_TRANSCEIVER_DEFAULT_QOS_BK_CWMIN     4  ///< Wi-Fi transceiver default BK cwmin contention param value
#define SL_WIFI_TRANSCEIVER_DEFAULT_QOS_BK_CWMAX     10 ///< Wi-Fi transceiver default BK cwmax contention param value
#define SL_WIFI_TRANSCEIVER_DEFAULT_QOS_BK_AIFSN     7  ///< Wi-Fi transceiver default BK aifsn contention param value
#define SL_WIFI_TRANSCEIVER_DEFAULT_QOS_VI_CWMIN     3  ///< Wi-Fi transceiver default VI cwmin contention param value
#define SL_WIFI_TRANSCEIVER_DEFAULT_QOS_VI_CWMAX     4  ///< Wi-Fi transceiver default VI cwmax contention param value
#define SL_WIFI_TRANSCEIVER_DEFAULT_QOS_VI_AIFSN     1  ///< Wi-Fi transceiver default VI aifsn contention param value
#define SL_WIFI_TRANSCEIVER_DEFAULT_QOS_VO_CWMIN     2  ///< Wi-Fi transceiver default VO cwmin contention param value
#define SL_WIFI_TRANSCEIVER_DEFAULT_QOS_VO_CWMAX     3  ///< Wi-Fi transceiver default VO cwmax contention param value
#define SL_WIFI_TRANSCEIVER_DEFAULT_QOS_VO_AIFSN     1  ///< Wi-Fi transceiver default VO aifsn contention param value
/** @} */

// Vendor-specific IE Auto-Assign identifier. Use this value to request firmware to automatically
// assign an available memory for the vendor-specific IE. The actual assigned unique id will be returned
// in the fw_unique_id field of the sl_wifi_vendor_ie_t structure after successful addition.
#define SL_WIFI_VENDOR_IE_AUTO_ASSIGN 0

// Management frame bitmap for vendor-specific IE feature.
#define SL_WIFI_VENDOR_IE_IN_BEACON     (1U << 0)
#define SL_WIFI_VENDOR_IE_IN_PROBE_RESP (1U << 1)
#define SL_WIFI_VENDOR_IE_IN_ASSOC_RESP (1U << 2)
#define SL_WIFI_VENDOR_IE_IN_PROBE_REQ  (1U << 3)
#define SL_WIFI_VENDOR_IE_IN_ASSOC_REQ  (1U << 4)

/** @addtogroup SL_WIFI_TYPES Types
  * @{ */

/**
 * @struct sl_wifi_buffer_t
 * @brief Structure representing a Wi-Fi buffer.
 */
typedef struct {
  sl_slist_node_t node; ///< Pointer to the node of the list of which the buffer is part of
  uint32_t length;      ///< Size of the buffer in bytes
  uint8_t
    type; ///< Indicates the buffer type (SL_WIFI_TX_FRAME_BUFFER, SL_WIFI_RX_FRAME_BUFFER, and so on.) corresponding to the buffer.
  uint8_t id;           ///< Buffer identifier. Can be used to uniquely identify a buffer. Loops every 256 packets.
  uint8_t _reserved[2]; ///< Reserved.
  uint8_t data[];       ///< Stores the data (header + payload) to be send to NWP
} sl_wifi_buffer_t;

/**
 * @typedef sl_wifi_event_handler_t
 * @brief Generic callback for handling Wi-Fi events.
 *
 * @param event
 *   Wi-Fi event of type @ref sl_wifi_event_t.
 * @param buffer
 *   Pointer to a Wi-Fi buffer contains information related to the event, of type @ref sl_wifi_buffer_t
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   In case of event failure, SL_WIFI_FAIL_EVENT_STATUS_INDICATION bit is set in the event.
 *   The data would be of type sl_status_t, and data_length can be ignored.
 */
typedef sl_status_t (*sl_wifi_event_handler_t)(sl_wifi_event_t event, sl_wifi_buffer_t *buffer);

/**
 * @typedef sl_wifi_credential_id_t
 * @brief Wi-Fi credential identifier type.
 * 
 * @details
 * This type stores credential identifiers for Wi-Fi configurations.
 *
 * @note 
 * For open security networks (SL_WIFI_OPEN), use SL_WIFI_NO_CREDENTIAL_ID.
 */
typedef uint32_t sl_wifi_credential_id_t;

/**
 * @struct sl_wifi_channel_t
 * @brief Wi-Fi channel configuration.
 */
typedef struct {
  uint16_t channel;              ///< Channel number
  sl_wifi_band_t band;           ///< Wi-Fi radio band of type @ref sl_wifi_band_t
  sl_wifi_bandwidth_t bandwidth; ///< Channel bandwidth of type @ref sl_wifi_bandwidth_t
} sl_wifi_channel_t;

/**
 * @struct sl_wifi_ssid_t
 * @brief SSID data structure.
 *
 * Specifies the Service Set Identifier (SSID) used in Wi-Fi networks.
 */
typedef struct {
  uint8_t value[32]; ///< SSID value
  uint8_t length;    ///< Length of the SSID
} sl_wifi_ssid_t;

/**
 * @struct sl_wifi_roam_configuration_t
 * @brief Wi-Fi roaming configuration structure.
 */
typedef struct {
  int32_t
    trigger_level; ///< RSSI level to trigger the roam algorithm, setting the value to SL_WIFI_NEVER_ROAM disables roaming configuration
  uint32_t trigger_level_change; ///< RSSI level delta change to trigger the roam algorithm
} sl_wifi_roam_configuration_t;

/**
 * @struct sl_wifi_firmware_version_t
 * @brief Wi-Fi firmware version information.
 *
 * Contains the detailed information about the Wi-Fi firmware version.
 */
typedef struct {
  uint8_t chip_id;          ///< Chip ID
  uint8_t rom_id;           ///< ROM ID
  uint8_t major;            ///< Major version number
  uint8_t minor;            ///< Minor version number
  uint8_t security_version; ///< Security enabled or disabled
  uint8_t patch_num;        ///< Patch number
  uint8_t customer_id;      ///< Customer ID
  uint16_t build_num;       ///< Build number
} sl_wifi_firmware_version_t;

/**
 * @struct sl_wifi_scan_result_t
 * @brief Wi-Fi scan result structure.
 *
 * Indicates the results of a Wi-Fi scan operation.
 */
typedef struct {
  uint32_t scan_count; ///< Number of available scan results
  uint32_t reserved;   ///< Reserved
  struct {
    uint8_t rf_channel;    ///< Channel number of the AP
    uint8_t security_mode; ///< Security mode of the AP
    uint8_t rssi_val;      ///< RSSI value of the AP
    uint8_t network_type;  ///< AP network type
    uint8_t ssid[34];      ///< SSID of the AP
    uint8_t bssid[6];      ///< BSSID of the AP
    uint8_t reserved[2];   ///< Reserved
  } scan_info[];           ///< Array of scan result data
} sl_wifi_scan_result_t;

/// Extended Wi-Fi scan result
typedef struct {
  uint8_t rf_channel;    ///< Channel number of the AP
  uint8_t security_mode; ///< Security mode of the AP
  uint8_t rssi;          ///< RSSI value of the AP
  uint8_t network_type;  ///< Network type of the AP
  uint8_t ssid[34];      ///< SSID of the AP
  uint8_t bssid[6];      ///< BSSID of the AP
} sl_wifi_extended_scan_result_t;

/// Extended Wi-Fi scan result parameters
typedef struct {
  sl_wifi_extended_scan_result_t
    *scan_results;         ///< Pointer to an array contains scan results of type @ref sl_wifi_extended_scan_result_t
  uint16_t array_length;   ///< Length of the scan results array provided by the user.
  uint16_t *result_count;  ///< Pointer to store the total count of scan results returned.
  uint8_t *channel_filter; ///< Pointer to Channel number (Filter based on Channel number of the AP).
  uint8_t *security_mode_filter; ///< Pointer to Security mode (Filter based on the Security mode of the AP).
  uint8_t *rssi_filter; ///< Pointer to RSSI (Filter for APs with an RSSI greater than or equal to given RSSI value).
  uint8_t *network_type_filter; ///< Pointer to Network type (Filter based on APs network type).
} sl_wifi_extended_scan_result_parameters_t;

/**
 * @struct sl_wifi_scan_configuration_t
 * @brief Wi-Fi scan configuration structure.
 *
 * Indicates the configuration parameters for a Wi-Fi scan operation.
 *
 * @note The Quick Scan feature is enabled when a specific channel and SSID are given for scanning.
 *       SiWx91x scans for the AP given in the scan API and posts the scan results immediately
 *       after finding the access point.
 * @note The `channel_bitmap_2g4` uses the lower 14 bits to represent channels from 1 to 14,
 *       where channel 1 = (1 << 0), channel 2 = (1 << 1), and so on.
 * @note When `channel_bitmap_2g4` is not set (value is 0), the device will scan all available channels.
 * @note `periodic_scan_interval` is only applicable for SL_WIFI_SCAN_TYPE_ADV_SCAN of type @ref sl_wifi_scan_type_t.
 * @note To configure active_channel_time and passive_channel_time for active and passive scans, 
 *       use the sl_wifi_set_advanced_scan_configuration() API with @ref sl_wifi_advanced_scan_configuration_t.
 * @note Channel scanning behavior:
 *       - For active scans: Scans the channels specified in `channel_bitmap_2g4`. If set to 0, scans all available channels. Can be configured as a subset of channels.
 *       - For background scans (SL_WIFI_SCAN_TYPE_ADV_SCAN): Scans the channels that were originally specified in `channel_bitmap_2g4` during the first scan. The channel bitmap cannot be modified for background scans.
 * @note 5 GHz is not supported.
 *
 * | Channel Number 2.4 GHz | channel_bitmap_2g4    |
 * |------------------------|-----------------------|
 * | 1                      | (1 << 0)              |
 * | 2                      | (1 << 1)              |
 * | 3                      | (1 << 2)              |
 * | 4                      | (1 << 3)              |
 * | 5                      | (1 << 4)              |
 * | 6                      | (1 << 5)              |
 * | 7                      | (1 << 6)              |
 * | 8                      | (1 << 7)              |
 * | 9                      | (1 << 8)              |
 * | 10                     | (1 << 9)              |
 * | 11                     | (1 << 10)             |
 * | 12                     | (1 << 11)             |
 * | 13                     | (1 << 12)             |
 * | 14                     | (1 << 13)             |
 */
typedef struct {
  sl_wifi_scan_type_t type;        ///< Scan type to be configured of type @ref sl_wifi_scan_type_t
  uint32_t flags;                  ///< Reserved
  uint32_t periodic_scan_interval; ///< Duration in milliseconds between periodic scans
  uint16_t channel_bitmap_2g4;     ///< Bitmap of selected 2.4 GHz channels
  uint32_t channel_bitmap_5g[8];   ///< Bitmap of selected 5 GHz channels (currently not supported)
  uint8_t lp_mode;                 ///< Enable LP mode, 1 - Enable LP mode, 0 - Disable LP mode
} sl_wifi_scan_configuration_t;

/**
 * @struct sl_wifi_advanced_scan_configuration_t
 * @brief Wi-Fi advanced scan configuration options.
 *
 * Indicates the configuration parameters for an advanced Wi-Fi scan operation.
 *
 * @note active_channel_time and passive_channel_time are applicable for foreground scan (station before connection).
 * @note If active_channel_time and passive_channel_time are not set or set to 0, default values are used.
 *       Default value of 100 msec is used for active_channel_time when SL_WIFI_DEFAULT_ACTIVE_CHANNEL_SCAN_TIME is passed.
 *       Default value of 400 msec is used for passive_channel_time when SL_WIFI_DEFAULT_PASSIVE_CHANNEL_SCAN_TIME is passed.
 * @note trigger_level and trigger_level_change are used for automatic roaming functionality. When connected to an AP:
 *       - If RSSI drops below trigger_level, a background scan is automatically initiated to find better APs for roaming
 *       - If RSSI drops by trigger_level_change delta from the previous measurement, a background scan is triggered
 *       - The background scan operates independently and helps facilitate seamless roaming to better APs when available
 */
typedef struct {
  int32_t
    trigger_level; ///< RSSI threshold level to trigger background scan for roaming. When connected to an AP, if the RSSI drops below this level, a background scan is automatically triggered to find better APs for potential roaming.
  uint32_t
    trigger_level_change; ///< RSSI delta change threshold to trigger background scan for roaming. If the current RSSI value drops by this delta amount from the previous measurement, a background scan is triggered to find better APs for potential roaming.
  uint16_t active_channel_time;  ///< Time spent on each channel during active scan (milliseconds)
  uint16_t passive_channel_time; ///< Time spent on each channel during passive scan (milliseconds)
  uint8_t enable_instant_scan;   ///< Flag to start advanced scan immediately
  uint8_t
    enable_multi_probe; ///< Flag to send multiple probes to AP. If the value is set to 1, a probe request would be sent to all access points in addition to the connected SSID.
} sl_wifi_advanced_scan_configuration_t;

/**
 * @struct sl_wifi_ap_configuration_t
 * @brief Wi-Fi Access Point configuration structure.
 *
 * Indicates the configuration parameters for setting up a Wi-Fi Access Point (AP).
 * 
 * @note When configuring AP interface in open security mode, the credential ID must be set to `SL_WIFI_NO_CREDENTIAL_ID`.
 * @note The security field refers to the security type of the Wi-Fi AP.
 * In Wi-Fi access-point mode, the AP supports OPEN, WPA-PSK, WPA2-PSK, WPA/WPA2 Mixed, and WPA3 security modes.
 * Valid configurations are:
 * - SL_WIFI_OPEN                       - For OPEN security mode (no encryption)
 * - SL_WIFI_WPA                        - For WPA-PSK security mode
 * - SL_WIFI_WPA2                       - For WPA2-PSK security mode
 * - SL_WIFI_WPA_WPA2_MIXED             - For WPA/WPA2 mixed security mode
 * - SL_WIFI_WPA3                       - For WPA3 security mode
 *
 * @note client_idle_timeout - This is the period after which the AP will disconnect the station if there are no wireless exchanges from the station to the AP. The keep-alive period is calculated in terms of 32 multiples of the beacon interval (i.e, if there are no wireless transfers from station to AP within a
 * (32 x beacon_interval) milliseconds time period, the station will be disconnected).
 * If null data-based method is selected, the AP checks the connectivity of the station by sending null data packet. If the station does not acknowledge the packet, that station will be disconnected from the AP after 4 retries.
 * The maximum valid range supported is up to 255.
 */
typedef struct {
  sl_wifi_ssid_t ssid;                   ///< SSID (Service Set Identifier) of the Access Point
  sl_wifi_security_t security;           ///< Security mode of the Access Point
  sl_wifi_encryption_t encryption;       ///< Encryption mode of the Access Point
  sl_wifi_channel_t channel;             ///< Channel configuration of the Access Point
  sl_wifi_rate_protocol_t rate_protocol; ///< Rate protocol of the Access Point
  sl_wifi_ap_flag_t
    options; ///< Optional flags for AP configuration. @note Dynamic configurability of hidden SSIDs is only available in APCONF when it is disabled in opermode.
  sl_wifi_credential_id_t credential_id; ///< ID of secure credentials
  uint8_t
    keepalive_type; ///< Keep alive type of the access point. One of the values from [sl_wifi_ap_keepalive_type_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91-x-types#sl-si91x-ap-keepalive-type-t)
  uint16_t beacon_interval;     ///< Beacon interval of the access point in milliseconds
  uint32_t client_idle_timeout; ///< Period after which AP will disconnect the station
  uint16_t dtim_beacon_count;   ///< Number of beacons per DTIM
  uint8_t
    maximum_clients; ///< The maximum number of associated clients must not exceed [SL_WIFI_CUSTOM_FEAT_MAX_NUM_OF_CLIENTS](../wiseconnect-api-reference-guide-si91x-driver/si91-x-custom-feature-bitmap#sl-si91x-custom-feat-max-num-of-clients). If this bit is not set in device configuration, the default maximum is 8 clients in AP-only mode and 4 clients in concurrent mode.
  uint8_t beacon_stop; ///< Flag to stop beaconing when there are no associated clients
  sl_wifi_tdi_t
    tdi_flags; ///< Flags to enable Transition Disable Indication (TDI). One of the values from @ref sl_wifi_tdi_t
  uint8_t is_11n_enabled; ///< A flag to enable 11n.
} sl_wifi_ap_configuration_t;

/**
 * @struct sl_wifi_advanced_ap_configuration_t
 * @brief Wi-Fi Access Point advanced configuration structure.
 *
 * Indicates the advanced configuration parameters for a Wi-Fi Access Point (AP).
 */
typedef struct {
  uint8_t csa_announcement_delay; ///< In beacon periods
  uint32_t tbd;                   ///< Advanced configuration option to be added
} sl_wifi_advanced_ap_configuration_t;

/**
 * @struct sl_wifi_channel_bitmap_t
 * @brief Channel bitmap for scanning in a set of selective channels.
 *
 * @note A 2.4 GHz channel is enabled by setting the bit of the corresponding channel number minus 1.
 * For example, for channel 1, set bit 0; 
 				for channel 2, set bit 1, and so on. @ref sl_wifi_scan_configuration_t
 * @note 5 GHz chnannels are not supported.
 */
typedef struct {
  uint16_t channel_bitmap_2_4; ///< Channel bitmap for scanning in a set of selective channels in 2.4 GHz.
  uint32_t
    channel_bitmap_5; ///< Channel bitmap for scanning in a set of selective channels in 5 GHz. (Currently not supported).
} sl_wifi_channel_bitmap_t;

/**
 * @struct sl_wifi_client_configuration_t
 * @brief Wi-Fi Client interface configuration structure.
 *
 * Defines the configuration parameters for a Wi-Fi client interface.
 * 
 * @note When configuring client interface in open security mode, the credential ID must be set to `SL_WIFI_NO_CREDENTIAL_ID`.
 * 
 * @note The security field refers to the security type for the Wi-Fi Station (STA) interface.
 * @note In Wi-Fi concurrent mode, the client interface supports Open, WPA-PSK, WPA2-PSK, WPA/WPA2 Mixed, WPA3, and WPA3 Transition security (WPA2/WPA3) modes.
 * Valid configurations are:
 * - SL_WIFI_OPEN                       - For Open security mode (no encryption)
 * - SL_WIFI_WPA                        - For WPA security mode
 * - SL_WIFI_WPA2                       - For WPA2 security mode
 * - SL_WIFI_WPA_WPA2_MIXED             - For WPA/WPA2 mixed security mode
 * - SL_WIFI_WPA3                       - For WPA3 security mode
 * - SL_WIFI_WPA3_TRANSITION            - For WPA3 Transition security mode
 * 
 *  @note  In Wi-Fi enterprise client mode, the client interface supports WPA-Enterprise, WPA2-Enterprise, WPA3-Enterprise, and WPA3 Transition Enterprise (WPA2/WPA3 Enterprise) security modes.
 * Valid configurations are:
 * - SL_WIFI_WPA_ENTERPRISE                         - For WPA Enterprise security mode
 * - SL_WIFI_WPA2_ENTERPRISE                        - For WPA2 Enterprise mode
 * - SL_WIFI_WPA3_ENTERPRISE                        - For WPA3 Enterprise security mode
 * - SL_WIFI_WPA3_TRANSITION_ENTERPRISE             - For WPA3 Transition Enterprise Security mode
 * 
 * @note In Wi-Fi personal client mode, the client interface supports Open, WPA-PSK, WPA2-PSK, WPA/WPA2 Mixed, WPA3, and WPA3 Transition (WPA2/WPA3) security modes.
 * Valid configurations are:
 * - SL_WIFI_OPEN                       - For Open security mode (no encryption)
 * - SL_WIFI_WPA                        - For WPA security mode
 * - SL_WIFI_WPA2                       - For WPA2 security mode
 * - SL_WIFI_WPA_WPA2_MIXED             - For WPA/WPA2 mixed security mode
 * - SL_WIFI_WPA3                       - For WPA3 security mode
 * - SL_WIFI_WPA3_TRANSITION            - For WPA3 Transition security mode
 */
typedef struct {
  sl_wifi_ssid_t ssid; ///< SSID (Service Set Identifier) of the Wi-Fi network. This is of type @ref sl_wifi_ssid_t
  sl_wifi_channel_t channel; ///< The channel configuration of the Wi-Fi network. This is of type @ref sl_wifi_channel_t
  sl_mac_address_t bssid;    ///< BSSID of the Wi-Fi network
  sl_wifi_bss_type_t bss_type;     ///< BSS type of the Wi-Fi network
  sl_wifi_security_t security;     ///< Security mode of type @ref sl_wifi_security_t
  sl_wifi_encryption_t encryption; ///< Encryption mode of the Wi-Fi network. This is of type @ref sl_wifi_encryption_t
  sl_wifi_client_flag_t client_options;  ///< Optional flags for client configuration of type @ref sl_wifi_client_flag_t
  sl_wifi_credential_id_t credential_id; ///< ID of secure credentials of type @ref sl_wifi_credential_id_t
  sl_wifi_channel_bitmap_t channel_bitmap; ///< Channel bitmap for scanning of type @ref sl_wifi_channel_bitmap_t
} sl_wifi_client_configuration_t;

/**
 * @struct sl_wifi_advanced_client_configuration_t
 * @brief Wi-Fi Client interface advanced configuration structure.
 * @note  The default beacon missed count is set to 40. A unicast probe request is sent from the module to the Access Point (AP) at the 21st beacon count and again at the 31st beacon count.
 */
typedef struct {
  uint32_t max_retry_attempts;  ///< Maximum number of retries before indicating join failure
  uint32_t scan_interval;       ///< Scan interval in seconds between each retry
  uint32_t beacon_missed_count; ///< Number of missed beacons that will trigger rejoin
  uint32_t
    first_time_retry_enable; ///< Enable or disable retry attempts for initial join failures. If set to 0 (default), no retry attempts are made for first-time join failures - the API will return failure immediately. If set to 1, retry attempts will be made using max_retry_attempts and scan_interval parameters. Note: This setting only affects initial connection attempts; roaming and background scan disconnections will always use the retry mechanism regardless of this setting.
} sl_wifi_advanced_client_configuration_t;

/**
 * @struct sl_wifi_psk_credential_t
 * @brief Wi-Fi PSK (Pre-Shared Key) security credentials structure.
 *
 * Specifies the PSK security credentials used for Wi-Fi connections.
 * The PSK is used for authenticating and securing the Wi-Fi connection.
 */
typedef struct {
  uint8_t value[SL_WIFI_MAX_PSK_LENGTH]; ///< PSK buffer
} sl_wifi_psk_credential_t;

/**
 * @struct sl_wifi_pmk_credential_t
 * @brief Wi-Fi PMK (Pairwise Master Key) security credentials structure.
 *
 * Specifies the PMK security credentials used for Wi-Fi connections.
 * The PMK is used for authenticating and securing the Wi-Fi connection.
 */
typedef struct {
  uint8_t value[SL_WIFI_MAX_PMK_LENGTH]; ///< PMK buffer
} sl_wifi_pmk_credential_t;

/**
 * @struct sl_wifi_wep_credential_t
 * @brief Wi-Fi WEP (Wired Equivalent Privacy) security credentials structure.
 *
 * Specifies the WEP security credentials used for Wi-Fi connections.
 * These keys are used for authentication and securing the Wi-Fi connection.
 */
typedef struct {
  uint8_t index[2];                                           ///< Index of the active WEP key
  uint8_t key[SL_WIFI_WEP_KEY_COUNT][SL_WIFI_WEP_KEY_LENGTH]; ///< WEP Keys
} sl_wifi_wep_credential_t;

/**
 * @struct sl_wifi_eap_credential_t
 * @brief Wi-Fi Enterprise security credentials structure.
 *
 * Specifies the security credentials used for Wi-Fi Enterprise authentication.
 */
typedef struct {
  uint8_t username[SL_WIFI_EAP_USER_NAME_LENGTH];              ///< Enterprise User Name
  uint8_t password[SL_WIFI_EAP_PASSWORD_LENGTH];               ///< Enterprise password
  uint8_t certificate_key[SL_WIFI_EAP_CERTIFICATE_KEY_LENGTH]; ///< Certificate password
  uint32_t certificate_id;                                     ///< Certificate Id for Enterprise authentication
  /**
	   * @note
     *  - BIT[0] of Opportunistic Key Caching (OKC) is used to enable or disable OKC:
     *    - 0 – disable
     *    - 1 – enable
     *    When this is enabled, the module uses cached PMKID to get the Master Session Key (MSK), which is needed for generating PMK that is needed for the 4-way handshake.
     * - BIT[1] of OKC is used to enable or disable CA certification for PEAP connection:
     *   – 0 – CA certificate is not required
     *   – 1 – CA certificate is required
	   * - BIT[2-12] of OKC argument are used for cipher list selection for EAP connection. All possible ciphers are listed further:
	   *       | BIT position| Cipher selected       |
	   *       |-------------|-----------------------|
	   *       | 2           | DHE-RSA-AES256-SHA256 |
	   *       | 3           | DHE-RSA-AES128-SHA256 |
	   *       | 4           | DHE-RSA-AES256-SHA    |
	   *       | 5           | DHE-RSA-AES128-SHA    |
	   *       | 6           | AES256-SHA256         |
	   *       | 7           | AES128-SHA256         |
	   *       | 8           | AES256-SHA            |
	   *       | 9           | AES128-SHA            |
	   *       | 10          | RC4-SHA               |
	   *       | 11          | DES-CBC3-SHA          |
	   *       | 12          | RC4-MD5               |
	   * - BIT[13-31] of OKC argument is reserved.
	   * @note If a user sets BIT[1] and does not provide the CA certificate for PEAP connection, an error is thrown. If a user provides an invalid CA certificate, an error is also thrown. A user can set either one or multiple bits from BIT[2-12] to provide the cipher list. If a user does not provide any values in OKC BIT[2-12], all ciphers are selected by default.
	   */
  uint32_t eap_flags; ///< EAP Flags of type @ref sl_wifi_eap_client_flag_t
} sl_wifi_eap_credential_t;

/**
 * @struct sl_wifi_credential_t
 * @brief Wi-Fi security credentials structure.
 */
typedef struct {
  sl_wifi_credential_type_t type; ///< Credential type
  union {
    sl_wifi_psk_credential_t psk; ///< Wi-Fi Personal credentials
    sl_wifi_pmk_credential_t pmk; ///< Wi-Fi PMK credentials
    sl_wifi_wep_credential_t wep; ///< WEP keys
    sl_wifi_eap_credential_t eap; ///< Enterprise client credentials
  };                              ///< Wi-Fi Credential structure
} sl_wifi_credential_t;

/**
 * @struct sl_wifi_twt_request_t
 * @brief TWT (Target Wake Time) request structure to configure a session.
 */
typedef struct {
  uint8_t wake_duration; ///< Nominal minimum wake duration. Range : 0 - 255
  uint8_t
    wake_duration_tol; ///< Tolerance allowed for wake duration in case of suggest TWT. Received TWT wake duration from AP validates against tolerance limits and decides if TWT config received is in acceptable range or not. Range : 0 - 255.
  uint8_t wake_int_exp; ///< Wake interval exponent to the base 2. Range : 0 - 31.
  uint8_t
    wake_int_exp_tol; ///< Tolerance allowed for wake_int_exp in case of suggest TWT request. Received TWT wake interval exponent from AP validates against tolerance limits and decides if TWT config received is in acceptable range or not. Range : 0 - 31.
  uint16_t wake_int_mantissa; ///< Wake interval mantissa. Range : 0 - 65535.
  uint16_t
    wake_int_mantissa_tol; ///< Tolerance allowed for wake_int_mantissa in case of suggest TWT. Received TWT wake interval mantissa from AP validates against tolerance limits and decides if TWT config received is in acceptable range or not. Range : 0 - 65535.
  uint8_t
    implicit_twt; ///< If enabled (1), the TWT requests STA to calculate the next TWT by adding a fixed value to the current TWT value. Currently, explicit TWT is not allowed.
  uint8_t
    un_announced_twt; ///< If enabled (1), the TWT requests STA to not announce its wake up to AP through PS-POLLs or UAPSD Trigger frames. Values : 0 or 1.
  uint8_t
    triggered_twt; ///< If enabled(1), at least one trigger frame is included in the TWT Service Period(TSP). Values : 0 or 1.
  uint8_t negotiation_type; ///< Negotiation type : 0 - Individual TWT; 1 - Broadcast TWT.
  uint8_t twt_channel;      ///< Currently this configuration is not supported. Range : 0 - 7.
  uint8_t
    twt_protection; ///< If enabled (1), TSP is protected. This is negotiable with AP. Currently this is not supported. Values : 0 or 1.
  uint8_t twt_flow_id; ///< TWT session flow id. 0 - 7 valid. 0xFF to disable all active TWT sessions.
  uint8_t
    restrict_tx_outside_tsp;  ///< 1 - Any Tx outside the TSP is restricted. 0 - TX can happen outside the TSP also.
  uint8_t twt_retry_limit;    ///< TWT retry limit. Range : 0 - 15.
  uint8_t twt_retry_interval; ///< TWT retry interval in seconds between two twt requests. Range : 5 - 255.
  uint8_t req_type;           ///< TWT request type. 0 - Request TWT; 1 - Suggest TWT; 2 - Demand TWT.
  uint8_t twt_enable;         ///< TWT enable. 0 - TWT session teardown; 1 - TWT session setup.
  uint8_t wake_duration_unit; ///< Wake duration unit. 0 - 256 microseconds ; 1 - 1024 microseconds.
} sl_wifi_twt_request_t;

/**
 * @struct sl_wifi_twt_selection_t
 * @brief TWT (Target Wake Time) request structure to auto select a session.
 */
typedef struct {
  uint8_t twt_enable; ///< TWT enable. 0 - TWT session teardown; 1 - TWT session setup.
  uint16_t
    average_tx_throughput; ///< This is the expected average Tx throughput in Kbps. Value ranges from 0 to 10 Mbps, which is half of the default [device_average_throughput](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-wi-fi/sl-wifi-twt-selection-t#device-average-throughput) (20 Mbps by default).
  uint32_t
    tx_latency; ///< The allowed latency, in milliseconds, within which the given Tx operation is expected to be completed. If 0 is configured, maximum allowed Tx latency is same as rx_latency. Otherwise, valid values are in the range of [200 msec - 6 hrs].
  uint32_t
    rx_latency; ///< The maximum latency, in milliseconds, for receiving buffered packets from the AP. The device wakes up at least once for a TWT service period within the configured rx_latency if there are any pending packets destined for the device from the AP. If set to 0, the default latency of 2 seconds is used. Valid range is between 2 seconds to 6 hours. Recommended range is 2 seconds to 60 seconds to avoid connection failures with AP due to longer sleep time.
  uint16_t
    device_average_throughput; ///< Refers to the average Tx throughput that the device is capable of achieving in Kbps. The default value is 20 Mbps. Internal SDK use only: do not use.
  uint8_t
    estimated_extra_wake_duration_percent; ///< The percentage by which wake duration is supposed to be overestimated to compensate for bss congestion. Recommended input range is 0 - 50%. The default value is 0. Internal SDK use only: do not use.
  uint8_t
    twt_tolerable_deviation; ///< The allowed deviation percentage of wake duration TWT response. Recommended input range is 0 - 50%. The default value is 10. Internal SDK use only: do not use.
  uint32_t
    default_wake_interval_ms; ///< Default minimum wake interval. Recommended Range: 512 to 1024 msec. The default value is 1024 msec. Internal SDK use only: do not use.
  uint32_t
    default_minimum_wake_duration_ms; ///< Default minimum wake interval. Recommended Range: 8- 16 msec. The default value is 8 msec. Internal SDK use only: do not use.
  uint8_t
    beacon_wake_up_count_after_sp; ///< The number of beacons after the service period completion for which the module wakes up and listens for any pending RX. The default value is 2. Internal SDK use only: do not use.
} sl_wifi_twt_selection_t;

/**
 * @struct sl_wifi_reschedule_twt_config_t
 * @brief TWT (Target Wake Time) reschedule configuration structure.
 */
typedef struct {
  uint8_t flow_id; ///< TWT session flow ID
  sl_wifi_reschedule_twt_action_t
    twt_action; ///< Specifies the action need to be taken for rescheduling the TWT session. This determines how and when the TWT session would be suspended or adjusted. See @ref sl_wifi_reschedule_twt_action_t for the possible actions.
  uint16_t reserved1;        ///< Reserved
  uint8_t reserved2;         ///< Reserved
  uint64_t suspend_duration; ///< Duration to suspend the respective TWT session, in microseconds.
} sl_wifi_reschedule_twt_config_t;

/**
 * @struct sl_wifi_status_t
 * @brief Wi-Fi device status structure.
 *
 * Indicates the status of various Wi-Fi functionalities on the device.
 * It uses bit fields to indicate the status of different Wi-Fi modes and operations.
 */
typedef struct {
  uint8_t client_active : 1;       ///< Wi-Fi Client active
  uint8_t ap_active : 1;           ///< Wi-Fi Access point active
  uint8_t monitor_mode_active : 1; ///< Wi-Fi promiscuous mode active
  uint8_t wfd_go_active : 1;       ///< Reserved Status bit
  uint8_t wfd_client_active : 1;   ///< Reserved Status bit
  uint8_t scan_active : 1;         ///< Scan in Progress
  uint8_t _reserved : 1;           ///< Reserved Status bit
  uint8_t _reserved2 : 1;          ///< Reserved Status bit
} sl_wifi_status_t;

/**
 * @struct sl_wifi_statistics_t
 * @brief Wi-Fi interface statistics structure.
 */
typedef struct {
  uint32_t beacon_lost_count; ///< Number of missed beacons
  uint32_t beacon_rx_count;   ///< Number of received beacons
  uint32_t mcast_rx_count;    ///< Multicast packets received
  uint32_t mcast_tx_count;    ///< Multicast packets transmitted
  uint32_t ucast_rx_count;    ///< Unicast packets received
  uint32_t ucast_tx_count;    ///< Unicast packets transmitted
  uint32_t
    overrun_count; ///< Number of packets dropped either at ingress or egress, due to lack of buffer memory to retain all packets.
} sl_wifi_statistics_t;

/**
 * @struct sl_wifi_operational_statistics_t
 * @brief Wi-Fi Operational Statistics structure.
 */
typedef struct {
  uint8_t operating_mode; ///< Operating mode of the Wi-Fi interface
  uint8_t
    dtim_period; ///< DTIM (Delivery Traffic Indication Message) period. Indicates the number of beacon intervals between DTIM frames
  uint8_t ideal_beacon_info[2]; ///< Idle beacon information
  uint8_t busy_beacon_info[2];  ///< Busy beacon information
  uint8_t beacon_interval
    [2]; ///< Beacon Interval. Indicates the time interval between successive beacons, in Time Units (TUs).
} sl_wifi_operational_statistics_t;

/**
 * @struct sl_wifi_p2p_configuration_t
 * @brief Wi-Fi Direct (P2P) configuration structure.
 */
typedef struct {
  uint16_t group_owner_intent; ///< Group owner intent
  const char *device_name;     ///< Device name
  sl_wifi_channel_t channel;   ///< Wi-Fi channel. This is of type @ref sl_wifi_channel_t
  char ssid_suffix[6];         ///< SSID suffix
} sl_wifi_p2p_configuration_t;

/**
 * @union sl_wifi_event_data_t
 * @brief Wi-Fi event data.
 *
 * Indicates the data associated with a Wi-Fi event.
 */
typedef union {
  sl_wifi_scan_result_t scan_results; ///< Scan Result structure. This is of type @ref sl_wifi_scan_result_t
  uint32_t join_status;               ///< Join status
} sl_wifi_event_data_t;

/**
 * @struct sl_wifi_wps_pin_t
 * @brief Wi-Fi WPS PIN object that is an 8 digit number.
 */
typedef struct {
  char digits[8]; ///< Array to store digits of WPS Pin
} sl_wifi_wps_pin_t;

/**
 * @struct sl_wifi_wps_config_t
 * @brief WPS (Wi-Fi Protected Setup) configuration structure.
 *
 * This structure defines the device role, WPS method (PBC or PIN), optional PIN value, and whether to
 * automatically connect to the network.
 *
 */
typedef struct {
  sl_wifi_wps_role_t
    role; ///< Role of the device. Refer [sl_wifi_wps_role_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-constants#sl-wifi-wps-role-t)
  sl_wifi_wps_mode_t
    mode; ///< WPS mode. Refer [sl_wifi_wps_mode_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-constants#sl-wifi-wps-mode-t)
  char optional_pin[9]; ///< PIN-based WPS configuration. Null-terminated string
  bool
    auto_connect; ///< Set to true to enable auto connect after WPS, false to only receive credentials without connecting
} sl_wifi_wps_config_t;

/**
 * @struct sl_wifi_wps_response_t
 * @brief Wi-Fi WPS response structure containing network credentials received from WPS exchange.
 *
 * This structure contains all the network information needed to connect to the Wi-Fi network,
 * including SSID, security type, and network key.
 */
#pragma pack(1)
typedef struct {
  uint32_t status;       ///< Status of the WPS operation
  uint8_t ssid[32];      ///< SSID of the connected network
  uint8_t ssid_len;      ///< Length of the SSID
  uint8_t security_type; ///< Security type
  uint8_t key[32];       ///< Network key
  uint8_t mac_addr[6];   ///< MAC address of the access point
  uint32_t reserved;     ///< Reserved for future use
} sl_wifi_wps_response_t;
#pragma pack()

/**
 * @struct sl_wifi_listen_interval_t
 * @brief Wi-Fi Listen interval structure.
 *
 * Specifies the Wi-Fi Listen interval in milliseconds.
 * The listen interval is the time interval between two consecutive Target Beacon Transmission (TBTT) events.
 * Moving forward, this structure will be deprecated. Instead, use the [sl_wifi_listen_interval_v2_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-listen-interval-v2-t) structure. This is retained for backward compatibility.
 */
typedef struct {
  uint32_t listen_interval; ///< Wi-Fi Listen interval in milliseconds
} sl_wifi_listen_interval_t;

/**
 * @struct sl_wifi_listen_interval_v2_t
 * @brief Wi-Fi Listen interval structure.
 *
 * Specifies the Wi-Fi Listen interval in milliseconds and listen interval multiplier.
 * The listen interval is the time interval between two consecutive Target Beacon Transmission (TBTT) events.
 * The listen interval is multiplied with listen interval multiplier and advertised in the assoc request.
 */
typedef struct {
  uint32_t listen_interval; ///< Wi-Fi Listen interval in milliseconds
  uint32_t
    listen_interval_multiplier; ///< Multiplier for the listen interval, sent by the device in the association request to the AP. Default: 1. Max recommended: 10. Higher values may lead to interoperability issues.
} sl_wifi_listen_interval_v2_t;

/**
 * @struct sl_wifi_client_info_t
 * @brief Wi-Fi client information structure.
 *
 * Indicates the MAC and IP address information related to a Wi-Fi client connected to the network. 
 */
typedef struct {
  sl_mac_address_t mac_adddress; ///< MAC address of the client
  sl_ip_address_t ip_address;    ///< IP address of client
} sl_wifi_client_info_t;

/**
 * @struct sl_wifi_client_info_response_t
 * @brief Wi-Fi client information response structure.
 */
typedef struct {
  uint8_t client_count; ///< Indicates the total count of Wi-Fi clients currently connected to the network
  sl_wifi_client_info_t client_info[SL_WIFI_MAX_CLIENT_COUNT]; ///< Array of client information
} sl_wifi_client_info_response_t;

/**
 * @struct sl_wifi_max_tx_power_t
 * @brief Wi-Fi maximum transmit power structure.
 *
 * Indicates the maximum transmit power settings for the Wi-Fi interface.
 *
 * @note
 * The effective transmit power is subject to regional and device limitations. If the specified transmit power exceeds the
 * maximum supported value for that region, or if the specified transmit power exceeds the maximum supported value of the device,
 * the transmission would occur at the maximum supported transmit power.
 * 
 * @note
 * The software supports configuration up to 31 dBm. Refer to the data sheet for the maximum supported power.
 * The device automatically selects the highest transmit power allowed for each data rate.
 * If the configured power level exceeds the device’s supported power for that rate, the device limits the power to the maximum supported level.
 *
 */
typedef struct {
  uint8_t scan_tx_power; ///< Transmit power during scan. Valid input range: 1 to 31 dBm
  uint8_t join_tx_power; ///< Transmit power during join. Valid input range: 1 to 31 dBm
} sl_wifi_max_tx_power_t;

/**
 * @struct sl_wifi_rts_threshold_t
 * @brief Wi-Fi Request to Send (RTS) threshold structure.
 * 
 * This structure is used to configure the RTS threshold for Wi-Fi operations.
 * 
 * Members:
 * - rts_threshold: RTS threshold in bytes. The device performs an RTS/CTS handshake when the packet size exceeds the threshold.
 * - reserved: Reserved fields for future use, should be set to zero.
 * 
 */
typedef struct {
  uint16_t rts_threshold; ///< Request to Send (RTS) threshold in bytes
  uint8_t reserved[2];    ///< Reserved fields
} sl_wifi_rts_threshold_t;

/**
 * @enum sl_wifi_mfp_mode_t
 * @brief Wi-Fi Management Frame Protection (MFP) mode enumeration.
 *
 * This enumeration defines the modes for Management Frame Protection (MFP) in Wi-Fi.
 * MFP is used to protect management frames from spoofing and other attacks.
 * The modes are:
 * - SL_WIFI_MFP_DISABLED: MFP is disabled (0b00)
 * - SL_WIFI_MFP_CAPABLE: MFP is capable/optional (0b01)
 * - SL_WIFI_MFP_REQUIRED: MFP is required/mandatory (0b10)
 * 
 */
typedef enum {
  SL_WIFI_MFP_DISABLED = 0, ///< MFP disabled (0b00)
  SL_WIFI_MFP_CAPABLE  = 1, ///< MFP capable/optional (0b01)
  SL_WIFI_MFP_REQUIRED = 2  ///< MFP required/mandatory (0b10)
} sl_wifi_mfp_mode_t;

/**
 * @struct sl_wifi_mfp_config_t
 * @brief Wi-Fi Management Frame Protection (MFP) configuration structure.
 *
 * This structure is used to configure the MFP mode for Wi-Fi operations.
 * It includes the MFP mode and a flag indicating whether the MFP mode is explicitly configured or automatically detected.
 * Members:
 * - mfp_mode: MFP mode setting of type @ref sl_wifi_mfp_mode_t
 * - is_configured: True if MFP mode is explicitly configured, false for automatic detection.
 * - reserved: Reserved for future use and alignment.
 * 
 */
typedef struct {
  sl_wifi_mfp_mode_t mfp_mode; ///< MFP mode setting of type @ref sl_wifi_mfp_mode_t
  bool is_configured;          ///< True if MFP mode is explicitly configured, false for automatic detection
  uint8_t reserved[2];         ///< Reserved for future use and alignment
} sl_wifi_mfp_config_t;

/**
 * @enum sl_wifi_pll_mode_t
 * @brief Wi-Fi Phase-Locked Loop (PLL) mode selection.
 *
 * Specifies the PLL operating mode for Wi-Fi hardware, determining the clock frequency bandwidth.
 *
 * - SL_WIFI_PLL_MODE_20MHZ: PLL mode for 20 MHz channel bandwidth (default).
 * - SL_WIFI_PLL_MODE_40MHZ: PLL mode for 40 MHz channel bandwidth.
 */
typedef enum {
  SL_WIFI_PLL_MODE_20MHZ = 0, ///< PLL mode for 20 MHz channel bandwidth (default)
  SL_WIFI_PLL_MODE_40MHZ = 1  ///< PLL mode for 40 MHz channel bandwidth
} sl_wifi_pll_mode_t;

/**
 * @enum sl_wifi_power_chain_t
 * @brief Wi-Fi power chain selection.
 *
 * This enum defines the available power chains for Wi-Fi operation.
 * - SL_WIFI_HP_CHAIN: High-power chain (default).
 * - SL_WIFI_LP_CHAIN: Low-power chain.
 */
typedef enum {
  SL_WIFI_HP_CHAIN = 0, ///< High-power chain (default)
  SL_WIFI_LP_CHAIN = 12 ///< Low-power chain
} sl_wifi_power_chain_t;

/**
 * @struct sl_wifi_async_stats_response_t
 * @brief Structure representing asynchronous Wi-Fi statistics response.
 *
 * This structure contains various statistics related to Wi-Fi operations,
 * such as transmitted packets, retries, CRC checks, and more.
 */
typedef struct {
  uint16_t tx_pkts;                           ///< Number of transmitted packets
  uint8_t reserved_1[2];                      ///< Reserved fields
  uint16_t tx_retries;                        ///< Number of transmission retries
  uint16_t crc_pass;                          ///< Number of packets that passed CRC check
  uint16_t crc_fail;                          ///< Number of packets that failed CRC check
  uint16_t cca_stk;                           ///< Number of times CCA got stuck
  uint16_t cca_not_stk;                       ///< Number of times CCA didn't get stuck
  uint16_t pkt_abort;                         ///< Number of packet aborts
  uint16_t fls_rx_start;                      ///< Number of false RX starts
  uint16_t cca_idle;                          ///< CCA idle time
  uint8_t reserved_2[26];                     ///< Reserved fields
  uint16_t rx_retries;                        ///< Number of reception retries
  uint8_t reserved_3[2];                      ///< Reserved fields
  uint16_t cal_rssi;                          ///< Calibrated RSSI
  uint8_t reserved_4[4];                      ///< Reserved fields
  uint16_t xretries;                          ///< Number of transmitted packets dropped after maximum retries
  uint16_t max_cons_pkts_dropped;             ///< Maximum consecutive packets dropped
  uint8_t reserved_5[2];                      ///< Reserved fields
  uint16_t bss_broadcast_pkts;                ///< BSSID matched broadcast packets count
  uint16_t bss_multicast_pkts;                ///< BSSID matched multicast packets count
  uint16_t bss_filter_matched_multicast_pkts; ///< BSSID & multicast filter matched packets count
} sl_wifi_async_stats_response_t;

/**
 * @struct sl_wifi_cw_tone_config_t
 * @brief Structure representing the configuration for CW tone settings.
 *
 * This structure contains parameters to configure single-tone and two-tone modes, 
 * tone frequencies, amplitude scaling, and DC mode settings.
 * @note 
 *    - This structure is only supported on SiWx353 devices, not on SiWx91x devices.
 */
typedef struct {
  uint8_t freq_val_en; ///< Enable or disable frequency value. 1 to enable, 0 to disable.
  uint8_t two_tone_en; ///< Enable or disable two-tone mode. 1 to enable, 0 to disable.
  uint16_t
    tone1_freq_val; ///< Frequency value for the first tone in KHz. Indicates the frequency of the 1st tone to be generated in TX test modes. Valid range of values is from 0 to 5000. This is ignored when @ref freq_val_en is 0.
  sl_wifi_cw_tone_frequency_t
    tone1_freq_sel; ///< Frequency selection for the first tone. Indicates the frequency of the 1st Tone to be generated in TX test modes. This is ignored when @ref freq_val_en is 1. Possible values are defined in @ref sl_wifi_cw_tone_frequency_t.
  uint16_t
    tone2_freq_val; ///< Frequency value for the second tone in KHz. Indicates the frequency of the 2nd Tone to be generated in TX test modes. Valid range of values is from 0 to 5000. This is ignored when @ref freq_val_en is 0 or @ref two_tone_en is 0.
  sl_wifi_cw_tone_frequency_t
    tone2_freq_sel; ///< Frequency selection for the second tone. Indicates the frequency of the 2nd Tone to be generated in TX test modes. This is ignored when @ref freq_val_en is 1 or @ref two_tone_en is 0. Possible values are defined in @ref sl_wifi_cw_tone_frequency_t.
  sl_wifi_cw_tone_amplitude_t
    tone_scale_val; ///< This is used to control the amplitude of the WAVE. Possible values are defined in @ref sl_wifi_cw_tone_amplitude_t.
  uint8_t neg_tone_en; ///< Enable or disable negative tone. 1 to enable, 0 to disable.
  uint8_t dc_mode_en;  ///< Enable or disable DC mode. 1 to enable, 0 to disable.
  sl_wifi_dc_val_iq_t
    dc_val_iq; ///< DC Value to be transmitted. Possible values are defined in @ref sl_wifi_dc_val_iq_t. This is ignored when dc_mode_en is 0.
} sl_wifi_cw_tone_config_t;

/**
 * @struct sl_wifi_response_get_ctune_data_t
 * @brief Structure representing the response for getting CTUNE data.
 * 
 * This structure contains the flags indicating the presence of CTUNE data and the actual CTUNE data values.
 * @note
 * This structure is only supported on SiWx353 devices, not on SiWx91x devices.
 */
typedef struct {
  uint32_t flags;         ///< Flags indicating the presence of CTUNE data
  uint32_t ctune_data[2]; ///< CTUNE data values
} sl_wifi_response_get_ctune_data_t;

/**
 * @struct sl_wifi_rx_stats_request_t
 * @brief Structure representing the request for RX statistics in Wi-Fi.
 *
 * This structure contains parameters to start or stop the collection of RX statistics
 * and the channel number on which the statistics are to be collected.
 */
typedef struct {
  /// 0 - start, 1 - stop
  uint8_t start[2];
  /// channel number
  uint8_t channel[2];
} sl_wifi_rx_stats_request_t;

/**
 * @struct sl_wifi_transmitter_test_info_t
 * @brief Structure representing the configuration for a Wi-Fi TX test.
 *
 * This structure contains various parameters for configuring a Wi-Fi TX test,
 * such as enabling the test, setting the power, rate, length, mode, and other
 * related settings.
 */
typedef struct {
  uint16_t enable; ///< Enable/disable TX test mode
  uint16_t power;  ///< TX power in dBm.  Range : 2 - 18 dBm.
                   ///<
  ///< @note 1. User can configure the maximum power level allowed for the given frequncey in the configured region by providing 127 as power level.
  ///< @note 2. User should configure a minimum delay (approx. 10 msec) before and after \ref sl_wifi_transmit_test_start API to observe a stable output at requested dBm level.
  uint32_t rate;   ///< Transmit data rate
                   ///<     ### Data Rates ###
                   ///<          Data rate(Mbps) |   Value of rate
                   ///<          :--------------:|:-------------------:
                   ///<          1                   |   0
                   ///<          2                   |   2
                   ///<          5.5               | 4
                   ///<          11                | 6
                   ///<          6                   |   139
                   ///<          9                   |   143
                   ///<          12                | 138
                   ///<          18                | 142
                   ///<          24                | 137
                   ///<          36                | 141
                   ///<          48                | 136
                   ///<          54                | 140
                   ///<          MCS0                |   256
                   ///<          MCS1                |   257
                   ///<          MCS2                |   258
                   ///<          MCS3                |   259
                   ///<          MCS4                |   260
                   ///<          MCS5                |   261
                   ///<          MCS6                |   262
                   ///<          MCS7                |   263
  uint16_t length; ///< TX packet length. Range: [24 - 1500] bytes in Burst mode and [24 - 260] bytes in Continuous mode
  uint16_t mode;   ///< TX test mode mode.
                   ///<
                   ///< 0 - Burst Mode.
                   ///<
                   ///< 1 - Continuous Mode.
                   ///<
                   ///< 2 - Continuous wave Mode (non modulation) in DC mode.
                   ///<
                   ///< 3 - Continuous wave Mode (non modulation) in single tone mode (center frequency -2.5 MHz).
                   ///<
                   ///< 4 - Continuous wave Mode (non modulation) in single tone mode (center frequency +5 MHz).
                   ///<
  ///< `Burst mode`: DUT transmits a burst of packets with the given power, rate, length in the channel configured.
  ///<               The burst size will be determined by the number of packets and if its zero, then DUT keeps transmitting till a @ref sl_wifi_transmit_test_stop API is called.
  ///<
  ///< `Continuous Mode`: The DUT transmits a unmodulated waveform continuously
  ///<
  ///< `Continuous Wave Mode (Non-Modulation) in DC Mode`: The DUT transmits a spectrum only at the center frequency of the channel.
  ///<                                                   A basic signal with no modulation is that of a sine wave and is usually referred to as a continuous wave (CW) signal.
  ///<                                                   A basic signal source produces sine waves. Ideally, the sine wave is perfect. In the frequency domain, it is viewed as a single line at some specified frequency.
  ///<
  ///<  `Continuous Wave Mode (Non-Modulation) in single tone Mode (Center frequency -2.5 MHz)`: The DUT transmits a spectrum that is generated at -2.5MHz from the center frequency of the channel selected.
  ///<                                                                                        Some amount of carrier leakage will be seen at Center Frequency. For example, for 2412 MHz, the output would be seen at 2409.5 MHz.
  ///<
  ///<  `Continuous Wave Mode (Non-Modulation) in single tone Mode (Center frequency +5 MHz)`: The DUT transmits a spectrum that is generated at 5MHz from the center frequency of the channel selected.
  ///<                                                                                      Some amount of carrier leakage will be seen at Center Frequency. For example, for 2412MHz, the output would be seen at 2417 MHz.
  uint16_t channel; ///< Channel number in 2.4 GHZ / 5 GHZ.
  ///<         ###The following table maps the channel number to the actual radio frequency in the 2.4 GHz spectrum. ###
  ///<         Channel numbers (2.4GHz)|   Center frequencies for 20 MHz channel width
  ///<         :----------------------:|:-----------------------------------------------:
  ///<         1           |   2412
  ///<         2           |   2417
  ///<         3           |   2422
  ///<         4           |   2427
  ///<         5           |   2432
  ///<         6           |   2437
  ///<         7           |   2442
  ///<         8           |   2447
  ///<         9           |   2452
  ///<         10          |   2457
  ///<         11          |   2462
  ///<         12          |   2467
  ///<         13          |   2472
  ///< @note   To start transmit test in 12,13 channels, configure set region parameters in  sl_si91x_set_device_region
  ///<    ###  The following table maps the channel number to the actual radio frequency in the 5 GHz spectrum for 20MHz channel bandwidth. The channel numbers in 5 GHz range is from 36 to 165. ###
  ///<         Channel Numbers(5GHz) | Center frequencies for 20MHz channel width
  ///<         :--------------------:|:------------------------------------------:
  ///<     36            |5180
  ///<     40            |5200
  ///<     44            |5220
  ///<     48            |5240
  ///<     52            |5260
  ///<     56          |5280
  ///<     60            |5300
  ///<     64            |5320
  ///<     149           |5745
  ///<     153           |5765
  ///<     157           |5785
  ///<     161           |5805
  ///<     165           |5825
  uint16_t rate_flags; ///< Rate flags
  ///< BIT(6) - Immediate Transfer, set this bit to transfer packets immediately ignoring energy/traffic in channel.
  uint16_t channel_bw;  ///< Channel Bandwidth
  uint16_t aggr_enable; ///< tx test mode aggr_enable
  uint16_t reserved;    ///< Reserved
  uint16_t no_of_pkts;  ///< Number of packets
  uint32_t delay;       ///< Delay
  uint8_t enable_11ax;  ///< 11AX_ENABLE 0-disable, 1-enable
  uint8_t coding_type;  ///< Coding_type 0-BCC 1-LDPC
  uint8_t nominal_pe;   ///< Indicates Nominal T-PE value. 0-0Us 1-8Us 2-16Us
  uint8_t
    ul_dl; ///< Indicates whether the PPDU is UL/DL. Set it to 1 if PPDU is to be sent by station to AP; 0 if PPDU is to be sent by AP to station.
  uint8_t he_ppdu_type; ///< he_ppdu_type 0-HE SU PPDU, 1-HE ER SU PPDU, 2-HE TB PPDU, 3-HE MU PPDU
  uint8_t
    beam_change; ///< Indicates the spatial mapping of pre-HE and HE fields. Enter 0 for pre-HE and HE fields are spatially mapped in the same way and 1 for pre-HE and HE fields are spatially mapped differently.
  uint8_t bw;    ///< Indicates the BW for the PPDU: 0 for 242-tone RU, 1 for upper 106-tone RU.
  uint8_t
    stbc; ///< Indicates whether STBC is used for PPDU transmission. Set to 0 for no STBC and 1 for STBC (only if DCM field is set to 0).
  uint8_t
    tx_bf; ///< Indicates whether beamforming matrix is applied to the transmission. 0 - no beamforming matrix, 1 - beamforming matrix.
  uint8_t gi_ltf;        ///< Indicates the GI and LTF size. GI_LTF shall be in the range 0-3
  uint8_t dcm;           ///< Indicates whether DCM is applied to Data Symbols. 0 - No DCM, 1 - DCM.
  uint8_t nsts_midamble; ///< Indicates the NSTS and Midamble Periodicity. NSTS_MIDAMBLE shall be in the range 0-7
  uint8_t
    spatial_reuse; ///< spatial_reuse shall be in the range 0-15. 4 indicates that spatial reuse is allowed during the transmission of PPDU.
  uint8_t bss_color;              ///< Color value of BSS. Must be in the range 0 to 63
  uint16_t he_siga2_reserved;     ///< HE_SIGA2_RESERVED shall be in the range 0-511
  uint8_t ru_allocation;          ///< Indicates the RU Allocation Subfield for 20MHz BW. Must be in the range 0-255.
  uint8_t n_heltf_tot;            ///< Indicates the number of HE-LTF to be transmitted. Can be in the range 0-7.
  uint8_t sigb_dcm;               ///< Indicates whether DCM is applied to SIG-B Symbols. 0-disable, 1-enable
  uint8_t sigb_mcs;               ///< Indicates the MCS for SIG-B Symbols. Allowed range is 0-5.
  uint16_t user_sta_id;           ///< Indicates the Station ID of the intended user. Allowed range is 0-2047.
  uint8_t user_idx;               ///< USER_IDX shall be in the range 0-8
  uint8_t sigb_compression_field; ///< SIGB_COMPRESSION_FIELD shall be 0/1
} sl_wifi_transmitter_test_info_t;

/**
 * @struct sl_wifi_freq_offset_t
 * @brief Structure representing the frequency offset for Wi-Fi.
 *
 * This structure contains the frequency offset value in kHz.
 */
typedef struct {
  int32_t frequency_offset_in_khz; ///< Frequency offset in KHZ
} sl_wifi_freq_offset_t;

/**
 * @struct sl_wifi_dpd_calib_data_t
 * @brief Structure representing the DPD (Digital Pre-Distortion) calibration data for Wi-Fi.
 *
 * This structure contains the DPD power index provided by the user.
 */
typedef struct {
  int8_t dpd_power_index; ///< Dpd power index given by the user
} sl_wifi_dpd_calib_data_t;

/**
 * @struct sl_wifi_multicast_filter_info_t
 * @brief Wi-Fi Multicast filter information structure.
 */
typedef struct {
  sl_wifi_multicast_filter_command_t
    command_type; ///< Command type for multicast filter operation. Specifies the action to be taken (for example, add or remove a multicast filter). See @ref sl_wifi_multicast_filter_command_t for possible values.
  sl_mac_address_t mac_address; ///< MAC address to which the filter has to be applied.
} sl_wifi_multicast_filter_info_t;

/**
 * @struct sl_wifi_tsf64_t
 * @brief Wi-Fi station TSF (Timing Synchronization Function) structure.
 *
 * Contains the 64-bit TSF timer value for a Wi-Fi station.
 */
typedef struct {
  uint32_t tsf_l; ///< Lower 32 bits of the TSF timer. Used to store the Least Significant Bits (LSB) of the TSF
  uint32_t tsf_m; ///< Upper 32 bits of the TSF timer. Used to store the Most Significant Bits (MSB) of the TSF.
} sl_wifi_tsf64_t;

/**
 * @struct sl_wifi_transceiver_tx_data_control_t
 * @brief Control block structure used to hold metadata for the payload passed in @ref sl_wifi_send_transceiver_data.
 *
 * Contains control flags and other metadata for the payload.
 * The control flags specify various options for the packet, such as whether it is a 4-address packet, a QoS packet, or if a fixed data rate should be used.
 * @note
 * Bits 6 and 7 of ctrl_flags, bit 0 of ctrl_flags1, and the channel and tx_power fields are currently not supported.
 */
typedef struct {
  /// Control flags bit description:
  /// | Bit position | ctrl_flags bit description                                                                                                                     |
  /// |--------------|------------------------------------------------------------------------------------------------------------------------------------------------|
  /// | 0            | Should be set for 4-address packet or unset for 3-address packet. addr4 is ignored if set to 0.                                                 |
  /// | 1            | Should be set for QoS packet. QoS control field shall not be present in the MAC header for non-QoS packet. priority is ignored if set to 0.     |
  /// | 2            | Should be set to use the fixed data rate provided in the rate field. If set to 0, rate field is ignored and auto rate shall be used.            |
  /// | 3            | Should be set to enable To DS bit in Frame Control. Valid only for 3-addr packet (bit 0 is unset).                                               |
  /// | 4            | Should be set to enable From DS bit in Frame Control. Valid only for 3-addr packet (bit 0 is unset).                                             |
  /// | 5            | Should be set if host requires TX data status report. Token is used for synchronization between data packets sent and reports received.         |
  /// | 6            | Should be set if the extended descriptor contains channel, tx_power and is_last_packet information. This is currently not supported.            |
  /// | 7            | Should be set if immediate transfer is enabled. This is currently not supported.                                                                |
  /// @note If addr1 is multicast/broadcast, ctrl_flags bit 1 is ignored, and the frame is sent as a non-QoS frame, that is, QoS control field should not be present in the MAC header.
  uint8_t ctrl_flags;
  /// ctrl_flags1 bit description:
  /// | Bit position | flags bit description
  /// |--------------|------------------------------------------------------------------------------------------------------------------------------------------------|
  /// | 0            | Should be set if it is the last packet of that channel. This is currently not supported.                                                       |
  /// | 1:7          | Reserved.                                                                                                                                      |
  uint8_t ctrl_flags1;
  uint8_t reserved2; ///< Reserved
  uint8_t
    priority; ///< Data Packets are queued to respective queue based on priority. Best Effort - 0, Background - 1, Video - 2, Voice - 3
  sl_wifi_data_rate_t
    rate; ///< Rates shall be provided as per @ref sl_wifi_data_rate_t. Only 11b/g rates shall be supported
  uint32_t
    token; ///< Used for synchronization between data packets sent and reports received. Application provides token/identifier as per PPDU. MAC layer sends the same token/identifier in status report along with the status of the transmitted packet
  uint8_t addr1[6]; ///< Receiver MAC address
  uint8_t addr2[6]; ///< Transmitter MAC address
  uint8_t addr3[6]; ///< Destination MAC address
  uint8_t addr4[6]; ///< Source MAC address. Initialization of addr4 is optional
  uint8_t channel;  ///< Channel is currently not supported.
  uint8_t tx_power; ///< Transmission power is currently not supported.
} sl_wifi_transceiver_tx_data_control_t;

/**
 * @struct sl_wifi_transceiver_cw_config_t
 * @brief Wi-Fi transceiver contention window configuration structure.
 *
 * Specifies the configuration parameters for the contention window of the Wi-Fi transceiver.
 */
typedef struct {
  uint8_t
    cwmin; ///< Minimum contention window size. Value is calculated from 2^N - 1 where exponent is provided as the input. Valid values for exponent N are 0 - 15
  uint8_t
    cwmax; ///< Maximum contention window size. Value is calculated from 2^N - 1 where exponent is provided as the input. Valid values for exponent N are 0 - 15
  uint8_t aifsn;    ///< Arbitration Inter-Frame Space Number (AIFSN). Valid range is 0 to 15
  uint8_t reserved; ///< Reserved
} sl_wifi_transceiver_cw_config_t;

/**
 * @struct sl_wifi_transceiver_parameters_t
 * @brief Wi-Fi transceiver parameters structure.
 *
 * Specifies the configuration parameters for a Wi-Fi transceiver.
 */
typedef struct {
  uint8_t
    set; ///< Set to 1 to configure the transceiver config params in MAC layer. Sets to 0 to query the transceiver config params from MAC layer
  uint8_t
    retransmit_count; ///< Retransmit count. Common across all peers and access categories and valid only for unicast data frames. Valid range is 1 to 15
  uint16_t flags;     ///< Reserved
  sl_wifi_transceiver_cw_config_t
    cw_params[4]; ///< CW params for respective queues. AC index: Best Effort - 0, Background - 1, Video - 2, Voice - 3
} sl_wifi_transceiver_parameters_t;

/**
 * @struct sl_wifi_transceiver_set_channel_t
 * @brief Wi-Fi transceiver channel information structure.
 *
 * Indicates the channel information for the Wi-Fi transceiver.
 */
typedef struct {
  sl_wifi_channel_t chan_info; ///< Channel information
  uint8_t tx_power;            ///< Maximum transmission power
} sl_wifi_transceiver_set_channel_t;

/**
 * @struct sl_wifi_transceiver_peer_update_t
 * @brief Wi-Fi transceiver peer update structure.
 *
 * Defines the configuration parameters for adding or deleting a peer in the Wi-Fi transceiver.
 * The peer can be added or deleted based on the MAC address.
 */
typedef struct {
  /// | Bit position | Flags bit description                                                                                              |
  /// |--------------|--------------------------------------------------------------------------------------------------------------------|
  /// | 0            | Shall be set to add the peer, else reset to 0 to delete the peer.                                                  |
  /// | 1            | Shall be set for auto-rate enable. To enable auto-rate, application needs to provide peer_supported_rate_bitmap    |
  uint8_t flags;
  /// MAC address of peer to be added or deleted.
  uint8_t peer_mac_address[6];
  /// Rate bitmap of peer station
  /// | peer_supported_rate_bitmap | Data rate  |
  /// | :--------------------------| :----------|
  /// | BIT(0)                     | 1 Mbps     |
  /// | BIT(1)                     | 2 Mbps     |
  /// | BIT(2)                     | 5.5 Mbps   |
  /// | BIT(3)                     | 11 Mbps    |
  /// | BIT(4)                     | 6 Mbps     |
  /// | BIT(5)                     | 9 Mbps     |
  /// | BIT(6)                     | 12 Mbps    |
  /// | BIT(7)                     | 18 Mbps    |
  /// | BIT(8)                     | 24 Mbps    |
  /// | BIT(9)                     | 36 Mbps    |
  /// | BIT(10)                    | 48 Mbps    |
  /// | BIT(11)                    | 54 Mbps    |
  /// | BIT(12:31)                 | Reserved   |
  uint32_t peer_supported_rate_bitmap;
} sl_wifi_transceiver_peer_update_t;

/**
 * @struct sl_wifi_transceiver_mcast_filter_t
 * @brief Wi-Fi transceiver multicast filter structure.
 *
 * Defines the configuration parameters for enabling or disabling multicast filtering in the Wi-Fi transceiver.
 */
typedef struct {
  uint8_t
    flags; ///< Bit 0 is set to 1 to enable filtering for the specified MAC addresses, else set to 0 to disable filtering
  uint8_t
    num_of_mcast_addr; ///< Number of multicast addresses. Valid values are 1, and 2. This field is ignored when filtering is disabled
  uint8_t mac[2][6]; ///< List of multicast addresses. This field is ignored when filtering is disabled
} sl_wifi_transceiver_mcast_filter_t;

/**
 * @struct sl_wifi_transceiver_configuration_t
 * @brief Wi-Fi transceiver interface configuration structure.
 */
typedef struct {
  sl_wifi_transceiver_set_channel_t
    channel; ///< Channel information. See @ref sl_wifi_transceiver_set_channel_t for details on channel settings.
  sl_mac_address_t dut_mac; ///< DUT MAC address
  sl_wifi_transceiver_parameters_t
    parameters; ///< Transceiver parameters. See @ref sl_wifi_transceiver_parameters_t for details on transceiver settings
} sl_wifi_transceiver_configuration_t;

/**
 * @struct sl_wifi_transceiver_tx_data_confirmation_t
 * @brief Wi-Fi transceiver TX data confirmation structure.
 *
 * Indicates the status report for a transmitted data packet identified by a token.
 */
typedef struct {
  /// Status report for the data packet identified by token.
  /// | Status                       | Description                                                                                                                         |
  /// | :----------------------------| :-----------------------------------------------------------------------------------------------------------------------------------|
  /// | SL_STATUS_OK (0x0)           | Received Ack                                                                                                                        |
  /// | SL_STATUS_ACK_ERR (0x1)      | Ack error                                                                                                                           |
  /// | SL_STATUS_CS_BUSY (0x2)      | Carrier sense busy                                                                                                                  |
  /// | SL_STATUS_UNKNOWN_PEER (0x3) | If @ref sl_wifi_send_transceiver_data was called for a peer that was not added or was deleted before the data packet was sent out.  |
  uint32_t status;
  /// Rate at which data packet has been sent. Rate is invalid if error is SL_STATUS_CS_BUSY or SL_STATUS_UNKNOWN_PEER.
  uint32_t rate;
  /// Priority used for the data packet from control->priority in the corresponding call to @ref sl_wifi_send_transceiver_data.
  uint8_t priority;
  /// Data packet identifier from control->token value passed in the corresponding call to @ref sl_wifi_send_transceiver_data.
  uint32_t token;
} sl_wifi_transceiver_tx_data_confirmation_t;

/**
 * @struct sl_wifi_transceiver_rx_data_t
 * @brief Structure for handling received Wi-Fi transceiver data.
 *
 * Contains information about the received Wi-Fi transceiver data, which includes status, RSSI, data rate, length, and the actual data buffer.
 */
typedef struct {
  /// Status code for the received RX packet.
  /// | Status                       | Description                                                                                                                          |
  /// | :----------------------------| :------------------------------------------------------------------------------------------------------------------------------------|
  /// | SL_STATUS_OK (0x0)           | Success                                                                                                                              |
  /// | SL_STATUS_UNKNOWN_PEER (0x3) | If SL_WIFI_FEAT_TRANSCEIVER_MAC_PEER_DS_SUPPORT feature is enabled and data packet is received from a peer not present in MAC layer |
  uint32_t status;
  /// RSSI of the received 802.11 frame. This field is valid only if status is set to success.
  int8_t rssi;
  /// Rate of the received 802.11 frame as per @ref sl_wifi_data_rate_t. This field is valid only if status is set to success.
  uint32_t rate;
  /// Length of the buffer.
  uint32_t length;
  /// IEEE 802.11 frame received from firmware. `buffer` points to the beginning of the MAC header. Contents are not valid once the function returns. If the contents need to be accessed after return, it needs to be copied to the application buffer. If the status is not successful, buffer is set to NULL and shall not be accessed.
  ///
  /// *Format of IEEE 802.11 frame received from firmware in buffer*
  /// | Field name | Frame Control  | Duration | Addr1 | Addr2 | Adddr3 | Seq Ctrl | Addr4                  | QoS ctrl              | Payload (LLC + Data)  |
  /// |:-----------|:---------------|:---------|:------|:------|:-------|:---------|:-----------------------|:----------------------|:----------------------|
  /// | Size(bytes)| 2              | 2        | 6     | 6     | 6      | 2        | 6 (Optionally present) | 2 (Optionally present)| Variable              |
  /// @note All unicast data frames received where Address 1 (RA) matches the device MAC address are passed to the host application.
  /// @note All broadcast data frames are sent to the host.
  /// @note All multicast data frames are sent to the host unless filtering is enabled using @ref sl_wifi_set_transceiver_multicast_filter.
  /// @note On chip duplicate detection is not supported and is expected to be handled by the application.
  /// @note On chip MAC level decryption is not supported.
  uint8_t *buffer;
} sl_wifi_transceiver_rx_data_t;

/// Wi-Fi module state statistics
#pragma pack(1)
typedef struct {
  uint32_t
    timestamp; ///< Timestamp. This is the value of the counter at the time of message. This counter is continuously incremented by one per 100ms time.
  uint8_t
    state_code; ///< State code. This field indicates the state of the module. State code contains two parts (upper and lower nibbles). Upper nibble represents the state of rejoin process, and the lower nibble of state code represents the StateCode.
  uint8_t reason_code; ///< Reason code. This is used to get the reason code from firmware point of view.
  uint8_t
    channel; ///< Channel number. If the value of the channel is 0, channel information is not available. In State-I, this represents the channel of association or Invalid if it is startup. In State-II, this represents the channel of next association if module finds better AP in bgscan result. In State-III, this represents the channel at the time of association.
  uint8_t
    rssi; ///< RSSI VALUE. If value of rssi is 100, RSSI information is not available. In State-I, this represents the RSSI of AP at the time of trigger. In State-II, this represent the RSSI of next association. In State-III, this represents the RSSI at the time of final association.
  uint8_t bssid
    [6]; ///< BSSID of AP. If the value of AP BSSID is 00:00:00:00:00:00, MAC information is not available. In State-I, it represents the MAC of AP at the time of scan trigger. In State-II, this represents the MAC of next association. In State-III, this represents the MAC at the time of association.
} sl_wifi_module_state_stats_response_t;
#pragma pack()

/**
 * @struct sl_wifi_system_fw_version_info_t
 * @brief Firmware version information structure.
 */
typedef struct {
  uint8_t build_num;        ///< Build number of the firmware
  uint8_t security_version; ///< Security version indicating if security is enabled or disabled
  uint8_t minor;            ///< Minor version number of the firmware
  uint8_t major;            ///< Major version number of the firmware
} sl_wifi_system_fw_version_info_t;

/**
 * @struct sl_wifi_fw_version_ext_info_t
 * @brief Firmware version extended information structure.
 */
typedef struct {
  uint8_t patch_num;                  ///< Patch number of the firmware
  uint8_t customer_id : 4;            ///< Customer ID
  uint8_t build_number_extension : 4; ///< Build number extension
  uint8_t rom_id;                     ///< ROM ID of the firmware
  uint8_t chip_id;                    ///< Chip ID of the device
} sl_wifi_fw_version_ext_info_t;

/**
 * @struct sl_wifi_firmware_header_t
 * @brief Firmware header information structure.
 */
typedef struct {
  uint16_t control_flags;                           ///< Control flags for the firmware image
  uint16_t sha_type;                                ///< SHA type used for the firmware image
  uint32_t magic_no;                                ///< Magic number identifying the firmware image
  uint32_t image_size;                              ///< Size of the firmware image in bytes
  sl_wifi_system_fw_version_info_t fw_version_info; ///< Firmware version information
  uint32_t flash_location; ///< Address location in flash memory where the firmware image is stored
  uint32_t crc;            ///< Cyclic Redundancy Check (CRC) value of the firmware image
  uint32_t mic[4];         ///< Message Integrity Code (MIC) of the firmware image
  uint32_t reserved;       ///< Reserved fields for future use
  sl_wifi_fw_version_ext_info_t fw_version_ext_info; ///< Firmware version extended information
  uint32_t reserved1[4];                             ///< Reserved fields for future use
} sl_wifi_firmware_header_t;

/**
 * @struct sl_wifi_ap_reconfiguration_t
 * @brief Wi-Fi Access Point dynamic configuration structure.
 *
 * Specifies the dynamic configuration parameters for a Wi-Fi Access Point (AP).
 */
typedef struct {
  uint8_t beacon_stop; ///< Beaconing control when no clients are connected
} sl_wifi_ap_reconfiguration_t;

/// Wi-Fi Access Point dynamic configuration structure.
typedef sl_wifi_ap_reconfiguration_t SL_DEPRECATED_API_WISECONNECT_4_0 sl_si91x_ap_reconfiguration_t;

/**
 * @enum sl_wifi_system_coex_mode_t
 * @brief Wireless co-existence mode
 * @note Only BLE, WLAN, and WLAN + BLE modes are supported.
 */
typedef enum {
  SL_WIFI_SYSTEM_WLAN_ONLY_MODE      = 0, ///< Wireless local area network (WLAN) only mode
  SL_WIFI_SYSTEM_WLAN_MODE           = 1, ///< WLAN mode (not currently supported)
  SL_WIFI_SYSTEM_BLUETOOTH_MODE      = 4, ///< Bluetooth only mode (not currently supported)
  SL_WIFI_SYSTEM_WLAN_BLUETOOTH_MODE = 5, ///< WLAN and Bluetooth mode (not currently supported)
  SL_WIFI_SYSTEM_DUAL_MODE           = 8, ///< Dual mode (not currently supported)
  SL_WIFI_SYSTEM_WLAN_DUAL_MODE      = 9, ///< WLAN dual mode (not currently supported)
  SL_WIFI_SYSTEM_BLE_MODE      = 12, ///< Bluetooth Low Energy (BLE) only mode, used when power save mode is not needed.
  SL_WIFI_SYSTEM_WLAN_BLE_MODE = 13, ///< WLAN and BLE mode
  __SL_WIFI_FORCE_COEX_ENUM_16BIT = 0xFFFF ///< Force the enumeration to be 16-bit
} sl_wifi_system_coex_mode_t;

/**
 * @enum sl_wifi_operation_mode_t
 * @brief Enumeration of Wi-Fi operation modes for the Si91x wireless device.
 *
 * @details This enumeration defines the various operation modes supported by the wireless device.
 * Each mode configures the device for a specific use case, such as client mode, access point mode, or transceiver mode.
 *
 * @var SL_WIFI_CLIENT_MODE
 * Wi-Fi personal client mode.
 *
 * @var SL_WIFI_ENTERPRISE_CLIENT_MODE
 * Wi-Fi enterprise client mode.
 *
 * @var SL_WIFI_ACCESS_POINT_MODE
 * Wi-Fi access point mode.
 *
 * @var SL_WIFI_TRANSCEIVER_MODE
 * Wi-Fi transceiver mode.
 *
 * @var SL_WIFI_TRANSMIT_TEST_MODE
 * Wi-Fi transmit test mode.
 *
 * @var SL_WIFI_CONCURRENT_MODE
 * Wi-Fi concurrent mode (e.g., AP + STA).
 *
 * @var __SL_WIFI_FORCE_OPERATION_ENUM_16BIT
 * Forces the enumeration to be 16-bit wide.
 **/
typedef enum {
  SL_WIFI_CLIENT_MODE                  = 0,     ///< Wi-Fi personal client mode
  SL_WIFI_ENTERPRISE_CLIENT_MODE       = 2,     ///< Wi-Fi enterprise client mode
  SL_WIFI_ACCESS_POINT_MODE            = 6,     ///< Wi-Fi access point mode
  SL_WIFI_TRANSCEIVER_MODE             = 7,     ///< Wi-Fi transceiver mode
  SL_WIFI_TRANSMIT_TEST_MODE           = 8,     ///< Wi-Fi transmit test mode
  SL_WIFI_CONCURRENT_MODE              = 9,     ///< Wi-Fi concurrent mode
  __SL_WIFI_FORCE_OPERATION_ENUM_16BIT = 0xFFFF ///< Force the enumeration to be 16-bit
} sl_wifi_operation_mode_t;

/**
 * @enum sl_wifi_system_performance_profile_t
 * @brief Performance profile
 */
typedef enum {
  SL_WIFI_SYSTEM_HIGH_PERFORMANCE,      ///< Power save is disabled and throughput is maximum.
  SL_WIFI_SYSTEM_ASSOCIATED_POWER_SAVE, ///< Low power profile when the device is associated with an AP (MAX PSP).
  SL_WIFI_SYSTEM_ASSOCIATED_POWER_SAVE_LOW_LATENCY, ///< Low power profile when the device is associated with an AP (FAST PSP).
  SL_WIFI_SYSTEM_DEEP_SLEEP_WITHOUT_RAM_RETENTION, ///< Deep Sleep without RAM Retention when the device is not associated with AP.
  SL_WIFI_SYSTEM_DEEP_SLEEP_WITH_RAM_RETENTION ///< Deep Sleep with RAM Retention when the device is not associated with AP.
} sl_wifi_system_performance_profile_t;

/**
 * @struct sl_wifi_timeout_t
 * @brief Timeout Configuration Structure
 */
typedef struct {
  uint16_t
    active_chan_scan_timeout_value; ///< Time spent on each channel when performing active scan (milliseconds). Default value of 100 msec is used when SL_WIFI_DEFAULT_ACTIVE_CHANNEL_SCAN_TIME is passed.
  uint16_t
    auth_assoc_timeout_value; ///< Authentication and association timeout value. Default value of 300 msec is used when SL_WIFI_DEFAULT_AUTH_ASSOCIATION_TIMEOUT is passed.
  uint16_t
    keep_alive_timeout_value; ///< Keep Alive Timeout value. Default value of 30 seconds is used when SL_WIFI_DEFAULT_KEEP_ALIVE_TIMEOUT is passed.
  uint16_t
    passive_scan_timeout_value; ///< Time spent on each channel when performing passive scan (milliseconds). The minimum passive_scan_timeout_value is 5 msec, and maximum is 1000 msec. Default value of 400 msec is used when SL_WIFI_DEFAULT_PASSIVE_CHANNEL_SCAN_TIME is passed.
} sl_wifi_timeout_t;

/**
 * @struct sl_wifi_request_tx_test_info_t
 * @brief Wi-Fi TX test info
 */
typedef struct {
  uint16_t enable; ///< Enable/disable TX test mode
  uint16_t power;  ///< TX power in dBm.  Range : 2 - 18 dBm.
                   ///<
  ///< @note 1. User can configure the maximum power level allowed for the given frequency in the configured region by providing 127 as power level.
  ///< @note 2. User should configure a minimum delay (approx. 10 msec) before and after sl_si91x_transmit_test_start API to observe a stable output at requested dBm level.
  uint32_t rate;   ///< Transmit data rate
                   ///<     ### Data Rates ###
                   ///<			Data rate(Mbps)	|	Value of rate
                   ///<			:--------------:|:-------------------:
                   ///<			1		            |	0
                   ///<			2		            |	2
                   ///<			5.5		          |	4
                   ///<			11		          |	6
                   ///<			6		            |	139
                   ///<			9		            |	143
                   ///<			12		          |	138
                   ///<			18		          |	142
                   ///<			24		          |	137
                   ///<			36		          |	141
                   ///<			48		          |	136
                   ///<			54		          |	140
                   ///<			MCS0		        |	256
                   ///<			MCS1		        |	257
                   ///<			MCS2		        |	258
                   ///<			MCS3		        |	259
                   ///<			MCS4		        |	260
                   ///<			MCS5		        |	261
                   ///<			MCS6		        |	262
                   ///<			MCS7		        |	263
  uint16_t length; ///< TX packet length. Range: [24 - 1500] bytes in Burst mode and [24 - 260] bytes in Continuous mode
  uint16_t mode;   ///< TX test mode mode.
                   ///<
                   ///< 0 - Burst Mode.
                   ///<
                   ///< 1 - Continuous Mode.
                   ///<
                   ///< 2 - Continuous wave Mode (non modulation) in DC mode.
                   ///<
                   ///< 3 - Continuous wave Mode (non modulation) in single tone mode (center frequency -2.5 MHz).
                   ///<
                   ///< 4 - Continuous wave Mode (non modulation) in single tone mode (center frequency +5 MHz).
                   ///<
  ///< `Burst mode`: DUT transmits a burst of packets with the given power, rate, and length in the channel configured.
  ///<               The burst size will be determined by the number of packets and if its zero, then DUT keeps transmitting till a sl_si91x_transmit_test_stop API is called.
  ///<
  ///< `Continuous Mode`: The DUT transmits a unmodulated waveform continuously
  ///<
  ///< `Continuous Wave Mode (Non-Modulation) in DC Mode`: The DUT transmits a spectrum only at the center frequency of the channel.
  ///<                                                   A basic signal with no modulation is that of a sine wave and is usually referred to as a continuous wave (CW) signal.
  ///<                                                   A basic signal source produces sine waves. Ideally, the sine wave is perfect. In the frequency domain, it is viewed as a single line at some specified frequency.
  ///<
  ///<  `Continuous Wave Mode (Non-Modulation) in single tone Mode (Center frequency -2.5 MHz)`: The DUT transmits a spectrum that is generated at -2.5MHz from the center frequency of the channel selected.
  ///<                                                                                        Some amount of carrier leakage will be seen at Center Frequency. For example, for 2412 MHz, the output would be seen at 2409.5 MHz.
  ///<
  ///<  `Continuous Wave Mode (Non-Modulation) in single tone Mode (Center frequency +5 MHz)`: The DUT transmits a spectrum that is generated at 5MHz from the center frequency of the channel selected.
  ///<                                                                                      Some amount of carrier leakage will be seen at Center Frequency. For example, for 2412MHz, the output would be seen at 2417 MHz.
  uint16_t channel; ///< Channel number in 2.4 GHZ / 5 GHZ.
  ///<			###The following table maps the channel number to the actual radio frequency in the 2.4 GHz spectrum. ###
  ///<			Channel numbers (2.4GHz)|	Center frequencies for 20 MHz channel width
  ///<			:----------------------:|:-----------------------------------------------:
  ///<			1			|	2412
  ///<			2			|	2417
  ///<			3			|	2422
  ///<			4			|	2427
  ///<			5			|	2432
  ///<			6			|	2437
  ///<			7			|	2442
  ///<			8			|	2447
  ///<			9			|	2452
  ///<			10			|	2457
  ///<			11			|	2462
  ///<			12			|	2467
  ///<			13			|	2472
  ///< @note	To start transmit test in channels 12 and 13, configure region parameters in sl_si91x_set_device_region API.
  ///<    ###	The following table maps the channel number to the actual radio frequency in the 5 GHz spectrum for 20MHz channel bandwidth. The channel numbers in 5 GHz range is from 36 to 165. ###
  ///< 		Channel Numbers(5GHz) |	Center frequencies for 20MHz channel width
  ///< 		:--------------------:|:------------------------------------------:
  ///<		36		      |5180
  ///<		40		      |5200
  ///<		44		      |5220
  ///<		48		      |5240
  ///<		52		      |5260
  ///<		56	        |5280
  ///<		60		      |5300
  ///<		64		      |5320
  ///<		149		      |5745
  ///<		153		      |5765
  ///<		157		      |5785
  ///<		161		      |5805
  ///<		165		      |5825
  uint16_t rate_flags; ///< Rate flags
  ///< BIT(6) - Immediate Transfer, set this bit to transfer packets immediately ignoring energy/traffic in channel.
  uint16_t channel_bw;  ///< Channel Bandwidth
  uint16_t aggr_enable; ///< tx test mode aggr_enable
  uint16_t reserved;    ///< Reserved
  uint16_t no_of_pkts;  ///< Number of packets
  uint32_t delay;       ///< Delay
  uint8_t enable_11ax;  ///< 11AX_ENABLE 0-disable, 1-enable
  uint8_t coding_type;  ///< Coding_type 0-BCC 1-LDPC
  uint8_t nominal_pe;   ///< Indicates Nominal T-PE value. 0-0Us 1-8Us 2-16Us
  uint8_t
    ul_dl; ///< Indicates whether the PPDU is UL/DL. Set it to 1 if PPDU is to be sent by station to AP; 0 if PPDU is to be sent by AP to station.
  uint8_t he_ppdu_type; ///< he_ppdu_type 0-HE SU PPDU, 1-HE ER SU PPDU, 2-HE TB PPDU, 3-HE MU PPDU
  uint8_t
    beam_change; ///< Indicates the spatial mapping of pre-HE and HE fields. Enter 0 for pre-HE and HE fields are spatially mapped in the same way, and enter 1 for pre-HE and HE fields are spatially mapped differently.
  uint8_t bw;    ///< Indicates the BW for the PPDU: 0 for 242-tone RU, 1 for upper 106-tone RU.
  uint8_t
    stbc; ///< Indicates whether STBC is used for PPDU transmission. Set to 0 for no STBC and 1 for STBC (only if DCM field is set to 0).
  uint8_t
    tx_bf; ///< Indicates whether beamforming matrix is applied to the transmission. 0 - no beamforming matrix, 1 - beamforming matrix.
  uint8_t gi_ltf;        ///< Indicates the GI and LTF size. GI_LTF should be in the range 0-3.
  uint8_t dcm;           ///< Indicates whether DCM is applied to Data Symbols. 0 - No DCM, 1 - DCM.
  uint8_t nsts_midamble; ///< Indicates the NSTS and Midamble Periodicity. NSTS_MIDAMBLE should be in the range 0-7.
  uint8_t
    spatial_reuse; ///< spatial_reuse shall be in the range 0-15. 4 indicates that spatial reuse is allowed during the transmission of PPDU.
  uint8_t bss_color;              ///< Color value of BSS. Must be in the range 0 to 63.
  uint16_t he_siga2_reserved;     ///< HE_SIGA2_RESERVED should be in the range 0-511.
  uint8_t ru_allocation;          ///< Indicates the RU Allocation Subfield for 20MHz BW. Must be in the range 0-255.
  uint8_t n_heltf_tot;            ///< Indicates the number of HE-LTF to be transmitted. Can be in the range 0-7.
  uint8_t sigb_dcm;               ///< Indicates whether DCM is applied to SIG-B Symbols. 0-disable, 1-enable
  uint8_t sigb_mcs;               ///< Indicates the MCS for SIG-B Symbols. Allowed range is 0-5.
  uint16_t user_sta_id;           ///< Indicates the Station ID of the intended user. Allowed range is 0-2047.
  uint8_t user_idx;               ///< USER_IDX should be in the range 0-8.
  uint8_t sigb_compression_field; ///< SIGB_COMPRESSION_FIELD should be 0/1.
} sl_wifi_request_tx_test_info_t;

/**
 * @struct sl_wifi_advanced_stats_response_t
 * @brief Specific Wi-Fi advanced statistics structure.
 */
typedef struct {
  uint32_t beacon_lost_count; ///< Number of missed beacons
  uint32_t beacon_rx_count;   ///< Number of received beacons
  uint32_t mcast_rx_count;    ///< Multicast packets received
  uint32_t mcast_tx_count;    ///< Multicast packets transmitted
  uint32_t ucast_rx_count;    ///< Unicast packets received
  uint32_t ucast_tx_count;    ///< Unicast packets transmitted
  uint32_t
    overrun_count; ///< Number of packets dropped either at ingress or egress, due to lack of buffer memory to retain all packets.
} sl_wifi_advanced_stats_response_t;

/**
 * @struct sl_wifi_twt_response_t
 * @brief TWT response structure.
 */
typedef struct {
  uint8_t wake_duration;      ///< Wake duration
  uint8_t wake_duration_unit; ///< Wake duration unit
  uint8_t wake_int_exp;       ///< Wake interval exponent
  uint8_t negotiation_type;   ///< Negotiation type
  uint16_t wake_int_mantissa; ///< Wake interval mantissa
  uint8_t implicit_twt;       ///< Impilcit TWT
  uint8_t un_announced_twt;   ///< Unannounced TWT
  uint8_t triggered_twt;      ///< Triggered TWT
  uint8_t twt_channel;        ///< TWT channel
  uint8_t twt_protection;     ///< TWT Protection
  uint8_t twt_flow_id;        ///< TWT flow ID
} sl_wifi_twt_response_t;

/**
 * @struct sl_wifi_vendor_ie_t
 * @brief Structure to configure a vendor-specific IE.
 *
 * This structure is used to add or remove a vendor-specific IE.
 */
typedef struct {
  uint8_t unique_id;          ///< Unique ID for the IE (must be < SLI_MAX_VENDOR_IE)
  uint16_t mgmt_frame_bitmap; ///< Bitmap indicating which management frames to include the IE in
  uint16_t ie_buffer_length;  ///< Length of the IE buffer (must be < SLI_MAX_VENDOR_IE_BUFFER_LENGTH)
  uint8_t *ie_buffer;         ///< Pointer to raw IE buffer.
} sl_wifi_vendor_ie_t;

/// Generic Wi-Fi interface information structure
typedef struct {
  uint16_t
    wlan_state; ///< WLAN state: 1 = connected, 0 = disconnected in station mode; number of stations connected in AP mode.
  uint16_t channel_number; ///< Channel number of connected AP in station mode; channel number of the module in AP mode.
  uint8_t ssid[SL_WIFI_SSID_LEN]; ///< SSID of connected AP in station mode; SSID of the module in AP mode
  uint8_t sec_type;               ///< Security type of connected AP is supported in station mode, but not in AP mode.
  uint8_t psk_pmk[SL_WIFI_MAX_PSK_LENGTH]; ///< PSK for AP mode, PMK for station mode.
  uint8_t bssid[SL_WIFI_BSSID_LENGTH];     ///< BSSID address of connected AP in station mode; not supported in AP mode.
  uint8_t
    wireless_mode; ///< Wireless mode used in connected AP (6 - AX, 4 - N, 3 - G, 1 - B) in station mode; not supported in AP mode.
  uint8_t mac_address[SL_WIFI_MAC_ADDRESS_LENGTH]; ///< MAC address of the module.
} sl_wifi_interface_info_t;
/** @} */
