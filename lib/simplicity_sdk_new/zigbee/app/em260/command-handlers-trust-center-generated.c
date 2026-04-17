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

bool sli_zigbee_af_process_ezsp_command_trust_center(uint16_t commandId)
{
  switch (commandId) {
//------------------------------------------------------------------------------

    case SL_ZIGBEE_EZSP_BROADCAST_NEXT_NETWORK_KEY: {
      sl_status_t status;
      sl_zigbee_key_data_t key;
      fetch_sl_zigbee_key_data_t(&key);
      status = sli_zigbee_stack_broadcast_next_network_key(&key);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_BROADCAST_NETWORK_KEY_SWITCH: {
      sl_status_t status;
      status = sli_zigbee_stack_broadcast_network_key_switch();
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_AES_MMO_HASH: {
      sl_status_t status;
      sl_zigbee_aes_mmo_hash_context_t context;
      bool finalize;
      uint8_t length;
      uint8_t *data;
      sl_zigbee_aes_mmo_hash_context_t returnContext;
      fetch_sl_zigbee_aes_mmo_hash_context_t(&context);
      finalize = fetchInt8u();
      length = fetchInt8u();
      data = (uint8_t *)fetchInt8uPointer(length);
      status = sl_zigbee_af_ezsp_aes_mmo_hash_command_cb(&context, finalize, length, data, &returnContext);
      appendInt32u(status);
      append_sl_zigbee_aes_mmo_hash_context_t(&returnContext);
      break;
    }

    case SL_ZIGBEE_EZSP_REMOVE_DEVICE: {
      sl_status_t status;
      sl_802154_short_addr_t destShort;
      uint8_t destLong[8];
      uint8_t targetLong[8];
      destShort = fetchInt16u();
      fetchInt8uArray(8, destLong);
      fetchInt8uArray(8, targetLong);
      status = sli_zigbee_stack_remove_device(destShort, destLong, targetLong);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_UNICAST_NWK_KEY_UPDATE: {
      sl_status_t status;
      sl_802154_short_addr_t destShort;
      uint8_t destLong[8];
      sl_zigbee_key_data_t key;
      destShort = fetchInt16u();
      fetchInt8uArray(8, destLong);
      fetch_sl_zigbee_key_data_t(&key);
      status = sli_zigbee_stack_unicast_nwk_key_update(destShort, destLong, &key);
      appendInt32u(status);
      break;
    }

//------------------------------------------------------------------------------

    default: {
      return false;
    }
  }

  return true;
}
