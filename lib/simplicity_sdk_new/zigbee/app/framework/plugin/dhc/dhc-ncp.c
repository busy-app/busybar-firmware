/***************************************************************************//**
 * @file
 * @brief Zigbee Dynamic Hardware Configuration (DHC) NCP API
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: LicenseRef-MSLA
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of the Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement
 * By installing, copying or otherwise using this software, you agree to the
 * terms of the MSLA.
 *
 ******************************************************************************/

#include <stdint.h>
#include "sl_status.h"
#include "sl_rail.h"
#include "sl_rail_types.h"
#include "sl_rail_util_compatible_pa.h"
#include "sl_rail_util_pa_nvm_configs.h"
#include "dhc-ncp.h"
#include "sl_zigbee_dhc.h"
#include "sl_clock_manager.h"
#include "sl_clock_manager_oscillator_calibration_override.h"

// NVM read returns NOT_FOUND when no PA config exists; status determines "no config", not version.
// Write path when NOT_FOUND uses a zero-initialized config (compound literal) to avoid memset bloat.

// Metadata
sl_status_t sli_zigbee_stack_read_pa_metadata(sl_zigbee_dhc_pa_metadata_t *metadata)
{
  if (!metadata) {
    return SL_STATUS_NULL_POINTER;
  }
  sl_rail_nvm_pa_config_t cfg;
  sl_status_t st = sl_rail_util_pa_nvm_read_config(&cfg);
  if (st != SL_STATUS_OK) {
    return st;
  }
  metadata->version = cfg.version;
  metadata->num_descriptors = cfg.num_descriptors;
  metadata->pa_voltage = cfg.pa_voltage;
  metadata->signature = cfg.signature;
  return SL_STATUS_OK;
}
sl_status_t sli_zigbee_stack_write_pa_metadata(sl_zigbee_dhc_pa_metadata_t *metadata)
{
  if (!metadata) {
    return SL_STATUS_NULL_POINTER;
  }
  if (metadata->num_descriptors != SL_RAIL_NVM_PA_COUNT) {
    return SL_STATUS_INVALID_COUNT;
  }
  sl_rail_nvm_pa_config_t cfg;
  sl_status_t st = sl_rail_util_pa_nvm_read_config(&cfg);
  if (st == SL_STATUS_NOT_FOUND) {
    cfg = (sl_rail_nvm_pa_config_t){ 0 };
  } else if (st != SL_STATUS_OK) {
    return st;
  }
  cfg.version = metadata->version;
  cfg.num_descriptors = metadata->num_descriptors;
  cfg.pa_voltage = metadata->pa_voltage;
  cfg.signature = metadata->signature;
  return sl_rail_util_pa_nvm_write_config(&cfg);
}

// Descriptor (index maps directly into pa_descriptors[])
sl_status_t sli_zigbee_stack_read_pa_descriptor(uint8_t index, sl_zigbee_dhc_pa_descriptor_t *descriptor)
{
  if (!descriptor) {
    return SL_STATUS_NULL_POINTER;
  }
  sl_rail_nvm_pa_config_t cfg;
  sl_status_t st = sl_rail_util_pa_nvm_read_config(&cfg);
  if (st != SL_STATUS_OK) {
    return st;
  }
  if (index >= cfg.num_descriptors) {
    return SL_STATUS_INVALID_INDEX;
  }
  sl_rail_nvm_pa_descriptor_t *d = &cfg.pa_descriptors[index];
  descriptor->algorithm = (uint8_t)d->algorithm;
  descriptor->num_segments_or_entries = d->num_segments_or_entries;
  descriptor->min_ddbm = (int16_t)d->min;
  descriptor->max_ddbm = (int16_t)d->max;
  return SL_STATUS_OK;
}
sl_status_t sli_zigbee_stack_write_pa_descriptor(uint8_t index, sl_zigbee_dhc_pa_descriptor_t *descriptor)
{
  if (!descriptor) {
    return SL_STATUS_NULL_POINTER;
  }
  sl_rail_nvm_pa_config_t cfg;
  sl_status_t st = sl_rail_util_pa_nvm_read_config(&cfg);
  if (st == SL_STATUS_NOT_FOUND) {
    cfg = (sl_rail_nvm_pa_config_t){ 0 };
  } else if (st != SL_STATUS_OK) {
    return st;
  }
  if (index >= SL_RAIL_NVM_PA_COUNT) {
    return SL_STATUS_INVALID_INDEX;
  }
  sl_rail_nvm_pa_descriptor_t *d = &cfg.pa_descriptors[index];
  d->algorithm = (RAIL_PaConversionAlgorithm_t)descriptor->algorithm;
  d->num_segments_or_entries = descriptor->num_segments_or_entries;
  d->min = (RAIL_TxPowerLevel_t)descriptor->min_ddbm;
  d->max = (RAIL_TxPowerLevel_t)descriptor->max_ddbm;
  if (index >= cfg.num_descriptors) {
    cfg.num_descriptors = index + 1;
  }
  return sl_rail_util_pa_nvm_write_config(&cfg);
}

