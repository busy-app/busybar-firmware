/***************************************************************************//**
 * @file sl_rail_sdk_phy_selector.h
 * @brief RAIL SDK - RAIL PHY Selector Component
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_RAIL_SDK_PHY_SELECTOR_H
#define SL_RAIL_SDK_PHY_SELECTOR_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_component_catalog.h"
#include "sl_rail.h"
#if defined(SL_CATALOG_APP_LOG_PRESENT)
#include "app_log.h"
#endif
#if defined(SL_CATALOG_APP_ASSERT_PRESENT)
#include "app_assert.h"
#endif
#include "rail_config.h"

/**
 * \addtogroup rail_sdk_utility
 * @{
 */
/**
 * \addtogroup rail_sdk_phy_selector
 * @{
 */
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/**
 * @brief Set the PHY the app will use.
 * @param[in] new_phy The requested new PHY number.
 * @return 1 if the PHY number is valid, otherwise 0.
 */
uint8_t set_selected_phy(uint16_t new_phy);

/**
 * @brief Get the currently selected PHY number.
 * @return The selected PHY number.
 */
uint16_t get_selected_phy(void);

#endif // SL_RAIL_SDK_PHY_SELECTOR_H
/** @} */ // end of phy_selector group
/** @} */ // end of utility group
