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

#include <stdint.h>
#include "sl_ieee802_types.h"
#include "sl_ip_types.h"
#include "sl_wifi_constants.h"
#include "sl_wifi_types.h"
#include "sl_si91x_constants.h"
#include "sl_common.h"

//! @cond Doxygen_Suppress
// below defines and structure for CFG_GET: Getting user store configuration.
#define SLI_IP_ADDRESS_SZ            4
#define SLI_SSID_LEN                 34
#define SLI_WISE_PMK_LEN             32
#define SLI_MAX_HTTP_SERVER_USERNAME 31
#define SLI_MAX_HTTP_SERVER_PASSWORD 31
#define SLI_PSK_LEN                  64
#define SLI_MAC_ADDR_LEN             6

// Maximum length of the domain name for TLS certificate verification
#define SLI_SI91X_MAX_DOMAIN_NAME_LENGTH 256

// Maximum Access points that can be scanned
#define SLI_AP_SCANNED_MAX 11

// Quick reference: Bit 3 -> DHCPv6 client, Bit 5 -> DHCPv6 server, Bit 17 -> TCP_IP_FEAT_IPV6
// Details: https://docs.silabs.com/rs9116-wiseconnect/latest/wifibt-wc-sapi-reference/opermode#rsi-tcp-ip-feature-bit-map
#define SLI_SI91X_IPV6_MODE (1 << 3) | (1 << 5) | (1 << 17)
#define SLI_IP_ADDRESS_LEN  4
// Maximum payload size
#define SLI_MAX_PAYLOAD_SIZE 1500

#define SLI_MAX_FWUP_CHUNK_SIZE 1024
#define SLI_RPS_HEADER_SIZE     64
//! @endcond

/** \addtogroup SL_FWUP_PKT_DEFINE
  * \ingroup SL_SI91X_CONSTANTS
  * @{ */
/*=========================================================================*/
// Firmware upgrade packet types !//
/*=========================================================================*/
/// Indicates an abort command for the firmware update process
#define SL_FWUP_ABORT 2

/// Represents the header portion of an RPS
#define SL_FWUP_RPS_HEADER 1

/// Refers to the actual content or payload of the RPS
#define SL_FWUP_RPS_CONTENT 0
/** @} */

//! @cond Doxygen_Suppress
// Websocket max url length
#define SLI_WEBS_MAX_URL_LENGTH 51

// Websocket max host length
#define SLI_WEBS_MAX_HOST_LENGTH 51

// Websocket max subprotocol length
#define SLI_WEBS_MAX_SUBPROTOCOL_LENGTH 51

#if defined(SLI_SI917)
#define SLI_SI91X_MAX_SIZE_OF_EXTENSION_DATA 256
#else
#define SLI_SI91X_MAX_SIZE_OF_EXTENSION_DATA 64
#endif

#define SLI_SI91X_DNS_REQUEST_MAX_URL_LEN 90

#define SLI_SI91X_DNS_RESPONSE_MAX_ENTRIES 10

#define SLI_SI91X_MAX_CERT_SEND_SIZE 1400

/** NOTE: For power save related info
 * https://docs.silabs.com/rs9116/wiseconnect/rs9116w-wifi-at-command-prm/latest/wlan-commands#rsi-pwmode----power-mode
 * ****************************** POWER RELATED DEFINES START *******************************/
#define SLI_POWER_MODE_DISABLE      0
#define SLI_CONNECTED_SLEEP_PS      1
#define SLI_CONNECTED_GPIO_BASED_PS 2
#define SLI_CONNECTED_MSG_BASED_PS  3

#ifdef SLI_SI91X_MCU_INTERFACE
#define SLI_CONNECTED_M4_BASED_PS 4
#endif

#define SLI_GPIO_BASED_DEEP_SLEEP 8
#define SLI_MSG_BASED_DEEP_SLEEP  9

#ifdef SLI_SI91X_MCU_INTERFACE
#define SLI_M4_BASED_DEEP_SLEEP 10
#endif

#define SLI_DEFAULT_DTIM_SKIP      0
#define SLI_DEFAULT_DTIM_ALIGNMENT 0

#define SLI_LP_MODE 0

/****************************** POWER RELATED DEFINES END ***********************************/

// **************************** Macros for BG scan **************************************/
#define SLI_SI91X_BG_SCAN_DISABLE 0
#define SLI_SI91X_BG_SCAN_ENABLE  1

//**************************** Macros for BG end *********************************/
#define SLI_NUMBER_OF_SOCKETS 20

//**************************** Macros for WPS Method request START *********************************/

#define SLI_SI91X_SET_WPS_METHOD_PUSH_BUTTON 0
#define SLI_SI91X_SET_WPS_VALIDATE_PIN       0
#define SLI_SI91X_WPS_PIN_LEN                8

//**************************** Macros for WPS Method request END ***********************************/
//! @endcond

/** \addtogroup SI91X_JOIN_FEATURE_BIT_MAP
  * @{ */
/*=========================================================================*/
// Join feature bit map parameters description !//
/*=========================================================================*/
/// To enable b/g only mode in station mode
#define SL_SI91X_JOIN_FEAT_STA_BG_ONLY_MODE_ENABLE SL_WIFI_JOIN_FEAT_STA_BG_ONLY_MODE_ENABLE

/// To take listen interval from join command.
#define SL_SI91X_JOIN_FEAT_LISTEN_INTERVAL_VALID SL_WIFI_JOIN_FEAT_LISTEN_INTERVAL_VALID

/// To enable quick join feature
#define SL_SI91X_JOIN_FEAT_QUICK_JOIN SL_WIFI_JOIN_FEAT_QUICK_JOIN

/// To enable CCXV2 feature
#define SL_SI91X_JOIN_FEAT_CCXV2_FEATURE SL_WIFI_JOIN_FEAT_CCXV2_FEATURE

/// To connect to AP based on BSSID together with configured SSID
#define SL_SI91X_JOIN_FEAT_BSSID_BASED SL_WIFI_JOIN_FEAT_BSSID_BASED

/// MFP Capable only
#define SL_SI91X_JOIN_FEAT_MFP_CAPABLE_ONLY SL_WIFI_JOIN_FEAT_MFP_CAPABLE_ONLY

/// MFP Capable required
#define SL_SI91X_JOIN_FEAT_MFP_CAPABLE_REQUIRED SL_WIFI_JOIN_FEAT_MFP_CAPABLE_REQUIRED

/// Listen interval from power save command
#define SL_SI91X_JOIN_FEAT_PS_CMD_LISTEN_INTERVAL_VALID SL_WIFI_JOIN_FEAT_PS_CMD_LISTEN_INTERVAL_VALID
/** @} */

//! @cond Doxygen_Suppress
//**************************** Macros for FEATURE frame Method request START *********************************/
#define SLI_FEAT_FRAME_PREAMBLE_DUTY_CYCLE       (1 << 0)
#define SLI_FEAT_FRAME_PERMIT_UNDESTINED_PACKETS (1 << 1)
#define SLI_FEAT_FRAME_LP_CHAIN                  (1 << 4)
#define SLI_FEAT_FRAME_IN_PACKET_DUTY_CYCLE      (1 << 5)

#define PLL_MODE      0
#define RF_TYPE       1 // 0 - External RF 1- Internal RF
#define WIRELESS_MODE 0
#define ENABLE_PPP    0
#define AFE_TYPE      1
#ifndef SLI_FEATURE_ENABLES
#define SLI_FEATURE_ENABLES \
  (SLI_FEAT_FRAME_PREAMBLE_DUTY_CYCLE | SLI_FEAT_FRAME_LP_CHAIN | SLI_FEAT_FRAME_IN_PACKET_DUTY_CYCLE)
#endif

//**************************** Macros for FEATURE frame Method request END *********************************/

//**************************** Macros for HTTP Client START *********************************/

#define SLI_SI91X_HTTP_BUFFER_LEN     2400
#define SLI_SI91X_MAX_HTTP_URL_SIZE   2048
#define SLI_SI91X_MAX_HTTP_CHUNK_SIZE 900
#define SLI_HTTP_GET_FIRST_PKT        BIT(0)
#define SLI_HTTP_GET_MIDDLE_PKT       BIT(1)
#define SLI_HTTP_GET_LAST_PKT         BIT(2)

