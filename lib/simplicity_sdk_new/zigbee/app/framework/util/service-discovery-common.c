/***************************************************************************//**
 * @file
 * @brief Service discovery code that is common to different types of service
 * discovery, e.g. match descriptor, NWK address lookup, and IEEE address
 * lookup.
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
#include "app/util/zigbee-framework/zigbee-device-common.h"
#ifdef EZSP_HOST
  #include "app/util/zigbee-framework/zigbee-device-host.h"
#endif
#include "service-discovery.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#if SL_ZIGBEE_SUPPORTED_NETWORKS > 4
  #error "Service discovery is limited to four networks."
#endif

#define MAX_SIMPLE_DESCRIPTOR_RESPONSE_PAYLOAD                           \
  (128u   /* 802.15.4 MTU */                                             \
   - 1u   /* PHY length byte */                                          \
   - 11u  /* MAC header 9 + 2 byte CRC */                                \
   - 8u   /* NWK header */                                               \
   - 18u  /* NWK security header + 4-byte MIC */                         \
   - 8u   /* APS header */                                               \
   - SL_ZIGBEE_AF_ZDO_RESPONSE_OVERHEAD                                  \
   - 3u   /* Active Endpoint Response overhead: Address(2), length(1) */ \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_ENDPOINT_OFFSET \
  (SL_ZIGBEE_AF_ZDO_RESPONSE_OVERHEAD              \
   + 2u /* address of interest */                  \
   + 1u /* length value */                         \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_PROFILE_ID_OFFSET \
  (SIMPLE_DESCRIPTOR_RESPONSE_ENDPOINT_OFFSET        \
   + 1u /* endpoint */                               \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET \
  (SIMPLE_DESCRIPTOR_RESPONSE_PROFILE_ID_OFFSET + 2u)

#define SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX \
  (SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET                    \
   + 2u  /* device ID length */                                   \
   + 1u  /* version (4-bits), reserved (4-bits) */                \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_INDEX \
  (SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX + 1u)

#define MAX_CLUSTERS_IN_ACTIVE_ENDPOINT_RESPONSE (MAX_SIMPLE_DESCRIPTOR_RESPONSE_PAYLOAD >> 1)

#define ACTIVE_ENDPOINT_RESPONSE_COUNT_OFFSET \
  (SL_ZIGBEE_AF_ZDO_RESPONSE_OVERHEAD         \
   + 2u) // Address of Interest

//==============================================================================
// Service discovery state machine
//
//   This code handles initiating a limited set of ZDO, receiving
//   the response and sending it back to the cluster or code element that
//   requested it.  Unfortunately the ZDO message does not have any distinct
//   identifiers that would allow us to determine what cluster/endpoint on our
//   local device initiated the request.  Therefore we can only allow one
//    outstanding request at a time.

sl_zigbee_af_event_t sli_zigbee_af_service_discovery_events[SL_ZIGBEE_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK][SL_ZIGBEE_SUPPORTED_NETWORKS];

typedef struct {
  bool active;
  sl_zigbee_af_service_discovery_callback_t *callback;
  // This will contain the target type: broadcast or unicast (high bit)
  // and the ZDO cluster ID of the request.  Since ZDO requests
  // clear the high bit (only responses use it), we can use that leftover bit
  // for something else.
  uint16_t requestData;
  // Tracking the sequence when sending out discovery requests to be able to get
  // the correct discovery state for the request when the response comes back.
  // If the state is not active then it can be considered uninitialized.
  uint8_t requestSequence;
  // tracking the state index in Service Discovery States
  uint8_t stateIndex;
  //tracking the network index in Service Discovery States
  uint8_t networkIndex;
} sli_zigbee_service_discovery_state_t;
static sli_zigbee_service_discovery_state_t states[SL_ZIGBEE_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK][SL_ZIGBEE_SUPPORTED_NETWORKS];
static uint8_t stateLimit = SL_ZIGBEE_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK;
static bool statesInitialized = false;
#define UNICAST_QUERY_BIT (0x8000u)
#define isUnicastQuery(state) (UNICAST_QUERY_BIT == ((state)->requestData & UNICAST_QUERY_BIT))
#define setUnicastQuery(state) ((state)->requestData |= UNICAST_QUERY_BIT)
#define getRequestCluster(state) ((state)->requestData & ~UNICAST_QUERY_BIT)
#define serviceDiscoveryInProgress(state) ((state)->active)

// seq. number (1), status (1), address (2), length (1)
#define MATCH_DESCRIPTOR_OVERHEAD               5u
#define MINIMUM_MATCH_DESCRIPTOR_SUCCESS_LENGTH MATCH_DESCRIPTOR_OVERHEAD

// seq. number (1), status (1)
#define SL_ZIGBEE_AF_ZDO_RESPONSE_OVERHEAD 2u
// EUI64 (8), node ID (2),
#define MINIMUM_ADDRESS_REQEUST_SUCCESS (SL_ZIGBEE_AF_ZDO_RESPONSE_OVERHEAD + 10u)
#define ADDRESS_RESPONSE_NODE_ID_OFFSET (SL_ZIGBEE_AF_ZDO_RESPONSE_OVERHEAD + EUI64_SIZE)

// Address (2), EP Count (1)
#define MINIMUM_ACTIVE_ENDPOINT_RESPONSE (SL_ZIGBEE_AF_ZDO_RESPONSE_OVERHEAD + 3u)
#define ACTIVE_ENDPOINT_RESPONSE_NODE_ID_OFFSET (SL_ZIGBEE_AF_ZDO_RESPONSE_OVERHEAD)
#define ACTIVE_ENDPOINT_RESPONSE_LIST_OFFSET (ACTIVE_ENDPOINT_RESPONSE_NODE_ID_OFFSET + 3u)

#define PREFIX "Svc Disc: "

//==============================================================================
// Forward Declarations

static void setupDiscoveryData(sli_zigbee_service_discovery_state_t *state,
                               sl_802154_short_addr_t messageDest,
                               sl_zigbee_af_service_discovery_callback_t *callback,
                               uint16_t zdoClusterId,
                               uint8_t sequenceNumber);
static void firstTimeInitStates(void);
static sli_zigbee_service_discovery_state_t *findUnusedState(void);
static sli_zigbee_service_discovery_state_t *findStateForResponse(uint8_t sequenceNumber, uint8_t networkIndex);
uint8_t sl_zigbee_get_last_app_zig_dev_request_sequence(void);
static sli_zigbee_service_discovery_state_t *find_state_for_event_control(sl_zigbee_af_event_t *event);
static bool isStateActiveInCurrentNetwork(void);
#ifdef SL_ZIGBEE_TEST
void setDiscoveryStateLimit(uint8_t limit);
#endif

//==============================================================================

void sli_zigbee_af_service_discovery_init_events_callback(uint8_t init_level)
{
  (void)init_level;

  uint8_t i;
  for (i = 0u; i < SL_ZIGBEE_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK; i++) {
    sl_zigbee_af_network_event_init(sli_zigbee_af_service_discovery_events[i],
                                    sli_zigbee_af_service_discovery_timeout_handler);
  }
}

sl_status_t sl_zigbee_af_find_devices_by_profile_and_cluster(sl_802154_short_addr_t target,
                                                             sl_zigbee_af_profile_id_t profileId,
                                                             sl_zigbee_af_cluster_id_t clusterId,
                                                             bool serverCluster,
                                                             sl_zigbee_af_service_discovery_callback_t *callback)
{
  sli_zigbee_service_discovery_state_t *state = findUnusedState();
  if (state == NULL) {
    sl_zigbee_af_service_discovery_println("%sService Discovery State limits reached\n", PREFIX);
    return SL_STATUS_INVALID_STATE;
  }
  sl_status_t status;

  if (SL_ZIGBEE_BROADCAST_ADDRESS <= target
      && target != SL_ZIGBEE_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS) {
    // Note:  The core spec. only allows a Match Descriptor broadcast to
    // the 'rx on when idle' address.  No other broadcast addresses are allowed.
    // The Ember stack will silently discard broadcast match descriptors
    // to invalid broadcast addresses.
    sl_zigbee_af_service_discovery_println("%sIllegal broadcast address, remapping to valid one.",
                                           PREFIX);
    target = SL_ZIGBEE_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS;
  }

  status = sli_zigbee_af_send_match_descriptor(target, profileId, clusterId, serverCluster);
  // Get the sequence number sent in the request.
  uint8_t sequenceNumber = sl_zigbee_get_last_app_zig_dev_request_sequence();
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_service_discovery_println("%sFailed to send match discovery: 0x%02X",
                                           PREFIX,
                                           status);
    return status;
  }

  sl_zigbee_af_service_discovery_println("%sStarting discovery for cluster 0x%04X",
                                         PREFIX,
                                         clusterId);
  setupDiscoveryData(state,
                     target,
                     callback,
                     MATCH_DESCRIPTORS_REQUEST,
                     sequenceNumber);

  return SL_STATUS_OK;
}

sl_status_t sl_zigbee_af_find_active_endpoints(sl_802154_short_addr_t target,
                                               sl_zigbee_af_service_discovery_callback_t *callback)
{
  sli_zigbee_service_discovery_state_t *state = findUnusedState();
  if (state == NULL) {
    sl_zigbee_af_service_discovery_println("%sService Discovery State limits reached\n", PREFIX);
    return SL_STATUS_INVALID_STATE;
  }
  sl_status_t status;

  status = sl_zigbee_active_endpoints_request(target, SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS);
  uint8_t sequenceNumber = sl_zigbee_get_last_app_zig_dev_request_sequence();
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_service_discovery_println("%sFailed to send active endpoint request: 0x%02X",
                                           PREFIX,
                                           status);
    return status;
  }

  setupDiscoveryData(state,
                     target,
                     callback,
                     ACTIVE_ENDPOINTS_REQUEST,
                     sequenceNumber);
  return status;
}

sl_status_t sl_zigbee_af_find_clusters_by_device_and_endpoint(sl_802154_short_addr_t target,
                                                              uint8_t targetEndpoint,
                                                              sl_zigbee_af_service_discovery_callback_t *callback)
{
  sli_zigbee_service_discovery_state_t *state = findUnusedState();
  if (state == NULL) {
    sl_zigbee_af_service_discovery_println("%sService Discovery State limits reached\n", PREFIX);
    return SL_STATUS_INVALID_STATE;
  }
  sl_status_t status;

  status = sl_zigbee_simple_descriptor_request(target,
                                               targetEndpoint,
                                               SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS);
  //Get the sequence number sent in the request.
  uint8_t sequenceNumber = sl_zigbee_get_last_app_zig_dev_request_sequence();

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_service_discovery_println("%sFailed to send simple descriptor request: 0x%02X",
                                           PREFIX,
                                           status);
    return status;
  }

  setupDiscoveryData(state,
                     target,
                     callback,
                     SIMPLE_DESCRIPTOR_REQUEST,
                     sequenceNumber);

  return status;
}

