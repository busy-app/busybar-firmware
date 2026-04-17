
/*****************************************************************************/
/**
 * Copyright 2021 Silicon Laboratories, Inc.
 *
 *****************************************************************************/
//
// *** Generated file. Do not edit! ***
//
// Description: Handlers for the EZSP frames that directly correspond to Ember
// API calls.

#include PLATFORM_HEADER
#include "stack/include/sl_zigbee_types.h"
#include "stack/internal/inc/internal-defs-patch.h"
#include "ezsp-enum.h"
#include "app/em260/command-context.h"
#include "stack/include/cbke-crypto-engine.h"
#include "stack/include/zigbee-security-manager.h"
#include "stack/internal/inc/mfglib_internal_def.h"
#include "stack/include/binding-table.h"
#include "stack/include/message.h"
#include "stack/include/mac-layer.h"
#include "app/util/ezsp/ezsp-frame-utilities.h"
#include "app/em260/command-handlers-cbke.h"
#include "app/em260/command-handlers-binding.h"
#include "app/em260/command-handlers-mfglib.h"
#include "app/em260/command-handlers-security.h"
#include "app/em260/command-handlers-zll.h"
#include "app/em260/command-handlers-zigbee-pro.h"
#include "child.h"
#include "message.h"
#include "zll-api.h"
#include "security.h"
#include "stack-info.h"
#include "network-formation.h"
#include "zigbee-device-stack.h"
#include "sl_zigbee_duty_cycle.h"
#include "multi-phy.h"
#include "stack/include/gp-sink-table.h"
#include "stack/include/gp-proxy-table.h"
#include "stack/include/source-route.h"
#include "stack/include/multi-network.h"
#include "stack/include/sl_zigbee_dhc.h"
#include "app/framework/plugin/dhc/dhc-ncp.h"

