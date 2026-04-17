/***************************************************************************//**
 * @file
 * @brief PA power conversion types used by Silicon Labs PA power
 *   conversion functions.
 * @details This file contains the types needed convert PA power levels
 *   to dBm powers.
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

#ifndef SL_RAIL_UTIL_PA_TYPES_H
#define SL_RAIL_UTIL_PA_TYPES_H

#include "sl_rail_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup PA_Power_Conversions PA Power Conversions
 * @ingroup PA
 * @{
 */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#if defined(SL_RAIL_UTIL_PA_POWERSETTING_TABLE_VERSION)
#if RAIL_SUPPORTS_COMMON_PA_INTERFACE \
  || SL_RAIL_SUPPORTS_COMMON_PA_INTERFACE
#if SL_RAIL_UTIL_PA_POWERSETTING_TABLE_VERSION == 1
/**
 * Mask for submode.
 * An entry in the powersetting table has the below bitfields:
 * |15-14 =sub-mode|13-8:unused|7-0:scalor(stripe+slice)|
 */
#define SLI_RAIL_UTIL_PA_TABLE_SUBMODE_MASK 0xC000UL
/**
 * Shift for submode.
 */
#define SLI_RAIL_UTIL_PA_TABLE_SUBMODE_SHIFT 14U
/**
 * Mask for scalor.
 */
#define SLI_RAIL_UTIL_PATABLE_SCALOR_MASK 0xFFU
/**
 * Shift for scalor.
 */
#define SLI_RAIL_UTIL_PA_TABLE_SCALOR_SHIFT 0U
#endif //SL_RAIL_UTIL_PA_POWERSETTING_TABLE_VERSION == 1
#endif //RAIL_SUPPORTS_COMMON_PA_INTERFACE
#endif //defined(SL_RAIL_UTIL_PA_POWERSETTING_TABLE_VERSION)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/**
 * @struct sl_rail_pa_descriptor_t
 *
 * @brief Struct containing specifics of PA configuration.
 *   PA descriptor as used in the PA conversion functions.
 */
typedef struct sl_rail_pa_descriptor {
  /**
   * Lookup table for PA's to convert
   * between deci-dBm and power levels.
   */
  const int32_t *p_power_setting_table;
  /** Min power in deci-dBm for this PA. */
  sl_rail_tx_power_t min_power_ddbm;
  /** Max power in deci-dBm for this PA. */
  sl_rail_tx_power_t max_power_ddbm;
  /** Table step size in deci-dBm for this PA. */
  sl_rail_tx_power_t step_power_ddbm;
  /**
   * The number of powersetting entries present in the table.
   */
  uint16_t num_of_values;
#if 0
  /** Rounding mode used to generate the table. */
  //TODO : Add the enum
  sl_rail_pa_rounding_mode_t pa_rounding_mode;
#endif
} sl_rail_pa_descriptor_t;

/**
 * @struct sl_rail_tx_power_table_config_t
 *
 * @brief More generic structure containing information about
 *   mapping tables, instead of specific PA's.
 */
typedef struct sl_rail_tx_power_table_config {
  /** Descriptor for each PA. */
  sl_rail_pa_descriptor_t *p_pa_table_descriptor;
  /** Signature used for validation of the configuruation. */
  uint32_t signature;
  /** PA VDD voltage, in millivolts. */
  uint16_t pa_voltage_mv;
  /** Number of tables available */
  uint8_t num_of_tables;
  /** Padding to align the structures to 32bits */
  uint8_t padding;
} sl_rail_tx_power_table_config_t;

/** @} */ // PA_Power_Conversions

#ifdef __cplusplus
}
#endif

#endif // SL_RAIL_PA_UITL_TYPES_H