#define SLI_SI91X_HTTP_CLIENT_MAX_WRITE_BUFFER_LENGTH 900
#define SLI_SI91X_HTTP_CLIENT_PUT_MAX_BUFFER_LENGTH   900
#define SLI_SI91X_HTTP_CLIENT_POST_MAX_BUFFER_LENGTH  900

// HTTP client PUT create command
#define SLI_SI91X_HTTP_CLIENT_PUT_CREATE 1

// HTTP client PUT START command
#define SLI_SI91X_HTTP_CLIENT_PUT_START 2

// HTTP client PUT PKT command
#define SLI_SI91X_HTTP_CLIENT_PUT_PKT 3

// HTTP client PUT DELETE command
#define SLI_SI91X_HTTP_CLIENT_PUT_DELETE 4

// HTTP client PUT PKT OFFSET command
#define SLI_SI91X_HTTP_CLIENT_PUT_OFFSET_PKT 5

//***************************** Macros for HTTP Client End **********************************/

#define SLI_SI91X_COUNTRY_CODE_LENGTH  3
#define SLI_SI91X_MAX_POSSIBLE_CHANNEL 24

#define ATTESTATION 30

#define NONCE_DATA_SIZE 32

typedef enum { SLI_NONE, SLI_TKIP, SLI_CCMP } sli_si91x_encryption_mode_t;
//! @endcond

/// Scan information response structure
typedef struct {
  uint8_t rf_channel;              ///< channel number of the scanned AP
  uint8_t security_mode;           ///<  security mode of the scanned AP
  uint8_t rssi_val;                ///< rssi value of the scanned AP
  uint8_t network_type;            ///< network type of the scanned AP
  uint8_t ssid[SLI_SSID_LEN];      ///< SSID of the scanned AP
  uint8_t bssid[SLI_MAC_ADDR_LEN]; ///< BSSID of the scanned AP
  uint8_t reserved[2];             ///< Reserved
} sli_si91x_scan_info_t;

/// Scan command response structure
typedef struct {
  uint8_t scan_count[4];                               ///< number of access points scanned
  uint8_t reserved[4];                                 ///< Reserved
  sli_si91x_scan_info_t scan_info[SLI_AP_SCANNED_MAX]; ///< scanned access points information
} sli_si91x_rsp_scan_t;

/// Set certificate information structure
typedef struct {
  uint16_t total_len;          ///< total length of the certificate
  uint8_t certificate_type;    ///< type of certificate
  uint8_t more_chunks;         ///< more chunks flag
  uint16_t certificate_length; ///< length of the current segment
  uint8_t certificate_inx;     ///< index of certificate
  uint8_t key_password[127];   ///< reserved
} sli_si91x_cert_info_t;

/// Si91x specific certificate validation
typedef struct {
  uint16_t socket_id;       ///< Socket ID
  volatile uint16_t status; ///< certificate valid status
} sli_si91x_req_cert_valid_t;

/// Set certificate command request structure
typedef struct {
  sli_si91x_cert_info_t cert_info;                   ///< certificate information structure
  uint8_t certificate[SLI_SI91X_MAX_CERT_SEND_SIZE]; ///< certificate
} sli_si91x_req_set_certificate_t;

/// IPV4 ipconfig command request  structure
typedef struct {
  /// 0=Manual, 1=Use DHCP
  uint8_t dhcp_mode;

  /// IP address of this module if in manual mode
  uint8_t ipaddress[4];

  /// Netmask used if in manual mode
  uint8_t netmask[4];

  /// IP address of default gateway if in manual mode
  uint8_t gateway[4];

  /// DHCP client host name
  uint8_t hostname[31];

  /// vap id, 0 - station and 1 - AP
  uint8_t vap_id;

  /// FQDN flag
  uint8_t fqdn_flag[4];

  /// minimum interval for DHCP discover retries
  uint16_t dhcp_discover_rtr_interval_min;

  /// minimum interval for DHCP request retries
  uint16_t dhcp_request_rtr_interval_min;

  /// maximum interval for DHCP discover retries
  uint16_t dhcp_discover_rtr_interval_max;

  /// maximum interval for DHCP request retries
  uint16_t dhcp_request_rtr_interval_max;

  /// maximum number of DHCP discover retries
  uint8_t dhcp_discover_max_retries;

  /// maximum number of DHCP request retries
  uint8_t dhcp_request_max_retries;
} SL_ATTRIBUTE_PACKED sli_si91x_req_ipv4_params_t;

/// IPV4 ipconfig command response  structure
typedef struct {
  /// MAC address of this module
  uint8_t macAddr[6];

  /// Configured IP address
  uint8_t ipaddr[4];

  /// Configured netmask
  uint8_t netmask[4];

  /// Configured default gateway
  uint8_t gateway[4];
} sli_si91x_rsp_ipv4_params_t;

/// IPV6 ipconfig command request  structure
typedef struct {
  /// 0=Manual, 1=Use DHCP
  uint8_t mode[2];

  /// prefix length
  uint8_t prefixLength[2];

  /// IPV6 address of the module
  uint8_t ipaddr6[16];

  /// address of gateway
  uint8_t gateway6[16];

  /// vap id, 0 - station and 1 - AP
  uint8_t vap_id;
} sli_si91x_req_ipv6_params_t;

/// IPV6 ipconfig command response structure
typedef struct {
  /// prefix length
  uint16_t prefixLength;

  /// reserved bytes
  uint16_t reserved;

  /// Link local address
  uint32_t link_local_address[4];

  /// Global address
  uint32_t global_address[4];

  /// Gateway address
  uint32_t gateway_address[4];

  /// Mac address
  uint8_t mac_address[6];

} SL_ATTRIBUTE_PACKED sli_si91x_rsp_ipv6_params_t;

/// Structure for firmware upgradation
typedef struct {
  /// Type of the packet
  uint16_t type;

  /// Length of the packet
  uint16_t length;

  /// RPS content
  uint8_t content[SLI_MAX_FWUP_CHUNK_SIZE];
} sli_si91x_req_fwup_t;

/** \addtogroup SL_SI91X_TYPES
 * @{
 * */
/// Si91x specific module RTC time
typedef struct {
  uint32_t tm_sec;  ///< Seconds [0-59]
  uint32_t tm_min;  ///< Minutes [0-59]
  uint32_t tm_hour; ///< Hours since midnight [0-23]
  uint32_t tm_mday; ///< Day of the month [1-31]
  uint32_t tm_mon;  ///< Months since January [0-11]
  uint32_t tm_year; ///< Years since 1990
  uint32_t tm_wday; ///< Weekday from Sunday to Saturday [1-7]
} sl_si91x_module_rtc_time_t;
/** @} */

///< socket create command request structure
#pragma pack(1)
typedef struct {
  uint16_t ip_version;  ///< ip version4 or 6
  uint16_t socket_type; ///< 0= TCP Client, 1= UDP Client, 2= TCP Server (Listening TCP)
  uint16_t local_port;  ///< Our local module port number
  uint16_t remote_port; ///< Port number of what we are connecting to
  union {
    uint8_t ipv4_address[4];  ///<  remote IPv4 Address
    uint8_t ipv6_address[16]; ///<  remote IPv6 Address
  } dest_ip_addr;             ///< Destination IP address
  uint16_t max_count;         ///< maximum no of LTCP sockets on same port
#if defined(SLI_SI917)
  uint16_t tos; ///< type of service

#else
  uint32_t tos;        ///< type of service

#endif

#if defined(SLI_SI917)
  uint32_t ssl_bitmap; ///< ssl version select bit map

#else
  uint8_t ssl_bitmap;  ///< ssl version select bit map
  uint8_t ssl_ciphers; ///< ssl ciphers bitmap
#endif
  uint8_t webs_resource_name[SLI_WEBS_MAX_URL_LENGTH]; ///< web socket resource name
  uint8_t webs_host_name[SLI_WEBS_MAX_HOST_LENGTH];    ///< web socket host name
  uint8_t max_tcp_retries_count;                       ///< TCP retries
  uint8_t socket_bitmap;                               ///< Socket bitmap
  uint8_t rx_window_size;                              ///< RX window size
  uint16_t tcp_keepalive_initial_time;                 ///< TCP keepalive initial timeout
  uint8_t vap_id;                                      ///< VAPID
  uint8_t socket_cert_inx;                             ///< socket cert inx
  uint32_t ssl_ciphers_bitmap;                         ///< ssl ciphers bitmap
#if defined(SLI_SI917)
  uint32_t ssl_ext_ciphers_bitmap;          ///< ssl extended ciphers bitmap
  uint8_t max_retransmission_timeout_value; ///< max retransmission timeout value
#endif
  uint8_t tcp_retry_transmit_timer;                                 ///< tcp retry transmission timer
  uint16_t tcp_mss;                                                 ///< TCP MSS
  uint16_t no_of_tls_extensions;                                    ///< number of TLS extensions
  uint16_t total_extension_length;                                  ///< total extension length
  uint8_t tls_extension_data[SLI_SI91X_MAX_SIZE_OF_EXTENSION_DATA]; ///< TLS extension data
#if defined(SLI_SI917)
  uint16_t recv_buff_len; ///< receive buffer length
#endif
  uint32_t reserved;                                              ///< reserved
  uint8_t webs_subprotocol_name[SLI_WEBS_MAX_SUBPROTOCOL_LENGTH]; ///< web socket subprotocol name
  uint8_t domain_name[SLI_SI91X_MAX_DOMAIN_NAME_LENGTH];          ///< Domain name for SNI or Host header
} sli_si91x_socket_create_request_t;
#pragma pack()

