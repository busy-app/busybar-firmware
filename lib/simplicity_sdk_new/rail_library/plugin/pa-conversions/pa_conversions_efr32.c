/***************************************************************************//**
 * @file
 * @brief PA power conversion functions provided to the customer as source for
 *   highest level of customization.
 * @details This file contains the curves and logic that convert PA power
 *   levels to dBm powers.
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
// For details on how to use this plugin, see
//   https://www.silabs.com/documents/public/application-notes/an1127-power-amplifier-power-conversion-functions.pdf

#include "em_device.h"
#ifndef RISCVSEQUENCER
#if defined(_SILICON_LABS_32B_SERIES_2) || defined(SIMULATION_DEVICE)
#include "em_cmu.h"
#else
#include "sl_clock_manager.h"
#endif
#endif //#ifndef RISCVSEQUENCER

#include "rail.h"
#include "sl_rail.h"
#include "pa_conversions_efr32.h"

#include "sl_common.h"

const bool sli_rail_3_pa = false;

#if !RAILTEST
static
#endif
RAIL_TxPowerCurvesConfigAlt_t powerCurvesState;

// Make sure SUPPORTED_PA_INDICES match the per-platform PA curves
// provided by RAIL_DECLARE_TX_POWER_CURVES_CONFIG_ALT and resulting
// RAIL_TxPowerCurvesConfigAlt_t!
#ifndef SUPPORTED_PA_INDICES
#if (_SILICON_LABS_32B_SERIES_2_CONFIG == 1)
#define SUPPORTED_PA_INDICES {     \
    0U,        /* 2P4GIG_HP  */    \
    1U,        /* 2P4GIG_MP  */    \
    2U,        /* 2P4GIG_LP  */    \
    /* The rest are unsupported */ \
}
#elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 2)
#define SUPPORTED_PA_INDICES {     \
    0U,        /* 2P4GIG_HP  */    \
    RAIL_NUM_PA, /* 2P4GIG_MP  */  \
    1U,        /* 2P4GIG_LP  */    \
    /* The rest are unsupported */ \
}
#elif ((_SILICON_LABS_32B_SERIES_2_CONFIG == 3) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))
#define SUPPORTED_PA_INDICES {                   \
    RAIL_NUM_PA, /* 2P4GIG_HP  */                \
    RAIL_NUM_PA, /* 2P4GIG_MP  */                \
    RAIL_NUM_PA, /* 2P4GIG_LP  */                \
    RAIL_NUM_PA, /* 2P4GIG_LLP */                \
    RAIL_NUM_PA, /* 2P4GIG_HIGHEST */            \
    RAIL_NUM_PA, /* SUBGIG_POWERSETTING_TABLE */ \
    0U,        /* SUBGIG_HP */                   \
    1U,        /* SUBGIG_MP */                   \
    2U,        /* SUBGIG_LP */                   \
    3U,        /* SUBGIG_LLP */                  \
    /* The rest are unsupported */               \
}
#elif ((_SILICON_LABS_32B_SERIES_2_CONFIG == 4) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6))
#define SUPPORTED_PA_INDICES {     \
    0U,        /* 2P4GIG_HP  */    \
    RAIL_NUM_PA, /* 2P4GIG_MP  */  \
    1U,        /* 2P4GIG_LP  */    \
    /* The rest are unsupported */ \
}
#elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 5)
#define SUPPORTED_PA_INDICES {                  \
    RAIL_NUM_PA, /* 2P4GIG_HP  */               \
    RAIL_NUM_PA, /* 2P4GIG_MP  */               \
    RAIL_NUM_PA, /* 2P4GIG_LP  */               \
    RAIL_NUM_PA, /* 2P4GIG_LLP */               \
    RAIL_NUM_PA, /* 2P4GIG_HIGHEST */           \
    0U,        /* SUBGIG_POWERSETTING_TABLE */  \
    RAIL_NUM_PA, /* SUBGIG_HP */                \
    RAIL_NUM_PA, /* SUBGIG_MP */                \
    RAIL_NUM_PA, /* SUBGIG_LP */                \
    RAIL_NUM_PA, /* SUBGIG_LLP */               \
    RAIL_NUM_PA, /* SUBGIG_HIGHEST */           \
    1U,        /* OFDM_PA_POWERSETTING_TABLE */ \
}
#elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 7)
#define SUPPORTED_PA_INDICES {     \
    0U,        /* 2P4GIG_HP  */    \
    RAIL_NUM_PA, /* 2P4GIG_MP  */  \
    1U,        /* 2P4GIG_LP  */    \
    /* The rest are unsupported */ \
}
#elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 8)
#define SUPPORTED_PA_INDICES {                   \
    0U,        /* 2P4GIG_HP  */                  \
    RAIL_NUM_PA, /* 2P4GIG_MP  */                \
    RAIL_NUM_PA, /* 2P4GIG_LP  */                \
    RAIL_NUM_PA, /* 2P4GIG_LLP */                \
    RAIL_NUM_PA, /* 2P4GIG_HIGHEST */            \
    RAIL_NUM_PA, /* SUBGIG_POWERSETTING_TABLE */ \
    1U,        /* SUBGIG_HP */                   \
    2U,        /* SUBGIG_MP */                   \
    3U,        /* SUBGIG_LP */                   \
    4U,        /* SUBGIG_LLP */                  \
    /* The rest are unsupported */               \
}
#elif ((_SILICON_LABS_32B_SERIES_2_CONFIG == 9) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 11))
#define SUPPORTED_PA_INDICES {     \
    0U,        /* 2P4GIG_HP  */    \
    RAIL_NUM_PA, /* 2P4GIG_MP  */  \
    1U,        /* 2P4GIG_LP  */    \
    /* The rest are unsupported */ \
}
#elif defined(_SILICON_LABS_32B_SERIES_3)
#define SUPPORTED_PA_INDICES {     \
    0U,      /* 2P4GIG_HP  */      \
    RAIL_NUM_PA, /* 2P4GIG_MP  */  \
    1U,      /* 2P4GIG_LP  */      \
    /* The rest are unsupported */ \
}

