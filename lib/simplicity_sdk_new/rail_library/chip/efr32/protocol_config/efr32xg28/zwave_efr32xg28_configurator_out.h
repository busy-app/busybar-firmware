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

#ifndef __ZWAVE_EFR32XG28_CONFIGURATOR_OUT_H__
#define __ZWAVE_EFR32XG28_CONFIGURATOR_OUT_H__

#include <stdint.h>
#include "rail_types.h"
#include "timing_state.h"

#define ZWAVE_ACCELERATION_BUFFER zwaveAccelerationBuffer
extern uint32_t zwaveAccelerationBuffer[];

extern const uint32_t zwave_40kb_modemConfigBase[];

extern const uint32_t zwave_40kb_0_modemConfig[];
extern const uint32_t zwave_9p6kb_0_modemConfig[];
extern const uint32_t zwave_9p6kb_conc_0_modemConfig[];
extern const uint32_t zwave_100kb_0_modemConfig[];
extern const uint32_t zwave_lr_0_modemConfig[];
extern const uint32_t zwave_100kb_lowside_0_modemConfig[];
extern const uint32_t zwave_100kb_beam_0_modemConfig[];
extern const uint32_t zwave_100kb_beam_lowside_0_modemConfig[];
extern const uint32_t zwave_40kb_beam_0_modemConfig[];
extern const uint32_t zwave_lr_beam_0_modemConfig[];

#define RAIL0_ZWAVE_40KB_PHY_ZWAVE_40KBPS_9P6KBPS_908MHZ_VITERBI_CONC
#define RAIL0_ZWAVE_40KB_PROFILE_BASE
extern const RAIL_ChannelConfig_t zwave_40kb_channelConfig;
extern const RAIL_ChannelConfigEntry_t zwave_40kb_channels[];

#define RAIL0_ZWAVE_9P6KB_PHY_ZWAVE_9P6KBPS_908MHZ_BCR_CONC
#define RAIL0_ZWAVE_9P6KB_PROFILE_BASE
extern const RAIL_ChannelConfig_t zwave_9p6kb_channelConfig;
extern const RAIL_ChannelConfigEntry_t zwave_9p6kb_channels[];

#define RAIL0_ZWAVE_9P6KB_CONC_PHY_ZWAVE_9P6KBPS_908MHZ_BCR_CONC
#define RAIL0_ZWAVE_9P6KB_CONC_PROFILE_BASE
extern const RAIL_ChannelConfig_t zwave_9p6kb_conc_channelConfig;
extern const RAIL_ChannelConfigEntry_t zwave_9p6kb_conc_channels[];

#define RAIL0_ZWAVE_100KB_PHY_ZWAVE_100KBPS_916MHZ_VITERBI
#define RAIL0_ZWAVE_100KB_PROFILE_BASE
extern const RAIL_ChannelConfig_t zwave_100kb_channelConfig;
extern const RAIL_ChannelConfigEntry_t zwave_100kb_channels[];

#define RAIL0_ZWAVE_LR_PHY_ZWAVE_100KBPS_912MHZ_OQPSK
#define RAIL0_ZWAVE_LR_PROFILE_BASE
extern const RAIL_ChannelConfig_t zwave_lr_channelConfig;
extern const RAIL_ChannelConfigEntry_t zwave_lr_channels[];

#define RAIL0_ZWAVE_100KB_LOWSIDE_PHY_ZWAVE_100KBPS_916MHZ_VITERBI_LOWSIDE
#define RAIL0_ZWAVE_100KB_LOWSIDE_PROFILE_BASE
extern const RAIL_ChannelConfig_t zwave_100kb_lowside_channelConfig;
extern const RAIL_ChannelConfigEntry_t zwave_100kb_lowside_channels[];

#define RAIL0_ZWAVE_100KB_BEAM_PHY_ZWAVE_100KBPS_916MHZ_VITERBI_BEAM
#define RAIL0_ZWAVE_100KB_BEAM_PROFILE_BASE
extern const RAIL_ChannelConfig_t zwave_100kb_beam_channelConfig;
extern const RAIL_ChannelConfigEntry_t zwave_100kb_beam_channels[];

#define RAIL0_ZWAVE_100KB_BEAM_LOWSIDE_PHY_ZWAVE_100KBPS_916MHZ_VITERBI_LOWSIDE_BEAM
#define RAIL0_ZWAVE_100KB_BEAM_LOWSIDE_PROFILE_BASE
extern const RAIL_ChannelConfig_t zwave_100kb_beam_lowside_channelConfig;
extern const RAIL_ChannelConfigEntry_t zwave_100kb_beam_lowside_channels[];

#define RAIL0_ZWAVE_40KB_BEAM_PHY_ZWAVE_40KBPS_908MHZ_VITERBI_BEAM
#define RAIL0_ZWAVE_40KB_BEAM_PROFILE_BASE
extern const RAIL_ChannelConfig_t zwave_40kb_beam_channelConfig;
extern const RAIL_ChannelConfigEntry_t zwave_40kb_beam_channels[];

#define RAIL0_ZWAVE_LR_BEAM_PHY_ZWAVE_100KBPS_912MHZ_OQPSK_BEAM
#define RAIL0_ZWAVE_LR_BEAM_PROFILE_BASE
extern const RAIL_ChannelConfig_t zwave_lr_beam_channelConfig;
extern const RAIL_ChannelConfigEntry_t zwave_lr_beam_channels[];

#endif // __ZWAVE_EFR32XG28_CONFIGURATOR_OUT_H__
