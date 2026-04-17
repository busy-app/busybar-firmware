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

#ifndef __IEEE802154_EFR32XG23_CONFIGURATOR_OUT_H__
#define __IEEE802154_EFR32XG23_CONFIGURATOR_OUT_H__

#include <stdint.h>
#include "rail_types.h"
#include "timing_state.h"

#define IEEE802154_ACCELERATION_BUFFER ieee802154AccelerationBuffer
extern uint32_t ieee802154AccelerationBuffer[];

extern const uint32_t ieee802154_gb868_863_modemConfigBase[];
extern const uint32_t ieee802154_gb868_915_modemConfigBase[];

#define RAIL0_IEEE802154_GB868_863_PHY_IEEE802154_GB868_863MHZ_PHR2
#define RAIL0_IEEE802154_GB868_863_PROFILE_BASE
extern const RAIL_ChannelConfig_t ieee802154_gb868_863_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_gb868_863_channels[];

#define RAIL0_IEEE802154_GB868_915_PHY_IEEE802154_GB868_915MHZ_PHR2
#define RAIL0_IEEE802154_GB868_915_PROFILE_BASE
extern const RAIL_ChannelConfig_t ieee802154_gb868_915_channelConfig;
extern const RAIL_ChannelConfigEntry_t ieee802154_gb868_915_channels[];

#endif // __IEEE802154_EFR32XG23_CONFIGURATOR_OUT_H__