// Curve (we assume single curve per PA index accessing union.curve)
sl_status_t sli_zigbee_stack_read_pa_curve(uint8_t index, sl_zigbee_dhc_pa_curve_t *curve)
{
  if (!curve) {
    return SL_STATUS_NULL_POINTER;
  }
  sl_rail_nvm_pa_config_t cfg;
  sl_status_t st = sl_rail_util_pa_nvm_read_config(&cfg);
  if (st != SL_STATUS_OK) {
    return st;
  }
  if (index >= cfg.num_descriptors) {
    return SL_STATUS_INVALID_INDEX;
  }
  sl_rail_nvm_pa_curve_t *c = &cfg.pa_curve_or_table[index].curve;
  curve->curve_min_ddbm = c->curve_min_ddbm;
  curve->curve_max_ddbm = c->curve_max_ddbm;
  for (uint8_t i = 0; i < SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT; i++) {
    curve->segments[i].maxPowerLevel = c->curve_segments[i].maxPowerLevel;
    curve->segments[i].slope = c->curve_segments[i].slope;
    curve->segments[i].intercept = c->curve_segments[i].intercept;
  }
  return SL_STATUS_OK;
}
sl_status_t sli_zigbee_stack_write_pa_curve(uint8_t index, sl_zigbee_dhc_pa_curve_t *curve)
{
  if (!curve) {
    return SL_STATUS_NULL_POINTER;
  }
  sl_rail_nvm_pa_config_t cfg;
  sl_status_t st = sl_rail_util_pa_nvm_read_config(&cfg);
  if (st == SL_STATUS_NOT_FOUND) {
    cfg = (sl_rail_nvm_pa_config_t){ 0 };
  } else if (st != SL_STATUS_OK) {
    return st;
  }
  if (index >= SL_RAIL_NVM_PA_COUNT) {
    return SL_STATUS_INVALID_INDEX;
  }
  sl_rail_nvm_pa_curve_t *c = &cfg.pa_curve_or_table[index].curve;
  c->curve_min_ddbm = curve->curve_min_ddbm;
  c->curve_max_ddbm = curve->curve_max_ddbm;
  for (uint8_t i = 0; i < SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT; i++) {
    c->curve_segments[i].maxPowerLevel = curve->segments[i].maxPowerLevel;
    c->curve_segments[i].slope = curve->segments[i].slope;
    c->curve_segments[i].intercept = curve->segments[i].intercept;
  }
  return sl_rail_util_pa_nvm_write_config(&cfg);
}

sl_status_t sli_zigbee_stack_read_pa_curve_segment(uint8_t index, uint8_t segment_index, sl_zigbee_dhc_pa_curve_segment_t *segment)
{
  if (!segment) {
    return SL_STATUS_NULL_POINTER;
  }
  if (segment_index >= SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT) {
    return SL_STATUS_INVALID_INDEX;
  }
  sl_rail_nvm_pa_config_t cfg;
  sl_status_t st = sl_rail_util_pa_nvm_read_config(&cfg);
  if (st != SL_STATUS_OK) {
    return st;
  }
  if (index >= cfg.num_descriptors) {
    return SL_STATUS_INVALID_INDEX;
  }
  sl_rail_nvm_pa_curve_t *c = &cfg.pa_curve_or_table[index].curve;
  segment->maxPowerLevel = c->curve_segments[segment_index].maxPowerLevel;
  segment->slope = c->curve_segments[segment_index].slope;
  segment->intercept = c->curve_segments[segment_index].intercept;
  return SL_STATUS_OK;
}
sl_status_t sli_zigbee_stack_write_pa_curve_segment(uint8_t index, uint8_t segment_index, sl_zigbee_dhc_pa_curve_segment_t *segment)
{
  if (!segment) {
    return SL_STATUS_NULL_POINTER;
  }
  if (segment_index >= SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT) {
    return SL_STATUS_INVALID_INDEX;
  }
  sl_rail_nvm_pa_config_t cfg;
  sl_status_t st = sl_rail_util_pa_nvm_read_config(&cfg);
  if (st == SL_STATUS_NOT_FOUND) {
    cfg = (sl_rail_nvm_pa_config_t){ 0 };
  } else if (st != SL_STATUS_OK) {
    return st;
  }
  if (index >= SL_RAIL_NVM_PA_COUNT) {
    return SL_STATUS_INVALID_INDEX;
  }
  sl_rail_nvm_pa_curve_t *c = &cfg.pa_curve_or_table[index].curve;
  c->curve_segments[segment_index].maxPowerLevel = segment->maxPowerLevel;
  c->curve_segments[segment_index].slope = segment->slope;
  c->curve_segments[segment_index].intercept = segment->intercept;
  return sl_rail_util_pa_nvm_write_config(&cfg);
}

