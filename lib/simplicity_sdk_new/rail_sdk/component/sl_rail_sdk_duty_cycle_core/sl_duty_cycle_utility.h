/***************************************************************************//**
 * @file sl_duty_cycle_utility.h
 * @brief RAIL SDK - Duty Cycle Core Component
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
#ifndef SL_DUTY_CYCLE_UTILITY_H
#define SL_DUTY_CYCLE_UTILITY_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include "sl_rail_types.h"

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
 * Calculates the proper length of the preamble.
 *
 * @param[in] bit_rate The bitrate used for transmission.
 * @param[in] duty_cycle_config Pointer to a preconfigured structure.
 * @param[in] bit_length Pointer to the function output (preamble bit length).
 *
 * @return sl_rail_status_t   Status code indicating success or failure.
 ******************************************************************************/
sl_rail_status_t calculate_preamble_bit_length_from_time(const uint32_t bit_rate,
                                                         sl_rail_rx_duty_cycle_config_t * duty_cycle_config,
                                                         uint16_t * bit_length);

#endif // SL_DUTY_CYCLE_UTILITY_H