/// socket create command response structure
typedef struct {
  uint8_t ip_version[2];  ///< ip version 4 or 6
  uint8_t socket_type[2]; ///< 2 bytes, type of socket created
  uint8_t socket_id[2];   ///< 2 bytes socket descriptor, like a file handle, usually 0x00
  uint8_t module_port[2]; ///< 2 bytes, Port number of our local socket
  uint8_t dst_port[2];    ///< Destination port number
  union {
    uint8_t ipv4_addr[4];  ///< 4 bytes, Our (module) IPv4 Address
    uint8_t ipv6_addr[16]; ///< 4 bytes, Our (module) IPv6 Address
  } module_ip_addr;        ///< Module IP address
  union {
    uint8_t ipv4_addr[4];  ///< 4 bytes, Our (module) IPv4 Address
    uint8_t ipv6_addr[16]; ///< 4 bytes, Our (module) IPv6 Address
  } dest_ip_addr;          ///< Destrination IP address
  uint8_t mss[2];          ///< 2 bytes, Remote peer MSS size
  uint8_t window_size[4];  ///< 4 bytes, Remote peer Window size
} sli_si91x_socket_create_response_t;
#pragma pack()

#pragma pack(1)
/// Socket close command request structure
typedef struct {
  uint16_t socket_id;   ///< 2 bytes, socket that was closed
  uint16_t port_number; ///< 4 bytes, port number
} sli_si91x_socket_close_request_t;
#pragma pack()

#pragma pack(1)
/// Socket close command response structure

typedef struct {
  uint16_t socket_id;        ///< 2 bytes, socket that was closed
  uint32_t sent_bytes_count; ///< 4 bytes, sent bytes count
  uint16_t port_number;      ///< 2 bytes, port number
} sl_si91x_socket_close_response_t;
#pragma pack()

#pragma pack(1)
/// Si91x specifc send data on socket request structure
typedef struct {
  uint16_t ip_version;  ///< ip version 4 or 6
  uint16_t socket_id;   ///< socket descriptor of the already opened socket connection
  uint32_t length;      ///< length of the data to be sent
  uint16_t data_offset; ///< Data Offset
  uint16_t dest_port;   ///< destination port
  union {
    uint8_t ipv4_address[SLI_IP_ADDRESS_LEN];     ///< 4 bytes, IPv4 Address of the remote device
    uint8_t ipv6_address[SLI_IP_ADDRESS_LEN * 4]; ///< 16 bytes, IPv6 Address of the remote device
  } dest_ip_addr;                                 ///< IP address of the remote device
  uint8_t send_buffer[];                          ///< data buffer to send
} sli_si91x_socket_send_request_t;
#pragma pack()

/// socket accept request structure
#pragma pack(1)
typedef struct {
  uint8_t socket_id;    ///< Socket ID
  uint16_t source_port; ///< Local port number
} sli_si91x_socket_accept_request_t;
#pragma pack()

/// LTCP socket establish request structure
#pragma pack(1)
typedef struct {
  /// IP version
  uint16_t ip_version;

  /// 2 bytes, socket handle
  uint16_t socket_id;

  /// 2 bytes, remote port number
  uint16_t dest_port;

  union {

    ///  remote IPv4 Address
    uint8_t ipv4_address[4];

    ///  remote IPv6 Address
    uint8_t ipv6_address[16];
  } dest_ip_addr; ///< Destination IP address

  /// 2 bytes, remote peer MSS size
  uint16_t mss;

  /// 4 bytes, remote peer Window size
  uint32_t window_size;

  /// source port number
  uint16_t src_port_num;
} sli_si91x_rsp_ltcp_est_t;
#pragma pack()

/// Internal SiWx91x Socket information query
/// @note: This is internal structure and should not be used by the applicatiom. This is identical to sl_si91x_sock_info_query_t and, would be cleaned to have single structure in future.
typedef struct {
  uint8_t sock_id[2]; ///< Identifier for the socket

  uint8_t sock_type[2]; ///< Type of the socket (TCP, UDP, and so on.)

  uint8_t source_port[2]; ///< Port number used by the source

  uint8_t dest_port[2]; ///< Port number used by the destination

  union {
    uint8_t ipv4_address[4]; ///< IPv4 address of the remote host

    uint8_t ipv6_address[16]; ///< IPv6 address of the remote host

  } dest_ip_address; ///< IP address of the destination host
} sli_sock_info_query_t;

/// Network params command response structure
#pragma pack(1)
typedef struct {
  /// uint8, 0= NOT Connected, 1= Connected
  uint8_t wlan_state;

  /// channel number of connected AP
  uint8_t channel_number;

  /// PSK
  uint8_t psk[64];

  /// Mac address
  uint8_t mac_address[6];

  /// uint8[32], SSID of connected access point
  uint8_t ssid[SLI_SSID_LEN];

  /// 2 bytes, 0= AdHoc, 1= Infrastructure
  uint8_t connType[2];

  /// security type
  uint8_t sec_type;

  /// uint8, 0= Manual IP Configuration,1= DHCP
  uint8_t dhcpMode;

  /// uint8[4], Module IP Address
  uint8_t ipv4_address[4];

  /// uint8[4], Module Subnet Mask
  uint8_t subnetMask[4];

  /// uint8[4], Gateway address for the Module
  uint8_t gateway[4];

  /// number of sockets opened
  uint8_t num_open_socks[2];

  /// prefix length for ipv6 address
  uint8_t prefix_length[2];

  /// modules ipv6 address
  uint8_t ipv6_address[16];

  /// router ipv6 address
  uint8_t defaultgw6[16];

  /// BIT(0) =1 - ipv4, BIT(1)=2 - ipv6, BIT(0) & BIT(1)=3 - BOTH
  uint8_t tcp_stack_used;

  /// sockets information array
  sli_sock_info_query_t socket_info[10];

  /// BSSID address of connected AP
  uint8_t bssid[6];

  /// Wireless mode used in connected AP (6 - AX, 4 - N, 3 - G, 1 - B)
  uint8_t wireless_mode;
} sli_si91x_network_params_response_t;
#pragma pack()

/// Si91x specific sockt send data parameters
typedef struct {
  uint8_t sock_fd; ///< Socket identifier
  uint32_t length; ///< Length of data
  // Need to discuss and increase data size
  uint8_t data[50]; ///< Data
} sli_si91x_socket_send_data_parameters_t;

/// Si91x specific sockt receive data parameters
typedef struct {
  uint8_t new_sock_fd; ///< New socket identifier
  uint32_t length;     ///< Length of data
  uint8_t data[1460];  ///< Data
} sli_si91x_socket_receive_data_parameters_t;

/// Si91x specific socket create parameters
typedef struct {
  uint8_t domain;   ///< Domain
  uint8_t type;     ///< Socket protocol types (TCP/UDP/RAW) :: 1(SOCK_STREAM), 2(SOCK_DGRAM), 3(SOCK_RAW)
  uint8_t protocol; ///< Protocol default : 0(IPPROTO_IP)
} sli_si91x_socket_create_parameters_t;

/// Socket connection parameters
typedef struct {
  uint8_t domain;          ///< Domain
  int8_t sock_fd;          ///< Socket identifier
  uint32_t port;           ///< Port number
  uint32_t remote_ip_addr; ///< Remote IP address
} sli_si91x_socket_connect_or_listen_parameters_t;

