/***************************************************************************//**
 * @file
 * @brief This file provides a function set for initiating ez-mode commissioning
 * as both a client and a server.
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

// *****************************************************************************
// * ez-mode.c
// *
// *
// *
// * Copyright 2013 Silicon Laboratories, Inc.                              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "stack/include/zigbee-device-stack.h"
#include "ez-mode.h"

//------------------------------------------------------------------------------
// Forward Declaration

#include "ezmode-commissioning-config.h"
static sl_zigbee_af_event_t stateEvent;
static void stateEventHandler(sl_zigbee_af_event_t * event);
static void serviceDiscoveryCallback(const sl_zigbee_af_service_discovery_result_t *result);
static void createBinding(uint8_t *address);

//------------------------------------------------------------------------------
// Globals

static uint8_t currentIdentifyingEndpoint;
static sl_802154_short_addr_t currentIdentifyingAddress;

static uint8_t ezmodeClientEndpoint;

static sl_zigbee_af_ez_mode_commissioning_direction_t bindingDirection;
static const uint16_t* clusterIdsForEzModeMatch;
static uint8_t clusterIdsForEzModeMatchLength;
static uint16_t ezmodeClientCluster;
static uint8_t bindingIndex;
static uint8_t networkIndex;

typedef enum {
  EZMODE_OFF,
  EZMODE_BROAD_PJOIN,
  EZMODE_IDENTIFY,
  EZMODE_IDENTIFY_WAIT,
  EZMODE_MATCH,
  EZMODE_BIND,
  EZMODE_BOUND
} sli_zigbee_ez_mode_state_t;

static sli_zigbee_ez_mode_state_t ezModeState = EZMODE_OFF;

//------------------------------------------------------------------------------

static void complete(void)
{
  sl_zigbee_af_ezmode_commissioning_client_complete_cb(bindingIndex);
  ezModeState = EZMODE_OFF;
}

static void identifyRequestMessageSentCallback(sl_zigbee_outgoing_message_type_t type,
                                               uint16_t indexOrDestination,
                                               sl_zigbee_aps_frame_t *apsFrame,
                                               uint16_t msgLen,
                                               uint8_t *message,
                                               sl_status_t status)
{
  UNUSED_VAR(type);
  UNUSED_VAR(indexOrDestination);
  UNUSED_VAR(apsFrame);
  UNUSED_VAR(msgLen);
  UNUSED_VAR(message);
  UNUSED_VAR(status);
  // We only care about cleaning up state machine if still in Identify state
  // If we've moved on to Match state, don't do these things as they will abort
  // the state machine prematurely.
  if (ezModeState == EZMODE_IDENTIFY ) {
    if (status == SL_STATUS_OK) {
      ezModeState = EZMODE_IDENTIFY_WAIT;
      sl_zigbee_af_event_set_delay_ms(&stateEvent,
                                      (10 * MILLISECOND_TICKS_PER_SECOND));
    } else {
      complete();
    }
  }
}

void sli_zigbee_af_ezmode_commissioning_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_event_init(&stateEvent,
                          stateEventHandler);
}
static void stateEventHandler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);

  sl_status_t status;
  sl_802154_long_addr_t add;

  if (sl_zigbee_af_push_network_index(networkIndex) != SL_STATUS_OK) {
    complete();
    return;
  }

  sl_zigbee_af_event_set_inactive(&stateEvent);
  switch (ezModeState) {
    case EZMODE_BROAD_PJOIN:
      sl_zigbee_af_core_println("<ezmode bpjoin>");
      sli_zigbee_af_permit_join(180, true); //Send out a broadcast pjoin
      ezModeState = EZMODE_IDENTIFY;
      sl_zigbee_af_event_set_delay_ms(&stateEvent, MILLISECOND_TICKS_PER_SECOND);
      break;
    case EZMODE_IDENTIFY:
      sl_zigbee_af_core_println("<ezmode identify>");
      sli_zigbee_af_permit_join(180, true); //Send out a broadcast pjoin
      sl_zigbee_af_fill_command_identify_cluster_identify_query();
      sl_zigbee_af_set_command_endpoints(ezmodeClientEndpoint,
                                         SL_ZIGBEE_BROADCAST_ENDPOINT);
      status = sl_zigbee_af_send_command_broadcast_with_cb(SL_ZIGBEE_SLEEPY_BROADCAST_ADDRESS,
                                                           SL_ZIGBEE_NULL_NODE_ID,
                                                           0,
                                                           identifyRequestMessageSentCallback);
      if (status != SL_STATUS_OK) {
        complete();
      }
      break;
    case EZMODE_IDENTIFY_WAIT:
      sl_zigbee_af_core_println("<ezmode identify timeout>");
      complete();
      break;
    case EZMODE_MATCH:
      sl_zigbee_af_core_println("<ezmode match>");
      status = sl_zigbee_af_find_clusters_by_device_and_endpoint(currentIdentifyingAddress,
                                                                 currentIdentifyingEndpoint,
                                                                 serviceDiscoveryCallback);
      if (status != SL_STATUS_OK) {
        complete();
      }
      break;
    case EZMODE_BIND:
      sl_zigbee_af_core_println("<ezmode bind>");
      status = sl_zigbee_lookup_eui64_by_node_id(currentIdentifyingAddress, add);
      if (status == SL_STATUS_OK) {
        createBinding(add);
      } else {
        status = sl_zigbee_af_find_ieee_address(currentIdentifyingAddress,
                                                serviceDiscoveryCallback);
        if (status != SL_STATUS_OK) {
          complete();
        }
      }
      break;
    case EZMODE_BOUND:
      sl_zigbee_af_core_println("<ezmode bound>");
      complete();
      break;
    default:
      break;
  }

  (void) sl_zigbee_af_pop_network_index();
}

/** EZ-MODE CLIENT **/
/**
 * Kicks off the ezmode commissioning process by sending out
 * an identify query command to the given endpoint
 *
 * input:
 *   endpoint:
 *   direction: server to client / client to server
 *   clusterIds: list of cluster ids. *NOTE* The API only keeps the pointer to
 *     to the data structure. The data is expected to exist throughout the
 *     ezmode-commissioning calls.
 *   clusterIdsLength: # of ids defined in clusterIds
 */
