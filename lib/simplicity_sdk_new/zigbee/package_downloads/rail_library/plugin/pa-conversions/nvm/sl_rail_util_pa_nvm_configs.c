/***************************************************************************//**
 * @file
 * @brief This provides functions to store and retrieve PA calibrations
 *  from NVM3.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "rail.h"
#include "sl_rail_util_pa_nvm_configs.h"
#include "nvm3_default.h"

#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 4) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6))

#if     (SL_RAIL_NVM_PA_COUNT != RAIL_NUM_PA)
#error "SL_RAIL_NVM_PA_COUNT does not match RAIL_NUM_PA"
#endif//(SL_RAIL_NVM_PA_COUNT != RAIL_NUM_PA)
_Static_assert((sizeof(sl_rail_nvm_pa_curve_t) % 4) == 0,
               "sl_rail_nvm_pa_curve_t size is not multiple of 4");
_Static_assert((sizeof(sl_rail_nvm_pa_table_t) % 4) == 0,
               "sl_rail_nvm_pa_mapping_table_t size is not multiple of 4");
_Static_assert((sizeof(sl_rail_nvm_pa_descriptor_t) % 4) == 0,
               "sl_rail_nvm_pa_descriptor_t size is not multiple of 4");
_Static_assert((sizeof(sl_rail_nvm_pa_config_t) % 4) == 0,
               "sl_rail_nvm_pa_config_t size is not multiple of 4");

sl_status_t sl_rail_util_pa_nvm_write_mode(RAIL_TxPowerMode_t pa_mode)
{
  return nvm3_writeData(nvm3_defaultHandle,
                        SL_RAIL_UTIL_PA_NVM_MODE_TAG,
                        &pa_mode,
                        sizeof(pa_mode));
}

RAIL_TxPowerMode_t sl_rail_util_pa_nvm_read_mode(void)
{
  RAIL_TxPowerMode_t pa_mode;
  sl_status_t status = nvm3_readData(nvm3_defaultHandle,
                                     SL_RAIL_UTIL_PA_NVM_MODE_TAG,
                                     &pa_mode,
                                     sizeof(pa_mode));
  if ((status == SL_STATUS_OK)
      && RAIL_SupportsTxPowerModeAlt(RAIL_EFR32_HANDLE, &pa_mode, NULL, NULL)) {
    return pa_mode;
  }
  return RAIL_TX_POWER_MODE_NONE;
}

sl_status_t sl_rail_util_pa_nvm_write_config(const sl_rail_nvm_pa_config_t *p_pa_config)
{
  return nvm3_writeData(nvm3_defaultHandle,
                        SL_RAIL_UTIL_PA_NVM_CONFIG_TAG,
                        p_pa_config,
                        sizeof(*p_pa_config));
}

sl_status_t sl_rail_util_pa_nvm_read_config(sl_rail_nvm_pa_config_t *p_pa_config)
{
  sl_status_t status;
  uint32_t obj_type;
  size_t obj_size;

  status = nvm3_getObjectInfo(nvm3_defaultHandle,
                              SL_RAIL_UTIL_PA_NVM_CONFIG_TAG,
                              &obj_type, &obj_size);
  if (status != SL_STATUS_OK) {
    return status;
  }
  if ((obj_type != NVM3_OBJECTTYPE_DATA)
      || (obj_size != sizeof(*p_pa_config))) {
    return SL_STATUS_INVALID_TYPE;
  }
  status = nvm3_readData(nvm3_defaultHandle,
                         SL_RAIL_UTIL_PA_NVM_CONFIG_TAG,
                         p_pa_config,
                         sizeof(*p_pa_config));
  if (status != SL_STATUS_OK) {
    return status;
  }
  if (p_pa_config->version != SL_RAIL_NVM_PA_CONFIGS_VERSION) {
    return SL_STATUS_INVALID_SIGNATURE;
  }
  if (p_pa_config->num_descriptors != SL_RAIL_NVM_PA_COUNT) {
    return SL_STATUS_INVALID_COUNT;
  }
  return SL_STATUS_OK;
}

// This structure is a fixed-size instantiation of RAIL_TxPowerCurveAlt_t.
// It's field offsets and semantics must match.
typedef struct {
  int16_t max_power_ddbm;
  int16_t min_power_ddbm;
  RAIL_TxPowerCurveSegment_t power_params[SL_RAIL_NVM_PA_CURVE_SEGMENTS];
} fixed_size_curve_t;

// Static storage for holding the deserialized RAIL PA configuration.
static fixed_size_curve_t tx_curves[SL_RAIL_NVM_PA_COUNT];
static sl_rail_nvm_pa_table_t tx_tables[SL_RAIL_NVM_PA_COUNT];
static RAIL_TxPowerCurvesConfigAlt_t tx_config;

RAIL_TxPowerCurvesConfigAlt_t *sli_rail_util_pa_nvm_deserialize_config(const sl_rail_nvm_pa_config_t *p_pa_config)
{
  if ((p_pa_config->version != SL_RAIL_NVM_PA_CONFIGS_VERSION)
      || (p_pa_config->num_descriptors != SL_RAIL_NVM_PA_COUNT)) {
    return NULL;
  }

  // Start by clearing any earlier config
  memset(&tx_curves, 0, sizeof(tx_curves));
  memset(&tx_tables, 0, sizeof(tx_tables));
  memset(&tx_config, 0, sizeof(tx_config));

  tx_config.paVoltage = p_pa_config->pa_voltage;
  tx_config.signature = p_pa_config->signature;
  // Now for tx_config.curves[]...
  for (unsigned int desc = 0U; desc < p_pa_config->num_descriptors; desc++) {
    RAIL_PaDescriptor_t *p_tx_desc = &tx_config.curves[desc];
    const sl_rail_nvm_pa_descriptor_t *p_pa_desc = &p_pa_config->pa_descriptors[desc];
    RAIL_PaConversionAlgorithm_t algo = p_pa_desc->algorithm;
    if (algo >= RAIL_PA_ALGORITHM_COUNT) {
      return NULL;
    }
    uint8_t segs = p_pa_desc->num_segments_or_entries;
    if (algo == RAIL_PA_ALGORITHM_PIECEWISE_LINEAR) {
      // Support fewer segments than allowed
      if (segs > SL_RAIL_NVM_PA_CURVE_SEGMENTS) {
        return NULL;
      }
      // Curve contents are not validated
      fixed_size_curve_t *p_tx_curve = &tx_curves[desc];
      const sl_rail_nvm_pa_curve_t *p_pa_curve = &p_pa_config->pa_curve_or_table[desc].curve;
      p_tx_curve->min_power_ddbm = p_pa_curve->curve_min_ddbm;
      p_tx_curve->max_power_ddbm = p_pa_curve->curve_max_ddbm;
      memcpy(&p_tx_curve->power_params, &p_pa_curve->curve_segments,
             segs * sizeof(RAIL_TxPowerCurveSegment_t));
      p_tx_desc->conversion.powerCurve = (const RAIL_TxPowerCurveAlt_t *)p_tx_curve;
    } else { // Mapping table
      // Only support the exact number of table entries needed
      if (segs != SL_RAIL_NVM_PA_TABLE_ENTRIES) {
        return NULL;
      }
      // Table contents are not validated
      sl_rail_nvm_pa_table_t *p_tx_table = &tx_tables[desc];
      const sl_rail_nvm_pa_table_t *p_pa_table = &p_pa_config->pa_curve_or_table[desc].table;
      memcpy(p_tx_table, p_pa_table, sizeof(sl_rail_nvm_pa_table_t));
      tx_config.curves[desc].conversion.mappingTable = (const int16_t *)p_tx_table;
    }
    p_tx_desc->algorithm = algo;
    p_tx_desc->segments = segs;
    // min/max are not validated
    p_tx_desc->min = p_pa_desc->min;
    p_tx_desc->max = p_pa_desc->max;
  }
  return &tx_config;
}

sl_status_t sli_rail_util_pa_nvm_serialize_config(const RAIL_TxPowerCurvesConfigAlt_t *p_tx_curves,
                                                  sl_rail_nvm_pa_config_t *p_pa_config)
{
  // The RAIL_TxPowerCurvesConfigAlt_t structures have few little sizing
  // information embedded, so we assume they have exactly
  // SL_RAIL_NVM_PA_COUNT curves[] and a table has exactly
  // SL_RAIL_NVM_PA_TABLE_ENTRIES entries. For curves we
  // validate that its segments are <= SL_RAIL_NVM_PA_CURVE_SEGMENTS.
  // There is no other validation of the p_tx_curves content.

  // Start by clearing the output config
  memset(p_pa_config, 0, sizeof(*p_pa_config));

  p_pa_config->version = SL_RAIL_NVM_PA_CONFIGS_VERSION;
  p_pa_config->num_descriptors = SL_RAIL_NVM_PA_COUNT;
  p_pa_config->pa_voltage = p_tx_curves->paVoltage;
  p_pa_config->signature = p_tx_curves->signature;
  // Now for pa_curve_or_table[]...
  for (unsigned int desc = 0U; desc < SL_RAIL_NVM_PA_COUNT; desc++) {
    const RAIL_PaDescriptor_t *p_tx_desc = &p_tx_curves->curves[desc];
    sl_rail_nvm_pa_descriptor_t *p_pa_desc = &p_pa_config->pa_descriptors[desc];
    RAIL_PaConversionAlgorithm_t algo = p_tx_desc->algorithm;
    if (algo == RAIL_PA_ALGORITHM_PIECEWISE_LINEAR) {
      uint8_t segs = p_tx_desc->segments;
      if (segs > SL_RAIL_NVM_PA_CURVE_SEGMENTS) {
        return SL_STATUS_INVALID_COUNT;
      }
      p_pa_desc->num_segments_or_entries = segs;
      const RAIL_TxPowerCurveAlt_t *p_tx_curve = p_tx_desc->conversion.powerCurve;
      sl_rail_nvm_pa_curve_t *p_pa_curve = &p_pa_config->pa_curve_or_table[desc].curve;
      p_pa_curve->curve_min_ddbm = p_tx_curve->minPower;
      p_pa_curve->curve_max_ddbm = p_tx_curve->maxPower;
      memcpy(&p_pa_curve->curve_segments, &p_tx_curve->powerParams,
             segs * sizeof(RAIL_TxPowerCurveSegment_t));
    } else { // Mapping table
      p_pa_desc->num_segments_or_entries = SL_RAIL_NVM_PA_TABLE_ENTRIES;
      const sl_rail_nvm_pa_table_t *p_tx_table = (sl_rail_nvm_pa_table_t *)p_tx_desc->conversion.mappingTable;
      sl_rail_nvm_pa_table_t *p_pa_table = &p_pa_config->pa_curve_or_table[desc].table;
      memcpy(p_pa_table, p_tx_table, sizeof(sl_rail_nvm_pa_table_t));
    }
    p_pa_desc->algorithm = algo;
    p_pa_desc->min = p_tx_desc->min;
    p_pa_desc->max = p_tx_desc->max;
  }
  return SL_STATUS_OK;
}

#if     RAILTEST

#include <stdio.h>
#include <inttypes.h>

/* *INDENT-OFF* */
// Sample out from sli_rail_util_pa_nvm_print_serialized():
static sl_rail_nvm_pa_config_t serialized_pa_config = {
  .version           = 1,
  .num_descriptors   = 2,
  .pa_voltage        = 0,
  .signature         = 0,
  .pa_descriptors    = {
    { .algorithm               = 0,
      .num_segments_or_entries = 8,
      .min                     = 0,
      .max                     = 180,
    },
    { .algorithm               = 1,
      .num_segments_or_entries = 16,
      .min                     = 0,
      .max                     = 15,
    },
  },
  .pa_curve_or_table = {
    { .curve = {
        .curve_min_ddbm = -338,
        .curve_max_ddbm = 200,
        .curve_segments = {
          { .maxPowerLevel = 180,
            .slope         = 2280,
            .intercept     = -291457,
          },
          { .maxPowerLevel = 78,
            .slope         = 770,
            .intercept     = -46749,
          },
          { .maxPowerLevel = 44,
            .slope         = 431,
            .intercept     = -6673,
          },
          { .maxPowerLevel = 27,
            .slope         = 255,
            .intercept     = 6886,
          },
          { .maxPowerLevel = 17,
            .slope         = 167,
            .intercept     = 10458,
          },
          { .maxPowerLevel = 10,
            .slope         = 98,
            .intercept     = 10261,
          },
          { .maxPowerLevel = 6,
            .slope         = 59,
            .intercept     = 8616,
          },
          { .maxPowerLevel = 3,
            .slope         = 11,
            .intercept     = 3745,
          },
        },
      },
    },
    { .table = {
        { -250, -148, -95, -68, -51, -40, -32, -26, -22, -18, -16, -13, -12, -10, -9, -9, },
      },
    },
  },
};
/* *INDENT-ON* */