#else
#error "unknown platform"
#endif
#endif

const uint8_t sli_rail_supportedPaIndices[] = SUPPORTED_PA_INDICES;

#ifndef RISCVSEQUENCER
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
  #define PA_CONVERSION_MINIMUM_PWRLVL 1U
#else
  #define PA_CONVERSION_MINIMUM_PWRLVL 0U
#endif

//   This macro is defined when Silicon Labs builds this into the library as WEAK
//   to ensure it can be overriden by customer versions of these functions. The macro
//   should *not* be defined in a customer build.
#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
const RAIL_TxPowerCurves_t *RAIL_GetTxPowerCurve(RAIL_TxPowerMode_t mode)
{
  static RAIL_TxPowerCurves_t powerCurves;
  RAIL_TxPowerLevel_t maxPowerLevel, minPowerLevel;
  if (RAIL_SupportsTxPowerModeAlt(RAIL_EFR32_HANDLE,
                                  &mode,
                                  &maxPowerLevel,
                                  &minPowerLevel)
      && (mode < sizeof(sli_rail_supportedPaIndices))
      && (sli_rail_supportedPaIndices[mode] < RAIL_NUM_PA)) {
    const RAIL_PaDescriptor_t *modeInfo = &powerCurvesState.curves[sli_rail_supportedPaIndices[mode]];
    const RAIL_TxPowerCurveAlt_t *curve = modeInfo->conversion.powerCurve;
    // Check for an invalid power curve
    if (curve == NULL) {
      return NULL;
    }

#if RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
    if (modeInfo->algorithm == RAIL_PA_ALGORITHM_DBM_POWERSETTING_MAPPING_TABLE) {
      powerCurves.maxPower = modeInfo->maxPowerDbm;
      powerCurves.minPower = modeInfo->minPowerDbm;
      // Mapping table does not have RAIL_TxPowerCurveSegment_t segments
      powerCurves.powerParams = NULL;
    } else
#endif
    {
      powerCurves.maxPower = curve->maxPower;
      powerCurves.minPower = curve->minPower;
      powerCurves.powerParams = &curve->powerParams[0];
    }
    return &powerCurves;
  }
  return NULL;
}

// This function will not be supported for any parts after efr32xg1x
#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
RAIL_Status_t RAIL_InitTxPowerCurves(const RAIL_TxPowerCurvesConfig_t *config)
{
  (void) config;
  return RAIL_STATUS_INVALID_CALL;
}

#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
RAIL_Status_t RAIL_InitTxPowerCurvesAlt(const RAIL_TxPowerCurvesConfigAlt_t *config)
{
  RAIL_Status_t status = RAIL_VerifyTxPowerCurves(config);
  if (status == RAIL_STATUS_NO_ERROR) {
    powerCurvesState = *config;
  }
  return status;
}

#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
#if RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
const RAIL_PaPowerSetting_t *RAIL_GetPowerSettingTable(RAIL_Handle_t railHandle, RAIL_TxPowerMode_t mode,
                                                       RAIL_TxPower_t *minPower, RAIL_TxPower_t *maxPower,
                                                       RAIL_TxPowerLevel_t *step)
{
  (void)railHandle;
#if RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
  if ((mode < sizeof(sli_rail_supportedPaIndices))
      && (sli_rail_supportedPaIndices[mode] < RAIL_NUM_PA)) {
    RAIL_PaDescriptor_t *modeInfo = &powerCurvesState.curves[sli_rail_supportedPaIndices[mode]];
    *minPower = modeInfo->minPowerDbm;
    *maxPower = modeInfo->maxPowerDbm;
    *step = modeInfo->step;
    return (RAIL_PaPowerSetting_t*)(modeInfo->conversion.mappingTable);
  }
  return NULL;
#else
  (void)mode;
  (void)minPower;
  (void)maxPower;
  (void)step;
  return NULL;
#endif
}
#endif // RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
#endif //#ifndef RISCVSEQUENCER

