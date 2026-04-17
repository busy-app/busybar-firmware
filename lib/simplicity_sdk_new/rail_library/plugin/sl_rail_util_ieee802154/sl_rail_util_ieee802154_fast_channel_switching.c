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

#include "sl_rail_util_ieee802154_fast_channel_switching.h"

sl_rail_status_t sl_rail_ieee802154_config_2p4_ghz_radio_fast_channel_switching(sl_rail_handle_t rail_handle)
{
#if SL_RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING && defined(_SILICON_LABS_32B_SERIES_3)
  return sl_rail_ieee802154_config_channels(rail_handle,
                                            (const sl_rail_channel_config_t*) sl_rail_ieee802154_phy_2p4_ghz_rx_ch_switching,
                                            SL_RAIL_IEEE802154_PHY_2P4_GHZ_RX_CH_SWITCHING);
#else
  (void) rail_handle;
  return SL_RAIL_STATUS_INVALID_CALL;
#endif
}

__WEAK sl_rail_ieee802154_phy_features_t sl_rail_util_ieee802154_get_fast_channel_switching_phy_features(void)
{
#if SL_RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING && defined(_SILICON_LABS_32B_SERIES_3)
  return SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_CH_SWITCHING;
#else
  return SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ;
#endif
}