// Table
sl_status_t sli_zigbee_stack_read_pa_table(uint8_t index, sl_zigbee_dhc_pa_table_t *table)
{
  if (!table) {
    return SL_STATUS_NULL_POINTER;
  }
  sl_rail_nvm_pa_config_t cfg;
  sl_status_t st = sl_rail_util_pa_nvm_read_config(&cfg);
  if (st != SL_STATUS_OK) {
    return st;
  }
  if (index >= cfg.num_descriptors) {
    return SL_STATUS_INVALID_INDEX;
  }
  sl_rail_nvm_pa_table_t *t = &cfg.pa_curve_or_table[index].table;
  for (uint8_t i = 0; i < SL_ZIGBEE_DHC_TABLE_ENTRY_COUNT; i++) {
    table->ddbm_values[i] = t->ddbm_values[i];
  }
  return SL_STATUS_OK;
}
sl_status_t sli_zigbee_stack_write_pa_table(uint8_t index, sl_zigbee_dhc_pa_table_t *table)
{
  if (!table) {
    return SL_STATUS_NULL_POINTER;
  }
  sl_rail_nvm_pa_config_t cfg;
  sl_status_t st = sl_rail_util_pa_nvm_read_config(&cfg);
  if (st == SL_STATUS_NOT_FOUND) {
    cfg = (sl_rail_nvm_pa_config_t){ 0 };
  } else if (st != SL_STATUS_OK) {
    return st;
  }
  if (index >= SL_RAIL_NVM_PA_COUNT) {
    return SL_STATUS_INVALID_INDEX;
  }
  sl_rail_nvm_pa_table_t *t = &cfg.pa_curve_or_table[index].table;
  for (uint8_t i = 0; i < SL_ZIGBEE_DHC_TABLE_ENTRY_COUNT; i++) {
    t->ddbm_values[i] = table->ddbm_values[i];
  }
  return sl_rail_util_pa_nvm_write_config(&cfg);
}

// Scalar wrappers implemented directly against RAIL / clock manager
sl_status_t sli_zigbee_stack_read_rssi_offset(sl_zigbee_dhc_rssi_offset_t *rssi_offset)
{
  if (!rssi_offset) {
    return SL_STATUS_NULL_POINTER;
  }
  rssi_offset->rssi_offset = (int8_t)sl_rail_get_rssi_offset(SL_RAIL_EFR32_HANDLE);
  return SL_STATUS_OK;
}
sl_status_t sli_zigbee_stack_write_rssi_offset(sl_zigbee_dhc_rssi_offset_t *rssi_offset)
{
  if (!rssi_offset) {
    return SL_STATUS_NULL_POINTER;
  }
  return sl_rail_set_rssi_offset(SL_RAIL_EFR32_HANDLE, rssi_offset->rssi_offset);
}

sl_status_t sli_zigbee_stack_read_pa_voltage(uint16_t *pa_voltage)
{
  if (!pa_voltage) {
    return SL_STATUS_NULL_POINTER;
  }
  sl_rail_nvm_pa_config_t cfg;
  sl_status_t st = sl_rail_util_pa_nvm_read_config(&cfg);
  if (st != SL_STATUS_OK) {
    return st;
  }
  *pa_voltage = cfg.pa_voltage;
  return SL_STATUS_OK;
}
sl_status_t sli_zigbee_stack_write_pa_voltage(uint16_t pa_voltage)
{
  sl_rail_nvm_pa_config_t cfg;
  sl_status_t st = sl_rail_util_pa_nvm_read_config(&cfg);
  if (st == SL_STATUS_NOT_FOUND) {
    cfg = (sl_rail_nvm_pa_config_t){ 0 };
  } else if (st != SL_STATUS_OK) {
    return st;
  }
  cfg.pa_voltage = pa_voltage;
  return sl_rail_util_pa_nvm_write_config(&cfg);
}