sl_status_t sl_zigbee_af_find_ieee_address(sl_802154_short_addr_t shortAddress,
                                           sl_zigbee_af_service_discovery_callback_t *callback)
{
  sli_zigbee_service_discovery_state_t *state = findUnusedState();
  if (state == NULL) {
    sl_zigbee_af_service_discovery_println("%sService Discovery State limits reached\n", PREFIX);
    return SL_STATUS_INVALID_STATE;
  }
  sl_status_t status;

  status = sl_zigbee_ieee_address_request(shortAddress,
                                          false,  // report kids?
                                          0u,     // child start index
                                          SL_ZIGBEE_APS_OPTION_RETRY);
  // Get the sequence number sent in the request.
  // For SoC, most requests use the application layer's half
  // of the sequence numbering space (0x00-0x7f), but this request
  // uses the stack's half of the sequence numbering space (0x80-0xFF).
  // For Host, all requests use the application layer space.
#if defined(EZSP_HOST)
  uint8_t sequenceNumber = sl_zigbee_get_last_app_zig_dev_request_sequence();
#else
  uint8_t sequenceNumber = sl_zigbee_get_last_stack_zig_dev_request_sequence();
#endif

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_service_discovery_println("%sFailed to send IEEE address request: 0x%02X",
                                           PREFIX,
                                           status);
    return status;
  }

  setupDiscoveryData(state,
                     shortAddress,
                     callback,
                     IEEE_ADDRESS_REQUEST,
                     sequenceNumber);

  return status;
}

