/***************************************************************************//**
 * @file
 * @brief Initiator routines for the Find and Bind Initiator plugin.
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

#include "app/framework/include/af.h"

#include "app/framework/util/af-main.h" // sl_zigbee_af_get_binding_table_size
#ifndef SL_ZIGBEE_SCRIPTED_TEST
#include "find-and-bind-initiator-config.h"
#endif

#include "zap-cluster-command-parser.h"

#include "find-and-bind-initiator.h"

//#define EM_AF_PLUGIN_FIND_AND_BIND_INITIATOR_DEBUG
#ifdef  EM_AF_PLUGIN_FIND_AND_BIND_INITIATOR_DEBUG
  #ifdef SL_ZIGBEE_SCRIPTED_TEST
    #define debugPrintln(...) simPrint(__VA_ARGS__)
  #else
    #define debugPrintln(...) sl_zigbee_af_core_println(__VA_ARGS__)
  #endif
#else
  #define debugPrintln(...)
#endif /* EM_AF_PLUGIN_FIND_AND_BIND_DEBUG */

#ifdef SL_ZIGBEE_SCRIPTED_TEST
  #include "find-and-bind-test-configuration.h"
#endif

// -----------------------------------------------------------------------------
// Constants

#define INVALID_TARGET_RESPONSES_INDEX SL_ZIGBEE_AF_PLUGIN_FIND_AND_BIND_INITIATOR_TARGET_RESPONSES_COUNT

#ifdef ZCL_GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH
  #define GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH ZCL_GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH
#else
  #define GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH (16)
#endif

#define GROUPS_CLUSTER_INVALID_GROUP_ID (0xFFFF)

// -----------------------------------------------------------------------------
// Globals

static sl_zigbee_af_event_t checkTargetResponsesEvent;

typedef struct {
  sl_802154_short_addr_t nodeId;
  uint8_t endpoint;
} sli_zigbee_find_and_bind_target_info_t;

static sli_zigbee_find_and_bind_target_info_t targetResponses[SL_ZIGBEE_AF_PLUGIN_FIND_AND_BIND_INITIATOR_TARGET_RESPONSES_COUNT];
static uint8_t targetResponsesReceived = 0;
static uint8_t currentTargetResponsesIndex = INVALID_TARGET_RESPONSES_INDEX;
static sl_802154_long_addr_t currentTargetResponseIeeeAddr;

static uint8_t initiatorEndpoint = SL_ZIGBEE_AF_INVALID_ENDPOINT;

// -----------------------------------------------------------------------------
// Private API Prototypes

/* state */
static uint8_t state = 0;
enum {
  STATE_NONE    = 0x00,
  FIND_TARGETS  = 0x01,
  IEEE_REQUEST  = 0x02,
  DESCR_REQUEST = 0x03,
};

/* state machine */
static sl_status_t broadcastIdentifyQuery(void);

static sl_status_t sendIeeeAddrRequest(void);
static void handleIeeeAddrResponse(const sl_zigbee_af_service_discovery_result_t *result);

static sl_status_t sendSimpleDescriptorRequest(void);
static void handleSimpleDescriptorResponse(const sl_zigbee_af_service_discovery_result_t *result);
static void processClusterList(bool clientList,
                               uint8_t clusterCount,
                               const sl_zigbee_af_cluster_id_t *clusters,
                               sl_status_t *status);

static sl_status_t writeSimpleDescriptorResponse(sl_zigbee_binding_table_entry_t *entry,
                                                 uint8_t *groupName);

static void cleanupAndStop(sl_status_t status);

/* target responses */
// get the next index of a response
static void targetResponsesGetNext();

#define targetInfosAreEqual(targetInfo1, targetInfo2) \
  ((targetInfo1).nodeId == (targetInfo2).nodeId       \
   && (targetInfo1).endpoint == (targetInfo2).endpoint)

#define currentTargetInfoIsNull() \
  (currentTargetResponsesIndex == INVALID_TARGET_RESPONSES_INDEX)

#define currentTargetInfoNodeId \
  (targetResponses[currentTargetResponsesIndex].nodeId)
#define currentTargetInfoEndpoint \
  (targetResponses[currentTargetResponsesIndex].endpoint)