#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
RAIL_Status_t RAIL_ConvertDbmToPowerSettingEntry(RAIL_Handle_t railHandle,
                                                 RAIL_TxPowerMode_t mode,
                                                 RAIL_TxPower_t power,
                                                 RAIL_TxPowerSettingEntry_t *powerSettingInfo)
{
  (void)railHandle;
#if RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
  if ((mode < sizeof(sli_rail_supportedPaIndices))
      && (sli_rail_supportedPaIndices[mode] < RAIL_NUM_PA)) {
    RAIL_PaDescriptor_t const *modeInfo = &powerCurvesState.curves[sli_rail_supportedPaIndices[mode]];
    if (modeInfo->algorithm == RAIL_PA_ALGORITHM_DBM_POWERSETTING_MAPPING_TABLE) {
      RAIL_TxPower_t minPower = modeInfo->minPowerDbm;
      RAIL_TxPower_t maxPower = modeInfo->maxPowerDbm;
      RAIL_TxPowerLevel_t step = modeInfo->step;

      // Cap the power to within the range of the mapping table
      if (power < minPower) {
        power = minPower;
      } else if (power > maxPower) {
        power = maxPower;
      } else {
        // Power level is within bounds (MISRA required else)
      }

      // Calculate indices
      uint32_t maxIndex = (uint32_t)((maxPower - minPower) / step);
      uint32_t powerIndex = (uint32_t)((power - minPower) / step);

      // Ensure powerIndex is within bounds
      if (powerIndex > maxIndex) {
        powerIndex = maxIndex;
      }

      RAIL_PaPowerSetting_t powerSetting = modeInfo->conversion.mappingTable[powerIndex];

      while ((powerIndex > 0U)
             && (powerSetting == (RAIL_PaPowerSetting_t)modeInfo->conversion.mappingTable[powerIndex - 1U])) {
        powerIndex--;
      }
      power = minPower + ((RAIL_TxPower_t)powerIndex * step);

      powerSettingInfo->paPowerSetting = powerSetting;
      powerSettingInfo->minPaPowerDdbm = minPower;
      powerSettingInfo->maxPaPowerDdbm = maxPower;
      powerSettingInfo->currentPaPowerDdbm = power;
      return RAIL_STATUS_NO_ERROR;
    }
  }
  return RAIL_STATUS_INVALID_CALL;
#else
  (void) mode;
  (void) power;
  (void) powerSettingInfo;
  return RAIL_STATUS_INVALID_CALL;
#endif //RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
}

sl_rail_status_t sl_railcb_convert_ddbm_to_power_setting_entry(sl_rail_handle_t rail_handle,
                                                               sl_rail_tx_power_t power_ddbm,
                                                               sl_rail_tx_pa_mode_t pa_mode,
                                                               sl_rail_tx_power_t channel_restr_max_power_ddbm,
                                                               sl_rail_tx_power_setting_entry_t *p_power_setting_info)
{
  (void) rail_handle;
  (void) power_ddbm;
  (void) pa_mode;
  (void) channel_restr_max_power_ddbm;
  (void) p_power_setting_info;
  return SL_RAIL_STATUS_INVALID_CALL;
}

#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
RAIL_TxPowerLevel_t RAIL_ConvertDbmToRaw(RAIL_Handle_t railHandle,
                                         RAIL_TxPowerMode_t mode,
                                         RAIL_TxPower_t power)
{
  (void)railHandle;
#if RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
  // Powersetting tables do not have raw powerlevels.
  // Could use RAIL_ConvertDbmToPowerSettingEntry
  (void)mode;
  (void) power;
#else

  // When a channel dBm limitation greater than or equal to \ref RAIL_TX_POWER_MAX
  // is converted to raw units, the max RAIL_TxPowerLevel_t will be
  // returned. When compared to the current power level of the PA,
  // it will always be greater, indicating that no power coercion
  // is necessary to comply with channel limitations.
  if (power >= RAIL_TX_POWER_MAX) {
    return 255U;
  }

  if ((mode < sizeof(sli_rail_supportedPaIndices))
      && (sli_rail_supportedPaIndices[mode] < RAIL_NUM_PA)) {
    RAIL_PaDescriptor_t const *modeInfo = &powerCurvesState.curves[sli_rail_supportedPaIndices[mode]];
    uint32_t minPowerLevel = SL_MAX(modeInfo->min, PA_CONVERSION_MINIMUM_PWRLVL);

    // If we're in low power mode, just use the simple lookup table
    if (modeInfo->algorithm == RAIL_PA_ALGORITHM_MAPPING_TABLE) {
      // Binary search through the lookup table to find the closest power level
      // without going over.
      uint32_t lower = 0U;
      // Track the high side of the estimate
      uint32_t powerIndex = modeInfo->max - minPowerLevel;

      while (lower < powerIndex) {
        // Calculate the midpoint of the current range
        uint32_t index = powerIndex - (powerIndex - lower) / 2U;
        if (power < modeInfo->conversion.mappingTable[index]) {
          powerIndex = index - 1U;
        } else {
          lower = index;
        }
      }
      return (RAIL_TxPowerLevel_t)(powerIndex + minPowerLevel);
    }

    // Here we know we're using the piecewise linear conversion
    RAIL_TxPowerCurveAlt_t const *paParams = modeInfo->conversion.powerCurve;
    // Check for valid paParams before using them
    if (paParams == NULL) {
      return 0U;
    }

    // Cap the power based on the PA settings.
    if (power > paParams->maxPower) {
      // If we go above the maximum dbm the chip supports
      // Then provide maximum powerLevel
      power = paParams->maxPower;
    } else if (power < paParams->minPower) {
      // If we go below the minimum we want included in the curve fit, force it.
      power = paParams->minPower;
    } else {
      // Do nothing, power is OK
    }
    // Map the power value to a 0 - 7 curveIndex value
    //There are 8 segments of step size of RAIL_TX_POWER_CURVE_INCREMENT in deci dBm
    //starting from maximum RAIL_TX_POWER_CURVE_MAX in deci dBm
    // These are just starting points to give the code
    // a rough idea of which segment to use, based on
    // how they were fit. Adjustments are made later on
    // if this turns out to be incorrect.
    RAIL_TxPower_t txPowerMax = RAIL_TX_POWER_CURVE_DEFAULT_MAX;
    RAIL_TxPower_t txPowerIncrement = RAIL_TX_POWER_CURVE_DEFAULT_INCREMENT;
    int16_t curveIndex = 0;
    // if the first curve segment starts with RAIL_TX_POWER_LEVEL_INVALID
    //It is an extra curve segment to depict the maxpower and increment
    // (in deci-dBm) used while generating the curves.
    // The extra segment is only present when curve segment is generated by
    //using values different from the default - RAIL_TX_POWER_CURVE_DEFAULT_MAX
    // and RAIL_TX_POWER_CURVE_DEFAULT_INCREMENT.
    if ((paParams->powerParams[0].maxPowerLevel) == RAIL_TX_POWER_LEVEL_INVALID) {
      curveIndex += 1;
      txPowerMax = (RAIL_TxPower_t) paParams->powerParams[0].slope;
      txPowerIncrement = (RAIL_TxPower_t) paParams->powerParams[0].intercept;
    }

    curveIndex += (txPowerMax - power) / txPowerIncrement;
    if ((curveIndex > ((int16_t)modeInfo->segments - 1))
        || (curveIndex < 0)) {
      curveIndex = ((int16_t)modeInfo->segments - 1);
    }

    uint32_t powerLevel;
    do {
      // Select the correct piecewise segment to use for conversion.
      RAIL_TxPowerCurveSegment_t const *powerParams =
        &paParams->powerParams[curveIndex];

      // powerLevel can only go down to 0.
      int32_t powerLevelInt = powerParams->intercept + ((int32_t)powerParams->slope * (int32_t)power);
      if (powerLevelInt < 0) {
        powerLevel = 0U;
      } else {
        powerLevel = (uint32_t) powerLevelInt;
      }
      // RAIL_LIB-8330: Modified from adding 500 to adding 92, this was tested on xg21 as being the highest
      // number we can use without exceeding the requested power in dBm
      powerLevel = ((powerLevel + 92U) / 1000U);

      // In case it turns out the resultant power level was too low and we have
      // to recalculate with the next curve...
      curveIndex++;
    } while ((curveIndex < (int16_t)modeInfo->segments)
             && (powerLevel <= paParams->powerParams[curveIndex].maxPowerLevel));

    // We already know that curveIndex is at most modeInfo->segments
    if (powerLevel > paParams->powerParams[curveIndex - 1].maxPowerLevel) {
      powerLevel = paParams->powerParams[curveIndex - 1].maxPowerLevel;
    }

    // If we go below the minimum we want included in the curve fit, force it.
    if (powerLevel < minPowerLevel) {
      powerLevel = minPowerLevel;
    }

    return (RAIL_TxPowerLevel_t)powerLevel;
  }
#endif // RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
  return 0U;
}