sl_status_t sl_zigbee_af_find_node_id(sl_802154_long_addr_t longAddress,
                                      sl_zigbee_af_service_discovery_callback_t *callback)
{
  sli_zigbee_service_discovery_state_t *state = findUnusedState();
  if (state == NULL) {
    sl_zigbee_af_service_discovery_println("%sService Discovery State limits reached\n", PREFIX);
    return SL_STATUS_INVALID_STATE;
  }
  sl_status_t status;

  status = sl_zigbee_network_address_request(longAddress,
                                             false,  // report kids?
                                             0u);    // child start index
  // Get the sequence number sent in the request.
  // For SoC, most requests use the application layer's half
  // of the sequence numbering space (0x00-0x7f), but this request
  // uses the stack's half of the sequence numbering space (0x80-0xFF).
  // For Host, all requests use the application layer space.
#if defined(EZSP_HOST)
  uint8_t sequenceNumber = sl_zigbee_get_last_app_zig_dev_request_sequence();
#else
  uint8_t sequenceNumber = sl_zigbee_get_last_stack_zig_dev_request_sequence();
#endif
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_service_discovery_println("%sFailed to send NWK address request: 0x%02X",
                                           PREFIX,
                                           status);
    return status;
  }

  setupDiscoveryData(state,
                     SL_ZIGBEE_BROADCAST_ADDRESS,
                     callback,
                     NETWORK_ADDRESS_REQUEST,
                     sequenceNumber);

  return status;
}

