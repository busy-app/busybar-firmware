/***************************************************************************//**
 * @file
 * @brief PA power conversion curve types used by Silicon Labs PA power
 *   conversion functions.
 * @details This file contains the curve types needed convert PA power levels
 *   to dBm powers.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef PA_CURVE_TYPES_EFR32_H
#define PA_CURVE_TYPES_EFR32_H

#include "rail_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup PA_Curve_Conversions PA Curve Conversions
 * @ingroup PA
 * @{
 */

/**
 * @struct RAIL_TxPowerCurveSegment_t
 *
 * @brief Structure containing data defining each segment of the
 *   deci-dBm to raw power level mapping curve fits.
 *
 * Note, these used in an equation of the form:
 *
 *    powerLevel * 1000 = slope * power + intercept
 *
 * powerLevel is the 0-252/0-248/1-7 values used in the RAIL_Get/SetTxPower()
 * functions, and power is the actual output power of the PA, specified
 * in deci-dBm.
 *
 * @note If the curves are generated with
 *   maxPower and increment other than \ref RAIL_TX_POWER_CURVE_DEFAULT_MAX and
 *   \ref RAIL_TX_POWER_CURVE_DEFAULT_INCREMENT respectively, then the first
 *   \ref RAIL_TxPowerCurveSegment_t has its maxPowerLevel equal to
 *   \ref RAIL_TX_POWER_LEVEL_INVALID and its slope and intercept stores the
 *   maxPower and increment in deci-dBm respectively.
 *
 * @deprecated This RAIL 2.x type has been eliminated in RAIL 3 by
 *   use of PA power tables.
 */
