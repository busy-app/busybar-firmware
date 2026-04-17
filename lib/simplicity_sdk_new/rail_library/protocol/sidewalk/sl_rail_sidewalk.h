/***************************************************************************//**
 * @file
 * @brief The Sidewalk specific header file for the RAIL library.
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

#ifndef SL_RAIL_SIDEWALK_H
#define SL_RAIL_SIDEWALK_H

#ifdef __cplusplus
extern "C" {
#endif

// Get the standard include types
#include <stdint.h>
#include <stdbool.h>

// Get the RAIL specific structures and types
#include "sl_rail_types.h"

///
/// @addtogroup SIDEWALK_PHY Sidewalk Radio Configurations
/// @ingroup Protocol_Specific
/// Radio configurations for the RAIL Sidewalk Accelerator
///
/// These radio configurations are used to configure Sidewalk when a function
/// such as \ref sl_rail_sidewalk_config_phy_2gfsk_50_kbps() is called. Each radio
/// configuration listed below is compiled into the RAIL library as a weak
/// symbol that will take into account per-die defaults. If the board
/// configuration in use has different settings than the default, such as a
/// different radio subsystem clock frequency, these radio configurations can
/// be overriden to account for those settings.
/// @{
///

/**
 * Default PHY to use for Sidewalk 2GFSK 50kbps. Will be NULL if
 * \ref SL_RAIL_SUPPORTS_PROTOCOL_SIDEWALK is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_sidewalk_phy_2gfsk_50_kbps;

/**
 * @enum sl_rail_sidewalk_phy_t
 * @brief The Sidewalk PHY identifier (aka PTI radio configuration id).
 */
SLI_RAIL_ENUM(sl_rail_sidewalk_phy_t) {
  /** Use the 2GFSK 50 kbps Sidewalk PHY */
  SL_RAIL_SIDEWALK_PHY_2GFSK_50_KBPS = 0u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_SIDEWALK_2GFSK_50_KBPS ((sl_rail_sidewalk_phy_t)SL_RAIL_SIDEWALK_2GFSK_50_KBPS)
#endif

/**
 * Switch to the 2GFSK 50 kbps Sidewalk PHY.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * Use this function to switch to the 2GFSK 50 kbps Sidewalk PHY.
 *
 * @note The Sidewalk PHY is supported only on some parts.
 *   The preprocessor symbol \ref SL_RAIL_SUPPORTS_PROTOCOL_SIDEWALK and the
 *   runtime function \ref sl_rail_supports_protocol_sidewalk() may be used to
 *   test for support of the Sidewalk PHY.
 */
sl_rail_status_t sl_rail_sidewalk_config_phy_2gfsk_50_kbps(sl_rail_handle_t rail_handle);

/// @} // End of group SIDEWALK_PHY

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// Temporary RAIL 3.x API mappings to RAIL 2.x APIs

#include "rail_sidewalk.h"
#define sl_rail_sidewalk_phy_2gfsk_50_kbps \
  ((const sl_rail_channel_config_t *const)RAIL_Sidewalk_Phy2GFSK50kbps)

#endif//DOXYGEN_SHOULD_SKIP_THIS

#ifdef __cplusplus
}
#endif

#endif // SL_RAIL_SIDEWALK_H
