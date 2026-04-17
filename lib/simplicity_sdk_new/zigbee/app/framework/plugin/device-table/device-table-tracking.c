/***************************************************************************//**
 * @file
 * @brief Routines for the Device Table plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#include PLATFORM_HEADER
#ifdef EZSP_HOST
// Includes needed for ember related functions for the EZSP host
  #include "stack/include/sl_zigbee_types.h"
  #include "app/util/ezsp/ezsp-protocol.h"
  #include "app/util/ezsp/ezsp.h"
  #include "app/util/ezsp/serial-interface.h"
  #include "app/util/zigbee-framework/zigbee-device-host.h"
#else
  #include "stack/include/sl_zigbee.h"
#endif

#include "hal/hal.h"
#include "app/framework/include/af.h"

#include <stdlib.h>
#include "app/framework/plugin/device-table/device-table.h"
#include "app/framework/plugin/device-table/device-table-internal.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"

// Device tracking declarations
static void optionallyChangeState(sl_802154_short_addr_t nodeId,
                                  sl_zigbee_af_plugin_device_table_device_state_t state);
static sl_zigbee_af_plugin_device_table_device_state_t getCurrentState(sl_802154_short_addr_t nodeId);

// --------------------------------
// Route Repair functions
static void serviceReturn(const sl_zigbee_af_service_discovery_result_t* result)
{
  sl_zigbee_af_core_println("ROUTE REPAIR SERVICE RETURN RESULT: status=%d",
                            result->status);
}

bool sli_zigbee_af_device_table_pre_command_received_callback(sl_zigbee_af_cluster_command_t* cmd)
{
  uint16_t nodeId = cmd->source;
  uint32_t currentTimestamp = halCommonGetInt32uMillisecondTick();
  uint32_t index = sli_zigbee_af_device_table_find_first_endpoint_node_id(nodeId);
  sl_zigbee_af_plugin_device_table_entry_t *deviceTable = sl_zigbee_af_device_table_pointer();

  if (getCurrentState(nodeId)
      >= SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_LEAVE_SENT
      && getCurrentState(nodeId) != SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_UNKNOWN) {
    // We have heard from a device that should have left but hasn't.  Try
    // re-sending the leave message.
    sl_zigbee_af_device_table_send_leave(sl_zigbee_af_device_table_get_index_from_node_id(nodeId));
    return false;
  }

  optionallyChangeState(nodeId, SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_JOINED);

  while (index != SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    deviceTable[index].lastMsgTimestamp = currentTimestamp;
    index = sli_zigbee_af_device_table_find_next_endpoint(index);
  }

  return false;
}

void sl_zigbee_af_device_table_initiate_route_repair(sl_802154_short_addr_t nodeId)
{
  sl_802154_long_addr_t eui64;

  // We only attempt to repair the situation where the node ID has changed.
  // To do that, simply send out a node ID request.
  if (sl_zigbee_af_device_table_get_eui64_from_node_id(nodeId, eui64)) {
    sl_zigbee_af_find_node_id(eui64, serviceReturn);
  }
}

// --------------------------------
// device state tracking
void sl_zigbee_af_device_table_send_leave(uint16_t index)
{
  sl_zigbee_af_plugin_device_table_entry_t *deviceTable = sl_zigbee_af_device_table_pointer();
  uint16_t nodeId = deviceTable[index].nodeId;
  sl_802154_long_addr_t destinationEui64;

  // make sure this is a valid index value
  assert(index < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE);
  // make sure this is a valid device table entry
  assert(nodeId != SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID);

  sl_zigbee_aps_option_t apsOptions = SL_ZIGBEE_APS_OPTION_RETRY
                                      | SL_ZIGBEE_APS_OPTION_ENABLE_ROUTE_DISCOVERY
                                      | SL_ZIGBEE_APS_OPTION_ENABLE_ADDRESS_DISCOVERY;

  // Copy elements from device table
  memcpy(destinationEui64, deviceTable[index].eui64, sizeof(sl_802154_long_addr_t));

  // Use the ZDO command to remove the device
  sl_zigbee_leave_request(nodeId,
                          destinationEui64,
                          0x00,  // Just leave.  Do not remove children, if any.
                          apsOptions);
  sl_zigbee_af_core_println("LEAVE_SENT");

  optionallyChangeState(deviceTable[index].nodeId, SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_LEAVE_SENT);
}

static void handleUnknownDevice(sl_802154_short_addr_t nodeId)
{
  // To do that, we need to send an IEEE request based on the node ID.
  sl_zigbee_ieee_address_request(nodeId,
                                 false,  // report kids?
                                 0,      // child start index
                                 SL_ZIGBEE_APS_OPTION_RETRY);
}

static sl_zigbee_af_plugin_device_table_device_state_t getCurrentState(sl_802154_short_addr_t nodeId)
{
  sl_zigbee_af_plugin_device_table_entry_t *deviceTable = sl_zigbee_af_device_table_pointer();
  uint16_t index = sl_zigbee_af_device_table_get_index_from_node_id(nodeId);

  if (index == SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    return SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_UNKNOWN;
  }

  return deviceTable[index].state;
}

// Handle state transitions.  If the requested state is different from the
// current state, we need to call the state change callback.
static void optionallyChangeState(sl_802154_short_addr_t nodeId,
                                  sl_zigbee_af_plugin_device_table_device_state_t state)
{
  sl_zigbee_af_plugin_device_table_entry_t *deviceTable = sl_zigbee_af_device_table_pointer();
  uint16_t index = sl_zigbee_af_device_table_get_index_from_node_id(nodeId);
  sl_zigbee_af_plugin_device_table_device_state_t originalState;

  //if we have a 0x0000 NodeID, we shouldn't do anything
  if (nodeId == 0x0000) {
    return;
  }
  if (index == SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    // We don't know this device.  Kick off a discovery process here.
    handleUnknownDevice(nodeId);
    return;
  }

  originalState = deviceTable[index].state;

  if (originalState < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_JOINED) {
    // Still in discovery mode...do nothing
    return;
  }

  if (originalState != state) {
    // Need to change state on all endpoints
    sli_zigbee_af_device_table_update_device_state(index, state);
    sl_zigbee_af_device_table_state_change_cb(deviceTable[index].nodeId,
                                              state);
  }
}

static bool isZdoLeaveCommand(uint16_t profileId,
                              uint16_t clusterId)
{
  return profileId == SL_ZIGBEE_ZDO_PROFILE_ID
         && clusterId == LEAVE_REQUEST ? true : false;
}

// If we send a message to a device and it was successful, then we label the
// device as joined.  If we send a message to a devcie and it was unsuccessful,
// change state to unresponsive.  Track whether it was successful here.
void sl_zigbee_af_device_table_message_sent_status(sl_802154_short_addr_t nodeId,
                                                   sl_status_t status,
                                                   uint16_t profileId,
                                                   uint16_t clusterId)
{
  // Do not transition if we are in the leaving state or if we are sending to an
  // unknown node.
  if ((getCurrentState(nodeId)
       == SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_LEAVE_SENT)
      || (getCurrentState(nodeId)
          == SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_UNKNOWN)) {
    return;
  }

  // See if we sent a leave command, if so always change the state to LEAVE_SENT
  // (because we still want to keep track of this even if the leave didn't
  // succeed). Otherwise check the status of the last message and determine if
  // the device is JOINED or UNRESPONSIVE.
  if (isZdoLeaveCommand(profileId, clusterId)) {
    optionallyChangeState(nodeId,
                          SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_LEAVE_SENT);
  } else {
    if (status == SL_STATUS_OK) {
      optionallyChangeState(nodeId, SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_JOINED);
    } else {
      optionallyChangeState(nodeId,
                            SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_UNRESPONSIVE);
    }
  }
}

static bool shouldDeviceLeave(sl_802154_short_addr_t nodeId)
{
  if (getCurrentState(nodeId)
      == SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_LEAVE_SENT) {
    sl_zigbee_af_device_table_send_leave(sl_zigbee_af_device_table_get_index_from_node_id(nodeId));
    return true;
  }

  return false;
}

bool sli_zigbee_af_device_table_should_device_leave(sl_802154_short_addr_t nodeId)
{
  return shouldDeviceLeave(nodeId);
}