typedef struct RAIL_TxPowerCurveSegment {
  /**
   * The highest power level that this segment will be used to convert.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  uint16_t maxPowerLevel;
  /**
   * Slope of the line.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  int16_t slope;
  /**
   * Y-intercept of the line.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  int32_t intercept;
} RAIL_TxPowerCurveSegment_t;

/**
 * @struct RAIL_TxPowerCurves_t
 *
 * @brief Structure containing the min and max values for a given
 *   PA and voltage supply combination (in deci-dBm).
 *
 * @deprecated This RAIL 2.x type has been eliminated in RAIL 3 by
 *   use of PA power tables.
 */
typedef struct RAIL_TxPowerCurves {
  /**
   * Max deci-dBm value.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  int16_t maxPower;
  /**
   * Min deci-dBm value.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  int16_t minPower;
  /**
   * Pointer to an array of \ref RAIL_TxPowerCurvesConfig_t::piecewiseSegments
   * elements of \ref RAIL_TxPowerCurveSegment_t for deci-dBm to raw
   * power level conversion fits.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  const RAIL_TxPowerCurveSegment_t *powerParams;
} RAIL_TxPowerCurves_t;

/**
 * @struct RAIL_TxPowerCurvesConfig_t
 *
 * @brief Structure containing curve fit information and other metadata
 *   required to properly use the WEAK versions of RAIL_ConvertRawToDb
 *   and RAIL_ConvertDbmToRaw.
 *
 * @deprecated This RAIL 2.x enum has been eliminated in RAIL 3 which
 *   uses PA power tables instead.
 */
typedef struct RAIL_TxPowerCurvesConfig {
  /**
   * Pointer a RAIL_TxPowerCurves_t representing the piecewise linear segments
   * of curves that map power level to power in dBm for the 2.4 GHz high power
   * PA.
   *
   * @note By the default conversion implementation, segments must be specified
   *   in decreasing power order. That is, the 0th entry of this array should be
   *   used to convert the highest power (levels). Segment at position n is valid
   *   from maxPowerLevel+1 from the segment at n+1 (or 0 if n is array length - 1)
   *   to maxPowerLevel of segment n, inclusive.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  const RAIL_TxPowerCurves_t *txPower24HpCurves;
  /**
   * Pointer a RAIL_TxPowerCurves_t representing the piecewise linear segments
   * of curves that map power level to power in dBm for the subgig PA.
   *
   * @note By the default conversion implementation, segments must be specified
   *   in decreasing power order. That is, the 0th entry of this array should be
   *   used to convert the highest power (levels). Segment at position n is valid
   *   from maxPowerLevel+1 from the segment at n+1 (or 0 if n is array length - 1)
   *   to maxPowerLevel of segment n, inclusive.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  const RAIL_TxPowerCurves_t *txPowerSgCurves;
  /**
   * Look up table for each of the power levels of the 2.4GHz low power
   * amplifier and their equivalent deci-dB value.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  const int16_t *txPower24LpCurves;
  /**
   * The number of piecewise segments provided to the PA in each of the four
   * conversion curve fits. The default is 8, but regardless of the number, it
   * must be the same for all curves.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  uint8_t piecewiseSegments;
} RAIL_TxPowerCurvesConfig_t;

/**
 * @enum RAIL_PaConversionAlgorithm_t
 * @brief PA conversion algorithms types for converting between dBm and power levels
 *
 * @deprecated This RAIL 2.x enum has been eliminated in RAIL 3 which
 *   uses PA power tables instead.
 */
RAIL_ENUM(RAIL_PaConversionAlgorithm_t) {
  /**
   * Piecewise linear fit.
   *
   * @deprecated This RAIL 2.x enum value has been eliminated in RAIL 3.
   */
  RAIL_PA_ALGORITHM_PIECEWISE_LINEAR,
  /**
   * Mapping table between quantities.
   *
   * @deprecated This RAIL 2.x enum value has been eliminated in RAIL 3.
   */
  RAIL_PA_ALGORITHM_MAPPING_TABLE,
  /**
   * Mapping table between pa power settings and dBm values.
   *
   * @deprecated This RAIL 2.x enum value has been eliminated in RAIL 3.
   */
  RAIL_PA_ALGORITHM_DBM_POWERSETTING_MAPPING_TABLE,
  /**
   * Algorithm count.  Must be last.
   *
   * @deprecated This RAIL 2.x enum value has been eliminated in RAIL 3.
   */
  RAIL_PA_ALGORITHM_COUNT
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using RAIL_ENUM
#define RAIL_PA_ALGORITHM_PIECEWISE_LINEAR               ((RAIL_PaConversionAlgorithm_t) RAIL_PA_ALGORITHM_PIECEWISE_LINEAR)
#define RAIL_PA_ALGORITHM_MAPPING_TABLE                  ((RAIL_PaConversionAlgorithm_t) RAIL_PA_ALGORITHM_MAPPING_TABLE)
#define RAIL_PA_ALGORITHM_DBM_POWERSETTING_MAPPING_TABLE ((RAIL_PaConversionAlgorithm_t) RAIL_PA_ALGORITHM_DBM_POWERSETTING_MAPPING_TABLE)
#define RAIL_PA_ALGORITHM_COUNT                          ((RAIL_PaConversionAlgorithm_t) RAIL_PA_ALGORITHM_COUNT)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @struct RAIL_TxPowerCurveAlt_t
 *
 * @brief Structure containing the min and max values for a given
 *   PA and voltage supply combination (in deci-dBm).
 *
 * @deprecated This RAIL 2.x type has been eliminated in RAIL 3 by
 *   use of PA power tables. See \ref sl_rail_pa_descriptor_t.
 */
typedef struct RAIL_TxPowerCurveAlt {
  /**
   * Max deci-dBm value.
   *
   * @deprecated This RAIL 2.x field has been replaced in RAIL 3 by
   *   \ref sl_rail_pa_descriptor_t::max_power_ddbm.
   */
  int16_t maxPower;
  /**
   * Min deci-dBm value.
   *
   * @deprecated This RAIL 2.x field has been replaced in RAIL 3 by
   *   \ref sl_rail_pa_descriptor_t::min_power_ddbm.
   */
  int16_t minPower;
  /**
   * Array of \ref RAIL_PaDescriptor_t::segments \ref RAIL_TxPowerCurveSegment_t
   * structures for the deci-dBm to raw power level conversion fits.
   *
   * @deprecated This RAIL 2.x field has been replaced in RAIL 3 by
   *   \ref sl_rail_pa_descriptor_t::p_power_setting_table.
   */
//Array does not have a size since it can be various sizes.
//No further fields allowed after this one.
  RAIL_TxPowerCurveSegment_t powerParams[];
} RAIL_TxPowerCurveAlt_t;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#if defined(SL_RAIL_UTIL_PA_POWERSETTING_TABLE_VERSION)
#if RAIL_SUPPORTS_COMMON_PA_INTERFACE
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
 * @struct RAIL_PowerConversion_t
 *
 * @brief Union containing a pointer to algorithm-specific conversion data.
 *
 * @deprecated This RAIL 2.x type has been eliminated in RAIL 3 by
 *   use of PA power tables. See \ref sl_rail_pa_descriptor_t.
 */
typedef union RAIL_PowerConversion {
  /**
   * Pointer to a powerCurve containing line segment data for the curves
   * corresponding to a specific PA.
   *
   * @note By the default conversion implementation, segments must be specified
   *   in decreasing power order. That is, the 0th entry of this array should be
   *   used to convert the highest power (levels). Segment at position n is valid
   *   from maxPowerLevel+1 from the segment at n+1 (or 0 if n is array length - 1)
   *   to maxPowerLevel of segment n, inclusive.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  const RAIL_TxPowerCurveAlt_t *powerCurve;
  /**
   * Lookup table for PA's which use the mapping table algorithm for converting
   * between deci-dBm and power levels.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
#if RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
#if RAIL_SUPPORTS_COMMON_PA_INTERFACE
  const int16_t *mappingTable;
#else
  const int32_t *mappingTable;
#endif
#else
  const int16_t *mappingTable;
#endif
} RAIL_PowerConversion_t;

/**
 * @struct RAIL_PaDescriptor_t
 *
 * @brief Struct containing specifics of PA configuration.
 *   PA descriptor as used in the PA conversion functions.
 *
 * @deprecated This RAIL 2.x type has been replaced in RAIL 3 by
 *   \ref sl_rail_pa_descriptor_t with different fields.
 */
typedef struct RAIL_PaDescriptor {
  /**
   * Algorithm used to map dBm to power levels for this PA.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  RAIL_PaConversionAlgorithm_t algorithm;
  /**
   * The number of piecewise segments provided to the PA in a piecewise linear
   * curve fit. The default is 8. Should be set to 0 when not using the
   * piecewise linear algorithm.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  uint8_t segments;
  /**
   * Min power level for this PA.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  RAIL_TxPowerLevel_t min;
  /**
   * Max power level for this PA.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  RAIL_TxPowerLevel_t max;
#if RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
  /**
   * Step size in deci-dBm between entries in table.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  RAIL_TxPowerLevel_t step;
  /**
   * Structure padding.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  uint8_t padding;
  /**
   * Structure padding.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  uint16_t padding2;
  /**
   * Min power in deci-dBm for this PA.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  RAIL_TxPower_t minPowerDbm;
  /**
   * Max power in deci-dBm for this PA.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  RAIL_TxPower_t maxPowerDbm;
#endif
  /**
   * Union containing a pointer to algorithm-specific conversion data.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3.
   */
  RAIL_PowerConversion_t conversion;
} RAIL_PaDescriptor_t;

/**
 * @struct RAIL_TxPowerCurvesConfigAlt_t
 *
 * @brief More generic structure containing information about
 *   piecewise linear curves and mapping tables, instead of specific PA's.
 *
 * @deprecated This RAIL 2.x type has been replaced in RAIL 3 by
 *   \ref sl_rail_tx_power_table_config_t with different fields.
 */
typedef struct RAIL_TxPowerCurvesConfigAlt {
  /**
   * The curves for each PA.
   *
   * @deprecated This RAIL 2.x field has been replaced in RAIL 3 by
   *   other \ref sl_rail_tx_power_table_config_t fields.
   */
  RAIL_PaDescriptor_t curves[RAIL_NUM_PA];
  /**
   * Signature used for validation of the curves configuruation.
   *
   * @deprecated This RAIL 2.x field has been replaced in RAIL 3 by
   *   \ref sl_rail_tx_power_table_config_t::signature.
   */
  uint32_t signature;
  /**
   * PA VDD voltage, in millivolts.
   *
   * @deprecated This RAIL 2.x field has been replaced in RAIL 3 by
   *   \ref sl_rail_tx_power_table_config_t::pa_voltage_mv .
   */
  uint16_t paVoltage;
} RAIL_TxPowerCurvesConfigAlt_t;

/** @} */ // PA_Curve_Conversions

#ifdef __cplusplus
}
#endif

#endif // PA_CURVE_TYPES_EFR32_H
