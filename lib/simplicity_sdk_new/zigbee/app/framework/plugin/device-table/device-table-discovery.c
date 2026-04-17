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
#else
  #include "stack/include/sl_zigbee.h"
#endif

#include "hal/hal.h"
#include "app/framework/include/af.h"

#include <stdlib.h>
#include "app/framework/plugin/device-table/device-table.h"
#include "app/framework/plugin/device-table/device-table-internal.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"

// Device Discovery declaractions
#define EVENT_TICK_MS   50
#define PJOIN_BROADCAST_PERIOD 1

// Number of times to attempt device discovery messages
#define MAX_RELAY_COUNT 10

//definitions for task queues
#define DEVICE_QUEUE_NULL 0xFF
#define MAX_QUEUE_SIZE 128

#define DEVICE_DISCOVERY_ENDPONT_RETRY_TIME_QS   4
#define DEVICE_DISCOVERY_SIMPLE_RETRY_TIME_QS    4
#define DEVICE_DISCOVERY_MAX_WAIT_TIME_S       120

#define DEVICE_DISCOVERY_MAX_WAIT_TICKS \
  ((DEVICE_DISCOVERY_MAX_WAIT_TIME_S * 1000) / EVENT_TICK_MS)

#define DEVICE_DISCOVERY_ENDPOINT_RETRY_TICKS \
  ((DEVICE_DISCOVERY_ENDPONT_RETRY_TIME_QS * 250) / EVENT_TICK_MS)
#define DEVICE_DISCOVERY_SIMPLE_RETRY_TICKS \
  ((DEVICE_DISCOVERY_SIMPLE_RETRY_TIME_QS * 250) / EVENT_TICK_MS)

#define APS_OPTION_DISCOVER SL_ZIGBEE_APS_OPTION_RETRY

// ZDO offsets
#define END_DEVICE_ANNOUNCE_IEEE_OFFSET 3
#define NETWORK_ADDRESS_RESPONSE_IEEE_OFFSET 2
#define NETWORK_ADDRESS_RESPONSE_NODE_ID_OFFSET 10 // 1-status, 8-ieee addr
#define IEEE_ADDRESS_RESPONSE_IEEE_OFFSET 2
#define IEEE_ADDRESS_RESPONSE_NODE_ID_OFFSET 10

// in out cluster types
#define CLUSTER_IN               0
#define CLUSTER_OUT              1
#define NUMBER_OF_CLUSTER_IN_OUT 2

