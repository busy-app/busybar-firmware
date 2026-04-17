/***************************************************************************/ /**
 * @file  sl_net_ip_types.h
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

#include <stdint.h>
#include <limits.h>
#include "sl_ip_types.h"
#include "sl_constants.h"

#if defined(__CC_ARM)
#pragma anon_unions
#endif

/** \addtogroup SL_NET_TYPES Types
 * @{ */

/**
 * @brief IPv4 address settings for a network interface.
 * 
 * @details
 * This structure holds the IPv4 address configuration for a network interface, including the IP address, gateway, and netmask.
 * 
 * @note
 * Each field is of type [sl_ipv4_address_t](../wiseconnect-api-reference-guide-common/sl-ipv4-address-t).
 */
typedef struct {
  sl_ipv4_address_t
    ip_address; ///< IPv4 address of type [sl_ipv4_address_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-ipv4-address-t)
  sl_ipv4_address_t
    gateway; ///< IPv4 gateway address of [sl_ipv4_address_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-ipv4-address-t)
  sl_ipv4_address_t
    netmask; ///< IPv4 netmask of type of [sl_ipv4_address_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-ipv4-address-t)
} sl_net_ipv4_setting_t;

/**
 * @brief IPv6 address settings for a network interface.
 * 
 * @details
 * This structure holds the IPv6 address configuration for a network interface, including the link-local address, global address, and gateway.
 * 
 * @note
 * Each field is of type [sl_ipv6_address_t](../wiseconnect-api-reference-guide-common/sl-ipv6-address-t).
 */
typedef struct {
  sl_ipv6_address_t
    link_local_address; ///< IPv6 link local address of type [sl_ipv6_address_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-ipv6-address-t)
  sl_ipv6_address_t
    global_address; ///< IPv6 global address of type [sl_ipv6_address_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-ipv6-address-t)
  sl_ipv6_address_t
    gateway; ///< IPv6 gateway address of type of [sl_ipv6_address_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-ipv6-address-t)
} sl_net_ipv6_setting_t;

/**
 * @brief Structure representing the DHCP configuration for the network manager.
 *
 * This structure holds the DHCP configuration parameters for the network manager.
 * It includes the minimum and maximum retry intervals for discovery and request,
 * as well as the maximum number of retries for discovery and request.
 *
 * ## Theory Behind the Retry Interval and Total Time Calculation
 *
 * When retrying a request (such as DHCP discovery or request), the retry interval increases exponentially,
 * starting from the minimum retry interval (e.g., `min_discover_retry_interval` or `min_request_retry_interval`). 
 * Each successive retry interval doubles, but it is capped by the maximum retry interval 
 * (e.g., `max_discover_retry_interval` or `max_request_retry_interval`).
 *
 * - **Retry Interval Formula**:
 *   ```c
 *   R_n = min(max_retry_interval, min_retry_interval × 2^(n-1))
 *   ```
 *   Where:
 *     - `R_n`: Retry interval for the nth attempt.
 *     - `min_retry_interval`: Starting retry interval (e.g., `min_discover_retry_interval`).
 *     - `max_retry_interval`: Upper limit for the retry interval (e.g., `max_discover_retry_interval`).
 *     - `n`: Retry attempt number (1-based index).
 *
 * As retries progress, the interval continues to double until it reaches the `max_retry_interval`, 
 * after which it remains constant.
 *
 * - **Total Retry Time Formula**:
 *   ```c
 *   T_total = sum(R_n) for n = 1 to N_max
 *   ```
 *   Where:
 *     - `T_total`: Total retry time.
 *     - `N_max`: Maximum number of retries (e.g., `max_discover_retries` or `max_request_retries`).
 *
 * ## Example
 * Given the following configuration:
 *   - `min_discover_retry_interval = 5` seconds
 *   - `max_discover_retry_interval = 100` seconds
 *   - `max_discover_retries = 10`
 *
 * The retry intervals will be:
 *   `R = [5, 10, 20, 40, 80, 100, 100, 100, 100, 100]`
 * Total retry time:
 *   `T_total = 655 seconds`
 *
 * ## Reference Implementation
 * Below is an example implementation to calculate the total retry time based on the retry parameters:
 * 
 * ```c
 * uint16_t calculateTotalRetryTime(uint16_t min_interval, uint16_t max_interval, uint8_t max_retries) {
 *     uint16_t total_time = 0, interval = min_interval;
 *     for (uint8_t i = 1; i <= max_retries; i++) {
 *         total_time += interval;  // Add the current retry interval to the total time
 *         interval = (interval * 2 > max_interval) ? max_interval : interval * 2;  // Cap interval at max_interval
 *     }
 *     return total_time;
 * }
 *
 * // Example usage:
 * uint16_t total_time = calculateTotalRetryTime(5, 100, 10);
 * printf("Total Retry Time: %d seconds\n", total_time); // Output: 655 seconds
 * ```
 * 
 * @note
 * - This configuration is not supported for IPv6 in SI91X_INTERNAL_STACK.
 * - The retry interval formula and total retry time formula apply to both discovery 
 *   (`min_discover_retry_interval`, `max_discover_retry_interval`, `max_discover_retries`) 
 *   and request (`min_request_retry_interval`, `max_request_retry_interval`, `max_request_retries`).
 * - The total wait time is the sum of the total discovery retry time and the total request retry time.
 * - Moving forward, 'min_discover_retries' member variable will be deprecated. Instead, use the `max_discover_retries`. This is retained for backward compatibility.
 */