#define OFFSET_IN_PA_CONFIG(field) ((unsigned)&(field) - (unsigned)p_pa_config)
#define PRINT_OFFSET(offset) (void)(with_offsets ? printf("/*%4u*/", (offset)) : 0)

void sli_rail_util_pa_nvm_print_serialized(const sl_rail_nvm_pa_config_t *p_pa_config,
                                           bool with_offsets)
{
  (void) serialized_pa_config;
  printf("static sl_rail_nvm_pa_config_t serialized_pa_config = {\n");
  PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_pa_config->version));
  printf("  .version           = %u"  ",\n", p_pa_config->version);
  PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_pa_config->num_descriptors));
  printf("  .num_descriptors   = %u"  ",\n", p_pa_config->num_descriptors);
  PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_pa_config->pa_voltage));
  printf("  .pa_voltage        = %u"  ",\n", p_pa_config->pa_voltage);
  PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_pa_config->signature));
  printf("  .signature         = %" PRIu32 ",\n", p_pa_config->signature);
  PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_pa_config->pa_descriptors));
  printf("  .pa_descriptors    = {\n");
  for (unsigned int desc = 0U; desc < p_pa_config->num_descriptors; desc++) {
    const sl_rail_nvm_pa_descriptor_t *p_desc = &p_pa_config->pa_descriptors[desc];
    PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_desc->algorithm));
    printf("    { .algorithm               = %u" ",\n", p_desc->algorithm);
    PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_desc->num_segments_or_entries));
    printf("      .num_segments_or_entries = %u" ",\n", p_desc->num_segments_or_entries);
    PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_desc->min));
    printf("      .min                     = %u" ",\n", p_desc->min);
    PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_desc->max));
    printf("      .max                     = %u" ",\n", p_desc->max);
    PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_desc->max) + sizeof(p_desc->max));
    printf("    },\n");
  }
  PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_pa_config->pa_descriptors)
               + sizeof(p_pa_config->pa_descriptors));
  printf("  },\n");
  PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_pa_config->pa_curve_or_table));
  printf("  .pa_curve_or_table = {\n");
  for (unsigned int desc = 0U; desc < p_pa_config->num_descriptors; desc++) {
    const sl_rail_nvm_pa_descriptor_t *p_desc = &p_pa_config->pa_descriptors[desc];
    if (p_desc->algorithm == RAIL_PA_ALGORITHM_PIECEWISE_LINEAR) {
      const sl_rail_nvm_pa_curve_t *p_pa_curve = &p_pa_config->pa_curve_or_table[desc].curve;
      PRINT_OFFSET(OFFSET_IN_PA_CONFIG(*p_pa_curve));
      printf("    { .curve = {\n");
      PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_pa_curve->curve_min_ddbm));
      printf("        .curve_min_ddbm = %d" ",\n", p_pa_curve->curve_min_ddbm);
      PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_pa_curve->curve_max_ddbm));
      printf("        .curve_max_ddbm = %d" ",\n", p_pa_curve->curve_max_ddbm);
      PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_pa_curve->curve_segments));
      printf("        .curve_segments = {\n");
      for (unsigned int seg = 0U; seg < p_desc->num_segments_or_entries; seg++) {
        const RAIL_TxPowerCurveSegment_t *p_seg = &p_pa_curve->curve_segments[seg];
        PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_seg->maxPowerLevel));
        printf("          { .maxPowerLevel = %u"  ",\n", p_seg->maxPowerLevel);
        PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_seg->slope));
        printf("            .slope         = %d"  ",\n", p_seg->slope);
        PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_seg->intercept));
        printf("            .intercept     = %" PRId32 ",\n", p_seg->intercept);
        PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_seg->intercept)
                     + sizeof(p_seg->intercept));
        printf("          },\n");
      }
      PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_pa_curve->curve_segments)
                   + sizeof(p_pa_curve->curve_segments));
      printf("        },\n");
      PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_pa_curve->curve_segments)
                   + sizeof(p_pa_curve->curve_segments));
    } else {
      const int16_t *p_pa_table_vals = &p_pa_config->pa_curve_or_table[desc].table.ddbm_values[0];
      PRINT_OFFSET(OFFSET_IN_PA_CONFIG(*p_pa_table_vals));
      printf("    { .table = {\n");
      PRINT_OFFSET(OFFSET_IN_PA_CONFIG(*p_pa_table_vals));
      printf("        {");
      for (unsigned int val = 0U; val < p_desc->num_segments_or_entries; val++) {
        printf(" %d,", p_pa_table_vals[val]);
      }
      printf(" },\n");
      PRINT_OFFSET(OFFSET_IN_PA_CONFIG(*p_pa_table_vals)
                   + (p_desc->num_segments_or_entries
                      * sizeof(*p_pa_table_vals)));
    }
    printf("      },\n");
    PRINT_OFFSET(OFFSET_IN_PA_CONFIG(p_pa_config->pa_curve_or_table)
                 + (desc + 1U) * sizeof(*p_pa_config->pa_curve_or_table));
    printf("    },\n");
  }
  PRINT_OFFSET(sizeof(*p_pa_config));
  printf("  },\n");
  printf("};\n");
}

