/***************************************************************************//**
 * @file
 * @brief Auxiliary header for the RAIL library. Includes consistent definitions
 *   of features available across different chips.
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

#ifndef SL_RAIL_FEATURES_H
#define SL_RAIL_FEATURES_H

#include "em_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup RAIL_API
 * @{
 */

/******************************************************************************
 * RAIL Features
 *****************************************************************************/
/**
 * @addtogroup Features
 * @brief Overview of support for various features across hardware platforms.
 *   These defines can be used at compile time to determine which
 *   features are available on your platform. However, keep in mind that
 *   these defines hold true for chip families. Your specific part
 *   may have further restrictions (band limitations, power amplifier
 *   restrictions, and so on) on top of those listed below, for which
 *   runtime sl_rail_*supports*() APIs can be used to check availability
 *   on a particular chip (after \ref sl_rail_init() has been called).
 *   In general, an attempt to call an API that is not supported on your
 *   chip family as listed below will result in a
 *   \ref SL_RAIL_STATUS_INVALID_CALL.
 * @{
 */

/// Boolean to indicate whether the selected chip supports both Sub-GHz and 2.4 GHz bands.
/// See also runtime refinement \ref sl_rail_supports_dual_band().
#if ((_SILICON_LABS_EFR32_RADIO_TYPE == _SILICON_LABS_EFR32_RADIO_DUALBAND) \
  || ((FEAT_RF_2G4 == 1) && (FEAT_RF_SUBG == 1)))
#define SL_RAIL_SUPPORTS_DUAL_BAND 1
#else
#define SL_RAIL_SUPPORTS_DUAL_BAND 0
#endif

/// Boolean to indicate whether the selected chip supports the 2.4 GHz band.
/// See also runtime refinement \ref sl_rail_supports_2p4_ghz_band().
#if (((_SILICON_LABS_EFR32_RADIO_TYPE == _SILICON_LABS_EFR32_RADIO_DUALBAND) \
  || (_SILICON_LABS_EFR32_RADIO_TYPE == _SILICON_LABS_EFR32_RADIO_2G4HZ))    \
  || (FEAT_RF_2G4 == 1))
#define SL_RAIL_SUPPORTS_2P4_GHZ_BAND 1
#else
#define SL_RAIL_SUPPORTS_2P4_GHZ_BAND 0
#endif

/// Boolean to indicate whether the selected chip supports Sub-GHz bands.
/// See also runtime refinement \ref sl_rail_supports_sub_ghz_band().
#if (((_SILICON_LABS_EFR32_RADIO_TYPE == _SILICON_LABS_EFR32_RADIO_DUALBAND) \
  || (_SILICON_LABS_EFR32_RADIO_TYPE == _SILICON_LABS_EFR32_RADIO_SUBGHZ))   \
  || (FEAT_RF_SUBG == 1))
#define SL_RAIL_SUPPORTS_SUB_GHZ_BAND 1
#else
#define SL_RAIL_SUPPORTS_SUB_GHZ_BAND 0
#endif

/// Boolean to indicate whether the selected chip supports OFDM PA.
/// See also runtime refinement \ref sl_rail_supports_ofdm_pa().
#if (_SILICON_LABS_32B_SERIES_2_CONFIG == 5)
#define SL_RAIL_SUPPORTS_OFDM_PA 1
#else
#define SL_RAIL_SUPPORTS_OFDM_PA 0
#endif

/// Boolean to indicate whether the selected chip supports
/// bit masked address filtering.
/// See also runtime refinement \ref sl_rail_supports_addr_filter_address_bit_mask().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG >= 2) \
  || (_SILICON_LABS_32B_SERIES_3_CONFIG >= 300))
#define SL_RAIL_SUPPORTS_ADDR_FILTER_ADDRESS_BIT_MASK 1
#else
#define SL_RAIL_SUPPORTS_ADDR_FILTER_ADDRESS_BIT_MASK 0
#endif

/// Boolean to indicate whether the selected chip supports
/// address filter mask information for incoming packets in
/// \ref sl_rail_rx_packet_info_t::filter_mask and
/// \ref sl_rail_ieee802154_address_t::filter_mask.
/// See also runtime refinement \ref sl_rail_supports_addr_filter_mask().
#if 1
#define SL_RAIL_SUPPORTS_ADDR_FILTER_MASK 1
#else
#define SL_RAIL_SUPPORTS_ADDR_FILTER_MASK 0
#endif

/// Boolean to indicate whether the selected chip supports
/// alternate power settings for the Power Amplifier.
/// See also runtime refinement \ref sl_rail_supports_alternate_tx_power().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))
#define SL_RAIL_SUPPORTS_ALTERNATE_TX_POWER 1
#else
#define SL_RAIL_SUPPORTS_ALTERNATE_TX_POWER 0
#endif

