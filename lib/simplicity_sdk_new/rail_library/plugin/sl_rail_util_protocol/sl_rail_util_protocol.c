/***************************************************************************//**
 * @file
 * @brief This file implements the radio config commands for RAILtest
 *   applications.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <string.h>
#include <stdio.h>

#include "sl_rail.h"
#include "rail_types.h" // Back-compat for RAIL_RF_STATE_x in config files
#include "sl_rail_ble.h"
#include "sl_rail_ieee802154.h"
#include "sl_rail_zwave.h"
#include "sl_rail_sidewalk.h"

#include "sl_rail_util_protocol.h"
#include "sl_rail_util_protocol_config.h"

static sl_rail_status_t sl_rail_util_protocol_config_proprietary(sl_rail_handle_t handle)
{
  (void) sl_rail_set_pti_protocol(handle, SL_RAIL_PTI_PROTOCOL_CUSTOM);
  return SL_RAIL_STATUS_NO_ERROR;
}

#if SL_RAIL_SUPPORTS_PROTOCOL_BLE && SL_RAIL_UTIL_PROTOCOL_BLE_ENABLE
static sl_rail_status_t sl_rail_util_protocol_config_ble(sl_rail_handle_t handle,
                                                         sl_rail_util_protocol_type_t protocol)
{
  sl_rail_status_t status;
  // Override BLE's default timings to get rid of the default rx search timeout
  sl_rail_state_timing_t timings = {
    .idle_to_rx = SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_IDLE_TO_RX_US,
    .tx_to_rx = SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_TX_TO_RX_US,
    .idle_to_tx = SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_IDLE_TO_TX_US,
    .rx_to_tx = SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_RX_TO_TX_US,
    .rxsearch_timeout = (SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_ENABLE
                         ? SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_US
                         : 0U),
    .tx_to_rxsearch_timeout = (SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_ENABLE
                               ? SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_US
                               : 0U),
  };

  (void) sl_rail_ble_init(handle);
  switch (protocol) {
    case SL_RAIL_UTIL_PROTOCOL_BLE_1MBPS:
      status = sl_rail_ble_config_phy_1_mbps(handle);
      break;
    case SL_RAIL_UTIL_PROTOCOL_BLE_2MBPS:
      status = sl_rail_ble_config_phy_2_mbps(handle);
      break;
    case SL_RAIL_UTIL_PROTOCOL_BLE_CODED_125KBPS:
      status = sl_rail_ble_config_phy_coded(handle, SL_RAIL_BLE_CODING_125_KBPS);
      break;
    case SL_RAIL_UTIL_PROTOCOL_BLE_CODED_500KBPS:
      status = sl_rail_ble_config_phy_coded(handle, SL_RAIL_BLE_CODING_500_KBPS);
      break;
    case SL_RAIL_UTIL_PROTOCOL_BLE_QUUPPA_1MBPS:
      status = sl_rail_ble_config_phy_quuppa(handle);
      break;
    default:
      status = SL_RAIL_STATUS_INVALID_PARAMETER;
      break;
  }
  if (SL_RAIL_STATUS_NO_ERROR == status) {
    status = sl_rail_set_state_timing(handle, &timings);
  }
  if (SL_RAIL_STATUS_NO_ERROR != status) {
    (void) sl_rail_ble_deinit(handle);
  }
  return status;
}
#endif // SL_RAIL_SUPPORTS_PROTOCOL_BLE && SL_RAIL_UTIL_PROTOCOL_BLE_ENABLE

#if SL_RAIL_IEEE802154_SUPPORTS_2P4_GHZ_BAND && SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ENABLE
static sl_rail_status_t sl_rail_util_protocol_config_ieee802154_2p4_ghz(sl_rail_handle_t handle,
                                                                        sl_rail_util_protocol_type_t protocol)
{
  sl_rail_status_t status;
  sl_rail_ieee802154_config_t config = {
    .p_addresses = NULL,
    .ack_config = {
      .enable = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_AUTO_ACK_ENABLE,
      .ack_timeout_us = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_AUTO_ACK_TIMEOUT_US,
      .rx_transitions = {
        .success = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_AUTO_ACK_RX_TRANSITION_STATE,
        .error = SL_RAIL_RF_STATE_IDLE // this parameter ignored
      },
      .tx_transitions = {
        .success = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_AUTO_ACK_TX_TRANSITION_STATE,
        .error = SL_RAIL_RF_STATE_IDLE // this parameter ignored
      }
    },
    .timings = {
      .idle_to_tx = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_TIMING_IDLE_TO_TX_US,
      .idle_to_rx = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_TIMING_IDLE_TO_RX_US,
      .rx_to_tx = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_TIMING_RX_TO_TX_US,
      // Make tx_to_rx slightly lower than desired to make sure we get to
      // RX in time.
      .tx_to_rx = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_TIMING_TX_TO_RX_US,
      .rxsearch_timeout = (SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_ENABLE
                           ? SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_US
                           : 0),
      .tx_to_rxsearch_timeout = (SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_ENABLE
                                 ? SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_US
                                 : 0),
    },
    .frames_mask = (0U // enable appropriate mask bits
                    | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ACCEPT_BEACON_FRAME_ENABLE
                       ? SL_RAIL_IEEE802154_ACCEPT_BEACON_FRAMES : 0U)
                    | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ACCEPT_DATA_FRAME_ENABLE
                       ? SL_RAIL_IEEE802154_ACCEPT_DATA_FRAMES : 0U)
                    | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ACCEPT_ACK_FRAME_ENABLE
                       ? SL_RAIL_IEEE802154_ACCEPT_ACK_FRAMES : 0U)
                    | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ACCEPT_COMMAND_FRAME_ENABLE
                       ? SL_RAIL_IEEE802154_ACCEPT_COMMAND_FRAMES : 0U)),
    // Enable promiscous mode since no PANID or destination address is
    // specified.
    .promiscuous_mode = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_PROMISCUOUS_MODE_ENABLE,
    .is_pan_coordinator = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_PAN_COORDINATOR_ENABLE,
    .default_frame_pending_in_outgoing_acks = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_DEFAULT_FRAME_PENDING_STATE,
  };
  status = sl_rail_ieee802154_init(handle, &config);
  if (SL_RAIL_STATUS_NO_ERROR == status) {
    switch (protocol) {
      case SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ:
        status = sl_rail_ieee802154_config_2p4_ghz_radio(handle);
        break;
      case SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ANTDIV:
        status = sl_rail_ieee802154_config_2p4_ghz_radio_ant_div(handle);
        break;
      case SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_COEX:
        status = sl_rail_ieee802154_config_2p4_ghz_radio_coex(handle);
        break;
      case SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ANTDIV_COEX:
        status = sl_rail_ieee802154_config_2p4_ghz_radio_ant_div_coex(handle);
        break;
      default:
        status = SL_RAIL_STATUS_INVALID_PARAMETER;
        break;
    }
  }
  if (SL_RAIL_STATUS_NO_ERROR != status) {
    (void) sl_rail_ieee802154_deinit(handle);
  } else {
    (void) sl_rail_set_pti_protocol(handle, SL_RAIL_PTI_PROTOCOL_802154);
  }
  return status;
}
#endif // SL_RAIL_IEEE802154_SUPPORTS_2P4_GHZ_BAND && SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ENABLE

#if SL_RAIL_SUPPORTS_SUB_GHZ_BAND && SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_ENABLE
static sl_rail_status_t sl_rail_util_protocol_config_ieee802154_gb868(sl_rail_handle_t handle,
                                                                      sl_rail_util_protocol_type_t protocol)
{
  sl_rail_status_t status;
  sl_rail_ieee802154_config_t config = {
    .p_addresses = NULL,
    .ack_config = {
      .enable = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_AUTO_ACK_ENABLE,
      .ack_timeout_us = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_AUTO_ACK_TIMEOUT_US,
      .rx_transitions = {
        .success = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_AUTO_ACK_RX_TRANSITION_STATE,
        .error = SL_RAIL_RF_STATE_IDLE // this parameter ignored
      },
      .tx_transitions = {
        .success = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_AUTO_ACK_TX_TRANSITION_STATE,
        .error = SL_RAIL_RF_STATE_IDLE // this parameter ignored
      }
    },
    .timings = {
      .idle_to_tx = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_IDLE_TO_TX_US,
      .idle_to_rx = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_IDLE_TO_RX_US,
      .rx_to_tx = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_RX_TO_TX_US,
      // Make tx_to_rx slightly lower than desired to make sure we get to
      // RX in time.
      .tx_to_rx = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_TX_TO_RX_US,
      .rxsearch_timeout = (SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_ENABLE
                           ? SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_US
                           : 0),
      .tx_to_rxsearch_timeout = (SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_ENABLE
                                 ? SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_US
                                 : 0),
    },
    .frames_mask = (0U // enable appropriate mask bits
                    | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_ACCEPT_BEACON_FRAME_ENABLE
                       ? SL_RAIL_IEEE802154_ACCEPT_BEACON_FRAMES : 0U)
                    | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_ACCEPT_DATA_FRAME_ENABLE
                       ? SL_RAIL_IEEE802154_ACCEPT_DATA_FRAMES : 0U)
                    | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_ACCEPT_ACK_FRAME_ENABLE
                       ? SL_RAIL_IEEE802154_ACCEPT_ACK_FRAMES : 0U)
                    | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_ACCEPT_COMMAND_FRAME_ENABLE
                       ? SL_RAIL_IEEE802154_ACCEPT_COMMAND_FRAMES : 0U)),
    // Enable promiscous mode since no PANID or destination address is
    // specified.
    .promiscuous_mode = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_PROMISCUOUS_MODE_ENABLE,
    .is_pan_coordinator = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_PAN_COORDINATOR_ENABLE,
    .default_frame_pending_in_outgoing_acks = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_DEFAULT_FRAME_PENDING_STATE,
  };
  status = sl_rail_ieee802154_init(handle, &config);
  if (SL_RAIL_STATUS_NO_ERROR == status) {
    switch (protocol) {
      case SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_915MHZ:
        status = sl_rail_ieee802154_config_gb915_mhz_radio(handle);
        break;
      case SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_863MHZ:
        status = sl_rail_ieee802154_config_gb863_mhz_radio(handle);
        break;
      default:
        status = SL_RAIL_STATUS_INVALID_PARAMETER;
        break;
    }
  }
  if (SL_RAIL_STATUS_NO_ERROR != status) {
    (void) sl_rail_ieee802154_deinit(handle);
  } else {
    (void) sl_rail_set_pti_protocol(handle, SL_RAIL_PTI_PROTOCOL_802154);
  }
  return status;
}
#endif // SL_RAIL_SUPPORTS_SUB_GHZ_BAND && SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_ENABLE

#if SL_RAIL_SUPPORTS_SUB_GHZ_BAND && SL_RAIL_UTIL_PROTOCOL_ZWAVE_ENABLE
static sl_rail_status_t sl_rail_util_protocol_config_zwave(sl_rail_handle_t handle,
                                                           sl_rail_util_protocol_type_t protocol)
{
  sl_rail_status_t status;
  sl_rail_zwave_config_t config = {
    .options = 0U // enable appropriate mask bits
               | (SL_RAIL_UTIL_PROTOCOL_ZWAVE_PROMISCUOUS_MODE_ENABLE
                  ? SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_MODE : 0U)
               | (SL_RAIL_UTIL_PROTOCOL_ZWAVE_DETECT_BEAM_FRAME_ENABLE
                  ? SL_RAIL_ZWAVE_OPTION_DETECT_BEAM_FRAMES : 0U)
               | (SL_RAIL_UTIL_PROTOCOL_ZWAVE_NODE_ID_FILTERING_ENABLE
                  ? SL_RAIL_ZWAVE_OPTION_NODE_ID_FILTERING : 0U)
               | (SL_RAIL_UTIL_PROTOCOL_ZWAVE_PROMISCUOUS_BEAM_MODE_ENABLE
                  ? SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_BEAM_MODE : 0U),
    .ack_config = {
      .enable = SL_RAIL_UTIL_PROTOCOL_ZWAVE_AUTO_ACK_ENABLE,
      .ack_timeout_us = SL_RAIL_UTIL_PROTOCOL_ZWAVE_AUTO_ACK_TIMEOUT_US,
      .rx_transitions = {
        .success = SL_RAIL_UTIL_PROTOCOL_ZWAVE_AUTO_ACK_RX_TRANSITION_STATE,
        .error = SL_RAIL_RF_STATE_IDLE // this parameter ignored
      },
      .tx_transitions = {
        .success = SL_RAIL_UTIL_PROTOCOL_ZWAVE_AUTO_ACK_TX_TRANSITION_STATE,
        .error = SL_RAIL_RF_STATE_IDLE // this parameter ignored
      }
    },
    .timings = {
      .idle_to_tx = SL_RAIL_UTIL_PROTOCOL_ZWAVE_TIMING_IDLE_TO_TX_US,
      .idle_to_rx = SL_RAIL_UTIL_PROTOCOL_ZWAVE_TIMING_IDLE_TO_RX_US,
      .rx_to_tx = SL_RAIL_UTIL_PROTOCOL_ZWAVE_TIMING_RX_TO_TX_US,
      // Make tx_to_rx slightly lower than desired to make sure we get to
      // RX in time.
      .tx_to_rx = SL_RAIL_UTIL_PROTOCOL_ZWAVE_TIMING_TX_TO_RX_US,
      .rxsearch_timeout = (SL_RAIL_UTIL_PROTOCOL_ZWAVE_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_ENABLE
                           ? SL_RAIL_UTIL_PROTOCOL_ZWAVE_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_US
                           : 0),
      .tx_to_rxsearch_timeout = (SL_RAIL_UTIL_PROTOCOL_ZWAVE_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_ENABLE
                                 ? SL_RAIL_UTIL_PROTOCOL_ZWAVE_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_US
                                 : 0),
    }
  };
  status = sl_rail_zwave_init(handle, &config);
  if (SL_RAIL_STATUS_NO_ERROR == status) {
    switch (protocol) {
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_ANZ: // Australia
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_anz);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_CN: // China
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_cn);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU: // European Union
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_eu);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_HK: // Hong Kong
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_hk);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_IN: // India
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_in);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_IL: // Israel
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_il);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_JP: // Japan
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_jp);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_KR: // Korea
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_kr);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_MY: // Malaysia
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_my);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_RU: // Russia
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_ru);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_US: // United States
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_us);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_US_LR1: // US, Long Range 1
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_us_lr_1);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_US_LR2: // US, Long Range 2
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_us_lr_2);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_US_LR3: // US, Long Range 3
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_us_lr_3);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU_LR1: // EU, Long Range 1
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_eu_lr_1);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU_LR2: // EU, Long Range 2
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_eu_lr_2);
        break;
      case SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU_LR3: // EU, Long Range 3
        status = sl_rail_zwave_config_region(handle, &sl_rail_zwave_region_eu_lr_3);
        break;
      default:
        status = SL_RAIL_STATUS_INVALID_PARAMETER;
        break;
    }
  }
  if (SL_RAIL_STATUS_NO_ERROR == status) {
    status = sl_rail_zwave_set_node_id(handle, SL_RAIL_ZWAVE_NODE_ID_DEFAULT);
  }
  if (SL_RAIL_STATUS_NO_ERROR != status) {
    (void) sl_rail_zwave_deinit(handle);
  }
  return status;
}
#endif // SL_RAIL_SUPPORTS_SUB_GHZ_BAND && SL_RAIL_UTIL_PROTOCOL_ZWAVE_ENABLE

#if SL_RAIL_SUPPORTS_PROTOCOL_SIDEWALK && SL_RAIL_UTIL_PROTOCOL_SIDEWALK_ENABLE
static sl_rail_status_t sl_rail_util_protocol_config_sidewalk(sl_rail_handle_t handle,
                                                              sl_rail_util_protocol_type_t protocol)
{
  sl_rail_status_t status;
  switch (protocol) {
    case SL_RAIL_UTIL_PROTOCOL_SIDEWALK_2GFSK_50KBPS:
      status = sl_rail_sidewalk_config_phy_2gfsk_50_kbps(handle);
      break;
    default:
      status = SL_RAIL_STATUS_INVALID_PARAMETER;
      break;
  }
  return status;
}
#endif // SL_RAIL_SUPPORTS_PROTOCOL_SIDEWALK && SL_RAIL_UTIL_PROTOCOL_SIDEWALK_ENABLE

sl_rail_status_t sl_rail_util_protocol_config(sl_rail_handle_t handle,
                                              sl_rail_util_protocol_type_t protocol)
{
  switch (protocol) {
    case SL_RAIL_UTIL_PROTOCOL_PROPRIETARY:
      return sl_rail_util_protocol_config_proprietary(handle);
#if SL_RAIL_SUPPORTS_PROTOCOL_BLE && SL_RAIL_UTIL_PROTOCOL_BLE_ENABLE
    case SL_RAIL_UTIL_PROTOCOL_BLE_1MBPS:
    case SL_RAIL_UTIL_PROTOCOL_BLE_2MBPS:
    case SL_RAIL_UTIL_PROTOCOL_BLE_CODED_125KBPS:
    case SL_RAIL_UTIL_PROTOCOL_BLE_CODED_500KBPS:
    case SL_RAIL_UTIL_PROTOCOL_BLE_QUUPPA_1MBPS:
      return sl_rail_util_protocol_config_ble(handle, protocol);
#endif
#if SL_RAIL_IEEE802154_SUPPORTS_2P4_GHZ_BAND  && SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ENABLE
    case SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ:
    case SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ANTDIV:
    case SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_COEX:
    case SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ANTDIV_COEX:
      return sl_rail_util_protocol_config_ieee802154_2p4_ghz(handle, protocol);
#endif
#if SL_RAIL_SUPPORTS_SUB_GHZ_BAND && SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_ENABLE
    case SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_915MHZ:
    case SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_863MHZ:
      return sl_rail_util_protocol_config_ieee802154_gb868(handle, protocol);
#endif
#if SL_RAIL_SUPPORTS_SUB_GHZ_BAND && SL_RAIL_UTIL_PROTOCOL_ZWAVE_ENABLE
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_ANZ: // Australia
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_CN: // China
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU: // European Union
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_HK: // Hong Kong
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_IN: // India
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_IL: // Israel
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_JP: // Japan
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_KR: // Korea
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_MY: // Malaysia
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_RU: // Russia
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_US: // United States
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_US_LR1: // United States, Long Range 1
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_US_LR2: // United States, Long Range 2
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_US_LR3: // United States, Long Range 3
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU_LR1: // European Union, Long Range 1
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU_LR2: // European Union, Long Range 2
    case SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU_LR3: // European Union, Long Range 3
      return sl_rail_util_protocol_config_zwave(handle, protocol);
#endif
#if SL_RAIL_SUPPORTS_PROTOCOL_SIDEWALK && SL_RAIL_UTIL_PROTOCOL_SIDEWALK_ENABLE
    case SL_RAIL_UTIL_PROTOCOL_SIDEWALK_2GFSK_50KBPS:
      return sl_rail_util_protocol_config_sidewalk(handle, protocol);
#endif
    default:
      return SL_RAIL_STATUS_INVALID_PARAMETER;
  }
}
