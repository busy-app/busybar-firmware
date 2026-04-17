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

#ifndef __IEEE802154_EFR32XG2D_CONFIGURATOR_OUT_H__
#define __IEEE802154_EFR32XG2D_CONFIGURATOR_OUT_H__

#include <stdint.h>
#include "rail_types.h"
#include "timing_state.h"

#define IEEE802154_ACCELERATION_BUFFER ieee802154AccelerationBuffer
extern uint32_t ieee802154AccelerationBuffer[];

extern const uint32_t ieee802154_fsk169_2p4_modemConfigBase[];
extern const uint32_t ieee802154_fsk169_4p8_modemConfigBase[];
extern const uint32_t ieee802154_fsk169_9p6_modemConfigBase[];
extern const uint32_t ieee802154_fsk896_40_modemConfigBase[];
extern const uint32_t ieee802154_fsk920_400_modemConfigBase[];

#define RAIL0_IEEE802154_FSK169_2P4_PHY_IEEE802154_SUN_FSK_169MHZ_2FSK_2P4KBPS
#define RAIL0_IEEE802154_FSK169_2P4_PROFILE_BASE
extern const RAIL_ChannelConfig_t ieee802154_fsk169_2p4_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_fsk169_2p4_channels[];

#define RAIL0_IEEE802154_FSK169_4P8_PHY_IEEE802154_SUN_FSK_169MHZ_2FSK_4P8KBPS
#define RAIL0_IEEE802154_FSK169_4P8_PROFILE_BASE
extern const RAIL_ChannelConfig_t ieee802154_fsk169_4p8_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_fsk169_4p8_channels[];

#define RAIL0_IEEE802154_FSK169_9P6_PHY_IEEE802154_SUN_FSK_169MHZ_4FSK_9P6KBPS
#define RAIL0_IEEE802154_FSK169_9P6_PROFILE_BASE
extern const RAIL_ChannelConfig_t ieee802154_fsk169_9p6_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_fsk169_9p6_channels[];

#define RAIL0_IEEE802154_FSK896_40_PHY_IEEE802154_SUN_FSK_896MHZ_2FSK_40KBPS
#define RAIL0_IEEE802154_FSK896_40_PROFILE_BASE
extern const RAIL_ChannelConfig_t ieee802154_fsk896_40_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_fsk896_40_channels[];

#define RAIL0_IEEE802154_FSK920_400_PHY_IEEE802154_SUN_FSK_920MHZ_4FSK_400KBPS
#define RAIL0_IEEE802154_FSK920_400_PROFILE_BASE
extern const RAIL_ChannelConfig_t ieee802154_fsk920_400_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_fsk920_400_channels[];

#endif // __IEEE802154_EFR32XG2D_CONFIGURATOR_OUT_H__