/// Boolean to indicate whether the selected chip supports antenna diversity.
/// See also runtime refinement \ref sl_rail_supports_antenna_diversity().
#if (_SILICON_LABS_32B_SERIES == 2)
#define SL_RAIL_SUPPORTS_ANTENNA_DIVERSITY 1
#else
#define SL_RAIL_SUPPORTS_ANTENNA_DIVERSITY 0
#endif

/// Boolean to indicate whether the selected chip supports internal RF path diversity.
/// See also runtime refinement \ref sl_rail_supports_path_diversity().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))
#define SL_RAIL_SUPPORTS_PATH_DIVERSITY 1
#else
#define SL_RAIL_SUPPORTS_PATH_DIVERSITY 0
#endif

/// Boolean to indicate whether the selected chip supports channel hopping.
/// See also runtime refinement \ref sl_rail_supports_channel_hopping().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG >= 1) \
  || (_SILICON_LABS_32B_SERIES_3_CONFIG >= 300))
#define SL_RAIL_SUPPORTS_CHANNEL_HOPPING 1
#else
#define SL_RAIL_SUPPORTS_CHANNEL_HOPPING 0
#endif

/// Boolean to indicate whether the selected chip supports dual sync words.
/// See also runtime refinement \ref sl_rail_supports_dual_sync_words().
#if 1
#define SL_RAIL_SUPPORTS_DUAL_SYNC_WORDS 1
#else
#define SL_RAIL_SUPPORTS_DUAL_SYNC_WORDS 0
#endif

/// Boolean to indicate whether the selected chip supports automatic transitions
/// from TX to TX.
/// See also runtime refinement \ref sl_rail_supports_tx_to_tx().
#if 1
#define SL_RAIL_SUPPORTS_TX_TO_TX 1
#else
#define SL_RAIL_SUPPORTS_TX_TO_TX 0
#endif

/// Boolean to indicate whether the selected chip supports \ref SL_RAIL_TX_REPEAT_OPTION_START_TO_START.
/// See also runtime refinement \ref sl_rail_supports_tx_repeat_start_to_start().
#if (_SILICON_LABS_32B_SERIES_2_CONFIG >= 2)
#define SL_RAIL_SUPPORTS_TX_REPEAT_START_TO_START SL_RAIL_SUPPORTS_TX_TO_TX
#else
#define SL_RAIL_SUPPORTS_TX_REPEAT_START_TO_START 0
#endif

/// Boolean to indicate whether the selected chip supports thermistor measurements.
/// See also runtime refinement \ref sl_rail_supports_external_thermistor().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 5) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))
#define SL_RAIL_SUPPORTS_EXTERNAL_THERMISTOR 1
#else
#define SL_RAIL_SUPPORTS_EXTERNAL_THERMISTOR 0
#endif

/// Boolean to indicate whether the selected chip supports HFXO compensation.
/// See also runtime refinement \ref sl_rail_supports_hfxo_compensation().
#if (_SILICON_LABS_32B_SERIES_2_CONFIG == 5)
#define SL_RAIL_SUPPORTS_HFXO_COMPENSATION SL_RAIL_SUPPORTS_EXTERNAL_THERMISTOR
#else
#define SL_RAIL_SUPPORTS_HFXO_COMPENSATION 0
#endif

/// Boolean to indicate whether the selected chip supports AUXADC measurements.
/// See also runtime refinement \ref sl_rail_supports_aux_adc().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 5) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))
#define SL_RAIL_SUPPORTS_AUXADC 1
#else
#define SL_RAIL_SUPPORTS_AUXADC 0
#endif

/// Boolean to indicate whether the selected chip supports a high-precision
/// LFRCO.
/// Best to use the runtime refinement \ref sl_rail_supports_precision_lfrco()
/// because some chip revisions do not support it.
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9))
#define SL_RAIL_SUPPORTS_PRECISION_LFRCO 1
#else
#define SL_RAIL_SUPPORTS_PRECISION_LFRCO 0
#endif

/// Boolean to indicate whether the selected chip supports radio entropy.
/// See also runtime refinement \ref sl_rail_supports_radio_entropy().
#ifndef FPGA
#define SL_RAIL_SUPPORTS_RADIO_ENTROPY 1
#else
#define SL_RAIL_SUPPORTS_RADIO_ENTROPY 0
#endif

/// Boolean to indicate whether the selected chip supports
/// RF Sense Energy Detection Mode.
/// See also runtime refinement \ref sl_rail_supports_rf_sense_energy_detection().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9))
#define SL_RAIL_SUPPORTS_RF_SENSE_ENERGY_DETECTION 1
#else
#define SL_RAIL_SUPPORTS_RF_SENSE_ENERGY_DETECTION 0
#endif

/// Boolean to indicate whether the selected chip supports
/// RF Sense Selective(OOK) Mode.
/// See also runtime refinement \ref sl_rail_supports_rf_sense_selective_ook().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9))
#define SL_RAIL_SUPPORTS_RF_SENSE_SELECTIVE_OOK 1
#else
#define SL_RAIL_SUPPORTS_RF_SENSE_SELECTIVE_OOK 0
#endif

