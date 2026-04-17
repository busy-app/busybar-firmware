/***************************************************************************//**
 * @file
 * @brief IEEE802.15.4 high speed PHY configuration file for railtest.
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

#ifndef SL_RAIL_UTIL_IEEE802154_HIGH_SPEED_PHY_CONFIG_H
#define SL_RAIL_UTIL_IEEE802154_HIGH_SPEED_PHY_CONFIG_H

// RAIL test applications enable both high speed PHYs for testing purposes
// This overrides the default configuration in plugin/rail_util_ieee802154/config/sl_rail_util_ieee802154_high_speed_phy_config.h

// Enable 1 Mbps FEC PHY support for railtest
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_1MBPS_FEC_SUPPORTED 1

// Enable 2 Mbps PHY support for railtest
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_2MBPS_SUPPORTED 1

#endif //SL_RAIL_UTIL_IEEE802154_HIGH_SPEED_PHY_CONFIG_H
