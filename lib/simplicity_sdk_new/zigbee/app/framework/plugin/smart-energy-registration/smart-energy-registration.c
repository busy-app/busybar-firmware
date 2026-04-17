/***************************************************************************//**
 * @file
 * @brief This defines the state machine and procedure for a newly joined device to
 * perform the necessary steps to register in a network.
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

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"                     //sl_zigbee_af_is_full_smart_energy_security_present
#include "app/util/zigbee-framework/zigbee-device-common.h" //sl_zigbee_bind_request
#ifdef EZSP_HOST                                            //sl_zigbee_ieee_address_request
  #include "app/util/zigbee-framework/zigbee-device-host.h"
#else //EZSP_HOST
  #include "stack/include/sl_zigbee.h"
#endif //EZSP_HOST

#include "smart-energy-registration.h"
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
#include "app/framework/plugin/test-harness/test-harness.h"
#endif // SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT

#include "app/framework/plugin/esi-management/esi-management.h"
#include "stack/include/zigbee-security-manager.h"

// A helper macro for a human readable TimeStatus flags check.
// Usage e.g.: if (IS_TIME_STATUS(bla, MASTER_CLOCK) { ... }
// Permitted values for 'mask' are defined in enums.h, under SL_ZIGBEE_AF_TIME_STATUS_MASK_...
#define IS_TIME_STATUS(reg, mask) ((reg) & SL_ZIGBEE_AF_TIME_STATUS_MASK_##mask)

sl_zigbee_af_event_t sl_zigbee_af_smart_energy_registration_tick_network_events[SL_ZIGBEE_SUPPORTED_NETWORKS];
void sl_zigbee_af_smart_energy_registration_tick_network_event_handler(sl_zigbee_af_event_t * event);

#if defined(SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED)
  #define PARTNER_KE_REQUIRED
  #define NEXT_STATE_AFTER_KE           STATE_DISCOVER_ENERGY_SERVICE_INTERFACES
  #define NEXT_STATE_AFTER_PARTNER_KE   STATE_PERFORM_BINDING
#elif defined(ALLOW_NON_ESI_TIME_SERVERS)
  #define PARTNER_KE_REQUIRED
  #define NEXT_STATE_AFTER_KE           STATE_DISCOVER_TIME_SERVERS
  #define NEXT_STATE_AFTER_PARTNER_KE   STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE
#else
  #define NEXT_STATE_AFTER_KE           STATE_REGISTRATION_COMPLETE
#endif

typedef enum {
  STATE_INITIAL,
  STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER,
  STATE_PERFORM_KEY_ESTABLISHMENT,
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  STATE_DISCOVER_ENERGY_SERVICE_INTERFACES,
#endif
#ifdef ALLOW_NON_ESI_TIME_SERVERS
  // If non-ESIs are allowed to be time servers, then we neeed to discover them
  // separately. Otherwise we use the discovered ESIs as time server candidates.
  STATE_DISCOVER_TIME_SERVERS,
#endif
#ifdef PARTNER_KE_REQUIRED
  STATE_DISCOVER_IEEE_ADDRESSES,
  STATE_PERFORM_PARTNER_LINK_KEY_EXCHANGE,
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  STATE_PERFORM_BINDING,
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED
  STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE,
#endif
  STATE_REGISTRATION_COMPLETE,
  STATE_REGISTRATION_FAILED,
} sli_zigbee_af_smart_energy_registration_state;

#define UNDEFINED_ENDPOINT 0xFF

#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED
typedef struct {
  bool valid;
  sl_802154_short_addr_t nodeId;
  uint32_t  time;
  uint8_t   timeStatus;
  uint32_t  lastSetTime;
  uint32_t  validUntilTime;
} sli_zigbee_af_smart_energy_registration_time_source;
#endif //SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED

#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED

#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD
uint32_t sli_zigbee_af_smart_energy_registration_discovery_period =
  SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD;
#endif //SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD

#ifndef SE_PROFILE_ID
  #define SE_PROFILE_ID 0x0109
#endif

#define UNDEFINED_CLUSTER_ID 0xFFFF
static sl_zigbee_af_cluster_id_t clusterList[] = {
#ifdef ZCL_USING_PRICE_CLUSTER_CLIENT
  ZCL_PRICE_CLUSTER_ID,
#endif //ZCL_USING_PRICE_CLUSTER_CLIENT

#ifdef ZCL_USING_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_CLIENT
  ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID,
#endif //ZCL_USING_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_CLIENT

#ifdef ZCL_USING_SIMPLE_METERING_CLUSTER_CLIENT
  ZCL_SIMPLE_METERING_CLUSTER_ID,
#endif //ZCL_USING_SIMPLE_METERING_CLUSTER_CLIENT

#ifdef ZCL_USING_MESSAGING_CLUSTER_CLIENT
  ZCL_MESSAGING_CLUSTER_ID,
#endif //ZCL_USING_MESSAGING_CLUSTER_CLIENT

#ifdef ZCL_USING_TUNNELING_CLUSTER_CLIENT
  ZCL_TUNNELING_CLUSTER_ID,
#endif //ZCL_USING_TUNNELING_CLUSTER_CLIENT

#ifdef ZCL_USING_PREPAYMENT_CLUSTER_CLIENT
  ZCL_PREPAYMENT_CLUSTER_ID,
#endif //ZCL_USING_PREPAYMENT_CLUSTER_CLIENT

#ifdef ZCL_USING_CALENDAR_CLUSTER_CLIENT
  ZCL_CALENDAR_CLUSTER_ID,
#endif //ZCL_USING_CALENDAR_CLUSTER_CLIENT

#ifdef ZCL_USING_DEVICE_MANAGEMENT_CLUSTER_CLIENT
  ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
#endif //ZCL_USING_DEVICE_MANAGEMENT_CLUSTER_CLIENT

#ifdef ZCL_USING_EVENTS_CLUSTER_CLIENT
  ZCL_EVENTS_CLUSTER_ID,
#endif //ZCL_USING_EVENTS_CLUSTER_CLIENT

  UNDEFINED_CLUSTER_ID,
};

#endif //SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED

static bool checkErrorCountAndSetEventControl(uint32_t delayMs,
                                              bool errorFlag,
                                              bool resetErrorCount);
static bool resumeAfterDelay(sl_status_t status, uint32_t delayMs);

#define resumeAfterFixedDelay(status) \
  resumeAfterDelay((status),          \
                   SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_RETRY)

#define transition(next) transitionAfterDelay((next),                                                         \
                                              SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_TRANSITION, \
                                              true)     // reset error count
static bool transitionAfterDelay(sli_zigbee_af_smart_energy_registration_state next,
                                 uint32_t delay,
                                 bool resetErrorCount);

static void performKeyEstablishment(void);

#ifdef PARTNER_KE_REQUIRED
static void performPartnerLinkKeyExchange(void);
static void partnerLinkKeyExchangeCallback(bool success);
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
static void performBinding(void);
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED
static void determineAuthoritativeTimeSource(void);
#endif

static void stopRegistration(bool success);

static void performDiscovery(void);
static void discoveryCallback(const sl_zigbee_af_service_discovery_result_t *result);

#ifdef ALLOW_NON_ESI_TIME_SERVERS
  #define MAX_TIME_SOURCE_CANDIDATES    4
#endif

typedef struct {
  sli_zigbee_af_smart_energy_registration_state state;
  uint8_t errors;
  uint8_t trustCenterKeyEstablishmentEndpoint;
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  sl_zigbee_af_plugin_esi_management_esi_entry_t *esiEntry;
  uint8_t endpointIndex; // performBinding
  uint8_t clusterIndex;  // performBinding
#endif
#ifdef ALLOW_NON_ESI_TIME_SERVERS
  struct {
    sl_802154_short_addr_t nodeId;
    uint8_t endpoint;
  } timeSourceCandidates[MAX_TIME_SOURCE_CANDIDATES];
  uint8_t totalCandidates;
  uint8_t currentCandidate;
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED
  bool resuming;    // determineAuthoritativeTimeSource
  sli_zigbee_af_smart_energy_registration_time_source source;
#endif
} State;
static State states[SL_ZIGBEE_SUPPORTED_NETWORKS];

#ifdef ALLOW_NON_ESI_TIME_SERVERS
  #define TIME_SOURCE_CANDIDATE(state)  (state)->timeSourceCandidates[(state)->currentCandidate]
#else
  #define TIME_SOURCE_CANDIDATE(state)  (*(state)->esiEntry)
#endif

//------------------------------------------------------------------------------
//---internal callback
void sli_zigbee_af_smart_energy_registration_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_network_event_init(sl_zigbee_af_smart_energy_registration_tick_network_events,
                                  sl_zigbee_af_smart_energy_registration_tick_network_event_handler);
}
//------------------------------------------------------------------------------

sl_status_t sl_zigbee_af_registration_start_cb(void)
{
  State *state = &states[sl_zigbee_get_current_network()];

  if (!sl_zigbee_af_is_current_security_profile_smart_energy()) {
    return SL_STATUS_INVALID_STATE;
  }

  if (state->state == STATE_REGISTRATION_COMPLETE) {
    // If we got called again after registration has already completed,
    // this means that it was due to a rejoin.  The trust center keepalive
    // may have initiated this rejoin due to a TC failure.
    sl_zigbee_af_trust_center_keepalive_update_cb(true);
    return SL_STATUS_OK;
  }

  if (state->state != STATE_INITIAL) {
    return SL_STATUS_INVALID_STATE;
  }

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
  if (!sli_zigbee_af_test_harness_allow_registration) {
    return SL_STATUS_INVALID_CONFIGURATION;
  }
#endif // SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT

  // Registration is unnecessary for the trust center.  For other nodes, wait
  // for the network broadcast traffic to die down and neighbor information to
  // be populated before continuing.
  if (sl_zigbee_af_get_node_id() == SL_ZIGBEE_TRUST_CENTER_NODE_ID) {
    transition(STATE_REGISTRATION_COMPLETE);
  } else {
    transitionAfterDelay(STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER,
                         SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_INITIAL,
                         true);
  }
  sl_zigbee_af_add_to_current_app_tasks(SL_ZIGBEE_AF_WAITING_FOR_REGISTRATION);
  return SL_STATUS_OK;
}

void sl_zigbee_af_registration_abort_cb(void)
{
  if (sl_zigbee_af_is_current_security_profile_smart_energy()) {
    // We need registration to stop immediately because it may be started up
    // again in the same call chain.
    State *state = &states[sl_zigbee_get_current_network()];
    state->state = STATE_REGISTRATION_FAILED;
    sl_zigbee_af_smart_energy_registration_tick_network_event_handler(sl_zigbee_af_smart_energy_registration_tick_network_events);
  }
}

void sl_zigbee_af_smart_energy_registration_tick_network_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  State *state = &states[sl_zigbee_get_current_network()];
  switch (state->state) {
    case STATE_INITIAL:
      sl_zigbee_af_registration_start_cb();
      break;
    case STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER:
      performDiscovery();
      break;
    case STATE_PERFORM_KEY_ESTABLISHMENT:
      performKeyEstablishment();
      break;
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
    case STATE_DISCOVER_ENERGY_SERVICE_INTERFACES:
      performDiscovery();
      break;
#endif
#ifdef ALLOW_NON_ESI_TIME_SERVERS
    case STATE_DISCOVER_TIME_SERVERS:
      performDiscovery();
      break;
#endif
#ifdef PARTNER_KE_REQUIRED
    case STATE_DISCOVER_IEEE_ADDRESSES:
      performDiscovery();
      break;
    case STATE_PERFORM_PARTNER_LINK_KEY_EXCHANGE:
      performPartnerLinkKeyExchange();
      break;
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
    case STATE_PERFORM_BINDING:
      performBinding();
      break;
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED
    case STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE:
      determineAuthoritativeTimeSource();
      break;
#endif
    case STATE_REGISTRATION_COMPLETE:
    // FALLTHROUGH
    case STATE_REGISTRATION_FAILED:
      stopRegistration(state->state == STATE_REGISTRATION_COMPLETE);
      break;
    default:
      sl_zigbee_af_registration_println("ERR: Invalid state (0x%02X)", state->state);
      sl_zigbee_af_registration_abort_cb();
      break;
  }
}

uint8_t sli_zigbee_af_smart_energy_registration_trust_center_key_establishment_endpoint(void)
{
  // When we start, the key establishment endpoint will be zero.  This is okay
  // internally in this plugin, but we really want to use a better "undefined"
  // value for external modules.
  State *state = &states[sl_zigbee_get_current_network()];
  if (state->trustCenterKeyEstablishmentEndpoint == 0x00) {
    state->trustCenterKeyEstablishmentEndpoint = UNDEFINED_ENDPOINT;
  }
  return state->trustCenterKeyEstablishmentEndpoint;
}

static bool checkErrorCountAndSetEventControl(uint32_t delayMs,
                                              bool errorFlag,
                                              bool resetErrorCount)
{
  State *state = &states[sl_zigbee_get_current_network()];

  // Increment the error count if we're delaying due to an error; otherwise,
  // reset the error count so that failures in any particular state don't affect
  // subsequent states.
  if (errorFlag) {
    state->errors++;
    sl_zigbee_af_registration_println("Registration error count %d of %d",
                                      state->errors,
                                      SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ERROR_LIMIT);
  } else if (resetErrorCount) {
    state->errors = 0;
  } else {
    // MISRA nop
  }

  if (state->errors >= SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ERROR_LIMIT) {
    sl_zigbee_af_registration_flush();
    sl_zigbee_af_registration_println("ERR: Aborting registration"
                                      " because error limit reached (%d)",
                                      SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ERROR_LIMIT);
    sl_zigbee_af_registration_abort_cb();
    return false;
  }

  sl_zigbee_af_event_set_delay_ms(sl_zigbee_af_smart_energy_registration_tick_network_events,
                                  delayMs);

  return true;
}

static bool resumeAfterDelay(sl_status_t status, uint32_t delayMs)
{
  const bool errorFlag = (status != SL_STATUS_OK);
  return checkErrorCountAndSetEventControl((errorFlag
                                            ? delayMs
                                            : SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_RESUME),
                                           errorFlag,
                                           false);
}

static bool transitionAfterDelay(sli_zigbee_af_smart_energy_registration_state next,
                                 uint32_t delay,
                                 bool resetErrorCount)
{
  State *state = &states[sl_zigbee_get_current_network()];
  state->state = next;
  return checkErrorCountAndSetEventControl(delay, false, resetErrorCount);
}

static void performKeyEstablishment(void)
{
  State *state = &states[sl_zigbee_get_current_network()];
  sl_status_t status;
  sl_zigbee_af_registration_println("Performing key establishment");

  // Transient failures may prevent us from performing key establishment.  If
  // so, we will try again later.
  status = sl_zigbee_af_initiate_key_establishment(SL_ZIGBEE_TRUST_CENTER_NODE_ID,
                                                   state->trustCenterKeyEstablishmentEndpoint);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_registration_println("ERR: Failed to start key establishment (0x%02X)",
                                      status);
    resumeAfterFixedDelay(status);
  }
}

bool sl_zigbee_af_key_establishment_event_cb(sl_zigbee_af_key_establishment_notify_message_t status,
                                             bool amInitiator,
                                             sl_802154_short_addr_t partnerShortId,
                                             uint8_t delayInSeconds)
{
  UNUSED_VAR(amInitiator);
  UNUSED_VAR(partnerShortId);
  State *state = &states[sl_zigbee_get_current_network()];

  // The notification only matters if we are performing key establishment.
  if (state->state == STATE_PERFORM_KEY_ESTABLISHMENT) {
    if (status == LINK_KEY_ESTABLISHED) {
      transition(NEXT_STATE_AFTER_KE);
    } else if (status >= APP_NOTIFY_ERROR_CODE_START) {
      uint32_t delayMs = delayInSeconds * MILLISECOND_TICKS_PER_SECOND;
      sl_zigbee_af_registration_println("ERR: Key establishment failed (0x%02X)", status);
      resumeAfterDelay(SL_STATUS_FAIL,
                       delayMs);
    }
  }

  // Always allow key establishment to continue.
  return true;
}

#ifdef PARTNER_KE_REQUIRED
// This function deserves an explanation.
// It is a helper whose purpose is to find the next suitable candidate to query
// the IEEE address from or perform a partner Key Establishment with. Depending
// on compilation options, the candidate can be chosen from the list of ESIs,
// the list of discovered Time servers, or both. If both, we try to take care
// of avoiding duplicates.
// It could be called in two different contexts. One, before a discovery starts,
// in which case the first located candidate is used.
// The second context is within a callback, when we want to advance to the next
// suitable candidate and skip the current one. Hence the 'skipCurrent' argument.
static bool findNextNodeForDiscoveryOrPartnerKeyExchange(State *state,
                                                         bool skipCurrent)
{
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  while (state->esiEntry != NULL) {
    assert(state->esiEntry->nodeId != SL_ZIGBEE_NULL_NODE_ID);
    if (skipCurrent
        || state->esiEntry->nodeId == SL_ZIGBEE_TRUST_CENTER_NODE_ID) {
      // Skip the Trust Center: we have already done KE with it
      state->esiEntry = sl_zigbee_af_esi_management_get_next_entry(state->esiEntry, 0);
      skipCurrent = false;
      continue;
    }
    return true;
  }
#endif
#ifdef ALLOW_NON_ESI_TIME_SERVERS
  while (state->currentCandidate < state->totalCandidates) {
    if (skipCurrent
        || state->timeSourceCandidates[state->currentCandidate].nodeId == SL_ZIGBEE_TRUST_CENTER_NODE_ID
  #ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
        || sl_zigbee_af_esi_management_esi_look_up_by_short_id_and_endpoint(state->timeSourceCandidates[state->currentCandidate].nodeId,
                                                                            state->timeSourceCandidates[state->currentCandidate].endpoint) != NULL
  #endif
        ) {
      // Skip TC and ESI nodes; we have already done partner KE with them
      state->currentCandidate++;
      skipCurrent = false;
      continue;
    }
    return true;
  }
#endif
  return false;
}

static bool retrieveNodeForDiscoveryOrPartnerKeyExchange(const State *state,
                                                         sl_802154_short_addr_t *nodeId,
                                                         uint8_t *endpoint)
{
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  if (state->esiEntry != NULL) {
    *nodeId = state->esiEntry->nodeId;
    *endpoint = state->esiEntry->endpoint;
    return true;
  }
#endif
#ifdef ALLOW_NON_ESI_TIME_SERVERS
  if (state->currentCandidate < state->totalCandidates) {
    *nodeId = state->timeSourceCandidates[state->currentCandidate].nodeId;
    *endpoint = state->timeSourceCandidates[state->currentCandidate].endpoint;
    return true;
  }
#endif
  return false;
}

static void performPartnerLinkKeyExchange(void)
{
  State *state = &states[sl_zigbee_get_current_network()];
  if (findNextNodeForDiscoveryOrPartnerKeyExchange(state, false)) {
    sl_802154_short_addr_t nodeId;
    uint8_t endpoint;
    sl_status_t status;

    retrieveNodeForDiscoveryOrPartnerKeyExchange(state, &nodeId, &endpoint);
    sl_zigbee_af_registration_println("Perform%s partner link key exchange"
                                      " with node 0x%04X endpoint 0x%02X",
                                      "ing",
                                      nodeId,
                                      endpoint);
    status = sl_zigbee_af_initiate_partner_link_key_exchange(nodeId,
                                                             endpoint,
                                                             partnerLinkKeyExchangeCallback);
    if (status != SL_STATUS_OK) {
      sl_zigbee_af_registration_println("ERR: Failed to %s partner link key request"
                                        " with node 0x%04X endpoint 0x%02X (0x%02X)",
                                        "initiate",
                                        nodeId,
                                        endpoint,
                                        status);
      resumeAfterFixedDelay(status);
    }
  } else {
    // No more ESIs to partner KE with, move on to the next step.
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
    state->esiEntry = sl_zigbee_af_esi_management_get_next_entry(NULL, 0);
#endif
#ifdef ALLOW_NON_ESI_TIME_SERVERS
    state->currentCandidate = 0;
#endif
    transition(NEXT_STATE_AFTER_PARTNER_KE);
  }
}

static void partnerLinkKeyExchangeCallback(bool success)
{
  State *state = &states[sl_zigbee_get_current_network()];
  sl_802154_short_addr_t nodeId;
  uint8_t endpoint;

  if (state->state == STATE_PERFORM_PARTNER_LINK_KEY_EXCHANGE
      && retrieveNodeForDiscoveryOrPartnerKeyExchange(state, &nodeId, &endpoint)) {
    if (success) {
      sl_zigbee_af_registration_println("Perform%s partner link key exchange"
                                        " with node 0x%04X endpoint 0x%02X",
                                        "ed",
                                        nodeId,
                                        endpoint);
      findNextNodeForDiscoveryOrPartnerKeyExchange(state, true);
    } else {
      sl_zigbee_af_registration_println("ERR: Failed to %s partner link key exchange"
                                        " with node 0x%04X endpoint 0x%02X",
                                        "perform",
                                        nodeId,
                                        endpoint);
    }
    resumeAfterFixedDelay(success ? SL_STATUS_OK : SL_STATUS_FAIL);
  }
}
#endif //PARTNER_KE_REQUIRED

#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
static void performBinding(void)
{
  State *state = &states[sl_zigbee_get_current_network()];
  sl_802154_long_addr_t eui64;
  uint8_t networkIndex = sl_zigbee_get_current_network();

  sl_zigbee_af_get_eui64(eui64);

  // The spec requires that we bind in order to inform the ESI that we want
  // unsolicited updates, but the ESI is not required to use binding and may
  // send a negative response.  In addition, there is no recourse if a binding
  // is required but the ESI refuses.  So, send the bindings, but don't pay
  // attention to whether it works or not.
  for (;
       state->esiEntry != NULL;
       state->esiEntry = sl_zigbee_af_esi_management_get_next_entry(state->esiEntry, 0)) {
    assert(state->esiEntry->nodeId != SL_ZIGBEE_NULL_NODE_ID);
    for (;
         state->endpointIndex < sl_zigbee_af_endpoint_count();
         state->endpointIndex++) {
      uint8_t endpoint = sl_zigbee_af_endpoint_from_index(state->endpointIndex);
      if (networkIndex
          != sl_zigbee_af_network_index_from_endpoint_index(state->endpointIndex)) {
        continue;
      }
      for (;
           clusterList[state->clusterIndex] != UNDEFINED_CLUSTER_ID;
           state->clusterIndex++) {
        sl_zigbee_af_cluster_id_t clusterId = clusterList[state->clusterIndex];
        if (sl_zigbee_af_contains_client(endpoint, clusterId)) {
          sl_status_t status;
          sl_zigbee_af_registration_println("Performing binding"
                                            " to node 0x%04X endpoint 0x%02X"
                                            " from endpoint 0x%02X"
                                            " for cluster 0x%04X",
                                            state->esiEntry->nodeId,
                                            state->esiEntry->endpoint,
                                            endpoint,
                                            clusterId);
          status = sl_zigbee_bind_request(state->esiEntry->nodeId,
                                          state->esiEntry->eui64,
                                          state->esiEntry->endpoint,
                                          clusterId,
                                          UNICAST_BINDING,
                                          eui64,
                                          0, // multicast group identifier - ignored
                                          endpoint,
                                          SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS);
          if (status == SL_STATUS_OK) {
            state->clusterIndex++;
          } else {
            sl_zigbee_af_registration_println("ERR: Failed to send bind request"
                                              " to node 0x%04X endpoint 0x%02X"
                                              " from endpoint 0x%02X"
                                              " for cluster 0x%04X (0x%02X)",
                                              state->esiEntry->nodeId,
                                              state->esiEntry->endpoint,
                                              endpoint,
                                              clusterId,
                                              status);
          }

          // We may hit the error limit if we delay here due to an error.  If
          // so, we have to clear our internal static indices; otherwise, when
          // registration is restarted, we won't pick up from the beginning.
          if (!resumeAfterFixedDelay(status)) {
            state->endpointIndex = state->clusterIndex = 0;
          }
          return;
        }
      }
      state->clusterIndex = 0;
    }
    state->endpointIndex = 0;
  }

  // Point to the first active entry with age 0 (if any).
  state->esiEntry = sl_zigbee_af_esi_management_get_next_entry(NULL, 0);
  transition(STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE);
}
#endif //SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED

#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED
static void determineAuthoritativeTimeSource(void)
{
  State *state = &states[sl_zigbee_get_current_network()];
  static const uint8_t attributeIds[] = {
    LOW_BYTE(ZCL_TIME_ATTRIBUTE_ID), HIGH_BYTE(ZCL_TIME_ATTRIBUTE_ID),
    LOW_BYTE(ZCL_TIME_STATUS_ATTRIBUTE_ID), HIGH_BYTE(ZCL_TIME_STATUS_ATTRIBUTE_ID),
    LOW_BYTE(ZCL_LAST_SET_TIME_ATTRIBUTE_ID), HIGH_BYTE(ZCL_LAST_SET_TIME_ATTRIBUTE_ID),
    LOW_BYTE(ZCL_VALID_UNTIL_TIME_ATTRIBUTE_ID), HIGH_BYTE(ZCL_VALID_UNTIL_TIME_ATTRIBUTE_ID),
  };
  const uint8_t sourceEndpoint = sl_zigbee_af_primary_endpoint_for_current_network_index();
  assert(sourceEndpoint != 0xFF);

  if (!state->resuming) {
    sl_zigbee_af_registration_println("Determining authoritative time source");
    state->source.valid = false;
  }

#ifdef ALLOW_NON_ESI_TIME_SERVERS
  while (state->currentCandidate < state->totalCandidates) {
#else
  while (state->esiEntry != NULL) {
#endif
    sl_status_t status;
    assert(TIME_SOURCE_CANDIDATE(state).nodeId != SL_ZIGBEE_NULL_NODE_ID);
    sl_zigbee_af_registration_println("Requesting time attributes"
                                      " from node 0x%04X endpoint 0x%02X",
                                      TIME_SOURCE_CANDIDATE(state).nodeId,
                                      TIME_SOURCE_CANDIDATE(state).endpoint);

    sl_zigbee_af_fill_command_global_client_to_server_read_attributes(ZCL_TIME_CLUSTER_ID,
                                                                      attributeIds,
                                                                      sizeof(attributeIds));
    sl_zigbee_af_set_command_endpoints(sourceEndpoint,
                                       TIME_SOURCE_CANDIDATE(state).endpoint);
    status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                               TIME_SOURCE_CANDIDATE(state).nodeId);

    if (status == SL_STATUS_OK) {
#ifdef ALLOW_NON_ESI_TIME_SERVERS
      state->currentCandidate++;
      if (state->currentCandidate >= state->totalCandidates) {
#else
      state->esiEntry = sl_zigbee_af_esi_management_get_next_entry(state->esiEntry, 0);
      if (state->esiEntry == NULL) {
#endif
        state->resuming = transitionAfterDelay(STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE,
                                               SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_BRIEF,
                                               false);  // do not reset error count
        return;
      }
    } else {
      sl_zigbee_af_registration_println("ERR: Failed to request time attributes"
                                        " from node 0x%04X endpoint 0x%02X (0x%02X)",
                                        TIME_SOURCE_CANDIDATE(state).nodeId,
                                        TIME_SOURCE_CANDIDATE(state).endpoint,
                                        status);
    }
    state->resuming = resumeAfterFixedDelay(status);
    return;
  }

  // Reset the position regardless of whether we have succeeded or failed.
  // If we failed, the reset position will be used in retries.
#ifdef ALLOW_NON_ESI_TIME_SERVERS
  state->currentCandidate = 0;
#else
  state->esiEntry = sl_zigbee_af_esi_management_get_next_entry(NULL, 0);
#endif

  if (!state->source.valid) {
    sl_zigbee_af_registration_println("ERR: Failed to determine"
                                      " authoritative time source");
    state->resuming = resumeAfterFixedDelay(SL_STATUS_FAIL);
  } else {
    sl_zigbee_af_registration_println("Determined authoritative time source,"
                                      " node 0x%04X",
                                      state->source.nodeId);
    transition(STATE_REGISTRATION_COMPLETE);
  }
}

void sli_zigbee_af_smart_energy_registration_read_attributes_response_callback(uint8_t *buffer,
                                                                               uint16_t bufLen)
{
  State *state = &states[sl_zigbee_get_current_network()];
  uint32_t time           = 0x00000000UL;
  uint8_t  timeStatus     = 0x00;
  uint32_t lastSetTime    = 0x00000000UL;
  uint32_t validUntilTime = 0xFFFFFFFFUL;
  uint16_t bufIndex = 0;
  sl_zigbee_af_cluster_command_t *cmd = sl_zigbee_af_current_command();

  if (state->state != STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE) {
    return;
  }

  // Each record in the response has a two-byte attribute id and a one-byte
  // status.  If the status is SUCCESS, there will also be a one-byte type and
  // variable-length data.
  while (bufIndex + 3 <= bufLen) {
    sl_zigbee_af_attribute_id_t attributeId;
    sl_zigbee_af_status_t status;
    attributeId = (sl_zigbee_af_attribute_id_t)sl_zigbee_af_get_int16u(buffer, bufIndex, bufLen);
    bufIndex += 2;
    status = (sl_zigbee_af_status_t)sl_zigbee_af_get_int8u(buffer, bufIndex, bufLen);
    bufIndex++;

    if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_attribute_type_t type;
      if (bufLen - bufIndex < 1) {
        // premature end of buffer after success status
        break;
      }
      type = (sl_zigbee_af_attribute_type_t)sl_zigbee_af_get_int8u(buffer, bufIndex, bufLen);
      bufIndex++;

      uint16_t dataSize = sl_zigbee_af_attribute_value_size(type,
                                                            buffer + bufIndex,
                                                            bufLen - bufIndex);
      if (dataSize == 0) {
        // dataSize exceeds buffer length, terminate loop
        break;
      }

      switch (attributeId) {
        case ZCL_TIME_ATTRIBUTE_ID:
          time = sl_zigbee_af_get_int32u(buffer, bufIndex, bufLen);
          break;
        case ZCL_TIME_STATUS_ATTRIBUTE_ID:
          timeStatus = sl_zigbee_af_get_int8u(buffer, bufIndex, bufLen);
          break;
        case ZCL_LAST_SET_TIME_ATTRIBUTE_ID:
          lastSetTime = sl_zigbee_af_get_int32u(buffer, bufIndex, bufLen);
          break;
        case ZCL_VALID_UNTIL_TIME_ATTRIBUTE_ID:
          validUntilTime = sl_zigbee_af_get_int32u(buffer, bufIndex, bufLen);
          break;
        default:
          break;
      }

      bufIndex += dataSize;
    }
  }

  sl_zigbee_af_registration_println("Received time attributes from node 0x%04X",
                                    cmd->source);
  sl_zigbee_af_registration_println("time 0x%08X", time);
  sl_zigbee_af_registration_println("time status 0x%02X", timeStatus);

  // The process for determining the most authoritative time source is outlined
  // in section 3.12.2.2.2 of 07-5123-06 (ZigBee Cluster Library Specification).
  // Devices shall synchronize to a Time server with the highest rank according
  // to the following rules, listed in order of precedence:
  //   1. A server with the Superseding bit set shall be chosen over a server
  //      without the bit set.
  //   2. A server with the Master bit set shall be chosen over a server without
  //      the bit set.
  //   3. The server with the lower short address shall be chosen (note that
  //      this means a coordinator with the Superseding and Master bit set will
  //      always be chosen as the network time server).
  //   4. A Time server with neither the Master nor Synchronized bits set should
  //      not be chosen as the network time server.

  // This logic could be reduced if needed. However, this implementation
  // is way more readable.

  if (time == 0xFFFFFFFFUL) {
    if (state->source.valid
        && state->source.nodeId == cmd->source) {
      // We received an invalid time from our current time server. We set it to
      // invalid and wait for another server.
      state->source.valid = false;
      // TODO: Should we kick off the registration process again here?
    }
  } else {
    // The candidate must have at least one of Master or Synchronized bits set.
    if ((IS_TIME_STATUS(timeStatus, MASTER_CLOCK)
         || IS_TIME_STATUS(timeStatus, SYNCHRONIZED))
        // If we do not have a server yet, use this one.
        // If we do, use the precedence list above to decide whether the new
        // candidate is better than the one we currently have.
        && (!state->source.valid
            || (IS_TIME_STATUS(timeStatus, SUPERSEDING)
                && !IS_TIME_STATUS(state->source.timeStatus, SUPERSEDING))
            || (IS_TIME_STATUS(timeStatus, SUPERSEDING) == IS_TIME_STATUS(state->source.timeStatus, SUPERSEDING)
                && IS_TIME_STATUS(timeStatus, MASTER_CLOCK)
                && !IS_TIME_STATUS(state->source.timeStatus, MASTER_CLOCK))
            || (IS_TIME_STATUS(timeStatus, SUPERSEDING) == IS_TIME_STATUS(state->source.timeStatus, SUPERSEDING)
                && IS_TIME_STATUS(timeStatus, MASTER_CLOCK) == IS_TIME_STATUS(state->source.timeStatus, MASTER_CLOCK)
                && cmd->source < state->source.nodeId))) {
      state->source.valid          = true;
      state->source.nodeId         = cmd->source;
      state->source.time           = time;
      state->source.timeStatus     = timeStatus;
      state->source.lastSetTime    = lastSetTime;
      state->source.validUntilTime = validUntilTime;
      sl_zigbee_af_set_time(time);

      sl_zigbee_af_registration_println("Node 0x%04X chosen as"
                                        " authoritative time source",
                                        cmd->source);
    }
  }
}
#endif //SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED

static void stopRegistration(bool success)
{
  State *state = &states[sl_zigbee_get_current_network()];

  sl_zigbee_af_remove_from_current_app_tasks(SL_ZIGBEE_AF_WAITING_FOR_REGISTRATION);
  sl_zigbee_af_registration_cb(success);

  sl_zigbee_af_trust_center_keepalive_update_cb(success);

#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD
  if (success) {
    transitionAfterDelay(STATE_DISCOVER_ENERGY_SERVICE_INTERFACES,
                         sli_zigbee_af_smart_energy_registration_discovery_period,
                         true);
    return;
  }
#endif //SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD
  state->state = STATE_INITIAL;
}

static void performDiscovery(void)
{
  State *state = &states[sl_zigbee_get_current_network()];
  sl_status_t status;
  sl_802154_short_addr_t target;
  sl_zigbee_af_cluster_id_t clusterId;

  // When performing key establishment, search the trust center for the Key
  // Establishment cluster.  When searching for ESIs, broadcast for the DRLC
  // server cluster, which only ESIs should have.
  if (state->state == STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER) {
    sl_zigbee_af_registration_println("Discovering %ss", "Key Establishment cluster");
    state->trustCenterKeyEstablishmentEndpoint = UNDEFINED_ENDPOINT;
    target = SL_ZIGBEE_TRUST_CENTER_NODE_ID;
    clusterId = ZCL_KEY_ESTABLISHMENT_CLUSTER_ID;
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  } else if (state->state == STATE_DISCOVER_ENERGY_SERVICE_INTERFACES) {
    sl_zigbee_af_registration_println("Discovering %ss", "Energy Service Interface");
    // Aging the entries in the ESI table before starting the discovery process.
    sl_zigbee_af_esi_management_age_all_entries();
    target = SL_ZIGBEE_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS;
    clusterId = ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID;
#endif
#ifdef ALLOW_NON_ESI_TIME_SERVERS
  } else if (state->state == STATE_DISCOVER_TIME_SERVERS) {
    sl_zigbee_af_registration_println("Discovering %ss", "Time server");
    state->totalCandidates = 0;
    state->currentCandidate = 0;
    target = SL_ZIGBEE_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS;
    clusterId = ZCL_TIME_CLUSTER_ID;
#endif
#ifdef PARTNER_KE_REQUIRED
  } else if (state->state == STATE_DISCOVER_IEEE_ADDRESSES) {
    uint8_t ep; // retrieveNextNodeFor... needs it, otherwise ignored
    retrieveNodeForDiscoveryOrPartnerKeyExchange(state, &target, &ep);
    assert(target != SL_ZIGBEE_NULL_NODE_ID);
    sl_zigbee_af_registration_println("Discovering IEEE address"
                                      " for node 0x%04X",
                                      target);
    status = sl_zigbee_af_find_ieee_address(target, discoveryCallback);
    goto kickout;
#endif
  } else {
    sl_zigbee_af_registration_println("ERR: Invalid state for discovery (0x%02X)",
                                      state->state);
    sl_zigbee_af_registration_abort_cb();
    return;
  }

  sl_zigbee_af_registration_println("Calling the FindDevices");
  // Transient failures may prevent us from performing discovery.  If so, we
  // will try again later.
  status = sl_zigbee_af_find_devices_by_profile_and_cluster(target,
                                                            SE_PROFILE_ID,
                                                            clusterId,
                                                            SL_ZIGBEE_AF_SERVER_CLUSTER_DISCOVERY,
                                                            discoveryCallback);
#ifdef PARTNER_KE_REQUIRED
  kickout:
#endif
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_registration_println("ERR: Failed to start discovery (0x%02X)", status);
    resumeAfterFixedDelay(status);
  }
}

static void discoveryCallback(const sl_zigbee_af_service_discovery_result_t *result)
{
  uint8_t networkIndex = sl_zigbee_get_current_network();
  State *state = &states[networkIndex];

  if (sl_zigbee_af_have_discovery_response_status(result->status)) {
    if (result->zdoRequestClusterId == MATCH_DESCRIPTORS_REQUEST) {
      const sl_zigbee_af_endpoint_list_t* endpointList = (const sl_zigbee_af_endpoint_list_t *)result->responseData;
      uint8_t i;

      // Need to ignore any discovery results from ourselves
      if (result->matchAddress == sl_zigbee_af_get_node_id()) {
        sl_zigbee_af_registration_println("Ignoring discovery result from loopback");
        return;
      }

      for (i = 0; i < endpointList->count; i++) {
        if (state->state == STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER) {
          // Key Establishment is global to the device so we can ignore anything
          // beyond the first endpoint that responds.
          if (state->trustCenterKeyEstablishmentEndpoint
              == UNDEFINED_ENDPOINT) {
            sl_zigbee_af_registration_println("%s %s on node 0x%04X endpoint 0x%02X%s",
                                              "Discovered",
                                              "Key Establishment cluster",
                                              result->matchAddress,
                                              endpointList->list[i],
                                              "");
            state->trustCenterKeyEstablishmentEndpoint = endpointList->list[i];
          } else {
            sl_zigbee_af_registration_println("%s %s on node 0x%04X endpoint 0x%02X%s",
                                              "INFO: Ignored",
                                              "Key Establishment cluster",
                                              result->matchAddress,
                                              endpointList->list[i],
                                              "");
          }
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
        } else if (state->state == STATE_DISCOVER_ENERGY_SERVICE_INTERFACES) {
          sl_zigbee_af_registration_println("%s %s on node 0x%04X endpoint 0x%02X%s",
                                            "Discovered",
                                            "Energy Server Interface",
                                            result->matchAddress,
                                            endpointList->list[i],
                                            "");

          state->esiEntry = sl_zigbee_af_esi_management_esi_look_up_by_short_id_and_endpoint(result->matchAddress,
                                                                                             endpointList->list[i]);
          if (state->esiEntry == NULL) {
            state->esiEntry = sl_zigbee_af_esi_management_get_free_entry();
          }

          if (state->esiEntry != NULL) {
            state->esiEntry->nodeId = result->matchAddress;
            state->esiEntry->networkIndex = networkIndex;
            state->esiEntry->endpoint = endpointList->list[i];
            state->esiEntry->age = 0;
          } else {
            sl_zigbee_af_registration_println("%s %s on node 0x%04X endpoint 0x%02X%s",
                                              "INFO: Ignored",
                                              "Energy Server Interface",
                                              result->matchAddress,
                                              endpointList->list[i],
                                              " because table is full");
          }
#endif //SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
#ifdef ALLOW_NON_ESI_TIME_SERVERS
        } else if (state->state == STATE_DISCOVER_TIME_SERVERS) {
          sl_zigbee_af_registration_println("%s %s on node 0x%04X endpoint 0x%02X%s",
                                            "Discovered",
                                            "Time server",
                                            result->matchAddress,
                                            endpointList->list[i],
                                            "");

          if (state->totalCandidates < MAX_TIME_SOURCE_CANDIDATES) {
            state->timeSourceCandidates[state->totalCandidates].nodeId = result->matchAddress;
            state->timeSourceCandidates[state->totalCandidates].endpoint = endpointList->list[i];
            state->totalCandidates++;
          } else {
            sl_zigbee_af_registration_println("%s %s on node 0x%04X endpoint 0x%02X%s",
                                              "INFO: Ignored",
                                              "Time server",
                                              result->matchAddress,
                                              endpointList->list[i],
                                              " because table is full");
          }
#endif // ALLOW_NON_ESI_TIME_SERVERS
        }
      }
    }
  }

  if (state->state == STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER) {
    if (state->trustCenterKeyEstablishmentEndpoint == UNDEFINED_ENDPOINT) {
      sl_zigbee_af_registration_println("ERR: Failed to find %s",
                                        "Key Establishment cluster");
      resumeAfterFixedDelay(SL_STATUS_FAIL);
    } else {
      sl_zigbee_sec_man_context_t context;
      sl_zigbee_sec_man_aps_key_metadata_t key_data;
      sl_zigbee_sec_man_init_context(&context);

      context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;
      sl_status_t status = sl_zigbee_sec_man_get_aps_key_info(&context, &key_data);

      if (status != SL_STATUS_OK) {
        sl_zigbee_af_registration_println("ERR: Failed to get trust center link key");
        sl_zigbee_af_registration_abort_cb();
        return;
      }

      // If we don't have full Smart Energy Security or if the key is already
      // authorized, we can skip key establishment and move on to ESI discovery.
      if (sl_zigbee_af_is_full_smart_energy_security_present() == SL_ZIGBEE_AF_INVALID_KEY_ESTABLISHMENT_SUITE) {
        sl_zigbee_af_registration_flush();
        sl_zigbee_af_registration_println("%sSkipping key establishment%s",
                                          "WARN: ",
                                          " due to missing libraries or certificate"
                                          " - see 'info' command for more detail");
        sl_zigbee_af_registration_flush();
        transition(NEXT_STATE_AFTER_KE);
      } else if ((key_data.bitmask & SL_ZIGBEE_KEY_IS_AUTHORIZED) != 0U) {
        sl_zigbee_af_registration_println("%sSkipping key establishment%s",
                                          "",
                                          " because key is already authorized");
        transition(NEXT_STATE_AFTER_KE);
      } else {
        transition(STATE_PERFORM_KEY_ESTABLISHMENT);
      }
    }
#ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  } else if (state->state == STATE_DISCOVER_ENERGY_SERVICE_INTERFACES) {
    if (result->status == SL_ZIGBEE_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE) {
      // Point to the first active entry with age 0 (if any).
      state->esiEntry = sl_zigbee_af_esi_management_get_next_entry(NULL, 0);
      // There is no entry with age 0, i.e., we did not discover any ESI during
      // this discovery cycle.
      if (state->esiEntry == NULL) {
        // TODO: For now we just return an error. We might consider checking if
        // we have an ESI in the table "young enough" before returning an error.
        sl_zigbee_af_registration_println("ERR: Failed to find %s",
                                          "Energy Service Interfaces");
        resumeAfterFixedDelay(SL_STATUS_FAIL);
      } else {
  #ifdef ALLOW_NON_ESI_TIME_SERVERS
        transition(STATE_DISCOVER_TIME_SERVERS);
  #else
        transition(STATE_DISCOVER_IEEE_ADDRESSES);
  #endif
      }
    }
#endif //SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
#ifdef ALLOW_NON_ESI_TIME_SERVERS
  } else if (state->state == STATE_DISCOVER_TIME_SERVERS) {
    if (result->status == SL_ZIGBEE_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE) {
      if (state->totalCandidates == 0) {
        sl_zigbee_af_registration_println("ERR: Failed to find %s",
                                          "Time servers");
        resumeAfterFixedDelay(SL_STATUS_FAIL);
      } else {
        state->currentCandidate = 0;
        transition(STATE_DISCOVER_IEEE_ADDRESSES);
      }
    }
#endif // ALLOW_NON_ESI_TIME_SERVERS
#ifdef PARTNER_KE_REQUIRED
  } else if (state->state == STATE_DISCOVER_IEEE_ADDRESSES) {
    sl_802154_short_addr_t nodeId;
    uint8_t ep; // retrieveNextNodeFor... needs it, otherwise ignored
    retrieveNodeForDiscoveryOrPartnerKeyExchange(state, &nodeId, &ep);
    if (nodeId != SL_ZIGBEE_NULL_NODE_ID
        && nodeId == result->matchAddress
        && result->status == SL_ZIGBEE_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE) {
      sl_zigbee_af_registration_println("%s IEEE address for node 0x%04X",
                                        "Discovered",
                                        nodeId);
  #ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
      if (state->esiEntry != NULL) {
        // The discovered IEEE address may have been of an ESI or a Time server.
        // If the former, update the relevant ESI entry.
        // Otherwise, the entry is going top be NULL.
        memmove(state->esiEntry->eui64, result->responseData, EUI64_SIZE);
      }
  #endif
      if (sl_zigbee_af_add_address_table_entry((uint8_t*)result->responseData, nodeId)
          == SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX) {
        sl_zigbee_af_registration_println("WARN: Could not add address table entry"
                                          " for node 0x%04X",
                                          nodeId);
      }
      if (!findNextNodeForDiscoveryOrPartnerKeyExchange(state, true)) {
  #ifdef SL_ZIGBEE_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
        // Point to the first active entry with age 0 (if any).
        state->esiEntry = sl_zigbee_af_esi_management_get_next_entry(NULL, 0);
  #endif
  #ifdef ALLOW_NON_ESI_TIME_SERVERS
        state->currentCandidate = 0;
  #endif
        transition(STATE_PERFORM_PARTNER_LINK_KEY_EXCHANGE);
      } else {
        resumeAfterFixedDelay(SL_STATUS_OK);
      }
    } else {
      sl_zigbee_af_registration_println("ERR: Failed to discover IEEE address"
                                        " for node 0x%04X",
                                        nodeId);
      resumeAfterFixedDelay(SL_STATUS_FAIL);
    }
#endif //PARTNER_KE_REQUIRED
  }
}