/// Boolean to indicate whether the selected chip supports RFSENSE OOK PHY.

#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 4) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9))
#define SL_RAIL_SUPPORTS_RF_SENSE_OOK_PHY 1
#else
#define SL_RAIL_SUPPORTS_RF_SENSE_OOK_PHY 0
#endif

/// Boolean to indicate whether the selected chip supports the
/// VDET voltage measurement feature.
/// See also runtime refinement \ref sl_rail_supports_vdet().
#if (_SILICON_LABS_32B_SERIES_2_CONFIG == 5)
#define SL_RAIL_SUPPORTS_VDET 1
#else
#define SL_RAIL_SUPPORTS_VDET 0
#endif

/// Boolean to indicate whether the selected chip supports RX duty cycling.
/// See also runtime refinement \ref sl_rail_supports_rx_duty_cycling().
#if (_SILICON_LABS_32B_SERIES_3_CONFIG == 301)
#define SL_RAIL_SUPPORTS_RX_DUTY_CYCLING (1U)
#else
#define SL_RAIL_SUPPORTS_RX_DUTY_CYCLING (0U)
#endif

#ifndef DOXYGEN_UNDOCUMENTED
/// Boolean to indicate whether the selected chip supports the Sequencer User
/// See also runtime refinement \ref sl_rail_supports_sequencer_user().
#if (_SILICON_LABS_32B_SERIES_3_CONFIG >= 300)
#define SL_RAIL_SUPPORTS_SEQUENCER_USER 1
#else
#define SL_RAIL_SUPPORTS_SEQUENCER_USER 0
#endif
#endif //DOXYGEN_UNDOCUMENTED

// BLE features
// Some features may not be available on all platforms
// due to radio hardware limitations.

/// Boolean to indicate whether the selected chip supports BLE.
/// See also runtime refinement \ref sl_rail_supports_protocol_ble().
#if 1
#define SL_RAIL_SUPPORTS_PROTOCOL_BLE SL_RAIL_SUPPORTS_2P4_GHZ_BAND
#else
#define SL_RAIL_SUPPORTS_PROTOCOL_BLE 0
#endif

/// Boolean to indicate whether the selected chip supports BLE 1 Mbps PHY.
/// See also runtime refinement \ref sl_rail_ble_supports_1_mbps().
#if 1
#define SL_RAIL_BLE_SUPPORTS_1_MBPS SL_RAIL_SUPPORTS_PROTOCOL_BLE
#else
#define SL_RAIL_BLE_SUPPORTS_1_MBPS 0
#endif

/// Boolean to indicate whether the selected chip supports BLE 2 Mbps PHY.
/// See also runtime refinement \ref sl_rail_ble_supports_2_mbps().
#if 1
#define SL_RAIL_BLE_SUPPORTS_2_MBPS SL_RAIL_SUPPORTS_PROTOCOL_BLE
#else
#define SL_RAIL_BLE_SUPPORTS_2_MBPS 0
#endif

/// Boolean to indicate whether the selected chip supports BLE
/// Antenna Switching needed for Angle-of-Arrival receives or
/// Angle-of-Departure transmits.
/// See also runtime refinement \ref sl_rail_ble_supports_antenna_switching().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 4) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6) \
  || (_SILICON_LABS_32B_SERIES_3_CONFIG >= 300))
#define SL_RAIL_BLE_SUPPORTS_ANTENNA_SWITCHING SL_RAIL_SUPPORTS_PROTOCOL_BLE
#else
#define SL_RAIL_BLE_SUPPORTS_ANTENNA_SWITCHING 0
#endif

/// Boolean to indicate whether the selected chip supports the BLE Coded PHY
/// used for Long-Range.
/// See also runtime refinement \ref sl_rail_ble_supports_coded_phy().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 1) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 4) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9) \
  || (_SILICON_LABS_32B_SERIES_3_CONFIG >= 300))
#define SL_RAIL_BLE_SUPPORTS_CODED_PHY SL_RAIL_SUPPORTS_PROTOCOL_BLE
#else
#define SL_RAIL_BLE_SUPPORTS_CODED_PHY 0
#endif

/// Boolean to indicate whether the selected chip supports the BLE Simulscan PHY
/// used for simultaneous BLE 1 Mbps and Coded PHY reception.
/// See also runtime refinement \ref sl_rail_ble_supports_simulscan_phy().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 4) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9) \
  || (_SILICON_LABS_32B_SERIES_3_CONFIG >= 300))
#define SL_RAIL_BLE_SUPPORTS_SIMULSCAN_PHY SL_RAIL_SUPPORTS_PROTOCOL_BLE
#else
#define SL_RAIL_BLE_SUPPORTS_SIMULSCAN_PHY 0
#endif