#ifndef RISCVSEQUENCER
#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
RAIL_TxPower_t RAIL_ConvertRawToDbm(RAIL_Handle_t railHandle,
                                    RAIL_TxPowerMode_t mode,
                                    RAIL_TxPowerLevel_t powerLevel)
{
  (void)railHandle;

  if ((mode < sizeof(sli_rail_supportedPaIndices))
      && (sli_rail_supportedPaIndices[mode] < RAIL_NUM_PA)) {
    RAIL_PaDescriptor_t const *modeInfo = &powerCurvesState.curves[sli_rail_supportedPaIndices[mode]];
    if (modeInfo->algorithm == RAIL_PA_ALGORITHM_MAPPING_TABLE) {
      // Limit the max power level
      if (powerLevel > modeInfo->max) {
        powerLevel = modeInfo->max;
      }

      // We 1-index low power PA power levels, but of course arrays are 0 indexed
      powerLevel -= SL_MAX(modeInfo->min, PA_CONVERSION_MINIMUM_PWRLVL);

      //If the index calculation above underflowed, then provide the lowest array index.
      if (powerLevel > (modeInfo->max - modeInfo->min)) {
        powerLevel = 0U;
      }
      return modeInfo->conversion.mappingTable[powerLevel];
    } else {
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
      // Although 0 is a legitimate power on non-2.4 LP PA's and can be set via
      // "RAIL_SetTxPower(railHandle, 0)" it is MUCH lower than power
      // level 1 (approximately -50 dBm). Including it in the piecewise
      // linear fit would skew the curve substantially, so we exclude it
      // from the conversion.
      if (powerLevel == 0U) {
        return -500;
      }
#endif

      RAIL_TxPowerCurveAlt_t const *powerCurve = modeInfo->conversion.powerCurve;
      // Check for a valid powerCurve pointer before using it
      if (powerCurve == NULL) {
        return RAIL_TX_POWER_MIN;
      }

      RAIL_TxPowerCurveSegment_t const *powerParams = powerCurve->powerParams;

      // Hard code the extremes (i.e. don't use the curve fit) in order
      // to make it clear that we are reaching the extent of the chip's
      // capabilities
      if (powerLevel <= modeInfo->min) {
        return powerCurve->minPower;
      } else if (powerLevel >= modeInfo->max) {
        return powerCurve->maxPower;
      } else {
        // Power level is within bounds (MISRA required else)
      }

      // Figure out which parameter to use based on the power level
      uint8_t x = 0;
      uint8_t upperBound = modeInfo->segments - 1U;

      // If the first curve segment starts with RAIL_TX_POWER_LEVEL_INVALID,
      // then it is an additional curve segment that stores maxpower and increment
      // (in deci-dBm) used to generate the curves.
      // The extra info segment is present only if the curves were generated using
      // values other than default - RAIL_TX_POWER_CURVE_DEFAULT_MAX and
      // RAIL_TX_POWER_CURVE_DEFAULT_INCREMENT.
      if ((powerParams[0].maxPowerLevel) == RAIL_TX_POWER_LEVEL_INVALID) {
        x = 1U; // skip over the first entry
      }

      for (; x < upperBound; x++) {
        if (powerParams[x + 1U].maxPowerLevel < powerLevel) {
          break;
        }
      }
      int32_t power;
      power = ((1000 * (int32_t)(powerLevel)) - powerParams[x].intercept);
      power = ((power + ((int32_t)powerParams[x].slope / 2)) / (int32_t)powerParams[x].slope);

      if (power > powerCurve->maxPower) {
        return powerCurve->maxPower;
      } else if (power < powerCurve->minPower) {
        return powerCurve->minPower;
      } else {
        return (RAIL_TxPower_t)power;
      }
    }
  }
  return RAIL_TX_POWER_MIN;
}