#define currentTargetInfoIeeeAddr \
  (currentTargetResponseIeeeAddr)

// -----------------------------------------------------------------------------
// Public API

sl_status_t sl_zigbee_af_find_and_bind_initiator_start(uint8_t endpoint)
{
  sl_status_t status;

  initiatorEndpoint = endpoint;

  state = STATE_NONE;
  targetResponsesReceived = 0;

  status = broadcastIdentifyQuery();
  if (status == SL_STATUS_OK) {
    sl_zigbee_af_event_set_delay_ms(&checkTargetResponsesEvent,
                                    SL_ZIGBEE_AF_PLUGIN_FIND_AND_BIND_INITIATOR_TARGET_RESPONSES_DELAY_MS);
    state = FIND_TARGETS;
  } else {
    cleanupAndStop(status);
  }

  return status;
}

static void checkTargetResponsesEventHandler(sl_zigbee_af_event_t * event)
{
  (void) event;
  sl_status_t status = SL_STATUS_OK;
  bool finished = false;

  sl_zigbee_af_event_set_inactive(&checkTargetResponsesEvent);

  switch (state) {
    case FIND_TARGETS:
      if (targetResponsesReceived) {
        state = IEEE_REQUEST;
        targetResponsesGetNext();
        if (currentTargetInfoIsNull()) {
          // If currentTargetInfo is NULL, then the set is "empty",
          // so we are done!
          finished = true;
          status = SL_STATUS_OK;
          break;
        }
      } else {
        // If we didn't get any responses to our query, die.
        status = SL_STATUS_FAIL;
        break;
      }

    // FALLTHROUGH

    case IEEE_REQUEST:
      status = sendIeeeAddrRequest();
      state = DESCR_REQUEST;
      break;

    case DESCR_REQUEST:
      status = sendSimpleDescriptorRequest();
      state = FIND_TARGETS;
      break;

    case STATE_NONE:
    default:
      debugPrintln("%s: %s: 0x%02X",
                   SL_ZIGBEE_AF_PLUGIN_FIND_AND_BIND_INITIATOR_PLUGIN_NAME,
                   "Bad state",
                   state);
      status = SL_STATUS_INVALID_PARAMETER;
  }

  if (status != SL_STATUS_OK || finished) {
    cleanupAndStop(status);
  }
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_identify_cluster_identify_query_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;

  sl_zigbee_af_cluster_command_t *currentCommand;
  sli_zigbee_find_and_bind_target_info_t targetInfo;
  uint8_t i;

// As of now, we don't use the timeout field in the command so no point to do
// any parsing at all.
#if 0
  sl_zcl_identify_cluster_identify_query_response_command_t cmd_data;
  if (zcl_decode_identify_cluster_identify_query_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }
#endif

  currentCommand = sl_zigbee_af_current_command();

  if (currentCommand->apsFrame->destinationEndpoint == initiatorEndpoint) {
    targetInfo.nodeId   = currentCommand->source;
    targetInfo.endpoint = currentCommand->apsFrame->sourceEndpoint;
    // If you haven't added the response yet, then add it now.
    for (i = 0;
         (i < targetResponsesReceived
          && !targetInfosAreEqual(targetInfo, targetResponses[i]));
         i++) {
      ; // pass
    }
    if (i < SL_ZIGBEE_AF_PLUGIN_FIND_AND_BIND_INITIATOR_TARGET_RESPONSES_COUNT
        && i == targetResponsesReceived) {
      targetResponses[i].nodeId = currentCommand->source;
      targetResponses[i].endpoint = currentCommand->apsFrame->sourceEndpoint;
      targetResponsesReceived++;
    }
  }

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

void sli_zigbee_af_find_and_bind_initiator_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_event_init(&checkTargetResponsesEvent,
                          checkTargetResponsesEventHandler);
}

// -----------------------------------------------------------------------------
// Target Responses Set (Private) API

static void targetResponsesGetNext()
{
  // If the current index if invalid, then set it back to 0. Else, increment it.
  if (currentTargetResponsesIndex == INVALID_TARGET_RESPONSES_INDEX) {
    currentTargetResponsesIndex = 0;
  } else {
    currentTargetResponsesIndex++;
  }

  // If you have looked at all the responses, set the responses index
  // to invalid.
  if (currentTargetResponsesIndex == targetResponsesReceived) {
    currentTargetResponsesIndex = INVALID_TARGET_RESPONSES_INDEX;
  }
}