static void setupDiscoveryData(sli_zigbee_service_discovery_state_t *state,
                               sl_802154_short_addr_t messageDest,
                               sl_zigbee_af_service_discovery_callback_t *callback,
                               uint16_t zdoClusterRequest,
                               uint8_t sequenceNumber)
{
  state->active = true;
  state->requestData = zdoClusterRequest;
  if (messageDest < SL_ZIGBEE_BROADCAST_ADDRESS) {
    setUnicastQuery(state);
  }
  state->callback = callback;
  state->requestSequence = sequenceNumber;
#if !defined (SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT)
  sl_zigbee_af_service_discovery_println("%sWaiting %d sec for discovery to complete",
                                         PREFIX,
                                         SL_ZIGBEE_AF_DISCOVERY_TIMEOUT_QS >> 2);
#endif // !SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
  sl_zigbee_af_event_set_delay_qs(sli_zigbee_af_service_discovery_events[state->stateIndex],
                                  SL_ZIGBEE_AF_DISCOVERY_TIMEOUT_QS);
  // keep sleepy end devices out of hibernation until
  // service discovery is complete
  sl_zigbee_af_add_to_current_app_tasks(SL_ZIGBEE_AF_WAITING_FOR_SERVICE_DISCOVERY);
}

static void serviceDiscoveryComplete(sli_zigbee_service_discovery_state_t *state)
{
  if (state == NULL) {
    sl_zigbee_af_service_discovery_println("State corresponding to the the request not found\n");
    return;
  }

  (void) sl_zigbee_af_push_network_index(state->networkIndex);
  state->active = false;
  sl_zigbee_af_service_discovery_println("%scomplete.\n", PREFIX);
  sl_zigbee_af_event_set_inactive(sli_zigbee_af_service_discovery_events[state->stateIndex]);
  // Since there can be multiple states active on the same network,
  // sl_zigbee_af_remove_from_current_app_tasks should be called only when all the states
  // in the current network are inactive.
  if (!isStateActiveInCurrentNetwork()) {
    // allow sleepy end devices to go into hibernation now.
    sl_zigbee_af_remove_from_current_app_tasks(SL_ZIGBEE_AF_WAITING_FOR_SERVICE_DISCOVERY);
  }

  if (state->callback != NULL) {
    sl_zigbee_af_service_discovery_result_t result;
    result.status = (isUnicastQuery(state)
                     ? SL_ZIGBEE_AF_UNICAST_SERVICE_DISCOVERY_TIMEOUT
                     : SL_ZIGBEE_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE);
    result.zdoRequestClusterId = getRequestCluster(state);
    result.matchAddress = SL_ZIGBEE_NULL_NODE_ID;
    result.responseData = NULL;
    (*state->callback)(&result);
  }

  (void) sl_zigbee_af_pop_network_index();
}

