/***************************************************************************//**
 * @file
 * @brief RAIL Util for FEM VDET configuration file.
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

#ifndef SL_RAIL_UTIL_VDET_CONFIG_H
#define SL_RAIL_UTIL_VDET_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> VDET Configuration

// <o SL_RAIL_UTIL_VDET_MODE> Select VDET mode
// <SL_RAIL_VDET_MODE_DISABLED=> Disable VDET.
// <SL_RAIL_VDET_MODE_AUTOMATIC=> VDET measurements are taken every Tx packet at the specified time.
// <SL_RAIL_VDET_MODE_IMMEDIATE=> VDET measurements are taken immediately when requested.
// <i> Default: SL_RAIL_VDET_MODE_DISABLED
#define SL_RAIL_UTIL_VDET_MODE SL_RAIL_VDET_MODE_DISABLED

// <o SL_RAIL_UTIL_VDET_RESOLUTION> Select VDET resolution
// <SL_RAIL_VDET_RESOLUTION_10_BIT=> ~10 bit resolution
// <SL_RAIL_VDET_RESOLUTION_11_BIT=> ~11 bit resolution
// <SL_RAIL_VDET_RESOLUTION_12_BIT=> ~12 bit resolution
// <i> Default: SL_RAIL_VDET_RESOLUTION_10_BIT
#define SL_RAIL_UTIL_VDET_RESOLUTION SL_RAIL_VDET_RESOLUTION_10_BIT

// <o SL_RAIL_UTIL_VDET_DELAY_US> Select VDET measurement delay in microseconds when in RAIL_VDET_MODE_AUTOMATIC
// <1..150000:1>
// <i> Default: 200
#define SL_RAIL_UTIL_VDET_DELAY_US 200

// </h>

// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>

// VDET SENSE GPIO
// <gpio readonly=true> SL_RAIL_UTIL_VDET_SENSE
// $[GPIO_SL_RAIL_UTIL_VDET_SENSE]
#define SL_RAIL_UTIL_VDET_SENSE_PORT              gpioPortC
#define SL_RAIL_UTIL_VDET_SENSE_PIN               8
// [GPIO_SL_RAIL_UTIL_VDET_SENSE]$

// <<< sl:end pin_tool >>>

#endif // SL_RAIL_UTIL_VDET_CONFIG_H