#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
RAIL_Status_t RAIL_GetTxPowerCurveLimits(RAIL_Handle_t railHandle,
                                         RAIL_TxPowerMode_t mode,
                                         RAIL_TxPower_t *maxPower,
                                         RAIL_TxPower_t *increment)
{
  (void)railHandle;
  if ((mode < sizeof(sli_rail_supportedPaIndices))
      && (sli_rail_supportedPaIndices[mode] < RAIL_NUM_PA)) {
    RAIL_PaDescriptor_t const *modeInfo = &powerCurvesState.curves[sli_rail_supportedPaIndices[mode]];
#if RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
    if (modeInfo->algorithm == RAIL_PA_ALGORITHM_DBM_POWERSETTING_MAPPING_TABLE) {
      *maxPower = modeInfo->maxPowerDbm;
      *increment = modeInfo->step;
      return RAIL_STATUS_NO_ERROR;
    }
#endif

    //The power max info only for available Linear fit
    if (modeInfo->algorithm == RAIL_PA_ALGORITHM_MAPPING_TABLE) {
      return RAIL_STATUS_INVALID_CALL;
    }
    *maxPower = RAIL_TX_POWER_CURVE_DEFAULT_MAX;
    *increment = RAIL_TX_POWER_CURVE_DEFAULT_INCREMENT;
    RAIL_TxPowerCurveAlt_t const *paParams = modeInfo->conversion.powerCurve;
    if ((paParams->powerParams[0].maxPowerLevel) == RAIL_TX_POWER_LEVEL_INVALID) {
      *maxPower = paParams->powerParams[0].slope;
      *increment = (RAIL_TxPower_t)paParams->powerParams[0].intercept;
    }
    return RAIL_STATUS_NO_ERROR;
  }
  return RAIL_STATUS_INVALID_PARAMETER;
}

// This macro is defined when Silicon Labs builds curves into the library as WEAK
// to ensure it can be overriden by customer versions of these functions. It
// should *not* be defined in a customer build.
#if !defined(RAIL_PA_CONVERSIONS_WEAK) && !defined(HAL_CONFIG)

#if     SL_RAIL_UTIL_PA_NVM_ENABLED
#include "sl_rail_util_pa_nvm_configs.h"
#endif//SL_RAIL_UTIL_PA_NVM_ENABLED

#if RAIL_SUPPORTS_2P4GHZ_BAND
static RAIL_TxPowerConfig_t txPowerConfig2p4Ghz = {
  .mode = SL_RAIL_UTIL_PA_SELECTION_2P4GHZ,
  .voltage = SL_RAIL_UTIL_PA_VOLTAGE_MV,
  .rampTime = SL_RAIL_UTIL_PA_RAMP_TIME_US,
};
#endif
#if RAIL_SUPPORTS_SUBGHZ_BAND
static RAIL_TxPowerConfig_t txPowerConfigSubGhz = {
  .mode = SL_RAIL_UTIL_PA_SELECTION_SUBGHZ,
  .voltage = SL_RAIL_UTIL_PA_VOLTAGE_MV,
  .rampTime = SL_RAIL_UTIL_PA_RAMP_TIME_US,
};
#endif
#if RAIL_SUPPORTS_OFDM_PA
static RAIL_TxPowerConfig_t txPowerConfigOFDM = {
  .mode = SL_RAIL_UTIL_PA_SELECTION_OFDM,
  .voltage = SL_RAIL_UTIL_PA_VOLTAGE_MV,
};
#endif // RAIL_SUPPORTS_OFDM_PA

void sl_rail_util_pa_init(void)
{
  const RAIL_TxPowerCurvesConfigAlt_t *txPowerCurves;
#if SL_RAIL_UTIL_PA_VOLTAGE_MV > 1800
  txPowerCurves = &RAIL_TxPowerCurvesVbat;
#else
  txPowerCurves = &RAIL_TxPowerCurvesDcdc;
#endif

#if     SL_RAIL_UTIL_PA_NVM_ENABLED
  // Check and process NVM power mode
  RAIL_TxPowerMode_t nvmMode = sl_rail_util_pa_nvm_read_mode();
  if (nvmMode != RAIL_TX_POWER_MODE_NONE) {
    if (nvmMode <= RAIL_TX_POWER_MODE_2P4GIG_HIGHEST) { // A 2.4GHz mode
     #if RAIL_SUPPORTS_2P4GHZ_BAND
      txPowerConfig2p4Ghz.mode = nvmMode;
     #endif
    } else if (nvmMode <= RAIL_TX_POWER_MODE_SUBGIG_HIGHEST) { // A SubGHz mode
     #if RAIL_SUPPORTS_SUBGHZ_BAND
      txPowerConfigSubGhz.mode = nvmMode;
     #endif
    } else { // An ODFM mode
     #if RAIL_SUPPORTS_OFDM_PA
      txPowerConfigOFDM.mode = nvmMode;
     #endif
    }
  } else {
    // Use app's compile-time-configured txPowerConfig*.modes
  }
  // Check and process NVM power curve configs
  sl_rail_nvm_pa_config_t nvm_config;
  sl_status_t status = sl_rail_util_pa_nvm_read_config(&nvm_config);
  if (status == SL_STATUS_OK) {
    const RAIL_TxPowerCurvesConfigAlt_t *p_deserialized_config
      = sli_rail_util_pa_nvm_deserialize_config(&nvm_config);
    if (p_deserialized_config != NULL) {
     #if RAIL_SUPPORTS_2P4GHZ_BAND
      txPowerConfig2p4Ghz.voltage = p_deserialized_config->paVoltage;
     #endif
     #if RAIL_SUPPORTS_SUBGHZ_BAND
      txPowerConfigSubGhz.voltage = p_deserialized_config->paVoltage;
     #endif
     #if RAIL_SUPPORTS_OFDM_PA
      txPowerConfigOFDM.voltage = p_deserialized_config->paVoltage;
     #endif
      // If application already called one of the
      // sl_rail_util_pa_get_tx_power_config_*() functions
      // then this voltage update is probably too late.
      // But we don't have a railHandle here to give
      // RAIL_ConfigTxPower() a buzz.
      txPowerCurves = p_deserialized_config;
    }
  }
#endif//SL_RAIL_UTIL_PA_NVM_ENABLED

  (void)RAIL_InitTxPowerCurvesAlt(txPowerCurves);

#if SL_RAIL_UTIL_PA_CALIBRATION_ENABLE
  RAIL_EnablePaCal(true);
#else
  RAIL_EnablePaCal(false);
#endif
}

