/***************************************************************************//**
 * @file sl_wisun_types.h
 * @brief Wi-SUN types
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_WISUN_TYPES_H
#define SL_WISUN_TYPES_H

#include <stdint.h>
#include "sl_common.h"
#include "sl_status.h"
#include "socket/socket.h"

/**************************************************************************//**
 * @defgroup SL_WISUN_TYPES API type definitions
 * @ingroup SL_WISUN_API
 * @{
 *****************************************************************************/

/// Maximum size of the Wi-SUN network name
#define SL_WISUN_NETWORK_NAME_SIZE 32
/// Size of a MAC address
#define SL_WISUN_MAC_ADDRESS_SIZE 8
/// Size of a channel mask
#define SL_WISUN_CHANNEL_MASK_SIZE 32
/// Size of the filter bitfield
#define SL_WISUN_FILTER_BITFIELD_SIZE ((SL_WISUN_TRACE_GROUP_COUNT + 7) / 8)
/// Maximum fragment duration. Disables advert fragmentation.
#define SL_WISUN_ADVERT_FRAGMENT_DISABLE UINT32_MAX
/// Maximum number of PhyModeId allowed in POM-IE
#define SL_WISUN_MAX_PHY_MODE_ID_COUNT 15
/// Number of GTKs returned by sl_wisun_get_gtks()
#define SL_WISUN_GTK_NUM  4
/// Length of a GTK in bytes
#define SL_WISUN_GTK_LEN  16
/// Number of LGTKs returned by sl_wisun_get_gtks()
#define SL_WISUN_LGTK_NUM  3
/// Length of a PMK in bytes
#define SL_WISUN_PMK_LEN 32
/// Length of a GAK in bytes
#define SL_WISUN_GAK_LEN  16
/// Maximum size of EAP identity
#define SL_WISUN_EAP_IDENTITY_SIZE 32

/// Enumerations for device type
typedef enum {
  /// FFN Router
  SL_WISUN_ROUTER = 0,
  /// LFN Router (experimental, for evaluation purposes only)
  SL_WISUN_LFN = 1,
  /// Border Router
  SL_WISUN_BORDER_ROUTER = 2
} sl_wisun_device_type_t;

/// Enumerations for network size
typedef enum {
  /// Determine the size from PAN advertisements
  SL_WISUN_NETWORK_SIZE_AUTOMATIC     = 0,
  /// Small size (less than 100 nodes)
  SL_WISUN_NETWORK_SIZE_SMALL         = 1,
  /// Medium size (100 to 800 nodes)
  SL_WISUN_NETWORK_SIZE_MEDIUM        = 2,
  /// Large size (800 to 1500 nodes)
  SL_WISUN_NETWORK_SIZE_LARGE         = 3,
  /// Test network (a few nodes)
  SL_WISUN_NETWORK_SIZE_TEST          = 4,
  /// Certification configuration
  SL_WISUN_NETWORK_SIZE_CERTIFICATION = 5
} sl_wisun_network_size_t;

/// Enumerations for IP address type
typedef enum {
  /// Device link-local address
  SL_WISUN_IP_ADDRESS_TYPE_LINK_LOCAL       = 0,
  /// Device global unicast address
  SL_WISUN_IP_ADDRESS_TYPE_GLOBAL           = 1,
  /// Border router global unicast address
  SL_WISUN_IP_ADDRESS_TYPE_BORDER_ROUTER    = 2,
  /// Link-local address of the primary parent
  SL_WISUN_IP_ADDRESS_TYPE_PRIMARY_PARENT   = 3,
  /// Link-local address of the secondary parent
  SL_WISUN_IP_ADDRESS_TYPE_SECONDARY_PARENT = 4
} sl_wisun_ip_address_type_t;

/// Enumerations for certificate options
typedef enum {
  /// Empty option
  SL_WISUN_CERTIFICATE_OPTION_NONE    = 0,
  /// Certificate is appended to a chain
  SL_WISUN_CERTIFICATE_OPTION_APPEND  = 1,
  /// Certificate data will remain in scope
  SL_WISUN_CERTIFICATE_OPTION_IS_REF  = 2,
  /// Certificate has a private key
  SL_WISUN_CERTIFICATE_OPTION_HAS_KEY = 4
} sl_wisun_certificate_option_t;

/// Enumerations for private key options
typedef enum {
  /// Empty option
  SL_WISUN_PRIVATE_KEY_OPTION_NONE    = 0,
  /// Private key data will remain in scope
  SL_WISUN_PRIVATE_KEY_OPTION_IS_REF  = 1
} sl_wisun_private_key_option_t;

/// Enumerations for statistics type
typedef enum {
  /// PHY/RF statistics
  SL_WISUN_STATISTICS_TYPE_PHY        = 0,
  /// MAC statistics
  SL_WISUN_STATISTICS_TYPE_MAC        = 1,
  /// Frequency hopping statistics
  SL_WISUN_STATISTICS_TYPE_FHSS       = 2,
  /// Wi-SUN statistics
  SL_WISUN_STATISTICS_TYPE_WISUN      = 3,
  /// 6LoWPAN/IP stack statistics
  SL_WISUN_STATISTICS_TYPE_NETWORK    = 4,
  /// Regional regulation
  SL_WISUN_STATISTICS_TYPE_REGULATION = 5,
  /// Heap usage
  SL_WISUN_STATISTICS_TYPE_HEAP       = 6
} sl_wisun_statistics_type_t;

/// Enumerations for regulatory domain
typedef enum {
  /// World-wide (2.4 GHz)
  SL_WISUN_REGULATORY_DOMAIN_WW       = 0,
  /// North America
  SL_WISUN_REGULATORY_DOMAIN_NA       = 1,
  /// Japan
  SL_WISUN_REGULATORY_DOMAIN_JP       = 2,
  /// Europe
  SL_WISUN_REGULATORY_DOMAIN_EU       = 3,
  /// China
  SL_WISUN_REGULATORY_DOMAIN_CN       = 4,
  /// India
  SL_WISUN_REGULATORY_DOMAIN_IN       = 5,
  /// Mexico
  SL_WISUN_REGULATORY_DOMAIN_MX       = 6,
  /// Brazil
  SL_WISUN_REGULATORY_DOMAIN_BZ       = 7,
  /// Australia
  SL_WISUN_REGULATORY_DOMAIN_AZ       = 8, // shared with NZ
  /// New Zealand
  SL_WISUN_REGULATORY_DOMAIN_NZ       = 8, // shared with AZ
  /// South Korea
  SL_WISUN_REGULATORY_DOMAIN_KR       = 9,
  /// Philippines
  SL_WISUN_REGULATORY_DOMAIN_PH       = 10,
  /// Malaysia
  SL_WISUN_REGULATORY_DOMAIN_MY       = 11,
  /// Hong Kong
  SL_WISUN_REGULATORY_DOMAIN_HK       = 12,
  /// Singapore
  SL_WISUN_REGULATORY_DOMAIN_SG       = 13,
  /// Thailand
  SL_WISUN_REGULATORY_DOMAIN_TH       = 14,
  /// Vietnam
  SL_WISUN_REGULATORY_DOMAIN_VN       = 15,
  /// Application-specific domain
  SL_WISUN_REGULATORY_DOMAIN_APP      = 255
} sl_wisun_regulatory_domain_t;

/// Enumerations for operating class
typedef enum {
  /// Operating class# 1
  SL_WISUN_OPERATING_CLASS_1         = 1,
  /// Operating class# 2
  SL_WISUN_OPERATING_CLASS_2         = 2,
  /// Operating class# 3
  SL_WISUN_OPERATING_CLASS_3         = 3,
  /// Operating class# 4
  SL_WISUN_OPERATING_CLASS_4         = 4,
  /// Operating class# 5
  SL_WISUN_OPERATING_CLASS_5         = 5,
  /// Application-specific class
  SL_WISUN_OPERATING_CLASS_APP       = 255
} sl_wisun_operating_class_t;

