/***************************************************************************//**
 * @file
 * @brief sl_rail_sdk_util_802154_protocol.c
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <string.h>
#include "sl_rail.h"
#include "sl_rail_ieee802154.h"
#include "sl_rail_sdk_util_802154_protocol.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
#if SL_RAIL_SUPPORTS_2P4_GHZ_BAND
/**************************************************************************//**
 * This function configures the RAIL for IEEE 802.15.4 on 2.4GHz protocol
 *
 * @param[in] handle      RAIL handle
 * @param[in] protocol    IEEE 802.15.4 protocol
 *
 * @return RAIL status, SL_RAIL_STATUS_NO_ERROR if the configuration has been successful.
 *****************************************************************************/
static sl_rail_status_t sl_rail_sdk_util_protocol_config_ieee802154_2p4ghz(sl_rail_handle_t handle,
                                                                           sl_rail_sdk_util_802154_protocol_type_t protocol);
#endif

#if SL_RAIL_SUPPORTS_SUB_GHZ_BAND && defined(SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_AUTO_ACK_ENABLE)
/**************************************************************************//**
 * This function configures the RAIL for IEEE 802.15.4 on 863MHz protocol
 *
 * @param[in] handle      RAIL handle
 * @param[in] protocol    IEEE 802.15.4 protocol
 *
 * @return RAIL status, SL_RAIL_STATUS_NO_ERROR if the configuration has been successful.
 *****************************************************************************/
static sl_rail_status_t sl_rail_sdk_util_protocol_config_ieee802154_gb868_863mhz(sl_rail_handle_t handle,
                                                                                 sl_rail_sdk_util_802154_protocol_type_t protocol);

/**************************************************************************//**
 * This function configures the RAIL for IEEE 802.15.4 on 915MHz protocol
 *
 * @param[in] handle      RAIL handle
 * @param[in] protocol    IEEE 802.15.4 protocol
 *
 * @return RAIL status, SL_RAIL_STATUS_NO_ERROR if the configuration has been successful.
 *****************************************************************************/
static sl_rail_status_t sl_rail_sdk_util_protocol_config_ieee802154_gb868_915mhz(sl_rail_handle_t handle,
                                                                                 sl_rail_sdk_util_802154_protocol_type_t protocol);
#endif

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
sl_rail_status_t sl_rail_sdk_util_802154_protocol_config(sl_rail_handle_t handle,
                                                         sl_rail_sdk_util_802154_protocol_type_t protocol)
{
  switch (protocol) {
#if SL_RAIL_SUPPORTS_2P4_GHZ_BAND
    case SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_2P4GHZ:
    case SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ANTDIV:
    case SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_2P4GHZ_COEX:
    case SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ANTDIV_COEX:
      return sl_rail_sdk_util_protocol_config_ieee802154_2p4ghz(handle, protocol);
#endif
#if SL_RAIL_SUPPORTS_SUB_GHZ_BAND && defined(SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_AUTO_ACK_ENABLE)
    case SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_GB868_915MHZ:
      return sl_rail_sdk_util_protocol_config_ieee802154_gb868_915mhz(handle, protocol);
    case SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_GB868_863MHZ:
      return sl_rail_sdk_util_protocol_config_ieee802154_gb868_863mhz(handle, protocol);
#endif
    default:
      return SL_RAIL_STATUS_INVALID_PARAMETER;
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
#if SL_RAIL_SUPPORTS_2P4_GHZ_BAND
static sl_rail_status_t sl_rail_sdk_util_protocol_config_ieee802154_2p4ghz(sl_rail_handle_t handle,
                                                                           sl_rail_sdk_util_802154_protocol_type_t protocol)
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
      .rxsearch_timeout = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_ENABLE
                          ? SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_US
                          : 0,
      .tx_to_rxsearch_timeout = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_ENABLE
                                ? SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_US
                                : 0
    },
    .frames_mask = 0U // enable appropriate mask bits
                   | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ACCEPT_BEACON_FRAME_ENABLE
                      ? SL_RAIL_IEEE802154_ACCEPT_BEACON_FRAMES : 0U)
                   | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ACCEPT_DATA_FRAME_ENABLE
                      ? SL_RAIL_IEEE802154_ACCEPT_DATA_FRAMES : 0U)
                   | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ACCEPT_ACK_FRAME_ENABLE
                      ? SL_RAIL_IEEE802154_ACCEPT_ACK_FRAMES : 0U)
                   | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ACCEPT_COMMAND_FRAME_ENABLE
                      ? SL_RAIL_IEEE802154_ACCEPT_COMMAND_FRAMES : 0U),
    // Enable promiscuous mode since no PANID or destination address is
    // specified.
    .promiscuous_mode = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_PROMISCUOUS_MODE_ENABLE,
    .is_pan_coordinator = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_PAN_COORDINATOR_ENABLE,
    .default_frame_pending_in_outgoing_acks = SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_DEFAULT_FRAME_PENDING_STATE,
  };
  status = sl_rail_ieee802154_init(handle, &config);
  if (SL_RAIL_STATUS_NO_ERROR == status) {
    switch (protocol) {
      case SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_2P4GHZ:
        status = sl_rail_ieee802154_config_2p4_ghz_radio(handle);
        break;
      case SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ANTDIV:
        status = sl_rail_ieee802154_config_2p4_ghz_radio_ant_div(handle);
        break;
      case SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_2P4GHZ_COEX:
        status = sl_rail_ieee802154_config_2p4_ghz_radio_coex(handle);
        break;
      case SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ANTDIV_COEX:
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
#endif // SL_RAIL_SUPPORTS_2P4_GHZ_BAND

#if SL_RAIL_SUPPORTS_SUB_GHZ_BAND && defined(SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_AUTO_ACK_ENABLE)
static sl_rail_status_t sl_rail_sdk_util_protocol_config_ieee802154_gb868_863mhz(sl_rail_handle_t handle,
                                                                                 sl_rail_sdk_util_802154_protocol_type_t protocol)
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
      .rxsearch_timeout = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_ENABLE
                          ? SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_US
                          : 0,
      .tx_to_rxsearch_timeout = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_ENABLE
                                ? SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_US
                                : 0
    },
    .frames_mask = 0U // enable appropriate mask bits
                   | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_ACCEPT_BEACON_FRAME_ENABLE
                      ? SL_RAIL_IEEE802154_ACCEPT_BEACON_FRAMES : 0U)
                   | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_ACCEPT_DATA_FRAME_ENABLE
                      ? SL_RAIL_IEEE802154_ACCEPT_DATA_FRAMES : 0U)
                   | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_ACCEPT_ACK_FRAME_ENABLE
                      ? SL_RAIL_IEEE802154_ACCEPT_ACK_FRAMES : 0U)
                   | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_ACCEPT_COMMAND_FRAME_ENABLE
                      ? SL_RAIL_IEEE802154_ACCEPT_COMMAND_FRAMES : 0U),
    // Enable promiscuous mode since no PANID or destination address is
    // specified.
    .promiscuous_mode = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_PROMISCUOUS_MODE_ENABLE,
    .is_pan_coordinator = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_PAN_COORDINATOR_ENABLE,
    .default_frame_pending_in_outgoing_acks = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_DEFAULT_FRAME_PENDING_STATE,
  };
  status = sl_rail_ieee802154_init(handle, &config);
  if (SL_RAIL_STATUS_NO_ERROR == status) {
    if (protocol == SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_GB868_863MHZ) {
      status = sl_rail_ieee802154_config_gb863_mhz_radio(handle);
    } else {
      status = SL_RAIL_STATUS_INVALID_PARAMETER;
    }
  }
  if (SL_RAIL_STATUS_NO_ERROR != status) {
    (void) sl_rail_ieee802154_deinit(handle);
    return status;
  }

  // G option set
  status = sl_rail_ieee802154_config_g_options(handle,
                                               SL_RAIL_IEEE802154_G_OPTION_GB868,
                                               SL_RAIL_IEEE802154_G_OPTION_GB868);

  return status;
}