RAIL_TxPowerConfig_t *sl_rail_util_pa_get_tx_power_config_2p4ghz(void)
{
#if RAIL_SUPPORTS_2P4GHZ_BAND
  return &txPowerConfig2p4Ghz;
#else
  return NULL;
#endif
}

RAIL_TxPowerConfig_t *sl_rail_util_pa_get_tx_power_config_subghz(void)
{
#if RAIL_SUPPORTS_SUBGHZ_BAND
  return &txPowerConfigSubGhz;
#else
  return NULL;
#endif
}

RAIL_TxPowerConfig_t *sl_rail_util_pa_get_tx_power_config_ofdm(void)
{
#if RAIL_SUPPORTS_OFDM_PA
  return &txPowerConfigOFDM;
#else
  return NULL;
#endif // RAIL_SUPPORTS_OFDM_PA
}

void sl_rail_util_pa_on_channel_config_change(RAIL_Handle_t rail_handle,
                                              const RAIL_ChannelConfigEntry_t *entry)
{
  if (!RAIL_IsPaAutoModeEnabled(rail_handle)) {
    RAIL_TxPowerConfig_t currentTxPowerConfig;
    RAIL_TxPowerConfig_t *newTxPowerConfigPtr;
    RAIL_Status_t status;

    // Get current TX Power Config.
    status = RAIL_GetTxPowerConfig(rail_handle, &currentTxPowerConfig);
    if (status != RAIL_STATUS_NO_ERROR) {
      while (true) {
      } // Error: Can't get TX Power Config
    }

#if RAIL_SUPPORTS_DUAL_BAND
    // Determine new TX Power Config.
    if (entry->baseFrequency < 1000000000UL) {
      newTxPowerConfigPtr = &txPowerConfigSubGhz;
    } else {
      newTxPowerConfigPtr = &txPowerConfig2p4Ghz;
    }
#else
    (void) entry;
#if RAIL_SUPPORTS_2P4GHZ_BAND
    newTxPowerConfigPtr = &txPowerConfig2p4Ghz;
#else
    newTxPowerConfigPtr = &txPowerConfigSubGhz;
#endif
#endif

#if RAIL_IEEE802154_SUPPORTS_DUAL_PA_CONFIG
    if (currentTxPowerConfig.mode == RAIL_TX_POWER_MODE_NONE) {
#if RAIL_SUPPORTS_OFDM_PA
      if (RAIL_SupportsTxPowerModeAlt(rail_handle,
                                      &txPowerConfigOFDM.mode,
                                      NULL, NULL)) {
        // Apply OFDM Power Config.
        status = RAIL_ConfigTxPower(rail_handle, &txPowerConfigOFDM);
        if (status != RAIL_STATUS_NO_ERROR) {
          while (true) {
          } // Error: Can't set TX Power Config
        }
        // Set default TX power after RAIL_ConfigTxPower.
        status = RAIL_SetTxPowerDbm(rail_handle, SL_RAIL_UTIL_PA_POWER_DECI_DBM);
        if (status != RAIL_STATUS_NO_ERROR) {
          while (true) {
          } // Error: Can't set TX Power
        }
      }
#endif // RAIL_SUPPORTS_OFDM_PA
      // Apply FSK Power Config.
      status = RAIL_ConfigTxPower(rail_handle, newTxPowerConfigPtr);
      if (status != RAIL_STATUS_NO_ERROR) {
        while (true) {
        } // Error: Can't set TX Power Config
      }
      // Set default TX power after RAIL_ConfigTxPower.
      status = RAIL_SetTxPowerDbm(rail_handle, SL_RAIL_UTIL_PA_POWER_DECI_DBM);
      if (status != RAIL_STATUS_NO_ERROR) {
        while (true) {
        } // Error: Can't set TX Power
      }
    }
#else
    // Call RAIL_ConfigTxPower only if TX Power Config mode has changed.
    if (currentTxPowerConfig.mode != newTxPowerConfigPtr->mode) {
      // Save current TX power before RAIL_ConfigTxPower (because not preserved).
      RAIL_TxPower_t txPowerDeciDbm;
      if (currentTxPowerConfig.mode == RAIL_TX_POWER_MODE_NONE) {
        txPowerDeciDbm = SL_RAIL_UTIL_PA_POWER_DECI_DBM;
      } else {
        txPowerDeciDbm = RAIL_GetTxPowerDbm(rail_handle);
      }

      // Apply new TX Power Config.
      status = RAIL_ConfigTxPower(rail_handle, newTxPowerConfigPtr);
      if (status != RAIL_STATUS_NO_ERROR) {
        while (true) {
        } // Error: Can't set TX Power Config
      }
      // Restore TX power after RAIL_ConfigTxPower.
      status = RAIL_SetTxPowerDbm(rail_handle, txPowerDeciDbm);
      if (status != RAIL_STATUS_NO_ERROR) {
        while (true) {
        } // Error: Can't set TX Power
      }
      // If requested a HIGHEST setting, update it with the real one selected
      // to short-circuit the next time through here since HIGHEST never
      // matches the real PA returned by RAIL_GetTxPowerConfig(), causing
      // reconfiguration of the same PA on every callback.
      if (false
         #ifdef  RAIL_TX_POWER_MODE_2P4GIG_HIGHEST
          || (newTxPowerConfigPtr->mode == RAIL_TX_POWER_MODE_2P4GIG_HIGHEST)
         #endif
         #ifdef  RAIL_TX_POWER_MODE_SUBGIG_HIGHEST
          || (newTxPowerConfigPtr->mode == RAIL_TX_POWER_MODE_SUBGIG_HIGHEST)
         #endif
          ) {
        (void) RAIL_GetTxPowerConfig(rail_handle, &currentTxPowerConfig);
        newTxPowerConfigPtr->mode = currentTxPowerConfig.mode;
      }
    }
#endif
  } // !RAIL_IsPaAutoModeEnabled
}