/// Si91x specific feature frame request
typedef struct {
  uint8_t
    pll_mode; ///< PLL Mode. 0 - less than 120 Mhz NWP SoC clock; 1 - greater than 120 Mhz NWP SoC clock (Mode 1 is not currently supported for coex)
  uint8_t rf_type;          ///< RF Type.
  uint8_t wireless_mode;    ///< Wireless Mode.
  uint8_t enable_ppp;       ///< Enable PPP.
  uint8_t afe_type;         ///< AFE Type.
  uint32_t feature_enables; ///< Feature Enables.
} sli_si91x_feature_frame_request;

/// DNS query request structure
typedef struct {
  //! Ip version value
  uint8_t ip_version[2];

  //! URL name
  uint8_t url_name[SLI_SI91X_DNS_REQUEST_MAX_URL_LEN];

  //! DNS servers count
  uint8_t dns_server_number[2];
} sli_si91x_dns_query_request_t;

/// DNS query response structure
typedef struct {
  //! Ip version of the DNS server
  uint8_t ip_version[2];

  //! DNS response count
  uint8_t ip_count[2];

  //! DNS address responses
  union {
    uint8_t ipv4_address[4];
    uint8_t ipv6_address[16];
  } ip_address[SLI_SI91X_DNS_RESPONSE_MAX_ENTRIES];
} sli_si91x_dns_response_t;

/**
 * @brief DNS Server add request structure.
 *
 * This structure holds the information needed to add DNS servers, supporting both IPv4 and IPv6 addresses.
 */
typedef struct {
  uint8_t ip_version[2]; ///< IP version value. The second byte is reserved for future use.
  uint8_t dns_mode[2];   ///< DNS mode to use. The second byte is reserved for future use.

  union {
    uint8_t primary_dns_ipv4[4];  ///< Primary DNS address in IPv4 format.
    uint8_t primary_dns_ipv6[16]; ///< Primary DNS address in IPv6 format.
  } sli_ip_address1;              ///< Primary DNS address.

  union {
    uint8_t secondary_dns_ipv4[4];  ///< Secondary DNS address in IPv4 format.
    uint8_t secondary_dns_ipv6[16]; ///< Secondary DNS address in IPv6 format.
  } sli_ip_address2;                ///< Secondary DNS address.
} sli_dns_server_add_request_t;

/// Structure for TCP ACK indication
typedef struct {
  /// Socket ID
  uint8_t socket_id;

  /// Length
  uint8_t length[2];

} sli_si91x_rsp_tcp_ack_t;

/// Config command request structure
typedef struct {
  /// config type
  uint16_t config_type;

  /// value to set
  uint16_t value;
} sli_si91x_config_request_t;

/// read bytes coming on socket request structure
typedef struct {
  /// socket id
  uint8_t socket_id;

  /// requested bytes
  uint8_t requested_bytes[4];

  /// Timeout for read
  uint8_t read_timeout[2];
} sli_si91x_req_socket_read_t;

/// Si91x specific time value
typedef struct {
  uint32_t tv_sec;  ///< Time in Seconds
  uint32_t tv_usec; ///< Time in microseconds
} sl_si91x_time_value;

/// The select socket array manager.  */
typedef struct {
  uint32_t fd_array[(SLI_NUMBER_OF_SOCKETS + 31) / 32]; ///< Bit map of SOCKET Descriptors.
  int32_t fd_count;                                     ///< How many are SET
} sli_si91x_fd_set_t;

/// Si91x specifc socket select request structure
typedef struct {
  uint8_t num_fd;                     ///< Number of file descriptors
  uint8_t select_id;                  ///< Select ID
  sli_si91x_fd_set_t read_fds;        ///< Read file descriptors
  sli_si91x_fd_set_t write_fds;       ///< Write file descriptors
  sl_si91x_time_value select_timeout; ///< Select timeout
  uint8_t no_timeout;                 ///< No timeout
} sli_si91x_socket_select_req_t;

/// Si91x specific socket select response structure
typedef struct {
  uint8_t select_id;                ///< Select ID
  sli_si91x_fd_set_t read_fds;      ///< Read file descriptors
  sli_si91x_fd_set_t write_fds;     ///< Write file descriptors
  uint32_t socket_terminate_bitmap; ///< Socket terminate bitmap
} sli_si91x_socket_select_rsp_t;

/// Structure for OTA firmware upgradation
typedef struct {
  /// Type of the packet
  uint8_t ip_version;

  union {
    /// 4 bytes, IPv4 Address of the server
    uint8_t ipv4_address[4];

    /// 16 bytes, IPv6 Address of the server
    uint8_t ipv6_address[16];

  } server_ip_address; ///< Server IP address

  /// server port
  uint8_t server_port[4];

  /// Chunk number
  uint8_t chunk_number[2];

  /// Timeout
  uint8_t timeout[2];

  /// TCP retry count
  uint8_t retry_count[2];

} sli_si91x_ota_firmware_update_request_t;

/// Multicast request structure
typedef struct {
  /// IP version
  uint8_t ip_version[2];

  /// command type
  uint8_t type[2];

  union {
    uint8_t ipv4_address[4];  ///< IPv4 address
    uint8_t ipv6_address[16]; ///< IPv6 address
  } multicast_address;        ///< Multicast address
} sli_si91x_req_multicast_t;

/// Ping Request Frame
typedef struct {
  /// ip version
  uint16_t ip_version;

  /// ping size
  uint16_t ping_size;

  union {
    /// ipv4 address
    uint8_t ipv4_address[4];

    /// ipv6 address
    uint8_t ipv6_address[16];

  } ping_address; ///< Ping address

  /// ping request timeout
  uint16_t timeout;
} sli_si91x_ping_request_t;

//! HTTP Get/Post request structure
typedef struct {
  //! ip version
  uint16_t ip_version;

  //! https enable
  uint16_t https_enable;

  //! port number
  uint16_t port_number;

  //! buffer
  uint8_t buffer[SLI_SI91X_HTTP_BUFFER_LEN];
} sli_si91x_http_client_request_t;

//! SNI for embedded sockets structure
typedef enum {
  SI91X_SNI_FOR_HTTPS = 1, ///< SNI target: HTTPS
  SI91X_SNI_FOR_MQTT  = 2  ///< SNI target: MQTT
} sli_si91x_sni_target_protocol_t;

/// Si91x specific SNI for embedded socket request
typedef struct si91x_sni_for_embedded_socket_request_s {
  //! offset from which hostname starts
  uint16_t offset;

  //! application protocol
  uint16_t protocol;

  //! sni extension data
  uint8_t tls_extension_data[SLI_SI91X_MAX_SIZE_OF_EXTENSION_DATA];
} sli_si91x_sni_for_embedded_socket_request_t;

//! HTTP client PUT START create structure
typedef struct {
  //! HTTP server ip version
  uint8_t ip_version;

  //! HTTPS bit map
  uint16_t https_enable;

  //! HTTP server port number
  uint32_t port_number;

  //! HTTP Content Length
  uint32_t content_length;
} SL_ATTRIBUTE_PACKED sli_si91x_http_client_put_start_t;

typedef struct {
  //! Current chunk length
  uint16_t current_length;
} SL_ATTRIBUTE_PACKED sli_si91x_http_client_put_data_request_t;

//! @cond Doxygen_Suppress
//! HTTP client PUT request structure
typedef struct {
  //! Command type
  uint8_t command_type;

  union http_client_put_s {
    //! HTTP PUT START command structure
    sli_si91x_http_client_put_start_t http_client_put_start;

    //! HTTP PUT PACKET command structure
    sli_si91x_http_client_put_data_request_t http_client_put_data_req;

  } SL_ATTRIBUTE_PACKED sli_http_client_put_struct;

  //! HTTP PUT buffer
  uint8_t http_put_buffer[SLI_SI91X_HTTP_CLIENT_PUT_MAX_BUFFER_LENGTH];
} SL_ATTRIBUTE_PACKED sl_si91x_http_client_put_request_t;
//! @endcond

//! HTTP Client POST DATA PKT request structure
typedef struct {
  //! Current http data chunk length
  uint16_t current_length;

  //! HTTP POST buffer
  uint8_t http_post_data_buffer[SLI_SI91X_HTTP_CLIENT_POST_MAX_BUFFER_LENGTH];
} sli_si91x_http_client_post_data_request_t;