sl_status_t sl_zigbee_af_ezmode_client_commission(uint8_t endpoint,
                                                  sl_zigbee_af_ez_mode_commissioning_direction_t direction,
                                                  const uint16_t* clusterIds,
                                                  uint8_t  clusterIdsLength)
{
  // sanity check inputs...
  if (!clusterIds) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  bindingIndex = SL_ZIGBEE_NULL_BINDING;
  bindingDirection = direction;
  clusterIdsForEzModeMatchLength = clusterIdsLength;

  if (clusterIdsLength > 0) {
    clusterIdsForEzModeMatch = clusterIds;
  } else {
    clusterIdsForEzModeMatch = NULL;
  }

  ezmodeClientEndpoint = endpoint;
  ezModeState = EZMODE_BROAD_PJOIN;
  networkIndex = sl_zigbee_get_current_network();
  sl_zigbee_af_event_set_active(&stateEvent);
  return SL_STATUS_OK;
}

bool sl_zigbee_af_identify_cluster_identify_query_response_cb(uint16_t timeout)
{
  // ignore our own broadcast and only take the first identify
  // Note that we may still be in IDENTIFY state if someone responds to the broadast before it
  // is done getting echoed to its neighbors (to trigger Message Sent callback).
  if (sl_zigbee_af_get_node_id() != sl_zigbee_af_current_command()->source) {
    if ((ezModeState == EZMODE_IDENTIFY_WAIT || ezModeState == EZMODE_IDENTIFY)
        && timeout != 0) {
      currentIdentifyingAddress = sl_zigbee_af_current_command()->source;
      currentIdentifyingEndpoint = sl_zigbee_af_current_command()->apsFrame->sourceEndpoint;
      ezModeState = EZMODE_MATCH;
      sl_zigbee_af_event_set_active(&stateEvent);
    }
    sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_SUCCESS);
  }
  return true;
}

