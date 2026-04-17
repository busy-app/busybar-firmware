/***************************************************************************//**
 * @file
 * @brief This plugin unicasts a permit joins message to joining devices.
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

// Procedure:

// When a device is attempting to the join the network:
// 1.If the node isn't an end device AND already in the joiningDeviceList,
// we record the node in the joiningDeviceList and set of a timer for
// SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_DEVICE_ANNOUNCE_TIMEOUT milliseconds.
// 2. If we receive a device Announce from the message AND
// if we have an entry in the joining device table,
// we unicast a Permit Joins message to the device and remove it from the queue.
// 3. Else if the timer expires, we unicast a permit Joins message to the device and
// dequeue it.

#include "app/framework/include/af.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_PERMIT_JOIN_MANAGER_PRESENT
#include "permit-join-manager-config.h"
#else
#include "app/framework/plugin/permit-join-manager/config/permit-join-manager-config.h"
#endif

//=============================================================================
// Globals

//The following globals are defined for unit tests.

#if !defined(SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_JOINING_DEVICES_QUEUE_LENGTH)
  #define SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_JOINING_DEVICES_QUEUE_LENGTH 4
#endif

#if !defined(SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_PERMIT_JOINS_DURATION)
  #define SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_PERMIT_JOINS_DURATION 40
#endif

#if !defined(SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_DEVICE_ANNOUNCE_TIMEOUT)
  #define SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_DEVICE_ANNOUNCE_TIMEOUT 1000
#endif

sl_zigbee_af_event_t sl_zigbee_af_permit_join_manager_timeout_network_events[SL_ZIGBEE_SUPPORTED_NETWORKS];
void sl_zigbee_af_permit_join_manager_timeout_network_event_handler(sl_zigbee_af_event_t * event);
static sl_zigbee_af_joining_device_t joiningDeviceList[SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_JOINING_DEVICES_QUEUE_LENGTH];

// With device announce there is only the ZDO sequence number, there is no status code.
#define DEVICE_ANNOUNCE_NODE_ID_OFFSET 1
#define DEVICE_ANNOUNCE_EUI64_OFFSET   (DEVICE_ANNOUNCE_NODE_ID_OFFSET + 2)
#define DEVICE_ANNOUNCE_CAPABILITIES_OFFSET (DEVICE_ANNOUNCE_EUI64_OFFSET + EUI64_SIZE)

#define RECEIVER_ON_WHEN_IDLE 0x08
//============================================================================
// Forward Declarations
static sl_status_t sendPermitJoinsRequest(sl_802154_short_addr_t sl_zigbee_node_id);
static void addDeviceToJoiningList(sl_802154_short_addr_t sl_zigbee_node_id);
//============================================================================

static sl_status_t sendPermitJoinsRequest(sl_802154_short_addr_t sl_zigbee_node_id)
{
  sl_status_t status;
  uint8_t duration;

  duration = SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_PERMIT_JOINS_DURATION;
  status = sl_zigbee_permit_joining_request(sl_zigbee_node_id,
                                            duration,
                                            0x01, // TC significance is always 1
                                            0);
  return status;
}

static void addDeviceToJoiningList(sl_802154_short_addr_t sl_zigbee_node_id)
{
  int i = 0;

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_JOINING_DEVICES_QUEUE_LENGTH; i++) {
    if (sl_zigbee_node_id == joiningDeviceList[i].sl_zigbee_node_id) {
      //fprintf(stderr, "Found sl_zigbee_node_id %u\n", sl_zigbee_node_id);
      return;
    }
  }
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_JOINING_DEVICES_QUEUE_LENGTH; i++) {
    if (joiningDeviceList[i].sl_zigbee_node_id == SL_ZIGBEE_NULL_NODE_ID) {
      sl_zigbee_af_core_println("Adding to queue at index %u", i);
      joiningDeviceList[i].sl_zigbee_node_id = sl_zigbee_node_id;
      joiningDeviceList[i].timeStamp = halCommonGetInt32uMillisecondTick();
      sl_zigbee_af_event_set_delay_ms(sl_zigbee_af_permit_join_manager_timeout_network_events,
                                      SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_DEVICE_ANNOUNCE_TIMEOUT);

      return;
    }
  }
  sl_zigbee_af_core_println("Ran out of space on Joining device queue");
}

static void removeDeviceFromJoiningList(sl_802154_short_addr_t sl_zigbee_node_id)
{
  int i = 0;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_JOINING_DEVICES_QUEUE_LENGTH; i++) {
    if (joiningDeviceList[i].sl_zigbee_node_id == sl_zigbee_node_id) {
      //fprintf(stderr,"\r\nFound node Id %u at index %u",sl_zigbee_node_id,i);
      joiningDeviceList[i].sl_zigbee_node_id = SL_ZIGBEE_NULL_NODE_ID;
      joiningDeviceList[i].timeStamp = 0;
      return;
    }
  }
}

static sl_802154_short_addr_t findDeviceInJoiningList(sl_802154_short_addr_t sl_zigbee_node_id)
{
  int i = 0;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_JOINING_DEVICES_QUEUE_LENGTH; i++) {
    if (joiningDeviceList[i].sl_zigbee_node_id == sl_zigbee_node_id) {
      return sl_zigbee_node_id;
    }
  }
  return SL_ZIGBEE_NULL_NODE_ID;
}

void sl_zigbee_af_permit_join_manager_init_cb(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
      sl_zigbee_af_network_event_init(sl_zigbee_af_permit_join_manager_timeout_network_events,
                                      sl_zigbee_af_permit_join_manager_timeout_network_event_handler);
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA:
    {
      int i = 0;
      for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_JOINING_DEVICES_QUEUE_LENGTH; i++) {
        joiningDeviceList[i].sl_zigbee_node_id = SL_ZIGBEE_NULL_NODE_ID;
        joiningDeviceList[i].timeStamp = 0;
      }
      break;
    }
  }
}

bool sl_zigbee_af_permit_join_manager_zdo_message_received_cb(sl_802154_short_addr_t sl_zigbee_node_id,
                                                              sl_zigbee_aps_frame_t* apsFrame,
                                                              uint8_t* message,
                                                              uint16_t length)
{
  if (apsFrame->clusterId == END_DEVICE_ANNOUNCE) {
    if ((message[DEVICE_ANNOUNCE_CAPABILITIES_OFFSET] & RECEIVER_ON_WHEN_IDLE) == 0) {
      //fprintf(stderr,"Ignoring sleep end device");
      return true;
    }

    //Dropping nodes that aren't in our list. We have probably already sent a permit Joins Request
    //because we didn't hear a Device Announce within the designated time.
    if (findDeviceInJoiningList(sl_zigbee_node_id) == SL_ZIGBEE_NULL_NODE_ID) {
      return true;
    }

    sendPermitJoinsRequest(sl_zigbee_node_id);
    // fprintf(stderr,"\r\nDevice Announce heard,Sending Join Request %u\r\n",sl_zigbee_node_id);
    removeDeviceFromJoiningList(sl_zigbee_node_id);
    //Doesn't matter if we can't Unicast the request.
    //TODO: investigate if it should matter.  The original comment doesn't say why.
    return true;
  }
  return false;
}

void sl_zigbee_af_permit_join_manager_timeout_network_event_handler(sl_zigbee_af_event_t * event)
{
  sl_zigbee_af_event_set_inactive(sl_zigbee_af_permit_join_manager_timeout_network_events);
  int i = 0;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_JOINING_DEVICES_QUEUE_LENGTH; i++) {
    if (joiningDeviceList[i].sl_zigbee_node_id != SL_ZIGBEE_NULL_NODE_ID
        && (elapsedTimeInt32u(joiningDeviceList[i].timeStamp, halCommonGetInt32uMillisecondTick())
            >= SL_ZIGBEE_AF_PLUGIN_PERMIT_JOIN_MANAGER_JOINING_DEVICES_QUEUE_LENGTH)) {
      //TODO: should we check the return value?
      sendPermitJoinsRequest(joiningDeviceList[i].sl_zigbee_node_id);
      removeDeviceFromJoiningList(joiningDeviceList[i].sl_zigbee_node_id);
      return;
    }
  }
}

void sl_zigbee_af_permit_join_manager_trust_center_join_cb(sl_802154_short_addr_t newNodeId,
                                                           sl_802154_long_addr_t newNodeEui64,
                                                           sl_802154_short_addr_t parentOfNewNode,
                                                           sl_zigbee_device_update_t status,
                                                           sl_zigbee_join_decision_t decision)
{
  if (decision == SL_ZIGBEE_DENY_JOIN) {
    sl_zigbee_af_core_println("Join denied, not queueing for permit joining %u", newNodeId);
  } else if (decision == SL_ZIGBEE_USE_PRECONFIGURED_KEY
             || decision == SL_ZIGBEE_SEND_KEY_IN_THE_CLEAR) {
    sl_zigbee_af_core_println("Adding device to the queue");
    addDeviceToJoiningList(newNodeId);
  } // else (decision == SL_ZIGBEE_NO_ACTION)
}