static RAIL_TxPowerMode_t map_pa_mode_to_power_mode(sl_rail_handle_t rail_handle,
                                                    sl_rail_tx_pa_mode_t pa_mode,
                                                    sl_rail_tx_power_t ddbm)
{
  // Map pa_mode to a RAIL_TxPowerMode_t
  RAIL_TxPowerMode_t mode;
  if (pa_mode == SL_RAIL_TX_PA_MODE_SUB_GHZ_OFDM) {
    mode = RAIL_TX_POWER_MODE_OFDM_PA_POWERSETTING_TABLE;
  } else {
    // Can't guess how to map pa_mode to a RAIL_TxPowerMode_t so use
    // whatever is currently configured, or HIGHEST if what's configured
    // isn't the right band.
    // This purposefully includes SL_RAIL_TX_PA_MODE_INVALID.
    RAIL_TxPowerConfig_t config;
    RAIL_Status_t status = RAIL_GetTxPowerConfig((RAIL_Handle_t)rail_handle, &config);
    if (status != RAIL_STATUS_NO_ERROR) {
      return RAIL_TX_POWER_MODE_NONE;
    }
    mode = config.mode;
    if ((pa_mode == SL_RAIL_TX_PA_MODE_2P4_GHZ)
        && (mode > RAIL_TX_POWER_MODE_2P4GIG_HIGHEST)) {
      mode = RAIL_TX_POWER_MODE_2P4GIG_HIGHEST;
    } else
    if ((pa_mode == SL_RAIL_TX_PA_MODE_SUB_GHZ)
        && ((mode <= RAIL_TX_POWER_MODE_2P4GIG_HIGHEST)
            || (mode > RAIL_TX_POWER_MODE_SUBGIG_HIGHEST))) {
      mode = RAIL_TX_POWER_MODE_SUBGIG_HIGHEST;
    }
    if (RAIL_IsPaAutoModeEnabled(rail_handle)) {
      RAIL_ChannelConfigEntry_t bandEntry = {
        .baseFrequency = ((pa_mode == SL_RAIL_TX_PA_MODE_2P4_GHZ)
                          ? 2400000000UL : 915000000UL),
        // That's the only field used by the callback function
      };
      RAIL_Status_t status = RAILCb_PaAutoModeDecision(rail_handle, &ddbm, &mode,
                                                       &bandEntry);
      if (status != RAIL_STATUS_NO_ERROR) {
        return RAIL_TX_POWER_MODE_NONE;
      }
    }
  }
  if (RAIL_SupportsTxPowerModeAlt(rail_handle, &mode, NULL, NULL)) {
    return mode;
  }
  return RAIL_TX_POWER_MODE_NONE;
}

sl_rail_status_t sl_rail_util_pa_post_init(sl_rail_handle_t rail_handle,
                                           sl_rail_tx_pa_mode_t pa_mode)
{
  RAIL_Status_t status = RAIL_STATUS_INVALID_PARAMETER;
  switch (pa_mode) {
   #if RAIL_SUPPORTS_2P4GHZ_BAND
    case SL_RAIL_TX_PA_MODE_2P4_GHZ:
      status = RAIL_ConfigTxPower(rail_handle, &txPowerConfig2p4Ghz);
      break;
   #endif
   #if RAIL_SUPPORTS_SUBGHZ_BAND
    case SL_RAIL_TX_PA_MODE_SUB_GHZ:
      status = RAIL_ConfigTxPower(rail_handle, &txPowerConfigSubGhz);
      break;
   #endif
   #if RAIL_SUPPORTS_OFDM_PA
    case SL_RAIL_TX_PA_MODE_SUB_GHZ_OFDM:
      status = RAIL_ConfigTxPower(rail_handle, &txPowerConfigOFDM);
      break;
   #endif
    default:
      break;
  }
  return (sl_rail_status_t)status;
}