static sl_rail_status_t sl_rail_sdk_util_protocol_config_ieee802154_gb868_915mhz(sl_rail_handle_t handle,
                                                                                 sl_rail_sdk_util_802154_protocol_type_t protocol)
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
      .rxsearch_timeout = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_ENABLE
                          ? SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_US
                          : 0,
      .tx_to_rxsearch_timeout = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_ENABLE
                                ? SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_US
                                : 0
    },
    .frames_mask = 0U // enable appropriate mask bits
                   | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_ACCEPT_BEACON_FRAME_ENABLE
                      ? SL_RAIL_IEEE802154_ACCEPT_BEACON_FRAMES : 0U)
                   | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_ACCEPT_DATA_FRAME_ENABLE
                      ? SL_RAIL_IEEE802154_ACCEPT_DATA_FRAMES : 0U)
                   | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_ACCEPT_ACK_FRAME_ENABLE
                      ? SL_RAIL_IEEE802154_ACCEPT_ACK_FRAMES : 0U)
                   | (SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_ACCEPT_COMMAND_FRAME_ENABLE
                      ? SL_RAIL_IEEE802154_ACCEPT_COMMAND_FRAMES : 0U),
    // Enable promiscuous mode since no PANID or destination address is
    // specified.
    .promiscuous_mode = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_PROMISCUOUS_MODE_ENABLE,
    .is_pan_coordinator = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_PAN_COORDINATOR_ENABLE,
    .default_frame_pending_in_outgoing_acks = SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_DEFAULT_FRAME_PENDING_STATE,
  };
  status = sl_rail_ieee802154_init(handle, &config);
  if (SL_RAIL_STATUS_NO_ERROR == status) {
    if (protocol == SL_RAIL_SDK_UTIL_PROTOCOL_IEEE802154_GB868_915MHZ) {
      status = sl_rail_ieee802154_config_gb915_mhz_radio(handle);
    } else {
      status = SL_RAIL_STATUS_INVALID_PARAMETER;
    }
  }
  if (SL_RAIL_STATUS_NO_ERROR != status) {
    (void) sl_rail_ieee802154_deinit(handle);
    return status;
  }

  // G option set
  status = sl_rail_ieee802154_config_g_options(handle,
                                               SL_RAIL_IEEE802154_G_OPTION_GB868,
                                               SL_RAIL_IEEE802154_G_OPTION_GB868);
  return status;
}
#endif // SL_RAIL_SUPPORTS_SUB_GHZ_BAND