sl_status_t sli_zigbee_stack_read_pa_mode(sl_zigbee_dhc_pa_mode_t *pa_mode)
{
  if (!pa_mode) {
    return SL_STATUS_NULL_POINTER;
  }
  pa_mode->pa_mode = (uint8_t)sl_rail_util_pa_nvm_read_mode();
  return SL_STATUS_OK;
}
sl_status_t sli_zigbee_stack_write_pa_mode(sl_zigbee_dhc_pa_mode_t *pa_mode)
{
  if (!pa_mode) {
    return SL_STATUS_NULL_POINTER;
  }
  return sl_rail_util_pa_nvm_write_mode((RAIL_TxPowerMode_t)pa_mode->pa_mode);
}

sl_status_t sli_zigbee_stack_read_ctune(sl_zigbee_dhc_ctune_t *ctune)
{
  if (!ctune) {
    return SL_STATUS_NULL_POINTER;
  }
  uint32_t val = 0;
  sl_status_t st = slx_clock_manager_hfxo_get_ctune(&val);
  if (st != SL_STATUS_OK) {
    return st;
  }
  ctune->ctune = val;
  return SL_STATUS_OK;
}
sl_status_t sli_zigbee_stack_write_ctune(sl_zigbee_dhc_ctune_t *ctune)
{
  if (!ctune) {
    return SL_STATUS_NULL_POINTER;
  }
  uint32_t val = ctune->ctune;
  sl_status_t st = slx_clock_manager_hfxo_set_ctune(val);
  if (st != SL_STATUS_OK) {
    return st;
  }
  /* Persist to NVM3 so ctune survives gateway restart and NCP reset. */
  return sl_clock_manager_write_hfxo_calibration_override(val);
}

// DHC version separate from metadata.version (store locally in signature field upper bits for now)
static uint8_t g_dhc_version_cache = SL_ZIGBEE_DHC_VERSION;
sl_status_t sli_zigbee_stack_read_dhc_version(uint8_t *dhc_version)
{
  if (!dhc_version) {
    return SL_STATUS_NULL_POINTER;
  }
  *dhc_version = g_dhc_version_cache;
  return SL_STATUS_OK;
}
sl_status_t sli_zigbee_stack_write_dhc_version(uint8_t dhc_version)
{
  g_dhc_version_cache = dhc_version;
  return SL_STATUS_OK;
}

// PA version wrapper (use metadata.version for now)
sl_status_t sli_zigbee_stack_read_pa_version(sl_zigbee_dhc_pa_version_t *pa_version)
{
  if (!pa_version) {
    return SL_STATUS_NULL_POINTER;
  }
  sl_rail_nvm_pa_config_t cfg;
  sl_status_t st = sl_rail_util_pa_nvm_read_config(&cfg);
  if (st != SL_STATUS_OK) {
    return st;
  }
  pa_version->pa_version = cfg.version;
  return SL_STATUS_OK;
}

// PA signature wrapper
sl_status_t sli_zigbee_stack_read_pa_signature(sl_zigbee_dhc_pa_signature_t *pa_signature)
{
  if (!pa_signature) {
    return SL_STATUS_NULL_POINTER;
  }
  sl_rail_nvm_pa_config_t cfg;
  sl_status_t st = sl_rail_util_pa_nvm_read_config(&cfg);
  if (st != SL_STATUS_OK) {
    return st;
  }
  pa_signature->pa_signature = cfg.signature;
  return SL_STATUS_OK;
}
sl_status_t sli_zigbee_stack_write_pa_signature(sl_zigbee_dhc_pa_signature_t *pa_signature)
{
  if (!pa_signature) {
    return SL_STATUS_NULL_POINTER;
  }
  sl_rail_nvm_pa_config_t cfg;
  sl_status_t st = sl_rail_util_pa_nvm_read_config(&cfg);
  if (st == SL_STATUS_NOT_FOUND) {
    cfg = (sl_rail_nvm_pa_config_t){ 0 };
  } else if (st != SL_STATUS_OK) {
    return st;
  }
  cfg.signature = pa_signature->pa_signature;
  return sl_rail_util_pa_nvm_write_config(&cfg);
}
