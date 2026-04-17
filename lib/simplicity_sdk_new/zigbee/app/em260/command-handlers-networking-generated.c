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

bool sli_zigbee_af_process_ezsp_command_networking(uint16_t commandId)
{
  switch (commandId) {
//------------------------------------------------------------------------------

    case SL_ZIGBEE_EZSP_SET_MANUFACTURER_CODE: {
      sl_status_t status;
      uint16_t code;
      code = fetchInt16u();
      status = sli_zigbee_stack_set_manufacturer_code(code);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_MANUFACTURER_CODE: {
      uint16_t code;
      code = sli_zigbee_stack_get_manufacturer_code();
      appendInt16u(code);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_POWER_DESCRIPTOR: {
      sl_status_t status;
      uint16_t descriptor;
      descriptor = fetchInt16u();
      status = sli_zigbee_stack_set_power_descriptor(descriptor);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_NETWORK_INIT: {
      sl_status_t status;
      sl_zigbee_network_init_struct_t networkInitStruct;
      fetch_sl_zigbee_network_init_struct_t(&networkInitStruct);
      status = sli_zigbee_stack_network_init(&networkInitStruct);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_NETWORK_STATE: {
      sl_zigbee_network_status_t status;
      status = sli_zigbee_stack_network_state();
      appendInt8u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_START_SCAN: {
      sl_status_t status;
      sl_zigbee_ezsp_network_scan_type_t scanType;
      uint32_t channelMask;
      uint8_t duration;
      scanType = fetchInt8u();
      channelMask = fetchInt32u();
      duration = fetchInt8u();
      status = sli_zigbee_stack_start_scan(scanType, channelMask, duration);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_FIND_UNUSED_PAN_ID: {
      sl_status_t status;
      uint32_t channelMask;
      uint8_t duration;
      channelMask = fetchInt32u();
      duration = fetchInt8u();
      status = sli_zigbee_stack_find_unused_pan_id(channelMask, duration);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_STOP_SCAN: {
      sl_status_t status;
      status = sli_zigbee_stack_stop_scan();
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_FORM_NETWORK: {
      sl_status_t status;
      sl_zigbee_network_parameters_t parameters;
      fetch_sl_zigbee_network_parameters_t(&parameters);
      status = sli_zigbee_stack_form_network(&parameters);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_JOIN_NETWORK: {
      sl_status_t status;
      sl_zigbee_node_type_t nodeType;
      sl_zigbee_network_parameters_t parameters;
      nodeType = fetchInt8u();
      fetch_sl_zigbee_network_parameters_t(&parameters);
      status = sli_zigbee_stack_join_network(nodeType, &parameters);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_JOIN_NETWORK_DIRECTLY: {
      sl_status_t status;
      sl_zigbee_node_type_t localNodeType;
      sl_zigbee_beacon_data_t beacon;
      int8_t radioTxPower;
      bool clearBeaconsAfterNetworkUp;
      localNodeType = fetchInt8u();
      fetch_sl_zigbee_beacon_data_t(&beacon);
      radioTxPower = fetchInt8();
      clearBeaconsAfterNetworkUp = fetchInt8u();
      status = sli_zigbee_stack_join_network_directly(localNodeType, &beacon, radioTxPower, clearBeaconsAfterNetworkUp);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_LEAVE_NETWORK: {
      sl_status_t status;
      sl_zigbee_leave_network_option_t options;
      options = fetchInt8u();
      status = sli_zigbee_stack_leave_network(options);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_FIND_AND_REJOIN_NETWORK: {
      sl_status_t status;
      bool haveCurrentNetworkKey;
      uint32_t channelMask;
      uint8_t reason;
      uint8_t nodeType;
      haveCurrentNetworkKey = fetchInt8u();
      channelMask = fetchInt32u();
      reason = fetchInt8u();
      nodeType = fetchInt8u();
      status = sl_zigbee_af_ezsp_find_and_rejoin_network_command_cb(haveCurrentNetworkKey, channelMask, reason, nodeType);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_PERMIT_JOINING: {
      sl_status_t status;
      uint8_t duration;
      duration = fetchInt8u();
      status = sli_zigbee_stack_permit_joining(duration);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_ENERGY_SCAN_REQUEST: {
      sl_status_t status;
      sl_802154_short_addr_t target;
      uint32_t scanChannels;
      uint8_t scanDuration;
      uint16_t scanCount;
      target = fetchInt16u();
      scanChannels = fetchInt32u();
      scanDuration = fetchInt8u();
      scanCount = fetchInt16u();
      status = sli_zigbee_stack_energy_scan_request(target, scanChannels, scanDuration, scanCount);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_NETWORK_PARAMETERS: {
      sl_status_t status;
      sl_zigbee_node_type_t nodeType;
      sl_zigbee_network_parameters_t parameters;
      status = sl_zigbee_af_ezsp_get_network_parameters_command_cb(&nodeType, &parameters);
      appendInt32u(status);
      appendInt8u(nodeType);
      append_sl_zigbee_network_parameters_t(&parameters);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_RADIO_PARAMETERS: {
      sl_status_t status;
      uint8_t phyIndex;
      sl_zigbee_multi_phy_radio_parameters_t parameters;
      phyIndex = fetchInt8u();
      status = sli_zigbee_stack_get_radio_parameters(phyIndex, &parameters);
      appendInt32u(status);
      append_sl_zigbee_multi_phy_radio_parameters_t(&parameters);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_PARENT_CHILD_PARAMETERS: {
      uint8_t childCount;
      uint8_t parentEui64[8];
      sl_802154_short_addr_t parentNodeId;
      childCount = sl_zigbee_af_ezsp_get_parent_child_parameters_command_cb(parentEui64, &parentNodeId);
      appendInt8u(childCount);
      appendInt8uArray(8, parentEui64);
      appendInt16u(parentNodeId);
      break;
    }

    case SL_ZIGBEE_EZSP_ROUTER_CHILD_COUNT: {
      uint8_t routerChildCount;
      routerChildCount = sli_zigbee_stack_router_child_count();
      appendInt8u(routerChildCount);
      break;
    }

    case SL_ZIGBEE_EZSP_MAX_CHILD_COUNT: {
      uint8_t maxChildCount;
      maxChildCount = sli_zigbee_stack_max_child_count();
      appendInt8u(maxChildCount);
      break;
    }

    case SL_ZIGBEE_EZSP_MAX_ROUTER_CHILD_COUNT: {
      uint8_t maxRouterChildCount;
      maxRouterChildCount = sli_zigbee_stack_max_router_child_count();
      appendInt8u(maxRouterChildCount);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_PARENT_INCOMING_NWK_FRAME_COUNTER: {
      uint32_t parentIncomingNwkFrameCounter;
      parentIncomingNwkFrameCounter = sli_zigbee_stack_get_parent_incoming_nwk_frame_counter();
      appendInt32u(parentIncomingNwkFrameCounter);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_PARENT_INCOMING_NWK_FRAME_COUNTER: {
      sl_status_t status;
      uint32_t value;
      value = fetchInt32u();
      status = sli_zigbee_stack_set_parent_incoming_nwk_frame_counter(value);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_CURRENT_STACK_TASKS: {
      uint16_t activeTasks;
      activeTasks = sli_zigbee_stack_current_stack_tasks();
      appendInt16u(activeTasks);
      break;
    }

    case SL_ZIGBEE_EZSP_OK_TO_NAP: {
      bool value;
      value = sli_zigbee_stack_ok_to_nap();
      appendInt8u(value);
      break;
    }

    case SL_ZIGBEE_EZSP_PARENT_TOKEN_SET: {
      bool indicator;
      indicator = sli_zigbee_stack_parent_token_set();
      appendInt8u(indicator);
      break;
    }

    case SL_ZIGBEE_EZSP_OK_TO_HIBERNATE: {
      bool indicator;
      indicator = sli_zigbee_stack_ok_to_hibernate();
      appendInt8u(indicator);
      break;
    }

    case SL_ZIGBEE_EZSP_OK_TO_LONG_POLL: {
      bool indicator;
      indicator = sli_zigbee_stack_ok_to_long_poll();
      appendInt8u(indicator);
      break;
    }

    case SL_ZIGBEE_EZSP_STACK_POWER_DOWN: {
      sli_zigbee_stack_stack_power_down();
      break;
    }

    case SL_ZIGBEE_EZSP_STACK_POWER_UP: {
      sli_zigbee_stack_stack_power_up();
      break;
    }

    case SL_ZIGBEE_EZSP_GET_CHILD_DATA: {
      sl_status_t status;
      uint8_t index;
      sl_zigbee_child_data_t childData;
      index = fetchInt8u();
      status = sli_zigbee_stack_get_child_data(index, &childData);
      appendInt32u(status);
      append_sl_zigbee_child_data_t(&childData);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_CHILD_DATA: {
      sl_status_t status;
      uint8_t index;
      sl_zigbee_child_data_t childData;
      index = fetchInt8u();
      fetch_sl_zigbee_child_data_t(&childData);
      status = sli_zigbee_stack_set_child_data(index, &childData);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_CHILD_ID: {
      sl_802154_short_addr_t childId;
      uint8_t childIndex;
      childIndex = fetchInt8u();
      childId = sli_zigbee_stack_child_id(childIndex);
      appendInt16u(childId);
      break;
    }

    case SL_ZIGBEE_EZSP_CHILD_POWER: {
      int8_t childPower;
      uint8_t childIndex;
      childIndex = fetchInt8u();
      childPower = sli_zigbee_stack_child_power(childIndex);
      appendInt8(childPower);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_CHILD_POWER: {
      uint8_t childIndex;
      int8_t newPower;
      childIndex = fetchInt8u();
      newPower = fetchInt8();
      sli_zigbee_stack_set_child_power(childIndex, newPower);
      break;
    }

    case SL_ZIGBEE_EZSP_CHILD_INDEX: {
      uint8_t childIndex;
      sl_802154_short_addr_t childId;
      childId = fetchInt16u();
      childIndex = sli_zigbee_stack_child_index(childId);
      appendInt8u(childIndex);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_SOURCE_ROUTE_TABLE_TOTAL_SIZE: {
      uint8_t sourceRouteTableTotalSize;
      sourceRouteTableTotalSize = sl_zigbee_af_ezsp_get_source_route_table_total_size_command_cb();
      appendInt8u(sourceRouteTableTotalSize);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_SOURCE_ROUTE_TABLE_FILLED_SIZE: {
      uint8_t sourceRouteTableFilledSize;
      sourceRouteTableFilledSize = sl_zigbee_af_ezsp_get_source_route_table_filled_size_command_cb();
      appendInt8u(sourceRouteTableFilledSize);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_SOURCE_ROUTE_TABLE_ENTRY: {
      sl_status_t status;
      uint8_t index;
      sl_802154_short_addr_t destination;
      uint8_t closerIndex;
      index = fetchInt8u();
      status = sl_zigbee_af_ezsp_get_source_route_table_entry_command_cb(index, &destination, &closerIndex);
      appendInt32u(status);
      appendInt16u(destination);
      appendInt8u(closerIndex);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_NEIGHBOR: {
      sl_status_t status;
      uint8_t index;
      sl_zigbee_neighbor_table_entry_t value;
      index = fetchInt8u();
      status = sli_zigbee_stack_get_neighbor(index, &value);
      appendInt32u(status);
      append_sl_zigbee_neighbor_table_entry_t(&value);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_NEIGHBOR_FRAME_COUNTER: {
      sl_status_t status;
      uint8_t eui64[8];
      uint32_t returnFrameCounter;
      fetchInt8uArray(8, eui64);
      status = sli_zigbee_stack_get_neighbor_frame_counter(eui64, &returnFrameCounter);
      appendInt32u(status);
      appendInt32u(returnFrameCounter);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_NEIGHBOR_FRAME_COUNTER: {
      sl_status_t status;
      uint8_t eui64[8];
      uint32_t frameCounter;
      fetchInt8uArray(8, eui64);
      frameCounter = fetchInt32u();
      status = sli_zigbee_stack_set_neighbor_frame_counter(eui64, frameCounter);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_ROUTING_SHORTCUT_THRESHOLD: {
      sl_status_t status;
      uint8_t costThresh;
      costThresh = fetchInt8u();
      status = sl_zigbee_af_ezsp_set_routing_shortcut_threshold_command_cb(costThresh);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_ROUTING_SHORTCUT_THRESHOLD: {
      uint8_t routingShortcutThresh;
      routingShortcutThresh = sl_zigbee_af_ezsp_get_routing_shortcut_threshold_command_cb();
      appendInt8u(routingShortcutThresh);
      break;
    }

    case SL_ZIGBEE_EZSP_NEIGHBOR_COUNT: {
      uint8_t value;
      value = sli_zigbee_stack_neighbor_count();
      appendInt8u(value);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_ROUTE_TABLE_ENTRY: {
      sl_status_t status;
      uint8_t index;
      sl_zigbee_route_table_entry_t value;
      index = fetchInt8u();
      status = sli_zigbee_stack_get_route_table_entry(index, &value);
      appendInt32u(status);
      append_sl_zigbee_route_table_entry_t(&value);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_RADIO_POWER: {
      sl_status_t status;
      int8_t power;
      power = fetchInt8();
      status = sli_zigbee_stack_set_radio_power(power);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_RADIO_CHANNEL: {
      sl_status_t status;
      uint8_t channel;
      channel = fetchInt8u();
      status = sli_zigbee_stack_set_radio_channel(channel);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_RADIO_CHANNEL: {
      uint8_t channel;
      channel = sli_zigbee_stack_get_radio_channel();
      appendInt8u(channel);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_RADIO_IEEE802154_CCA_MODE: {
      sl_status_t status;
      uint8_t ccaMode;
      ccaMode = fetchInt8u();
      status = sli_zigbee_stack_set_radio_ieee802154_cca_mode(ccaMode);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_CONCENTRATOR: {
      sl_status_t status;
      bool on;
      uint16_t concentratorType;
      uint16_t minTime;
      uint16_t maxTime;
      uint8_t routeErrorThreshold;
      uint8_t deliveryFailureThreshold;
      uint8_t maxHops;
      on = fetchInt8u();
      concentratorType = fetchInt16u();
      minTime = fetchInt16u();
      maxTime = fetchInt16u();
      routeErrorThreshold = fetchInt8u();
      deliveryFailureThreshold = fetchInt8u();
      maxHops = fetchInt8u();
      status = sl_zigbee_af_ezsp_set_concentrator_command_cb(on, concentratorType, minTime, maxTime, routeErrorThreshold, deliveryFailureThreshold, maxHops);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_CONCENTRATOR_START_DISCOVERY: {
      sli_zigbee_stack_concentrator_start_discovery();
      break;
    }

    case SL_ZIGBEE_EZSP_CONCENTRATOR_STOP_DISCOVERY: {
      sli_zigbee_stack_concentrator_stop_discovery();
      break;
    }

    case SL_ZIGBEE_EZSP_CONCENTRATOR_NOTE_ROUTE_ERROR: {
      sl_status_t status;
      sl_802154_short_addr_t nodeId;
      status = fetchInt32u();
      nodeId = fetchInt16u();
      sli_zigbee_stack_concentrator_note_route_error(status, nodeId);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_BROKEN_ROUTE_ERROR_CODE: {
      sl_status_t status;
      uint8_t errorCode;
      errorCode = fetchInt8u();
      status = sl_zigbee_af_ezsp_set_broken_route_error_code_command_cb(errorCode);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_MULTI_PHY_START: {
      sl_status_t status;
      uint8_t phyIndex;
      uint8_t page;
      uint8_t channel;
      int8_t power;
      sl_zigbee_multi_phy_nwk_config_t bitmask;
      phyIndex = fetchInt8u();
      page = fetchInt8u();
      channel = fetchInt8u();
      power = fetchInt8();
      bitmask = fetchInt8u();
      status = sli_zigbee_stack_multi_phy_start(phyIndex, page, channel, power, bitmask);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_MULTI_PHY_STOP: {
      sl_status_t status;
      uint8_t phyIndex;
      phyIndex = fetchInt8u();
      status = sli_zigbee_stack_multi_phy_stop(phyIndex);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_MULTI_PHY_SET_RADIO_POWER: {
      sl_status_t status;
      uint8_t phyIndex;
      int8_t power;
      phyIndex = fetchInt8u();
      power = fetchInt8();
      status = sli_zigbee_stack_multi_phy_set_radio_power(phyIndex, power);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SEND_LINK_POWER_DELTA_REQUEST: {
      sl_status_t status;
      status = sli_zigbee_stack_send_link_power_delta_request();
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_MULTI_PHY_SET_RADIO_CHANNEL: {
      sl_status_t status;
      uint8_t phyIndex;
      uint8_t page;
      uint8_t channel;
      phyIndex = fetchInt8u();
      page = fetchInt8u();
      channel = fetchInt8u();
      status = sli_zigbee_stack_multi_phy_set_radio_channel(phyIndex, page, channel);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_DUTY_CYCLE_STATE: {
      sl_status_t status;
      sl_zigbee_duty_cycle_state_t returnedState;
      status = sli_zigbee_stack_get_duty_cycle_state(&returnedState);
      appendInt32u(status);
      appendInt8u(returnedState);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_DUTY_CYCLE_LIMITS_IN_STACK: {
      sl_status_t status;
      sl_zigbee_duty_cycle_limits_t limits;
      fetch_sl_zigbee_duty_cycle_limits_t(&limits);
      status = sli_zigbee_stack_set_duty_cycle_limits_in_stack(&limits);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_DUTY_CYCLE_LIMITS: {
      sl_status_t status;
      sl_zigbee_duty_cycle_limits_t returnedLimits;
      status = sli_zigbee_stack_get_duty_cycle_limits(&returnedLimits);
      appendInt32u(status);
      append_sl_zigbee_duty_cycle_limits_t(&returnedLimits);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_CURRENT_DUTY_CYCLE: {
      sl_status_t status;
      uint8_t maxDevices;
      uint8_t arrayOfDeviceDutyCycles[134];
      maxDevices = fetchInt8u();
      status = sl_zigbee_af_ezsp_get_current_duty_cycle_command_cb(maxDevices, arrayOfDeviceDutyCycles);
      appendInt32u(status);
      appendInt8uArray(134, arrayOfDeviceDutyCycles);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_NUM_BEACONS_TO_STORE: {
      sl_status_t status;
      uint8_t numBeacons;
      numBeacons = fetchInt8u();
      status = sli_zigbee_stack_set_num_beacons_to_store(numBeacons);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_STORED_BEACON: {
      sl_status_t status;
      uint8_t beacon_number;
      sl_zigbee_beacon_data_t beacon;
      beacon_number = fetchInt8u();
      status = sli_zigbee_stack_get_stored_beacon(beacon_number, &beacon);
      appendInt32u(status);
      append_sl_zigbee_beacon_data_t(&beacon);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_NUM_STORED_BEACONS: {
      uint8_t numBeacons;
      numBeacons = sli_zigbee_stack_get_num_stored_beacons();
      appendInt8u(numBeacons);
      break;
    }

    case SL_ZIGBEE_EZSP_CLEAR_STORED_BEACONS: {
      sl_status_t status;
      status = sli_zigbee_stack_clear_stored_beacons();
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_LOGICAL_AND_RADIO_CHANNEL: {
      sl_status_t status;
      uint8_t radioChannel;
      radioChannel = fetchInt8u();
      status = sli_zigbee_stack_set_logical_and_radio_channel(radioChannel);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SLEEPY_TO_SLEEPY_NETWORK_START: {
      sl_status_t status;
      sl_zigbee_network_parameters_t parameters;
      bool initiator;
      fetch_sl_zigbee_network_parameters_t(&parameters);
      initiator = fetchInt8u();
      status = sli_zigbee_stack_sleepy_to_sleepy_network_start(&parameters, initiator);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SEND_ZIGBEE_LEAVE: {
      sl_status_t status;
      sl_802154_pan_id_t destination;
      sl_zigbee_leave_request_flags_t flags;
      destination = fetchInt16u();
      flags = fetchInt8u();
      status = sli_zigbee_stack_send_zigbee_leave(destination, flags);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_PERMIT_JOINING: {
      bool joiningPermitted;
      joiningPermitted = sli_zigbee_stack_get_permit_joining();
      appendInt8u(joiningPermitted);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_EXTENDED_PAN_ID: {
      uint8_t extendedPanId[8];
      sli_zigbee_stack_get_extended_pan_id(extendedPanId);
      appendInt8uArray(8, extendedPanId);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_CURRENT_NETWORK: {
      uint8_t index;
      index = sli_zigbee_stack_get_current_network();
      appendInt8u(index);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_INITIAL_NEIGHBOR_OUTGOING_COST: {
      sl_status_t status;
      uint8_t cost;
      cost = fetchInt8u();
      status = sli_zigbee_stack_set_initial_neighbor_outgoing_cost(cost);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_INITIAL_NEIGHBOR_OUTGOING_COST: {
      uint8_t cost;
      cost = sli_zigbee_stack_get_initial_neighbor_outgoing_cost();
      appendInt8u(cost);
      break;
    }

    case SL_ZIGBEE_EZSP_RESET_REJOINING_NEIGHBORS_FRAME_COUNTER: {
      bool reset;
      reset = fetchInt8u();
      sli_zigbee_stack_reset_rejoining_neighbors_frame_counter(reset);
      break;
    }

    case SL_ZIGBEE_EZSP_IS_RESET_REJOINING_NEIGHBORS_FRAME_COUNTER_ENABLED: {
      bool getsReset;
      getsReset = sli_zigbee_stack_is_reset_rejoining_neighbors_frame_counter_enabled();
      appendInt8u(getsReset);
      break;
    }

//------------------------------------------------------------------------------

    default: {
      return false;
    }
  }

  return true;
}