/// Boolean to indicate whether the selected chip supports BLE
/// CTE (Constant Tone Extension) needed for Angle-of-Arrival/Departure
/// transmits.
/// See also runtime refinement \ref sl_rail_ble_supports_cte().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 4) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9) \
  || (_SILICON_LABS_32B_SERIES_3_CONFIG >= 300))
#define SL_RAIL_BLE_SUPPORTS_CTE SL_RAIL_SUPPORTS_PROTOCOL_BLE
#else
#define SL_RAIL_BLE_SUPPORTS_CTE 0
#endif

/// Boolean to indicate whether the selected chip supports the
/// Quuppa PHY.
/// See also runtime refinement \ref sl_rail_ble_supports_quuppa().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9))
#define SL_RAIL_BLE_SUPPORTS_QUUPPA SL_RAIL_SUPPORTS_PROTOCOL_BLE
#else
#define SL_RAIL_BLE_SUPPORTS_QUUPPA 0
#endif

/// Boolean to indicate whether the selected chip supports BLE
/// IQ Sampling needed for Angle-of-Arrival/Departure receives.
/// See also runtime refinement \ref sl_rail_ble_supports_iq_sampling().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 4) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6) \
  || (_SILICON_LABS_32B_SERIES_3_CONFIG >= 300))
#define SL_RAIL_BLE_SUPPORTS_IQ_SAMPLING SL_RAIL_SUPPORTS_PROTOCOL_BLE
#else
#define SL_RAIL_BLE_SUPPORTS_IQ_SAMPLING 0
#endif

/// Boolean to indicate whether the selected chip supports some BLE AOX
/// features.
#define SL_RAIL_BLE_SUPPORTS_AOX          \
  (SL_RAIL_BLE_SUPPORTS_ANTENNA_SWITCHING \
   || SL_RAIL_BLE_SUPPORTS_IQ_SAMPLING    \
   || SL_RAIL_BLE_SUPPORTS_CTE)

#ifndef DOXYGEN_UNDOCUMENTED
/// Boolean to indicate whether the selected chip supports BLE
/// CS (Channel Sounding).
#if (_SILICON_LABS_32B_SERIES_2_CONFIG == 4)
#define SL_RAIL_BLE_SUPPORTS_CS SL_RAIL_SUPPORTS_PROTOCOL_BLE
#else
#define SL_RAIL_BLE_SUPPORTS_CS 0
#endif
#endif//DOXYGEN_UNDOCUMENTED

/// Boolean to indicate whether the selected chip supports BLE PHY switch to RX
/// functionality, which is used to switch BLE PHYs at a specific time
/// to receive auxiliary packets.
/// See also runtime refinement \ref sl_rail_ble_supports_phy_switch_to_rx().
#if 1
#define SL_RAIL_BLE_SUPPORTS_PHY_SWITCH_TO_RX SL_RAIL_SUPPORTS_PROTOCOL_BLE
#else
#define SL_RAIL_BLE_SUPPORTS_PHY_SWITCH_TO_RX 0
#endif

// IEEE 802.15.4 features
// Some features may not be available on all platforms
// due to radio hardware limitations.

/// Boolean to indicate whether the selected chip supports IEEE 802.15.4.
/// See also runtime refinement \ref sl_rail_supports_protocol_ieee802154().
#if 1
#define SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154 1
#else
#define SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154 0
#endif

/// Boolean to indicate whether the selected chip supports the
/// 802.15.4 Wi-Fi Coexistence PHY.
/// See also runtime refinement \ref sl_rail_ieee802154_supports_coex_phy().
#if 0
#define SL_RAIL_IEEE802154_SUPPORTS_COEX_PHY (SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154 && SL_RAIL_SUPPORTS_2P4_GHZ_BAND)
#else
#define SL_RAIL_IEEE802154_SUPPORTS_COEX_PHY 0
#endif

/// Boolean to indicate whether the selected chip supports
/// the IEEE 802.15.4 2.4 GHz band variant.
/// See also runtime refinement \ref sl_rail_ieee802154_supports_2p4_ghz_band().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG != 3) \
  && (_SILICON_LABS_32B_SERIES_2_CONFIG != 13))
#define SL_RAIL_IEEE802154_SUPPORTS_2P4_GHZ_BAND (SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154 && SL_RAIL_SUPPORTS_2P4_GHZ_BAND)
#else
#define SL_RAIL_IEEE802154_SUPPORTS_2P4_GHZ_BAND 0
#endif

/// Boolean to indicate whether the selected chip supports
/// the IEEE 802.15.4 2.4 RX channel switching.
/// See also runtime refinement \ref sl_rail_ieee802154_supports_rx_channel_switching().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 1) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 4) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6) \
  || (_SILICON_LABS_32B_SERIES_3_CONFIG == 301))
#define SL_RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING (SL_RAIL_IEEE802154_SUPPORTS_2P4_GHZ_BAND)
#else
#define SL_RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING 0
#endif

