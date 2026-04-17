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

bool sli_zigbee_af_process_ezsp_command_binding(uint16_t commandId)
{
  switch (commandId) {
//------------------------------------------------------------------------------

    case SL_ZIGBEE_EZSP_CLEAR_BINDING_TABLE: {
      sl_status_t status;
      status = sli_zigbee_stack_clear_binding_table();
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_BINDING: {
      sl_status_t status;
      uint8_t index;
      sl_zigbee_binding_table_entry_t value;
      index = fetchInt8u();
      fetch_sl_zigbee_binding_table_entry_t(&value);
      status = sl_zigbee_af_ezsp_set_binding_command_cb(index, &value);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_BINDING: {
      sl_status_t status;
      uint8_t index;
      sl_zigbee_binding_table_entry_t value;
      index = fetchInt8u();
      status = sli_zigbee_stack_get_binding(index, &value);
      appendInt32u(status);
      append_sl_zigbee_binding_table_entry_t(&value);
      break;
    }

    case SL_ZIGBEE_EZSP_DELETE_BINDING: {
      sl_status_t status;
      uint8_t index;
      index = fetchInt8u();
      status = sl_zigbee_af_ezsp_delete_binding_command_cb(index);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_BINDING_IS_ACTIVE: {
      bool active;
      uint8_t index;
      index = fetchInt8u();
      active = sli_zigbee_stack_binding_is_active(index);
      appendInt8u(active);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_BINDING_REMOTE_NODE_ID: {
      sl_802154_short_addr_t nodeId;
      uint8_t index;
      index = fetchInt8u();
      nodeId = sli_zigbee_stack_get_binding_remote_node_id(index);
      appendInt16u(nodeId);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_BINDING_REMOTE_NODE_ID: {
      uint8_t index;
      sl_802154_short_addr_t nodeId;
      index = fetchInt8u();
      nodeId = fetchInt16u();
      sli_zigbee_stack_set_binding_remote_node_id(index, nodeId);
      break;
    }

//------------------------------------------------------------------------------

    default: {
      return false;
    }
  }

  return true;
}