// -----------------------------------------------------------------------------
// Private API

static sl_status_t broadcastIdentifyQuery(void)
{
  sl_status_t status;

  sl_zigbee_af_set_command_endpoints(initiatorEndpoint, SL_ZIGBEE_BROADCAST_ENDPOINT);
  sl_zigbee_af_fill_command_identify_cluster_identify_query();

  status // BDB wants 0xFFFF
    = sl_zigbee_af_send_command_broadcast(SL_ZIGBEE_SLEEPY_BROADCAST_ADDRESS, SL_ZIGBEE_NULL_NODE_ID, 0);

  debugPrintln("%s: %s: 0x%02X",
               SL_ZIGBEE_AF_PLUGIN_FIND_AND_BIND_INITIATOR_PLUGIN_NAME,
               "Broadcast",
               status);

  return status;
}

static sl_status_t sendIeeeAddrRequest(void)
{
  sl_status_t status;

  if (currentTargetInfoIsNull()) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = sl_zigbee_af_find_ieee_address(currentTargetInfoNodeId,
                                          handleIeeeAddrResponse);

  debugPrintln("%s: %s: 0x%02X",
               SL_ZIGBEE_AF_PLUGIN_FIND_AND_BIND_INITIATOR_PLUGIN_NAME,
               "Ieee request",
               status);

  return status;
}

static void handleIeeeAddrResponse(const sl_zigbee_af_service_discovery_result_t *result)
{
  debugPrintln("%s: %s: 0x%02X",
               SL_ZIGBEE_AF_PLUGIN_FIND_AND_BIND_INITIATOR_PLUGIN_NAME,
               "Ieee response",
               result->status);

  if (currentTargetInfoIsNull()
      || (result->status
          != SL_ZIGBEE_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE)) {
    cleanupAndStop(SL_STATUS_FAIL);
    return;
  }

  memcpy(currentTargetInfoIeeeAddr,
         result->responseData,
         EUI64_SIZE);

  sl_zigbee_af_event_set_active(&checkTargetResponsesEvent);
}

static sl_status_t sendSimpleDescriptorRequest()
{
  sl_status_t status;

  status = sl_zigbee_af_find_clusters_by_device_and_endpoint(currentTargetInfoNodeId,
                                                             currentTargetInfoEndpoint,
                                                             handleSimpleDescriptorResponse);

  debugPrintln("%s: %s: 0x%02X",
               SL_ZIGBEE_AF_PLUGIN_FIND_AND_BIND_INITIATOR_PLUGIN_NAME,
               "Descriptor request",
               status);

  return status;
}

static void handleSimpleDescriptorResponse(const sl_zigbee_af_service_discovery_result_t *result)
{
  sl_status_t status = SL_STATUS_OK;
  sl_zigbee_af_cluster_list_t *clusterList = (sl_zigbee_af_cluster_list_t *)(result->responseData);

  debugPrintln("%s: %s: 0x%02X",
               SL_ZIGBEE_AF_PLUGIN_FIND_AND_BIND_INITIATOR_PLUGIN_NAME,
               "Descriptor response",
               result->status);

  if (currentTargetInfoIsNull()
      || (result->status
          != SL_ZIGBEE_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE)
      || !clusterList) {
    cleanupAndStop(SL_STATUS_FAIL);
    return;
  }

  processClusterList(false, // server list
                     clusterList->inClusterCount,
                     clusterList->inClusterList,
                     &status);
  processClusterList(true,  // client list
                     clusterList->outClusterCount,
                     clusterList->outClusterList,
                     &status);

  sl_zigbee_af_event_set_active(&checkTargetResponsesEvent);
}

