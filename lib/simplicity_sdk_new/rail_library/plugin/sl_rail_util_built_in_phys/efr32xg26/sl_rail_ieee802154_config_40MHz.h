/***************************************************************************//**
 * @brief RAIL Configuration
 * @details
 *   WARNING: Auto-Generated Radio Config Header  -  DO NOT EDIT
 *   Radio Configurator Version: 2504.4.8
 *   RAIL Adapter Version: 2.4.33
 *   RAIL Compatibility: 2.x
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

#ifndef __SL_RAIL_IEEE802154_CONFIG_40MHZ_H__
#define __SL_RAIL_IEEE802154_CONFIG_40MHZ_H__

#include <stdint.h>
#include "rail_types.h"

extern const uint32_t sl_rail_ieee802154_phy_2G4Hz_2Mbps_40MHz_modemConfigBase[];
extern const uint32_t sl_rail_ieee802154_phy_2G4Hz_fcs_2Mbps_40MHz_modemConfigBase[];
extern const uint32_t sl_rail_ieee802154_phy_2G4Hz_40MHz_modemConfigBase[];
extern const uint32_t sl_rail_ieee802154_phy_2G4Hz_fast_switch_40MHz_modemConfigBase[];

extern const uint32_t sl_rail_ieee802154_2p4_dual_sync_40MHz_modemConfig[];
extern const uint32_t sl_rail_ieee802154_2p4_high_speed_2mbps_coherent_40MHz_modemConfig[];
extern const uint32_t sl_rail_ieee802154_2p4_high_speed_1mbps_fec_40MHz_modemConfig[];
extern const uint32_t sl_rail_ieee802154_2p4_dual_sync_fec_fcs_40MHz_modemConfig[];
extern const uint32_t sl_rail_ieee802154_2p4_high_speed_fcs_1mbps_fec_40MHz_modemConfig[];
extern const uint32_t sl_rail_ieee802154_2p4_dual_sync_fcs_40MHz_modemConfig[];
extern const uint32_t sl_rail_ieee802154_2p4_high_speed_fcs_2mbps_40MHz_modemConfig[];
extern const uint32_t sl_rail_ieee802154_phy_2G4Hz_40MHz_modemConfig[];
extern const uint32_t sl_rail_ieee802154_phy_2G4Hz_antdiv_40MHz_modemConfig[];
extern const uint32_t sl_rail_ieee802154_phy_2G4Hz_antdiv_fem_40MHz_modemConfig[];
extern const uint32_t sl_rail_ieee802154_phy_2G4Hz_fem_40MHz_modemConfig[];

#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_2MBPS_40MHZ_PHY_SIGNIFY_SUN_FSK_2MBPS_500KHZ
#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_2MBPS_40MHZ_PROFILE_BASE
extern const RAIL_ChannelConfig_t sl_rail_ieee802154_phy_2G4Hz_2Mbps_40MHz_channelConfig;
extern const RAIL_ChannelConfigEntry_t sl_rail_ieee802154_phy_2G4Hz_2Mbps_40MHz_channels[];

#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_1MBPS_FEC_40MHZ_PHY_SIGNIFY_SUN_FSK_1MBPS_500KHZ
#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_1MBPS_FEC_40MHZ_PROFILE_BASE
extern const RAIL_ChannelConfig_t sl_rail_ieee802154_phy_2G4Hz_1Mbps_fec_40MHz_channelConfig;
extern const RAIL_ChannelConfigEntry_t sl_rail_ieee802154_phy_2G4Hz_1Mbps_fec_40MHz_channels[];

#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_FCS_2MBPS_40MHZ_PHY_SIGNIFY_SUN_FSK_2MBPS_500KHZ
#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_FCS_2MBPS_40MHZ_PROFILE_BASE
extern const RAIL_ChannelConfig_t sl_rail_ieee802154_phy_2G4Hz_fcs_2Mbps_40MHz_channelConfig;
extern const RAIL_ChannelConfigEntry_t sl_rail_ieee802154_phy_2G4Hz_fcs_2Mbps_40MHz_channels[];

#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_FCS_1MBPS_FEC_40MHZ_PHY_SIGNIFY_SUN_FSK_1MBPS_500KHZ
#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_FCS_1MBPS_FEC_40MHZ_PROFILE_BASE
extern const RAIL_ChannelConfig_t sl_rail_ieee802154_phy_2G4Hz_fcs_1Mbps_fec_40MHz_channelConfig;
extern const RAIL_ChannelConfigEntry_t sl_rail_ieee802154_phy_2G4Hz_fcs_1Mbps_fec_40MHz_channels[];

#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_40MHZ_PHY_IEEE802154_2P4GHZ_PROD
#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_40MHZ_PROFILE_IEEE802154OQPSK
extern const RAIL_ChannelConfig_t sl_rail_ieee802154_phy_2G4Hz_40MHz_channelConfig;
extern const RAIL_ChannelConfigEntry_t sl_rail_ieee802154_phy_2G4Hz_40MHz_channels[];

#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_FAST_SWITCH_40MHZ_PHY_IEEE802154_2P4GHZ_ANTDIV_FASTSWITCH
#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_FAST_SWITCH_40MHZ_PROFILE_BASE
extern const RAIL_ChannelConfig_t sl_rail_ieee802154_phy_2G4Hz_fast_switch_40MHz_channelConfig;
extern const RAIL_ChannelConfigEntry_t sl_rail_ieee802154_phy_2G4Hz_fast_switch_40MHz_channels[];

#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_ANTDIV_40MHZ_PHY_IEEE802154_2P4GHZ_DIVERSITY_PROD
#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_ANTDIV_40MHZ_PROFILE_IEEE802154OQPSK
extern const RAIL_ChannelConfig_t sl_rail_ieee802154_phy_2G4Hz_antdiv_40MHz_channelConfig;
extern const RAIL_ChannelConfigEntry_t sl_rail_ieee802154_phy_2G4Hz_antdiv_40MHz_channels[];

#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_ANTDIV_FEM_40MHZ_PHY_IEEE802154_2P4GHZ_DIVERSITY_FEM_PROD
#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_ANTDIV_FEM_40MHZ_PROFILE_IEEE802154OQPSK
extern const RAIL_ChannelConfig_t sl_rail_ieee802154_phy_2G4Hz_antdiv_fem_40MHz_channelConfig;
extern const RAIL_ChannelConfigEntry_t sl_rail_ieee802154_phy_2G4Hz_antdiv_fem_40MHz_channels[];

#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_FEM_40MHZ_PHY_IEEE802154_2P4GHZ_FEM_PROD
#define RAIL0_SL_RAIL_IEEE802154_PHY_2G4HZ_FEM_40MHZ_PROFILE_IEEE802154OQPSK
extern const RAIL_ChannelConfig_t sl_rail_ieee802154_phy_2G4Hz_fem_40MHz_channelConfig;
extern const RAIL_ChannelConfigEntry_t sl_rail_ieee802154_phy_2G4Hz_fem_40MHz_channels[];

#endif // __SL_RAIL_IEEE802154_CONFIG_40MHZ_H__
