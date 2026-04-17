/***************************************************************************//**
 * @file
 * @brief sl_rail_sdk_util_ble_protocol.c
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
#include "sl_rail_ble.h"
#include "sl_rail_sdk_util_ble_protocol.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
#if SL_RAIL_SUPPORTS_2P4_GHZ_BAND
/**************************************************************************//**
 * This function configures the BLE based on the BLE protocols.
 *
 * @param[in] handle The RAIL handle to apply the radio configuration to.
 * @param[in] protocol The radio configuration type to initialize and configure.(BLE)
 * @return A status code indicating success of the function call.
 *****************************************************************************/
static sl_rail_status_t sl_rail_sdk_util_protocol_config_ble(sl_rail_handle_t handle,
                                                             sl_rail_sdk_util_ble_protocol_type_t protocol);
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
sl_rail_status_t sl_rail_sdk_util_ble_protocol_config(sl_rail_handle_t handle,
                                                      sl_rail_sdk_util_ble_protocol_type_t protocol)
{
  switch (protocol) {
#if SL_RAIL_SUPPORTS_2P4_GHZ_BAND
    case SL_RAIL_UTIL_PROTOCOL_BLE_1MBPS:
    case SL_RAIL_UTIL_PROTOCOL_BLE_2MBPS:
    case SL_RAIL_UTIL_PROTOCOL_BLE_CODED_125KBPS:
    case SL_RAIL_UTIL_PROTOCOL_BLE_CODED_500KBPS:
    case SL_RAIL_UTIL_PROTOCOL_BLE_QUUPPA_1MBPS:
      return sl_rail_sdk_util_protocol_config_ble(handle, protocol);
#endif
    default:
      return SL_RAIL_STATUS_INVALID_PARAMETER;
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
#if SL_RAIL_SUPPORTS_2P4_GHZ_BAND
static sl_rail_status_t sl_rail_sdk_util_protocol_config_ble(sl_rail_handle_t handle,
                                                             sl_rail_sdk_util_ble_protocol_type_t protocol)
{
  sl_rail_status_t status;
  // Override BLE's default timings to get rid of the default rx search timeout
  sl_rail_state_timing_t timings = {
    .idle_to_rx = SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_IDLE_TO_RX_US,
    .tx_to_rx = SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_TX_TO_RX_US,
    .idle_to_tx = SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_IDLE_TO_TX_US,
    .rx_to_tx = SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_RX_TO_TX_US,
    .rxsearch_timeout = SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_ENABLE
                        ? SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_RX_SEARCH_TIMEOUT_AFTER_IDLE_US
                        : 0U,
    .tx_to_rxsearch_timeout = SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_ENABLE
                              ? SL_RAIL_UTIL_PROTOCOL_BLE_TIMING_RX_SEARCH_TIMEOUT_AFTER_TX_US
                              : 0U,
  };

  sl_rail_ble_init(handle);
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
    sl_rail_ble_deinit(handle);
  }
  return status;
}
#endif // SL_RAIL_SUPPORTS_2P4_GHZ_BAND