void sli_zigbee_af_service_discovery_timeout_handler(sl_zigbee_af_event_t *event)
{
  sli_zigbee_service_discovery_state_t *state = find_state_for_event_control(event);
  serviceDiscoveryComplete(state);
}

void sli_zigbee_af_service_discovery_complete(uint8_t networkIndex, uint8_t sequenceNumber)
{
  sli_zigbee_service_discovery_state_t *state = findStateForResponse(sequenceNumber, networkIndex);
  serviceDiscoveryComplete(state);
}

static void executeCallback(sli_zigbee_service_discovery_state_t *state,
                            const sl_zigbee_af_service_discovery_result_t *result)
{
  (*state->callback)(result);
  if (isUnicastQuery(state)
      || result->status == SL_ZIGBEE_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE) {
    // If the request was unicast and we received a response then we are done.
    // No need to wait for the timer to expire.
    // Alternatively certain broadcast requests are only expected to get 1
    // response, or we only care about the 1st response.

    // We NULL the callback as a way of indicating we already fired it.
    // For timeouts, the callback will not be NULL and still fire.
    state->callback = NULL;
    serviceDiscoveryComplete(state);
  }
}

static bool processMatchDescriptorResponse(sli_zigbee_service_discovery_state_t *state,
                                           const uint8_t *message,
                                           uint16_t length)
{
  sl_802154_short_addr_t matchId;
  uint8_t listLength;

  if (length < MINIMUM_MATCH_DESCRIPTOR_SUCCESS_LENGTH) {
    sl_zigbee_af_service_discovery_println("%sMessage too short", PREFIX);
    return true;
  }

  // This will now be used as the length of the match list.
  length -= MATCH_DESCRIPTOR_OVERHEAD;

  // If the parent of a sleepy device supports caching its descriptor
  // information then the sender of the response may not be the device
  // that actually matches the request.  The device id that matches
  // is included in the message.
  matchId = message[2] + (message[3] << 8);
  listLength = message[4];

  if (listLength != length) {
    sl_zigbee_af_service_discovery_println("%sMessage too short for num. endpoints",
                                           PREFIX);
    return true;
  }

  sl_zigbee_af_service_discovery_println("%sMatch%s found from 0x%04X.",
                                         PREFIX,
                                         (listLength > 0u
                                          ? ""
                                          : " NOT"),
                                         matchId);

  sl_zigbee_af_service_discovery_result_t result;
  sl_zigbee_af_endpoint_list_t endpointList;
  endpointList.count = length;
  endpointList.list = &(message[MATCH_DESCRIPTOR_OVERHEAD]);
  if (listLength) {
    result.status = (isUnicastQuery(state)
                     ? SL_ZIGBEE_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE
                     : SL_ZIGBEE_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED);
  } else {
    // If we got an active response with an empty list then report it accordingly.
    result.status = (isUnicastQuery(state)
                     ? SL_ZIGBEE_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_EMPTY_RESPONSE
                     : SL_ZIGBEE_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_EMPTY_RESPONSE);
  }
  result.zdoRequestClusterId = getRequestCluster(state);
  result.matchAddress = matchId;
  result.responseData = &endpointList;
  executeCallback(state, &result);
  return true;
}

