/***************************************************************************//**
 * @file sl_wisun_br_lwip.h
 * @brief Integrates lwIP to Wi-SUN Border Router
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

#ifndef __SL_WISUN_BR_DHCPV6_RELAY_H__
#define __SL_WISUN_BR_DHCPV6_RELAY_H__

#include "sl_status.h"

#define DHCPV6_SERVER_PORT 547
#define DHCPV6_CLIENT_PORT 546

/***************************************************************************//**
 * Initialize DHCPv6 Relay.
 * This function must be called before any other DHCPv6 Relay function.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_dhcpv6_relay_init(void);

/***************************************************************************//**
 * Start DHCPv6 Relay.
 *
 * @param[in] socket Socket open and bound to DHCPv6 Server port on PAN interface.
 * It will be used to relay the replies from the external DHCPv6 Server.
 * @param[in] server_addr External DHCPv6 Server's IPv6 address
 * @return SL_STATUS_OK if successful, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_dhcpv6_relay_start(int socket, const uint8_t *server_addr);

/***************************************************************************//**
 * Handle DHCPv6 relay-forward or solicit messages received from the socket open
 * on PAN interface.
 *
 * @param[in] buffer Message's buffer
 * @param[in] length Message length
 * @param[in] peer_address Peer's IPv6 address
 * @param[in] remote_port Port
 ******************************************************************************/
void sl_wisun_br_dhcpv6_relay_pan_on_recv(uint8_t *buffer, ssize_t length, in6_addr_t peer_address, in_port_t remote_port);

/***************************************************************************//**
 * Stop DHCPv6 Relay.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_dhcpv6_relay_stop(void);

#endif  /* __SL_WISUN_BR_DHCPV6_RELAY_H__ */
