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

#ifndef SL_RAIL_UTIL_IEEE802154_RX_DUTY_CYCLING_H
#define SL_RAIL_UTIL_IEEE802154_RX_DUTY_CYCLING_H

#include "sl_rail_ieee802154.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup IEEE802154_RX_DUTY_CYCLING_API IEEE802.15.4 Duty Cycling
 * @{
 */

/**
 * Get PHY features selected by the duty cycling component.
 *
 * @note RAIL provides a weak implementation; users can override it with their own.
 *
 * @note This function is only available for Series 3 parts that support
 *   \ref SL_RAIL_SUPPORTS_RX_DUTY_CYCLING.
 *
 * @return Selected PHY features.
 */
sl_rail_ieee802154_phy_features_t sl_rail_util_ieee802154_get_rx_duty_cycling_phy_features(void);

/**
 * Configure the 2.4 GHz IEEE 802.15.4 radio with Rx duty cycling support.
 * @param[in] rail_handle A handle for the RAIL instance.
 *
 * After this function is called, the radio will begin duty cycling each time
 * it enters RX.
 *
 * @note This function is only available for Series 3 parts that support
 *   \ref SL_RAIL_SUPPORTS_RX_DUTY_CYCLING.
 *
 * @return Status code indicating the result of the operation.
 */
sl_rail_status_t sl_rail_ieee802154_config_2p4_ghz_radio_rx_duty_cycling(sl_rail_handle_t rail_handle);

/**
 * @}
 * end of IEEE802154_RX_DUTY_CYCLING_API
 */
#ifdef __cplusplus
}
#endif

#endif // SL_RAIL_UTIL_IEEE802154_RX_DUTY_CYCLING_H