/* *INDENT-OFF* */
/* Sample output from sli_rail_util_pa_nvm_print_serialized_json():
"sl_rail_nvm_pa_curves": {
  "version": 1,
  "num_descriptors": 2,
  "pa_voltage": 0,
  "signature": 0,
  "descriptor_1": {
    "algorithm": 0,
    "num_segments_or_entries": 8,
    "min": 0,
    "max": 180,
    "pa_curve_or_table": {
      "curve": {
        "curve_min_ddbm": -338,
        "curve_max_ddbm": 200,
        "curve_segments": [
          [180, 2280, -291457],
          [ 78,  770,  -46749],
          [ 44,  431,   -6673],
          [ 27,  255,    6886],
          [ 17,  167,   10458],
          [ 10,   98,   10261],
          [  6,   59,    8616],
          [  3,   11,    3745]
        ]
      }
    }
  },
  "descriptor_2": {
    "algorithm": 1,
    "num_segments_or_entries": 16,
    "min": 0,
    "max": 15,
    "pa_curve_or_table": {
      "table":
        [-250, -148, -95, -68, -51, -40, -32, -26, -22, -18, -16, -13, -12, -10, -9, -9]
    }
  }
}
*/
/* *INDENT-ON* */

void sli_rail_util_pa_nvm_print_serialized_json(const sl_rail_nvm_pa_config_t *p_pa_config)
{
  printf("\"sl_rail_nvm_pa_curves\": {\n");
  printf("  \"version\": %u"         ",\n", p_pa_config->version);
  printf("  \"num_descriptors\": %u" ",\n", p_pa_config->num_descriptors);
  printf("  \"pa_voltage\": %u"      ",\n", p_pa_config->pa_voltage);
  printf("  \"signature\": %" PRIu32 ",\n", p_pa_config->signature);
  for (unsigned int desc = 0U; desc < p_pa_config->num_descriptors; desc++) {
    const sl_rail_nvm_pa_descriptor_t *p_desc = &p_pa_config->pa_descriptors[desc];
    printf("  \"descriptor_%u\": {\n", desc + 1U);
    printf("    \"algorithm\": %u" ",\n", p_desc->algorithm);
    printf("    \"num_segments_or_entries\": %u"  ",\n", p_desc->num_segments_or_entries);
    printf("    \"min\": %u"       ",\n", p_desc->min);
    printf("    \"max\": %u"       ",\n", p_desc->max);
    printf("    \"pa_curve_or_table\": {\n");
    if (p_desc->algorithm == RAIL_PA_ALGORITHM_PIECEWISE_LINEAR) {
      const sl_rail_nvm_pa_curve_t *p_pa_curve = &p_pa_config->pa_curve_or_table[desc].curve;
      printf("      \"curve\": {\n");
      printf("        \"curve_min_ddbm\": %d" ",\n", p_pa_curve->curve_min_ddbm);
      printf("        \"curve_max_ddbm\": %d" ",\n", p_pa_curve->curve_max_ddbm);
      printf("        \"curve_segments\": [\n");
      for (unsigned int seg = 0U; seg < p_desc->num_segments_or_entries; seg++) {
        printf("          [%3u, %4d, %7" PRId32 "]%s\n",
               p_pa_curve->curve_segments[seg].maxPowerLevel,
               p_pa_curve->curve_segments[seg].slope,
               p_pa_curve->curve_segments[seg].intercept,
               (((seg + 1U) < p_desc->num_segments_or_entries) ? "," : ""));
      }
      printf("        ]\n");
      printf("      }\n");
    } else {
      const int16_t *p_pa_table_vals = &p_pa_config->pa_curve_or_table[desc].table.ddbm_values[0];
      printf("      \"table\":\n");
      printf("        [");
      for (unsigned int val = 0U; val < p_desc->num_segments_or_entries; val++) {
        printf("%s%d", ((val == 0) ? "" : ", "), p_pa_table_vals[val]);
      }
      printf("]\n");
    }
    printf("    }\n");
    printf("  }%s\n", (((desc + 1U) < p_pa_config->num_descriptors) ? "," : ""));
  }
  printf("}\n");
}

