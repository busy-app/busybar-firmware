/***************************************************************************//**
 * @file
 * @brief Implements an API for initializing and controlling the VDET.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_rail.h"
#include "rail_types.h" // Support RAIL 2.x VDET enums in configs
#include "sl_rail_util_vdet.h"
#include "sl_rail_util_vdet_config.h"

sl_rail_status_t sl_rail_util_vdet_init(void)
{
  sl_rail_vdet_config_t vdet_config = {
    .mode = SL_RAIL_UTIL_VDET_MODE,
    .resolution = SL_RAIL_UTIL_VDET_RESOLUTION,
    .delay_us = SL_RAIL_UTIL_VDET_DELAY_US,
  };

  return sl_rail_config_vdet(SL_RAIL_EFR32_HANDLE, &vdet_config);
}
