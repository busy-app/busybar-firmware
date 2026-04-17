/***************************************************************************//**
 * @file
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

#ifndef SL_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_H
#define SL_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_H

#include "sl_rail_ieee802154.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup IEEE802154_FAST_CHANNEL_SWITCHING_API IEEE802.15.4 Fast Channel Switching
 * @{
 */

/**
 * Get PHY features selected by the fast channel switching component.
 *
 * @note RAIL provides a weak implementation; users can override it with their own.
 *
 * @note This function is only available for Series 3 parts that support
 *   \ref SL_RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING or the runtime call
 *   \ref sl_rail_ieee802154_supports_rx_channel_switching().
 *
 * @return PHY features selected features.
 */
sl_rail_ieee802154_phy_features_t sl_rail_util_ieee802154_get_fast_channel_switching_phy_features(void);

/**
 * Configure the 2.4 GHz IEEE 802.15.4 radio with fast channel switching support.
 * @param[in] rail_handle A handle for the RAIL instance.
 *
 * @return Status code indicating the result of the operation.
 *
 * @note This function is only available for Series 3 parts that support
 *   \ref SL_RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING or the runtime call
 *   \ref sl_rail_ieee802154_supports_rx_channel_switching().
 *
 * @note \ref sl_rail_ieee802154_config_rx_channel_switching() must be called
 *   after configuring the radio with
 *   \ref sl_rail_ieee802154_config_2p4_ghz_radio_fast_channel_switching().
 */
sl_rail_status_t sl_rail_ieee802154_config_2p4_ghz_radio_fast_channel_switching(sl_rail_handle_t rail_handle);

/**
 * @}
 * end of IEEE802154_FAST_CHANNEL_SWITCHING_API
 */

#ifdef __cplusplus
}
#endif

#endif // SL_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_H
