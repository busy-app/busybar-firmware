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

bool sli_zigbee_af_process_ezsp_command_green_power(uint16_t commandId)
{
  switch (commandId) {
//------------------------------------------------------------------------------

    case SL_ZIGBEE_EZSP_GP_PROXY_TABLE_PROCESS_GP_PAIRING: {
      bool gpPairingAdded;
      uint32_t options;
      sl_zigbee_gp_address_t addr;
      uint8_t commMode;
      uint16_t sinkNetworkAddress;
      uint16_t sinkGroupId;
      uint16_t assignedAlias;
      uint8_t sinkIeeeAddress[8];
      sl_zigbee_key_data_t gpdKey;
      uint32_t gpdSecurityFrameCounter;
      uint8_t forwardingRadius;
      options = fetchInt32u();
      fetch_sl_zigbee_gp_address_t(&addr);
      commMode = fetchInt8u();
      sinkNetworkAddress = fetchInt16u();
      sinkGroupId = fetchInt16u();
      assignedAlias = fetchInt16u();
      fetchInt8uArray(8, sinkIeeeAddress);
      fetch_sl_zigbee_key_data_t(&gpdKey);
      gpdSecurityFrameCounter = fetchInt32u();
      forwardingRadius = fetchInt8u();
      gpPairingAdded = sli_zigbee_stack_gp_proxy_table_process_gp_pairing(options, &addr, commMode, sinkNetworkAddress, sinkGroupId, assignedAlias, sinkIeeeAddress, &gpdKey, gpdSecurityFrameCounter, forwardingRadius);
      appendInt8u(gpPairingAdded);
      break;
    }

    case SL_ZIGBEE_EZSP_D_GP_SEND: {
      sl_status_t status;
      bool action;
      bool useCca;
      sl_zigbee_gp_address_t addr;
      uint8_t gpdCommandId;
      uint8_t gpdAsduLength;
      uint8_t *gpdAsdu;
      uint8_t gpepHandle;
      uint16_t gpTxQueueEntryLifetimeMs;
      action = fetchInt8u();
      useCca = fetchInt8u();
      fetch_sl_zigbee_gp_address_t(&addr);
      gpdCommandId = fetchInt8u();
      gpdAsduLength = fetchInt8u();
      gpdAsdu = (uint8_t *)fetchInt8uPointer(gpdAsduLength);
      gpepHandle = fetchInt8u();
      gpTxQueueEntryLifetimeMs = fetchInt16u();
      status = sli_zigbee_stack_d_gp_send(action, useCca, &addr, gpdCommandId, gpdAsduLength, gpdAsdu, gpepHandle, gpTxQueueEntryLifetimeMs);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GP_PROXY_TABLE_GET_ENTRY: {
      sl_status_t status;
      uint8_t proxyIndex;
      sl_zigbee_gp_proxy_table_entry_t entry;
      proxyIndex = fetchInt8u();
      status = sli_zigbee_stack_gp_proxy_table_get_entry(proxyIndex, &entry);
      appendInt32u(status);
      append_sl_zigbee_gp_proxy_table_entry_t(&entry);
      break;
    }

    case SL_ZIGBEE_EZSP_GP_PROXY_TABLE_LOOKUP: {
      uint8_t index;
      sl_zigbee_gp_address_t addr;
      fetch_sl_zigbee_gp_address_t(&addr);
      index = sli_zigbee_stack_gp_proxy_table_lookup(&addr);
      appendInt8u(index);
      break;
    }

    case SL_ZIGBEE_EZSP_GP_PROXY_TABLE_REMOVE_ENTRY: {
      uint8_t proxyIndex;
      proxyIndex = fetchInt8u();
      sli_zigbee_stack_gp_proxy_table_remove_entry(proxyIndex);
      break;
    }

    case SL_ZIGBEE_EZSP_GP_CLEAR_PROXY_TABLE: {
      sli_zigbee_stack_gp_clear_proxy_table();
      break;
    }

    case SL_ZIGBEE_EZSP_GP_SINK_TABLE_GET_ENTRY: {
      sl_status_t status;
      uint8_t sinkIndex;
      sl_zigbee_gp_sink_table_entry_t entry;
      sinkIndex = fetchInt8u();
      status = sli_zigbee_stack_gp_sink_table_get_entry(sinkIndex, &entry);
      appendInt32u(status);
      append_sl_zigbee_gp_sink_table_entry_t(&entry);
      break;
    }

    case SL_ZIGBEE_EZSP_GP_SINK_TABLE_LOOKUP: {
      uint8_t index;
      sl_zigbee_gp_address_t addr;
      fetch_sl_zigbee_gp_address_t(&addr);
      index = sli_zigbee_stack_gp_sink_table_lookup(&addr);
      appendInt8u(index);
      break;
    }

    case SL_ZIGBEE_EZSP_GP_SINK_TABLE_SET_ENTRY: {
      sl_status_t status;
      uint8_t sinkIndex;
      sl_zigbee_gp_sink_table_entry_t entry;
      sinkIndex = fetchInt8u();
      fetch_sl_zigbee_gp_sink_table_entry_t(&entry);
      status = sli_zigbee_stack_gp_sink_table_set_entry(sinkIndex, &entry);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GP_SINK_TABLE_REMOVE_ENTRY: {
      uint8_t sinkIndex;
      sinkIndex = fetchInt8u();
      sli_zigbee_stack_gp_sink_table_remove_entry(sinkIndex);
      break;
    }

    case SL_ZIGBEE_EZSP_GP_SINK_TABLE_FIND_OR_ALLOCATE_ENTRY: {
      uint8_t index;
      sl_zigbee_gp_address_t addr;
      fetch_sl_zigbee_gp_address_t(&addr);
      index = sli_zigbee_stack_gp_sink_table_find_or_allocate_entry(&addr);
      appendInt8u(index);
      break;
    }

    case SL_ZIGBEE_EZSP_GP_SINK_TABLE_CLEAR_ALL: {
      sli_zigbee_stack_gp_sink_table_clear_all();
      break;
    }

    case SL_ZIGBEE_EZSP_GP_SINK_TABLE_INIT: {
      sli_zigbee_stack_gp_sink_table_init();
      break;
    }

    case SL_ZIGBEE_EZSP_GP_SINK_TABLE_SET_SECURITY_FRAME_COUNTER: {
      uint8_t index;
      uint32_t sfc;
      index = fetchInt8u();
      sfc = fetchInt32u();
      sli_zigbee_stack_gp_sink_table_set_security_frame_counter(index, sfc);
      break;
    }

    case SL_ZIGBEE_EZSP_GP_SINK_TABLE_GET_NUMBER_OF_ACTIVE_ENTRIES: {
      uint8_t number_of_entries;
      number_of_entries = sli_zigbee_stack_gp_sink_table_get_number_of_active_entries();
      appendInt8u(number_of_entries);
      break;
    }

//------------------------------------------------------------------------------

    default: {
      return false;
    }
  }

  return true;
}
