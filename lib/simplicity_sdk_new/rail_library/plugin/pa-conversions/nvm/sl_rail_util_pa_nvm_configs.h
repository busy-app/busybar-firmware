/***************************************************************************//**
 * @file
 * @brief This provides functions to store and retrieve PA configs from NVM3
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

#ifndef SL_RAIL_UTIL_PA_NVM_CONFIGS_H
#define SL_RAIL_UTIL_PA_NVM_CONFIGS_H

#include "sl_status.h"
#include "rail_types.h"
#include "pa_curve_types_efr32.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup PA Power Amplifier (PA)
 * @ingroup Transmit
 * @{
 */

// | Region | SubRegion            | NVM3 Key<19:16> | NVM3 Key<15:0>
// | Common | Antenna Calibration  | 0x8             | 0x9800 - 0x98ff
// where Platform owns 0x89800-0x8987f and RAIL owns 0x89880-0x898ff.
/**
 * The NVM3 PA mode tag.
 */
#define SL_RAIL_UTIL_PA_NVM_MODE_TAG   (0x89881UL)
/**
 * The NVM3 PA config tag.
 */
#define SL_RAIL_UTIL_PA_NVM_CONFIG_TAG (0x89882UL)

/**
 * The version number of the NVM structures defined herein.
 */
#define SL_RAIL_NVM_PA_CONFIGS_VERSION 1

/**
 * The number of curves/tables allowed in NVM storage.
 * EFR32xG24 and EFR32xG26 each have 2 PAs.
 */
#define SL_RAIL_NVM_PA_COUNT RAIL_NUM_PA

/**
 * The number of curve segments allowed per curve in NVM storage.
 * Currently suitable only for EFR32xG24 and EFR32xG26.
 */
#define SL_RAIL_NVM_PA_CURVE_SEGMENTS 9

/**
 * The number of table values allowed per mapping table in NVM storage.
 * Currently suitable only for EFR32xG24 and EFR32xG26.
 */
#define SL_RAIL_NVM_PA_TABLE_ENTRIES 16

/**
 * @struct sl_rail_nvm_pa_curve_t
 * @brief NVM-suitable PA curve structure to map to a
 *   \ref RAIL_TxPowerCurveAlt_t.
 *
 * The size of this structure must be a multiple of 4 bytes.
 */
typedef struct sl_rail_nvm_pa_curve {
  /** Minimum deci-dBm value. */
  int16_t curve_min_ddbm;
  /** Maximum deci-dBm value. */
  int16_t curve_max_ddbm;
  /** Curve segments. */
  RAIL_TxPowerCurveSegment_t curve_segments[SL_RAIL_NVM_PA_CURVE_SEGMENTS];
} sl_rail_nvm_pa_curve_t;

/**
 * @struct sl_rail_nvm_pa_table_t
 * @brief NVM-suitable PA mapping table structure to map to a
 *   \ref RAIL_PowerConversion_t::mappingTable.
 *
 * The size of this structure must be a multiple of 4 bytes.
 */
typedef struct sl_rail_nvm_pa_table {
  /** Mapping table of deci-dBm power values. */
  int16_t ddbm_values[SL_RAIL_NVM_PA_TABLE_ENTRIES];
} sl_rail_nvm_pa_table_t;

/**
 * @struct sl_rail_nvm_pa_descriptor_t
 * @brief NVM-suitable PA descriptor structure to map to a
 *   \ref RAIL_PaDescriptor_t.
 *
 * The size of this structure must be a multiple of 4 bytes.
 */
typedef struct sl_rail_nvm_pa_descriptor {
  /** Algorithm used to map dBm to power levels for this PA. */
  RAIL_PaConversionAlgorithm_t algorithm;
  /**
   * The number of curve piecewise segments or mapping table entries
   * provided for the PA according to its algorithm.
   * This should not exceed \ref SL_RAIL_NVM_PA_CURVE_SEGMENTS
   * for a PA using \ref RAIL_PA_ALGORITHM_PIECEWISE_LINEAR or
   * \ref SL_RAIL_NVM_PA_TABLE_ENTRIES for a PA using
   * \ref RAIL_PA_ALGORITHM_MAPPING_TABLE.
   */
  uint8_t num_segments_or_entries;
  /** Minimum power level for this PA. */
  RAIL_TxPowerLevel_t min;
  /** Maximum power level for this PA. */
  RAIL_TxPowerLevel_t max;
} sl_rail_nvm_pa_descriptor_t;

/**
 * @struct sl_rail_nvm_pa_config_t
 * @brief NVM-suitable PA curves/tables configuration structure to map to a
 *   \ref RAIL_TxPowerCurvesConfigAlt_t.
 */