static void processClusterList(bool clientList,
                               uint8_t clusterCount,
                               const sl_zigbee_af_cluster_id_t *clusters,
                               sl_status_t *status)
{
  uint8_t i;
  sl_zigbee_binding_table_entry_t bindingEntry;
  uint8_t groupName[GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH + 1];

  memset(groupName, 0x00, sizeof(groupName));

  for (i = 0; (*status != SL_STATUS_FULL) && (i < clusterCount); i++) {
    bindingEntry.type = SL_ZIGBEE_UNICAST_BINDING;
    bindingEntry.local = initiatorEndpoint;
    bindingEntry.clusterId = clusters[i];
    bindingEntry.remote = currentTargetInfoEndpoint;
    memmove(bindingEntry.identifier, currentTargetInfoIeeeAddr, EUI64_SIZE);
    bindingEntry.networkIndex = sl_zigbee_af_network_index_from_endpoint(initiatorEndpoint);
    if (((clientList && sl_zigbee_af_contains_server(initiatorEndpoint, clusters[i]))
         || (!clientList && sl_zigbee_af_contains_client(initiatorEndpoint, clusters[i])))
        && sl_zigbee_af_find_and_bind_initiator_bind_target_cb(currentTargetInfoNodeId,
                                                               &bindingEntry,
                                                               groupName)) {
      *status = writeSimpleDescriptorResponse(&bindingEntry, groupName);
      debugPrintln("%s: write cluster 0x%04X: 0x%02X",
                   SL_ZIGBEE_AF_PLUGIN_FIND_AND_BIND_INITIATOR_PLUGIN_NAME,
                   clusters[i],
                   *status);
    }
  }
}

static sl_status_t writeSimpleDescriptorResponse(sl_zigbee_binding_table_entry_t *newEntry,
                                                 uint8_t *groupName)
{
  sl_status_t status = SL_STATUS_FULL;
  sl_zigbee_binding_table_entry_t oldEntry;
  uint8_t i, goodIndex, bindingTableSize;

  bindingTableSize = sl_zigbee_af_get_binding_table_size();
  for (i = 0, goodIndex = bindingTableSize; i < bindingTableSize; i++) {
    if (sl_zigbee_get_binding(i, &oldEntry) == SL_STATUS_OK) {
      if (oldEntry.type == SL_ZIGBEE_UNUSED_BINDING) {
        if (goodIndex == bindingTableSize) {
          // This is a good index to write a binding.
          goodIndex = i;
        }
      } else {
        // Have we already written this binding?
        if (oldEntry.clusterId == newEntry->clusterId
            && oldEntry.remote == newEntry->remote
            && oldEntry.type == newEntry->type
            && !memcmp(oldEntry.identifier,
                       newEntry->identifier,
                       (oldEntry.type == SL_ZIGBEE_MULTICAST_BINDING
                        ? 2     // group id
                        : EUI64_SIZE))) {
          status = SL_STATUS_OK;
        }
      }
    }
  }

  // If we found a good index where we can write a binding...
  if (goodIndex < bindingTableSize) {
    // ...then if the application wanted us to write a multicast binding, and
    // we haven't sent one to this target yet, then we do so now...
    if (newEntry->type == SL_ZIGBEE_MULTICAST_BINDING) {
      uint16_t groupId
        = HIGH_LOW_TO_INT(newEntry->identifier[1], newEntry->identifier[0]);
      sl_zigbee_af_fill_command_groups_cluster_add_group(groupId, groupName);
      sl_zigbee_af_set_command_endpoints(initiatorEndpoint, currentTargetInfoEndpoint);
      sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, currentTargetInfoNodeId);
    }

    // ...and write the binding for this target if we have not done so already.
    if (status != SL_STATUS_OK) {
      status = sl_zigbee_set_binding(goodIndex, newEntry);
      sl_zigbee_set_binding_remote_node_id(goodIndex, currentTargetInfoNodeId);
    }
  }

  return status;
}

static void cleanupAndStop(sl_status_t status)
{
  debugPrintln("%s: Stop. Status: 0x%02X. State: 0x%02X",
               SL_ZIGBEE_AF_PLUGIN_FIND_AND_BIND_INITIATOR_PLUGIN_NAME,
               status,
               state);

  sl_zigbee_af_find_and_bind_initiator_complete_cb(status);
}

uint32_t sl_zigbee_af_identify_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                            sl_service_function_context_t *context)
{
  (void)opcode;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  if (!cmd->mfgSpecific && cmd->commandId == ZCL_IDENTIFY_QUERY_RESPONSE_COMMAND_ID) {
    status = sl_zigbee_af_identify_cluster_identify_query_response_cb(cmd);
  }

  return status;
}