static void createBinding(uint8_t *address)
{
  // create binding
  uint8_t i;
  sl_zigbee_binding_table_entry_t candidate;

  // first look for a duplicate binding, we should not add duplicates
  for (i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
    if (sl_zigbee_get_binding(i, &candidate) == SL_STATUS_OK
        && candidate.type == SL_ZIGBEE_UNICAST_BINDING
        && candidate.local == ezmodeClientEndpoint
        && candidate.clusterId == ezmodeClientCluster
        && candidate.remote == currentIdentifyingEndpoint
        && memcmp(candidate.identifier, address, EUI64_SIZE) == 0) {
      bindingIndex = i;
      ezModeState = EZMODE_BOUND;
      sl_zigbee_af_event_set_active(&stateEvent);
      return;
    }
  }

  for (i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
    if (sl_zigbee_get_binding(i, &candidate) == SL_STATUS_OK
        && candidate.type == SL_ZIGBEE_UNUSED_BINDING) {
      candidate.type = SL_ZIGBEE_UNICAST_BINDING;
      candidate.local = ezmodeClientEndpoint;
      candidate.remote = currentIdentifyingEndpoint;
      candidate.clusterId = ezmodeClientCluster;
      memmove(candidate.identifier, address, EUI64_SIZE);
      if (sl_zigbee_set_binding(i, &candidate) == SL_STATUS_OK) {
        sl_zigbee_set_binding_remote_node_id(i, currentIdentifyingAddress);
        bindingIndex = i;
        ezModeState = EZMODE_BOUND;
        sl_zigbee_af_event_set_active(&stateEvent);
        return;
      }
    }
  }

  complete();
}

static void serviceDiscoveryCallback(const sl_zigbee_af_service_discovery_result_t *result)
{
  uint8_t i = 0;
  uint8_t j = 0;
  if (sl_zigbee_af_have_discovery_response_status(result->status)) {
    if (result->zdoRequestClusterId == SIMPLE_DESCRIPTOR_REQUEST) {
      sl_zigbee_af_cluster_list_t *list = (sl_zigbee_af_cluster_list_t*)result->responseData;
      uint8_t clusterCount;
      const uint16_t* clusterList;

      // decide where to create the binding (server/client)
      if (bindingDirection == SL_ZIGBEE_AF_EZMODE_COMMISSIONING_CLIENT_TO_SERVER) {
        clusterCount = list->inClusterCount;
        clusterList = list->inClusterList;
      } else { // SL_ZIGBEE_AF_EZMODE_COMMISSIONING_SERVER_TO_CLIENT
        clusterCount = list->outClusterCount;
        clusterList = list->outClusterList;
      }

      for (i = 0; i < clusterCount; i++) {
        uint16_t cluster = clusterList[i];
        for (j = 0; j < clusterIdsForEzModeMatchLength; j++) {
          if (cluster == clusterIdsForEzModeMatch[j]) {
            ezmodeClientCluster = cluster;
            ezModeState = EZMODE_BIND;
            sl_zigbee_af_event_set_active(&stateEvent);
            return;
          }
        }
      }
    } else if (result->zdoRequestClusterId == IEEE_ADDRESS_REQUEST) {
      createBinding((uint8_t *)result->responseData);
      return;
    }
  }
  complete();
}

/** EZ-MODE SERVER **/
/**
 * Puts the device into identify mode for the given endpoint
 * this is all that an ezmode server is responsible for
 */
sl_status_t sl_zigbee_af_ezmode_server_commission(uint8_t endpoint)
{
  return sl_zigbee_af_ezmode_server_commission_with_timeout(endpoint, SL_ZIGBEE_AF_PLUGIN_EZMODE_COMMISSIONING_IDENTIFY_TIMEOUT);
}

sl_status_t sl_zigbee_af_ezmode_server_commission_with_timeout(uint8_t endpoint, uint16_t identifyTimeoutSeconds)
{
  sl_zigbee_af_status_t afStatus;
  if ((identifyTimeoutSeconds < 1) || (identifyTimeoutSeconds > 254)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  afStatus =  sl_zigbee_af_write_attribute(endpoint,
                                           ZCL_IDENTIFY_CLUSTER_ID,
                                           ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                                           CLUSTER_MASK_SERVER,
                                           (uint8_t *)&identifyTimeoutSeconds,
                                           ZCL_INT16U_ATTRIBUTE_TYPE);
  if (afStatus != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_STATUS_INVALID_PARAMETER;
  } else {
    return SL_STATUS_OK;
  }
}
