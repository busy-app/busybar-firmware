/***************************************************************************//**
 * @file
 * @brief
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

#include "sl_rail.h"
#include "sl_rail_ble.h"
#include "sl_rail_util_cs_antenna_offset.h"
#include "sl_rail_util_cs_antenna_offset_config.h"

void sl_rail_util_cs_antenna_offset_init(void)
{
  _Static_assert(((SL_RAIL_UTIL_CS_ANTENNA_COUNT >= 1) && (SL_RAIL_UTIL_CS_ANTENNA_COUNT <= SL_RAIL_BLE_CS_MAX_ANTENNAS)),
                 "SL_RAIL_UTIL_CS_ANTENNA_COUNT is set to an invalid value.");

  const int16_t sl_rail_util_antenna_offset_cm[SL_RAIL_BLE_CS_MAX_ANTENNAS] =
#if SL_RAIL_UTIL_CS_ANTENNA_CONFIG_TYPE == SL_RAIL_UTIL_CS_ANTENNA_OFFSET_CONFIG_TYPE_WIRED
    SL_RAIL_UTIL_CS_ANTENNA_OFFSET_WIRED_CM;
#elif SL_RAIL_UTIL_CS_ANTENNA_CONFIG_TYPE == SL_RAIL_UTIL_CS_ANTENNA_OFFSET_CONFIG_TYPE_WIRELESS
    SL_RAIL_UTIL_CS_ANTENNA_OFFSET_WIRELESS_CM;
#else
  #error "A wireless or wired antenna configuration must be selected!"
#endif

  sl_rail_ble_cs_antenna_config_t cs_antenna_config = {
    .antenna_count = SL_RAIL_UTIL_CS_ANTENNA_COUNT,
    .p_antenna_offset_cm = sl_rail_util_antenna_offset_cm,
  };
  sl_rail_ble_config_cs_antenna(SL_RAIL_EFR32_HANDLE, &cs_antenna_config);
}
