/***************************************************************************/ /**
 * @file
 * @brief Networking constants
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <limits.h>
#include "sl_wifi_constants.h"

/// Network Interface Type mask
#define NETWORK_INTERFACE_TYPE_MASK 0xFFFF

#define MAX_WIFI_CLIENT_PROFILES 2 ///< Maximum number of Wi-Fi client profiles.
#define MAX_WIFI_AP_PROFILES     2 ///< Maximum number of Wi-Fi access point profiles.

#define AUTO_JOIN_RETRY_COUNT 3 ///< Number of retries for auto-join.

#define AUTO_JOIN_SUCCESS_FLAG 0x01 ///< Flag indicating auto-join success.
#define AUTO_JOIN_FAILURE_FLAG 0x02 ///< Flag indicating auto-join failure.

/// Interface Type Selection
#define SL_NET_INTERFACE_TYPE(x) (x & NETWORK_INTERFACE_TYPE_MASK)

#define SL_NET_MAX_MAC_ADDR_LENGTH 6 ///< Length of the MAC address for network interfaces.

/** \addtogroup SL_NET_CONSTANTS Constants
 * @{ */

// Enumeration of network interfaces.
// @note Only Wi-Fi client and Wi-Fi access point interfaces currently supported.
/**
 * @enum sl_net_interface_t
 * @brief Enumeration of network interfaces.
 * 
 * @details
 * This enumeration defines the various network interfaces supported by the system. Currently, only `SL_NET_WIFI_CLIENT_INTERFACE` and `SL_NET_WIFI_AP_INTERFACE` are supported.
 */
typedef enum {
  SL_NET_WIFI_CLIENT_1_INTERFACE = (1 << 3),                       ///< Wi-Fi Client 1 Interface
  SL_NET_WIFI_CLIENT_2_INTERFACE = (1 << 3) + 1,                   ///< Wi-Fi Client 2 Interface
  SL_NET_WIFI_CLIENT_INTERFACE   = SL_NET_WIFI_CLIENT_1_INTERFACE, ///< Wi-Fi Client Interface
  SL_NET_WIFI_AP_1_INTERFACE     = (2 << 3),                       ///< Wi-Fi Access Point 1 Interface
  SL_NET_WIFI_AP_2_INTERFACE     = (2 << 3) + 1,                   ///< Wi-Fi Access Point 2 Interface
  SL_NET_WIFI_AP_INTERFACE       = SL_NET_WIFI_AP_1_INTERFACE,     ///< Wi-Fi Access Point Interface
  SL_NET_ETHERNET_INTERFACE      = (3 << 3),                       ///< Ethernet Interface (not currently supported)
  SL_NET_THREAD_INTERFACE        = (4 << 3),                       ///< Thread Interface (not currently supported)
  SL_NET_BLUETOOTH_INTERFACE     = (5 << 3),                       ///< Bluetooth Interface (not currently supported)
  SL_NET_ZWAVE_INTERFACE         = (6 << 3),                       ///< Z-Wave Interface (not currently supported)
} sl_net_interface_t;

/// This macro sets the wait period bit flag to indicate infinite wait time for network API calls.
#define SL_NET_WAIT_FOREVER (1 << 31)

/** @} */

///Maximum number of network interfaces supported
#define SL_NET_INTERFACE_MAX (SL_NET_WIFI_AP_2_INTERFACE + 1)

///Max Client Interfaces
#define MAX_NET_CLIENT_INTERFACES 2

///Max Access Point Interfaces
#define MAX_NET_AP_INTERFACES 2
/** \addtogroup SL_NET_CONSTANTS Constants
 * @{ */
/**
 * @enum sl_net_dns_resolution_ip_type_t
 * @brief Enumeration of DNS resolution IP types.
 * 
 * @details
 * This enumeration defines the types of IP addresses that can be resolved by DNS. It includes both IPv4 and IPv6 address types.
 */