//! HTTP Client PUT PKT response structure
typedef struct {
  //! Receive HTTP client PUT command type
  uint8_t command_type;

  //! End of resource content file
  uint8_t end_of_file;
} sli_si91x_http_client_put_pkt_rsp_t;

//! HTTP Client PUT pkt server response structure
typedef struct {
  uint32_t command_type; ///< Command type
  uint32_t more;         ///< More
  uint32_t offset;       ///< Offset
  uint32_t data_len;     ///< Data length
} sli_si91x_http_put_pkt_server_rsp_t;

/// Si91x specific ram dump
typedef struct {
  uint32_t address; ///< Address
  uint32_t length;  ///< Length
} sli_si91x_ram_dump_t;

#if defined(SLI_SI91X_MCU_INTERFACE) || defined(DOXYGEN)
/** \addtogroup SL_SI91X_CONSTANTS
  * @{ */
/// Managing interactions between the Trusted Application (TA) and the M4 core
typedef enum {
  SL_SI91X_TAKE_M4_64K                 = 1, ///< Allocates 64KB of memory for the M4 core
  SL_SI91X_GET_IPMU_PROGRAMMING_VALUES = 2, ///< Retrieves the programming values from the IPMU
  SL_SI91X_READ_TA_REGISTER            = 3, ///< Reads the value from a Trusted Application (TA) register
  SL_SI91X_WRITE_TA_REGISTER           = 4, ///< Writes a specified value to a TA register
  SL_SI91X_ENABLE_XTAL                 = 5, ///< Signals the NWP to configure clock switching between 1.3V and 3.3V
  SL_SI91X_WRITE_TO_COMMON_FLASH       = 6, ///< Writes data to the common flash memory
#if defined(SL_SI91X_SIDE_BAND_CRYPTO) || defined(DOXYGEN)
  SL_SI91X_ENABLE_SIDE_BAND = 7, ///< Enables side-band crypto processing if the feature is enabled.
#endif
  SL_SI91X_READ_FROM_COMMON_FLASH       = 8, ///< Reads data from the common flash memory
  SL_SI91X_SET_TIMESTAMP_MEMORY_ADDRESS = 11 ///< Sets the memory address for timestamp storage
} sl_si91x_ta_m4_commands_t;
/** @} */

///  M4 and NWP secure handshake request structure.
typedef struct {
  sl_si91x_ta_m4_commands_t
    sub_cmd; ///< sub_cmd form the  enum ta_m4_commands_e(Main command type is RSI_COMMON_REQ_TA_M4_COMMANDS)
  uint8_t input_data_size; ///< length of input_data
  uint8_t input_data
    []; ///< // Input data. In this input data first byte is reserved for enable(1) or Disable(0) sub_cmd of this structure.
} sli_si91x_ta_m4_handshake_parameters_t;
#endif

#ifdef SLI_SI91X_MCU_INTERFACE

#define MAX_CHUNK_SIZE    1400
#define FLASH_SECTOR_SIZE 4096
// TA2M4 handshake request structure.
typedef struct {
  // sub_cmd
  uint8_t sub_cmd;

  // NWP flash location
  uint32_t addr;

  // total length of input data
  uint16_t input_buffer_length;

  // erases multiples of 4kbytes
  uint8_t flash_sector_erase_enable;

  //data
  uint8_t input_data[MAX_CHUNK_SIZE];
} SL_ATTRIBUTE_PACKED sli_si91x_request_ta2m4_t;

typedef struct {
  // sub_cmd
  uint8_t sub_cmd;

  // nwp flash location
  uint32_t nwp_address;

  // total length of output data
  uint16_t output_buffer_length;

} SL_ATTRIBUTE_PACKED sli_si91x_read_flash_request_t;

typedef struct {
  // sub_cmd
  uint8_t sub_cmd;
} SL_ATTRIBUTE_PACKED sli_si91x_read_status_t;

#endif // SLI_SI91X_MCU_INTERFACE

/** \addtogroup SL_SI91X_TYPES
 * @{
 * */

/// Si91x specific TWT response
typedef sl_wifi_twt_response_t SL_DEPRECATED_API_WISECONNECT_4_0 sl_si91x_twt_response_t;

/// Si91x specific Wi-Fi TX test info
typedef sl_wifi_request_tx_test_info_t SL_DEPRECATED_API_WISECONNECT_4_0 sl_si91x_request_tx_test_info_t;

/** @} */

/// Si91x specific calibration write
typedef struct {
  /* Target
 * 			0 - BURN_INTO_EFUSE (Burns calibration data to EFuse) 
 -
 * 			1 - BURN_INTO_FLASH (Burns calibration data to Flash) 
 -
 **/
  uint8_t target;       ///< Target
  uint8_t reserved0[3]; ///< Reserved
  /* Flags - Validate information 
 *  Bit |	MACRO 		           |	Description
 *  :---|:---------------------:|:---------------------------------------------------
 *  0   |	RESERVED_0     |  Reserved
 *  1   |	BURN_FREQ_OFFSET     |	1 - Update XO Ctune to calibration data 
 -	0 - Skip XO Ctune update
 *  2   |	SW_XO_CTUNE_VALID    |	1 - Use XO Ctune provided as argument to update calibration data 
 -	0 - Use XO Ctune value as read from hardware register
 *  3   |	BURN_XO_FAST_DISABLE |     Used to apply patch for cold temperature issue(host interface detection) observed on CC0/CC1 modules. \ref appendix
 *  4   |  BURN_GAIN_OFFSET_LOW  | 1 - Update gain offset for low sub-band (2 GHz) 
 -	0 - Skip low sub-band gain-offset update
 *  5   |  BURN_GAIN_OFFSET_MID  | 1 - Update gain offset for mid sub-band (2 GHz) 
 -	0 - Skip mid sub-band gain-offset update
 *  6   |  BURN_GAIN_OFFSET_HIGH | 1 - Update gain offset for high sub-band (2 GHz) 
 -	0 - Skip high sub-band gain-offset update
 *  8   |  ENABLE_DPD_CALIB      | 1 - Collect dpd coefficients data 
 - 0 - Skip dpd coefficients calibration 
 *  9   |  BURN_DPD_COEFFICIENTS | 1 - Burn dpd coefficients data 
 - 0 - Skip dpd coefficients calibration 
 * 10  |  BURN_GAIN_OFFSET_CHANNEL-14  | 1 - Update gain offset for channel-14 sub-band (2 GHz)
 -	0 - Skip channel-14 sub-band gain-offset update
 *  31-4 |                       |	Reserved
 **/
  uint32_t flags; ///< flags
  /*
gain_offset_low - gain_offset as observed in dBm in channel-1
gain_offset_mid - gain_offset as observed in dBm in channel-6
gain_offset_high - gain_offset as observed in dBm in channel-11
*/
  int8_t gain_offset[3]; ///< Gain offset
  /*xo_ctune - Allow user to directly update xo_ctune value to calibration data bypassing the freq offset loop,
 *valid only when BURN_FREQ_OFFSET & SW_XO_CTUNE_VALID of flags is set. The range of xo_ctune is [0, 255], and the typical value is 80
 */
  int8_t xo_ctune; ///< XO Ctune
  /*gain_offset_channel-14 - gain_offset as observed in dBm in channel-14 */
  int8_t gain_offset_ch14; ///< Gain offset channel 14
} sl_si91x_calibration_write_t;

/// Si91x specific calibration read
typedef struct {

  /*  target
* 			0 - READ_FROM_EFUSE (read calibration data from the EFuse) 
* 			1 - READ_FROM_FLASH (read calibration data from the Flash)
*/
  uint8_t target;       ///<  target
  uint8_t reserved0[3]; ///< Reserved
  /*
 gain_offset_low - gain_offset in dBm that is applied for transmissions in channel-1.
 gain_offset_mid - gain_offset in dBm that is applied for transmissions in channel-6.
 gain_offset_high -gain_offset in dBm that is applied for transmissions in channel-11.
 */
  int8_t gain_offset[3]; ///< gain offset

  int8_t xo_ctune; ///< xo_ctune - xo_ctune value as read from the target memory.

  int8_t
    gain_offset_ch14; ///< gain_offset_channel-14 - gain_offset in dBm that is applied for transmissions in channel-14.

#if !defined(SLI_SI917)
  /// RSI EVM Data
  struct sli_evm_data_t {
    int8_t evm_offset[5]; ///< Evm offset
  } sli_evm_data_t;       ///< Evm data
#endif
} sl_si91x_calibration_read_t;