/* *INDENT-OFF* */
// Sample output from sli_rail_util_pa_nvm_print_deserialized():
static RAIL_TxPowerCurveAlt_t curve_1 = {
  .maxPower    = 200,
  .minPower    = -338,
  .powerParams = {
    { 180, 2280, -291457, },
    {  78,  770,  -46749, },
    {  44,  431,   -6673, },
    {  27,  255,    6886, },
    {  17,  167,   10458, },
    {  10,   98,   10261, },
    {   6,   59,    8616, },
    {   3,   11,    3745, },
  },
};
static int16_t table_2[] = {
  -250,
  -148,
  -95,
  -68,
  -51,
  -40,
  -32,
  -26,
  -22,
  -18,
  -16,
  -13,
  -12,
  -10,
  -9,
  -9,
};
static RAIL_TxPowerCurvesConfigAlt_t deserialized_tx_config = {
  .curves    = {
    { .algorithm  = 0,
      .segments   = 8,
      .min        = 0,
      .max        = 180,
      .conversion = { .powerCurve   = &curve_1, },
    },
    { .algorithm  = 1,
      .segments   = 16,
      .min        = 0,
      .max        = 15,
      .conversion = { .mappingTable =  table_2, },
    },
  },
  .signature = 0,
  .paVoltage = 0,
};
/* *INDENT-ON* */