typedef enum {
  SL_NET_DNS_TYPE_IPV4, ///< IPV4 DNS Address resolution.
  SL_NET_DNS_TYPE_IPV6  ///< IPV6 DNS Address resolution.
} sl_net_dns_resolution_ip_type_t;

/**
 * @enum sl_net_event_t
 * @brief Enumeration of SL-Net Events.
 * 
 * @details
 * This enumeration defines the various network events that can be handled by the system. Each event type corresponds to a specific network-related occurrence.
 */
typedef enum {
  SL_NET_PING_RESPONSE_EVENT,     ///< Event triggered when a ping response is received.
  SL_NET_DNS_RESOLVE_EVENT,       ///< Event triggered when a DNS address resolution occurs.
  SL_NET_OTA_FW_UPDATE_EVENT,     ///< Event triggered when an OTA firmware update is completed.
  SL_NET_DHCP_NOTIFICATION_EVENT, ///< Event triggered when a DHCP notification is received.
  SL_NET_IP_ADDRESS_CHANGE_EVENT, ///< Event triggered when there is a DHCP IP address change.
  SL_NET_AUTO_JOIN_EVENT, ///< Event triggered when the auto-join process starts, is in progress, or is completed.
  SL_NET_CONNECT_EVENT, ///< Event triggered when Wi-Fi connection completes (status: SL_STATUS_OK=success, else=specific error code). @note ONLY for @ref sl_net_up_async().
  SL_NET_IP_CONFIG_EVENT, ///< Event triggered when IP configuration completes (status: SL_STATUS_OK=success, else=specific error code). @note ONLY for @ref sl_net_up_async().
  SL_NET_MDNS_EVENT,      ///< Event triggered when an mDNS query response is received.
  SL_NET_MDNS_STOP_EVENT, ///< Event triggered when the mDNS service is stopped.
  SL_NET_EVENT_COUNT      ///< Represents the maximum count of events. Used internally by the SDK.
} sl_net_event_t;

/**
 * @enum sl_net_profile_id_t
 * @brief Enumeration of SL Network profile IDs.
 * 
 * @details
 * This enumeration defines the various profile IDs that can be used in the SL Network. Each profile ID corresponds to a specific network profile.
 * 
 * @note
 * Ethernet, Thread, and Z-Wave profiles are defined but not currently supported.
 */
typedef enum {
  SL_NET_PROFILE_ID_0  = 0,  ///< Profile Id 0
  SL_NET_PROFILE_ID_1  = 1,  ///< Profile Id 1
  SL_NET_PROFILE_ID_2  = 2,  ///< Profile Id 2
  SL_NET_PROFILE_ID_3  = 3,  ///< Profile Id 3
  SL_NET_PROFILE_ID_4  = 4,  ///< Profile Id 4
  SL_NET_PROFILE_ID_5  = 5,  ///< Profile Id 5
  SL_NET_PROFILE_ID_6  = 6,  ///< Profile Id 6
  SL_NET_PROFILE_ID_7  = 7,  ///< Profile Id 7
  SL_NET_PROFILE_ID_8  = 8,  ///< Profile Id 8
  SL_NET_PROFILE_ID_9  = 9,  ///< Profile Id 9
  SL_NET_PROFILE_ID_10 = 10, ///< Profile Id 10

  SL_NET_AUTO_JOIN = 0xFF, ///< Auto-join profile ID

  SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID = SL_NET_PROFILE_ID_0, ///< Wi-Fi Client Default Profile.
  SL_NET_DEFAULT_WIFI_AP_PROFILE_ID     = SL_NET_PROFILE_ID_0, ///< Wi-Fi Access Point Default Profile.
  SL_NET_DEFAULT_ETHERNET_PROFILE_ID    = SL_NET_PROFILE_ID_0, ///< Ethernet Default Profile (not currently supported).
  SL_NET_DEFAULT_THREAD_PROFILE_ID      = SL_NET_PROFILE_ID_0, ///< Thread Default Profile  (not currently supported).
  SL_NET_DEFAULT_ZWAVE_PROFILE_ID       = SL_NET_PROFILE_ID_0, ///< Zwave Default Profile  (not currently supported).
} sl_net_profile_id_t;

