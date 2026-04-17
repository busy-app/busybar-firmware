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

bool sli_zigbee_af_process_ezsp_command_zll(uint16_t commandId)
{
  switch (commandId) {
//------------------------------------------------------------------------------

    case SL_ZIGBEE_EZSP_ZLL_NETWORK_OPS: {
      sl_status_t status;
      sl_zigbee_zll_network_t networkInfo;
      sl_zigbee_ezsp_zll_network_operation_t op;
      int8_t radioTxPower;
      fetch_sl_zigbee_zll_network_t(&networkInfo);
      op = fetchInt8u();
      radioTxPower = fetchInt8();
      status = sl_zigbee_af_ezsp_zll_network_ops_command_cb(&networkInfo, op, radioTxPower);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_ZLL_SET_INITIAL_SECURITY_STATE: {
      sl_status_t status;
      sl_zigbee_key_data_t networkKey;
      sl_zigbee_zll_initial_security_state_t securityState;
      fetch_sl_zigbee_key_data_t(&networkKey);
      fetch_sl_zigbee_zll_initial_security_state_t(&securityState);
      status = sli_zigbee_stack_zll_set_initial_security_state(&networkKey, &securityState);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_ZLL_SET_SECURITY_STATE_WITHOUT_KEY: {
      sl_status_t status;
      sl_zigbee_zll_initial_security_state_t securityState;
      fetch_sl_zigbee_zll_initial_security_state_t(&securityState);
      status = sli_zigbee_stack_zll_set_security_state_without_key(&securityState);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_ZLL_START_SCAN: {
      sl_status_t status;
      uint32_t channelMask;
      int8_t radioPowerForScan;
      sl_zigbee_node_type_t nodeType;
      channelMask = fetchInt32u();
      radioPowerForScan = fetchInt8();
      nodeType = fetchInt8u();
      status = sli_zigbee_stack_zll_start_scan(channelMask, radioPowerForScan, nodeType);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_ZLL_SET_RX_ON_WHEN_IDLE: {
      sl_status_t status;
      uint32_t durationMs;
      durationMs = fetchInt32u();
      status = sli_zigbee_stack_zll_set_rx_on_when_idle(durationMs);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_ZLL_GET_TOKENS: {
      sl_zigbee_tok_type_stack_zll_data_t data;
      sl_zigbee_tok_type_stack_zll_security_t security;
      sli_zigbee_stack_zll_get_tokens(&data, &security);
      append_sl_zigbee_tok_type_stack_zll_data_t(&data);
      append_sl_zigbee_tok_type_stack_zll_security_t(&security);
      break;
    }

    case SL_ZIGBEE_EZSP_ZLL_SET_DATA_TOKEN: {
      sl_zigbee_tok_type_stack_zll_data_t data;
      fetch_sl_zigbee_tok_type_stack_zll_data_t(&data);
      sli_zigbee_stack_zll_set_data_token(&data);
      break;
    }

    case SL_ZIGBEE_EZSP_ZLL_SET_NON_ZLL_NETWORK: {
      sli_zigbee_stack_zll_set_non_zll_network();
      break;
    }

    case SL_ZIGBEE_EZSP_IS_ZLL_NETWORK: {
      bool isZllNetwork;
      isZllNetwork = sli_zigbee_stack_is_zll_network();
      appendInt8u(isZllNetwork);
      break;
    }

    case SL_ZIGBEE_EZSP_ZLL_SET_RADIO_IDLE_MODE: {
      sl_zigbee_radio_power_mode_t mode;
      mode = fetchInt8u();
      sli_zigbee_stack_zll_set_radio_idle_mode(mode);
      break;
    }

    case SL_ZIGBEE_EZSP_ZLL_GET_RADIO_IDLE_MODE: {
      uint8_t radioIdleMode;
      radioIdleMode = sli_zigbee_stack_zll_get_radio_idle_mode();
      appendInt8u(radioIdleMode);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_ZLL_NODE_TYPE: {
      sl_zigbee_node_type_t nodeType;
      nodeType = fetchInt8u();
      sli_zigbee_stack_set_zll_node_type(nodeType);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_ZLL_ADDITIONAL_STATE: {
      uint16_t state;
      state = fetchInt16u();
      sli_zigbee_stack_set_zll_additional_state(state);
      break;
    }

    case SL_ZIGBEE_EZSP_ZLL_OPERATION_IN_PROGRESS: {
      bool zllOperationInProgress;
      zllOperationInProgress = sli_zigbee_stack_zll_operation_in_progress();
      appendInt8u(zllOperationInProgress);
      break;
    }

    case SL_ZIGBEE_EZSP_ZLL_RX_ON_WHEN_IDLE_GET_ACTIVE: {
      bool zllRxOnWhenIdleGetActive;
      zllRxOnWhenIdleGetActive = sli_zigbee_stack_zll_rx_on_when_idle_get_active();
      appendInt8u(zllRxOnWhenIdleGetActive);
      break;
    }

    case SL_ZIGBEE_EZSP_ZLL_SCANNING_COMPLETE: {
      sli_zigbee_stack_zll_scanning_complete();
      break;
    }

    case SL_ZIGBEE_EZSP_GET_ZLL_PRIMARY_CHANNEL_MASK: {
      uint32_t zllPrimaryChannelMask;
      zllPrimaryChannelMask = sli_zigbee_stack_get_zll_primary_channel_mask();
      appendInt32u(zllPrimaryChannelMask);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_ZLL_SECONDARY_CHANNEL_MASK: {
      uint32_t zllSecondaryChannelMask;
      zllSecondaryChannelMask = sli_zigbee_stack_get_zll_secondary_channel_mask();
      appendInt32u(zllSecondaryChannelMask);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_ZLL_PRIMARY_CHANNEL_MASK: {
      uint32_t zllPrimaryChannelMask;
      zllPrimaryChannelMask = fetchInt32u();
      sli_zigbee_stack_set_zll_primary_channel_mask(zllPrimaryChannelMask);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_ZLL_SECONDARY_CHANNEL_MASK: {
      uint32_t zllSecondaryChannelMask;
      zllSecondaryChannelMask = fetchInt32u();
      sli_zigbee_stack_set_zll_secondary_channel_mask(zllSecondaryChannelMask);
      break;
    }

    case SL_ZIGBEE_EZSP_ZLL_CLEAR_TOKENS: {
      sli_zigbee_stack_zll_clear_tokens();
      break;
    }

//------------------------------------------------------------------------------

    default: {
      return false;
    }
  }

  return true;
}
