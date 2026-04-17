/***************************************************************************//**
 * @file
 * @brief PA Tables configuration file.
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

#ifndef SL_RAIL_UTIL_PA_TABLES_CONFIG_H
#define SL_RAIL_UTIL_PA_TABLES_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> PA Table Configuration
// <o SL_RAIL_UTIL_PA_TABLE_HEADER> PA Table Selection
// <"sl_rail_util_pa_dbm_powersetting_mapping_table_qfn_8dbm.h"=> 8dBm PA powersetting mapping table
// <"sl_rail_util_pa_dbm_powersetting_mapping_table_qfn_0dbm.h"=> 0dBm PA powersetting mapping table
// <"sl_rail_util_pa_dbm_powersetting_mapping_table_qfn_automode_0_8dbm.h"=> 0dBm-8dBm automode PA powersetting mapping table
// <i> Default: "sl_rail_util_pa_dbm_powersetting_mapping_table_qfn_automode_0_8dbm.h"
#define SL_RAIL_UTIL_PA_TABLE_HEADER    "sl_rail_util_pa_dbm_powersetting_mapping_table_qfn_automode_0_8dbm.h"
// </h>

// <<< end of configuration section >>>

#include SL_RAIL_UTIL_PA_TABLE_HEADER

#endif // SL_RAIL_UTIL_PA_TABLES_CONFIG_H