//for simple descriptor response
#define SIMPLE_DESCRIPTOR_RESPONSE_ENDPOINT_OFFSET                           \
  (SL_ZIGBEE_AF_ZDO_RESPONSE_OVERHEAD                                        \
   + 2 /* address of interest */                      + 1 /* length value */ \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_PROFILE_ID_OFFSET \
  (SIMPLE_DESCRIPTOR_RESPONSE_ENDPOINT_OFFSET        \
   + 1 /* endpoint */                                \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET \
  (SIMPLE_DESCRIPTOR_RESPONSE_PROFILE_ID_OFFSET + 2)

#define SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX \
  (SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET                    \
   + 2  /* device ID length */                                    \
   + 1  /* version (4-bits), reserved (4-bits) */                 \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_INDEX \
  (SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX + 1)

// For Active endpoint response
#define ACTIVE_ENDPOINT_RESPONSE_COUNT_OFFSET \
  (SL_ZIGBEE_AF_ZDO_RESPONSE_OVERHEAD         \
   + 2) // Address of Interest

#define ACTIVE_ENDPOINT_RESPONSE_NODE_ID_OFFSET \
  (SL_ZIGBEE_AF_ZDO_RESPONSE_OVERHEAD)

#define ACTIVE_ENDPOINT_RESPONSE_LIST_OFFSET \
  (ACTIVE_ENDPOINT_RESPONSE_NODE_ID_OFFSET   \
   + 3)

typedef struct {
  sl_802154_long_addr_t eui64;
  uint16_t nodeId;
  uint8_t endpoint;
  uint8_t state;
  uint16_t delay;
  uint32_t waitTime;
} sli_zigbee_device_table_queue_entry_t;

static uint8_t permitJoinBroadcastCounter = (PJOIN_BROADCAST_PERIOD - 1);
sl_zigbee_af_event_t sl_zigbee_af_device_table_new_device_event;
#define newDeviceEventControl (&sl_zigbee_af_device_table_new_device_event)
void sl_zigbee_af_device_table_new_device_event_handler(sl_zigbee_af_event_t * event);

enum {
  DEVICE_DISCOVERY_STATE_ENDPOINTS_SEND = 0x00,
  DEVICE_DISCOVERY_STATE_ENDPOINTS_WAITING = 0x01,
  DEVICE_DISCOVERY_STATE_SIMPLE_SEND = 0x02,
  DEVICE_DISCOVERY_STATE_SIMPLE_WAITING = 0x03,
};

#define STATUS_STRINGS_ARRAY_LENGTH 4
static const char * const statusStrings[] =
{
  "STANDARD_SECURITY_SECURED_REJOIN",
  "STANDARD_SECURITY_UNSECURED_JOIN",
  "DEVICE_LEFT",
  "STANDARD_SECURITY_UNSECURED_REJOIN",
};

sli_zigbee_device_table_queue_entry_t taskQueue[MAX_QUEUE_SIZE];
static uint8_t queueSize;

void sl_zigbee_af_device_table_index_added_cb(uint16_t index);

// Bookkeeping callbacks
void sli_zigbee_af_device_table_device_left_callback(sl_802154_long_addr_t newNodeEui64);

static uint8_t getQueueIndexFromNodeAndEndpoint(sli_zigbee_device_table_queue_entry_t * queue,
                                                uint16_t nodeId,
                                                uint8_t endpoint)
{
  uint8_t i;

  if ((queue == NULL) || (queueSize == 0)) {
    return DEVICE_QUEUE_NULL;
  }
  for (i = 0; i < queueSize; i++) {
    if ((queue[i].nodeId == nodeId) && (queue[i].endpoint == endpoint)) {
      return i;
    }
  }
  return DEVICE_QUEUE_NULL;
}

static void deleteQueueEntry(sli_zigbee_device_table_queue_entry_t * queue, uint8_t index)
{
  uint8_t i;

  if (queueSize == 0) {
    return;
  }

  for (i = index; i < queueSize; i++) {
    queue[i] = queue[i + 1];
  }
  queueSize--;
  queue[queueSize].nodeId = 0;
  queue[queueSize].state = 0;
  queue[queueSize].delay = 0;
  queue[queueSize].waitTime = 0;
  memset(queue[queueSize].eui64, 0, EUI64_SIZE);
}

static void bubbleSortQueue(sli_zigbee_device_table_queue_entry_t * queue)
{
  uint8_t passesIndex, stepsIndex;
  sli_zigbee_device_table_queue_entry_t swapDeviceEntry;

  if (queueSize == 0) {
    return;
  }
  for (passesIndex = 0; passesIndex < (queueSize - 1); passesIndex++) {
    for (stepsIndex = 0;
         stepsIndex < (queueSize - passesIndex - 1);
         stepsIndex++) {
      if (queue[stepsIndex].delay > queue[stepsIndex + 1].delay) {
        swapDeviceEntry = queue[stepsIndex];
        queue[stepsIndex] = queue[stepsIndex + 1];
        queue[stepsIndex + 1] = swapDeviceEntry;
      }
    }
  }
}

void printQueue(void)
{
  uint8_t i;

  sl_zigbee_af_core_println("index    node  ep  state  delay  wait");
  for (i = 0; i < queueSize; i++) {
    sl_zigbee_af_core_println("  %d   0x%04X  %d    %d     %d    %d",
                              i,
                              taskQueue[i].nodeId,
                              taskQueue[i].endpoint,
                              taskQueue[i].state,
                              taskQueue[i].delay,
                              taskQueue[i].waitTime);
  }
}

static void updateQueueEntry(sli_zigbee_device_table_queue_entry_t * queue,
                             uint8_t index,
                             uint8_t nextState,
                             uint8_t delay)
{
  queue[index].state = nextState;
  queue[index].delay = delay;
  bubbleSortQueue(queue);
}

static bool addQueueEntry(sli_zigbee_device_table_queue_entry_t * queue,
                          sli_zigbee_device_table_queue_entry_t * newEntryPtr)
{
  if (queueSize == MAX_QUEUE_SIZE) {
    return false;
  }

  queue[queueSize] = *newEntryPtr;
  queueSize++;
  bubbleSortQueue(queue);

  if (queueSize == 1) { // this is the first entry and the state machine is inactive
                        // we have to activated it.
    sl_zigbee_af_event_set_active(newDeviceEventControl);
  }
  return true;
}

static void updateTaskQueueByTick(void)
{
  uint8_t i = 0;

  while (i < queueSize) {
    taskQueue[i].waitTime++;
    if (taskQueue[i].delay != 0) {
      taskQueue[i].delay--;
    }
    if (taskQueue[i].waitTime > DEVICE_DISCOVERY_MAX_WAIT_TICKS) {
      deleteQueueEntry(taskQueue, i);
    } else {
      i++;
    }
  }
}

static void routeRepairReturn(const sl_zigbee_af_service_discovery_result_t* result)
{
  sl_zigbee_af_core_println("DISCOVERY ROUTE REPAIR RETURN RESULT: status=%d",
                            result->status);
}

// We have a new endpoint.
static void newEndpointDiscovered(sl_zigbee_af_plugin_device_table_entry_t *p_entry)
{
  // Figure out if we need to do anything, like write the CIE address to it.
  if (p_entry->deviceId == DEVICE_ID_IAS_ZONE) {
    // write IEEE address to CIE address location
    sli_zigbee_af_device_table_send_cie_address_write(p_entry->nodeId, p_entry->endpoint);
  }
  p_entry->state = SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_JOINED;
  // New device is set, time to make the callback to indicate a new device
  // has joined.
  sl_zigbee_af_device_table_new_device_cb(p_entry->eui64);
  sli_zigbee_af_device_table_save();
}

void sl_zigbee_af_device_table_new_device_event_handler(sl_zigbee_af_event_t * event)
{
  sli_zigbee_device_table_queue_entry_t * currentEntryPtr = &taskQueue[0];
  uint8_t taskQueueIndex, delay = 0;
  uint8_t nextState = 0;

  if ((currentEntryPtr->delay == 0) && (queueSize > 0)) {
    sl_zigbee_af_print_big_endian_eui64(currentEntryPtr->eui64);
    taskQueueIndex = getQueueIndexFromNodeAndEndpoint(taskQueue,
                                                      currentEntryPtr->nodeId,
                                                      currentEntryPtr->endpoint);
    switch (currentEntryPtr->state) {
      case DEVICE_DISCOVERY_STATE_ENDPOINTS_SEND:
        // send out active endpoints request.
        (void)sl_zigbee_active_endpoints_request(currentEntryPtr->nodeId,
                                                 APS_OPTION_DISCOVER);
        nextState = DEVICE_DISCOVERY_STATE_ENDPOINTS_WAITING;
        delay = DEVICE_DISCOVERY_ENDPOINT_RETRY_TICKS;
        break;
      case DEVICE_DISCOVERY_STATE_ENDPOINTS_WAITING:
        sl_zigbee_af_find_node_id(currentEntryPtr->eui64, routeRepairReturn);
        nextState = DEVICE_DISCOVERY_STATE_ENDPOINTS_SEND;
        delay = DEVICE_DISCOVERY_ENDPOINT_RETRY_TICKS;
        break;
      case DEVICE_DISCOVERY_STATE_SIMPLE_SEND:
        sl_zigbee_simple_descriptor_request(currentEntryPtr->nodeId,
                                            currentEntryPtr->endpoint,
                                            SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS);
        nextState = DEVICE_DISCOVERY_STATE_SIMPLE_WAITING;
        delay = DEVICE_DISCOVERY_SIMPLE_RETRY_TICKS;
        break;
      case DEVICE_DISCOVERY_STATE_SIMPLE_WAITING:
        sl_zigbee_af_find_node_id(currentEntryPtr->eui64, routeRepairReturn);
        nextState = DEVICE_DISCOVERY_STATE_SIMPLE_SEND;
        delay = DEVICE_DISCOVERY_SIMPLE_RETRY_TICKS;
        break;
      default:
        break;
    }
    updateQueueEntry(taskQueue, taskQueueIndex, nextState, delay);
  }
  if (queueSize > 0) {
    sl_zigbee_af_event_set_delay_ms(newDeviceEventControl, EVENT_TICK_MS);
    updateTaskQueueByTick();
  } else {
    sl_zigbee_af_event_set_inactive(newDeviceEventControl);
  }
}

static void newDeviceParseActiveEndpointsResponse(sl_802154_short_addr_t sl_zigbee_node_id,
                                                  sl_zigbee_aps_frame_t* apsFrame,
                                                  uint8_t* message,
                                                  uint16_t length)
{
  uint8_t i;
  sli_zigbee_device_table_queue_entry_t entry;

  // Make sure I have not used the redundant endpoint response
  // check if there is any active endpoint that is being processed
  i = getQueueIndexFromNodeAndEndpoint(taskQueue,
                                       sl_zigbee_node_id,
                                       DEVICE_TABLE_UNKNOWN_ENDPOINT);
  if (i != DEVICE_QUEUE_NULL) {
    memcpy(entry.eui64, taskQueue[i].eui64, EUI64_SIZE);
    deleteQueueEntry(taskQueue, i);//delete the ep initial task
    entry.nodeId = sl_zigbee_node_id;
    entry.state = DEVICE_DISCOVERY_STATE_SIMPLE_SEND;
    entry.delay = 0;
    entry.waitTime = 0;
    sl_zigbee_af_core_println("number of ep: %d",
                              message[ACTIVE_ENDPOINT_RESPONSE_COUNT_OFFSET]);
    for (i = 0; i < message[ACTIVE_ENDPOINT_RESPONSE_COUNT_OFFSET]; i++) {
      entry.endpoint = message[ACTIVE_ENDPOINT_RESPONSE_LIST_OFFSET + i];
      sl_zigbee_af_core_println("ep: %d", message[ACTIVE_ENDPOINT_RESPONSE_LIST_OFFSET + i]);
      addQueueEntry(taskQueue, &entry);
    }
  }
}

static void newDeviceParseSimpleDescriptorResponse(sl_802154_short_addr_t nodeId,
                                                   uint8_t* message,
                                                   uint16_t length)
{
  uint8_t endpoint;
  sl_zigbee_af_plugin_device_table_entry_t *pEntry;
  uint8_t clusterIndex = 0;
  uint16_t endpointIndex;
  uint8_t i, currentClusterType, ClusterCount;
  uint8_t msgArrayIndex = SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_INDEX;

  endpoint = message[SIMPLE_DESCRIPTOR_RESPONSE_ENDPOINT_OFFSET];

  //if we can find a entry in device table with same nodeId and ep, then update
  //it else we add a new one.
  endpointIndex = sl_zigbee_af_device_table_get_endpoint_from_node_id_and_endpoint(nodeId,
                                                                                   endpoint);
  if (endpointIndex == SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    endpointIndex = sli_zigbee_af_device_table_find_free_device_table_index();
    if (endpointIndex == SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
      // Error case... no more room in the index table
      sl_zigbee_af_core_println("Error: Device Table Full");
      return;
    }
    sl_zigbee_af_device_table_index_added_cb(endpointIndex);
  }
  pEntry = sl_zigbee_af_device_table_find_device_table_entry(endpointIndex);
  pEntry->deviceId =
    sl_util_fetch_low_high_int16u(message
                                  + SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET);

  ClusterCount = message[SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX];

  for (currentClusterType = CLUSTER_IN;
       currentClusterType < NUMBER_OF_CLUSTER_IN_OUT;
       currentClusterType++) {
    for (i = 0; i < ClusterCount; i++) {
      pEntry->clusterIds[clusterIndex] =
        HIGH_LOW_TO_INT(message[msgArrayIndex + 1], message[msgArrayIndex]);
      clusterIndex++;
      msgArrayIndex += 2; //advance the index by 2 bytes for each 16 bit
                          //cluster id
    }
    if (currentClusterType == CLUSTER_IN) {
      pEntry->clusterOutStartPosition = clusterIndex;
      // This is the output cluster count
      ClusterCount = message[msgArrayIndex++];
    }
  }

  i = getQueueIndexFromNodeAndEndpoint(taskQueue, nodeId, endpoint);
  if (i != DEVICE_QUEUE_NULL) {
    memcpy(pEntry->eui64, taskQueue[i].eui64, EUI64_SIZE);
    pEntry->nodeId = taskQueue[i].nodeId;
    pEntry->endpoint = taskQueue[i].endpoint;
    deleteQueueEntry(taskQueue, i);
    newEndpointDiscovered(pEntry);
  }
}

// two things have to be handled here:
// 1.) new device joined
// 2.) node Id has been changed.
void sl_zigbee_af_device_table_new_device_join_handler(sl_802154_short_addr_t newNodeId,
                                                       sl_802154_long_addr_t newNodeEui64)
{
  uint16_t deviceTableIndex = sl_zigbee_af_device_table_get_first_index_from_eui64(newNodeEui64);
  uint8_t i;
  sl_zigbee_af_plugin_device_table_entry_t *deviceTable = sl_zigbee_af_device_table_pointer();
  sli_zigbee_device_table_queue_entry_t entry;

  if (deviceTableIndex == SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    //search is there any old task in the taskQueue that matches the eui64,
    //then we should not do anything in that case
    for (i = 0; i < queueSize; i++) {
      if (sl_zigbee_af_device_table_match_eui64(taskQueue[i].eui64, newNodeEui64)) {
        deleteQueueEntry(taskQueue, i);
      }
    }
    //add new task
    entry.endpoint = DEVICE_TABLE_UNKNOWN_ENDPOINT;
    entry.nodeId = newNodeId;
    entry.state = DEVICE_DISCOVERY_STATE_ENDPOINTS_SEND;
    entry.delay = 0;
    entry.waitTime = 0;
    memcpy(entry.eui64, newNodeEui64, EUI64_SIZE);
    addQueueEntry(taskQueue, &entry);
  } else {
    // Is this a new node ID?
    if (newNodeId != deviceTable[deviceTableIndex].nodeId) {
      sl_zigbee_af_core_println("Node ID Change:  was %04X, is %04X",
                                deviceTable[deviceTableIndex].nodeId,
                                newNodeId);

      sli_zigbee_af_device_table_update_node_id(deviceTable[deviceTableIndex].nodeId,
                                                newNodeId);

      // Test code for failure to see leave request.
      uint16_t endpointIndex =
        sl_zigbee_af_device_table_get_endpoint_from_node_id_and_endpoint(
          deviceTable[deviceTableIndex].nodeId,
          deviceTable[deviceTableIndex].endpoint);

      if (endpointIndex == SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
        return;
      }

      // New device is set, time to make the callback to indicate a new device
      // has joined.
      sl_zigbee_af_device_table_rejoin_device_cb(deviceTable[deviceTableIndex].eui64);
      // Need to save when the node ID changes.
      sli_zigbee_af_device_table_save();
    }
  }
}

static void newDeviceLeftHandler(sl_802154_long_addr_t newNodeEui64)
{
  uint16_t index = sl_zigbee_af_device_table_get_first_index_from_eui64(newNodeEui64);

  if (index != SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    sli_zigbee_af_device_table_device_left_callback(newNodeEui64);
    sli_zigbee_af_device_table_delete_entry(index);
    // Save on Node Left
    sli_zigbee_af_device_table_save();
  }

  //search and delete any pending task in the taskQueue that matches the eui64
  for (index = 0; index < queueSize; index++) {
    if (sl_zigbee_af_device_table_match_eui64(taskQueue[index].eui64, newNodeEui64)) {
      deleteQueueEntry(taskQueue, index);
    }
  }
}

#if !(defined(SL_ZIGBEE_LEAF_STACK))
static sl_status_t broadcastPermitJoin(uint8_t duration)
{
  permitJoinBroadcastCounter++;
  sl_status_t status;

  if (permitJoinBroadcastCounter == PJOIN_BROADCAST_PERIOD) {
    status = sl_zigbee_permit_joining_request(SL_ZIGBEE_BROADCAST_ADDRESS, duration, 0, 0);
    permitJoinBroadcastCounter = 0;
  } else {
    status = 0;
  }

  return status;
}
#endif

void sli_zigbee_af_device_table_trust_center_join_callback(sl_802154_short_addr_t newNodeId,
                                                           sl_802154_long_addr_t newNodeEui64,
                                                           sl_zigbee_device_update_t status,
                                                           sl_802154_short_addr_t parentOfNewNode)
{
  uint8_t i;

  sl_zigbee_af_core_println("TC Join Callback %04X , status: %d",
                            newNodeId,
                            status);

  for (i = 0; i < 8; i++) {
    sl_zigbee_af_core_print("%02X",
                            newNodeEui64[7 - i]);
  }
  if (status < STATUS_STRINGS_ARRAY_LENGTH) {
    sl_zigbee_af_core_println(" %s", statusStrings[status]);
  } else {
    sl_zigbee_af_core_println(" %d", status);
  }

  switch (status) {
    case SL_ZIGBEE_STANDARD_SECURITY_UNSECURED_JOIN:
      // Broadcast permit joining to new router as it joins.
      broadcastPermitJoin(254);
      sl_zigbee_af_core_println("new device line %d", __LINE__);
      sl_zigbee_af_device_table_new_device_join_handler(newNodeId, newNodeEui64);
      break;
    case SL_ZIGBEE_DEVICE_LEFT:
      newDeviceLeftHandler(newNodeEui64);
      break;
    default:
      // If the device is in the left sent state, we want to send another
      // left message.
      if (sli_zigbee_af_device_table_should_device_leave(newNodeId)) {
        return;
      } else {
        sl_zigbee_af_core_println("new device line %d", __LINE__);
        sl_zigbee_af_device_table_new_device_join_handler(newNodeId, newNodeEui64);
      }
      break;
  }

  // If a new device did an unsecure join, we need to turn on permit joining,
  // as there may be more coming
  if (status == SL_ZIGBEE_STANDARD_SECURITY_UNSECURED_JOIN) {
    // Broadcast permit joining to new router as it joins.
    broadcastPermitJoin(254);
  }
}

/** @brief Pre ZDO Message Received
 *
 * This function passes the application an incoming ZDO message and gives the
 * appictation the opportunity to handle it. By default, this callback returns
 * false indicating that the incoming ZDO message has not been handled and
 * should be handled by the Application Framework.
 *
 * @param sl_zigbee_node_id   Ver.: always
 * @param apsFrame   Ver.: always
 * @param message   Ver.: always
 * @param length   Ver.: always
 */
bool sli_zigbee_af_device_table_pre_zdo_message_received(sl_802154_short_addr_t sl_zigbee_node_id,
                                                         sl_zigbee_aps_frame_t* apsFrame,
                                                         uint8_t* message,
                                                         uint16_t length)
{
  sl_802154_short_addr_t ieeeSourceNode, remoteDeviceNwkId;

  sl_zigbee_af_core_println("%04X:  ", sl_zigbee_node_id);
  switch (apsFrame->clusterId) {
    case ACTIVE_ENDPOINTS_RESPONSE:
      sl_zigbee_af_core_println("Active Endpoints Response");
      newDeviceParseActiveEndpointsResponse(sl_zigbee_node_id, apsFrame, message, length);
      return false;
      break;
    case SIMPLE_DESCRIPTOR_RESPONSE:
      sl_zigbee_af_core_println("Simple Descriptor Response");

      newDeviceParseSimpleDescriptorResponse(sl_zigbee_node_id,
                                             message,
                                             length);
      return false;
      break;
    case END_DEVICE_ANNOUNCE:
      // Any time an end device announces, we need to see if we have to update
      // the device handler.
      sl_zigbee_af_core_println("new device line %d", __LINE__);
      sl_zigbee_af_device_table_new_device_join_handler(sl_zigbee_node_id,
                                                        message + END_DEVICE_ANNOUNCE_IEEE_OFFSET);
      break;
    case PERMIT_JOINING_RESPONSE:
      break;
    case LEAVE_RESPONSE:
      break;
    case BIND_RESPONSE:
      break;
    case BINDING_TABLE_RESPONSE:
      break;
    case NETWORK_ADDRESS_RESPONSE:
      sl_zigbee_af_core_println("new device line %d", __LINE__);
      remoteDeviceNwkId =
        sl_util_fetch_low_high_int16u(message + NETWORK_ADDRESS_RESPONSE_NODE_ID_OFFSET);
      sl_zigbee_af_device_table_new_device_join_handler(remoteDeviceNwkId,
                                                        message + NETWORK_ADDRESS_RESPONSE_IEEE_OFFSET);
      break;
    case IEEE_ADDRESS_RESPONSE:
      sl_zigbee_af_core_println("new device line %d", __LINE__);
      ieeeSourceNode =
        sl_util_fetch_low_high_int16u(message + IEEE_ADDRESS_RESPONSE_NODE_ID_OFFSET);
      sl_zigbee_af_core_println("Ieee source node %04X", ieeeSourceNode);

      sl_zigbee_af_device_table_new_device_join_handler(ieeeSourceNode,
                                                        message + IEEE_ADDRESS_RESPONSE_IEEE_OFFSET);
      break;
    default:
      sl_zigbee_af_core_println("Untracked ZDO %04X", apsFrame->clusterId);
      break;
  }

  sl_zigbee_af_core_print("%04X ", sl_zigbee_node_id);
  sli_zigbee_af_device_table_print_buffer(message, length);

  return false;
}