static bool processSimpleDescriptorResponse(sli_zigbee_service_discovery_state_t *state,
                                            const uint8_t *message,
                                            uint16_t length)
{
  sl_zigbee_af_service_discovery_result_t result;
  sl_zigbee_af_cluster_list_t clusterList;

  clusterList.profileId = message[SIMPLE_DESCRIPTOR_RESPONSE_PROFILE_ID_OFFSET]
                          + (message[SIMPLE_DESCRIPTOR_RESPONSE_PROFILE_ID_OFFSET + 1u] << 8);
  clusterList.deviceId = message[SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET]
                         + (message[SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET + 1u] << 8);
  clusterList.endpoint = message[SIMPLE_DESCRIPTOR_RESPONSE_ENDPOINT_OFFSET];

  // Copying the cluster list to a separate array
  // is primarily necessary on Big endian CPUs where
  // the over-the-air Zigbee messages are all little-endian
  // and we cannot just cast the message pointer into an uint16_t* array.
  // However we also do it for the little endian CPUs because the
  // message can actually be truncated due to bad Zigbee stacks.
  uint16_t clusters[MAX_CLUSTERS_IN_ACTIVE_ENDPOINT_RESPONSE];
  uint8_t index = SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_INDEX;
  clusterList.inClusterCount = message[SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX];
  // The +1 is for the input cluster count length.
  clusterList.outClusterCount = message[SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX
                                        + 1u + (clusterList.inClusterCount * 2u)];

  if ((length - index - 1u) % 2u != 0u) {  // subtract 1 for the output cluster count.
    sl_zigbee_af_service_discovery_println("Error: %s bad simple descriptor length.", PREFIX);
    // Return true because we still tried to process the message.
    return true;
  }
  uint8_t loop;
  uint8_t clusterIndex = 0;
  for (loop = 0u; loop < 2u; loop++) {
    uint8_t count;
    uint8_t i;
    if (loop == 0u) {
      clusterList.inClusterList = &(clusters[clusterIndex]);
      count = clusterList.inClusterCount;
    } else {
      clusterList.outClusterList = &(clusters[clusterIndex]);
      count = clusterList.outClusterCount;
    }
    for (i = 0u; i < count; i++) {
      clusters[clusterIndex] = message[index] + (message[index + 1u] << 8);
      clusterIndex++;
      index += 2u;
    }
    if (loop == 0u) {
      // This is the output cluster count
      index++;
    }
  }

  result.status = (isUnicastQuery(state)
                   ? SL_ZIGBEE_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE
                   : SL_ZIGBEE_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED);
  result.matchAddress = message[2] + (message[3] << 8);
  result.zdoRequestClusterId = getRequestCluster(state);
  result.responseData = &clusterList;

  executeCallback(state, &result);
  return true;
}

