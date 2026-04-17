/***************************************************************************//**
 * @file sl_rail_phy_overrides.c
 * @brief Applies PHY overrides based on a device's HFXO frequency.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#if defined(SL_CATALOG_CLOCK_MANAGER_PRESENT)
#include "sl_clock_manager_oscillator_config.h"
#define SL_RAIL_PHY_INIT_HFXO_FREQ   SL_CLOCK_MANAGER_HFXO_FREQ
#else // !defined(SL_CATALOG_CLOCK_MANAGER_PRESENT)
#include "sl_device_init_hfxo_config.h"
#define SL_RAIL_PHY_INIT_HFXO_FREQ   SL_DEVICE_INIT_HFXO_FREQ
#endif // defined(SL_CATALOG_CLOCK_MANAGER_PRESENT)

#include "sl_rail_features.h"
#include "sl_rail_ble_config_38M4Hz.h"
#include "sl_rail_ieee802154_config_38M4Hz.h"
#include "sl_rail_ble_config_39MHz.h"
#include "sl_rail_ieee802154_config_39MHz.h"
#include "sl_rail_ble_config_40MHz.h"
#include "sl_rail_ieee802154_config_40MHz.h"
#if SL_RAIL_SUPPORTS_RF_SENSE_OOK_PHY
#include "sl_rail_rfsense_ook_config_38M4Hz.h"
#include "sl_rail_rfsense_ook_config_39MHz.h"
#include "sl_rail_rfsense_ook_config_40MHz.h"
#endif

#if SL_RAIL_PHY_INIT_HFXO_FREQ == 38400000

#if RAIL_BLE_SUPPORTS_1MBPS_NON_VITERBI
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy1Mbps =
  &sl_rail_ble_phy_1Mbps_38M4Hz_channelConfig;
#endif

#if RAIL_BLE_SUPPORTS_2MBPS_NON_VITERBI
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy2Mbps =
  &sl_rail_ble_phy_2Mbps_38M4Hz_channelConfig;
#endif

#if RAIL_BLE_SUPPORTS_1MBPS_VITERBI
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy1MbpsViterbi =
  &sl_rail_ble_phy_1Mbps_viterbi_38M4Hz_channelConfig;
#endif

#if RAIL_BLE_SUPPORTS_2MBPS_VITERBI
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy2MbpsViterbi =
  &sl_rail_ble_phy_2Mbps_viterbi_38M4Hz_channelConfig;
#if RAIL_BLE_SUPPORTS_AOX && ((_SILICON_LABS_32B_SERIES_2_CONFIG == 4) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6))
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy2MbpsAox =
  &sl_rail_ble_phy_2Mbps_aox_38M4Hz_channelConfig;
#else
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy2MbpsAox =
  &sl_rail_ble_phy_2Mbps_viterbi_38M4Hz_channelConfig;
#endif
#endif

#if RAIL_BLE_SUPPORTS_CS
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy1MbpsViterbiCs = NULL;
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy2MbpsViterbiCs = NULL;
#endif

#if RAIL_BLE_SUPPORTS_CODED_PHY
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy125kbps =
  &sl_rail_ble_phy_125kbps_38M4Hz_channelConfig;
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy500kbps =
  &sl_rail_ble_phy_500kbps_38M4Hz_channelConfig;
#endif

#if RAIL_BLE_SUPPORTS_SIMULSCAN_PHY
const RAIL_ChannelConfig_t *const RAIL_BLE_PhySimulscan =
  &sl_rail_ble_phy_simulscan_38M4Hz_channelConfig;
#endif

#if RAIL_SUPPORTS_2P4GHZ_BAND && RAIL_SUPPORTS_PROTOCOL_IEEE802154
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 4) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6))
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHz =
  &sl_rail_ieee802154_phy_2G4Hz_38M4Hz_channelConfig;
#else
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHz =
  &sl_rail_ieee802154_phy_2G4Hz_coherent_38M4Hz_channelConfig;
#endif
#endif

#if RAIL_IEEE802154_SUPPORTS_2MBPS_PHY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHz1MbpsFec =
  &sl_rail_ieee802154_phy_2G4Hz_1Mbps_fec_38M4Hz_channelConfig;
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHz2Mbps =
  &sl_rail_ieee802154_phy_2G4Hz_2Mbps_38M4Hz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_FCS_2MBPS_PHY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzFcs1MbpsFec =
  &sl_rail_ieee802154_phy_2G4Hz_fcs_1Mbps_fec_38M4Hz_channelConfig;
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzFcs2Mbps =
  &sl_rail_ieee802154_phy_2G4Hz_fcs_2Mbps_38M4Hz_channelConfig;
#endif

#if RAIL_SUPPORTS_ANTENNA_DIVERSITY && RAIL_SUPPORTS_2P4GHZ_BAND \
  && RAIL_SUPPORTS_PROTOCOL_IEEE802154
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzAntDiv =
  &sl_rail_ieee802154_phy_2G4Hz_antdiv_38M4Hz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING && (_SILICON_LABS_32B_SERIES_2_CONFIG > 1)
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzRxChSwitching =
  &sl_rail_ieee802154_phy_2G4Hz_fast_switch_38M4Hz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_COEX_PHY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzCoex =
  &sl_rail_ieee802154_phy_2G4Hz_coex_38M4Hz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_COEX_PHY && RAIL_SUPPORTS_ANTENNA_DIVERSITY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzAntDivCoex =
  &sl_rail_ieee802154_phy_2G4Hz_antdiv_coex_38M4Hz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_FEM_PHY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzFem =
  &sl_rail_ieee802154_phy_2G4Hz_fem_38M4Hz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_FEM_PHY && RAIL_SUPPORTS_ANTENNA_DIVERSITY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzAntDivFem =
  &sl_rail_ieee802154_phy_2G4Hz_antdiv_fem_38M4Hz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_FEM_PHY && RAIL_IEEE802154_SUPPORTS_COEX_PHY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzCoexFem =
  &sl_rail_ieee802154_phy_2G4Hz_coex_fem_38M4Hz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_FEM_PHY && RAIL_IEEE802154_SUPPORTS_COEX_PHY \
  && RAIL_SUPPORTS_ANTENNA_DIVERSITY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzAntDivCoexFem =
  &sl_rail_ieee802154_phy_2G4Hz_antdiv_coex_fem_38M4Hz_channelConfig;
#endif

#if SL_RAIL_SUPPORTS_RF_SENSE_OOK_PHY
const RAIL_ChannelConfig_t *const RAIL_RFSENSE_OOK_Phy1kbps =
  &sl_rfsense_ook_1kbps_38M4Hz_channelConfig;
#else
const RAIL_ChannelConfig_t *const RAIL_RFSENSE_OOK_Phy1kbps = NULL;
#endif

#elif SL_RAIL_PHY_INIT_HFXO_FREQ == 39000000

#if RAIL_BLE_SUPPORTS_1MBPS_NON_VITERBI
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy1Mbps =
  &sl_rail_ble_phy_1Mbps_39MHz_channelConfig;
#endif

#if RAIL_BLE_SUPPORTS_2MBPS_NON_VITERBI
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy2Mbps =
  &sl_rail_ble_phy_2Mbps_39MHz_channelConfig;
#endif

#if RAIL_BLE_SUPPORTS_1MBPS_VITERBI
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy1MbpsViterbi =
  &sl_rail_ble_phy_1Mbps_viterbi_39MHz_channelConfig;
#endif

#if RAIL_BLE_SUPPORTS_2MBPS_VITERBI
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy2MbpsViterbi =
  &sl_rail_ble_phy_2Mbps_viterbi_39MHz_channelConfig;
#if RAIL_BLE_SUPPORTS_AOX && ((_SILICON_LABS_32B_SERIES_2_CONFIG == 4) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6))
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy2MbpsAox =
  &sl_rail_ble_phy_2Mbps_aox_39MHz_channelConfig;
#else
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy2MbpsAox =
  &sl_rail_ble_phy_2Mbps_viterbi_39MHz_channelConfig;
#endif
#endif

#if RAIL_BLE_SUPPORTS_CS
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy1MbpsViterbiCs = NULL;
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy2MbpsViterbiCs = NULL;
#endif

#if RAIL_BLE_SUPPORTS_CODED_PHY
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy125kbps =
  &sl_rail_ble_phy_125kbps_39MHz_channelConfig;
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy500kbps =
  &sl_rail_ble_phy_500kbps_39MHz_channelConfig;
#endif

#if RAIL_BLE_SUPPORTS_SIMULSCAN_PHY
const RAIL_ChannelConfig_t *const RAIL_BLE_PhySimulscan =
  &sl_rail_ble_phy_simulscan_39MHz_channelConfig;
#endif

#if RAIL_SUPPORTS_2P4GHZ_BAND && RAIL_SUPPORTS_PROTOCOL_IEEE802154
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 4) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6))
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHz =
  &sl_rail_ieee802154_phy_2G4Hz_39MHz_channelConfig;
#else
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHz =
  &sl_rail_ieee802154_phy_2G4Hz_coherent_39MHz_channelConfig;
#endif
#endif

#if RAIL_SUPPORTS_ANTENNA_DIVERSITY && RAIL_SUPPORTS_2P4GHZ_BAND \
  && RAIL_SUPPORTS_PROTOCOL_IEEE802154
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzAntDiv =
  &sl_rail_ieee802154_phy_2G4Hz_antdiv_39MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING && (_SILICON_LABS_32B_SERIES_2_CONFIG > 1)
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzRxChSwitching =
  &sl_rail_ieee802154_phy_2G4Hz_fast_switch_39MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_COEX_PHY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzCoex =
  &sl_rail_ieee802154_phy_2G4Hz_coex_39MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_COEX_PHY && RAIL_SUPPORTS_ANTENNA_DIVERSITY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzAntDivCoex =
  &sl_rail_ieee802154_phy_2G4Hz_antdiv_coex_39MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_2MBPS_PHY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHz1MbpsFec =
  &sl_rail_ieee802154_phy_2G4Hz_1Mbps_fec_39MHz_channelConfig;
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHz2Mbps =
  &sl_rail_ieee802154_phy_2G4Hz_2Mbps_39MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_FCS_2MBPS_PHY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzFcs1MbpsFec =
  &sl_rail_ieee802154_phy_2G4Hz_fcs_1Mbps_fec_39MHz_channelConfig;
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzFcs2Mbps =
  &sl_rail_ieee802154_phy_2G4Hz_fcs_2Mbps_39MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_FEM_PHY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzFem =
  &sl_rail_ieee802154_phy_2G4Hz_fem_39MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_FEM_PHY && RAIL_SUPPORTS_ANTENNA_DIVERSITY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzAntDivFem =
  &sl_rail_ieee802154_phy_2G4Hz_antdiv_fem_39MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_FEM_PHY && RAIL_IEEE802154_SUPPORTS_COEX_PHY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzCoexFem =
  &sl_rail_ieee802154_phy_2G4Hz_coex_fem_39MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_FEM_PHY && RAIL_IEEE802154_SUPPORTS_COEX_PHY \
  && RAIL_SUPPORTS_ANTENNA_DIVERSITY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzAntDivCoexFem =
  &sl_rail_ieee802154_phy_2G4Hz_antdiv_coex_fem_39MHz_channelConfig;
#endif

#if SL_RAIL_SUPPORTS_RF_SENSE_OOK_PHY
const RAIL_ChannelConfig_t *const RAIL_RFSENSE_OOK_Phy1kbps =
  &sl_rfsense_ook_1kbps_39MHz_channelConfig;
#else
const RAIL_ChannelConfig_t *const RAIL_RFSENSE_OOK_Phy1kbps = NULL;
#endif

#elif SL_RAIL_PHY_INIT_HFXO_FREQ == 40000000

#if RAIL_BLE_SUPPORTS_1MBPS_NON_VITERBI
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy1Mbps =
  &sl_rail_ble_phy_1Mbps_40MHz_channelConfig;
#endif

#if RAIL_BLE_SUPPORTS_2MBPS_NON_VITERBI
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy2Mbps =
  &sl_rail_ble_phy_2Mbps_40MHz_channelConfig;
#endif

#if RAIL_BLE_SUPPORTS_1MBPS_VITERBI
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy1MbpsViterbi =
  &sl_rail_ble_phy_1Mbps_viterbi_40MHz_channelConfig;
#endif

#if RAIL_BLE_SUPPORTS_2MBPS_VITERBI
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy2MbpsViterbi =
  &sl_rail_ble_phy_2Mbps_viterbi_40MHz_channelConfig;
#if RAIL_BLE_SUPPORTS_AOX && ((_SILICON_LABS_32B_SERIES_2_CONFIG == 4) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6))
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy2MbpsAox =
  &sl_rail_ble_phy_2Mbps_aox_40MHz_channelConfig;
#else
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy2MbpsAox =
  &sl_rail_ble_phy_2Mbps_viterbi_40MHz_channelConfig;
#endif
#endif

#if RAIL_BLE_SUPPORTS_CS
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy1MbpsViterbiCs =
  &sl_rail_ble_phy_1Mbps_viterbi_cs_40MHz_channelConfig;
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy2MbpsViterbiCs =
  &sl_rail_ble_phy_2Mbps_viterbi_cs_40MHz_channelConfig;
#endif

#if RAIL_BLE_SUPPORTS_CODED_PHY
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy125kbps =
  &sl_rail_ble_phy_125kbps_40MHz_channelConfig;
const RAIL_ChannelConfig_t *const RAIL_BLE_Phy500kbps =
  &sl_rail_ble_phy_500kbps_40MHz_channelConfig;
#endif

#if RAIL_BLE_SUPPORTS_SIMULSCAN_PHY
const RAIL_ChannelConfig_t *const RAIL_BLE_PhySimulscan =
  &sl_rail_ble_phy_simulscan_40MHz_channelConfig;
#endif

#if RAIL_SUPPORTS_2P4GHZ_BAND && RAIL_SUPPORTS_PROTOCOL_IEEE802154
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 4) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6))
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHz =
  &sl_rail_ieee802154_phy_2G4Hz_40MHz_channelConfig;
#else
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHz =
  &sl_rail_ieee802154_phy_2G4Hz_coherent_40MHz_channelConfig;
#endif
#endif

#if RAIL_SUPPORTS_ANTENNA_DIVERSITY && RAIL_SUPPORTS_2P4GHZ_BAND \
  && RAIL_SUPPORTS_PROTOCOL_IEEE802154
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzAntDiv =
  &sl_rail_ieee802154_phy_2G4Hz_antdiv_40MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING && (_SILICON_LABS_32B_SERIES_2_CONFIG > 1)
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzRxChSwitching =
  &sl_rail_ieee802154_phy_2G4Hz_fast_switch_40MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_COEX_PHY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzCoex =
  &sl_rail_ieee802154_phy_2G4Hz_coex_40MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_COEX_PHY && RAIL_SUPPORTS_ANTENNA_DIVERSITY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzAntDivCoex =
  &sl_rail_ieee802154_phy_2G4Hz_antdiv_coex_40MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_2MBPS_PHY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHz1MbpsFec =
  &sl_rail_ieee802154_phy_2G4Hz_1Mbps_fec_40MHz_channelConfig;
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHz2Mbps =
  &sl_rail_ieee802154_phy_2G4Hz_2Mbps_40MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_FCS_2MBPS_PHY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzFcs1MbpsFec =
  &sl_rail_ieee802154_phy_2G4Hz_fcs_1Mbps_fec_40MHz_channelConfig;
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzFcs2Mbps =
  &sl_rail_ieee802154_phy_2G4Hz_fcs_2Mbps_40MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_FEM_PHY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzFem =
  &sl_rail_ieee802154_phy_2G4Hz_fem_40MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_FEM_PHY && RAIL_SUPPORTS_ANTENNA_DIVERSITY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzAntDivFem =
  &sl_rail_ieee802154_phy_2G4Hz_antdiv_fem_40MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_FEM_PHY && RAIL_IEEE802154_SUPPORTS_COEX_PHY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzCoexFem =
  &sl_rail_ieee802154_phy_2G4Hz_coex_fem_40MHz_channelConfig;
#endif

#if RAIL_IEEE802154_SUPPORTS_FEM_PHY && RAIL_IEEE802154_SUPPORTS_COEX_PHY \
  && RAIL_SUPPORTS_ANTENNA_DIVERSITY
const RAIL_ChannelConfig_t *const RAIL_IEEE802154_Phy2p4GHzAntDivCoexFem =
  &sl_rail_ieee802154_phy_2G4Hz_antdiv_coex_fem_40MHz_channelConfig;
#endif

#if SL_RAIL_SUPPORTS_RF_SENSE_OOK_PHY
const RAIL_ChannelConfig_t *const RAIL_RFSENSE_OOK_Phy1kbps =
  &sl_rfsense_ook_1kbps_40MHz_channelConfig;
#else
const RAIL_ChannelConfig_t *const RAIL_RFSENSE_OOK_Phy1kbps = NULL;
#endif

#endif // SL_RAIL_PHY_INIT_HFXO_FREQ