/// Boolean to indicate whether the selected chip supports a front end module.
/// See also runtime refinement \ref sl_rail_ieee802154_supports_fem_phy().
#if (_SILICON_LABS_32B_SERIES_2_CONFIG != 8)
#define SL_RAIL_IEEE802154_SUPPORTS_FEM_PHY (SL_RAIL_IEEE802154_SUPPORTS_2P4_GHZ_BAND)
#else
#define SL_RAIL_IEEE802154_SUPPORTS_FEM_PHY 0
#endif

/// Boolean to indicate whether the selected chip supports
/// IEEE 802.15.4E-2012 feature subset needed for Zigbee R22 GB868.
/// See also runtime refinement \ref
/// sl_rail_ieee802154_supports_e_subset_gb868().
#if 1
#define SL_RAIL_IEEE802154_SUPPORTS_E_SUBSET_GB868 SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154
#else
#define SL_RAIL_IEEE802154_SUPPORTS_E_SUBSET_GB868 0
#endif

/// Boolean to indicate whether the selected chip supports
/// IEEE 802.15.4E-2012 Enhanced ACKing.
/// See also runtime refinement \ref
/// sl_rail_ieee802154_supports_e_enhanced_ack().
#if 1
#define SL_RAIL_IEEE802154_SUPPORTS_E_ENHANCED_ACK SL_RAIL_IEEE802154_SUPPORTS_E_SUBSET_GB868
#else
#define SL_RAIL_IEEE802154_SUPPORTS_E_ENHANCED_ACK 0
#endif

/// Boolean to indicate whether the selected chip supports
/// receiving IEEE 802.15.4E-2012 Multipurpose frames.
/// See also runtime refinement \ref
/// sl_rail_ieee802154_supports_e_multipurpose_frames().
#if 1
#define SL_RAIL_IEEE802154_SUPPORTS_E_MULTIPURPOSE_FRAMES SL_RAIL_IEEE802154_SUPPORTS_E_SUBSET_GB868
#else
#define SL_RAIL_IEEE802154_SUPPORTS_E_MULTIPURPOSE_FRAMES 0
#endif

/// Boolean to indicate whether the selected chip supports
/// IEEE 802.15.4G-2012 feature subset needed for Zigbee R22 GB868.
/// See also runtime refinement \ref
/// sl_rail_ieee802154_supports_g_subset_gb868().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG != 3) \
  && (_SILICON_LABS_32B_SERIES_2_CONFIG != 13))
#define SL_RAIL_IEEE802154_SUPPORTS_G_SUBSET_GB868 \
  ((SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154 != 0) && (SL_RAIL_SUPPORTS_SUB_GHZ_BAND != 0))
#else
#define SL_RAIL_IEEE802154_SUPPORTS_G_SUBSET_GB868 0
#endif

/// Boolean to indicate whether the selected chip supports
/// dynamic FEC
/// See also runtime refinement \ref
/// sl_rail_ieee802154_supports_g_dyn_fec().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG > 1) \
  || (_SILICON_LABS_32B_SERIES_3_CONFIG >= 300))
#define SL_RAIL_IEEE802154_SUPPORTS_G_DYN_FEC \
  SL_RAIL_IEEE802154_SUPPORTS_G_SUBSET_GB868  // limit to Sub-GHz for now
#else
#define SL_RAIL_IEEE802154_SUPPORTS_G_DYN_FEC 0
#endif

/// Boolean to indicate whether the selected chip supports
/// Wi-SUN mode switching
/// See also runtime refinement \ref
/// sl_rail_ieee802154_supports_g_mode_switch().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 5) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8))
#define SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH \
  SL_RAIL_IEEE802154_SUPPORTS_G_SUBSET_GB868  // limit to Sub-GHz for now
#else
#define SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH 0
#endif

/// Boolean to indicate whether the selected chip supports
/// IEEE 802.15.4G-2012 reception and transmission of frames
/// with 4-byte CRC.
/// See also runtime refinement \ref sl_rail_ieee802154_supports_g_4_byte_crc().
#if 1
#define SL_RAIL_IEEE802154_SUPPORTS_G_4_BYTE_CRC SL_RAIL_IEEE802154_SUPPORTS_G_SUBSET_GB868
#else
#define SL_RAIL_IEEE802154_SUPPORTS_G_4_BYTE_CRC 0
#endif

/// Boolean to indicate whether the selected chip supports
/// IEEE 802.15.4G-2012 reception of unwhitened frames.
/// See also runtime refinement \ref
/// sl_rail_ieee802154_supports_g_unwhitened_rx().
#if 1
#define SL_RAIL_IEEE802154_SUPPORTS_G_UNWHITENED_RX SL_RAIL_IEEE802154_SUPPORTS_G_SUBSET_GB868
#else
#define SL_RAIL_IEEE802154_SUPPORTS_G_UNWHITENED_RX 0
#endif

