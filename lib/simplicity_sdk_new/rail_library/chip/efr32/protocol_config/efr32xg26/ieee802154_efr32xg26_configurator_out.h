/***************************************************************************//**
 * @brief RAIL Configuration
 * @details
 *   WARNING: Auto-Generated Radio Config Header  -  DO NOT EDIT
 *   Radio Configurator Version: 2504.4.1 (2ab5c852a7)
 *   RAIL Adapter Version: 2.4.33 (330b556b3a)
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

#ifndef __IEEE802154_EFR32XG26_CONFIGURATOR_OUT_H__
#define __IEEE802154_EFR32XG26_CONFIGURATOR_OUT_H__

#include <stdint.h>
#include "rail_types.h"
#include "timing_state.h"

#define IEEE802154_ACCELERATION_BUFFER ieee802154AccelerationBuffer
extern uint32_t ieee802154AccelerationBuffer[];

extern const uint32_t ieee802154_2p4_2mbps_modemConfigBase[];
extern const uint32_t ieee802154_2p4_rx_channel_switching_modemConfigBase[];
extern const uint32_t ieee802154_2p4_antdiv_modemConfigBase[];
extern const uint32_t ieee802154_2p4_antdiv_fem_modemConfigBase[];
extern const uint32_t ieee802154_2p4_coherent_modemConfigBase[];
extern const uint32_t ieee802154_2p4_fem_modemConfigBase[];

extern const uint32_t ieee802154_2p4_dual_sync_modemConfig[];
extern const uint32_t ieee802154_2p4_high_speed_2mbps_coherent_modemConfig[];
extern const uint32_t ieee802154_2p4_high_speed_1mbps_fec_modemConfig[];
extern const uint32_t ieee802154_2p4_dual_sync_fcs_modemConfig[];

#define RAIL0_IEEE802154_2P4_2MBPS_PHY_SIGNIFY_SUN_FSK_2MBPS_500KHZ
#define RAIL0_IEEE802154_2P4_2MBPS_PROFILE_BASE
extern const RAIL_ChannelConfig_t ieee802154_2p4_2mbps_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_2p4_2mbps_channels[];

#define RAIL0_IEEE802154_2P4_1MBPS_FEC_PHY_SIGNIFY_SUN_FSK_1MBPS_500KHZ
#define RAIL0_IEEE802154_2P4_1MBPS_FEC_PROFILE_BASE
extern const RAIL_ChannelConfig_t ieee802154_2p4_1mbps_fec_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_2p4_1mbps_fec_channels[];

#define RAIL0_IEEE802154_2P4_FCS_2MBPS_PHY_SIGNIFY_SUN_FSK_2MBPS_500KHZ
#define RAIL0_IEEE802154_2P4_FCS_2MBPS_PROFILE_BASE
extern const RAIL_ChannelConfig_t ieee802154_2p4_fcs_2mbps_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_2p4_fcs_2mbps_channels[];

#define RAIL0_IEEE802154_2P4_FCS_1MBPS_FEC_PHY_SIGNIFY_SUN_FSK_1MBPS_500KHZ
#define RAIL0_IEEE802154_2P4_FCS_1MBPS_FEC_PROFILE_BASE
extern const RAIL_ChannelConfig_t ieee802154_2p4_fcs_1mbps_fec_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_2p4_fcs_1mbps_fec_channels[];

#define RAIL0_IEEE802154_2P4_RX_CHANNEL_SWITCHING_PHY_IEEE802154_2P4GHZ_ANTDIV_FASTSWITCH
#define RAIL0_IEEE802154_2P4_RX_CHANNEL_SWITCHING_PROFILE_BASE
extern const RAIL_ChannelConfig_t ieee802154_2p4_rx_channel_switching_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_2p4_rx_channel_switching_channels[];

#define RAIL0_IEEE802154_2P4_ANTDIV_PHY_IEEE802154_2P4GHZ_DIVERSITY_PROD
#define RAIL0_IEEE802154_2P4_ANTDIV_PROFILE_IEEE802154OQPSK
extern const RAIL_ChannelConfig_t ieee802154_2p4_antdiv_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_2p4_antdiv_channels[];

#define RAIL0_IEEE802154_2P4_ANTDIV_FEM_PHY_IEEE802154_2P4GHZ_DIVERSITY_FEM_PROD
#define RAIL0_IEEE802154_2P4_ANTDIV_FEM_PROFILE_IEEE802154OQPSK
extern const RAIL_ChannelConfig_t ieee802154_2p4_antdiv_fem_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_2p4_antdiv_fem_channels[];

#define RAIL0_IEEE802154_2P4_COHERENT_PHY_IEEE802154_2P4GHZ_PROD
#define RAIL0_IEEE802154_2P4_COHERENT_PROFILE_IEEE802154OQPSK
extern const RAIL_ChannelConfig_t ieee802154_2p4_coherent_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_2p4_coherent_channels[];

#define RAIL0_IEEE802154_2P4_FEM_PHY_IEEE802154_2P4GHZ_FEM_PROD
#define RAIL0_IEEE802154_2P4_FEM_PROFILE_IEEE802154OQPSK
extern const RAIL_ChannelConfig_t ieee802154_2p4_fem_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_2p4_fem_channels[];

#endif // __IEEE802154_EFR32XG26_CONFIGURATOR_OUT_H__