sl_rail_status_t sli_rail_util_pa_get_mode_limits(RAIL_TxPowerMode_t mode,
                                                  sl_rail_tx_power_t *p_min_ddbm,
                                                  sl_rail_tx_power_t *p_max_ddbm,
                                                  sl_rail_tx_power_t *p_step_ddbm)
{
  if ((mode == RAIL_TX_POWER_MODE_NONE)
      || (sli_rail_supportedPaIndices[mode] >= RAIL_NUM_PA)) {
    return SL_RAIL_STATUS_INVALID_PARAMETER;
  }
  const RAIL_PaDescriptor_t *modeInfo = &powerCurvesState.curves[sli_rail_supportedPaIndices[mode]];
  sl_rail_tx_power_t min, max, step;
#if RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
  if (modeInfo->algorithm == RAIL_PA_ALGORITHM_DBM_POWERSETTING_MAPPING_TABLE) {
    min = modeInfo->minPowerDbm;
    max = modeInfo->maxPowerDbm;
    step = (sl_rail_tx_power_t) modeInfo->step;
  } else
#endif
  if (modeInfo->algorithm == RAIL_PA_ALGORITHM_MAPPING_TABLE) {
    if (modeInfo->conversion.mappingTable == NULL) {
      return SL_RAIL_STATUS_INVALID_CALL;
    }
    // modeinfo->segments is often annoying 0 rather than # elements in table
    // but # elements should always be modeInfo->max - modeInfo->min + 1.
    min = modeInfo->conversion.mappingTable[0];
    max = modeInfo->conversion.mappingTable[modeInfo->max - modeInfo->min];
    step = SL_MIN(RAIL_TX_POWER_CURVE_DEFAULT_INCREMENT, max - min); // Best guess
  } else
  if (modeInfo->algorithm == RAIL_PA_ALGORITHM_PIECEWISE_LINEAR) {
    // Use code from RAIL_GetTxPowerCurve() for min and max and default step
    // unless (per RAIL_GetTxPowerCurveLimits()) the 1st curve element's
    // maxPowerLevel is 'INVALID' which flags that element as overriding the
    // curve's max and step.
    RAIL_TxPowerCurveAlt_t const *curve = modeInfo->conversion.powerCurve;
    if (curve == NULL) {
      return SL_RAIL_STATUS_INVALID_CALL;
    }
    min = curve->minPower;
    if (curve->powerParams[0].maxPowerLevel == RAIL_TX_POWER_LEVEL_INVALID) {
      max = (sl_rail_tx_power_t) curve->powerParams[0].slope;
      step = (sl_rail_tx_power_t) curve->powerParams[0].intercept;
    } else {
      max = curve->maxPower;
      step = SL_MIN(RAIL_TX_POWER_CURVE_DEFAULT_INCREMENT, max - min); // Best guess
    }
  } else {
    return SL_RAIL_STATUS_INVALID_PARAMETER;
  }
  if (p_min_ddbm != NULL) {
    *p_min_ddbm = min;
  }
  if (p_max_ddbm != NULL) {
    *p_max_ddbm = max;
  }
  if (p_step_ddbm != NULL) {
    *p_step_ddbm = step;
  }
  return SL_RAIL_STATUS_NO_ERROR;
}

sl_rail_status_t sl_rail_util_pa_get_tx_power_limits(sl_rail_handle_t rail_handle,
                                                     sl_rail_tx_pa_mode_t pa_mode,
                                                     sl_rail_tx_power_t *p_min_ddbm,
                                                     sl_rail_tx_power_t *p_max_ddbm,
                                                     sl_rail_tx_power_t *p_step_ddbm)
{
  // Map pa_mode to a RAIL_TxPowerMode_t
  RAIL_TxPowerMode_t min_mode = map_pa_mode_to_power_mode(rail_handle, pa_mode,
                                                          RAIL_TX_POWER_MIN);
  sl_rail_status_t status = sli_rail_util_pa_get_mode_limits(min_mode, p_min_ddbm, p_max_ddbm, p_step_ddbm);
  if ((status == SL_RAIL_STATUS_NO_ERROR)
      && (p_max_ddbm != NULL)
      && RAIL_IsPaAutoModeEnabled(rail_handle)) {
    RAIL_TxPowerMode_t max_mode = map_pa_mode_to_power_mode(rail_handle, pa_mode,
                                                            RAIL_TX_POWER_MAX);
    if (max_mode != min_mode) {
      status = sli_rail_util_pa_get_mode_limits(max_mode, NULL, p_max_ddbm, NULL);
    }
  }
  return status;
}

sl_rail_status_t sl_rail_util_pa_convert_power_to_actual(sl_rail_handle_t rail_handle,
                                                         sl_rail_tx_pa_mode_t pa_mode,
                                                         sl_rail_tx_power_t *p_ddbm)
{
  if (p_ddbm == NULL) {
    return SL_RAIL_STATUS_INVALID_PARAMETER;
  }
  RAIL_TxPowerMode_t mode = map_pa_mode_to_power_mode(rail_handle, pa_mode, *p_ddbm);
  if (mode == RAIL_TX_POWER_MODE_NONE) {
    return SL_RAIL_STATUS_INVALID_PARAMETER;
  }
  *p_ddbm = RAIL_ConvertRawToDbm(rail_handle, mode,
                                 RAIL_ConvertDbmToRaw(rail_handle, mode, *p_ddbm));
  return SL_RAIL_STATUS_NO_ERROR;
}

#endif // !RAIL_PA_CONVERSIONS_WEAK
#endif //#ifndef RISCVSEQUENCER