typedef struct sl_rail_nvm_pa_config {
  /**
   * Version number of this structure.
   * Should correspond to \ref SL_RAIL_NVM_PA_CONFIGS_VERSION.
   */
  uint8_t version;
  /**
   * Number of descriptors described below in pa_descriptors[].
   * Must equal \ref SL_RAIL_NVM_PA_COUNT.
   */
  uint8_t num_descriptors;
  /** PA VDD voltage, in millivolts. */
  uint16_t pa_voltage;
  /** Signature used for validation of the curves configuration. */
  uint32_t signature;
  /** The PA data descriptor, one for each PA. */
  sl_rail_nvm_pa_descriptor_t pa_descriptors[SL_RAIL_NVM_PA_COUNT];
  /** Union for either a curve or table. */
  union {
    /** PA piecewise-linear curve. */
    sl_rail_nvm_pa_curve_t curve;
    /** PA raw to dBm mapping table. */
    sl_rail_nvm_pa_table_t table;
  } pa_curve_or_table[SL_RAIL_NVM_PA_COUNT];
} sl_rail_nvm_pa_config_t;

/**
 * Store a \ref RAIL_TxPowerMode_t into NVM.
 *
 * @param[in] pa_mode The PA mode.
 * @return Status code indicating success of the function call.
 */
sl_status_t sl_rail_util_pa_nvm_write_mode(RAIL_TxPowerMode_t pa_mode);

/**
 * Reads stored \ref RAIL_TxPowerMode_t from NVM.
 *
 * @return The stored PA mode, or RAIL_TX_POWER_MODE_NONE
 *   if no stored PA mode exists or is invalid.
 */
RAIL_TxPowerMode_t sl_rail_util_pa_nvm_read_mode(void);

/**
 * Stores a \ref sl_rail_nvm_pa_config_t into NVM.
 * @param[in] p_pa_config A non-NULL pointer to PA curves/table config.
 * @return Status code indicating success of the function call.
 */
sl_status_t sl_rail_util_pa_nvm_write_config(const sl_rail_nvm_pa_config_t *p_pa_config);

/**
 * Reads stored \ref sl_rail_nvm_pa_config_t from NVM.
 * @param[out] p_pa_config A non-NULL pointer to the PA curves/table config
 *   read from NVM.
 * @return Status code indicating success of the function call.
 */
sl_status_t sl_rail_util_pa_nvm_read_config(sl_rail_nvm_pa_config_t *p_pa_config);

#ifndef DOXYGEN_UNDOCUMENTED

/**
 * Convert \ref sl_rail_nvm_pa_config_t to \ref RAIL_TxPowerCurvesConfigAlt_t.
 * @param[in] p_pa_config A non-NULL pointer to the NVM PA curves/table config
 *   to convert.
 * @return A pointer to global or internally-allocated RAM storage containing
 *   the conversion, or NULL if there was a problem with the conversion.
 *   This storage shall remain intact until the next call to this function.
 */
RAIL_TxPowerCurvesConfigAlt_t *sli_rail_util_pa_nvm_deserialize_config(const sl_rail_nvm_pa_config_t *p_pa_config);

/**
 * Convert \ref RAIL_TxPowerCurvesConfigAlt_t to \ref sl_rail_nvm_pa_config_t.
 * @param[in] p_tx_curves A non-NULL pointer to the \ref
     RAIL_TxPowerCurvesConfigAlt_t to convert.
 * @param[out] p_pa_config A non-NULL pointer to an NVM PA curves/table config
 *    for the conversion result.
 * @return Status code indicating success of the function call.
 */
sl_status_t sli_rail_util_pa_nvm_serialize_config(const RAIL_TxPowerCurvesConfigAlt_t *p_tx_curves,
                                                  sl_rail_nvm_pa_config_t *p_pa_config);

#if     RAILTEST

/**
 * Print serialized PA TX power configuration in C initializer syntax.
 * @param[in] p_pa_config A non-NULL pointer to an NVM PA curves/table config.
 * @param[in] with_offsets true to show field offsets within the structure.
 */
void sli_rail_util_pa_nvm_print_serialized(const sl_rail_nvm_pa_config_t *p_pa_config,
                                           bool with_offsets);

/**
 * Print serialized PA TX power configuration in JSON syntax.
 * @param[in] p_pa_config A non-NULL pointer to an NVM PA curves/table config.
 */
void sli_rail_util_pa_nvm_print_serialized_json(const sl_rail_nvm_pa_config_t *p_pa_config);

/**
 * Print deserialized PA Tx Power configuration in C initializer syntax.
 * @param[in] p_pa_config A non-NULL pointer to a \ref
 *   RAIL_TxPowerCurvesConfigAlt_t structure.
 */
void sli_rail_util_pa_nvm_print_deserialized(const RAIL_TxPowerCurvesConfigAlt_t *p_tx_curves);

#endif//RAILTEST

#endif//DOXYGEN_UNDOCUMENTED

/** @} */ // PA Power Amplifier (PA)

#ifdef __cplusplus
}
#endif

#endif // SL_RAIL_UTIL_PA_NVM_CONFIGS_H
