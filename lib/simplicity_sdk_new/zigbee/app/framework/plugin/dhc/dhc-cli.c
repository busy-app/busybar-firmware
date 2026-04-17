/***************************************************************************//**
 * @file
 * @brief Zigbee Dynamic Hardware Configuration (DHC) CLI API.
 ******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>

#include "app/framework/util/common.h"
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "sl_cli_types.h"
#include "sl_status.h"
#include "sl_zigbee_dhc.h"
#include "dhc-parser.h"

#ifdef EZSP_HOST
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp.h"
#include "app/util/ezsp/command-prototypes.h"
#include "app/util/ezsp/rename-ezsp-functions.h"
#endif

// ------------------------- usage / helpers ---------------------------------
static void dhc_cli_usage(void)
{
  sl_zigbee_af_core_println("DHC CLI:");
  sl_zigbee_af_core_println("  dhc apply file <json>");
  sl_zigbee_af_core_println("  dhc validate file <json>");
  sl_zigbee_af_core_println("  dhc export");
  sl_zigbee_af_core_println("  dhc read-metadata");
  sl_zigbee_af_core_println("  dhc read-scalars");
  sl_zigbee_af_core_println("  dhc read-versions");
  sl_zigbee_af_core_println("  dhc read-all");
  sl_zigbee_af_core_println("  dhc read-descriptor <i>");
  sl_zigbee_af_core_println("  dhc read-curve <i>");
  sl_zigbee_af_core_println("  dhc read-table <i>");
  sl_zigbee_af_core_println("  dhc read-segment <i> <seg>");
  sl_zigbee_af_core_println("  dhc set-descriptor <i> <algorithm> <count> <min_ddbm> <max_ddbm>");
  sl_zigbee_af_core_println("  dhc set-segment <i> <seg> <mpl> <slope> <intercept>");
  sl_zigbee_af_core_println("  dhc set-table <i> <entry_index> <ddbm>");
  sl_zigbee_af_core_println("  dhc set-scalar rssi_offset|pa_mode|ctune <val>");
  sl_zigbee_af_core_println("  dhc set-voltage <mv>");
  sl_zigbee_af_core_println("  dhc set-signature <uint32/hex>");
  sl_zigbee_af_core_println("  dhc set-dhc-version <v>");
  sl_zigbee_af_core_println("  dhc recompute signature");
  sl_zigbee_af_core_println("  dhc raw <bytes...>    (binary frame: 0xDC <cmd> <setting> [values...])");
}

static void print_status(const char *label, sl_status_t st)
{
  sl_zigbee_af_core_println("%s -> 0x%08lX", label, (unsigned long)st);
}

static void print_metadata(void)
{
  sl_zigbee_dhc_pa_metadata_t md;
  sl_status_t st = sl_zigbee_dhc_read_pa_metadata(&md);
  if (st == SL_STATUS_NOT_FOUND) {
    sl_zigbee_af_core_println("DHC: not set (no configuration applied)");
    return;
  }
  if (st != SL_STATUS_OK) {
    sl_zigbee_af_core_println("metadata: unset");
    return;
  }
  sl_zigbee_af_core_println("metadata: status=0x%08lX version=%u num_desc=%u pa_voltage=%u signature=0x%08lX",
                            (unsigned long)st,
                            md.version,
                            md.num_descriptors,
                            md.pa_voltage,
                            (unsigned long)md.signature);
}

static void print_descriptor(uint8_t idx)
{
  sl_zigbee_dhc_pa_descriptor_t d;
  sl_status_t st = sl_zigbee_dhc_read_pa_descriptor(idx, &d);
  if (st != SL_STATUS_OK) {
    sl_zigbee_af_core_println("descriptor[%u]: status=0x%08lX (error)", idx, (unsigned long)st);
    return;
  }
  sl_zigbee_af_core_println("descriptor[%u]: status=0x%08lX algo=%u n=%u min_ddbm=%d max_ddbm=%d",
                            idx,
                            (unsigned long)st,
                            d.algorithm,
                            d.num_segments_or_entries,
                            d.min_ddbm, d.max_ddbm);
}

static void print_curve(uint8_t idx)
{
  sl_zigbee_dhc_pa_curve_t cv;
  sl_status_t st = sl_zigbee_dhc_read_pa_curve(idx, &cv);
  if (st != SL_STATUS_OK) {
    sl_zigbee_af_core_println("curve[%u]: status=0x%08lX (error)", idx, (unsigned long)st);
    return;
  }
  sl_zigbee_af_core_println("curve[%u]: status=0x%08lX min=%d max=%d",
                            idx, (unsigned long)st,
                            cv.curve_min_ddbm,
                            cv.curve_max_ddbm);
  for (uint8_t i = 0; i < SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT; i++) {
    sl_zigbee_af_core_println("  seg[%u]: mpl=%u slope=%ld intercept=%ld",
                              i,
                              cv.segments[i].maxPowerLevel,
                              (long)cv.segments[i].slope,
                              (long)cv.segments[i].intercept);
  }
}

static void print_table(uint8_t idx)
{
  sl_zigbee_dhc_pa_table_t t;
  sl_status_t st = sl_zigbee_dhc_read_pa_table(idx, &t);
  if (st != SL_STATUS_OK) {
    sl_zigbee_af_core_println("table[%u]: status=0x%08lX (error)", idx, (unsigned long)st);
    return;
  }
  sl_zigbee_af_core_print("table[%u]: status=0x%08lX", idx, (unsigned long)st);
  for (uint8_t i = 0; i < SL_ZIGBEE_DHC_TABLE_ENTRY_COUNT; i++) {
    sl_zigbee_af_core_print(" %d", t.ddbm_values[i]);
  }
  sl_zigbee_af_core_println("");
}

static void print_segment(uint8_t pa_idx, uint8_t seg_idx)
{
  sl_zigbee_dhc_pa_curve_segment_t seg;
  sl_status_t st = sl_zigbee_dhc_read_pa_curve_segment(pa_idx, seg_idx, &seg);
  if (st != SL_STATUS_OK) {
    sl_zigbee_af_core_println("segment[%u][%u]: status=0x%08lX (error)", pa_idx, seg_idx, (unsigned long)st);
    return;
  }
  sl_zigbee_af_core_println("segment[%u][%u]: status=0x%08lX mpl=%u slope=%ld intercept=%ld",
                            pa_idx,
                            seg_idx,
                            (unsigned long)st,
                            seg.maxPowerLevel,
                            (long)seg.slope,
                            (long)seg.intercept);
}

static void print_scalars(void)
{
  sl_zigbee_dhc_rssi_offset_t r;
  sl_zigbee_dhc_read_rssi_offset(&r);
  sl_zigbee_dhc_pa_mode_t m;
  sl_zigbee_dhc_read_pa_mode(&m);
  sl_zigbee_dhc_ctune_t c;
  sl_zigbee_dhc_read_ctune(&c);
  uint8_t dhc_v = 0;
  sl_zigbee_dhc_read_dhc_version(&dhc_v);

  sl_zigbee_dhc_pa_metadata_t md;
  sl_status_t st_md = sl_zigbee_dhc_read_pa_metadata(&md);
  if (st_md != SL_STATUS_OK) {
    sl_zigbee_af_core_println("scalars: rssi_offset=%d pa_mode=%u ctune=%lu dhc_version=%u pa_voltage=unset pa_signature=unset pa_version=unset",
                              r.rssi_offset, m.pa_mode, (unsigned long)c.ctune, dhc_v);
    return;
  }
  uint16_t mv = 0;
  sl_zigbee_dhc_read_pa_voltage(&mv);
  sl_zigbee_dhc_pa_signature_t sig;
  sl_zigbee_dhc_read_pa_signature(&sig);
  sl_zigbee_dhc_pa_version_t pav;
  sl_zigbee_dhc_read_pa_version(&pav);
  sl_zigbee_af_core_println("scalars: rssi_offset=%d pa_mode=%u ctune=%lu pa_voltage=%u pa_signature=0x%08lX pa_version=%u dhc_version=%u",
                            r.rssi_offset, m.pa_mode, (unsigned long)c.ctune, mv, (unsigned long)sig.pa_signature, pav.pa_version, dhc_v);
}

static void print_versions(void)
{
  sl_zigbee_dhc_pa_version_t pav;
  sl_status_t st = sl_zigbee_dhc_read_pa_version(&pav);
  if (st != SL_STATUS_OK) {
    sl_zigbee_af_core_println("versions: status=0x%08lX (error)", (unsigned long)st);
    return;
  }
  uint8_t dhc_v = 0;
  sl_zigbee_dhc_read_dhc_version(&dhc_v);
  sl_zigbee_af_core_println("versions: pa_version=%u dhc_version=%u", pav.pa_version, dhc_v);
}

// Raw frame helpers:
// Binary framing:
//   Byte0: 0xDC header
//   Byte1: command   (0x00 read, 0x01 write)
//   Byte2: setting id
//   Bytes3+: value payload (write only)
// Response (console printed hex):
//   DC <status8> <setting> [value bytes]

enum {
  DHC_FRAME_HEADER = 0xDC,
  DHC_FRAME_CMD_READ = 0x00,
  DHC_FRAME_CMD_WRITE = 0x01
};

// Minimal setting map for Zigbee DHC (extend as needed)
typedef enum {
  ZDHC_SET_VERSION       = 0x00, // metadata.version
  ZDHC_SET_RSSI_OFFSET   = 0x01,
  ZDHC_SET_PA_MODE       = 0x02,
  ZDHC_SET_CTUNE         = 0x03,
  ZDHC_SET_VOLTAGE       = 0x04,
  ZDHC_SET_SIGNATURE     = 0x05,
  ZDHC_SET_METADATA      = 0x06, // full metadata (version,num_desc,voltage,signature)
  ZDHC_SET_DHC_VERSION   = 0x07, // protocol/top-level
  // Future: descriptors / curves / tables could be chunked or indexed settings
} zdhc_setting_t;

static uint8_t hex_to_byte(const char *s)
{
  unsigned v = 0;
  if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
    s += 2;
  }
  for (int i = 0; s[i] && i < 2; i++) {
    char c = s[i];
    uint8_t nib;
    if (c >= '0' && c <= '9') {
      nib = (uint8_t)(c - '0');
    } else if (c >= 'a' && c <= 'f') {
      nib = (uint8_t)(c - 'a' + 10);
    } else if (c >= 'A' && c <= 'F') {
      nib = (uint8_t)(c - 'A' + 10);
    } else {
      break;
    }
    v = (v << 4) | nib;
  }
  return (uint8_t)v;
}

static void cli_print_hex_response(const uint8_t *buf, size_t len)
{
  sl_zigbee_af_core_print("RESP:");
  for (size_t i = 0; i < len; i++) {
    sl_zigbee_af_core_print(" %02X", buf[i]);
  }
  sl_zigbee_af_core_println("");
}

static void cli_print_interpreted_response(const uint8_t *buf, size_t len, uint8_t setting)
{
  if (len < 3) {
    return; // Not enough bytes for a valid response
  }
  
  uint8_t status = buf[1];
  const char *status_str = (status == 0) ? "OK" : "ERROR";
  
  if (status != 0) {
    sl_zigbee_af_core_println("Status: 0x%02X (%s)", status, status_str);
    return;
  }
  
  // Interpret based on setting ID
  switch (setting) {
    case ZDHC_SET_VERSION:
      if (len >= 4) {
        sl_zigbee_af_core_println("Version: %u", buf[3]);
      }
      break;
    case ZDHC_SET_RSSI_OFFSET:
      if (len >= 4) {
        int8_t val = (int8_t)buf[3];
        sl_zigbee_af_core_println("RSSI Offset: %d", val);
      }
      break;
    case ZDHC_SET_PA_MODE:
      if (len >= 4) {
        sl_zigbee_af_core_println("PA Mode: %u", buf[3]);
      }
      break;
    case ZDHC_SET_CTUNE:
      if (len >= 7) {
        uint32_t val = (uint32_t)buf[3]
                      | ((uint32_t)buf[4] << 8)
                      | ((uint32_t)buf[5] << 16)
                      | ((uint32_t)buf[6] << 24);
        sl_zigbee_af_core_println("CTUNE: %lu (0x%08lX)", (unsigned long)val, (unsigned long)val);
      }
      break;
    case ZDHC_SET_VOLTAGE:
      if (len >= 5) {
        uint16_t val = (uint16_t)buf[3] | ((uint16_t)buf[4] << 8);
        sl_zigbee_af_core_println("Voltage: %u mV", val);
      }
      break;
    case ZDHC_SET_SIGNATURE:
      if (len >= 7) {
        uint32_t val = (uint32_t)buf[3]
                      | ((uint32_t)buf[4] << 8)
                      | ((uint32_t)buf[5] << 16)
                      | ((uint32_t)buf[6] << 24);
        sl_zigbee_af_core_println("Signature: 0x%08lX", (unsigned long)val);
      }
      break;
    case ZDHC_SET_METADATA:
      if (len >= 11) {
        sl_zigbee_af_core_println("Metadata: version=%u num_desc=%u voltage=%u mV signature=0x%08lX",
                                  buf[3],
                                  buf[4],
                                  (uint16_t)buf[5] | ((uint16_t)buf[6] << 8),
                                  (unsigned long)((uint32_t)buf[7]
                                                 | ((uint32_t)buf[8] << 8)
                                                 | ((uint32_t)buf[9] << 16)
                                                 | ((uint32_t)buf[10] << 24)));
      }
      break;
    case ZDHC_SET_DHC_VERSION:
      if (len >= 4) {
        sl_zigbee_af_core_println("DHC Version: %u", buf[3]);
      }
      break;
    default:
      // Unknown setting, just show status
      sl_zigbee_af_core_println("Status: 0x%02X (%s)", status, status_str);
      break;
  }
}
// Help handler for CLI 'help' command
void sl_zigbee_af_dhc_cli_help_handler(sl_cli_command_arg_t *args)
{
  (void)args;
  dhc_cli_usage();
}
// Setter functions for CLI 'set' commands
static sl_status_t set_descriptor(uint8_t idx, int algo, int count, int min_ddbm, int max_ddbm)
{
  if (algo < 0 || algo > 1) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (count <= 0 || count > 255) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sl_zigbee_dhc_pa_descriptor_t d = { .algorithm = (uint8_t)algo,
                                      .num_segments_or_entries = (uint8_t)count,
                                      .min_ddbm = (int16_t)min_ddbm,
                                      .max_ddbm = (int16_t)max_ddbm };
  return sl_zigbee_dhc_write_pa_descriptor(idx, &d);
}

static sl_status_t set_segment(uint8_t pa_idx, uint8_t seg_idx, int mpl, int slope, int intercept)
{
  if (seg_idx >= SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT) {
    return SL_STATUS_INVALID_INDEX;
  }
  if (mpl < 0 || mpl > 255) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sl_zigbee_dhc_pa_curve_segment_t seg = { .maxPowerLevel = (uint8_t)mpl, .slope = slope, .intercept = intercept };
  return sl_zigbee_dhc_write_pa_curve_segment(pa_idx, seg_idx, &seg);
}

static sl_status_t set_table_entry(uint8_t pa_idx, uint8_t entry_idx, int ddbm)
{
  if (entry_idx >= SL_ZIGBEE_DHC_TABLE_ENTRY_COUNT) {
    return SL_STATUS_INVALID_INDEX;
  }
  if (ddbm < INT16_MIN || ddbm > INT16_MAX) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sl_zigbee_dhc_pa_table_t tbl;
  sl_status_t st = sl_zigbee_dhc_read_pa_table(pa_idx, &tbl);
  if (st != SL_STATUS_OK) {
    return st;
  }
  tbl.ddbm_values[entry_idx] = (int16_t)ddbm;
  return sl_zigbee_dhc_write_pa_table(pa_idx, &tbl);
}

static sl_status_t recompute_signature(void)
{
  sl_zigbee_dhc_pa_metadata_t md;
  sl_status_t st = sl_zigbee_dhc_read_pa_metadata(&md);
  if (st != SL_STATUS_OK) {
    return st;
  }
  uint32_t sig = 0;
  for (uint8_t i = 0; i < md.num_descriptors; i++) {
    sl_zigbee_dhc_pa_descriptor_t d;
    if (sl_zigbee_dhc_read_pa_descriptor(i, &d) != SL_STATUS_OK) {
      continue;
    }
    sig += d.algorithm + d.num_segments_or_entries + (uint16_t)d.min_ddbm + (uint16_t)d.max_ddbm;
    if (d.algorithm == SL_ZIGBEE_DHC_ALGO_CURVE) {
      sl_zigbee_dhc_pa_curve_t c;
      if (sl_zigbee_dhc_read_pa_curve(i, &c) == SL_STATUS_OK) {
        sig += (uint16_t)c.curve_min_ddbm + (uint16_t)c.curve_max_ddbm;
        for (uint8_t s = 0; s < SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT; s++) {
          sig += c.segments[s].maxPowerLevel;
        }
      }
    } else {
      sl_zigbee_dhc_pa_table_t t;
      if (sl_zigbee_dhc_read_pa_table(i, &t) == SL_STATUS_OK) {
        for (uint8_t e = 0; e < SL_ZIGBEE_DHC_TABLE_ENTRY_COUNT; e++) {
          sig += (uint16_t)t.ddbm_values[e];
        }
      }
    }
  }
  sl_zigbee_dhc_pa_metadata_t new_md = md;
  new_md.signature = sig;
  return sl_zigbee_dhc_write_pa_metadata(&new_md);
}

// Main dispatch function for CLI commands
void sl_zigbee_af_dhc_cli_apply(sl_cli_command_arg_t *args)
{
  uint8_t argc = sl_cli_get_argument_count(args);
  if (argc != 2) {
    sl_zigbee_af_core_println("Usage: dhc apply file <json>");
    return;
  }
  const char *kw = sl_cli_get_argument_string(args, 0);  // "file"
  const char *path = sl_cli_get_argument_string(args, 1);  // path
  if (strcmp(kw, "file") != 0) {
    sl_zigbee_af_core_println("Usage: dhc apply file <json>");
    return;
  }
  uint32_t flags = DHC_PARSE_FLAG_STOP_ON_ERROR;
  sl_status_t st = sl_zigbee_af_dhc_parse_file(path, flags);
  sl_zigbee_af_core_println("apply %s -> 0x%08lX", path, (unsigned long)st);
}

void sl_zigbee_af_dhc_cli_validate(sl_cli_command_arg_t *args)
{
  uint8_t argc = sl_cli_get_argument_count(args);
  if (argc != 2) {
    sl_zigbee_af_core_println("Usage: dhc validate file <json>");
    return;
  }
  const char *kw = sl_cli_get_argument_string(args, 0);  // "file"
  const char *path = sl_cli_get_argument_string(args, 1);  // path
  if (strcmp(kw, "file") != 0) {
    sl_zigbee_af_core_println("Usage: dhc validate file <json>");
    return;
  }
  uint32_t flags = DHC_PARSE_FLAG_STOP_ON_ERROR | DHC_PARSE_FLAG_DRY_RUN;
  sl_status_t st = sl_zigbee_af_dhc_parse_file(path, flags);
  sl_zigbee_af_core_println("validate %s -> 0x%08lX", path, (unsigned long)st);
}

void sl_zigbee_af_dhc_cli_export(sl_cli_command_arg_t *args)
{
  (void)args;  // export takes no arguments
  sl_zigbee_dhc_pa_metadata_t md;
  sl_zigbee_dhc_read_pa_metadata(&md);
  sl_zigbee_af_core_println("{\"silabs_dhc\":{\"version\":%u,\"metadata\":{\"num_descriptors\":%u,\"pa_voltage\":%u,\"signature\":%lu}}}",
                            SL_ZIGBEE_DHC_VERSION, md.num_descriptors, md.pa_voltage, (unsigned long)md.signature);
}

// Read command handlers
void sl_zigbee_af_dhc_cli_read_metadata(sl_cli_command_arg_t *args)
{
  (void)args;
  print_metadata();
}

void sl_zigbee_af_dhc_cli_read_scalars(sl_cli_command_arg_t *args)
{
  (void)args;
  print_scalars();
}

void sl_zigbee_af_dhc_cli_read_versions(sl_cli_command_arg_t *args)
{
  (void)args;
  print_versions();
}

void sl_zigbee_af_dhc_cli_read_all(sl_cli_command_arg_t *args)
{
  (void)args;
  sl_zigbee_dhc_pa_metadata_t md;
  sl_status_t st = sl_zigbee_dhc_read_pa_metadata(&md);
  if (st == SL_STATUS_NOT_FOUND) {
    sl_zigbee_af_core_println("DHC: not set (no configuration applied)");
    print_scalars();
    return;
  }
  if (st != SL_STATUS_OK) {
    sl_zigbee_af_core_println("metadata: unset");
    print_scalars();
    return;
  }
  sl_zigbee_af_core_println("metadata: status=0x%08lX version=%u num_desc=%u pa_voltage=%u signature=0x%08lX",
                            (unsigned long)st,
                            md.version,
                            md.num_descriptors,
                            md.pa_voltage,
                            (unsigned long)md.signature);
  for (uint8_t i = 0; i < md.num_descriptors; i++) {
    print_descriptor(i);
  }
  for (uint8_t i = 0; i < md.num_descriptors; i++) {
    sl_zigbee_dhc_pa_descriptor_t d;
    if (sl_zigbee_dhc_read_pa_descriptor(i, &d) != SL_STATUS_OK) {
      continue;
    }
    if (d.algorithm == SL_ZIGBEE_DHC_ALGO_CURVE) {
      print_curve(i);
    } else {
      print_table(i);
    }
  }
  print_scalars();
}

void sl_zigbee_af_dhc_cli_read_descriptor(sl_cli_command_arg_t *args)
{
  uint8_t idx = sl_cli_get_argument_uint8(args, 0);
  print_descriptor(idx);
}

void sl_zigbee_af_dhc_cli_read_curve(sl_cli_command_arg_t *args)
{
  uint8_t idx = sl_cli_get_argument_uint8(args, 0);
  print_curve(idx);
}

void sl_zigbee_af_dhc_cli_read_table(sl_cli_command_arg_t *args)
{
  uint8_t idx = sl_cli_get_argument_uint8(args, 0);
  print_table(idx);
}

void sl_zigbee_af_dhc_cli_read_segment(sl_cli_command_arg_t *args)
{
  uint8_t pa_idx = sl_cli_get_argument_uint8(args, 0);
  uint8_t seg_idx = sl_cli_get_argument_uint8(args, 1);
  print_segment(pa_idx, seg_idx);
}

// Set command handlers
void sl_zigbee_af_dhc_cli_set_descriptor(sl_cli_command_arg_t *args)
{
  uint8_t idx = sl_cli_get_argument_uint8(args, 0);
  uint8_t algo = sl_cli_get_argument_uint8(args, 1);
  uint8_t n = sl_cli_get_argument_uint8(args, 2);
  int16_t minv = sl_cli_get_argument_int16(args, 3);
  int16_t maxv = sl_cli_get_argument_int16(args, 4);
  sl_status_t st = set_descriptor(idx, algo, n, minv, maxv);
  print_status("set-descriptor", st);
}

void sl_zigbee_af_dhc_cli_set_segment(sl_cli_command_arg_t *args)
{
  uint8_t pa_idx = sl_cli_get_argument_uint8(args, 0);
  uint8_t seg_idx = sl_cli_get_argument_uint8(args, 1);
  uint8_t mpl = sl_cli_get_argument_uint8(args, 2);
  int16_t slope = sl_cli_get_argument_int16(args, 3);
  int16_t intercept = sl_cli_get_argument_int16(args, 4);
  sl_status_t st = set_segment(pa_idx, seg_idx, mpl, slope, intercept);
  print_status("set-segment", st);
}

void sl_zigbee_af_dhc_cli_set_table(sl_cli_command_arg_t *args)
{
  uint8_t pa_idx = sl_cli_get_argument_uint8(args, 0);
  uint8_t entry_index = sl_cli_get_argument_uint8(args, 1);
  int16_t ddbm = sl_cli_get_argument_int16(args, 2);
  sl_status_t st = set_table_entry(pa_idx, entry_index, ddbm);
  print_status("set-table", st);
}

void sl_zigbee_af_dhc_cli_set_scalar(sl_cli_command_arg_t *args)
{
  const char *which = sl_cli_get_argument_string(args, 0);
  int32_t val = sl_cli_get_argument_int32(args, 1);
  sl_status_t st = SL_STATUS_OK;
  if (strcmp(which, "rssi_offset") == 0) {
    sl_zigbee_dhc_rssi_offset_t r = { .rssi_offset = (int8_t)val };
    st = sl_zigbee_dhc_write_rssi_offset(&r);
  } else if (strcmp(which, "pa_mode") == 0) {
    sl_zigbee_dhc_pa_mode_t m = { .pa_mode = (uint8_t)val };
    st = sl_zigbee_dhc_write_pa_mode(&m);
  } else if (strcmp(which, "ctune") == 0) {
    sl_zigbee_dhc_ctune_t c = { .ctune = (uint32_t)val };
    st = sl_zigbee_dhc_write_ctune(&c);
  } else {
    sl_zigbee_af_core_println("Usage: dhc set-scalar rssi_offset|pa_mode|ctune <val>");
    return;
  }
  print_status("set-scalar", st);
}

void sl_zigbee_af_dhc_cli_set_voltage(sl_cli_command_arg_t *args)
{
  uint16_t mv = sl_cli_get_argument_uint16(args, 0);
  sl_status_t st = sl_zigbee_dhc_write_pa_voltage(mv);
  print_status("set-voltage", st);
}

void sl_zigbee_af_dhc_cli_set_signature(sl_cli_command_arg_t *args)
{
  uint32_t sig = sl_cli_get_argument_uint32(args, 0);
  sl_zigbee_dhc_pa_metadata_t md;
  sl_status_t st = sl_zigbee_dhc_read_pa_metadata(&md);
  if (st != SL_STATUS_OK) {
    print_status("set-signature", st);
    return;
  }
  md.signature = sig;
  st = sl_zigbee_dhc_write_pa_metadata(&md);
  print_status("set-signature", st);
}

void sl_zigbee_af_dhc_cli_set_dhc_version(sl_cli_command_arg_t *args)
{
  uint8_t v = sl_cli_get_argument_uint8(args, 0);
  sl_status_t st = sl_zigbee_dhc_write_dhc_version(v);
  print_status("set-dhc-version", st);
}

void sl_zigbee_af_dhc_cli_recompute(sl_cli_command_arg_t *args)
{
  uint8_t argc = sl_cli_get_argument_count(args);
  if (argc != 1) {
    sl_zigbee_af_core_println("Usage: dhc recompute signature");
    return;
  }
  const char *kw = sl_cli_get_argument_string(args, 0);
  if (strcmp(kw, "signature") == 0) {
    sl_status_t st = recompute_signature();
    print_status("recompute signature", st);
    return;
  }
  sl_zigbee_af_core_println("Usage: dhc recompute signature");
}

// Helper to parse a single hex byte from a string (handles "0xXX", "XX", or "xx")
static uint8_t parse_single_hex_byte(const char *str, const char **next)
{
  const char *p = str;
  // Skip whitespace
  while (*p == ' ' || *p == '\t') {
    p++;
  }
  if (*p == '\0') {
    if (next) *next = p;
    return 0;
  }
  // Parse the hex byte
  uint8_t val = hex_to_byte(p);
  // Advance past the hex digits
  if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    p += 2;
  }
  while ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F')) {
    p++;
  }
  if (next) *next = p;
  return val;
}

void sl_zigbee_af_dhc_cli_raw(sl_cli_command_arg_t *args)
{
  uint8_t argc = sl_cli_get_argument_count(args);
  if (argc < 1) {
    sl_zigbee_af_core_println("Usage: dhc raw <bytes...>");
    sl_zigbee_af_core_println("Example: dhc raw DC 01 03 57 00 00 00");
    sl_zigbee_af_core_println("   or:   dhc raw 0xDC 0x01 0x03 0x57 0x00 0x00 0x00");
    return;
  }
  // Build frame from arguments
  // Try two approaches:
  // 1. Multiple separate arguments (argc > 1)
  // 2. Single argument with space-separated hex bytes (argc == 1)
  uint8_t frame[64];
  uint8_t frame_len = 0;
  
  if (argc == 1) {
    // Single argument - parse as space-separated hex bytes
    const char *arg_str = sl_cli_get_argument_string(args, 0);
    if (arg_str == NULL || arg_str[0] == '\0') {
      sl_zigbee_af_core_println("ERR: empty argument");
      return;
    }
    const char *p = arg_str;
    while (*p != '\0' && frame_len < sizeof(frame)) {
      uint8_t byte_val = parse_single_hex_byte(p, &p);
      frame[frame_len++] = byte_val;
      // Skip whitespace
      while (*p == ' ' || *p == '\t') {
        p++;
      }
    }
  } else {
    // Multiple arguments - each is a hex byte
    for (uint8_t i = 0; i < argc && frame_len < sizeof(frame); i++) {
      const char *arg_str = sl_cli_get_argument_string(args, i);
      if (arg_str == NULL || arg_str[0] == '\0') {
        sl_zigbee_af_core_println("ERR: empty argument at index %d", i);
        return;
      }
      uint8_t byte_val = hex_to_byte(arg_str);
      frame[frame_len++] = byte_val;
    }
  }
  if (frame_len < 3) {
    sl_zigbee_af_core_println("ERR: frame too short (need at least 3 bytes: header, cmd, setting)");
    return;
  }
  if (frame[0] != DHC_FRAME_HEADER) {
    sl_zigbee_af_core_println("ERR: bad header (got 0x%02X, expected 0x%02X)", frame[0], DHC_FRAME_HEADER);
    return;
  }
  uint8_t command = frame[1];
  uint8_t setting = frame[2];
  uint8_t response[32];
  size_t rsp_len = 0;
  response[rsp_len++] = DHC_FRAME_HEADER;
  response[rsp_len++] = 0xFF; // placeholder for status
  response[rsp_len++] = setting;
  sl_status_t st = SL_STATUS_OK;

  if (command == DHC_FRAME_CMD_READ) {
    switch (setting) {
      case ZDHC_SET_VERSION: {
        sl_zigbee_dhc_pa_metadata_t md;
        st = sl_zigbee_dhc_read_pa_metadata(&md);
        response[rsp_len++] = md.version;
        break;
      }
      case ZDHC_SET_RSSI_OFFSET: {
        sl_zigbee_dhc_rssi_offset_t r;
        st = sl_zigbee_dhc_read_rssi_offset(&r);
        if (st == SL_STATUS_OK) {
          response[rsp_len++] = (uint8_t)r.rssi_offset;
        }
        break;
      }
      case ZDHC_SET_PA_MODE: {
        sl_zigbee_dhc_pa_mode_t m;
        st = sl_zigbee_dhc_read_pa_mode(&m);
        if (st == SL_STATUS_OK) {
          response[rsp_len++] = m.pa_mode;
        }
        break;
      }
      case ZDHC_SET_CTUNE: {
        sl_zigbee_dhc_ctune_t c;
        st = sl_zigbee_dhc_read_ctune(&c);
        if (st == SL_STATUS_OK) {
          response[rsp_len++] = (uint8_t)(c.ctune & 0xFF);
          response[rsp_len++] = (uint8_t)((c.ctune >> 8) & 0xFF);
          response[rsp_len++] = (uint8_t)((c.ctune >> 16) & 0xFF);
          response[rsp_len++] = (uint8_t)((c.ctune >> 24) & 0xFF);
        }
        break;
      }
      case ZDHC_SET_VOLTAGE: {
        uint16_t v = 0;
        st = sl_zigbee_dhc_read_pa_voltage(&v);
        if (st == SL_STATUS_OK) {
          response[rsp_len++] = (uint8_t)(v & 0xFF);
          response[rsp_len++] = (uint8_t)(v >> 8);
        }
        break;
      }
      case ZDHC_SET_SIGNATURE: {
        sl_zigbee_dhc_pa_metadata_t md;
        st = sl_zigbee_dhc_read_pa_metadata(&md);
        if (st == SL_STATUS_OK) {
          response[rsp_len++] = (uint8_t)(md.signature & 0xFF);
          response[rsp_len++] = (uint8_t)((md.signature >> 8) & 0xFF);
          response[rsp_len++] = (uint8_t)((md.signature >> 16) & 0xFF);
          response[rsp_len++] = (uint8_t)((md.signature >> 24) & 0xFF);
        }
        break;
      }
      case ZDHC_SET_METADATA: {
        sl_zigbee_dhc_pa_metadata_t md;
        st = sl_zigbee_dhc_read_pa_metadata(&md);
        if (st == SL_STATUS_OK) {
          response[rsp_len++] = md.version;
          response[rsp_len++] = md.num_descriptors;
          response[rsp_len++] = (uint8_t)(md.pa_voltage & 0xFF);
          response[rsp_len++] = (uint8_t)(md.pa_voltage >> 8);
          response[rsp_len++] = (uint8_t)(md.signature & 0xFF);
          response[rsp_len++] = (uint8_t)((md.signature >> 8) & 0xFF);
          response[rsp_len++] = (uint8_t)((md.signature >> 16) & 0xFF);
          response[rsp_len++] = (uint8_t)((md.signature >> 24) & 0xFF);
        }
        break;
      }
      case ZDHC_SET_DHC_VERSION: {
        uint8_t ver = 0;
        st = sl_zigbee_dhc_read_dhc_version(&ver);
        if (st == SL_STATUS_OK) {
          response[rsp_len++] = ver;
        }
        break;
      }
      default:
        st = SL_STATUS_INVALID_PARAMETER;
        break;
    }
  } else if (command == DHC_FRAME_CMD_WRITE) {
    const uint8_t *payload = frame + 3;
    size_t plen = (frame_len > 3) ? (size_t)(frame_len - 3) : 0;
    switch (setting) {
      case ZDHC_SET_RSSI_OFFSET:
        if (plen == 1) {
          sl_zigbee_dhc_rssi_offset_t r = { .rssi_offset = (int8_t)payload[0] };
          st = sl_zigbee_dhc_write_rssi_offset(&r);
        } else {
          st = SL_STATUS_INVALID_COUNT;
        }
        break;
      case ZDHC_SET_PA_MODE:
        if (plen == 1) {
          sl_zigbee_dhc_pa_mode_t m = { .pa_mode = payload[0] };
          st = sl_zigbee_dhc_write_pa_mode(&m);
        } else {
          st = SL_STATUS_INVALID_COUNT;
        }
        break;
      case ZDHC_SET_CTUNE:
        if (plen == 4) {
          uint32_t v = (uint32_t)payload[0]
                       | ((uint32_t)payload[1] << 8)
                       | ((uint32_t)payload[2] << 16)
                       | ((uint32_t)payload[3] << 24);
          sl_zigbee_dhc_ctune_t c = { .ctune = v };
          st = sl_zigbee_dhc_write_ctune(&c);
        } else {
          sl_zigbee_af_core_println("ERR: CTUNE requires 4 bytes, got %zu", plen);
          st = SL_STATUS_INVALID_COUNT;
        }
        break;
      case ZDHC_SET_VOLTAGE:
        if (plen == 2) {
          uint16_t v = (uint16_t)(payload[0] | (payload[1] << 8));
          st = sl_zigbee_dhc_write_pa_voltage(v);
        } else {
          st = SL_STATUS_INVALID_COUNT;
        }
        break;
      case ZDHC_SET_SIGNATURE:
        if (plen == 4) {
          sl_zigbee_dhc_pa_metadata_t md;
          st = sl_zigbee_dhc_read_pa_metadata(&md);
          if (st == SL_STATUS_OK) {
            md.signature = (uint32_t)payload[0]
                           | ((uint32_t)payload[1] << 8)
                           | ((uint32_t)payload[2] << 16)
                           | ((uint32_t)payload[3] << 24);
            st = sl_zigbee_dhc_write_pa_metadata(&md);
          }
        } else {
          st = SL_STATUS_INVALID_COUNT;
        }
        break;
      case ZDHC_SET_DHC_VERSION:
        if (plen == 1) {
          st = sl_zigbee_dhc_write_dhc_version(payload[0]);
        } else {
          st = SL_STATUS_INVALID_COUNT;
        }
        break;
      default:
        st = SL_STATUS_INVALID_PARAMETER;
        break;
    }
    // Echo back updated value
    if (st == SL_STATUS_OK) {
      // Simplified echo - you can expand this if needed
      response[rsp_len++] = 0x00; // placeholder
    }
  } else {
    st = SL_STATUS_NOT_SUPPORTED;
  }

  response[1] = (uint8_t)(st & 0xFF);
  cli_print_hex_response(response, rsp_len);
  cli_print_interpreted_response(response, rsp_len, setting);
}
