/***************************************************************************//**
 * @file sl_zigbee_dhc_internal_weak_stubs.c
 * @brief Stubbed stack-layer helpers for Dynamic Hardware Configuration.
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

#include "sl_common.h"
#include <string.h>

#include "sl_zigbee_dhc.h"

static sl_status_t dhc_stack_stub_status(void)
{
  return SL_STATUS_NOT_AVAILABLE;
}

static void dhc_stack_stub_zero(void *ptr, size_t size)
{
  if (ptr != NULL) {
    memset(ptr, 0, size);
  }
}

SL_WEAK sl_status_t sli_zigbee_stack_read_pa_metadata(sl_zigbee_dhc_pa_metadata_t *metadata)
{
  dhc_stack_stub_zero(metadata, sizeof(*metadata));
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_write_pa_metadata(sl_zigbee_dhc_pa_metadata_t *metadata)
{
  (void)metadata;
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_read_pa_descriptor(uint8_t index,
                                                        sl_zigbee_dhc_pa_descriptor_t *descriptor)
{
  (void)index;
  dhc_stack_stub_zero(descriptor, sizeof(*descriptor));
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_write_pa_descriptor(uint8_t index,
                                                         sl_zigbee_dhc_pa_descriptor_t *descriptor)
{
  (void)index;
  (void)descriptor;
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_read_pa_curve(uint8_t index,
                                                   sl_zigbee_dhc_pa_curve_t *curve)
{
  (void)index;
  dhc_stack_stub_zero(curve, sizeof(*curve));
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_write_pa_curve(uint8_t index,
                                                    sl_zigbee_dhc_pa_curve_t *curve)
{
  (void)index;
  (void)curve;
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_read_pa_curve_segment(uint8_t index,
                                                           uint8_t segment_index,
                                                           sl_zigbee_dhc_pa_curve_segment_t *segment)
{
  (void)index;
  (void)segment_index;
  dhc_stack_stub_zero(segment, sizeof(*segment));
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_write_pa_curve_segment(uint8_t index,
                                                            uint8_t segment_index,
                                                            sl_zigbee_dhc_pa_curve_segment_t *segment)
{
  (void)index;
  (void)segment_index;
  (void)segment;
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_read_pa_table(uint8_t index,
                                                   sl_zigbee_dhc_pa_table_t *table)
{
  (void)index;
  dhc_stack_stub_zero(table, sizeof(*table));
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_write_pa_table(uint8_t index,
                                                    sl_zigbee_dhc_pa_table_t *table)
{
  (void)index;
  (void)table;
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_read_rssi_offset(sl_zigbee_dhc_rssi_offset_t *rssi_offset)
{
  dhc_stack_stub_zero(rssi_offset, sizeof(*rssi_offset));
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_write_rssi_offset(sl_zigbee_dhc_rssi_offset_t *rssi_offset)
{
  (void)rssi_offset;
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_read_pa_voltage(uint16_t *pa_voltage)
{
  if (pa_voltage != NULL) {
    *pa_voltage = 0U;
  }
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_write_pa_voltage(uint16_t pa_voltage)
{
  (void)pa_voltage;
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_read_pa_mode(sl_zigbee_dhc_pa_mode_t *pa_mode)
{
  dhc_stack_stub_zero(pa_mode, sizeof(*pa_mode));
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_write_pa_mode(sl_zigbee_dhc_pa_mode_t *pa_mode)
{
  (void)pa_mode;
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_read_ctune(sl_zigbee_dhc_ctune_t *ctune)
{
  dhc_stack_stub_zero(ctune, sizeof(*ctune));
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_write_ctune(sl_zigbee_dhc_ctune_t *ctune)
{
  (void)ctune;
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_read_dhc_version(uint8_t *dhc_version)
{
  if (dhc_version != NULL) {
    *dhc_version = 0U;
  }
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_write_dhc_version(uint8_t dhc_version)
{
  (void)dhc_version;
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_read_pa_version(sl_zigbee_dhc_pa_version_t *pa_version)
{
  dhc_stack_stub_zero(pa_version, sizeof(*pa_version));
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_read_pa_signature(sl_zigbee_dhc_pa_signature_t *pa_signature)
{
  dhc_stack_stub_zero(pa_signature, sizeof(*pa_signature));
  return dhc_stack_stub_status();
}

SL_WEAK sl_status_t sli_zigbee_stack_write_pa_signature(sl_zigbee_dhc_pa_signature_t *pa_signature)
{
  (void)pa_signature;
  return dhc_stack_stub_status();
}
