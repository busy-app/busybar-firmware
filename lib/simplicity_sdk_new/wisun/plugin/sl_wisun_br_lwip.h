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

#ifndef __SL_WISUN_BR_LWIP_H__
#define __SL_WISUN_BR_LWIP_H__

#include <stdbool.h>
#include "sl_status.h"

typedef struct ip6_addr ip6_addr_t;

/***************************************************************************//**
 * Hook for lwIP to get the appropriate network interface to transmit one packet.
 * @param[in] src Source IPv6 address
 * @param[in] dest Destination IPv6 address
 * @return The destination network interface, or NULL if not found
 ******************************************************************************/
struct netif *sl_wisun_br_lwip_ip6_route_hook(const ip6_addr_t *src, const ip6_addr_t *dest);

/***************************************************************************//**
 * Called for an IPv6 packet from WAN network interface.
 *
 * @param[in] data IPv6 packet data
 * @param[out] data_length Length of IPv6 packet data in bytes
 * @return SL_STATUS_OK if successful, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_lwip_wan_input(const uint8_t *data, size_t data_length);

/***************************************************************************//**
 * Handler called for an IPv6 packet for WAN network interface.
 *
 * @param[in] data IPv6 packet data
 * @param[out] data_length Length of IPv6 packet data in bytes
 * @return SL_STATUS_OK if successful, an error code otherwise
 ******************************************************************************/
typedef sl_status_t (*sl_wisun_br_lwip_wan_output_handler_t)(const uint8_t* data, size_t data_length);

/***************************************************************************//**
 * Register a function to handle IPv6 packets for WAN interface network.
 *
 * @param[in] handler Handler function
 * @return SL_STATUS_OK if successful, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_lwip_set_wan_output_handler(sl_wisun_br_lwip_wan_output_handler_t handler);

/***************************************************************************//**
 * Get WAN interface's IPv6 Global Unicast Address (or Unique Local Address).
 *
 * @param[in] ipv6_address Pointer to IPv6 address
 * @return SL_STATUS_OK if successful, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_lwip_get_wan_address(uint8_t *ipv6_address);

/***************************************************************************//**
 * Handler called on WAN interface's link state changes.
 *
 * @param[in] link_up true if WAN interface is ready to exchange IPv6 packets
 ******************************************************************************/
typedef void (*sl_wisun_br_lwip_wan_link_state_handler_t)(bool link_up);

/***************************************************************************//**
 * Register a function to handle WAN interface's link state changes.
 *
 * @param[in] handler Handler function
 * @return SL_STATUS_OK if successful, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_lwip_set_wan_link_state_handler(sl_wisun_br_lwip_wan_link_state_handler_t handler);

/***************************************************************************//**
 * Bring WAN network interface up, available for processing traffic.
 *
 * @param[in] eui48 Link level hardware address
 ******************************************************************************/
void sl_wisun_br_lwip_wan_up(const uint8_t *eui48);

/***************************************************************************//**
 * Bring WAN network interface down, disabling any traffic processing.
 ******************************************************************************/
void sl_wisun_br_lwip_wan_down(void);

/***************************************************************************//**
 * Called for an IPv6 packet from PAN network interface.
 *
 * @param[in] data IPv6 packet data
 * @param[out] data_length Length of IPv6 packet data in bytes
 * @return SL_STATUS_OK if successful, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_lwip_pan_input(const uint8_t* data, size_t data_length);

/***************************************************************************//**
 * Get PAN interface's IPv6 Global Unicast Address (or Unique Local Address).
 *
 * @param[in] ipv6_address Pointer to IPv6 address
 * @return SL_STATUS_OK if successful, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_lwip_pan_get_address(uint8_t *ipv6_address);

/***************************************************************************//**
 * Bring PAN network interface up, available for processing traffic.
 *
 * @param[in] ll_addr Link-Local address
 * @param[in] gua_addr Global Unicast Address
 ******************************************************************************/
void sl_wisun_br_lwip_pan_up(const uint8_t *ll_addr, const uint8_t *gua_addr);

/***************************************************************************//**
 * Bring PAN network interface down, disabling any traffic processing.
 ******************************************************************************/
void sl_wisun_br_lwip_pan_down(void);

/***************************************************************************//**
 * Create and initialize PAN and WAN network interfaces.
 ******************************************************************************/
void sl_wisun_br_lwip_init(void);

#endif  /* __SL_WISUN_BR_LWIP_H__ */