void sli_rail_util_pa_nvm_print_deserialized(const RAIL_TxPowerCurvesConfigAlt_t *p_tx_curves)
{
  (void) deserialized_tx_config;
  for (unsigned int desc = 0U; desc < SL_RAIL_NVM_PA_COUNT; desc++) {
    const RAIL_PaDescriptor_t *p_desc = &p_tx_curves->curves[desc];
    if (p_desc->algorithm == RAIL_PA_ALGORITHM_PIECEWISE_LINEAR) {
      const RAIL_TxPowerCurveAlt_t *p_curve = p_desc->conversion.powerCurve;
      printf("static RAIL_TxPowerCurveAlt_t curve_%u = {\n", desc + 1U);
      printf("  .maxPower    = %d" ",\n", p_curve->maxPower);
      printf("  .minPower    = %d" ",\n", p_curve->minPower);
      printf("  .powerParams = {\n");
      for (unsigned int seg = 0U; seg < p_desc->segments; seg++) {
        const RAIL_TxPowerCurveSegment_t *p_seg = &p_curve->powerParams[seg];
        printf("    { %3u, %4d, %7" PRId32 ", },\n",
               p_seg->maxPowerLevel,
               p_seg->slope,
               p_seg->intercept);
      }
      printf("  },\n");
      printf("};\n");
    } else {
      printf("static int16_t table_%u[] = {\n", desc + 1);
      for (unsigned int val = 0U; val < SL_RAIL_NVM_PA_TABLE_ENTRIES; val++) {
        printf("  %d,\n", p_desc->conversion.mappingTable[val]);
      }
      printf("};\n");
    }
  }
  printf("static RAIL_TxPowerCurvesConfigAlt_t deserialized_tx_config = {\n");
  printf("  .curves    = {\n");
  for (unsigned int desc = 0U; desc < SL_RAIL_NVM_PA_COUNT; desc++) {
    const RAIL_PaDescriptor_t *p_desc = &p_tx_curves->curves[desc];
    printf("    { .algorithm  = %u"  ",\n", p_desc->algorithm);
    printf("      .segments   = %u"  ",\n", p_desc->segments);
    printf("      .min        = %u"  ",\n", p_desc->min);
    printf("      .max        = %u"  ",\n", p_desc->max);
    printf("      .conversion = { ");
    if (p_desc->algorithm == RAIL_PA_ALGORITHM_PIECEWISE_LINEAR) {
      printf(".powerCurve   = &curve_%u, },\n", desc + 1);
    } else {
      printf(".mappingTable =  table_%u, },\n", desc + 1);
    }
    printf("    },\n");
  }
  printf("  },\n");
  printf("  .signature = %" PRIu32 ",\n", p_tx_curves->signature);
  printf("  .paVoltage = %u"  ",\n", p_tx_curves->paVoltage);
  printf("};\n");
}

