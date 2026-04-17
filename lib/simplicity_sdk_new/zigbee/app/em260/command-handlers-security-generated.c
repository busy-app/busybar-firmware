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

bool sli_zigbee_af_process_ezsp_command_security(uint16_t commandId)
{
  switch (commandId) {
//------------------------------------------------------------------------------

    case SL_ZIGBEE_EZSP_SET_INITIAL_SECURITY_STATE: {
      sl_status_t success;
      sl_zigbee_initial_security_state_t state;
      fetch_sl_zigbee_initial_security_state_t(&state);
      success = sli_zigbee_stack_set_initial_security_state(&state);
      appendInt32u(success);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_CURRENT_SECURITY_STATE: {
      sl_status_t status;
      sl_zigbee_current_security_state_t state;
      status = sli_zigbee_stack_get_current_security_state(&state);
      appendInt32u(status);
      append_sl_zigbee_current_security_state_t(&state);
      break;
    }

    case SL_ZIGBEE_EZSP_SEC_MAN_EXPORT_KEY: {
      sl_status_t status;
      sl_zigbee_sec_man_context_t context;
      sl_zigbee_sec_man_key_t key;
      fetch_sl_zigbee_sec_man_context_t(&context);
      status = sli_zigbee_stack_sec_man_export_key(&context, &key);
      appendInt32u(status);
      append_sl_zigbee_sec_man_key_t(&key);
      append_sl_zigbee_sec_man_context_t(&context);
      break;
    }

    case SL_ZIGBEE_EZSP_SEC_MAN_IMPORT_KEY: {
      sl_status_t status;
      sl_zigbee_sec_man_context_t context;
      sl_zigbee_sec_man_key_t key;
      fetch_sl_zigbee_sec_man_context_t(&context);
      fetch_sl_zigbee_sec_man_key_t(&key);
      status = sli_zigbee_stack_sec_man_import_key(&context, &key);
      appendInt32u(status);
      append_sl_zigbee_sec_man_context_t(&context);
      break;
    }

    case SL_ZIGBEE_EZSP_FIND_KEY_TABLE_ENTRY: {
      uint8_t index;
      uint8_t address[8];
      bool linkKey;
      fetchInt8uArray(8, address);
      linkKey = fetchInt8u();
      index = sli_zigbee_stack_find_key_table_entry(address, linkKey);
      appendInt8u(index);
      break;
    }

    case SL_ZIGBEE_EZSP_SEND_TRUST_CENTER_LINK_KEY: {
      sl_status_t status;
      sl_802154_short_addr_t destinationNodeId;
      uint8_t destinationEui64[8];
      destinationNodeId = fetchInt16u();
      fetchInt8uArray(8, destinationEui64);
      status = sli_zigbee_stack_send_trust_center_link_key(destinationNodeId, destinationEui64);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_ERASE_KEY_TABLE_ENTRY: {
      sl_status_t status;
      uint8_t index;
      index = fetchInt8u();
      status = sli_zigbee_stack_erase_key_table_entry(index);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_CLEAR_KEY_TABLE: {
      sl_status_t status;
      status = sli_zigbee_stack_clear_key_table();
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_REQUEST_LINK_KEY: {
      sl_status_t status;
      uint8_t partner[8];
      fetchInt8uArray(8, partner);
      status = sli_zigbee_stack_request_link_key(partner);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_UPDATE_TC_LINK_KEY: {
      sl_status_t status;
      uint8_t maxAttempts;
      maxAttempts = fetchInt8u();
      status = sli_zigbee_stack_update_tc_link_key(maxAttempts);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_CLEAR_TRANSIENT_LINK_KEYS: {
      sli_zigbee_stack_clear_transient_link_keys();
      break;
    }

    case SL_ZIGBEE_EZSP_SEC_MAN_GET_NETWORK_KEY_INFO: {
      sl_status_t status;
      sl_zigbee_sec_man_network_key_info_t network_key_info;
      status = sli_zigbee_stack_sec_man_get_network_key_info(&network_key_info);
      appendInt32u(status);
      append_sl_zigbee_sec_man_network_key_info_t(&network_key_info);
      break;
    }

    case SL_ZIGBEE_EZSP_SEC_MAN_GET_APS_KEY_INFO: {
      sl_status_t status;
      sl_zigbee_sec_man_context_t context;
      sl_zigbee_sec_man_aps_key_metadata_t key_data;
      fetch_sl_zigbee_sec_man_context_t(&context);
      status = sli_zigbee_stack_sec_man_get_aps_key_info(&context, &key_data);
      appendInt32u(status);
      append_sl_zigbee_sec_man_aps_key_metadata_t(&key_data);
      append_sl_zigbee_sec_man_context_t(&context);
      break;
    }

    case SL_ZIGBEE_EZSP_SEC_MAN_IMPORT_LINK_KEY: {
      sl_status_t status;
      uint8_t index;
      uint8_t address[8];
      sl_zigbee_sec_man_key_t plaintext_key;
      index = fetchInt8u();
      fetchInt8uArray(8, address);
      fetch_sl_zigbee_sec_man_key_t(&plaintext_key);
      status = sli_zigbee_stack_sec_man_import_link_key(index, address, &plaintext_key);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SEC_MAN_EXPORT_LINK_KEY_BY_INDEX: {
      sl_status_t status;
      uint8_t index;
      sl_zigbee_sec_man_context_t context;
      sl_zigbee_sec_man_key_t plaintext_key;
      sl_zigbee_sec_man_aps_key_metadata_t key_data;
      index = fetchInt8u();
      status = sli_zigbee_stack_sec_man_export_link_key_by_index(index, &context, &plaintext_key, &key_data);
      appendInt32u(status);
      append_sl_zigbee_sec_man_context_t(&context);
      append_sl_zigbee_sec_man_key_t(&plaintext_key);
      append_sl_zigbee_sec_man_aps_key_metadata_t(&key_data);
      break;
    }

    case SL_ZIGBEE_EZSP_SEC_MAN_EXPORT_LINK_KEY_BY_EUI: {
      sl_status_t status;
      uint8_t eui[8];
      sl_zigbee_sec_man_context_t context;
      sl_zigbee_sec_man_key_t plaintext_key;
      sl_zigbee_sec_man_aps_key_metadata_t key_data;
      fetchInt8uArray(8, eui);
      status = sli_zigbee_stack_sec_man_export_link_key_by_eui(eui, &context, &plaintext_key, &key_data);
      appendInt32u(status);
      append_sl_zigbee_sec_man_context_t(&context);
      append_sl_zigbee_sec_man_key_t(&plaintext_key);
      append_sl_zigbee_sec_man_aps_key_metadata_t(&key_data);
      break;
    }

    case SL_ZIGBEE_EZSP_SEC_MAN_CHECK_KEY_CONTEXT: {
      sl_status_t status;
      sl_zigbee_sec_man_context_t context;
      fetch_sl_zigbee_sec_man_context_t(&context);
      status = sli_zigbee_stack_sec_man_check_key_context(&context);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SEC_MAN_IMPORT_TRANSIENT_KEY: {
      sl_status_t status;
      uint8_t eui64[8];
      sl_zigbee_sec_man_key_t plaintext_key;
      fetchInt8uArray(8, eui64);
      fetch_sl_zigbee_sec_man_key_t(&plaintext_key);
      status = sli_zigbee_stack_sec_man_import_transient_key(eui64, &plaintext_key);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SEC_MAN_EXPORT_TRANSIENT_KEY_BY_INDEX: {
      sl_status_t status;
      uint8_t index;
      sl_zigbee_sec_man_context_t context;
      sl_zigbee_sec_man_key_t plaintext_key;
      sl_zigbee_sec_man_aps_key_metadata_t key_data;
      index = fetchInt8u();
      status = sli_zigbee_stack_sec_man_export_transient_key_by_index(index, &context, &plaintext_key, &key_data);
      appendInt32u(status);
      append_sl_zigbee_sec_man_context_t(&context);
      append_sl_zigbee_sec_man_key_t(&plaintext_key);
      append_sl_zigbee_sec_man_aps_key_metadata_t(&key_data);
      break;
    }

    case SL_ZIGBEE_EZSP_SEC_MAN_EXPORT_TRANSIENT_KEY_BY_EUI: {
      sl_status_t status;
      uint8_t eui[8];
      sl_zigbee_sec_man_context_t context;
      sl_zigbee_sec_man_key_t plaintext_key;
      sl_zigbee_sec_man_aps_key_metadata_t key_data;
      fetchInt8uArray(8, eui);
      status = sli_zigbee_stack_sec_man_export_transient_key_by_eui(eui, &context, &plaintext_key, &key_data);
      appendInt32u(status);
      append_sl_zigbee_sec_man_context_t(&context);
      append_sl_zigbee_sec_man_key_t(&plaintext_key);
      append_sl_zigbee_sec_man_aps_key_metadata_t(&key_data);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_INCOMING_TC_LINK_KEY_FRAME_COUNTER: {
      uint32_t frameCounter;
      frameCounter = fetchInt32u();
      sli_zigbee_stack_set_incoming_tc_link_key_frame_counter(frameCounter);
      break;
    }

    case SL_ZIGBEE_EZSP_APS_CRYPT_MESSAGE: {
      sl_status_t status;
      bool encrypt;
      uint8_t length_combined_arg;
      uint8_t *message;
      uint8_t apsHeaderEndIndex;
      uint8_t remoteEui64[8];
      encrypt = fetchInt8u();
      length_combined_arg = fetchInt8u();
      message = (uint8_t *)fetchInt8uPointer(length_combined_arg);
      apsHeaderEndIndex = fetchInt8u();
      fetchInt8uArray(8, remoteEui64);
      status = sli_zigbee_stack_aps_crypt_message(encrypt, length_combined_arg, message, apsHeaderEndIndex, remoteEui64);
      appendInt32u(status);
      appendInt8uArray(length_combined_arg, message);
      break;
    }

//------------------------------------------------------------------------------

    default: {
      return false;
    }
  }

  return true;
}