/// Si91x specific frequency offset
typedef struct {
  int32_t frequency_offset_in_khz; ///< Frequency offset in KHZ
} sl_si91x_freq_offset_t;

/// Si91x specific get DPD calibration data
typedef struct {
  int8_t dpd_power_index; ///< Dpd power index given by the user
} sl_si91x_get_dpd_calib_data_t;

/// Si91x specific EVM offset
typedef struct {
  int8_t evm_offset_val; ///<EVM_offset_val - emv_offset value observed.
                         /*            index          -  index of EVM,range from[0 to 4].
 *             index | description
 *             0     | Update evm_offset_11B
 *             1     | Update evm_offset_11G_6M_24M_11N_MCS0_MCS2 
 *             2     | Update evm_offset_11G_36M_54M_11N_MCS3_MCS7 
 *             3     | Update evm_offset_11N_MCS0 
 *             4     | Update evm_offset_11N_MCS7 
 *             > 4   | Reserved
 * */
  uint8_t evm_index;     ///< EVM index
} sl_si91x_evm_offset_t;

/// Si91x Specific EVM write
typedef struct {
  /*
 *Target 
 * 0 - BURN_INTO_EFUSE (Burns calibration data to EFuse)(Not supported)
 * 1 - BURN_INTO_FLASH (Burns calibration data to Flash)
 **/
  uint8_t target; ///< Target
  /*
 * Flags - Validate information 
 -
 *                     Bit |	MACRO 		           |	Description
 *                     :---|:---------------------:|:---------------------------------------------------
 *                     0   |  EVM_OFFSET_CUST_0    | 1 - Update evm_offset_11B rate calibration data 
 -	0 - Skip evm_offset update
 *                     1   |  EVM_OFFSET_CUST_1    | 1 - Update evm_offset_11G_6M_24M_11N_MCS0_MCS2 rate calibration data 
 -	0 - Skip evm_offset update
 *                     2   |  EVM_OFFSET_CUST_2    | 1 - Update evm_offset_11G_36M_54M_11N_MCS3_MCS7 rate calibration data 
 -	0 - Skip evm_offset update
 *                     3   |  EVM_OFFSET_CUST_3    | 1 - Update evm_offset_11N_MCS0 rate calibration data 
 -	0 - Skip evm_offset update
 *                     4   |  EVM_OFFSET_CUST_4    | 1 - Update evm_offset_11N_MCS7 rate calibration data 
 -	0 - Skip evm_offset update
 *                     31-5|  Reserved
 */
  uint32_t flags;                               ///< Flags
  uint8_t evm_offset_11B;                       ///< evm_offset for 11B rate
  uint8_t evm_offset_11G_6M_24M_11N_MCS0_MCS2;  ///< evm_offset for 11G_6M_24M_11N_MCS0_MCS2 rate
  uint8_t evm_offset_11G_36M_54M_11N_MCS3_MCS7; ///< evm_offset for 11G_36M_54M_11N_MCS3_MCS7 rate
  uint8_t evm_offset_11N_MCS0;                  ///< evm_offset for 11N_MCS0 rate
  uint8_t evm_offset_11N_MCS7;                  ///< evm_offset for 11N_MCS7 rate
} sl_si91x_evm_write_t;

/// Si91x specific efuse read
typedef struct {
  ///  Efuse read addr offset
  /**
 *
 *  |efuse_read_addr_offset | Component           | Parameter                             | Size in bytes| Description                                                                                                                                    |
 *  |-----------------------|---------------------|---------------------------------------|--------------|------------------------------------------------------------------------------------------------------------------------------------------------|
 *  |       144             | WLAN info-Cust      | Magic byte                            |1             |Magic byte for WLAN Customer info                                                                                                               |
 *  |       145             |                     | Reserved                              |1             |Reserved                                                                                                                                        |
 *  |       146             |                     | Mac address                           |6             |If this MAC address is set, it overrides the WLAN MAC address set by Silabs.                                                                    |
 *  |       152             | BT/BLE info-Cust    | Magic byte                            |1             |Magic byte for BLE Customer info                                                                                                                |
 *  |       153             |                     | Reserved                              |1             |Reserved                                                                                                                                        |
 *  |       154             |                     | Mac address                           |6             |If this MAC address is set, it overrides the BLE MAC address set by Silabs.                                                                     |
 *  |       160             | XO_offset_cust      | Magic byte                            |1             |0x000B => XO_offset is set by customer.                                                                                                         |
 *  |       161             |                     | XO_offset                             |1             |If a valid XO_OFFSET is programmed here, the one in location 259 is ignored, and this one is taken; otherwise, the one in location 259 is taken.|
 *  |       162             | Gain_offset_cust    | customer_gain_offset_1_3p3            |1             |Gain offset for channel 1,2,3                                                                                                                                   |
 *  |       163             |                     | customer_gain_offset_6_3p3            |1             |Gain offset for channel 4,5,6,7,8                                                                                                                               |
 *  |       164             |                     | customer_gain_offset_11_3p3           |1             |Gain offset for channel 9,10,11,12,13, optional 14 (this is used if 168 magic byte is not set)                                                                  |
 *  |       165             |                     | customer_gain_offset_1_1p8            |1             |(For future use)                                                                                                                                       |
 *  |       166             |                     | customer_gain_offset_6_1p8            |1             |(For future use)                                                                                                                                                |
 *  |       167             |                     | customer_gain_offset_11_1p8           |1             |(For future use)                                                                                                                                                |
 *  |       168             |                     | customer_gain_offset_14_3p3_magic_byte|1             |Magic byte for customer gain offset CH14 0xAB                                                                                                   |
 *  |       169             |                     | customer_gain_offset_14_3p3           |1             |Customer gain offset for CH14, this is valid only when the Magic byte is valid when 168 Location.                                               |
 *  |       170             |                     | Anti-Rollback space for M4 image      |8             |Anti-Rollback bit map. 64 bits represent the version numbers for Anti-Rollback base.                                                             |
 *  |       178             | Customer EVM Offset | Magic byte                            |1             |0xAB => Magic byte set by customer while loading the offset                                                                                     |
 *  |       179             |                     | evm_offset_11B                        |1             |EVM offset of 11B rates                                                                                                                                                |
 *  |       180             |                     | evm_offset_11G_6M_24M_11N_MCS0_MSC2   |1             |EVM offset of 11G 6, 9, 12, 18, 24M and 11N MCS0, MCS1, MCS2                                                                                                                                            |
 *  |       181             |                     | evm_offset_11G_36M_54M_11N_MCS3_MSC7  |1             |EVM offset of 11G 36,48,54M and 11N MCS4, MCS5, MCS6                                                                                                                                               |
 *  |       182             |                     | evm_offset_11N_MCS0                   |1             |EVM offset of 11N MCS0                                                                                                                                           |
 *  |       183             |                     | evm_offset_11N_MCS7                   |1             |EVM offset of 11N MCS7                                                                                                                          |
 *  |       184             |                     | Reserved for Customer                 |71            |Reserved for future use                                                                                                                         |
 *  |       255             |                     | OTP_Lock_1                            |1             |Bit 0 : 1 -> Lock, 0 -> Unlocked (if Bit 0 in OTP_Lock_1 is set, it means locations (128-255) in efuse would not be writable.) Bit 1 : Debug port was opened. If Bit 1 is set, it implies that the debug port was opened using token exchange at least once in the lifetime of the device. Bit 2-7 : Reserved.                                                 |
 * */
  uint32_t efuse_read_addr_offset;
  ///  Efuse read data length
  uint16_t efuse_read_data_len;
} sl_si91x_efuse_read_t;

/** \addtogroup SL_SI91X_TYPES
 * @{
 * */
/// Si91x specific Wi-Fi asynchronous statistics
typedef sl_wifi_async_stats_response_t SL_DEPRECATED_API_WISECONNECT_4_0 sl_si91x_async_stats_response_t;

/// Si917 specific Wi-Fi advance statistics
typedef sl_wifi_advanced_stats_response_t SL_DEPRECATED_API_WISECONNECT_4_0 sl_si91x_advance_stats_response_t;
/** @} */

/// Debug log structure
typedef struct {
  uint32_t assertion_type;  ///< Assertion type. It must be in the range of 0 to 15 (both included).
  uint32_t assertion_level; ///< Assertion level. It must be in the range of 0 to 15 (both included).
} sli_si91x_debug_log_t;