#endif//RAILTEST

#else//!(_SILICON_LABS_32B_SERIES_2_CONFIG == 4) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6))

// Stub implementations for unsupported platforms

sl_status_t sl_rail_util_pa_nvm_write_mode(RAIL_TxPowerMode_t pa_mode)
{
  (void) pa_mode;
  return SL_STATUS_NOT_AVAILABLE;
}

RAIL_TxPowerMode_t sl_rail_util_pa_nvm_read_mode(void)
{
  return RAIL_TX_POWER_MODE_NONE;
}

sl_status_t sl_rail_util_pa_nvm_write_config(const sl_rail_nvm_pa_config_t *p_pa_config)
{
  (void) p_pa_config;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sl_rail_util_pa_nvm_read_config(sl_rail_nvm_pa_config_t *p_pa_config)
{
  (void) p_pa_config;
  return SL_STATUS_NOT_AVAILABLE;
}

RAIL_TxPowerCurvesConfigAlt_t *sli_rail_util_pa_nvm_deserialize_config(const sl_rail_nvm_pa_config_t *p_pa_config)
{
  (void) p_pa_config;
  return NULL;
}

sl_status_t sli_rail_util_pa_nvm_serialize_config(const RAIL_TxPowerCurvesConfigAlt_t *p_tx_curves,
                                                  sl_rail_nvm_pa_config_t *p_pa_config)
{
  (void) p_tx_curves;
  (void) p_pa_config;
  return SL_STATUS_NOT_AVAILABLE;
}

#if     RAILTEST

void sli_rail_util_pa_nvm_print_serialized(const sl_rail_nvm_pa_config_t *p_pa_config)
{
  (void) p_pa_config;
}

void sli_rail_util_pa_nvm_print_serialized_json(const sl_rail_nvm_pa_config_t *p_pa_config)
{
  (void) p_pa_config;
}

void sli_rail_util_pa_nvm_print_deserialized(const RAIL_TxPowerCurvesConfigAlt_t *p_tx_curves)
{
  (void) p_tx_curves;
}

#endif//RAILTEST

#endif//(_SILICON_LABS_32B_SERIES_2_CONFIG == 4) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6))
