/***************************************************************************//**
 * @file sl_wisun_br_dhcpv6_server.h
 * @brief Components that implements a DHCPv6 server for Wi-SUN Border Router
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef __SL_WISUN_BR_DHCPV6_SERVER_H__
#define __SL_WISUN_BR_DHCPV6_SERVER_H__

#include "sl_status.h"
#include "sl_wisun_types.h"
#include "common/ns_list.h"

/**  UDP Port Number definition */
#define DHCPV6_SERVER_PORT 547
#define DHCPV6_CLIENT_PORT 546

typedef struct dhcpv6_vendor_data_s {
    uint32_t enterprise_number;
    uint8_t *vendor_data;
    uint16_t vendor_data_length;
    ns_list_link_t link;
} dhcpv6_vendor_data_t;

typedef NS_LIST_HEAD(dhcpv6_vendor_data_t, link) dhcpv6_vendor_data_list_t;
/***************************************************************************//**
 * Initialize DHCPv6 Server.
 * This function must be called before any other DHCPv6 Server function.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_dhcpv6_server_init(void);

/***************************************************************************//**
 * Start DHCPv6 Server.
 *
 * @param[in] socket Socket open and bound to DHCPv6 Server port.
 * @param[in] prefix DHCPv6 Server's IPv6 prefix. Must be the same IPv6 prefix set with sl_wisun_br_set_ipv6_prefix().
 * @param[in] DUID DHCPv6 Server's DHCP Unique Identifier. Should be the equal
 * to the border router's MAC address.
 * @param[in] dhcp_address_lifetime Addresses' valid lifetime. Any value
 * different than LIFETIME_INFINITE is not supported.
 * @return SL_STATUS_OK if successful, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_dhcpv6_server_start(int socket, uint8_t prefix[8], uint8_t DUID[8], uint32_t dhcp_address_lifetime);

/***************************************************************************//**
 * Handle DHCPv6 relay-forward or solicit messages.
 *
 * @param[in] buffer Message's buffer
 * @param[in] length Length of the message received
 * @param[in] peer_address Peer's IPv6 address
 * @param[in] remote_port Port
 ******************************************************************************/
void sl_wisun_br_dhcpv6_server_on_recv(uint8_t *buffer, ssize_t length, in6_addr_t peer_address, in_port_t remote_port);

/***************************************************************************//**
 * Set vendor-specific data to be included in DHCPv6 Reply messages.
 *
 * @param[in] enterprise_number Vendor enterprise number
 * @param[in] vendor_data_length Length of vendor data
 * @param[in] vendor_data Vendor data
 * @return SL_STATUS_OK if successful, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_dhcpv6_set_vendor_data(uint32_t enterprise_number, uint16_t vendor_data_length, const uint8_t *vendor_data);

/***************************************************************************//**
 * Stop DHCPv6 Server.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_dhcpv6_server_stop(void);

#endif  /* __SL_WISUN_BR_DHCPV6_SERVER_H__ */
