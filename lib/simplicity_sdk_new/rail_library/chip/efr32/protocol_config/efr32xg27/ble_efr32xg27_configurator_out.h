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

#ifndef __BLE_EFR32XG27_CONFIGURATOR_OUT_H__
#define __BLE_EFR32XG27_CONFIGURATOR_OUT_H__

#include <stdint.h>
#include "rail_types.h"
#include "timing_state.h"

#define BLE_ACCELERATION_BUFFER bleAccelerationBuffer
extern uint32_t bleAccelerationBuffer[];

extern const uint32_t ble_viterbi1M_modemConfigBase[];
extern const uint32_t quuppa_viterbi1M_modemConfigBase[];

extern const uint32_t ble_viterbi1M_modemConfig[];
extern const uint32_t ble_viterbi2M_0_34_modemConfig[];
extern const uint32_t ble_LR_DSA_125kb_modemConfig[];
extern const uint32_t ble_LR_DSA_500kb_modemConfig[];
extern const uint32_t ble_LR_DSA_1Mb_modemConfig[];
extern const uint32_t quuppa_viterbi1M_0_modemConfig[];
extern const uint32_t quuppa_viterbi1M_1_modemConfig[];
extern const uint32_t quuppa_viterbi1M_2_modemConfig[];
extern const uint32_t quuppa_viterbi1M_3_modemConfig[];

#define RAIL0_BLE_VITERBI1M_PHY_BLUETOOTH_1M_PROD
#define RAIL0_BLE_VITERBI1M_PROFILE_BLE
extern const RAIL_ChannelConfig_t ble_viterbi1M_channelConfig;
extern const RAIL_ChannelConfigEntry_t ble_viterbi1M_channels[];

#define RAIL0_BLE_VITERBI2M_PHY_BLUETOOTH_2M_PROD
#define RAIL0_BLE_VITERBI2M_PROFILE_BLE
extern const RAIL_ChannelConfig_t ble_viterbi2M_channelConfig;
extern const RAIL_ChannelConfigEntry_t ble_viterbi2M_channels[];

#define RAIL0_BLE_LR_DSA_125KB_PHY_BLUETOOTH_LONGRANGE_DSA_125KBPS
#define RAIL0_BLE_LR_DSA_125KB_PROFILE_BASE
extern const RAIL_ChannelConfig_t ble_LR_DSA_125kb_channelConfig;
extern const RAIL_ChannelConfigEntry_t ble_LR_DSA_125kb_channels[];

#define RAIL0_BLE_LR_DSA_500KB_PHY_BLUETOOTH_LONGRANGE_DSA_500KBPS
#define RAIL0_BLE_LR_DSA_500KB_PROFILE_BASE
extern const RAIL_ChannelConfig_t ble_LR_DSA_500kb_channelConfig;
extern const RAIL_ChannelConfigEntry_t ble_LR_DSA_500kb_channels[];

#define RAIL0_BLE_LR_DSA_1MB_PHY_BLUETOOTH_1M_CONCURRENT
#define RAIL0_BLE_LR_DSA_1MB_PROFILE_BASE
extern const RAIL_ChannelConfig_t ble_LR_DSA_1Mb_channelConfig;
extern const RAIL_ChannelConfigEntry_t ble_LR_DSA_1Mb_channels[];

#define RAIL0_QUUPPA_VITERBI1M_PHY_QUUPPA_1M_PROD
#define RAIL0_QUUPPA_VITERBI1M_PROFILE_BASE
extern const RAIL_ChannelConfig_t quuppa_viterbi1M_channelConfig;
extern const RAIL_ChannelConfigEntry_t quuppa_viterbi1M_channels[];

#endif // __BLE_EFR32XG27_CONFIGURATOR_OUT_H__
