/***************************************************************************/ /**
 * @file
 * @brief Network types
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

#include "sl_ip_types.h"
#include "sl_net_constants.h"
#include "sl_net_ip_types.h"
#include "sl_constants.h"
#include "sl_status.h"
#include "sl_utility.h"
#include "sl_wifi_types.h"
#include <stdint.h>

/** \addtogroup SL_NET_TYPES Types
 * @{ */

/**
 * @typedef sl_net_event_handler_t
 * @brief Generic callback for network events.
 * 
 * @details
 * This typedef defines a callback function for handling various network events. The callback function receives the event type, status, data, and data length as parameters.
 * 
 * @param event
 * Network event of type @ref sl_net_event_t.
 * | @ref sl_net_event_t                  | DataType                               |
 * |:-------------------------------------|:---------------------------------------|
 * | SL_NET_PING_RESPONSE_EVENT           | @ref sl_net_ping_response_t          |
 * | SL_NET_DNS_RESOLVE_EVENT             | [sl_ip_address_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-ip-address-t) |
 * | SL_NET_OTA_FW_UPDATE_EVENT           | NULL in case of success, else uint16_t chunk number in case of failure |
 * | SL_NET_DHCP_NOTIFICATION_EVENT       | NULL                                   |
 * | SL_NET_IP_ADDRESS_CHANGE_EVENT       | @ref sl_net_ip_configuration_t         |
 * | SL_NET_AUTO_JOIN_EVENT               | @ref sl_net_auto_join_status_t         |
 * | SL_NET_CONNECT_EVENT                 | NULL  |
 * | SL_NET_IP_CONFIG_EVENT               | NULL  |
 * | SL_NET_EVENT_COUNT                   | Not Applicable, Internally used by SDK |
 * 
 * @param status
 * Status of type sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * 
 * @param data
 * Data received, corresponding to the event type.
 * 
 * @param data_length
 * Length of the data received.
 * 
 * @return
 * Status of type sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 */
typedef sl_status_t (*sl_net_event_handler_t)(sl_net_event_t event,
                                              sl_status_t status,
                                              void *data,
                                              uint32_t data_length);

/**
 * @brief Abstract profile for SL Net.
 * 
 * @details
 * This type represents an abstract profile in the SL Net framework. 
 * 
 * @note
 * This type is used internally by the SL Net framework and should be cast to the appropriate profile type when used.
 */
typedef void sl_net_profile_t;

/**
 * @struct sl_net_ping_response_t
 * @brief Ping Response structure.
 * 
 * @details
 * This structure holds the response data for a ping operation, including the IP version, ping size, and the pinged IP address.
 */
typedef struct {
  uint16_t
    ip_version; ///< IP version (for example, IPv4 or IPv6). One of the values from [sl_ip_version_t](../wiseconnect-api-reference-guide-common/ip-addresses#sl-ip-version-t).
  uint16_t ping_size; ///< Size of the ping packet
  union {
    uint8_t ipv4_address[4];  ///< IPv4 address
    uint8_t ipv6_address[16]; ///< IPv6 address
  } ping_address;             ///< Pinged IP address
} sl_net_ping_response_t;

/**
 * @brief Alias for sl_net_ping_response_t.
 * 
 * @details
 * This typedef provides an alias for the `sl_net_ping_response_t` structure, 
 * specifically for use in the Si91x platform. It represents the response data 
 * for a ping operation, including IP version, ping size, and the pinged IP address.
 * 
 * @note
 * Moving forward, this `sl_si91x_ping_response_t` type will be deprecated. Instead, use @ref sl_net_ping_response_t type. This is retained for backward compatibility.
 */
typedef sl_net_ping_response_t SL_DEPRECATED_API_WISECONNECT_4_0 sl_si91x_ping_response_t;

/**
 * @struct sl_net_nat_config_t
 * @brief NAT session configuration structure.
 * 
 * @details
 * This structure defines session timeout values for Network Address Translation (NAT)
 * on a network interface. It allows configuration of separate timeout durations for
 * TCP and non-TCP sessions, as well as specifying the network interface to which
 * these settings apply.
 */
typedef struct {
  uint32_t tcp_session_timeout;     ///< NAT TCP session timeout in seconds
  uint32_t non_tcp_session_timeout; ///< NAT non-TCP session timeout in seconds
  sl_net_interface_t interface;     ///< Network interface to apply NAT settings
} sl_net_nat_config_t;

/**
 * @struct sl_net_interface_info_t
 * @brief Structure containing network interface information.
 * 
 * @details
 * This structure holds information about a network interface, including IPv4 and IPv6 addresses,
 * and hardware-specific details. The hardware-specific information is stored in a union, allowing for extension
 * to other hardware types as needed.
 */
typedef struct {
  sl_ipv4_address_t ipv4_address; ///< Module IPv4 Address
  sl_ipv6_address_t ipv6_address; ///< Module IPv6 Address
  union {
    sl_wifi_interface_info_t
      wifi_info; ///< Wi-Fi specific interface information [sl_wifi_interface_info_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-interface-info-t)
    // Add other hardware-specific structs here if needed
  } hw_info; ///< Hardware-specific information
} sl_net_interface_info_t;
/** @} */
