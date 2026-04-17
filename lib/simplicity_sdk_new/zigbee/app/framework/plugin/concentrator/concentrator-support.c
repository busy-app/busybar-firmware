/***************************************************************************//**
 * @file
 * @brief Code common to SOC and host to handle periodically broadcasting
 * many-to-one route requests (MTORRs).
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
// * concentrator-support.c
// *
// * Code common to SOC and host to handle periodically broadcasting
// * many-to-one route requests (MTORRs).
// *
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "stack/include/zigbee-device-stack.h"
#include "concentrator-support.h"
#include "stack/include/source-route.h"

// *****************************************************************************
// Globals

#include "concentrator-config.h"
// TODO: It appears that the "update" event is not used at all in the code.

#define MIN_QS (SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_MIN_TIME_BETWEEN_BROADCASTS_SECONDS << 2)
#define MAX_QS (SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_MAX_TIME_BETWEEN_BROADCASTS_SECONDS << 2)

#if (MIN_QS > MAX_QS)
  #error "Minimum broadcast time must be less than max (EMBER_PLUGIN_CONCENTRATOR_MIN_TIME_BETWEEN_BROADCASTS_SECONDS < EMBER_PLUGIN_CONCENTRATOR_MAX_TIME_BETWEEN_BROADCASTS_SECONDS)"
#endif

// Handy values to make the code more readable.

typedef enum {
  USE_MIN_TIME = 0,
  USE_MAX_TIME = 1,
  USE_ADDRESS_DISCOVERY_TIME = 2,
} sli_zigbee_queue_delay_t;

#ifndef SL_ZIGBEE_AF_HAS_ROUTER_NETWORK
  #error "Concentrator support only allowed on routers and coordinators."
#endif

extern uint8_t sli_zigbee_supported_networks;

//This is used to store the sourceRouteOverhead to our last sender
//It defaults to 0xFF if no valid sourceRoute is found. When available, it
//is used once to prevent the overhead of calling sl_zigbee_ezsp_get_source_route_overhead()
//and cleared subsequently.
#if defined(EZSP_HOST)
static sl_802154_short_addr_t targetIds[SL_ZIGBEE_SUPPORTED_NETWORKS];
#define targetId targetIds[sl_zigbee_get_current_network()]
static uint8_t sourceRouteOverheads[SL_ZIGBEE_SUPPORTED_NETWORKS];
#define sourceRouteOverhead sourceRouteOverheads[sl_zigbee_get_current_network()]
#endif

// EMINSIGHT-2484 - allow applications to set whether or not they want
// routers to send mtorrs.
sl_zigbee_af_plugin_concentrator_router_behavior_t sli_zigbee_af_concentrator_router_behaviors[SL_ZIGBEE_SUPPORTED_NETWORKS];

// *****************************************************************************
// Functions

void sl_zigbee_af_concentrator_init_cb(uint8_t init_level)
{
  (void)init_level;

  for (uint8_t i = 0; i < sli_zigbee_supported_networks; i++) {
#if defined(EZSP_HOST)
    targetIds[i] = SL_ZIGBEE_UNKNOWN_NODE_ID;
    sourceRouteOverheads[i] = SL_ZIGBEE_SOURCE_ROUTE_OVERHEAD_UNKNOWN;
#endif // EZSP_HOST
    sli_zigbee_af_concentrator_router_behaviors[i] = SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_DEFAULT_ROUTER_BEHAVIOR;
  }
}

uint32_t sl_zigbee_af_concentrator_queue_discovery(void)
{
  return sl_zigbee_set_source_route_discovery_mode(SL_ZIGBEE_SOURCE_ROUTE_DISCOVERY_RESCHEDULE);
}

sl_status_t sl_zigbee_af_concentrator_change_router_behavior(sl_zigbee_af_plugin_concentrator_router_behavior_t behavior)
{
  sl_zigbee_node_type_t nodeType;
  sl_status_t returnStatus = sl_zigbee_af_get_node_type(&nodeType);
  if (returnStatus == SL_STATUS_OK) {
    sl_status_t status;
    if (nodeType == SL_ZIGBEE_COORDINATOR && behavior == SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_ROUTER_BEHAVIOR_NONE) {
      return SL_STATUS_NOT_AVAILABLE;
    } else if (behavior == SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_ROUTER_BEHAVIOR_FULL) {
      status = sl_zigbee_set_concentrator(true,
                                          SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_CONCENTRATOR_TYPE,
                                          SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_MIN_TIME_BETWEEN_BROADCASTS_SECONDS,
                                          SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_MAX_TIME_BETWEEN_BROADCASTS_SECONDS,
                                          SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_ROUTE_ERROR_THRESHOLD,
                                          SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_DELIVERY_FAILURE_THRESHOLD,
                                          SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_MAX_HOPS
                                          );
    } else {
      status = sl_zigbee_set_concentrator(false,
                                          INACTIVE_CONCENTRATOR,
                                          SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_MIN_TIME_BETWEEN_BROADCASTS_SECONDS,
                                          SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_MAX_TIME_BETWEEN_BROADCASTS_SECONDS,
                                          SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_ROUTE_ERROR_THRESHOLD,
                                          SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_DELIVERY_FAILURE_THRESHOLD,
                                          SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_MAX_HOPS
                                          );
    }
    if (status == SL_STATUS_OK) {
      sli_zigbee_af_concentrator_router_behavior = behavior;
    } else {
      returnStatus = status;
    }
  }
  return returnStatus;
}

void sl_zigbee_af_concentrator_message_sent_cb(sl_zigbee_outgoing_message_type_t type,
                                               uint16_t indexOrDestination,
                                               sl_zigbee_aps_frame_t *apsFrame,
                                               sl_status_t status,
                                               uint16_t messageLength,
                                               uint8_t *messageContents)
{
  (void)type;
  (void)indexOrDestination;
  (void)apsFrame;
  (void)status;
  (void)messageLength;
  (void)messageContents;
  // Handling of message sent failure is now done in the source route library.
  // the callback however, is left here in case there is still consumer side code depening on it
}

// We only store one valid overhead for one destination. We don't want to overwrite that with
// an invalid source route to another destination. We do however want to invalidate an
// overhead to our destination if it is now unknown.
void sl_zigbee_af_set_source_route_overhead_cb(sl_802154_short_addr_t destination, uint8_t overhead)
{
  (void)overhead;
  #if defined(EZSP_HOST)
  (void)sl_zigbee_set_current_network(sl_zigbee_get_callback_network());

  if (!(destination != targetId && overhead == SL_ZIGBEE_SOURCE_ROUTE_OVERHEAD_UNKNOWN)) {
    targetId = destination;
    sourceRouteOverhead = overhead;
  }
  #else
  (void)destination;
  #endif
}

// In an effort to reduce the traffic between the host and NCP, for each incoming message,
// the sourceRouteOverhead to that particular destination is sent from the NCP to the host
// as a part of the incomingMessageHandler(). This information is cached and can be used
// once to calculate the MaximumPayload() to that same destination. It is invalidated after
// one use.
uint8_t sl_zigbee_af_get_source_route_overhead_cb(sl_802154_short_addr_t destination)
{
#if defined(EZSP_HOST)
  // While this function is named .*Callback, it is not always called in the
  // context of an app framework callback, so we do not need to call
  // sl_zigbee_set_current_network(sl_zigbee_get_callback_network()) before referencing
  // targetId and sourceRouteOverhead

  if (targetId == destination && sourceRouteOverhead != SL_ZIGBEE_SOURCE_ROUTE_OVERHEAD_UNKNOWN) {
    sl_zigbee_af_debug_println("ValidSourceRouteFound %u ", sourceRouteOverhead);
    return sourceRouteOverhead;
  } else {
    return sl_zigbee_ezsp_get_source_route_overhead(destination);
  }
#else
  return sl_zigbee_get_source_route_overhead(destination);
#endif
}

void sl_zigbee_af_concentrator_stack_status_cb(sl_status_t status)
{
  (void)sl_zigbee_set_current_network(sl_zigbee_get_callback_network());

  sl_zigbee_node_type_t nodeType;
  if (status == SL_STATUS_NETWORK_DOWN
      && !sl_zigbee_is_performing_rejoin()) {
    //now we clear/init the source route table everytime the network is up , therefore we do clear the source route table on rejoin.
  } else if (status == SL_STATUS_NETWORK_UP
             && ((sl_zigbee_af_concentrator_get_router_behavior() == SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_ROUTER_BEHAVIOR_FULL)
                 || (sl_zigbee_af_get_node_type(&nodeType) == SL_STATUS_OK
                     && nodeType == SL_ZIGBEE_COORDINATOR))) {
    sli_zigbee_af_concentrator_router_behavior = SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_ROUTER_BEHAVIOR_FULL;
    sl_zigbee_af_concentrator_change_router_behavior(SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_ROUTER_BEHAVIOR_FULL);
  }
}
