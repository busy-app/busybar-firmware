/***************************************************************************//**
 * @file
 * @brief The WMBUS specific header file for the RAIL library.
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

#ifndef SL_RAIL_WMBUS_H
#define SL_RAIL_WMBUS_H

#include "sl_rail_types.h"

#ifdef __cplusplus
extern "C" {
#endif

///
/// @addtogroup WMBUS Wireless M-Bus
/// @ingroup Protocol_Specific
/// @brief Wireless M-Bus (WMBUS) configuration
/// @{
///

/**
 * @enum sl_rail_wmbus_phy_t
 * @brief The RX variant of the WMBUS T+C PHY.
 */
SLI_RAIL_ENUM(sl_rail_wmbus_phy_t) {
  /**
   * \ref sl_rail_rx_packet_details_t::sub_phy_id indicating a mode T frame A packet.
   */
  SL_RAIL_WMBUS_MODE_T_FRAME_A = 0U,
  /**
   * \ref sl_rail_rx_packet_details_t::sub_phy_id indicating a mode C frame A packet.
   */
  SL_RAIL_WMBUS_MODE_C_FRAME_A = 2U,
  /**
   * \ref sl_rail_rx_packet_details_t::sub_phy_id indicating a mode C frame B packet.
   */
  SL_RAIL_WMBUS_MODE_C_FRAME_B = 3U,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_WMBUS_MODE_T_FRAME_A ((sl_rail_wmbus_phy_t) SL_RAIL_WMBUS_MODE_T_FRAME_A)
#define SL_RAIL_WMBUS_MODE_C_FRAME_A ((sl_rail_wmbus_phy_t) SL_RAIL_WMBUS_MODE_C_FRAME_A)
#define SL_RAIL_WMBUS_MODE_C_FRAME_B ((sl_rail_wmbus_phy_t) SL_RAIL_WMBUS_MODE_C_FRAME_B)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/**
 * Configure WMBUS simultaneous M2O RX of T and C modes feature.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable_simultaneous_t_c_rx true to enable WMBUS simultaneous M2O RX of T and C modes.
 * @return Status code indicating success of the function call.
 *
 * If simultaneous M2O RX of T and C modes is enabled, when
 * PHY_wMbus_ModeTC_M2O_100k_frameA is loaded, mode T Frame A and mode C frame
 * A/B can be received. The mode and frame type of the last received packet is
 * available in \ref sl_rail_rx_packet_details_t::sub_phy_id.
 *
 * @note This WMBUS feature is supported only on some parts.
 * The preprocessor symbol \ref SL_RAIL_WMBUS_SUPPORTS_SIMULTANEOUS_T_C_RX and the
 * runtime function \ref sl_rail_wmbus_supports_simultaneous_t_c_rx() may be used to
 * test for support.
 */
sl_rail_status_t sl_rail_wmbus_config(sl_rail_handle_t rail_handle,
                                      bool enable_simultaneous_t_c_rx);

/// @} // End of group WMBUS

#ifdef __cplusplus
}
#endif

#endif // SL_RAIL_WMBUS_H
