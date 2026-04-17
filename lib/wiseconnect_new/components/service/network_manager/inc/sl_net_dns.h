/***************************************************************************/ /**
 * @file  sl_net_dns.h
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

#include "sl_ip_types.h"
#include "sl_status.h"
#include "sl_net_constants.h"

/** \addtogroup SL_NET_TYPES Types
 * @{ */

/**
 * @brief Structure to hold DNS server addresses for configuration.
 * 
 * @details
 * This structure contains pointers to the primary and secondary DNS server addresses.
 * It is used as a parameter in the sl_net_set_dns_server function to set the DNS server IP addresses.
 */
typedef struct {
  sl_ip_address_t *primary_server_address;   ///< Primary DNS server address
  sl_ip_address_t *secondary_server_address; ///< Secondary DNS server address
} sl_net_dns_address_t;

/** @} */

/** 
 * \addtogroup NET_INTERFACE_FUNCTIONS Network Interface
 * \ingroup SL_NET_FUNCTIONS
 * @{ */

/**
 * @brief
 *   Resolve the given host name to an IP address.
 * 
  * @details
 *   This function resolves a host name to its corresponding IP address. It requires
 *   the DNS client feature to be enabled in the TCP/IP feature bitmap before calling.
 *  

 * 
 * @pre Pre-conditions:
 * - The [sl_net_up](../wiseconnect-api-reference-guide-nwk-mgmt/net-interface-functions#sl-net-up) API must be called before this API.
 * - If [sl_net_up](../wiseconnect-api-reference-guide-nwk-mgmt/net-interface-functions#sl-net-up) is not used, then [sl_si91x_configure_ip_address](../wiseconnect-api-reference-guide-si91x-driver/si91-x-network-functions#sl-si91x-configure-ip-address) should be called prior to this API.
 * - The [SL_SI91X_TCP_IP_FEAT_DNS_CLIENT](../wiseconnect-api-reference-guide-si91x-driver/si91-x-tcp-ip-feature-bitmap#sl-si91-x-tcp-ip-feat-dns-client) bit should be enabled in the TCP/IP feature bitmap.
 * 
 * @param[in] host_name 			 
 *  Host name that needs to be resolved.
 * @param[in] timeout 				 
 *  Timeout in milliseconds.
 *  - If the timeout value is greater than zero, the caller will be blocked until the timeout period to get the response.
 *  - If the value is zero, the response will be sent through @ref sl_net_event_handler_t.
 * 
 * @param[in] dns_resolution_ip 	
 *  DNS resolution by IP of type @ref sl_net_dns_resolution_ip_type_t.
 * @param[out] ip_address 			
 *  IP address object to store resolved IP address of type [sl_ip_address_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-ip-address-t).
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *
 * @note
 *   This function uses a user-configurable timeout parameter that is not affected
 *   by the global timeout scaling factors (SL_WIFI_INTERNAL_COMMANDS_TIMEOUT_SF,
 *   SL_WIFI_MANAGEMENT_COMMANDS_TIMEOUT_SF, SL_WIFI_NETWORK_COMMANDS_TIMEOUT_SF)
 *   or the additional wait time configuration (SL_TX_ADDITIONAL_WAIT_TIME).
 */
sl_status_t sl_net_dns_resolve_hostname(const char *host_name,
                                        const uint32_t timeout,
                                        const sl_net_dns_resolution_ip_type_t dns_resolution_ip,
                                        sl_ip_address_t *ip_address);

/**
 * @brief
 *   Sets DNS server IP addresses.
 *
 * @details
*   This function configures the DNS server IP addresses for the specified network interface.  
*   | Condition                                                      | DNS Mode   |
*   |---------------------------------------------------------------|------------|
*   | Primary DNS and Secondary DNS are both NULL                    | DHCP       |
*   | Primary DNS and Secondary DNS are both zero IP addresses       | DHCP       |
*   | Primary DNS is NULL and Secondary DNS is a zero IP address     | DHCP       |
*   | Secondary DNS is NULL and Primary DNS is a zero IP address     | DHCP       |
*   | Primary DNS is a valid IP address and Secondary DNS is NULL    | Static     |
*   | Secondary DNS is a valid IP address and Primary DNS is NULL    | Static     |
*   | Both Primary DNS and Secondary DNS are valid IP addresses      | Static     |
*   | Primary DNS is zero IP address and Secondary DNS is a valid IP address | Static |
*   | Secondary DNS is zero IP address and Primary DNS is a valid IP address | Static |
 * @pre Pre-conditions:
 * - The [SL_SI91X_TCP_IP_FEAT_DNS_CLIENT](../wiseconnect-api-reference-guide-si91x-driver/si91-x-tcp-ip-feature-bitmap#sl-si91-x-tcp-ip-feat-dns-client) bit should be enabled in the TCP/IP feature bitmap.
 * 
 * @param[in] interface
 *   The network interface of type @ref sl_net_interface_t.
 * 
 * @param[in] address
 *   The structure containing the primary and secondary server addresses of type @ref sl_net_dns_address_t.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 */
sl_status_t sl_net_set_dns_server(sl_net_interface_t interface, const sl_net_dns_address_t *address);

/** @} */