//! @cond Doxygen_Suppress
#ifdef SL_SI91X_SIDE_BAND_CRYPTO
typedef struct crypto_key_s {
  uint32_t key_slot;      ///< For built-in key
  uint32_t wrap_iv_mode;  ///< IV mode 0-> ECB; 1-> CBC
  uint8_t wrap_iv[16];    ///< IV for CBC mode
  uint8_t key_buffer[32]; ///< Key data wrapped/ Plain text
} sli_si91x_crypto_key_t;

typedef struct {
  uint32_t key_size;
  sli_si91x_crypto_key_t key_spec;
} sli_si91x_key_info_t;

typedef struct {
  uint32_t key_type;
  uint32_t reserved;
  sli_si91x_key_info_t key_detail;
} sli_si91x_key_descriptor_t;

typedef struct {
  uint8_t algorithm_type;
  uint8_t algorithm_sub_type;
  uint16_t total_msg_length;
  uint16_t encrypt_decryption;
  uint16_t output_length;
  sli_si91x_key_descriptor_t key_info;
  uint8_t *IV;
  uint8_t *msg;
  uint8_t *output;
} sli_si91x_aes_request_t;

typedef struct {
  uint32_t key_type;
  uint16_t padding;
  uint16_t hmac_sha_mode;
  uint32_t key_size;
  uint32_t wrap_iv_mode;
  uint8_t *wrap_iv;
  uint8_t *key_buffer;
} sli_si91x_wrap_key_descriptor_t;

typedef struct {
  uint16_t algorithm_type;
  uint16_t output_length;
  sli_si91x_wrap_key_descriptor_t key_info;
  uint8_t *output;
} sli_si91x_wrap_request_t;

typedef struct {
  uint16_t algorithm_type;
  uint16_t algorithm_sub_type;
  uint16_t total_length;
  uint16_t output_length;
  sli_si91x_key_descriptor_t key_info;
  uint8_t *hmac_data;
  uint8_t *output;
} sli_si91x_hmac_sha_request_t;

typedef struct {
  uint16_t algorithm_type;
  uint16_t algorithm_sub_type;
  uint16_t total_msg_length;
  uint16_t output_length;
  uint8_t *msg;
  uint8_t *output;
} sli_si91x_sha_request_t;

typedef struct {
  uint8_t algorithm_type;
  uint8_t nonce_length;
  uint16_t encrypt_decryption;
  uint16_t total_msg_length;
  uint16_t ad_length;
  uint16_t tag_length;
  uint16_t output_length;
  sli_si91x_key_descriptor_t key_info;
  uint8_t *nonce;
  uint8_t *ad;
  uint8_t *tag; // tag size = 16
  uint8_t *msg;
  uint8_t *output;
} sli_si91x_ccm_request_t;

typedef struct {
  uint8_t algorithm_type;
  uint8_t dma_use;
  uint8_t gcm_mode;
  uint8_t encrypt_decryption;
  uint16_t total_msg_length;
  uint16_t ad_length;
  uint32_t output_length;
  sli_si91x_key_descriptor_t key_info;
  uint8_t *nonce; // iv length = 12 bytes
  uint8_t *ad;
  uint8_t *msg;
  uint8_t *output;
} sli_si91x_gcm_request_t;

typedef struct {
  uint8_t algorithm_type;
  uint8_t algorithm_sub_type;
  uint8_t encrypt_decryption;
  uint8_t dma_use;
  uint16_t total_msg_length;
  uint16_t header_length;
  uint32_t output_length;
  uint8_t *nonce;
  sli_si91x_key_descriptor_t key_info;
  uint8_t *header_input;
  uint8_t *msg;
  uint8_t *output;
} sli_si91x_chachapoly_request_t;

typedef struct {
  uint8_t algorithm_type;
  uint8_t ecdh_mode;
  uint8_t ecdh_sub_mode;
  uint8_t *sx;
  uint8_t *sy;
  uint8_t *sz;
  uint8_t *tx;
  uint8_t *ty;
  uint8_t *tz;
  uint8_t *rx;
  uint8_t *ry;
  uint8_t *rz;
} sli_si91x_ecdh_add_sub_request_t;

typedef struct {
  uint8_t algorithm_type;
  uint8_t ecdh_mode;
  uint8_t ecdh_sub_mode;
  uint8_t ecdh_curve_type;
  uint32_t affinity;
  uint8_t *d;
  uint8_t *sx;
  uint8_t *sy;
  uint8_t *sz;
  uint8_t *rx;
  uint8_t *ry;
  uint8_t *rz;
} sli_si91x_ecdh_mul_request_t;

typedef struct {
  uint8_t algorithm_type;
  uint8_t ecdh_mode;
  uint8_t ecdh_sub_mode;
  uint8_t *sx;
  uint8_t *sy;
  uint8_t *sz;
  uint8_t *rx;
  uint8_t *ry;
  uint8_t *rz;
} sli_si91x_ecdh_double_request_t;

typedef struct {
  uint8_t algorithm_type;
  uint8_t ecdh_mode;
  uint8_t ecdh_sub_mode;
  uint8_t ecdh_curve_type;
  uint8_t *sx;
  uint8_t *sy;
  uint8_t *sz;
  uint8_t *rx;
  uint8_t *ry;
  uint8_t *rz;
} sli_si91x_ecdh_affine_request_t;

/// Si91x Specific TRNG request
typedef struct {
  uint8_t algorithm_type;     ///< Algorithm type
  uint8_t algorithm_sub_type; ///< Algorithm sub type
  uint16_t total_msg_length;  ///< Total message length
  uint8_t *trng_key;          ///< TRNG key
  uint8_t *msg;               ///< Message
  uint8_t *output;            ///< Output
} sli_si91x_trng_request_t;

/// Attestation token Request Frames Structures
typedef struct {
  uint16_t algorithm_type;   ///< Algorithm type
  uint16_t total_msg_length; ///< Total message length
  uint32_t *msg;             ///< Message
  uint8_t *token_buf;        ///< Token buffer
} sli_si91x_rsi_token_req_t;
#else
typedef struct crypto_key_s {
  uint32_t key_slot;      ///< For built-in key
  uint32_t wrap_iv_mode;  ///< IV mode 0-> ECB; 1-> CBC
  uint8_t wrap_iv[16];    ///< IV for CBC mode
  uint8_t key_buffer[32]; ///< Key data wrapped/ Plain text
} sli_si91x_crypto_key_t;

typedef struct {
  uint32_t key_size;
  sli_si91x_crypto_key_t key_spec;
} sli_si91x_key_info_t;

typedef struct {
  uint32_t key_type;
  uint32_t reserved;
  sli_si91x_key_info_t key_detail;
} sli_si91x_key_descriptor_t;

typedef struct {
  uint16_t algorithm_type;
  uint8_t algorithm_sub_type;
  uint8_t aes_flags;
  uint16_t total_msg_length;
  uint16_t current_chunk_length;
  uint32_t encrypt_decryption;
#if defined(SLI_SI917B0)
  sli_si91x_key_descriptor_t key_info;
#else
  uint32_t key_length;
  uint8_t key[32];
#endif
  uint8_t IV[16];
  uint8_t msg[1408];
} sli_si91x_aes_request_t;

typedef struct {
  uint32_t key_type;
  uint16_t padding;
  uint16_t hmac_sha_mode;
  uint32_t key_size;
  uint32_t wrap_iv_mode;
  uint8_t wrap_iv[16];
  uint8_t key_buffer[1408];
} sli_si91x_wrap_key_descriptor_t;

typedef struct {
  uint8_t algorithm_type;
  uint8_t wrap_flags;
  uint16_t current_chunk_length;
  sli_si91x_wrap_key_descriptor_t key_info;
} sli_si91x_wrap_request_t;

typedef struct {
  uint16_t algorithm_type;
  uint8_t algorithm_sub_type;
  uint8_t hmac_sha_flags;
  uint16_t total_length;
  uint16_t current_chunk_length;
#if defined(SLI_SI917B0)
  sli_si91x_key_descriptor_t key_info;
#else
  uint32_t key_length;
#endif
  uint8_t hmac_data[1400];
} sli_si91x_hmac_sha_request_t;