/**
 * @enum sl_net_credential_type_t
 * @brief Enumeration of network credential types.
 * 
 * @details
 * This enumeration defines the various types of network credentials that can be used for authentication and security purposes.
 */
typedef enum {
  SL_NET_INVALID_CREDENTIAL_TYPE,  ///< Invalid Credential Type.
  SL_NET_WIFI_PSK,                 ///< Wi-Fi PSk Credential.
  SL_NET_WIFI_PMK,                 ///< Wi-Fi PMK Credential.
  SL_NET_WIFI_WEP,                 ///< Wi-Fi WEP Credential.
  SL_NET_CERTIFICATE,              ///< TLS Client Certificate.
  SL_NET_PUBLIC_KEY,               ///< TLS Certificate Public key.
  SL_NET_PRIVATE_KEY,              ///< TLS Certificate Private key.
  SL_NET_PACK_FILE,                ///< EAP Fast Pack File.
  SL_NET_SIGNING_CERTIFICATE,      ///< TLS CA Certificate.
  SL_NET_HTTP_CLIENT_CREDENTIAL,   ///< HTTP Client Credential.
  SL_NET_EAP_CLIENT_CREDENTIAL,    ///< Wi-Fi EAP Credential.
  SL_NET_MQTT_CLIENT_CREDENTIAL,   ///< MQTT Client Credential.
  SL_NET_TLS_PRIVATE_KEY_CBC_WRAP, ///< CBC TLS Private Certificate Key.
  SL_NET_TLS_PRIVATE_KEY_ECB_WRAP  ///< ECB TLS Private Certificate Key.
} sl_net_credential_type_t;

/**
 * @enum sl_net_auto_join_status_t
 * @brief Enumeration to represent the status of the auto-join process.
 * 
 */
typedef enum {
  SL_NET_AUTO_JOIN_IN_PROGRESS, ///< Auto-join process is in progress.
  SL_NET_AUTO_JOIN_CONNECTED,   ///< Successfully connected to an AP.
  SL_NET_AUTO_JOIN_FAILED       ///< Failed to connect to any AP.
} sl_net_auto_join_status_t;

/**
 * @typedef sl_net_certificate_id_t
 * @brief Unique identifier for a certificate in the certificate store.
 * 
 * @details
 * This defines a unique identifier for the certificates stored in the certificate store. It is used as a reference and to manage certificates within the system.
 */
typedef uint32_t sl_net_certificate_id_t;

/**
 * @enum sl_net_credential_id_t
 * @brief Enumeration of network credential identifiers.
 * 
 * @details
 * This enumeration defines the various identifiers for network credentials used within the system. Each identifier corresponds to a specific type of credential.
 * 
 * @note
 *  - In case of @ref sl_net_credential_type_t of @ref SL_NET_CERTIFICATE, @ref SL_NET_PUBLIC_KEY, @ref SL_NET_PRIVATE_KEY, @ref SL_NET_SIGNING_CERTIFICATE, @ref SL_NET_PACK_FILE, the user can ONLY set @ref SL_NET_WIFI_EAP_CLIENT_CREDENTIAL_ID, @ref SL_NET_WIFI_EAP_SERVER_CREDENTIAL_ID, @ref SL_NET_TLS_CLIENT_CREDENTIAL_START, or @ref SL_NET_TLS_SERVER_CREDENTIAL_START at a time for Si91x devices.
 */
