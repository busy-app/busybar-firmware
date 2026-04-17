/***************************************************************************/ /**
 * @file
 * @brief Zigbee Dynamic Hardware Configuration (DHC) Parser API
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "sl_status.h"
#include "sl_zigbee_dhc.h"
#include "dhc-parser.h"
#include "submodules/cjson/cJSON.h"

#ifdef EZSP_HOST
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp.h"
#endif


sl_status_t sl_zigbee_dhc_init_from_json(const char *path, bool dry_run, bool stop_on_error)
{
  if (path == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  uint32_t flags = 0;
  if (dry_run) {
    flags |= DHC_PARSE_FLAG_DRY_RUN;
  }
  if (stop_on_error) {
    flags |= DHC_PARSE_FLAG_STOP_ON_ERROR;
  }
  return sl_zigbee_af_dhc_parse_file(path, flags);
}

// Read an integer field (allows negative). Returns 1 on success, else 0.
static int json_get_int(const cJSON *object, const char *key, int *out_value)
{
  const cJSON *node = cJSON_GetObjectItemCaseSensitive(object, key);
  if (!cJSON_IsNumber(node)) {
    return 0;
  }
  *out_value = node->valueint;
  return 1;
}

// Read a non‑negative integer into an unsigned. Returns 1 on success.
static int json_get_uint(const cJSON *object, const char *key, unsigned *out_value)
{
  int tmp;
  if (!json_get_int(object, key, &tmp) || tmp < 0) {
    return 0;
  }
  *out_value = (unsigned)tmp;
  return 1;
}

// Get an array field or NULL if missing/not an array.
static const cJSON *json_get_array(const cJSON *object, const char *key)
{
  const cJSON *array = cJSON_GetObjectItemCaseSensitive(object, key);
  return cJSON_IsArray(array) ? array : NULL;
}

// Write scalar wrappers if present
static sl_status_t apply_scalars(const cJSON *parent, uint32_t flags)
{
  const cJSON *scalars = cJSON_GetObjectItemCaseSensitive(parent, "scalars");
  if (!cJSON_IsObject(scalars)) {
    return SL_STATUS_OK;
  }
  if (flags & DHC_PARSE_FLAG_DRY_RUN) {
    return SL_STATUS_OK;
  }

  const cJSON *node;
  // rssi_offset
  node = cJSON_GetObjectItemCaseSensitive(scalars, "rssi_offset");
  if (node) {
    if (!cJSON_IsNumber(node)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (!(flags & DHC_PARSE_FLAG_DRY_RUN)) {
      int v = node->valueint;
      if (v < INT8_MIN || v > INT8_MAX) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      sl_zigbee_dhc_rssi_offset_t r = { .rssi_offset = (int8_t)v };
      sl_status_t st = sl_zigbee_dhc_write_rssi_offset(&r);
      if (st != SL_STATUS_OK && st != SL_STATUS_NOT_AVAILABLE) {
        return st;
      }
    }
  }
  // pa_mode
  node = cJSON_GetObjectItemCaseSensitive(scalars, "pa_mode");
  if (node) {
    if (!cJSON_IsNumber(node)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (!(flags & DHC_PARSE_FLAG_DRY_RUN)) {
      sl_zigbee_dhc_pa_mode_t m = { .pa_mode = (uint8_t)node->valueint };
      sl_status_t st = sl_zigbee_dhc_write_pa_mode(&m);
      if (st != SL_STATUS_OK) {
        return st;
      }
    }
  }
  // ctune
  node = cJSON_GetObjectItemCaseSensitive(scalars, "ctune");
  if (node) {
    if (!cJSON_IsNumber(node)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (!(flags & DHC_PARSE_FLAG_DRY_RUN)) {
      int v = node->valueint;
      if (v < 0) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      sl_zigbee_dhc_ctune_t c = { .ctune = (uint32_t)v };
      sl_status_t st = sl_zigbee_dhc_write_ctune(&c);
      if (st != SL_STATUS_OK) {
        return st;
      }
    }
  }
  return SL_STATUS_OK;
}

// Apply metadata
static sl_status_t apply_metadata(const cJSON *parent, uint8_t inferred_num, uint32_t flags)
{
  const cJSON *meta = cJSON_GetObjectItemCaseSensitive(parent, "metadata");
  sl_zigbee_dhc_pa_metadata_t md = { 0 };
  md.version = SL_ZIGBEE_DHC_VERSION;
  md.num_descriptors = inferred_num;
  md.pa_voltage = 0; // default unless provided
  md.signature = 0;  // placeholder unless provided
  if (cJSON_IsObject(meta)) {
    int tmp;
    const cJSON *pv = cJSON_GetObjectItemCaseSensitive(meta, "pa_voltage");
    if (pv) {
      if (!json_get_int(meta, "pa_voltage", &tmp) || tmp < 0 || tmp > 0xFFFF) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      md.pa_voltage = (uint16_t)tmp;
    }
    unsigned u;
    if (json_get_uint(meta, "num_descriptors", &u)) {
      md.num_descriptors = (uint8_t)u;
    }
    const cJSON *sig = cJSON_GetObjectItemCaseSensitive(meta, "signature");
    if (sig) {
      if (!cJSON_IsNumber(sig)) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      md.signature = (uint32_t)sig->valuedouble;
    }
  }
  if (md.num_descriptors > SL_ZIGBEE_DHC_MAX_PA_DESCRIPTORS) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (!(flags & DHC_PARSE_FLAG_DRY_RUN)) {
    return sl_zigbee_dhc_write_pa_metadata(&md);
  }
  return SL_STATUS_OK;
}

// Descriptors array
static sl_status_t apply_descriptors(const cJSON *parent, uint8_t *out_count, uint32_t flags)
{
  const cJSON *arr = json_get_array(parent, "descriptors");
  if (!arr) {
    *out_count = 0;
    return SL_STATUS_OK;
  }
  int n = cJSON_GetArraySize(arr);
  if (n < 0 || n > (int)SL_ZIGBEE_DHC_MAX_PA_DESCRIPTORS) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (out_count) {
    *out_count = (uint8_t)n;
  }
  for (int i = 0; i < n; ++i) {
    const cJSON *obj = cJSON_GetArrayItem(arr, i);
    if (!cJSON_IsObject(obj)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    int algorithm = 0, nse = 0, min_ddbm = 0, max_ddbm = 0;
    if (!json_get_int(obj, "algorithm", &algorithm)
        || !json_get_int(obj, "num_segments_or_entries", &nse)
        || !json_get_int(obj, "min_ddbm", &min_ddbm)
        || !json_get_int(obj, "max_ddbm", &max_ddbm)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (algorithm != SL_ZIGBEE_DHC_ALGO_CURVE && algorithm != SL_ZIGBEE_DHC_ALGO_TABLE) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (nse <= 0 || nse > 255) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (!(flags & DHC_PARSE_FLAG_DRY_RUN)) {
      sl_zigbee_dhc_pa_descriptor_t d = { .algorithm = (uint8_t)algorithm,
                                          .num_segments_or_entries = (uint8_t)nse,
                                          .min_ddbm = (int16_t)min_ddbm,
                                          .max_ddbm = (int16_t)max_ddbm };
      sl_status_t st = sl_zigbee_dhc_write_pa_descriptor((uint8_t)i, &d);
      if (st != SL_STATUS_OK) {
        return st;
      }
    }
  }
  return SL_STATUS_OK;
}

static sl_status_t apply_curves(const cJSON *parent, uint32_t flags)
{
  const cJSON *arr = json_get_array(parent, "curves");
  if (!arr) {
    return SL_STATUS_OK;
  }
  int n = cJSON_GetArraySize(arr);
  for (int i = 0; i < n; ++i) {
    const cJSON *obj = cJSON_GetArrayItem(arr, i);
    if (!cJSON_IsObject(obj)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    int index = 0, cmin = 0, cmax = 0;
    if (!json_get_int(obj, "index", &index)
        || !json_get_int(obj, "curve_min_ddbm", &cmin)
        || !json_get_int(obj, "curve_max_ddbm", &cmax)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    const cJSON *segments = json_get_array(obj, "segments");
    if (!segments) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if ((unsigned)cJSON_GetArraySize(segments) > (unsigned)SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (!(flags & DHC_PARSE_FLAG_DRY_RUN)) {
      sl_zigbee_dhc_pa_curve_t curve = { .curve_min_ddbm = (int16_t)cmin, .curve_max_ddbm = (int16_t)cmax };
      // Initialize unused segments to zero
      for (uint8_t s = 0; s < SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT; s++) {
        curve.segments[s].maxPowerLevel = 0;
        curve.segments[s].slope = 0;
        curve.segments[s].intercept = 0;
      }
      for (int sj = 0; sj < cJSON_GetArraySize(segments); ++sj) {
        const cJSON *seg = cJSON_GetArrayItem(segments, sj);
        if (!cJSON_IsObject(seg)) {
          return SL_STATUS_INVALID_PARAMETER;
        }
        int seg_index = 0;
        int mpl = 0;
        int slope = 0;
        int intercept = 0;
        if (!json_get_int(seg, "segment_index", &seg_index)
            || !json_get_int(seg, "maxPowerLevel", &mpl)
            || !json_get_int(seg, "slope", &slope)
            || !json_get_int(seg, "intercept", &intercept)) {
          return SL_STATUS_INVALID_PARAMETER;
        }
        if (seg_index < 0 || seg_index >= (int)SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT) {
          return SL_STATUS_INVALID_PARAMETER;
        }
        curve.segments[seg_index].maxPowerLevel = (uint8_t)mpl;
        curve.segments[seg_index].slope = slope;
        curve.segments[seg_index].intercept = intercept;
      }
      sl_status_t st = sl_zigbee_dhc_write_pa_curve((uint8_t)index, &curve);
      if (st != SL_STATUS_OK) {
        return st;
      }
    }
  }
  return SL_STATUS_OK;
}

static sl_status_t apply_tables(const cJSON *parent, uint32_t flags)
{
  const cJSON *arr = json_get_array(parent, "tables");
  if (!arr) {
    return SL_STATUS_OK;
  }
  int n = cJSON_GetArraySize(arr);
  for (int i = 0; i < n; ++i) {
    const cJSON *obj = cJSON_GetArrayItem(arr, i);
    if (!cJSON_IsObject(obj)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    int index = 0;
    if (!json_get_int(obj, "index", &index)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    const cJSON *vals = json_get_array(obj, "ddbm_values");
    if (!vals) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    int vc = cJSON_GetArraySize(vals);
    if (vc != SL_ZIGBEE_DHC_TABLE_ENTRY_COUNT) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (!(flags & DHC_PARSE_FLAG_DRY_RUN)) {
      sl_zigbee_dhc_pa_table_t table = { 0 };
      for (int v = 0; v < vc; ++v) {
        const cJSON *valNode = cJSON_GetArrayItem(vals, v);
        if (!cJSON_IsNumber(valNode)) {
          return SL_STATUS_INVALID_PARAMETER;
        }
        int vi = valNode->valueint;
        if (vi < INT16_MIN || vi > INT16_MAX) {
          return SL_STATUS_INVALID_PARAMETER;
        }
        table.ddbm_values[v] = (int16_t)vi;
      }
      sl_status_t st = sl_zigbee_dhc_write_pa_table((uint8_t)index, &table);
      if (st != SL_STATUS_OK) {
        return st;
      }
    }
  }
  return SL_STATUS_OK;
}

static sl_status_t parse_legacy_document(const cJSON *silabs_dhc, uint32_t flags)
{
  // Top-level version (must match)
  int top_version = 0;
  if (!json_get_int(silabs_dhc, "version", &top_version)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (top_version != (int)SL_ZIGBEE_DHC_VERSION) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  const cJSON *pa_curves = cJSON_GetObjectItemCaseSensitive(silabs_dhc, "pa_curves");
  if (!cJSON_IsObject(pa_curves)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Inner (pa_curves) version (accept but ignore if matches)
  int inner_version = 0;
  if (!json_get_int(pa_curves, "version", &inner_version)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (inner_version != (int)SL_ZIGBEE_DHC_VERSION) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Metadata elements
  int num_desc_int = 0;
  int pa_voltage_int = 0;
  int signature_int = 0;
  if (!json_get_int(pa_curves, "num_descriptors", &num_desc_int)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (num_desc_int < 0 || num_desc_int > 255) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (num_desc_int > (int)SL_ZIGBEE_DHC_MAX_PA_DESCRIPTORS) {
    return SL_STATUS_INVALID_COUNT;
  }
  // Validate descriptor count and descriptor fields before writing anything to NCP.
  const cJSON *pa_descriptors = cJSON_GetObjectItemCaseSensitive(pa_curves, "pa_descriptors");
  if (!cJSON_IsArray(pa_descriptors)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (cJSON_GetArraySize(pa_descriptors) != num_desc_int) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  const cJSON *pa_curve_or_table = cJSON_GetObjectItemCaseSensitive(pa_curves, "pa_curve_or_table");
  if (!cJSON_IsArray(pa_curve_or_table)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (cJSON_GetArraySize(pa_curve_or_table) != num_desc_int) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  for (int i = 0; i < num_desc_int; ++i) {
    const cJSON *obj = cJSON_GetArrayItem(pa_descriptors, i);
    if (!cJSON_IsObject(obj)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    int algorithm = 0;
    if (!json_get_int(obj, "algorithm", &algorithm)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (algorithm != SL_ZIGBEE_DHC_ALGO_CURVE && algorithm != SL_ZIGBEE_DHC_ALGO_TABLE) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }

  if (cJSON_GetObjectItemCaseSensitive(pa_curves, "pa_voltage")) {
    if (!json_get_int(pa_curves, "pa_voltage", &pa_voltage_int)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }
  if (cJSON_GetObjectItemCaseSensitive(pa_curves, "signature")) {
    if (!json_get_int(pa_curves, "signature", &signature_int)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }

  // Write metadata first
  if (!(flags & DHC_PARSE_FLAG_DRY_RUN)) {
    sl_zigbee_dhc_pa_metadata_t md = { 0 };
    md.version = (uint8_t)inner_version;
    md.num_descriptors = (uint8_t)num_desc_int;
    if (pa_voltage_int < 0) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (pa_voltage_int > 0xFFFF) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    md.pa_voltage = (uint16_t)pa_voltage_int;
    md.signature = (uint32_t)signature_int;
    sl_status_t st_md = sl_zigbee_dhc_write_pa_metadata(&md);
    if (st_md != SL_STATUS_OK) {
      return st_md;
    }
  }

  for (int i = 0; i < num_desc_int; ++i) {
    const cJSON *obj = cJSON_GetArrayItem(pa_descriptors, i);
    if (!cJSON_IsObject(obj)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    int algorithm = 0;
    int nse = 0;
    int min_val = 0;
    int max_val = 0;
    if (!json_get_int(obj, "algorithm", &algorithm)
        || !json_get_int(obj, "num_segments_or_entries", &nse)
        || !json_get_int(obj, "min", &min_val)
        || !json_get_int(obj, "max", &max_val)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (algorithm != SL_ZIGBEE_DHC_ALGO_CURVE && algorithm != SL_ZIGBEE_DHC_ALGO_TABLE) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (nse <= 0 || nse > 255) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (!(flags & DHC_PARSE_FLAG_DRY_RUN)) {
      sl_zigbee_dhc_pa_descriptor_t d = { .algorithm = (uint8_t)algorithm,
                                          .num_segments_or_entries = (uint8_t)nse,
                                          .min_ddbm = (int16_t)min_val,
                                          .max_ddbm = (int16_t)max_val };
      sl_status_t st = sl_zigbee_dhc_write_pa_descriptor((uint8_t)i, &d);
      if (st != SL_STATUS_OK) {
        return st;
      }
    }
  }

  // Curves or tables (paired with descriptors by positional index).
  // pa_curve_or_table already validated above.
  for (int i = 0; i < num_desc_int; ++i) {
    const cJSON *obj = cJSON_GetArrayItem(pa_curve_or_table, i);
    if (!cJSON_IsObject(obj)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    // Use descriptor from JSON so validate (dry run) does not depend on NCP state
    const cJSON *desc_obj = cJSON_GetArrayItem(pa_descriptors, i);
    if (!cJSON_IsObject(desc_obj)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    int algorithm = 0;
    if (!json_get_int(desc_obj, "algorithm", &algorithm)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (algorithm != SL_ZIGBEE_DHC_ALGO_CURVE && algorithm != SL_ZIGBEE_DHC_ALGO_TABLE) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (algorithm == SL_ZIGBEE_DHC_ALGO_CURVE) {
      int cmin = 0;
      int cmax = 0;
      if (!json_get_int(obj, "curve_min_ddbm", &cmin) || !json_get_int(obj, "curve_max_ddbm", &cmax)) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      const cJSON *segments = cJSON_GetObjectItemCaseSensitive(obj, "curve_segments");
      if (!cJSON_IsArray(segments)) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      int scount = cJSON_GetArraySize(segments);
      if ((unsigned)scount > (unsigned)SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      if (!(flags & DHC_PARSE_FLAG_DRY_RUN)) {
        sl_zigbee_dhc_pa_curve_t curve = { .curve_min_ddbm = (int16_t)cmin, .curve_max_ddbm = (int16_t)cmax };
        // zero init segments
        for (uint8_t s = 0; s < SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT; s++) {
          curve.segments[s].maxPowerLevel = 0;
          curve.segments[s].slope = 0;
          curve.segments[s].intercept = 0;
        }
        for (int sj = 0; sj < scount; ++sj) {
          const cJSON *seg = cJSON_GetArrayItem(segments, sj);
          if (!cJSON_IsObject(seg)) {
            return SL_STATUS_INVALID_PARAMETER;
          }
          int mpl = 0, slope = 0, intercept = 0;
          if (!json_get_int(seg, "maxPowerLevel", &mpl)
              || !json_get_int(seg, "slope", &slope)
              || !json_get_int(seg, "intercept", &intercept)) {
            return SL_STATUS_INVALID_PARAMETER;
          }
          if (sj >= (int)SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT) {
            return SL_STATUS_INVALID_PARAMETER;
          }
          curve.segments[sj].maxPowerLevel = (uint8_t)mpl;
          curve.segments[sj].slope = slope;
          curve.segments[sj].intercept = intercept;
        }
        sl_status_t st_wc = sl_zigbee_dhc_write_pa_curve((uint8_t)i, &curve);
        if (st_wc != SL_STATUS_OK) {
          return st_wc;
        }
      }
    } else { // TABLE
      const cJSON *vals = cJSON_GetObjectItemCaseSensitive(obj, "ddbm_values");
      if (!cJSON_IsArray(vals)) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      int vc = cJSON_GetArraySize(vals);
      if (vc != SL_ZIGBEE_DHC_TABLE_ENTRY_COUNT) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      if (!(flags & DHC_PARSE_FLAG_DRY_RUN)) {
        sl_zigbee_dhc_pa_table_t table = { 0 };
        for (int v = 0; v < vc; ++v) {
          const cJSON *valNode = cJSON_GetArrayItem(vals, v);
          if (!cJSON_IsNumber(valNode)) {
            return SL_STATUS_INVALID_PARAMETER;
          }
          int vi = valNode->valueint;
          if (vi < INT16_MIN || vi > INT16_MAX) {
            return SL_STATUS_INVALID_PARAMETER;
          }
          table.ddbm_values[v] = (int16_t)vi;
        }
        sl_status_t st_wt = sl_zigbee_dhc_write_pa_table((uint8_t)i, &table);
        if (st_wt != SL_STATUS_OK) {
          return st_wt;
        }
      }
    }
  }

  // Scalars at top-level (not under "scalars" in legacy): rssi_offset, pa_mode, ctune
  if (!(flags & DHC_PARSE_FLAG_DRY_RUN)) {
    const cJSON *node;
    node = cJSON_GetObjectItemCaseSensitive(silabs_dhc, "rssi_offset");
    if (node) {
      if (!cJSON_IsNumber(node)) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      int v = node->valueint;
      if (v < INT8_MIN || v > INT8_MAX) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      sl_zigbee_dhc_rssi_offset_t r = { .rssi_offset = (int8_t)v };
      sl_status_t st = sl_zigbee_dhc_write_rssi_offset(&r);
      if (st != SL_STATUS_OK && st != SL_STATUS_NOT_AVAILABLE) {
        return st;
      }
    }
    node = cJSON_GetObjectItemCaseSensitive(silabs_dhc, "pa_mode");
    if (node) {
      if (!cJSON_IsNumber(node)) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      sl_zigbee_dhc_pa_mode_t m = { .pa_mode = (uint8_t)node->valueint };
      sl_status_t st = sl_zigbee_dhc_write_pa_mode(&m);
      if (st != SL_STATUS_OK) {
        return st;
      }
    }
    node = cJSON_GetObjectItemCaseSensitive(silabs_dhc, "ctune");
    if (node) {
      if (!cJSON_IsNumber(node)) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      int v = node->valueint;
      if (v < 0) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      sl_zigbee_dhc_ctune_t c = { .ctune = (uint32_t)v };
      sl_status_t st = sl_zigbee_dhc_write_ctune(&c);
      if (st != SL_STATUS_OK) {
        return st;
      }
    }
  }

  return SL_STATUS_OK;
}

static sl_status_t parse_new_document(const cJSON *silabs_dhc, uint32_t flags)
{
  int json_version = 0;
  if (!json_get_int(silabs_dhc, "version", &json_version)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (json_version != (int)SL_ZIGBEE_DHC_VERSION) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  uint8_t descriptor_count = 0;
  sl_status_t st = apply_descriptors(silabs_dhc, &descriptor_count, flags);
  if (st != SL_STATUS_OK) {
    return st;
  }
  st = apply_metadata(silabs_dhc, descriptor_count, flags);
  if (st != SL_STATUS_OK) {
    return st;
  }
  st = apply_curves(silabs_dhc, flags);
  if (st != SL_STATUS_OK) {
    return st;
  }
  st = apply_tables(silabs_dhc, flags);
  if (st != SL_STATUS_OK) {
    return st;
  }
  st = apply_scalars(silabs_dhc, flags);
  if (st != SL_STATUS_OK) {
    return st;
  }
  return SL_STATUS_OK;
}

// Auto-detect schema and parse
static sl_status_t parse_document(const cJSON *root, uint32_t flags)
{
  const cJSON *silabs_dhc = cJSON_GetObjectItemCaseSensitive(root, "silabs_dhc");
  if (!cJSON_IsObject(silabs_dhc)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  const cJSON *legacy_indicator = cJSON_GetObjectItemCaseSensitive(silabs_dhc, "pa_curves");
  if (cJSON_IsObject(legacy_indicator)) {
    return parse_legacy_document(silabs_dhc, flags);
  } else {
    return parse_new_document(silabs_dhc, flags);
  }
}

sl_status_t sl_zigbee_af_dhc_parse_file(const char *path, uint32_t flags)
{
  if (path == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  FILE *fp = fopen(path, "rb");
  if (!fp) {
    return SL_STATUS_IO;
  }

  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    return SL_STATUS_IO;
  }
  long len = ftell(fp);
  if (len < 0) {
    fclose(fp);
    return SL_STATUS_IO;
  }
  if (fseek(fp, 0, SEEK_SET) != 0) {
    fclose(fp);
    return SL_STATUS_IO;
  }

  char *buf = (char *)malloc((size_t)len + 1);
  if (!buf) {
    fclose(fp);
    return SL_STATUS_ALLOCATION_FAILED;
  }
  size_t rd = fread(buf, 1, (size_t)len, fp);
  fclose(fp);
  if (rd != (size_t)len) {
    free(buf);
    return SL_STATUS_IO;
  }
  buf[len] = '\0';

  sl_status_t status = sl_zigbee_af_dhc_parse_json(buf, flags);
  free(buf);
  return status;
}

sl_status_t sl_zigbee_af_dhc_parse_json(const char *json_text, uint32_t flags)
{
  if (json_text == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  cJSON *root = cJSON_Parse(json_text); // direct parse
  if (!root) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sl_status_t status = SL_STATUS_OK;

  status = parse_document(root, flags);
  cJSON_Delete(root);
  return status;
}