/// Boolean to indicate whether the selected chip supports
/// IEEE 802.15.4G-2012 transmission of unwhitened frames.
/// See also runtime refinement \ref
/// sl_rail_ieee802154_supports_g_unwhitened_tx().
#if 1
#define SL_RAIL_IEEE802154_SUPPORTS_G_UNWHITENED_TX SL_RAIL_IEEE802154_SUPPORTS_G_SUBSET_GB868
#else
#define SL_RAIL_IEEE802154_SUPPORTS_G_UNWHITENED_TX 0
#endif

/// Boolean to indicate whether the selected chip supports
/// canceling the frame-pending lookup event
/// \ref SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND
/// when the radio transitions to a state that renders the
/// the reporting of this event moot (i.e., too late for
/// the stack to influence the outgoing ACK).
/// See also runtime refinement \ref
/// sl_rail_ieee802154_supports_cancel_frame_pending_lookup().
#if 1
#define SL_RAIL_IEEE802154_SUPPORTS_CANCEL_FRAME_PENDING_LOOKUP SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154
#else
#define SL_RAIL_IEEE802154_SUPPORTS_CANCEL_FRAME_PENDING_LOOKUP 0
#endif

/// Boolean to indicate whether the selected chip supports
/// early triggering of the frame-pending lookup event
/// \ref SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND
/// just after MAC address fields have been received.
/// See also runtime refinement \ref
/// sl_rail_ieee802154_supports_early_frame_pending_lookup().
#if 1
#define SL_RAIL_IEEE802154_SUPPORTS_EARLY_FRAME_PENDING_LOOKUP SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154
#else
#define SL_RAIL_IEEE802154_SUPPORTS_EARLY_FRAME_PENDING_LOOKUP 0
#endif

/// Boolean to indicate whether the selected chip supports dual PA configs for mode switch
/// or concurrent mode.
/// See also runtime refinement \ref sl_rail_ieee802154_supports_dual_pa_config().
#if (_SILICON_LABS_32B_SERIES_2_CONFIG == 5)
#define SL_RAIL_IEEE802154_SUPPORTS_DUAL_PA_CONFIG 1
#else
#define SL_RAIL_IEEE802154_SUPPORTS_DUAL_PA_CONFIG 0
#endif

/// Boolean to indicate whether the selected chip supports the pa power setting table.
#ifdef RAIL_MODULE_LIB
#define SL_RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE 0
#else
#define SL_RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE 1
#endif

#ifndef DOXYGEN_UNDOCUMENTED
/// Boolean to indicate whether the selected chip uses the common PA interface.
/// This feature is available when the configuration for Silicon Labs Series 3
/// devices is set to 1, enabling the use of a unified Power Amplifier (PA) interface
/// across different configurations.
#if ((_SILICON_LABS_32B_SERIES == 2)           \
  && (_SILICON_LABS_32B_SERIES_2_CONFIG != 5)) \
  || (_SILICON_LABS_32B_SERIES_3_CONFIG >= 300)
#define SL_RAIL_SUPPORTS_COMMON_PA_INTERFACE SL_RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
#else
#define SL_RAIL_SUPPORTS_COMMON_PA_INTERFACE 0
#endif
#endif //DOXYGEN_UNDOCUMENTED

#ifndef DOXYGEN_UNDOCUMENTED
/// Boolean to indicate whether the selected chip supports
/// IEEE802.15.4 2.4 GHz at 2 Mbps
/// See also runtime refinement \ref
/// sl_rail_ieee802154_supports_2_mbps_phy().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 1) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6) \
  || (_SILICON_LABS_32B_SERIES_3_CONFIG >= 300))
#define SL_RAIL_IEEE802154_SUPPORTS_2_MBPS_PHY \
  (SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154 && SL_RAIL_SUPPORTS_2P4_GHZ_BAND)
#else
#define SL_RAIL_IEEE802154_SUPPORTS_2_MBPS_PHY 0
#endif

/// Boolean to indicate whether the selected chip supports
/// IEEE802.15.4 2.4 GHz at 2 Mbps and fast channel switching
/// See also runtime refinement \ref
/// sl_rail_ieee802154_supports_fcs_2_mbps_phy().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 6) \
  || (_SILICON_LABS_32B_SERIES_3_CONFIG == 301))
#define SL_RAIL_IEEE802154_SUPPORTS_FCS_2_MBPS_PHY \
  (SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154 && SL_RAIL_SUPPORTS_2P4_GHZ_BAND)
#else
#define SL_RAIL_IEEE802154_SUPPORTS_FCS_2_MBPS_PHY 0
#endif
#endif //DOXYGEN_UNDOCUMENTED

// Wi_SUN features

/// Boolean to indicate whether the selected chip supports
/// Wi-SUN
/// See also runtime refinement \ref
/// sl_rail_supports_protocol_wi_sun().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 5) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))
#define SL_RAIL_SUPPORTS_PROTOCOL_WI_SUN 1
#else
#define SL_RAIL_SUPPORTS_PROTOCOL_WI_SUN 0
#endif

// WMBUS features