typedef enum {
  SL_NET_INVALID_CREDENTIAL_ID = 0, ///< Invalid Credential Id. (Deprecated: Use SL_NET_NO_CREDENTIAL_ID instead)
  SL_NET_NO_CREDENTIAL_ID      = 0, ///< No Credential.
  SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID = 1, ///< Wi-Fi Client Credential Id.
  ///< @note In case of `Basic Network Configuration Manager`, by default `default_wifi_client_credential` from `sl_net_default_values.h` is used. User can choose to override this by setting the credential using @ref sl_net_set_credential.
  SL_NET_WIFI_CLIENT_CREDENTIAL_ID_1   = 2, ///< Wi-Fi Client Credential Id 1.
  SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID = 3, ///< Wi-Fi Access Point Credential Id.
  ///< @note In case of `Basic Network Configuration Manager`, by default `default_wifi_ap_credential` from `sl_net_default_values.h` is used. User can choose to override this by setting the credential using @ref sl_net_set_credential.
  SL_NET_WIFI_AP_CREDENTIAL_ID_1       = 4,        ///< Wi-Fi AP Credential Id 1.
  SL_NET_WIFI_EAP_CLIENT_CREDENTIAL_ID = 5,        ///< Wi-Fi EAP Client Credential Id.
  SL_NET_WIFI_EAP_SERVER_CREDENTIAL_ID = 6,        ///< Wi-Fi EAP Server Credential Id.
  SL_NET_USER_CREDENTIAL_ID            = 7,        ///< User Credential Id.
  SL_NET_TLS_CLIENT_CREDENTIAL_START   = (1 << 8), ///< TLS Client Credential Id.
  SL_NET_TLS_SERVER_CREDENTIAL_START   = (2 << 8), ///< TLS Server Credential Id.
  SL_NET_MQTT_SERVER_CREDENTIAL_START  = (3 << 8), ///< MQTT Server Credential Id.
  SL_NET_MQTT_CLIENT_CREDENTIAL_START  = (4 << 8), ///< MQTT Client Credential Id.
  SL_NET_HTTP_SERVER_CREDENTIAL_START  = (5 << 8), ///< HTTP Server Credential Id.
  SL_NET_HTTP_CLIENT_CREDENTIAL_START  = (6 << 8), ///< HTTP Client Credential Id.
} sl_net_credential_id_t;

/**
 * @def SL_NET_CREDENTIAL_GROUP_MASK
 * @brief Mask for extracting the credential group from a credential ID.
 * 
 * @details
 * This mask is used to extract the group portion of a credential ID. It helps in identifying the group to which a credential belongs.
 * 
 * The credential group can be one of the value from @ref sl_net_credential_id_t.
 */
#define SL_NET_CREDENTIAL_GROUP_MASK 0xFF00

/**
 * @def SL_NET_TLS_CLIENT_CREDENTIAL_ID(x)
 * @brief Macro to generate a TLS Client Credential ID.
 * 
 * @details
 * This macro generates a unique TLS Client Credential ID by adding an offset to the base TLS Client Credential ID.
 * 
 * x Offset to be added to the base TLS Client Credential ID. 
 * 
 * @note
 *   - In case of @ref sl_net_credential_type_t of @ref SL_NET_CERTIFICATE, @ref SL_NET_PUBLIC_KEY, @ref SL_NET_PRIVATE_KEY, @ref SL_NET_SIGNING_CERTIFICATE, @ref SL_NET_PACK_FILE, the valid values for x are 0 to 2 for Si91x devices.
 *   - In case of other types in @ref sl_net_credential_type_t, the valid values for x is only 0.
 */
#define SL_NET_TLS_CLIENT_CREDENTIAL_ID(x) (SL_NET_TLS_CLIENT_CREDENTIAL_START + x)