// Both NWK and IEEE responses have the same exact format.
static bool processAddressResponse(sli_zigbee_service_discovery_state_t *state,
                                   const uint8_t *message,
                                   uint16_t length)
{
  sl_zigbee_af_service_discovery_result_t result;
  sl_802154_long_addr_t eui64LittleEndian;

  if (length < MINIMUM_ADDRESS_REQEUST_SUCCESS) {
    sl_zigbee_af_service_discovery_println("%sMessage too short", PREFIX);
    return true;
  }
  memmove(eui64LittleEndian, message + SL_ZIGBEE_AF_ZDO_RESPONSE_OVERHEAD, EUI64_SIZE);
  // Given that we only expect one response, we set the status to complete even
  // though the request was broadcast.
  result.status = (isUnicastQuery(state)
                   ? SL_ZIGBEE_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE
                   : SL_ZIGBEE_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE);
  result.matchAddress = (message[ADDRESS_RESPONSE_NODE_ID_OFFSET]
                         + (message[ADDRESS_RESPONSE_NODE_ID_OFFSET + 1u] << 8));
  result.zdoRequestClusterId = getRequestCluster(state);
  result.responseData = eui64LittleEndian;

  executeCallback(state, &result);
  return true;
}

static bool processActiveEndpointResponse(sli_zigbee_service_discovery_state_t* state,
                                          const uint8_t* message,
                                          uint16_t length)
{
  sl_zigbee_af_service_discovery_result_t result;
  sl_zigbee_af_endpoint_list_t listStruct;
  if (length < MINIMUM_ACTIVE_ENDPOINT_RESPONSE) {
    sl_zigbee_af_service_discovery_println("%sMessage too short", PREFIX);
    return true;
  }
  result.matchAddress = (message[ACTIVE_ENDPOINT_RESPONSE_NODE_ID_OFFSET]
                         + (message[ACTIVE_ENDPOINT_RESPONSE_NODE_ID_OFFSET + 1u] << 8));
  uint8_t count = message[ACTIVE_ENDPOINT_RESPONSE_COUNT_OFFSET];

  // +1 for count value iteslf, +2 for node ID
  uint8_t expectedLength = SL_ZIGBEE_AF_ZDO_RESPONSE_OVERHEAD + 1u + 2u + count;

  if (expectedLength != length) {
    sl_zigbee_af_service_discovery_println("Error: %s invalid length (%d != %d) for Active Endpoint response",
                                           PREFIX,
                                           expectedLength,
                                           length);
    return true;
  }
  sl_zigbee_af_service_discovery_println("%sgot active EP response", PREFIX);
  result.status = (isUnicastQuery(state)
                   ? SL_ZIGBEE_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE
                   : SL_ZIGBEE_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED);
  result.zdoRequestClusterId = getRequestCluster(state);
  result.responseData = &listStruct;
  listStruct.list = &(message[ACTIVE_ENDPOINT_RESPONSE_LIST_OFFSET]);
  listStruct.count = message[ACTIVE_ENDPOINT_RESPONSE_COUNT_OFFSET];
  executeCallback(state, &result);
  return true;
}

bool sli_zigbee_af_service_discovery_incoming(sl_802154_short_addr_t sender,
                                              sl_zigbee_aps_frame_t *apsFrame,
                                              const uint8_t *message,
                                              uint16_t length)
{
  (void)sender;
  sli_zigbee_service_discovery_state_t *state = findStateForResponse(message[0], sl_zigbee_get_current_network());
  if (state == NULL) {
    return false;
  }

  if (!(serviceDiscoveryInProgress(state)
        && (apsFrame->profileId == SL_ZIGBEE_ZDO_PROFILE_ID
            // ZDO Responses set the high bit on the request cluster ID
            && (apsFrame->clusterId == (CLUSTER_ID_RESPONSE_MINIMUM
                                        | getRequestCluster(state)))))) {
    return false;
  }

  // The second byte is the status code
  if (message[1] != SL_ZIGBEE_ZDP_SUCCESS) {
    return true;
  }

  switch (apsFrame->clusterId) {
    case SIMPLE_DESCRIPTOR_RESPONSE:
      return processSimpleDescriptorResponse(state, message, length);

    case MATCH_DESCRIPTORS_RESPONSE:
      return processMatchDescriptorResponse(state, message, length);

    case NETWORK_ADDRESS_RESPONSE:
    case IEEE_ADDRESS_RESPONSE:
      return processAddressResponse(state, message, length);

    case ACTIVE_ENDPOINTS_RESPONSE:
      return processActiveEndpointResponse(state, message, length);

    default:
      // Some ZDO request we don't care about.
      break;
  }

  return false;
}

