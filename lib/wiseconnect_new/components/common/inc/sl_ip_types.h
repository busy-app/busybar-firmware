/***************************************************************************/ /**
 * @file
 * @brief IP networking types
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
#include "stdint.h"

/**
 * @addtogroup SL_NET_CONSTANTS
 * @{
 */

/// Enumeration of IP version
typedef enum {
  SL_IPV4_VERSION = 4, ///< IPv4 version
  SL_IPV6_VERSION = 6  ///< IPv6 version
} sl_ip_version_t;

/// Enumeration of IP address types/// Enumeration of IP address types.
typedef enum {
  SL_IPV4            = (1 << 2),    ///< IPv4 address
  SL_IPV6            = (1 << 3),    ///< IPv6, Unspecified sub-type
  SL_IPV6_LINK_LOCAL = SL_IPV6 + 1, ///< IPv6, Link local address
  SL_IPV6_SITE_LOCAL = SL_IPV6 + 2, ///< IPv6, Site local address
  SL_IPV6_GLOBAL     = SL_IPV6 + 3, ///< IPv6, Global address
  SL_INVALID_IP      = 0            ///< This enumeration value represents an invalid IP address.
} sl_ip_address_type_t;

/// Enumeration of IP Management type
typedef enum {
  SL_IP_MANAGEMENT_STATIC_IP = 1, ///< Assign STATIC IP address to an interface
  SL_IP_MANAGEMENT_DHCP,          ///< Assign IP address to an interface dynamically using DHCP
  SL_IP_MANAGEMENT_LINK_LOCAL     ///< Assign IP address using link-local addressing
} sl_ip_management_t;

/** @} */

/** \addtogroup SL_NET_TYPES */
/** @{ */

/// IPv4 address object
typedef union {
  uint32_t value;   ///< IPv4 address as a uint32_t
  uint8_t bytes[4]; ///< IPv4 address as uint8_t[4]
} sl_ipv4_address_t;

/// IPv6 address object
typedef union {
  uint32_t value[4]; ///< IPv6 address as a uint32_t[4]
  uint8_t bytes[16]; ///< IPv6 address as uint8_t[16]
} sl_ipv6_address_t;

/// Generic IP Address Structure. Supports both IPv4 and IPv6 addresses
#pragma pack(1)
typedef struct {
  /// IP address object
  union {
    sl_ipv4_address_t v4; ///< IPv4 address
    sl_ipv6_address_t v6; ///< IPv6 address
  } ip;

  sl_ip_address_type_t type; ///< IP address type
} sl_ip_address_t;
#pragma pack()

/** @} */

/// Macro to assist initializing an IPv4 address
#define SL_IPV4_ADDRESS(a, b, c, d)                \
  {                                                \
    .ip.v4.bytes = { a, b, c, d }, .type = SL_IPV4 \
  }