/**
 * @def SL_NET_TLS_SERVER_CREDENTIAL_ID(x)
 * @brief Macro to generate a TLS Server Credential ID.
 * 
 * @details
 * This macro generates a unique TLS Server Credential ID by adding an offset to the base TLS Server Credential ID.
 * 
 * x Offset to be added to the base TLS Server Credential ID.
 * 
 * @note
 *   - In case of @ref sl_net_credential_type_t of @ref SL_NET_CERTIFICATE, @ref SL_NET_PUBLIC_KEY, @ref SL_NET_PRIVATE_KEY, @ref SL_NET_SIGNING_CERTIFICATE, @ref SL_NET_PACK_FILE, the valid values for x are 0 to 2 for Si91x devices.
 *   - In case of other types in @ref sl_net_credential_type_t, the valid values for x is only 0.
 */
#define SL_NET_TLS_SERVER_CREDENTIAL_ID(x) (SL_NET_TLS_SERVER_CREDENTIAL_START + x)

/**
 * @def SL_NET_MQTT_SERVER_CREDENTIAL_ID(x)
 * @brief Macro to generate an MQTT Server Credential ID.
 * 
 * @details
 * This macro generates a unique MQTT Server Credential ID by adding an offset to the base MQTT Server Credential ID.
 * 
 * x Offset to be added to the base MQTT Server Credential ID.
 * 
 * @note
 * The valid values for x is only 0.
 */
#define SL_NET_MQTT_SERVER_CREDENTIAL_ID(x) (SL_NET_MQTT_SERVER_CREDENTIAL_START + x)

/**
 * @def SL_NET_MQTT_CLIENT_CREDENTIAL_ID(x)
 * @brief Macro to generate an MQTT Client Credential ID.
 * 
 * @details
 * This macro generates a unique MQTT Client Credential ID by adding an offset to the base MQTT Client Credential ID.
 * 
 * x Offset to be added to the base MQTT Client Credential ID.
 * 
 * @note
 * The valid values for x is only 0.
 */
#define SL_NET_MQTT_CLIENT_CREDENTIAL_ID(x) (SL_NET_MQTT_CLIENT_CREDENTIAL_START + x)

/**
 * @def SL_NET_HTTP_SERVER_CREDENTIAL_ID(x)
 * @brief Macro to generate an HTTP Server Credential ID.
 * 
 * @details
 * This macro generates a unique HTTP Server Credential ID by adding an offset to the base HTTP Server Credential ID.
 * 
 * x Offset to be added to the base HTTP Server Credential ID.
 * 
 * @note
 * The valid values for x is only 0.
 */
#define SL_NET_HTTP_SERVER_CREDENTIAL_ID(x) (SL_NET_HTTP_SERVER_CREDENTIAL_START + x)

/**
 * @def SL_NET_HTTP_CLIENT_CREDENTIAL_ID(x)
 * @brief Macro to generate an HTTP Client Credential ID.
 * 
 * @details
 * This macro generates a unique HTTP Client Credential ID by adding an offset to the base HTTP Client Credential ID.
 * 
 * x Offset to be added to the base HTTP Client Credential ID.
 * 
 * @note
 * The valid values for x is only 0.
 */
#define SL_NET_HTTP_CLIENT_CREDENTIAL_ID(x) (SL_NET_HTTP_CLIENT_CREDENTIAL_START + x)

/**
 * @brief Network Manager Thread Stack Size Configuration
 * 
 * Default value of 3072 bytes.
 * To use a custom value, define SL_NET_NETWORK_MANAGER_THREAD_STACK_SIZE in the preprocessor settings of the project.
 */
#ifndef SL_NET_NETWORK_MANAGER_THREAD_STACK_SIZE
#define SL_NET_NETWORK_MANAGER_THREAD_STACK_SIZE 3072
#endif

/**
 * @brief Network Manager Thread Priority Configuration
 * 
 * Default value is osPriorityNormal.
 * To use a custom value, define SL_NET_NETWORK_MANAGER_THREAD_PRIORITY in the preprocessor settings of the project.
 */
#ifndef SL_NET_NETWORK_MANAGER_THREAD_PRIORITY
#define SL_NET_NETWORK_MANAGER_THREAD_PRIORITY osPriorityNormal
#endif
/** @} */