bool sli_zigbee_af_process_ezsp_command_dynamic_hardware_configuration(uint16_t commandId)
{
  switch (commandId) {
//------------------------------------------------------------------------------

  case SL_ZIGBEE_EZSP_READ_PA_DESCRIPTOR: {
    sl_status_t status;
    uint8_t index;
    sl_zigbee_dhc_pa_descriptor_t descriptor;
    index = fetchInt8u();
    status = sli_zigbee_stack_read_pa_descriptor(index, &descriptor);
    appendInt32u(status);
    append_sl_zigbee_dhc_pa_descriptor_t(&descriptor);
    break;
  }

  case SL_ZIGBEE_EZSP_WRITE_PA_DESCRIPTOR: {
    sl_status_t status;
    uint8_t index;
    sl_zigbee_dhc_pa_descriptor_t descriptor;
    index = fetchInt8u();
    fetch_sl_zigbee_dhc_pa_descriptor_t(&descriptor);
    status = sli_zigbee_stack_write_pa_descriptor(index, &descriptor);
    appendInt32u(status);
    break;
  }

  case SL_ZIGBEE_EZSP_READ_PA_CURVE_SEGMENT: {
    sl_status_t status;
    uint8_t index;
    uint8_t segment_index;
    sl_zigbee_dhc_pa_curve_segment_t segment;
    index = fetchInt8u();
    segment_index = fetchInt8u();
    status = sli_zigbee_stack_read_pa_curve_segment(index, segment_index, &segment);
    appendInt32u(status);
    append_sl_zigbee_dhc_pa_curve_segment_t(&segment);
    break;
  }

  case SL_ZIGBEE_EZSP_WRITE_PA_CURVE_SEGMENT: {
    sl_status_t status;
    uint8_t index;
    uint8_t segment_index;
    sl_zigbee_dhc_pa_curve_segment_t segment;
    index = fetchInt8u();
    segment_index = fetchInt8u();
    fetch_sl_zigbee_dhc_pa_curve_segment_t(&segment);
    status = sli_zigbee_stack_write_pa_curve_segment(index, segment_index, &segment);
    appendInt32u(status);
    break;
  }

  case SL_ZIGBEE_EZSP_READ_PA_CURVE: {
    sl_status_t status;
    uint8_t index;
    sl_zigbee_dhc_pa_curve_t curve;
    index = fetchInt8u();
    status = sli_zigbee_stack_read_pa_curve(index, &curve);
    appendInt32u(status);
    append_sl_zigbee_dhc_pa_curve_t(&curve);
    break;
  }

  case SL_ZIGBEE_EZSP_WRITE_PA_CURVE: {
    sl_status_t status;
    uint8_t index;
    sl_zigbee_dhc_pa_curve_t curve;
    index = fetchInt8u();
    fetch_sl_zigbee_dhc_pa_curve_t(&curve);
    status = sli_zigbee_stack_write_pa_curve(index, &curve);
    appendInt32u(status);
    break;
  }

  case SL_ZIGBEE_EZSP_READ_PA_TABLE: {
    sl_status_t status;
    uint8_t index;
    sl_zigbee_dhc_pa_table_t table;
    index = fetchInt8u();
    status = sli_zigbee_stack_read_pa_table(index, &table);
    appendInt32u(status);
    append_sl_zigbee_dhc_pa_table_t(&table);
    break;
  }

  case SL_ZIGBEE_EZSP_WRITE_PA_TABLE: {
    sl_status_t status;
    uint8_t index;
    sl_zigbee_dhc_pa_table_t table;
    index = fetchInt8u();
    fetch_sl_zigbee_dhc_pa_table_t(&table);
    status = sli_zigbee_stack_write_pa_table(index, &table);
    appendInt32u(status);
    break;
  }

  case SL_ZIGBEE_EZSP_READ_RSSI_OFFSET: {
    sl_status_t status;
    sl_zigbee_dhc_rssi_offset_t rssi_offset;
    status = sli_zigbee_stack_read_rssi_offset(&rssi_offset);
    appendInt32u(status);
    append_sl_zigbee_dhc_rssi_offset_t(&rssi_offset);
    break;
  }

  case SL_ZIGBEE_EZSP_WRITE_RSSI_OFFSET: {
    sl_status_t status;
    sl_zigbee_dhc_rssi_offset_t rssi_offset;
    fetch_sl_zigbee_dhc_rssi_offset_t(&rssi_offset);
    status = sli_zigbee_stack_write_rssi_offset(&rssi_offset);
    appendInt32u(status);
    break;
  }

  case SL_ZIGBEE_EZSP_READ_PA_VOLTAGE: {
    sl_status_t status;
    uint16_t pa_voltage;
    status = sli_zigbee_stack_read_pa_voltage(&pa_voltage);
    appendInt32u(status);
    appendInt16u(pa_voltage);
    break;
  }

  case SL_ZIGBEE_EZSP_WRITE_PA_VOLTAGE: {
    sl_status_t status;
    uint16_t pa_voltage;
    pa_voltage = fetchInt16u();
    status = sli_zigbee_stack_write_pa_voltage(pa_voltage);
    appendInt32u(status);
    break;
  }

  case SL_ZIGBEE_EZSP_READ_PA_MODE: {
    sl_status_t status;
    sl_zigbee_dhc_pa_mode_t pa_mode;
    status = sli_zigbee_stack_read_pa_mode(&pa_mode);
    appendInt32u(status);
    append_sl_zigbee_dhc_pa_mode_t(&pa_mode);
    break;
  }

  case SL_ZIGBEE_EZSP_WRITE_PA_MODE: {
    sl_status_t status;
    sl_zigbee_dhc_pa_mode_t pa_mode;
    fetch_sl_zigbee_dhc_pa_mode_t(&pa_mode);
    status = sli_zigbee_stack_write_pa_mode(&pa_mode);
    appendInt32u(status);
    break;
  }

  case SL_ZIGBEE_EZSP_READ_CTUNE: {
    sl_status_t status;
    sl_zigbee_dhc_ctune_t ctune;
    status = sli_zigbee_stack_read_ctune(&ctune);
    appendInt32u(status);
    append_sl_zigbee_dhc_ctune_t(&ctune);
    break;
  }

  case SL_ZIGBEE_EZSP_WRITE_CTUNE: {
    sl_status_t status;
    sl_zigbee_dhc_ctune_t ctune;
    fetch_sl_zigbee_dhc_ctune_t(&ctune);
    status = sli_zigbee_stack_write_ctune(&ctune);
    appendInt32u(status);
    break;
  }

  case SL_ZIGBEE_EZSP_READ_DHC_VERSION: {
    sl_status_t status;
    uint8_t dhc_version;
    status = sli_zigbee_stack_read_dhc_version(&dhc_version);
    appendInt32u(status);
    appendInt8u(dhc_version);
    break;
  }

  case SL_ZIGBEE_EZSP_WRITE_DHC_VERSION: {
    sl_status_t status;
    uint8_t dhc_version;
    dhc_version = fetchInt8u();
    status = sli_zigbee_stack_write_dhc_version(dhc_version);
    appendInt32u(status);
    break;
  }

  case SL_ZIGBEE_EZSP_READ_PA_VERSION: {
    sl_status_t status;
    sl_zigbee_dhc_pa_version_t pa_version;
    status = sli_zigbee_stack_read_pa_version(&pa_version);
    appendInt32u(status);
    append_sl_zigbee_dhc_pa_version_t(&pa_version);
    break;
  }

  case SL_ZIGBEE_EZSP_READ_PA_SIGNATURE: {
    sl_status_t status;
    sl_zigbee_dhc_pa_signature_t pa_signature;
    status = sli_zigbee_stack_read_pa_signature(&pa_signature);
    appendInt32u(status);
    append_sl_zigbee_dhc_pa_signature_t(&pa_signature);
    break;
  }

  case SL_ZIGBEE_EZSP_WRITE_PA_SIGNATURE: {
    sl_status_t status;
    sl_zigbee_dhc_pa_signature_t pa_signature;
    fetch_sl_zigbee_dhc_pa_signature_t(&pa_signature);
    status = sli_zigbee_stack_write_pa_signature(&pa_signature);
    appendInt32u(status);
    break;
  }

  case SL_ZIGBEE_EZSP_READ_PA_METADATA: {
    sl_status_t status;
    sl_zigbee_dhc_pa_metadata_t metadata;
    status = sli_zigbee_stack_read_pa_metadata(&metadata);
    appendInt32u(status);
    append_sl_zigbee_dhc_pa_metadata_t(&metadata);
    break;
  }

  case SL_ZIGBEE_EZSP_WRITE_PA_METADATA: {
    sl_status_t status;
    sl_zigbee_dhc_pa_metadata_t metadata;
    fetch_sl_zigbee_dhc_pa_metadata_t(&metadata);
    status = sli_zigbee_stack_write_pa_metadata(&metadata);
    appendInt32u(status);
    break;
  }


    default: {
      return false;
    }
  }

  return true;
}
