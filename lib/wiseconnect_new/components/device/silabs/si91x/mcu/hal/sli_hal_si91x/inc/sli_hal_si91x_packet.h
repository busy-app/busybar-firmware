/***************************************************************************/ /**
 * @file sli_hal_si91x_packet.h
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_HAL_SI9X_PACKET_H
#define SLI_HAL_SI9X_PACKET_H

#include "sli_routing_utility.h"

typedef enum {
  SLI_HAL_SI91X_PACKET,    ///< Packet type for HAL to send packet to Command Engine
  SLI_HAL_SI91X_PACKET_MAX ///< Max packet type
} sli_hal_si91x_packet_t;

extern sli_routing_table_t hal_si91x_routing_table;
#endif