typedef struct {
  uint16_t min_discover_retry_interval; ///< Minimum retry interval for discovery
  uint16_t max_discover_retry_interval; ///< Maximum retry interval for discovery
  uint16_t min_request_retry_interval;  ///< Minimum retry interval for request
  uint16_t max_request_retry_interval;  ///< Maximum retry interval for request
  union {
    uint8_t min_discover_retries; ///< Deprecated. Use `max_discover_retries` instead.
    uint8_t max_discover_retries; ///< Maximum number of retries for discovery
  };
  uint8_t max_request_retries; ///< Maximum number of retries for request
} sl_net_dhcp_configuration_t;

/// IP configuration for a network interface
typedef struct {
  sl_ip_management_t
    mode; ///< IP Assignment Type of [sl_ip_management_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-net-constants#sl-ip-management-t)
  sl_ip_address_type_t
    type; ///< IP Address Type of [sl_ip_address_type_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-net-constants#sl-ip-address-type-t)
  char *host_name; ///< Host name visible on network
  struct {
    sl_net_ipv4_setting_t v4; ///< IPv4 setting to be used in IP address assignment of type @ref sl_net_ipv4_setting_t
    sl_net_ipv6_setting_t v6; ///< IPv6 setting to be used in IP address assignment of type @ref sl_net_ipv6_setting_t
  } ip;                       ///< IP setting to be used for IP address assignment

  sl_net_dhcp_configuration_t dhcp_config; ///< DHCP configuration for the network manager
} sl_net_ip_configuration_t;

/// IP Address of a network interface
typedef struct {
  sl_ip_management_t
    mode; ///< IP Assignment Type of [sl_ip_management_t](../wiseconnect-api-reference-guide-common/ip-addresses#sl-ip-management-t)
  sl_ip_address_type_t
    type; ///< IP Address Type of [sl_ip_address_type_t](../wiseconnect-api-reference-guide-common/ip-addresses#sl-ip-address-type-t)
  sl_net_ipv4_setting_t v4; ///< IPv4 setting to be used in IP address assignment of type @ref sl_net_ipv4_setting_t
  sl_net_ipv6_setting_t v6; ///< IPv6 setting to be used in IP address assignment of type @ref sl_net_ipv6_setting_t
} sl_net_ip_address_t;

/** @} */