/// Enumerations for operating mode
typedef enum {
  /// Operating mode# 1a
  SL_WISUN_OPERATING_MODE_1A         = 0x1a,
  /// Operating mode# 1b
  SL_WISUN_OPERATING_MODE_1B         = 0x1b,
  /// Operating mode# 2a
  SL_WISUN_OPERATING_MODE_2A         = 0x2a,
  /// Operating mode# 2b
  SL_WISUN_OPERATING_MODE_2B         = 0x2b,
  /// Operating mode# 3
  SL_WISUN_OPERATING_MODE_3          = 0x03,
  /// Operating mode# 4a
  SL_WISUN_OPERATING_MODE_4A         = 0x4a,
  /// Operating mode# 4b
  SL_WISUN_OPERATING_MODE_4B         = 0x4b,
  /// Operating mode# 5
  SL_WISUN_OPERATING_MODE_5          = 0x05
} sl_wisun_operating_mode_t;

/// Enumerations for multicast group action
typedef enum {
  /// Join a multicast group
  SL_WISUN_MULTICAST_GROUP_ACTION_JOIN  = 0,
  /// Leave a multicast group
  SL_WISUN_MULTICAST_GROUP_ACTION_LEAVE = 1
} sl_wisun_multicast_group_action_t;

/// Enumerations for channel spacing
typedef enum {
  /// 100 kHz
  SL_WISUN_CHANNEL_SPACING_100KHZ = 0,
  /// 200 kHz
  SL_WISUN_CHANNEL_SPACING_200KHZ = 1,
  /// 400 kHz
  SL_WISUN_CHANNEL_SPACING_400KHZ = 2,
  /// 600 kHz
  SL_WISUN_CHANNEL_SPACING_600KHZ = 3,
  /// 250 kHz
  SL_WISUN_CHANNEL_SPACING_250KHZ = 4,
  /// 800 kHz
  SL_WISUN_CHANNEL_SPACING_800KHZ = 5,
  /// 1200 kHz
  SL_WISUN_CHANNEL_SPACING_1200KHZ = 6
} sl_wisun_channel_spacing_t;

/// Channel spacing 100 kHz for backward compatibility
#define SL_WISUN_CHANNEL_SPACING_100HZ  SL_WISUN_CHANNEL_SPACING_100KHZ

/// Channel spacing 200 kHz for backward compatibility
#define SL_WISUN_CHANNEL_SPACING_200HZ  SL_WISUN_CHANNEL_SPACING_200KHZ

/// Channel spacing 400 kHz for backward compatibility
#define SL_WISUN_CHANNEL_SPACING_400HZ  SL_WISUN_CHANNEL_SPACING_400KHZ

/// Channel spacing 600 kHz for backward compatibility
#define SL_WISUN_CHANNEL_SPACING_600HZ  SL_WISUN_CHANNEL_SPACING_600KHZ

/// Enumerations for join state
typedef enum {
  /// Join state 0: Disconnected
  SL_WISUN_JOIN_STATE_DISCONNECTED       = 0,
  /// Join state 1: Select PAN
  SL_WISUN_JOIN_STATE_SELECT_PAN         = 1,
  /// Join state 2: Authenticate
  SL_WISUN_JOIN_STATE_AUTHENTICATE       = 2,
  /// Join state 3: Acquire PAN config
  SL_WISUN_JOIN_STATE_ACQUIRE_PAN_CONFIG = 3,
  /// Join state 4: Configure routing
  SL_WISUN_JOIN_STATE_CONFIGURE_ROUTING  = 4,
  /// Join state 5: Operational
  SL_WISUN_JOIN_STATE_OPERATIONAL        = 5,
  /// Disconnecting
  SL_WISUN_JOIN_STATE_DISCONNECTING      = 6,
  /// Join state 4: Preferred parent selection
  SL_WISUN_JOIN_STATE_PARENT_SELECT      = 41,
  /// Join state 4: DHCP address acquisition
  SL_WISUN_JOIN_STATE_DHCP               = 42,
  /// Join state 4: Address registration
  SL_WISUN_JOIN_STATE_EARO               = 43,
  /// Join state 4: DAO registration
  SL_WISUN_JOIN_STATE_DAO                = 44
} sl_wisun_join_state_t;

/// Enumerations for network update flags
typedef enum {
  /// Global IP modification flag bit
  SL_WISUN_NETWORK_UPDATE_FLAGS_GLOBAL_IP        = 0,
  /// Primary Parent modification flag bit
  SL_WISUN_NETWORK_UPDATE_FLAGS_PRIMARY_PARENT   = 1,
  /// Secondary parent modification flag bit
  SL_WISUN_NETWORK_UPDATE_FLAGS_SECONDARY_PARENT = 2,
  /// Hop count modification flag bit
  SL_WISUN_NETWORK_UPDATE_FLAGS_HOP_COUNT        = 3,
} sl_wisun_network_update_flags_t;

/// Enumerations for PHY config type
typedef enum {
  /// FAN1.0 PHY configuration
  SL_WISUN_PHY_CONFIG_FAN10        = 0,
  /// FAN1.1 PHY configuration
  SL_WISUN_PHY_CONFIG_FAN11        = 1,
  /// Explicit PHY configuration
  SL_WISUN_PHY_CONFIG_EXPLICIT     = 2,
  /// Explicit RAIL configuration
  SL_WISUN_PHY_CONFIG_IDS          = 3,
  /// Custom FSK PHY configuration
  SL_WISUN_PHY_CONFIG_CUSTOM_FSK   = 4,
  /// Custom OFDM PHY configuration
  SL_WISUN_PHY_CONFIG_CUSTOM_OFDM  = 5,
  /// Custom OQPSK PHY configuration
  SL_WISUN_PHY_CONFIG_CUSTOM_OQPSK = 6,
} sl_wisun_phy_config_type_t;

/// Enumeration for LFN configuration profile
typedef enum {
  /// Profile for test usage, best performance but highest power consumption
  SL_WISUN_LFN_PROFILE_TEST     = 0,
  /// Profile providing balance between power consumption and performance
  SL_WISUN_LFN_PROFILE_BALANCED = 1,
  /// Profile optimized for low power consumption
  SL_WISUN_LFN_PROFILE_ECO      = 2
} sl_wisun_lfn_profile_t;

/// Enumeration for CRC type
typedef enum {
  /// No CRC (OFDM and OQPSK only)
  SL_WISUN_NO_CRC      = 0,
  /// 2 bytes CRC
  SL_WISUN_2_BYTES_CRC = 1,
  /// 4 bytes CRC
  SL_WISUN_4_BYTES_CRC = 2
} sl_wisun_crc_type_t;

