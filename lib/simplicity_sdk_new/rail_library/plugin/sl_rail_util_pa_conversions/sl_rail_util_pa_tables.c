/***************************************************************************//**
 * @file
 * @brief Default PA power conversion structures with tables calibrated by the
 *   RAIL team.
 * @details This file contains the tables that convert PA power levels to dBm
 *   powers.
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

// This entire file should never be used on FCC pre-certified modules
#ifndef _SILICON_LABS_MODULE

#include "em_device.h"
#include "sl_rail_util_pa_conversions.h"
#include "sl_rail_util_pa_tables_config.h"

#ifdef SL_RAIL_PA_TABLE_0
static const int32_t sl_rail_table_0[SL_RAIL_PA_TABLE_0_NUM_VALUES] =
  SL_RAIL_PA_TABLE_0;
#endif

#ifdef SL_RAIL_PA_TABLE_1
static const int32_t sl_rail_table_1[SL_RAIL_PA_TABLE_1_NUM_VALUES] =
  SL_RAIL_PA_TABLE_1;
#endif

static sl_rail_pa_descriptor_t p_pa_descriptor[SL_RAIL_TX_PA_MODES_COUNT] =
{
#ifdef SL_RAIL_PA_TABLE_0
  {
    .p_power_setting_table = &sl_rail_table_0[0],
    .min_power_ddbm = SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM,
    .max_power_ddbm = SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM,
    .step_power_ddbm = SL_RAIL_PA_TABLE_0_STEP_DDBM,
    .num_of_values = SL_RAIL_PA_TABLE_0_NUM_VALUES,
  },
#endif
#ifdef SL_RAIL_PA_TABLE_1
  {
    .p_power_setting_table = &sl_rail_table_1[0],
    .min_power_ddbm = SL_RAIL_PA_TABLE_1_MIN_POWER_DDBM,
    .max_power_ddbm = SL_RAIL_PA_TABLE_1_MAX_POWER_DDBM,
    .step_power_ddbm = SL_RAIL_PA_TABLE_1_STEP_DDBM,
    .num_of_values = SL_RAIL_PA_TABLE_1_NUM_VALUES,
  }
#endif
};

const sl_rail_tx_power_table_config_t sl_rail_util_pa_tx_power_table = {
  .p_pa_table_descriptor = &p_pa_descriptor[0],
  .num_of_tables = SL_RAIL_TX_PA_MODES_COUNT,
};

#endif //_SILICON_LABS_MODULE
