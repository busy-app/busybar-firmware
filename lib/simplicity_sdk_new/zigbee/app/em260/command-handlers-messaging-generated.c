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

bool sli_zigbee_af_process_ezsp_command_messaging(uint16_t commandId)
{
  switch (commandId) {
//------------------------------------------------------------------------------

    case SL_ZIGBEE_EZSP_MAXIMUM_PAYLOAD_LENGTH: {
      uint8_t apsLength;
      apsLength = sli_zigbee_stack_maximum_payload_length();
      appendInt8u(apsLength);
      break;
    }

    case SL_ZIGBEE_EZSP_SEND_UNICAST: {
      sl_status_t status;
      sl_zigbee_outgoing_message_type_t type;
      sl_802154_short_addr_t indexOrDestination;
      sl_zigbee_aps_frame_t apsFrame;
      uint16_t messageTag;
      uint8_t messageLength;
      uint8_t *messageContents;
      uint8_t sequence;
      type = fetchInt8u();
      indexOrDestination = fetchInt16u();
      fetch_sl_zigbee_aps_frame_t(&apsFrame);
      messageTag = fetchInt16u();
      messageLength = fetchInt8u();
      messageContents = (uint8_t *)fetchInt8uPointer(messageLength);
      status = sl_zigbee_af_ezsp_send_unicast_command_cb(type, indexOrDestination, &apsFrame, messageTag, messageLength, messageContents, &sequence);
      appendInt32u(status);
      appendInt8u(sequence);
      break;
    }

    case SL_ZIGBEE_EZSP_SEND_BROADCAST: {
      sl_status_t status;
      sl_802154_short_addr_t alias;
      sl_802154_short_addr_t destination;
      uint8_t nwkSequence;
      sl_zigbee_aps_frame_t apsFrame;
      uint8_t radius;
      uint16_t messageTag;
      uint8_t messageLength;
      uint8_t *messageContents;
      uint8_t apsSequence;
      alias = fetchInt16u();
      destination = fetchInt16u();
      nwkSequence = fetchInt8u();
      fetch_sl_zigbee_aps_frame_t(&apsFrame);
      radius = fetchInt8u();
      messageTag = fetchInt16u();
      messageLength = fetchInt8u();
      messageContents = (uint8_t *)fetchInt8uPointer(messageLength);
      status = sl_zigbee_af_ezsp_send_broadcast_command_cb(alias, destination, nwkSequence, &apsFrame, radius, messageTag, messageLength, messageContents, &apsSequence);
      appendInt32u(status);
      appendInt8u(apsSequence);
      break;
    }

    case SL_ZIGBEE_EZSP_PROXY_NEXT_BROADCAST_FROM_LONG: {
      sl_status_t status;
      uint8_t euiSource[8];
      fetchInt8uArray(8, euiSource);
      status = sli_zigbee_stack_proxy_next_broadcast_from_long(euiSource);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SEND_MULTICAST: {
      sl_status_t status;
      sl_zigbee_aps_frame_t apsFrame;
      uint8_t hops;
      uint16_t broadcastAddr;
      uint16_t alias;
      uint8_t nwkSequence;
      uint16_t messageTag;
      uint8_t messageLength;
      uint8_t *messageContents;
      uint8_t sequence;
      fetch_sl_zigbee_aps_frame_t(&apsFrame);
      hops = fetchInt8u();
      broadcastAddr = fetchInt16u();
      alias = fetchInt16u();
      nwkSequence = fetchInt8u();
      messageTag = fetchInt16u();
      messageLength = fetchInt8u();
      messageContents = (uint8_t *)fetchInt8uPointer(messageLength);
      status = sl_zigbee_af_ezsp_send_multicast_command_cb(&apsFrame, hops, broadcastAddr, alias, nwkSequence, messageTag, messageLength, messageContents, &sequence);
      appendInt32u(status);
      appendInt8u(sequence);
      break;
    }

    case SL_ZIGBEE_EZSP_SEND_REPLY: {
      sl_status_t status;
      sl_802154_short_addr_t sender;
      sl_zigbee_aps_frame_t apsFrame;
      uint8_t messageLength;
      uint8_t *messageContents;
      sender = fetchInt16u();
      fetch_sl_zigbee_aps_frame_t(&apsFrame);
      messageLength = fetchInt8u();
      messageContents = (uint8_t *)fetchInt8uPointer(messageLength);
      status = sl_zigbee_af_ezsp_send_reply_command_cb(sender, &apsFrame, messageLength, messageContents);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SEND_MANY_TO_ONE_ROUTE_REQUEST: {
      sl_status_t status;
      uint16_t concentratorType;
      uint8_t radius;
      concentratorType = fetchInt16u();
      radius = fetchInt8u();
      status = sli_zigbee_stack_send_many_to_one_route_request(concentratorType, radius);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_POLL_FOR_DATA: {
      sl_status_t status;
      uint16_t interval;
      sl_zigbee_event_units_t units;
      uint8_t failureLimit;
      interval = fetchInt16u();
      units = fetchInt8u();
      failureLimit = fetchInt8u();
      status = sl_zigbee_af_ezsp_poll_for_data_command_cb(interval, units, failureLimit);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_MESSAGE_FLAG: {
      sl_status_t status;
      sl_802154_short_addr_t childId;
      childId = fetchInt16u();
      status = sli_zigbee_stack_set_message_flag(childId);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_CLEAR_MESSAGE_FLAG: {
      sl_status_t status;
      sl_802154_short_addr_t childId;
      childId = fetchInt16u();
      status = sli_zigbee_stack_clear_message_flag(childId);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_ADD_CHILD: {
      sl_status_t status;
      sl_802154_short_addr_t shortId;
      uint8_t longId[8];
      sl_zigbee_node_type_t nodeType;
      shortId = fetchInt16u();
      fetchInt8uArray(8, longId);
      nodeType = fetchInt8u();
      status = sli_zigbee_stack_add_child(shortId, longId, nodeType);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_REMOVE_CHILD: {
      sl_status_t status;
      uint8_t childEui64[8];
      fetchInt8uArray(8, childEui64);
      status = sli_zigbee_stack_remove_child(childEui64);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_REMOVE_NEIGHBOR: {
      sl_802154_short_addr_t shortId;
      uint8_t longId[8];
      shortId = fetchInt16u();
      fetchInt8uArray(8, longId);
      sli_zigbee_stack_remove_neighbor(shortId, longId);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_SOURCE_ROUTE_DISCOVERY_MODE: {
      uint32_t remainingTime;
      uint8_t mode;
      mode = fetchInt8u();
      remainingTime = sl_zigbee_af_ezsp_set_source_route_discovery_mode_command_cb(mode);
      appendInt32u(remainingTime);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_SOURCE_ROUTE: {
      sl_status_t status;
      sl_802154_short_addr_t destination;
      uint8_t relayCount;
      uint16_t *relayList;
      destination = fetchInt16u();
      relayCount = fetchInt8u();
      relayList = (uint16_t *)fetchInt16uPointer(relayCount);
      status = sl_zigbee_af_ezsp_set_source_route_command_cb(destination, relayCount, relayList);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_UNICAST_CURRENT_NETWORK_KEY: {
      sl_status_t status;
      sl_802154_short_addr_t targetShort;
      uint8_t targetLong[8];
      sl_802154_short_addr_t parentShortId;
      targetShort = fetchInt16u();
      fetchInt8uArray(8, targetLong);
      parentShortId = fetchInt16u();
      status = sl_zigbee_af_ezsp_unicast_current_network_key_command_cb(targetShort, targetLong, parentShortId);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_ADDRESS_TABLE_ENTRY_IS_ACTIVE: {
      bool active;
      uint8_t addressTableIndex;
      addressTableIndex = fetchInt8u();
      active = sli_zigbee_stack_address_table_entry_is_active(addressTableIndex);
      appendInt8u(active);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_ADDRESS_TABLE_INFO: {
      sl_status_t status;
      uint8_t addressTableIndex;
      uint8_t eui64[8];
      sl_802154_short_addr_t id;
      addressTableIndex = fetchInt8u();
      fetchInt8uArray(8, eui64);
      id = fetchInt16u();
      status = sli_zigbee_stack_set_address_table_info(addressTableIndex, eui64, id);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_ADDRESS_TABLE_INFO: {
      sl_status_t status;
      uint8_t addressTableIndex;
      sl_802154_short_addr_t nodeId;
      uint8_t eui64[8];
      addressTableIndex = fetchInt8u();
      status = sli_zigbee_stack_get_address_table_info(addressTableIndex, &nodeId, eui64);
      appendInt32u(status);
      appendInt16u(nodeId);
      appendInt8uArray(8, eui64);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_EXTENDED_TIMEOUT: {
      sl_status_t status;
      uint8_t remoteEui64[8];
      bool extendedTimeout;
      fetchInt8uArray(8, remoteEui64);
      extendedTimeout = fetchInt8u();
      status = sli_zigbee_stack_set_extended_timeout(remoteEui64, extendedTimeout);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_EXTENDED_TIMEOUT: {
      sl_status_t status;
      uint8_t remoteEui64[8];
      fetchInt8uArray(8, remoteEui64);
      status = sli_zigbee_stack_get_extended_timeout(remoteEui64);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_REPLACE_ADDRESS_TABLE_ENTRY: {
      sl_status_t status;
      uint8_t addressTableIndex;
      uint8_t newEui64[8];
      sl_802154_short_addr_t newId;
      bool newExtendedTimeout;
      uint8_t oldEui64[8];
      sl_802154_short_addr_t oldId;
      bool oldExtendedTimeout;
      addressTableIndex = fetchInt8u();
      fetchInt8uArray(8, newEui64);
      newId = fetchInt16u();
      newExtendedTimeout = fetchInt8u();
      status = sl_zigbee_af_ezsp_replace_address_table_entry_command_cb(addressTableIndex, newEui64, newId, newExtendedTimeout, oldEui64, &oldId, &oldExtendedTimeout);
      appendInt32u(status);
      appendInt8uArray(8, oldEui64);
      appendInt16u(oldId);
      appendInt8u(oldExtendedTimeout);
      break;
    }

    case SL_ZIGBEE_EZSP_LOOKUP_NODE_ID_BY_EUI64: {
      sl_status_t status;
      uint8_t eui64[8];
      sl_802154_short_addr_t nodeId;
      fetchInt8uArray(8, eui64);
      status = sli_zigbee_stack_lookup_node_id_by_eui64(eui64, &nodeId);
      appendInt32u(status);
      appendInt16u(nodeId);
      break;
    }

    case SL_ZIGBEE_EZSP_LOOKUP_EUI64_BY_NODE_ID: {
      sl_status_t status;
      sl_802154_short_addr_t nodeId;
      uint8_t eui64[8];
      nodeId = fetchInt16u();
      status = sli_zigbee_stack_lookup_eui64_by_node_id(nodeId, eui64);
      appendInt32u(status);
      appendInt8uArray(8, eui64);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_MULTICAST_TABLE_ENTRY: {
      sl_status_t status;
      uint8_t index;
      sl_zigbee_multicast_table_entry_t value;
      index = fetchInt8u();
      status = sl_zigbee_af_ezsp_get_multicast_table_entry_command_cb(index, &value);
      appendInt32u(status);
      append_sl_zigbee_multicast_table_entry_t(&value);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_MULTICAST_TABLE_ENTRY: {
      sl_status_t status;
      uint8_t index;
      sl_zigbee_multicast_table_entry_t value;
      index = fetchInt8u();
      fetch_sl_zigbee_multicast_table_entry_t(&value);
      status = sl_zigbee_af_ezsp_set_multicast_table_entry_command_cb(index, &value);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_WRITE_NODE_DATA: {
      sl_status_t status;
      bool erase;
      erase = fetchInt8u();
      status = sli_zigbee_stack_write_node_data(erase);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SEND_RAW_MESSAGE: {
      sl_status_t status;
      uint8_t messageLength;
      uint8_t *messageContents;
      uint8_t priority;
      bool useCca;
      messageLength = fetchInt8u();
      messageContents = (uint8_t *)fetchInt8uPointer(messageLength);
      priority = fetchInt8u();
      useCca = fetchInt8u();
      status = sl_zigbee_af_ezsp_send_raw_message_command_cb(messageLength, messageContents, priority, useCca);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_MAC_POLL_FAILURE_WAIT_TIME: {
      uint32_t waitBeforeRetryIntervalMs;
      waitBeforeRetryIntervalMs = fetchInt32u();
      sli_zigbee_stack_set_mac_poll_failure_wait_time(waitBeforeRetryIntervalMs);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_MAX_MAC_RETRIES: {
      uint8_t retries;
      retries = sli_zigbee_stack_get_max_mac_retries();
      appendInt8u(retries);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_BEACON_CLASSIFICATION_PARAMS: {
      sl_status_t status;
      sl_zigbee_beacon_classification_params_t param;
      status = sli_zigbee_stack_set_beacon_classification_params(&param);
      appendInt32u(status);
      append_sl_zigbee_beacon_classification_params_t(&param);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_BEACON_CLASSIFICATION_PARAMS: {
      sl_status_t status;
      sl_zigbee_beacon_classification_params_t param;
      status = sli_zigbee_stack_get_beacon_classification_params(&param);
      appendInt32u(status);
      append_sl_zigbee_beacon_classification_params_t(&param);
      break;
    }

    case SL_ZIGBEE_EZSP_PENDING_ACKED_MESSAGES: {
      bool pending_messages;
      pending_messages = sli_zigbee_stack_pending_acked_messages();
      appendInt8u(pending_messages);
      break;
    }

    case SL_ZIGBEE_EZSP_RESCHEDULE_LINK_STATUS_MSG: {
      sl_status_t status;
      status = sli_zigbee_stack_reschedule_link_status_msg();
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_NWK_UPDATE_ID: {
      sl_status_t status;
      uint8_t nwkUpdateId;
      bool set_when_on_network;
      nwkUpdateId = fetchInt8u();
      set_when_on_network = fetchInt8u();
      status = sli_zigbee_stack_set_nwk_update_id(nwkUpdateId, set_when_on_network);
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
