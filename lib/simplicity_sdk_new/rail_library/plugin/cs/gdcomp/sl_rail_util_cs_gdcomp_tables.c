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

#if SL_RAIL_BLE_SUPPORTS_CS
#include "sl_rail_util_cs_gdcomp_config.h"

// 0dBm PA LUTs
static const int16_t gdcompPbrPhaseLsb0Dbm[SL_RAIL_UTIL_CS_GDCOMP_TABLE_LENGTH] =
  SL_RAIL_UTIL_CS_GDCOMP_PBR_0DBM_PA;
static const int16_t gdcompRttSlope0Dbm[SL_RAIL_UTIL_CS_GDCOMP_TABLE_LENGTH] =
  SL_RAIL_UTIL_CS_GDCOMP_RTT_SLOPE_0DBM_PA;
static const int16_t gdcompRttOffset0Dbm[SL_RAIL_UTIL_CS_GDCOMP_TABLE_LENGTH] =
  SL_RAIL_UTIL_CS_GDCOMP_RTT_OFFSET_0DBM_PA;

SLI_LIBRAIL_WEAK
const sl_rail_ble_cs_gd_comp_tables_t sl_rail_ble_cs_gd_comp_0_dbm_tables = {
  .p_pbr_phase_lsb = gdcompPbrPhaseLsb0Dbm,
  .p_rtt_slope = gdcompRttSlope0Dbm,
  .p_rtt_offset = gdcompRttOffset0Dbm,
  .table_length_halfwords = SL_RAIL_UTIL_CS_GDCOMP_TABLE_LENGTH,
};

// 10dBm PA LUTs
static const int16_t gdcompPbrPhaseLsb10Dbm[SL_RAIL_UTIL_CS_GDCOMP_TABLE_LENGTH] =
  SL_RAIL_UTIL_CS_GDCOMP_PBR_10DBM_PA;
static const int16_t gdcompRttSlope10Dbm[SL_RAIL_UTIL_CS_GDCOMP_TABLE_LENGTH] =
  SL_RAIL_UTIL_CS_GDCOMP_RTT_SLOPE_10DBM_PA;
static const int16_t gdcompRttOffset10Dbm[SL_RAIL_UTIL_CS_GDCOMP_TABLE_LENGTH] =
  SL_RAIL_UTIL_CS_GDCOMP_RTT_OFFSET_10DBM_PA;

SLI_LIBRAIL_WEAK
const sl_rail_ble_cs_gd_comp_tables_t sl_rail_ble_cs_gd_comp_10_dbm_tables = {
  .p_pbr_phase_lsb = gdcompPbrPhaseLsb10Dbm,
  .p_rtt_slope = gdcompRttSlope10Dbm,
  .p_rtt_offset = gdcompRttOffset10Dbm,
  .table_length_halfwords = SL_RAIL_UTIL_CS_GDCOMP_TABLE_LENGTH,
};

// 20dBm PA LUTs
static const int16_t gdcompPbrPhaseLsb20Dbm[SL_RAIL_UTIL_CS_GDCOMP_TABLE_LENGTH] =
  SL_RAIL_UTIL_CS_GDCOMP_PBR_20DBM_PA;
static const int16_t gdcompRttSlope20Dbm[SL_RAIL_UTIL_CS_GDCOMP_TABLE_LENGTH] =
  SL_RAIL_UTIL_CS_GDCOMP_RTT_SLOPE_20DBM_PA;
static const int16_t gdcompRttOffset20Dbm[SL_RAIL_UTIL_CS_GDCOMP_TABLE_LENGTH] =
  SL_RAIL_UTIL_CS_GDCOMP_RTT_OFFSET_20DBM_PA;

SLI_LIBRAIL_WEAK
const sl_rail_ble_cs_gd_comp_tables_t sl_rail_ble_cs_gd_comp_20_dbm_tables = {
  .p_pbr_phase_lsb = gdcompPbrPhaseLsb20Dbm,
  .p_rtt_slope = gdcompRttSlope20Dbm,
  .p_rtt_offset = gdcompRttOffset20Dbm,
  .table_length_halfwords = SL_RAIL_UTIL_CS_GDCOMP_TABLE_LENGTH,
};

#endif // SL_RAIL_BLE_SUPPORTS_CS