/// Boolean to indicate whether the selected chip supports WMBUS simultaneous
/// M2O RX of T and C modes set by \ref sl_rail_wmbus_config().
/// See also runtime refinement \ref sl_rail_wmbus_supports_simultaneous_t_c_rx().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))
#define SL_RAIL_WMBUS_SUPPORTS_SIMULTANEOUS_T_C_RX 1
#else
#define SL_RAIL_WMBUS_SUPPORTS_SIMULTANEOUS_T_C_RX 0
#endif

// Z-Wave features
// Some features may not be available on all platforms
// due to radio hardware limitations.

/// Boolean to indicate whether the selected chip supports Z-Wave.
/// See also runtime refinement \ref sl_rail_supports_protocol_zwave().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8))
#define SL_RAIL_SUPPORTS_PROTOCOL_ZWAVE SL_RAIL_SUPPORTS_SUB_GHZ_BAND
#else
#define SL_RAIL_SUPPORTS_PROTOCOL_ZWAVE 0
#endif

/// Boolean to indicate whether the selected chip supports concurrent PHY.
/// See also runtime refinement \ref sl_rail_zwave_supports_conc_phy().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8))
#define SL_RAIL_ZWAVE_SUPPORTS_CONC_PHY SL_RAIL_SUPPORTS_PROTOCOL_ZWAVE
#else
#define SL_RAIL_ZWAVE_SUPPORTS_CONC_PHY 0
#endif

/// Boolean to indicate whether the selected chip supports SQ-based PHY.
/// See also runtime refinement \ref sl_rail_supports_sq_phy().
#if (((_SILICON_LABS_32B_SERIES_2_CONFIG >= 3) \
  && (_SILICON_LABS_32B_SERIES_2_CONFIG != 7)  \
  && (_SILICON_LABS_32B_SERIES_2_CONFIG != 9)) \
  || (_SILICON_LABS_32B_SERIES_3_CONFIG >= 300))
#define SL_RAIL_SUPPORTS_SQ_PHY 1
#else
#define SL_RAIL_SUPPORTS_SQ_PHY 0
#endif

/// Boolean to indicate whether the code supports Z-Wave
/// region information in PTI and
/// newer \ref sl_rail_zwave_region_config_t structure
/// See also runtime refinement \ref sl_rail_zwave_supports_region_pti().
#if 1
#define SL_RAIL_ZWAVE_SUPPORTS_REGION_PTI SL_RAIL_SUPPORTS_PROTOCOL_ZWAVE
#else
#define SL_RAIL_ZWAVE_SUPPORTS_REGION_PTI 0
#endif

/// Boolean to indicate whether the selected chip supports raw RX data
/// sources other than \ref sl_rail_rx_data_source_t::SL_RAIL_RX_DATA_SOURCE_PACKET_DATA.
/// See also runtime refinement \ref sl_rail_supports_rx_raw_data().
#if (_SILICON_LABS_32B_SERIES_3_CONFIG == 0)
#define SL_RAIL_SUPPORTS_RX_RAW_DATA 1
#else
#define SL_RAIL_SUPPORTS_RX_RAW_DATA 0
#endif

/// Boolean to indicate whether the selected chip supports
/// direct mode.
/// See also runtime refinement \ref sl_rail_supports_direct_mode().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))
#define SL_RAIL_SUPPORTS_DIRECT_MODE 1
#else
#define SL_RAIL_SUPPORTS_DIRECT_MODE 0
#endif

/// Boolean to indicate whether the selected chip supports
/// RX direct mode data to FIFO.
/// See also runtime refinement \ref sl_rail_supports_rx_direct_mode_data_to_fifo().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))
#define SL_RAIL_SUPPORTS_RX_DIRECT_MODE_DATA_TO_FIFO 1
#else
#define SL_RAIL_SUPPORTS_RX_DIRECT_MODE_DATA_TO_FIFO 0
#endif

/// Boolean to indicate whether the selected chip supports
/// MFM protocol.
/// See also runtime refinement \ref sl_rail_supports_mfm().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))
#define SL_RAIL_SUPPORTS_MFM 1
#else
#define SL_RAIL_SUPPORTS_MFM 0
#endif

/// Boolean to indicate whether the selected chip supports
/// 802.15.4 signal detection.
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 4) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6) \
  || (_SILICON_LABS_32B_SERIES_3_CONFIG >= 300))
#define SL_RAIL_IEEE802154_SUPPORTS_SIGNAL_IDENTIFIER  (SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154)
#else
#define SL_RAIL_IEEE802154_SUPPORTS_SIGNAL_IDENTIFIER  0
#endif

/// Boolean to indicate whether the selected chip supports
/// BLE signal detection.
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 4) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6) \
  || (_SILICON_LABS_32B_SERIES_3_CONFIG >= 300))
#define SL_RAIL_BLE_SUPPORTS_SIGNAL_IDENTIFIER         (SL_RAIL_SUPPORTS_PROTOCOL_BLE)
#else
#define SL_RAIL_BLE_SUPPORTS_SIGNAL_IDENTIFIER         0
#endif