// Perform one-time initialization of states[][] entries.
// Sets the self-identifying array indices stateIndex and networkIndex
// for convenient reference when the state entry is accessed.
static void firstTimeInitStates(void)
{
  if (!statesInitialized) {
    for (uint8_t si = 0u; si < stateLimit; ++si) {
      for (uint8_t ni = 0u; ni < SL_ZIGBEE_SUPPORTED_NETWORKS; ++ni) {
        sli_zigbee_service_discovery_state_t *state = &states[si][ni];
        // Coordinates of this state entry, for convenient reference.
        state->stateIndex = si;
        state->networkIndex = ni;
        // Operational state.
        state->active = false;
        state->callback = NULL;
        state->requestData = 0u;
        state->requestSequence = 0u;
      }
    }
    // Do this only once.
    statesInitialized = true;
  }
}

/**
   Given: N/A.
   Return: A pointer to the first unused state for the current network
   Description: The unused state is found by looking for the first state
   which is inactive in the the current network.
 */
static sli_zigbee_service_discovery_state_t *findUnusedState(void)
{
  firstTimeInitStates();
  for (int si = 0u; si < stateLimit; si++) {
    if (!states[si][sl_zigbee_get_current_network()].active) {
      return &states[si][sl_zigbee_get_current_network()];
    }
  }
  return NULL;
}

/**
   Given: The sequence number of the request and the network index
   Return: The active service discovery state with that sequence number
   in the given network index.
 */
static sli_zigbee_service_discovery_state_t *findStateForResponse(uint8_t sequenceNumber, uint8_t networkIndex)
{
  firstTimeInitStates();
  for (int si = 0u; si < stateLimit; si++) {
    if (states[si][networkIndex].requestSequence == sequenceNumber
        && states[si][networkIndex].active) {
      return &states[si][networkIndex];
    }
  }
  return NULL;
}

/**
   Given: Ember event control
   Return: A pointer to the service discovery state which corresponds to the
   control.
 */
static sli_zigbee_service_discovery_state_t *find_state_for_event_control(sl_zigbee_af_event_t *event)
{
  firstTimeInitStates();
  for (uint8_t si = 0u; si < stateLimit; si++) {
    for (uint8_t ni = 0u; ni < SL_ZIGBEE_SUPPORTED_NETWORKS; ni++) {
      if (event == &sli_zigbee_af_service_discovery_events[si][ni]) {
        return &states[si][ni];
      }
    }
  }
  return NULL;
}

/**
   Description: Checking if there are any active states in the current network.
 */
static bool isStateActiveInCurrentNetwork(void)
{
  uint8_t currentNetwork = sl_zigbee_get_current_network();
  for (uint8_t si = 0u; si < stateLimit; si++) {
    if (states[si][currentNetwork].active) {
      return true;
    }
  }
  return false;
}

/**
   Given: The limit the number of networks allowed
   Description: Even though SL_ZIGBEE_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK
   can be set to 4, the state limit can be set to a smaller number using this
   function for testing purposes.
 */
#ifdef SL_ZIGBEE_TEST
void setDiscoveryStateLimit(uint8_t limit)
{
  if (limit > SL_ZIGBEE_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK) {
    sl_zigbee_af_service_discovery_println("Limit exceeds the maximum number of service discovery states allowed."
                                           "Setting the limit to %d which is the maximum number if states allowed",
                                           SL_ZIGBEE_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK);
    stateLimit = SL_ZIGBEE_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK;
  } else {
    stateLimit = limit;
  }
}
#endif
