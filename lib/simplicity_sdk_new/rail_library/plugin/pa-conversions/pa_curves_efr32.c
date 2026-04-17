/***************************************************************************//**
 * @file
 * @brief Default PA power conversion structures with curves calibrated by the
 *   RAIL team.
 * @details This file contains the curves that convert PA power levels to dBm
 *   powers.
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

// This entire file should never be used on FCC pre-certified modules
#ifndef _SILICON_LABS_MODULE

#include "em_device.h"
#include "pa_conversions_efr32.h"

#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 11))

static const RAIL_TxPowerCurveAlt_t RAIL_piecewiseDataHpVbat = {
  RAIL_PA_CURVES_2P4_HP_VBAT_MAX_POWER,
  RAIL_PA_CURVES_2P4_HP_VBAT_MIN_POWER,
  RAIL_PA_CURVES_2P4_HP_VBAT_CURVES,
};
static const int16_t RAIL_curves24Lp[RAIL_PA_CURVES_LP_VALUES] =
  RAIL_PA_CURVES_2P4_LP_VBAT_CURVES;

#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
const RAIL_TxPowerCurvesConfigAlt_t RAIL_TxPowerCurvesVbat = {
  .curves = {
    {
      .algorithm = RAIL_PA_ALGORITHM_PIECEWISE_LINEAR,
      .segments = RAIL_PA_CURVES_PIECEWISE_SEGMENTS,
      .min = RAIL_TX_POWER_LEVEL_2P4_HP_MIN,
      .max = RAIL_TX_POWER_LEVEL_2P4_HP_MAX,
      .conversion = { .powerCurve = &RAIL_piecewiseDataHpVbat },
    },
    {
      .algorithm = RAIL_PA_ALGORITHM_MAPPING_TABLE,
      .segments = 0U,
      .min = RAIL_TX_POWER_LEVEL_2P4_LP_MIN,
      .max = RAIL_TX_POWER_LEVEL_2P4_LP_MAX,
      .conversion = { .mappingTable = &RAIL_curves24Lp[0] },
    },
  }
};

#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
const RAIL_TxPowerCurvesConfigAlt_t RAIL_TxPowerCurvesDcdc = {
  .curves = {
    {
      .algorithm = RAIL_PA_ALGORITHM_PIECEWISE_LINEAR,
      .segments = RAIL_PA_CURVES_PIECEWISE_SEGMENTS,
      .min = RAIL_TX_POWER_LEVEL_2P4_HP_MIN,
      .max = RAIL_TX_POWER_LEVEL_2P4_HP_MAX,
      .conversion = { .powerCurve = &RAIL_piecewiseDataHpVbat },
    },
    {
      .algorithm = RAIL_PA_ALGORITHM_MAPPING_TABLE,
      .segments = 0U,
      .min = RAIL_TX_POWER_LEVEL_2P4_LP_MIN,
      .max = RAIL_TX_POWER_LEVEL_2P4_LP_MAX,
      .conversion = { .mappingTable = &RAIL_curves24Lp[0] },
    },
  }
};

#elif ((_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8)   \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))

RAIL_DECLARE_TX_POWER_VBAT_CURVES_ALT;

// This chip has the same curve for Vbat and DCDC
#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
const RAIL_TxPowerCurvesConfigAlt_t RAIL_TxPowerCurvesVbat = RAIL_DECLARE_TX_POWER_CURVES_CONFIG_ALT;

#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
const RAIL_TxPowerCurvesConfigAlt_t RAIL_TxPowerCurvesDcdc = RAIL_DECLARE_TX_POWER_CURVES_CONFIG_ALT;

#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_5)

static const int32_t RAIL_curvesOFDM[RAIL_PA_CURVES_OFDM_CURVES_NUM_VALUES] =
  RAIL_PA_CURVES_OFDM_CURVES;

static const int32_t RAIL_curvesSubgig[RAIL_PA_CURVES_SUBGIG_CURVES_NUM_VALUES] =
  RAIL_PA_CURVES_SUBGIG_CURVES;

// This chip has the same curve for Vbat and DCDC
#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
const RAIL_TxPowerCurvesConfigAlt_t RAIL_TxPowerCurvesVbat = RAIL_DECLARE_TX_POWER_CURVES_CONFIG_ALT;

#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
const RAIL_TxPowerCurvesConfigAlt_t RAIL_TxPowerCurvesDcdc = RAIL_DECLARE_TX_POWER_CURVES_CONFIG_ALT;

#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)

static const RAIL_TxPowerCurveAlt_t RAIL_piecewiseDataHpVbat = {
  RAIL_PA_CURVES_2P4_HP_VBAT_MAX_POWER,
  RAIL_PA_CURVES_2P4_HP_VBAT_MIN_POWER,
  RAIL_PA_CURVES_2P4_HP_VBAT_CURVES,
};
static const RAIL_TxPowerCurveAlt_t RAIL_piecewiseDataMpVbat = {
  RAIL_PA_CURVES_2P4_MP_VBAT_MAX_POWER,
  RAIL_PA_CURVES_2P4_MP_VBAT_MIN_POWER,
  RAIL_PA_CURVES_2P4_MP_VBAT_CURVES,
};
static const RAIL_TxPowerCurveAlt_t RAIL_piecewiseDataLpVbat = {
  RAIL_PA_CURVES_2P4_LP_VBAT_MAX_POWER,
  RAIL_PA_CURVES_2P4_LP_VBAT_MIN_POWER,
  RAIL_PA_CURVES_2P4_LP,
};

#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
const RAIL_TxPowerCurvesConfigAlt_t RAIL_TxPowerCurvesVbat = {
  .curves = {
    {
      .algorithm = RAIL_PA_ALGORITHM_PIECEWISE_LINEAR,
      .segments = RAIL_PA_CURVES_PIECEWISE_SEGMENTS,
      .min = RAIL_TX_POWER_LEVEL_2P4_HP_MIN,
      .max = RAIL_TX_POWER_LEVEL_2P4_HP_MAX,
      .conversion = { .powerCurve = &RAIL_piecewiseDataHpVbat },
    },
    {
      .algorithm = RAIL_PA_ALGORITHM_PIECEWISE_LINEAR,
      .segments = RAIL_PA_CURVES_PIECEWISE_SEGMENTS,
      .min = RAIL_TX_POWER_LEVEL_2P4_MP_MIN,
      .max = RAIL_TX_POWER_LEVEL_2P4_MP_MAX,
      .conversion = { .powerCurve = &RAIL_piecewiseDataMpVbat },
    },
    {
      .algorithm = RAIL_PA_ALGORITHM_PIECEWISE_LINEAR,
      .segments = RAIL_PA_CURVES_PIECEWISE_SEGMENTS,
      .min = RAIL_TX_POWER_LEVEL_2P4_LP_MIN,
      .max = RAIL_TX_POWER_LEVEL_2P4_LP_MAX,
      .conversion = { .powerCurve = &RAIL_piecewiseDataLpVbat },
    },
  }
};

#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
const RAIL_TxPowerCurvesConfigAlt_t RAIL_TxPowerCurvesDcdc = {
  .curves = {
    {
      .algorithm = RAIL_PA_ALGORITHM_PIECEWISE_LINEAR,
      .segments = RAIL_PA_CURVES_PIECEWISE_SEGMENTS,
      .min = RAIL_TX_POWER_LEVEL_HP_MIN,
      .max = RAIL_TX_POWER_LEVEL_HP_MAX,
      .conversion = { .powerCurve = &RAIL_piecewiseDataHpVbat },
    },
    {
      .algorithm = RAIL_PA_ALGORITHM_PIECEWISE_LINEAR,
      .segments = RAIL_PA_CURVES_PIECEWISE_SEGMENTS,
      .min = RAIL_TX_POWER_LEVEL_MP_MIN,
      .max = RAIL_TX_POWER_LEVEL_MP_MAX,
      .conversion = { .powerCurve = &RAIL_piecewiseDataMpVbat },
    },
    {
      .algorithm = RAIL_PA_ALGORITHM_PIECEWISE_LINEAR,
      .segments = RAIL_PA_CURVES_PIECEWISE_SEGMENTS,
      .min = RAIL_TX_POWER_LEVEL_LP_MIN,
      .max = RAIL_TX_POWER_LEVEL_LP_MAX,
      .conversion = { .powerCurve = &RAIL_piecewiseDataLpVbat },
    },
  }
};
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_4) || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_6)
static const RAIL_TxPowerCurveAlt_t RAIL_piecewiseDataHpVbat = {
  RAIL_PA_CURVES_2P4_HP_VBAT_MAX_POWER,
  RAIL_PA_CURVES_2P4_HP_VBAT_MIN_POWER,
  RAIL_PA_CURVES_2P4_HP_VBAT_CURVES,
};

#if defined(RAIL_DECLARE_TX_POWER_DCDC_CURVES_ALT)
static const RAIL_TxPowerCurveAlt_t RAIL_piecewiseDataHpDcdc = {
  RAIL_PA_CURVES_2P4_HP_DCDC_MAX_POWER,
  RAIL_PA_CURVES_2P4_HP_DCDC_MIN_POWER,
  RAIL_PA_CURVES_2P4_HP_DCDC_CURVES,
};
#endif

static const int16_t RAIL_curves24Lp[RAIL_PA_CURVES_LP_VALUES] =
  RAIL_PA_CURVES_2P4_LP_VBAT_CURVES;

#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
const RAIL_TxPowerCurvesConfigAlt_t RAIL_TxPowerCurvesVbat = {
  .curves = {
    {
      .algorithm = RAIL_PA_ALGORITHM_PIECEWISE_LINEAR,
      .segments = RAIL_PA_CURVES_PIECEWISE_SEGMENTS,
      .min = RAIL_TX_POWER_LEVEL_2P4_HP_MIN,
      .max = RAIL_TX_POWER_LEVEL_2P4_HP_MAX,
      .conversion = { .powerCurve = &RAIL_piecewiseDataHpVbat },
    },
    {
      .algorithm = RAIL_PA_ALGORITHM_MAPPING_TABLE,
      .segments = 0U,
      .min = RAIL_TX_POWER_LEVEL_2P4_LP_MIN,
      .max = RAIL_TX_POWER_LEVEL_2P4_LP_MAX,
      .conversion = { .mappingTable = &RAIL_curves24Lp[0] },
    },
  }
};

#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
const RAIL_TxPowerCurvesConfigAlt_t RAIL_TxPowerCurvesDcdc = {
  .curves = {
    {
      .algorithm = RAIL_PA_ALGORITHM_PIECEWISE_LINEAR,
      .segments = RAIL_PA_CURVES_PIECEWISE_SEGMENTS,
      .min = RAIL_TX_POWER_LEVEL_2P4_HP_MIN,
      .max = RAIL_TX_POWER_LEVEL_2P4_HP_MAX,
#if defined(RAIL_DECLARE_TX_POWER_DCDC_CURVES_ALT)
      .conversion = { .powerCurve = &RAIL_piecewiseDataHpDcdc },
#else
      .conversion = { .powerCurve = &RAIL_piecewiseDataHpVbat },
#endif
    },
    {
      .algorithm = RAIL_PA_ALGORITHM_MAPPING_TABLE,
      .segments = 0U,
      .min = RAIL_TX_POWER_LEVEL_2P4_LP_MIN,
      .max = RAIL_TX_POWER_LEVEL_2P4_LP_MAX,
      .conversion = { .mappingTable = &RAIL_curves24Lp[0] },
    },
  }
};

#elif defined(_SILICON_LABS_32B_SERIES_3)

static const int16_t RAIL_curves10dbm[RAIL_PA_CURVES_COMMON_INTERFACE_10DBM_NUM_VALUES] =
  RAIL_PA_CURVES_COMMON_INTERFACE_10DBM_CURVES;

static const int16_t RAIL_curves0dbm[RAIL_PA_CURVES_COMMON_INTERFACE_0DBM_NUM_VALUES] =
  RAIL_PA_CURVES_COMMON_INTERFACE_0DBM_CURVES;

// This chip has the same curve for Vbat and DCDC
#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
const RAIL_TxPowerCurvesConfigAlt_t RAIL_TxPowerCurvesVbat = RAIL_DECLARE_TX_POWER_CURVES_CONFIG_ALT;

#ifdef RAIL_PA_CONVERSIONS_WEAK
__WEAK
#endif
const RAIL_TxPowerCurvesConfigAlt_t RAIL_TxPowerCurvesDcdc = RAIL_DECLARE_TX_POWER_CURVES_CONFIG_ALT;

#else
#ifndef RAIL_INTERNAL_BUILD
#error "Unsupported platform!"
#endif
#endif

#endif //_SILICON_LABS_MODULE
