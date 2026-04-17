/***************************************************************************//**
 * @file
 * @brief zw_cli_sleeping.h
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
#ifndef ZW_CLI_SLEEPING_H
#define ZW_CLI_SLEEPING_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZW_CLI_COMMON_PRESENT
#include "zw_cli_common.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * CLI - zw_cli_sleeping_util_prevent_sleeping
 *
 * @param[in] arguments true if prevent the application from sleep mode, else false
 * @returns None
 ******************************************************************************/
void zw_cli_sleeping_util_prevent_sleeping(bool is_prevent);

/*******************************************************************************
 * CLI - zw_cli_sleeping_util_prevent_sleeping_timeout
 *
 * @param[in] seconds the number of seconds to prevent the application from entering sleep mode
 * @returns None
 ******************************************************************************/
void zw_cli_sleeping_util_prevent_sleeping_timeout(uint8_t seconds);

#endif // SL_CATALOG_ZW_CLI_COMMON_PRESENT

#ifdef __cplusplus
}
#endif

#endif // ZW_CLI_SLEEPING_H