/// Wi-SUN Message API common header
SL_PACK_START(1)
typedef struct {
  /// Total length of the message in bytes, this field included
  uint16_t length;
  /// ID (request, confirmation, indication) of the message
  uint8_t id;
  /// Processing metadata for the message
  uint8_t info;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_header_t;
SL_PACK_END()

/// PHY/RF statistics
typedef struct {
  /// Number of CRC failures on reception.
  uint32_t crc_fails;
  /// Number of transmission timeouts.
  uint32_t tx_timeouts;
  /// Number of reception timeouts.
  uint32_t rx_timeouts;
} sl_wisun_statistics_phy_t;

/// MAC statistics
typedef struct {
  /// Current number of frames in the MAC transmission queue.
  uint16_t tx_queue_size;
  /// Highest number of frames in the MAC transmission queue.
  uint16_t tx_queue_peak;
  /// Number of successfully received MAC frames.
  uint32_t rx_count;
  /// Number of transmitted MAC frames.
  uint32_t tx_count;
  /// Number of successfully received broadcast MAC frames.
  uint32_t bc_rx_count;
  /// Number of transmitted broadcast MAC frames.
  uint32_t bc_tx_count;
  /// Number of successfully received MAC frames discarded during processing.
  uint32_t rx_drop_count;
  /// Amount of transmitted MAC data in bytes. FCS is not included.
  uint32_t tx_bytes;
  /// Amount of successfully received MAC data in bytes. FCS is not included.
  uint32_t rx_bytes;
  /// Number of failed MAC transmissions.
  uint32_t tx_failed_count;
  /// Number of retried MAC transmissions.
  uint32_t retry_count;
  /// Number of MAC CCA attempts.
  uint32_t cca_attempts_count;
  /// Number of failed MAC transmissions due to CCA.
  uint32_t failed_cca_count;
  /// Number of successfully received MAC frames using mode_switch.
  uint32_t rx_ms_count;
  /// Number of transmitted MAC frames using mode switch.
  uint32_t tx_ms_count;
  /// Number of failed reception using mode switch (no data after PHR or MDR Command).
  uint32_t rx_ms_failed_count;
  /// Number of failed MAC frames transmission using mode switch.
  uint32_t tx_ms_failed_count;
  /// Number of successfully received MAC frames using concurrent detection.
  uint32_t rx_cd_count;
  /// Number of transmitted MAC frames using concurrent detection.
  uint32_t tx_cd_count;
  /// Cumulated idle duration in seconds.
  uint32_t idle_duration_s;
  /// Radio TX duration in milliseconds
  uint32_t radio_tx_duration_ms;
  /// MAC RX availability percentage.
  uint8_t rx_availability_percentage;
  /// Reserved, set to zero
  uint8_t reserved[3];
} sl_wisun_statistics_mac_t;

/// Frequency hopping statistics
typedef struct {
  /// Estimated clock drift to the parent in microseconds.
  int16_t drift_compensation;
  /// Estimated number of hops to the border router based on RPL rank.
  uint16_t hop_count;
  /// Number of seconds since last timing information from the parent.
  uint16_t synch_interval;
  /// Deprecated
  int16_t prev_avg_synch_fix;
  /// Deprecated
  uint32_t synch_lost;
  /// Number of times a transmission attempt has failed due to lack of timing information.
  uint32_t unknown_neighbor;
} sl_wisun_statistics_fhss_t;

/// Wi-SUN statistics
typedef struct {
  /// Number of received PAN control frames.
  uint32_t pan_control_rx_count;
  /// Number of completed PAN control transmission requests.
  uint32_t pan_control_tx_count;
} sl_wisun_statistics_wisun_t;

/// 6LoWPAN/IP stack statistics
typedef struct {
  /// Number of received IP6 packets.
  uint32_t ip_rx_count;
  /// Number of transmitted IPv6 packets.
  uint32_t ip_tx_count;
  /// Number of discarded IPv6 packets during processing.
  uint32_t ip_rx_drop;
  /// Number of discarded IPv6 packets due to a checksum error.
  uint32_t ip_cksum_error;
  /// Amount of transmitted IPv6 data in bytes.
  uint32_t ip_tx_bytes;
  /// Amount received IPv6 data in bytes.
  uint32_t ip_rx_bytes;
  /// Amount of forwarded IPv6 data in bytes.
  uint32_t ip_routed_up;
  /// Number of discarded IPv6 packets due to lack routing information.
  uint32_t ip_no_route;
  /// Number of fragmentation errors in received IPv6 packets.
  uint32_t frag_rx_errors;
  /// Number of fragmentation errors in transmitted IPv6 packets.
  uint32_t frag_tx_errors;
  /// Number of RPL parent changes due to better route cost.
  uint32_t rpl_route_routecost_better_change;
  /// Number of RPL packet forwarding errors due to inconsistent routing information.
  uint32_t ip_routeloop_detect;
  /// Sum of RPL object sizes that have failed allocation in bytes.
  uint32_t rpl_memory_overflow;
  /// Number of failed RPL transmissions to the parent.
  uint32_t rpl_parent_tx_fail;
  /// Number of discarded RPL packets due to an unknown DODAG instance.
  uint32_t rpl_unknown_instance;
  /// Number of times a local repair procedure has been triggered by the node.
  uint32_t rpl_local_repair;
  /// Number of times a global repair has been triggered by the border router.
  uint32_t rpl_global_repair;
  /// Number of discarded RPL packets due to malformed content.
  uint32_t rpl_malformed_message;
  /// Number of seconds without an RPL parent.
  uint32_t rpl_time_no_next_hop;
  /// Amount of memory currently allocated for RPL objects in bytes.
  uint32_t rpl_total_memory;
  /// Number of data buffer allocation attempts.
  uint32_t buf_alloc;
  /// Number of times data buffers have been resized due to lack of header space.
  uint32_t buf_headroom_realloc;
  /// Number of times data buffers have been reorganized due to lack of header space.
  uint32_t buf_headroom_shuffle;
  /// Number of times data buffer resizing has failed.
  uint32_t buf_headroom_fail;
  /// ETX of the primary parent.
  uint16_t etx_1st_parent;
  /// ETX of the secondary parent.
  uint16_t etx_2nd_parent;
  /// Current number of frames in the adaptation layer transmission queue.
  uint16_t adapt_layer_tx_queue_size;
  /// Highest number of frames in the adaptation layer transmission queue.
  uint16_t adapt_layer_tx_queue_peak;
  /// Number of new MPL messages.
  uint16_t mpl_new_messages_count;
  /// Number of received MPL messages.
  uint16_t mpl_rx_count;
  /// Number of forwarded MPL messages.
  uint16_t mpl_forwarded_messages_count;
  /// Number of freed MPL messages.
  uint16_t mpl_freed_messages_count;
  /// Number of deleted MPL messages that were never sent.
  uint16_t mpl_not_tx_count;
} sl_wisun_statistics_network_t;

/// ARIB regulation statistics
typedef struct {
  /// Sum of transmission durations during the last hour in milliseconds
  uint32_t tx_duration_ms;
} sl_wisun_statistics_arib_regulation_t;

/// Regional regulation statistics
typedef union {
  /// ARIB statistics
  sl_wisun_statistics_arib_regulation_t arib;
  /// Regional regulation statistics
  struct {
    /// Sum of transmission durations during the last hour in milliseconds
    uint32_t tx_duration_ms;
    /// Sum of transmission durations per channel during the last hour in milliseconds
    uint32_t tx_duration_per_channel_ms;
    /// Duty cycle warning threshold in milliseconds
    uint32_t duty_cycle_warning_threshold_ms;
    /// Duty cycle warning threshold per channel in milliseconds
    uint32_t duty_cycle_warning_threshold_per_channel_ms;
    /// Duty cycle alert threshold in milliseconds
    uint32_t duty_cycle_alert_threshold_ms;
    /// Duty cycle alert threshold per channel in milliseconds
    uint32_t duty_cycle_alert_threshold_per_channel_ms;
  };
} sl_wisun_statistics_regulation_t;

/// Heap usage statistics
typedef struct {
  /// Highest heap usage in bytes
  uint32_t arena;
  /// Current heap usage in bytes
  uint32_t uordblks;
  /// Current remaining heap in bytes
  uint32_t free;
  /// Total available heap in bytes
  uint32_t total;
} sl_wisun_statistics_heap_t;

/// Statistics
typedef union {
  /// PHY/RF statistics
  sl_wisun_statistics_phy_t phy;
  /// MAC statistics
  sl_wisun_statistics_mac_t mac;
  /// Frequency hopping statistics
  sl_wisun_statistics_fhss_t fhss;
  /// Wi-SUN statistics
  sl_wisun_statistics_wisun_t wisun;
  /// 6LoWPAN/IP stack statistics
  sl_wisun_statistics_network_t network;
  /// Regional regulation statistics
  sl_wisun_statistics_regulation_t regulation;
  /// Heap usage statistics
  sl_wisun_statistics_heap_t heap;
} sl_wisun_statistics_t;

/// FAN1.0 PHY configuration
typedef struct {
  /// Regulatory domain (#sl_wisun_regulatory_domain_t)
  uint8_t reg_domain;
  /// Operating class (#sl_wisun_operating_class_t)
  uint8_t op_class;
  /// Operating mode (#sl_wisun_operating_mode_t)
  uint8_t op_mode;
  /// 1 if FEC is enabled, 0 if not
  uint8_t fec;
} sl_wisun_phy_config_fan10_t;

/// FAN1.1 PHY configuration
typedef struct {
  /// Regulatory domain (#sl_wisun_regulatory_domain_t)
  uint8_t reg_domain;
  /// Channel plan ID
  uint8_t chan_plan_id;
  /// PHY mode ID
  uint8_t phy_mode_id;
} sl_wisun_phy_config_fan11_t;

/// Explicit PHY configuration
typedef struct {
  /// Ch0 center frequency in kHz
  uint32_t ch0_frequency_khz;
  /// Number of channels
  uint16_t number_of_channels;
  /// Channel spacing (#sl_wisun_channel_spacing_t)
  uint8_t channel_spacing;
  /// PHY mode ID
  uint8_t phy_mode_id;
  /// Specifies the set of channels on which the device is permitted to transmit.
  /// This mask applies to asynchronous (PAS, PA, ...), unicast and broadcast frames.
  /// The channel mask is advertised in both US-IE and BS-IE elements.
  uint8_t channel_mask[SL_WISUN_CHANNEL_MASK_SIZE];
} sl_wisun_phy_config_explicit_t;

/// Explicit RAIL configuration
typedef struct {
  /// Protocol ID
  uint16_t protocol_id;
  /// Channel ID
  uint16_t channel_id;
  /// PHY mode ID
  uint8_t phy_mode_id;
  /// Reserved, set to zero
  uint8_t reserved[3];
} sl_wisun_phy_config_ids_t;

/// Custom FSK PHY configuration
typedef struct {
  /// Ch0 center frequency in kHz
  uint32_t ch0_frequency_khz;
  /// Channel spacing in kHz
  uint16_t channel_spacing_khz;
  /// Number of channels
  uint16_t number_of_channels;
  /// PHY mode ID
  uint8_t phy_mode_id;
  /// FSK CRC type (#sl_wisun_crc_type_t)
  uint8_t crc_type;
  /// FSK preamble length in bits
  uint8_t preamble_length;
  /// Reserved, set to zero
  uint8_t reserved[1];
} sl_wisun_phy_config_custom_fsk_t;

/// Custom OFDM PHY configuration
typedef struct {
  /// Ch0 center frequency in kHz
  uint32_t ch0_frequency_khz;
  /// Channel spacing in kHz
  uint16_t channel_spacing_khz;
  /// Number of channels
  uint16_t number_of_channels;
  /// PHY mode ID
  uint8_t phy_mode_id;
  /// OFDM CRC type (#sl_wisun_crc_type_t)
  uint8_t crc_type;
  /// STF length in number of symbols
  uint8_t stf_length;
  /// Reserved, set to zero
  uint8_t reserved[1];
} sl_wisun_phy_config_custom_ofdm_t;

/// Custom OQPSK PHY configuration
typedef struct {
  /// Ch0 center frequency in kHz
  uint32_t ch0_frequency_khz;
  /// Channel spacing in kHz
  uint16_t channel_spacing_khz;
  /// Number of channels
  uint16_t number_of_channels;
  /// PHY mode ID
  uint8_t phy_mode_id;
  /// OFDM CRC type (#sl_wisun_crc_type_t)
  uint8_t crc_type;
  /// OQPSK preamble length in bits
  uint8_t preamble_length;
  /// Reserved, set to zero
  uint8_t reserved[1];
} sl_wisun_phy_config_custom_oqpsk_t;

/// PHY configuration
SL_PACK_START(1)
typedef struct {
  /// Configuration type (#sl_wisun_phy_config_type_t)
  uint32_t type;
  /// Configuration
  union {
    /// Configuration for #SL_WISUN_PHY_CONFIG_FAN10 type
    sl_wisun_phy_config_fan10_t fan10;
    /// Configuration for #SL_WISUN_PHY_CONFIG_FAN11 type
    sl_wisun_phy_config_fan11_t fan11;
    /// Configuration for #SL_WISUN_PHY_CONFIG_EXPLICIT type
    sl_wisun_phy_config_explicit_t explicit_plan;
    /// Configuration for #SL_WISUN_PHY_CONFIG_IDS type
    sl_wisun_phy_config_ids_t ids;
    /// Configuration for #SL_WISUN_PHY_CONFIG_CUSTOM_FSK type
    sl_wisun_phy_config_custom_fsk_t custom_fsk;
    /// Configuration for #SL_WISUN_PHY_CONFIG_CUSTOM_OFDM type
    sl_wisun_phy_config_custom_ofdm_t custom_ofdm;
    /// Configuration for #SL_WISUN_PHY_CONFIG_CUSTOM_OQPSK type
    sl_wisun_phy_config_custom_oqpsk_t custom_oqpsk;
  } config;
} sl_wisun_phy_config_t;
SL_PACK_END()

/// MAC address
SL_PACK_START(1)
typedef struct {
  /// @brief MAC address
  /// @details This field contains a MAC address (EUI-64) stored in canonical
  ///          format where the first byte of the array is the most-significant
  ///          byte of the MAC address.
  uint8_t address[SL_WISUN_MAC_ADDRESS_SIZE];
} SL_ATTRIBUTE_PACKED sl_wisun_mac_address_t;
SL_PACK_END()

/// Channel mask
SL_PACK_START(1)
typedef struct {
  /// @brief Bit mask of channels
  /// @details This field specifies a bit mask of channels, one bit per
  ///          channel. First byte of the array represents channel numbers
  ///          0 - 7, with bit 0 being channel 0. Second byte represents
  ///          channel numbers 8 - 15 and so forth.
  uint8_t mask[SL_WISUN_CHANNEL_MASK_SIZE];
} SL_ATTRIBUTE_PACKED sl_wisun_channel_mask_t;
SL_PACK_END()

/// Enumerations for socket protocol
/// Deprecated since v2.0
/// MUST NOT be used with POSIX socket API
typedef enum {
  /// User Datagram Protocol (UDP)
  SL_WISUN_SOCKET_PROTOCOL_UDP  = 0,
  /// Transmission Control Protocol (TCP)
  SL_WISUN_SOCKET_PROTOCOL_TCP  = 1,
  /// Internet Control Message Protocol (ICMP)
  SL_WISUN_SOCKET_PROTOCOL_ICMP = 2
} sl_wisun_socket_protocol_t;

/// Enumerations for socket option
/// Deprecated since v2.0
/// MUST NOT be used with POSIX socket API
typedef enum {
  /// Option for socket event mode
  SL_WISUN_SOCKET_OPTION_EVENT_MODE = 0,
  /// Option for multicast group
  SL_WISUN_SOCKET_OPTION_MULTICAST_GROUP = 1,
  /// Option for send buffer limit
  SL_WISUN_SOCKET_OPTION_SEND_BUFFER_LIMIT = 2,
  /// Option to enable/disable Extended Directed Frame Exchange mode
  SL_WISUN_SOCKET_OPTION_EDFE_MODE = 3,
  /// Option to set socket unicast hop limit
  SL_WISUN_SOCKET_OPTION_UNICAST_HOP_LIMIT = 4,
  /// Option to set socket multicast hop limit
  SL_WISUN_SOCKET_OPTION_MULTICAST_HOP_LIMIT = 5
} sl_wisun_socket_option_t;

/// Socket option for event mode
/// Deprecated since v2.0
/// MUST NOT be used with POSIX socket API
typedef struct {
  /// Socket event mode
  uint32_t mode;
} sl_wisun_socket_option_event_mode_t;

/// IPv6 address structure
typedef in6_addr_t sl_wisun_ip_address_t;

/// Socket option for multicast group
/// Deprecated since v2.0
/// MUST NOT be used with POSIX socket API
typedef struct {
  /// Multicast group action
  uint32_t action;
  /// Multicast group address
  sl_wisun_ip_address_t address;
} sl_wisun_socket_option_multicast_group_t;

/// Socket option for send buffer limit
/// Deprecated since v2.0
/// MUST NOT be used with POSIX socket API
typedef struct {
  /// Send buffer limit
  uint32_t limit;
} sl_wisun_socket_option_send_buffer_limit_t;

/// Socket option for EDFE mode
/// Deprecated since v2.0
/// MUST NOT be used with POSIX socket API
typedef struct {
  /// Socket EDFE mode (1 to enable, 0 to disable)
  uint32_t mode;
} sl_wisun_socket_option_edfe_mode_t;

/// Socket option for socket unicast hop limit
/// Deprecated since v2.0
/// MUST NOT be used with POSIX socket API
typedef struct {
  /// Socket unicast hop limit (0 to 255 hops, -1 to use default)
  int16_t hop_limit;
  /// Reserved, set to 0
  uint16_t reserved;
} sl_wisun_socket_option_unicast_hop_limit;

/// Socket option for socket multicast hop limit
/// Deprecated since v2.0
/// MUST NOT be used with POSIX socket API
typedef struct {
  /// Socket multicast hop limit (0 to 255 hops, -1 to use default)
  int16_t hop_limit;
  /// Reserved, set to 0
  uint16_t reserved;
} sl_wisun_socket_option_multicast_hop_limit;

/// Socket options
/// Deprecated since v2.0
/// MUST NOT be used with POSIX socket API
SL_PACK_START(1)
typedef union {
  /// Socket event mode
  /// Deprecated
  sl_wisun_socket_option_event_mode_t event_mode;
  /// Socket multicast group
  /// Deprecated
  sl_wisun_socket_option_multicast_group_t multicast_group;
  /// Socket send buffer limit
  /// Deprecated
  sl_wisun_socket_option_send_buffer_limit_t send_buffer_limit;
  /// Socket EDFE mode
  /// Deprecated
  sl_wisun_socket_option_edfe_mode_t edfe_mode;
  /// Socket unicast hop limit
  /// Deprecated
  sl_wisun_socket_option_unicast_hop_limit unicast_hop_limit;
  /// Socket multicast hop limit
  /// Deprecated
  sl_wisun_socket_option_multicast_hop_limit multicast_hop_limit;
  /// Option-specific value
  int32_t value;
  /// IPv6 address
  in6_addr_t ipv6_address;
} SL_ATTRIBUTE_PACKED sl_wisun_socket_option_data_t;
SL_PACK_END()

/// Enumeration for RPL neighbor types
typedef enum {
  /// Primary parent
  SL_WISUN_NEIGHBOR_TYPE_PRIMARY_PARENT     = 0,
  /// Secondary parent
  SL_WISUN_NEIGHBOR_TYPE_SECONDARY_PARENT   = 1,
  /// Child
  SL_WISUN_NEIGHBOR_TYPE_CHILD              = 2
} sl_wisun_neighbor_type_t;

/// RPL neighbor information
SL_PACK_START(1)
typedef struct {
  /// Link-local address
  in6_addr_t link_local_address;
  /// ULA/GUA address (unspecified address :: if unknown)
  in6_addr_t global_address;
  /// Neighbor type (#sl_wisun_neighbor_type_t)
  uint32_t type;
  /// Remaining lifetime (Link lifetime for parents, EARO lifetime for children) in seconds
  uint32_t lifetime;
  /// MAC TX packet count
  uint32_t mac_tx_count;
  /// MAC TX failed count
  uint32_t mac_tx_failed_count;
  /// MAC TX packet count using mode switch
  uint32_t mac_tx_ms_count;
  /// MAC TX failed count using mode switch
  uint32_t mac_tx_ms_failed_count;
  /// MAC TX packet count using concurrent detection
  uint32_t mac_tx_cd_count;
  /// MAC RX packet count
  uint32_t mac_rx_count;
  /// RPL Rank value for parents (0xffff if unknown or child)
  uint16_t rpl_rank;
  /// Measured ETX value if known (0xffff if unknown)
  uint16_t etx;
  /// ETX to Border Router
  uint16_t routing_cost;
  /// Number devices connected to Border Router
  uint16_t pan_size;
  /// Parent RSSI Out measured RSSI value (0xff if unknown)
  /// Calculated using EWMA specified by Wi-SUN from range of -174 (0) to +80 (254) dBm.
  uint8_t rsl_out;
  /// Parent RSSI In measured RSSI value (0xff if unknown)
  /// Calculated using EWMA specified by Wi-SUN from range of -174 (0) to +80 (254) dBm.
  uint8_t rsl_in;
  /// RSSI of the last received packet in integer dBm. */
  int8_t rssi;
  /// Indicate if the device is an LFN. 1 = LFN, 0 = FFN
  uint8_t is_lfn;
  /// Number of PhyModeId supported
  uint8_t phy_mode_id_count;
  /// List of phy_mode_id_count PhyModeId
  uint8_t phy_mode_ids[SL_WISUN_MAX_PHY_MODE_ID_COUNT];
  /// Indicate if the neighbor supports MAC mode switch
  uint8_t is_mdr_command_capable;
} SL_ATTRIBUTE_PACKED sl_wisun_neighbor_info_t;
SL_PACK_END()

/// Adaptive rate algorithm rate stats
SL_PACK_START(1)
typedef struct {
  /// Frame tx time
  uint32_t frame_duration_us;
  /// Number of times the rate was selected
  uint32_t sample_count;
  /// Estimated chance of success
  float success_probability;
  /// Estimated throughput in kb/s
  float throughput;
  /// Number of successful transmissions
  uint16_t success_count;
  /// Number of tx without acknowledgement
  uint16_t fail_count;
  /// Number of times this rate was skipped
  uint8_t skipped_sample_count;
  /// Indicates if the rate can be probed
  int8_t probe_limit;
  /// Reserved, set to zero
  uint8_t reserved[2];
} SL_ATTRIBUTE_PACKED sl_wisun_rate_stats_t;
SL_PACK_END()

/// Adaptive rate algorithm rates
SL_PACK_START(1)
typedef struct {
  /// Struct containing the rate's tx statistics
  sl_wisun_rate_stats_t stats;
  /// Rate in kb/s
  uint32_t rate;
  /// Rate index on the rate list
  uint8_t index;
  /// Rate phy mode id
  uint8_t phy_mode_id;
  /// Maximum amount of retries with current rate
  uint8_t max_retries;
  /// Indicates if rate is supported by both the node and the neighbor
  uint8_t is_supported;
} SL_ATTRIBUTE_PACKED sl_wisun_rate_t;
SL_PACK_END()

/// Enumeration for trace group
typedef enum {
  SL_WISUN_TRACE_GROUP_MAC     = 0,     ///< Mac
  SL_WISUN_TRACE_GROUP_NW      = 1,     ///< Network
  SL_WISUN_TRACE_GROUP_LLC     = 2,     ///< LLC
  SL_WISUN_TRACE_GROUP_6LO     = 3,     ///< 6lowpan
  SL_WISUN_TRACE_GROUP_IPV6    = 4,     ///< IPV6
  SL_WISUN_TRACE_GROUP_TCP     = 5,     ///< TCP
  SL_WISUN_TRACE_GROUP_UDP     = 6,     ///< UDP
  SL_WISUN_TRACE_GROUP_ICMP    = 7,     ///< ICMP
  SL_WISUN_TRACE_GROUP_DHCP    = 8,     ///< DHCP
  SL_WISUN_TRACE_GROUP_MPL     = 9,     ///< MPL
  SL_WISUN_TRACE_GROUP_DNS     = 10,    ///< DNS
  SL_WISUN_TRACE_GROUP_RPL     = 11,    ///< RPL
  SL_WISUN_TRACE_GROUP_TRIC    = 12,    ///< Trickle
  SL_WISUN_TRACE_GROUP_WS      = 15,    ///< Wi-SUN Stack
  SL_WISUN_TRACE_GROUP_BOOT    = 16,    ///< Wi-SUN Bootstrap
  SL_WISUN_TRACE_GROUP_WSR     = 17,    ///< Wi-SUN Router
  SL_WISUN_TRACE_GROUP_WSBR    = 18,    ///< Border router
  SL_WISUN_TRACE_GROUP_SEC     = 19,    ///< Security
  SL_WISUN_TRACE_GROUP_TIME    = 20,    ///< Time and timers
  SL_WISUN_TRACE_GROUP_NEIGH   = 21,    ///< Neighbor
  SL_WISUN_TRACE_GROUP_STAT    = 22,    ///< Statistics
  SL_WISUN_TRACE_GROUP_BUFF    = 23,    ///< Dynamic Buffer
  SL_WISUN_TRACE_GROUP_ADDR    = 24,    ///< Address Manipulation
  SL_WISUN_TRACE_GROUP_MON     = 25,    ///< Monitoring
  SL_WISUN_TRACE_GROUP_SOCK    = 26,    ///< Socket
  SL_WISUN_TRACE_GROUP_DENY    = 27,    ///< Deny list
  SL_WISUN_TRACE_GROUP_ETX     = 28,    ///< ETX
  SL_WISUN_TRACE_GROUP_FHSS    = 29,    ///< FHSS
  SL_WISUN_TRACE_GROUP_ROUT    = 30,    ///< Routing table
  SL_WISUN_TRACE_GROUP_EVLP    = 31,    ///< Event loop
  SL_WISUN_TRACE_GROUP_NVM     = 32,    ///< NVM
  SL_WISUN_TRACE_GROUP_CRYPTO  = 33,    ///< Crypto
  SL_WISUN_TRACE_GROUP_RF      = 34,    ///< Wi-SUN RF Driver
  Sl_WISUN_TRACE_GROUP_WSIE    = 35,    ///< Wi-SUN IE
  SL_WISUN_TRACE_GROUP_CONFIG  = 36,    ///< Configuration
  SL_WISUN_TRACE_GROUP_TIM_SRV = 37,    ///< Timer service
  SL_WISUN_TRACE_GROUP_LFN_TIM = 38,    ///< LFN timing measurement
  SL_WISUN_TRACE_GROUP_RALG    = 39,    ///< Adaptive rate algorithms
  SL_WISUN_TRACE_GROUP_FSM     = 40,    ///< Finite state machine. Kept for backward compatibility.
  SL_WISUN_TRACE_GROUP_WS_FSM  = 40,    ///< Wi-SUN Finite state machine
  SL_WISUN_TRACE_GROUP_APP     = 41,    ///< Application
  SL_WISUN_TRACE_GROUP_DC      = 42,    ///< Direct Connect
  SL_WISUN_TRACE_GROUP_REG     = 43,    ///< Regional regulation
  SL_WISUN_TRACE_GROUP_TXALG   = 44,    ///< RFC 8415 TX algorithm
  SL_WISUN_TRACE_GROUP_MAC_FSM = 45,    ///< MAC Finite state machine

  // [...] reserved for future use
  SL_WISUN_TRACE_GROUP_INT     = 63,    ///< Internal usage
  SL_WISUN_TRACE_GROUP_COUNT   = 64     ///< Max number of trace group in this enum
} sl_wisun_trace_group_t;

/// Thread identifier "Wi-SUN Task"
#define SL_WISUN_TRACE_THREAD_WISUN         "WS"
/// Thread identifier "Wi-SUN Event Task"
#define SL_WISUN_TRACE_THREAD_EVENT_TASK    "EVT"
/// Thread identifier "Wi-SUN Event Loop Task"
#define SL_WISUN_TRACE_THREAD_EVENT_LOOP    "EVL"
/// Thread identifier "Wi-SUN RF Task"
#define SL_WISUN_TRACE_THREAD_MAC           "MAC"

/// Enumerations for trace level
typedef enum {
  /// No trace
  SL_WISUN_TRACE_LEVEL_NONE  = 0,
  /// Error only
  SL_WISUN_TRACE_LEVEL_ERROR = 1,
  /// Warning + error
  SL_WISUN_TRACE_LEVEL_WARN  = 2,
  /// Info + warning + error
  SL_WISUN_TRACE_LEVEL_INFO  = 3,
  /// Debug + info + warning + error
  SL_WISUN_TRACE_LEVEL_DEBUG = 4,
} sl_wisun_trace_level_t;

/// Configure the trace level of 1 group
typedef struct {
  /// Trace Group ID. Coded with enum sl_wisun_trace_group_t.
  uint8_t group_id;
  /// Maximum trace level to display for this group.
  /// It is coded using enum sl_wisun_trace_level_t.
  uint8_t trace_level;
} sl_wisun_trace_group_config_t;

/// Enumerations for regional regulation
typedef enum {
  /// No regulation
  SL_WISUN_REGULATION_NONE = 0,
  /// ARIB, should only be used with JP regulatory domain
  SL_WISUN_REGULATION_ARIB = 1,
  /// WPC, should only be used with IN regulatory domain
  SL_WISUN_REGULATION_WPC = 2,
  /// ETSI, should only be used with EU regulatory domain
  SL_WISUN_REGULATION_ETSI = 3,
} sl_wisun_regulation_t;

/// Enumeration for Mode Switch mode
typedef enum {
  /// Mode switch is not allowed
  SL_WISUN_MODE_SWITCH_DISABLED     = 0,
  /// Mode switch is allowed for all unicast data frames. Specified PhyModeId is used.
  SL_WISUN_MODE_SWITCH_ENABLED      = 1,
  /// Mode switch is allowed for all unicast data frames. Default PhyModeId is used.
  SL_WISUN_MODE_SWITCH_DEFAULT      = 2
} sl_wisun_mode_switch_mode_t;

/// Enumeration for regional regulation transmission level.
/// Thresholds are define with #sl_wisun_set_regulation_tx_thresholds.
typedef enum {
  /// Transmission duration is compliant with regional regulation
  SL_WISUN_REGULATION_TX_LEVEL_LOW = 0,
  /// Transmission duration is above warning threshold
  SL_WISUN_REGULATION_TX_LEVEL_WARNING = 1,
  /// Transmission duration is above alert threshold
  SL_WISUN_REGULATION_TX_LEVEL_ALERT = 2
} sl_wisun_regulation_tx_level_t;

/// Broadcast MAC address
extern const sl_wisun_mac_address_t sl_wisun_broadcast_mac;

/// Enumeration for channel exclusion modes.
typedef enum {
  /// Channels are excluded by range if possible (3 ranges maximum),
  /// otherwise channels will be excluded by mask
  SL_WISUN_CHANNEL_EXCLUSION_MODE_BY_RANGE = 1,
  /// Channels are excluded by mask
  SL_WISUN_CHANNEL_EXCLUSION_MODE_BY_MASK = 2
} sl_wisun_channel_exclusion_mode_t;

/// Enumeration for FAN versions to which the node is certified.
typedef enum {
  /// Keep node's default FAN TPS version
  SL_WISUN_FAN_VERSION_DEFAULT = 0,
  /// FAN 1.0 certified node
  SL_WISUN_FAN_VERSION_1_0 = 1,
  /// FAN 1.1 certified node
  SL_WISUN_FAN_VERSION_1_1 = 2
} sl_wisun_fan_version_t;

/// Enumeration for types of frame that can be triggered.
typedef enum {
  /// PAN Advertisement Solicit
  SL_WISUN_FRAME_TYPE_PAS = 0,
  /// PAN Advertisement
  SL_WISUN_FRAME_TYPE_PA = 1,
  /// PAN Configuration Solicit
  SL_WISUN_FRAME_TYPE_PCS = 2,
  /// PAN Configuration
  SL_WISUN_FRAME_TYPE_PC = 3,
  /// DODAG Information Solicitation
  SL_WISUN_FRAME_TYPE_DIS = 4,
  /// DODAG Information Object
  SL_WISUN_FRAME_TYPE_DIO = 5,
} sl_wisun_frame_type_t;

/// Wi-SUN network information
SL_PACK_START(1)
typedef struct {
  /// PAN ID
  uint16_t pan_id;
  /// Hop count
  uint8_t hop_count;
  /// Reserved, set to zero
  uint8_t reserved;
  } SL_ATTRIBUTE_PACKED sl_wisun_network_info_t;
SL_PACK_END()

/// RPL information
SL_PACK_START(1)
typedef struct {
  /// DODAG rank or the node
  uint16_t dodag_rank;
  /// DAG max rank increase, the allowable increase in Rank
  /// in support of local repair (0 to disable the mechanism)
  uint16_t dag_max_rank_increase;
  /// Min hop rank increase, minimum increase in Rank
  /// between a node and any of its DODAG parents
  uint16_t min_hop_rank_increase;
  /// Lifetime unit, unit in seconds that is used to express
  /// route lifetimes in RPL
  uint16_t lifetime_unit;
  /// Instance ID, set by the DODAG root, it indicates of which RPL Instance
  /// the DODAG is a part
  uint8_t instance_id;
  /// DODAG version number, set by the DODAG root
  uint8_t dodag_version_number;
  /// Grounded, indicates whether the DODAG advertised can satisfy
  /// the application-defined goal.  If set, the DODAG is grounded.
  /// If cleared, the DODAG is floating.
  uint8_t grounded;
  /// Mode of Operation (MOP), must be 1 for Non-Storing Mode of Operation
  uint8_t mode_of_operation;
  /// DODAG Preference, defines how preferable the root of this DODAG
  /// is compared to other DODAG roots within the instance.
  /// DAGPreference ranges from 0x00 (least preferred) to 0x07
  /// (most preferred).
  uint8_t dodag_preference;
  /// Destination Advertisement Trigger Sequence Number (DTSN)
  uint8_t dodag_dtsn;
  /// DIO minimum interval, used to configure Imin of the
  /// DIO Trickle timer
  uint8_t dio_interval_min;
  /// DIO interval doublings, used to configure Imax of the
  /// DIO Trickle timer
  uint8_t dio_interval_doublings;
  /// DIO redundancy constant, used to configure k of the
  /// DIO Trickle timer
  uint8_t dio_redundancy_constant;
  /// Default lifetime, lifetime that is used as default for
  /// all RPL routes. Expressed in units of Lifetime Units.
  uint8_t default_lifetime;
  /// Reserved, set to zero
  uint8_t reserved[2];
  } SL_ATTRIBUTE_PACKED sl_wisun_rpl_info_t;
SL_PACK_END()

/// RFC8415 TX algorithm parameter set
SL_PACK_START(1)
typedef struct {
  /// Randomization factor magnitude [RFC8415] for retransmission.
  /// Randomization factor is uniformly distributed between -rand and +rand
  float rand;
  /// Initial transmission maximum delay [RFC8415] (seconds).
  uint16_t max_delay_s;
  /// Initial retransmission time [RFC8415] (seconds).
  uint16_t irt_s;
  /// Maximum retransmission time [RFC8415] (seconds).
  uint16_t mrt_s;
  /// Maximum retransmission duration [RFC8415] (seconds).
  uint16_t mrd_s;
  /// Maximum retransmission count [RFC8415].
  uint8_t mrc;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_rfc8415_txalg_params_t;

/// Enumeration for channel mask types
typedef enum {
  /// Regulatory excluded channel mask (not advertised)
  SL_WISUN_CHANNEL_MASK_TYPE_REGULATORY,
  /// Regional excluded channel mask (not advertised)
  SL_WISUN_CHANNEL_MASK_TYPE_REGIONAL = SL_WISUN_CHANNEL_MASK_TYPE_REGULATORY,
  /// Excluded channel mask advertised in us-ie
  SL_WISUN_CHANNEL_MASK_TYPE_ADVERTISED_UNICAST,
  /// Excluded channel mask advertised in bs-ie
  SL_WISUN_CHANNEL_MASK_TYPE_ADVERTISED_BROADCAST,
  /// Excluded channel mask applied to async frames
  SL_WISUN_CHANNEL_MASK_TYPE_EFFECTIVE_ASYNC,
  /// Excluded channel mask applied to unicast frequency hopping
  SL_WISUN_CHANNEL_MASK_TYPE_EFFECTIVE_UNICAST,
  /// Excluded channel mask applied to broadcast frequency hopping
  SL_WISUN_CHANNEL_MASK_TYPE_EFFECTIVE_BROADCAST
} sl_wisun_channel_mask_type_t;

/// Enumeration for Direct Connect Link status
typedef enum {
  /// Direct Connect Link connected
  SL_WISUN_DIRECT_CONNECT_LINK_STATUS_CONNECTED,
  /// Direct Connect Link error
  SL_WISUN_DIRECT_CONNECT_LINK_STATUS_ERROR,
  /// Direct Connect Link disconnected
  SL_WISUN_DIRECT_CONNECT_LINK_STATUS_DISCONNECTED,
} sl_direct_connect_link_status_t;

/// Border Router state
typedef enum {
  /// Border Router is initialized
  SL_WISUN_BR_STATE_INITIALIZED = 0,
  /// Border Router is started
  SL_WISUN_BR_STATE_OPERATIONAL = 1
} sl_wisun_br_state_t;

/// Border Router routing table entry
SL_PACK_START(1)
typedef struct {
  /// GUA/ULA of the routed node
  in6_addr_t target;
  /// GUA/ULA of the routed node's preferred parent
  in6_addr_t preferred;
  /// GUA/ULA of the routed node's backup parent. INADDR_ANY if not set.
  in6_addr_t backup;
} SL_ATTRIBUTE_PACKED sl_wisun_br_routing_table_entry_t;
SL_PACK_END()

/// Enumeration for routing table update types
typedef enum {
  /// At least one route is updated in the routing table
  SL_WISUN_ROUTING_TABLE_UPDATE_ROUTE_CHANGED = 0,
} sl_wisun_routing_table_update_event_t;

/// MAC parameter set
SL_PACK_START(1)
typedef struct {
  /// Length of one backoff period in microseconds. If 0, the length will be calculated based on the PHY.
  /// The default value is 0.
  uint16_t backoff_period_us;
  /// Minimum value of CSMA-CA backoff exponent. The default value is 3.
  uint8_t min_be;
  /// Maximum value of CSMA-CA backoff exponent. The default value is 5.
  uint8_t max_be;
  /// Maximum number of CCA retries. The transmission is aborted if the channel is still busy
  /// after 1 + max_cca_retries attempts. The default value is 8.
  uint8_t max_cca_retries;
  /// Maximum number of transmission retries. The transmission is aborted if no acknowledgment
  /// has been received after 1 + max_frame_retries attempts. The default value is 7.
  uint8_t max_frame_retries;
  /// Reserved, set to zero
  uint8_t reserved[2];
} SL_ATTRIBUTE_PACKED sl_wisun_mac_params_t;
SL_PACK_END()

/// Enumeration for event types
typedef enum {
  SL_WISUN_LOGGER_EVENT_TYPE_NONE = 0,
  /// Event published when a neighbor's lifetime changes
  SL_WISUN_LOGGER_EVENT_TYPE_NEIGHBOR_LIFETIME_CHANGED = 1,
  /// Event published when a frame is received
  SL_WISUN_LOGGER_EVENT_TYPE_FRAME_RECEIVED = 2,
  /// Event published when a frame is dropped due to frame counter failure
  SL_WISUN_LOGGER_EVENT_TYPE_FRAME_COUNTER_FAILURE = 4,
  /// Event published when a tx fails
  SL_WISUN_LOGGER_EVENT_TYPE_TX_FAILURE = 8,
  /// Event published when RF test receives a packet or completes a TX
  SL_WISUN_LOGGER_EVENT_TYPE_RF_TEST = 16,
} sl_wisun_logger_event_type_t;

/// Enumeration for event log frame types
typedef enum {
  /// PAN Advertisement Solicit frame
  SL_WISUN_LOGGER_EVENT_FRAME_TYPE_PAS = 0,
  /// PAN Advertisement frame
  SL_WISUN_LOGGER_EVENT_FRAME_TYPE_PA = 1,
  /// LFN PAN Advertisement Solicit frame
  SL_WISUN_LOGGER_EVENT_FRAME_TYPE_LPAS = 2,
  /// LFN PAN Advertisement frame
  SL_WISUN_LOGGER_EVENT_FRAME_TYPE_LPA = 3,
  /// EAPOL frame
  SL_WISUN_LOGGER_EVENT_FRAME_TYPE_EAPOL = 4,
  /// PAN Configuration Solicit frame
  SL_WISUN_LOGGER_EVENT_FRAME_TYPE_PCS = 5,
  /// PAN Configuration frame
  SL_WISUN_LOGGER_EVENT_FRAME_TYPE_PC = 6,
  /// LFN PAN Configuration Solicit frame
  SL_WISUN_LOGGER_EVENT_FRAME_TYPE_LPCS = 7,
  /// LFN PAN Configuration frame
  SL_WISUN_LOGGER_EVENT_FRAME_TYPE_LPC = 8,
  /// LFN Time Sync
  SL_WISUN_LOGGER_EVENT_FRAME_TYPE_LTS = 9,
  /// Data frame
  SL_WISUN_LOGGER_EVENT_FRAME_TYPE_DATA = 10,
  /// Undefined frame type
  SL_WISUN_LOGGER_EVENT_FRAME_TYPE_UNDEF = 255,
} sl_wisun_logger_event_frame_type_t;

/// Neighbor lifetime change information
SL_PACK_START(1)
typedef struct {
  /// Lifetime of the neighbor in seconds after update
  uint32_t lifetime;
} SL_ATTRIBUTE_PACKED sl_wisun_logger_event_neighbor_lifetime_changed_t;
SL_PACK_END()

/// Wisun Pan Advertisement information
SL_PACK_START(1)
typedef struct {
  /// PAN ID
  uint16_t pan_id;
  /// Routing cost
  uint16_t routing_cost;
  /// PAN size
  uint16_t pan_size;
  /// Load factor
  uint8_t load_factor;
} SL_ATTRIBUTE_PACKED sl_wisun_logger_event_pa_data_t;
SL_PACK_END()

/// Frame received information
SL_PACK_START(1)
typedef struct {
  /// Frame type
  uint32_t type;
  /// RSSI of the received frame
  int8_t rssi;
  /// Data of the received frame
  union {
    /// PAN Advertisement information
    sl_wisun_logger_event_pa_data_t pa;
  } data;
} SL_ATTRIBUTE_PACKED sl_wisun_logger_event_frame_received_t;
SL_PACK_END()

/// TX failure information
SL_PACK_START(1)
typedef struct {
  /// Frame type
  uint32_t type;
} SL_ATTRIBUTE_PACKED sl_wisun_logger_event_tx_failure_t;
SL_PACK_END()

/// Value for rssi when not applicable (e.g. TX or non-RX events)
#define SL_WISUN_RF_TEST_RSSI_NOT_AVAILABLE  (-128)

/// RF test RX event information (valid when @ref SL_RAIL_EVENT_RX_PACKET_RECEIVED is set)
SL_PACK_START(1)
typedef struct {
  /// RSSI in dBm; @ref SL_WISUN_RF_TEST_RSSI_NOT_AVAILABLE when not applicable
  int8_t rssi;
  /// Reserved for future use
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_logger_event_rf_test_rx_t;
SL_PACK_END()

/// RF test event information
SL_PACK_START(1)
typedef struct {
  /// RAIL events associated with the RF test event
  uint64_t events;
  /// Event-specific data
  union {
    /// RX packet received information (when @ref SL_RAIL_EVENT_RX_PACKET_RECEIVED is set in events)
    sl_wisun_logger_event_rf_test_rx_t rx;
  } u;
} SL_ATTRIBUTE_PACKED sl_wisun_logger_event_rf_test_t;
SL_PACK_END()

/// Wisun Event information
SL_PACK_START(1)
typedef struct {
  /// Event type
  uint64_t type;
  /// Address of the node
  sl_wisun_mac_address_t address;
  /// Event specific data
  union {
    /// Neighbor lifetime change information
    sl_wisun_logger_event_neighbor_lifetime_changed_t neighbor_lifetime_changed;
    /// Frame received information
    sl_wisun_logger_event_frame_received_t frame_received;
    /// TX failure information
    sl_wisun_logger_event_tx_failure_t tx_failure;
    /// RF test event information
    sl_wisun_logger_event_rf_test_t rf_test;
  } u;
} SL_ATTRIBUTE_PACKED sl_wisun_logger_event_t;
SL_PACK_END()

/**************************************************************************//**
 * Handler called for an IPv6 packet from Wi-SUN network.
 *
 * @param[in] data Pointer to IPv6 header
 * @param[out] data_length Length of IPv6 packet data in bytes
 * @return SL_STATUS_OK if successful, an error code otherwise
 *****************************************************************************/
typedef sl_status_t (*sl_wisun_ipv6_up_handler_t)(const uint8_t* data, size_t data_length);

/** @} (end SL_WISUN_TYPES) */

#endif  // SL_WISUN_TYPES_H