/// Si91x specific SHA request
typedef struct {
  uint16_t algorithm_type;       ///< Algorithm type
  uint8_t algorithm_sub_type;    ///< Algorithm sub type
  uint8_t sha_flags;             ///< SHA flags
  uint16_t total_msg_length;     ///< Total message length
  uint16_t current_chunk_length; ///< Current chunk length
  uint8_t msg[1400];             ///< Message
} sli_si91x_sha_request_t;

/// Si91x specific CCM request
typedef struct {
  uint16_t algorithm_type;                  ///< Algorithm type
  uint8_t ccm_flags;                        ///< CCM flags
  uint8_t nonce_length;                     ///< Nonce length
  uint16_t encrypt_decryption;              ///< Encrypt/Decrypt
  uint16_t total_msg_length;                ///< Total message length
  uint16_t current_chunk_length;            ///< Current chunk length
  uint16_t ad_length;                       ///< AD length
  uint32_t tag_length;                      ///< Tag length
#if defined(SLI_SI917B0)
  sli_si91x_key_descriptor_t key_info;      ///< Key info
#else
  uint32_t key_length;                   ///< Key length
  uint8_t key[SL_SI91X_KEY_BUFFER_SIZE]; ///< Key
#endif
  uint8_t nonce[SLI_SI91X_CCM_IV_BUFF_LEN]; ///< Nonce
  uint8_t ad[SLI_SI91X_CCM_AD_MAX_SIZE];    ///< AD
  uint8_t tag[SL_SI91X_TAG_SIZE];           ///< tag size = 16
  uint8_t msg[SLI_SI91X_CCM_MSG_MAX_SIZE];  ///< max msg size = 1200 bytes
} sli_si91x_ccm_request_t;

typedef struct {
  uint16_t algorithm_type;
  uint8_t gcm_flags;
  uint8_t encrypt_decryption;
  uint16_t total_msg_length;
  uint16_t current_chunk_length;
  uint16_t ad_length;
  uint16_t dma_use;
#if defined(SLI_SI917B0)
  uint32_t gcm_mode;
  sli_si91x_key_descriptor_t key_info;
#else
  uint32_t key_length;
  uint8_t key[SL_SI91X_KEY_BUFFER_SIZE];
#endif
  uint8_t nonce[SLI_SI91X_GCM_IV_SIZE]; // iv length = 12 bytes
  uint8_t ad[SLI_SI91X_GCM_AD_MAX_SIZE];
  uint8_t msg[SLI_SI91X_GCM_MSG_MAX_SIZE];
} sli_si91x_gcm_request_t;

typedef struct {
  uint16_t algorithm_type;
  uint8_t algorithm_sub_type;
  uint8_t chachapoly_flags;
  uint16_t total_msg_length;
  uint16_t header_length;
  uint16_t current_chunk_length;
  uint16_t encrypt_decryption;
  uint32_t dma_use;
  uint8_t nonce[SL_SI91X_IV_SIZE];
#if defined(SLI_SI917B0)
  sli_si91x_key_descriptor_t key_info;
#else
  uint8_t key_chacha[SL_SI91X_KEY_BUFFER_SIZE];
  uint8_t keyr_in[SLI_SI91X_KEYR_SIZE];
  uint8_t keys_in[SLI_SI91X_KEYS_SIZE];
#endif
  uint8_t header_input[SLI_SI91X_GCM_AD_MAX_SIZE];
  uint8_t msg[SLI_SI91X_CHACHAPOLY_MSG_MAX_SIZE];
} sli_si91x_chachapoly_request_t;

typedef struct {
  uint8_t algorithm_type;
  uint8_t algorithm_sub_type;
  uint8_t ecdsa_flags;
  uint8_t curve_id;
  uint8_t sha_mode;
  uint8_t private_key_length;
  uint8_t public_key_length;
  uint8_t signature_length;
  uint16_t current_chunk_length;
  uint16_t msg_len;
#if defined(SLI_SI917B0)
  sli_si91x_key_descriptor_t key_info;
#else
  uint32_t key_length;
#endif
  uint8_t private_key[SLI_SI91X_PRIVATE_KEY_MAX_SIZE];
  uint8_t public_key[SLI_SI91X_PUBLIC_KEY_MAX_SIZE];
  uint8_t signature[SLI_SI91X_SIGNATURE_MAX_SIZE];
  uint8_t msg[SLI_SI91X_ECDSA_MSG_MAX_SIZE];
} sl_si91x_ecdsa_request_t;

typedef struct {
  uint8_t algorithm_type;
  uint8_t ecdh_mode;
  uint8_t ecdh_sub_mode;
  uint8_t sx[SLI_ECDH_BUFFER_SIZE];
  uint8_t sy[SLI_ECDH_BUFFER_SIZE];
  uint8_t sz[SLI_ECDH_BUFFER_SIZE];
  uint8_t tx[SLI_ECDH_BUFFER_SIZE];
  uint8_t ty[SLI_ECDH_BUFFER_SIZE];
  uint8_t tz[SLI_ECDH_BUFFER_SIZE];
} sli_si91x_ecdh_add_sub_request_t;

typedef struct {
  uint8_t algorithm_type;
  uint8_t ecdh_mode;
  uint8_t ecdh_sub_mode;
  uint8_t ecdh_curve_type;
  uint32_t affinity;
  uint8_t d[SLI_ECDH_BUFFER_SIZE];
  uint8_t sx[SLI_ECDH_BUFFER_SIZE];
  uint8_t sy[SLI_ECDH_BUFFER_SIZE];
  uint8_t sz[SLI_ECDH_BUFFER_SIZE];
} sli_si91x_ecdh_mul_request_t;

typedef struct {
  uint8_t algorithm_type;
  uint8_t ecdh_mode;
  uint8_t ecdh_sub_mode;
  uint8_t sx[SLI_ECDH_BUFFER_SIZE];
  uint8_t sy[SLI_ECDH_BUFFER_SIZE];
  uint8_t sz[SLI_ECDH_BUFFER_SIZE];
} sli_si91x_ecdh_double_request_t;

typedef struct {
  uint8_t algorithm_type;
  uint8_t ecdh_mode;
  uint8_t ecdh_sub_mode;
  uint8_t ecdh_curve_type;
  uint8_t sx[SLI_ECDH_BUFFER_SIZE];
  uint8_t sy[SLI_ECDH_BUFFER_SIZE];
  uint8_t sz[SLI_ECDH_BUFFER_SIZE];
} sli_si91x_ecdh_affine_request_t;

/// Si91x specific TRNG request
typedef struct {
  uint8_t algorithm_type;                ///< Algorithm type
  uint8_t algorithm_sub_type;            ///< Algorithm sub type
  uint16_t total_msg_length;             ///< Total message length
  uint32_t trng_key[TRNG_KEY_SIZE];      ///< TRNG key
  uint32_t msg[SLI_TRNG_TEST_DATA_SIZE]; ///< Message
} sli_si91x_trng_request_t;

/// Attestation token Request Frames Structures
typedef struct {
  uint8_t algorithm_type;        ///< Algorithm type
  uint16_t total_msg_length;     ///< Total message length
  uint32_t msg[NONCE_DATA_SIZE]; ///< Message
} sli_si91x_rsi_token_req_t;
#endif

typedef struct sli_net_mdns_result_s {
  uint16_t query_type; ///< Type of DNS query (e.g., A, AAAA, SRV, TXT)
  uint16_t port;       ///< Port number associated with the service
  uint32_t ttl;        ///< Time-to-live for the DNS record (in seconds)
  uint32_t txt_count;  ///< Number of TXT records in the response
  uint32_t addr_count; ///< Number of IP addresses in the response
  uint8_t data[];      ///< Flexible array member holding TXT and address data
} sli_net_mdns_response_t;
//! @endcond

/// Request timeout Structure
typedef struct {
  uint32_t timeout_bitmap; ///< Timeout bitmap
  uint16_t timeout_value;  ///< Timeout value
} sli_si91x_request_timeout_t;

/// Access point disconnect response structure
#pragma pack(1)
typedef struct {
  sl_mac_address_t client_mac_address; ///< Client MAC address
  uint8_t flag;                        ///< Flag field
  uint8_t ipv4_address[4];             ///< Remote IPv4 Address
  uint8_t link_local_address[16];      ///< Remote link-local IPv6 Address
  uint8_t global_address[16];          ///< Remote unicast global IPv6 Address
} sli_si91x_ap_disconnect_resp_t;

#pragma pack()
