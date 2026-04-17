/***************************************************************************//**
 * @file sl_rail_sdk_sleep.h
 * @brief RAIL SDK - Power Manager Sleep Component
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
#ifndef SL_RAIL_SDK_SLEEP_H
#define SL_RAIL_SDK_SLEEP_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_rail_types.h"

/**
 * \addtogroup rail_sdk_utility
 * @{
 */
/**
 * \addtogroup sl_rail_sdk_sleep
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
 * @brief Initializes RAIL and Power Manager to work together in SYNC mode.
 *
 * This function sets up the necessary configurations and prepares the RAIL SDK
 * sleep module for operation. It should be called before using any other
 * functions in the sleep module.
 *
 * @return sl_rail_status_t
 *   - SL_RAIL_STATUS_NO_ERROR if initialization was successful.
 *   - An appropriate error code otherwise.
 */
sl_rail_status_t sl_rail_sdk_sleep_init(void);

#endif // SL_RAIL_SDK_SLEEP_H
/** @} */ // end of sl_rail_sdk_sleep group
/** @} */ // end of utility group
