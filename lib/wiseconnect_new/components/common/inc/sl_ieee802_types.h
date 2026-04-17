/***************************************************************************/ /**
 * @file
 * @brief Common IEEE 802 types
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

/** \addtogroup SL_NET_TYPES */
/** @{ */

/// The IEEE defined 48-bit extended unique identifier (EUI-48)
typedef struct {
  uint8_t octet[6]; ///< Value of EUI-48 address
} sl_eui48_address_t;

/// The IEEE defined 64-bit extended unique identifier (EUI-64)
typedef struct {
  uint8_t octet[8]; ///< Value of EUI-64 address
} sl_eui64_address_t;

/// Generic SL MAC address type
typedef sl_eui48_address_t sl_mac_address_t;

/// Ethernet MAC address
typedef sl_eui48_address_t sl_ethernet_address_t;

/** @} */
