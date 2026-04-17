/***************************************************************************//**
 * @file
 * @brief sl_rail_sdk_util_802154_init_config.h
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_RAIL_SDK_UTIL_802154_INIT_CONFIG_H
#define SL_RAIL_SDK_UTIL_802154_INIT_CONFIG_H

#include "sl_rail_sdk_util_802154_protocol_types.h"

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Available IEEE 802.15.4 standards
// <o SL_RAIL_SDK_UTIL_INIT_PROTOCOL_INSTANCE_DEFAULT> Default Radio Configuration
// <SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_2P4GHZ=> IEEE 802.15.4 2.4GHz
// <SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ANTDIV=> IEEE 802.15.4 2.4GHz, antenna diversity
// <SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_2P4GHZ_COEX=> IEEE 802.15.4 2.4GHz, coexistence
// <SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ANTDIV_COEX=> IEEE 802.15.4 2.4GHz, antenna diversity, coexistence
// <SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_GB868_915MHZ=> IEEE 802.15.4 GB868 915MHz
// <i> Default: SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_2P4GHZ
#define SL_RAIL_SDK_UTIL_INIT_PROTOCOL_INSTANCE_DEFAULT  SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_2P4GHZ
// </h>

// <<< end of configuration section >>>

#endif // SL_RAIL_SDK_UTIL_802154_INIT_CONFIG_H
