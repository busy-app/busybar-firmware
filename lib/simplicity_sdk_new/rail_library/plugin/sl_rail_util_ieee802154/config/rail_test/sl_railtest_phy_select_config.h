/***************************************************************************//**
 * @file
 * @brief Railtest PHY selection configuration for testing purposes.
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

#ifndef SL_RAILTEST_PHY_SELECT_CONFIG_H
#define SL_RAILTEST_PHY_SELECT_CONFIG_H

#include "sl_rail_ieee802154.h"

// Railtest runtime PHY features - enable all supported PHYs for testing
// Includes: standard, 2MBPS, 1MBPS_FEC, FCS variants, FEM, ANT_DIV, COEX
#define SL_RAILTEST_RUNTIME_PHY_FEATURES_BASE (             \
    SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ                  \
    | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FEM            \
    | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_ANT_DIV        \
    | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_COEX           \
    | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_2_MBPS         \
    | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_1_MBPS_FEC     \
    | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FCS_2_MBPS     \
    | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FCS_1_MBPS_FEC \
    )

// Default to standard PHY (feature 0x0)
#define SL_RAILTEST_DEFAULT_PHY_FEATURES_BASE SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ

// Forward declaration of railtest base PHY features callback
sl_rail_ieee802154_phy_features_t sl_railtest_get_base_phy_features(void);

#endif // SL_RAILTEST_PHY_SELECT_CONFIG_H