/// Boolean to indicate whether the selected chip supports
/// configurable RSSI threshold set by \ref sl_rail_set_rssi_detect_threshold().
/// See also runtime refinement \ref sl_rail_supports_rssi_detect_threshold().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 5) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))
#define SL_RAIL_SUPPORTS_RSSI_DETECT_THRESHOLD (1U)
#else
#define SL_RAIL_SUPPORTS_RSSI_DETECT_THRESHOLD (0U)
#endif

/// Boolean to indicate whether the selected chip supports
/// thermal protection set by \ref sl_rail_config_thermal_protection().
/// See also runtime refinement \ref sl_rail_supports_thermal_protection().
#if (_SILICON_LABS_32B_SERIES_2_CONFIG == 5)
  #define SL_RAIL_SUPPORTS_THERMAL_PROTECTION  (1U)
#else
  #define SL_RAIL_SUPPORTS_THERMAL_PROTECTION  (0U)
#endif

/// Boolean to indicate whether the selected chip supports fast RX-to-RX enabled by
/// \ref SL_RAIL_RX_OPTION_FAST_RX_TO_RX.
/// See also runtime refinement \ref sl_rail_supports_fast_rx_to_rx().
#if (_SILICON_LABS_32B_SERIES_2_CONFIG >= 2)
  #define SL_RAIL_SUPPORTS_FAST_RX_TO_RX  (1U)
#else
  #define SL_RAIL_SUPPORTS_FAST_RX_TO_RX  (0U)
#endif

/// Boolean to indicate whether the selected chip supports collision detection
/// enabled by \ref SL_RAIL_RX_OPTION_ENABLE_COLLISION_DETECTION
/// See also runtime refinement \ref sl_rail_supports_collision_detection().
#if (_SILICON_LABS_32B_SERIES_2_CONFIG == 5)
  #define SL_RAIL_SUPPORTS_COLLISION_DETECTION  (1U)
#else
  #define SL_RAIL_SUPPORTS_COLLISION_DETECTION  (0U)
#endif

/// Boolean to indicate whether the selected chip supports Sidewalk protocol.
/// See also runtime refinement \ref sl_rail_supports_protocol_sidewalk().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))
  #define SL_RAIL_SUPPORTS_PROTOCOL_SIDEWALK (1U)
#else
  #define SL_RAIL_SUPPORTS_PROTOCOL_SIDEWALK (0U)
#endif

/// Boolean to indicate whether the selected chip supports TrustZone secure
/// configuration of peripherals used by RAIL.
/// See also runtime refinement \ref sl_rail_supports_trustzone_secure_peripherals().
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))
  #define SL_RAIL_SUPPORTS_TRUSTZONE_SECURE_PERIPHERALS (1U)
#else
  #define SL_RAIL_SUPPORTS_TRUSTZONE_SECURE_PERIPHERALS (0U)
#endif

/// Boolean to indicate whether the selected chip supports automatic prs LNA
/// bypass for external FEM.
/// See also runtime refinement \ref sl_rail_supports_prs_lna_bypass().
#if (_SILICON_LABS_32B_SERIES_2_CONFIG == 5)
  #define SL_RAIL_SUPPORTS_PRS_LNA_BYPASS (1U)
#else
  #define SL_RAIL_SUPPORTS_PRS_LNA_BYPASS (0U)
#endif

#ifndef DOXYGEN_UNDOCUMENTED
/// Boolean to indicate whether the selected chip supports BTC.
/// See also runtime refinement \ref sl_rail_supports_protocol_btc().
#if (_SILICON_LABS_32B_SERIES_3_CONFIG == 300)
#define SL_RAIL_SUPPORTS_PROTOCOL_BTC SL_RAIL_SUPPORTS_2P4_GHZ_BAND
#else
#define SL_RAIL_SUPPORTS_PROTOCOL_BTC 0
#endif
#endif//DOXYGEN_UNDOCUMENTED

#ifndef DOXYGEN_UNDOCUMENTED
/// Boolean to indicate whether the selected chip supports ANT.
/// See also runtime refinement \ref sl_rail_supports_protocol_ant().
#if (_SILICON_LABS_32B_SERIES_3_CONFIG == 300)
#define SL_RAIL_SUPPORTS_PROTOCOL_ANT SL_RAIL_SUPPORTS_2P4_GHZ_BAND
#else
#define SL_RAIL_SUPPORTS_PROTOCOL_ANT 0
#endif
#endif//DOXYGEN_UNDOCUMENTED

/** @} */ // end of group Features

/** @} */ // end of group RAIL_API

#ifdef __cplusplus
}
#endif

#ifdef RAIL_PRIVATE_BUILD
#include "sl_rail_features_internal.h"
#else
#ifdef RAIL_INTERNAL_BUILD
#include "sl_rail_features_internal.h"
#endif
#endif

#endif // SL_RAIL_FEATURES_H
