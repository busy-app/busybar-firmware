/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_RAIL_UTIL_IEEE802154_PHY_SELECT_H
#define SL_RAIL_UTIL_IEEE802154_PHY_SELECT_H

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#include "sl_rail_types.h"
#include "sl_rail_ieee802154.h"
#include "rail_types.h"
#include "sl_rail_util_ieee802154_stack_event.h"

// Backwards compatibility macros
#define sl_rail_util_radio_config_t \
  sl_rail_util_ieee802154_radio_config_t
#define SL_RAIL_UTIL_RADIO_CONFIG_154_2P4_DEFAULT \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ
#define SL_RAIL_UTIL_RADIO_CONFIG_154_2P4_ANTDIV \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_ANT_DIV
#define SL_RAIL_UTIL_RADIO_CONFIG_154_2P4_COEX \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_COEX
#define SL_RAIL_UTIL_RADIO_CONFIG_154_2P4_ANTDIV_COEX \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_ANT_DIV_COEX
#define SL_RAIL_UTIL_RADIO_CONFIG_154_2P4_FEM \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM
#define SL_RAIL_UTIL_RADIO_CONFIG_154_2P4_ANTDIV_FEM \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_ANT_DIV
#define SL_RAIL_UTIL_RADIO_CONFIG_154_2P4_COEX_FEM \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_COEX
#define SL_RAIL_UTIL_RADIO_CONFIG_154_2P4_ANTDIV_COEX_FEM \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_ANT_DIV_COEX
#define sl_rail_util_get_active_radio_config \
  sl_rail_util_ieee802154_get_active_radio_config
#define sl_rail_util_plugin_config_2p4ghz_radio \
  sl_rail_util_ieee802154_config_radio
#define sl_rail_util_ieee802154_radio_config_t \
  sl_rail_ieee802154_phy_t
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_DEFAULT \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_ANT_DIV
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_COEX \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_COEX
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_COEX \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_ANT_DIV_COEX
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_FEM \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_FEM \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_ANT_DIV
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_COEX_FEM \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_COEX
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_COEX_FEM \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_ANT_DIV_COEX
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_2MBPS \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_2_MBPS
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_1MBPS_FEC \
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_1_MBPS_FEC

#ifdef  SL_CATALOG_SL_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT
#define SL_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_ENABLED    \
  (SL_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_DEFAULT_ENABLED \
   | SL_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_RUNTIME_ENABLED)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup IEEE802154_Phy_Select IEEE 802.15.4 Phy Select
 * @brief APIs for selecting the PHY configuration for IEEE 802.15.4.
 *
 * The Phy Select component provides an abstraction for selecting and
 * configuring the radio configuration used by the IEEE 802.15.4 stack.
 * It enables applications to choose among multiple supported PHY
 * configurations (such as default, antenna diversity, coexistence, FEM,
 * and high-speed PHYs) at build time or runtime, depending on the UC
 * component settings.
 *
 * The component exposes APIs to query and set the active PHY, and a
 * table of supported PHYs, each associated with a set of features and a
 * configuration callback. Each component that contributes to this Phy
 * selection logic (such as \ref sl_rail_util_ieee802154_fast_channel_switching)
 * must implement the necessary APIs to interact with the PHY selection logic.
 * @{
 */

/**
 * PHY select contribution for IEEE 802.15.4 stack event handler
 *
 * @param[in] stack_event event to handle
 * @param[in] supplement optional event information
 * @return Status code indicating success of the function call.
 */
sl_rail_util_ieee802154_stack_status_t sl_rail_util_ieee802154_phy_select_on_event(
  sl_rail_util_ieee802154_stack_event_t stack_event,
  uint32_t supplement);

/**
 * Get the active IEEE 802.15.4 2.4Ghz radio configuration.
 *
 * @return Active IEEE 802.15.4 2.4Ghz radio configuration
 */
sl_rail_util_radio_config_t sl_rail_util_ieee802154_get_active_radio_config(void);

/**
 * Configure IEEE 802.15.4 2.4Ghz radio configuration.
 *
 * @param[in] railHandle A RAIL instance handle.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_util_ieee802154_config_radio(sl_rail_handle_t railHandle);
/**
 * @}
 * end of IEEE802154_PHY_SELECT_API
 */

#ifdef __cplusplus
}
#endif

#endif // SL_RAIL_UTIL_IEEE802154_PHY_SELECT_H
