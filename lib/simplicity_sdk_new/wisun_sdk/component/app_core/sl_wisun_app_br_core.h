/***************************************************************************//**
 * @file sl_wisun_app_br_core.h
 * @brief Wi-SUN Application Border Router Core
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

#ifndef SL_WISUN_APP_BR_CORE_H
#define SL_WISUN_APP_BR_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @addtogroup SL_WISUN_APP_CORE_API
 * @{
 *****************************************************************************/

#include "sl_wisun_types.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @addtogroup SL_WISUN_APP_CORE_API_TYPES Type definitions
 * @ingroup SL_WISUN_APP_CORE_API
 * @{
 *****************************************************************************/

/// Current address storage structure definition
typedef struct sl_wisun_app_br_core_current_addr {
  /// Link local address
  sl_wisun_ip_address_t addr_ll;
  /// Unique Local Address or Global Unicast Address
  sl_wisun_ip_address_t addr_gua;
  /// DODAG ID
  sl_wisun_ip_address_t addr_dodagid;
} sl_wisun_app_br_core_current_addr_t;

/** @} (end SL_WISUN_APP_CORE_API_TYPES) */
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Start the Border Router functionality.
 * @details This function starts the Border Router functionality
 *          if the Border Router stack component is added to the project.
 *****************************************************************************/
void sl_wisun_app_br_core_start(void);

/** @}*/

#ifdef __cplusplus
}
#endif

#endif // SL_WISUN_APP_BR_CORE_H
