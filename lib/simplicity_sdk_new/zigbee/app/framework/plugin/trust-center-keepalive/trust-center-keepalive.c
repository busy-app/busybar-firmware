/***************************************************************************//**
 * @file
 * @brief Routines for the Trust Center Keepalive plugin, which implements a
 *        mechanism for ensuring the trust center is responding on the network.
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
#include "app/framework/util/common.h"
#include "trust-center-keepalive.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_TRUST_CENTER_KEEPALIVE_PRESENT
#include "trust-center-keepalive-config.h"
#else
#include "config/trust-center-keepalive-config.h"
#endif

// #define PLUGIN_DEBUG
#if defined(PLUGIN_DEBUG)
  #define debug_print(...) sl_zigbee_af_core_println(__VA_ARGS__)
#else
  #define debug_print(...)
#endif

//------------------------------------------------------------------------------
// Globals

#if defined(SL_ZIGBEE_SCRIPTED_TEST)
  #define SL_ZIGBEE_AF_PLUGIN_TRUST_CENTER_KEEPALIVE_DEFAULT_JITTER_PERIOD_SECONDS  60
#endif

typedef enum {
  STATE_DISABLED,
  STATE_NONE,
  STATE_DISCOVER_ENDPOINT,
  STATE_SEND_KEEPALIVE_SIGNAL,
  STATE_INITIATE_NETWORK_SEARCH,
} sli_zigbee_trust_center_keepalive_state_t;

sl_zigbee_af_event_t sl_zigbee_af_trust_center_keepalive_tick_network_events[SL_ZIGBEE_SUPPORTED_NETWORKS];
void sl_zigbee_af_trust_center_keepalive_tick_network_event_handler(sl_zigbee_af_event_t * event);

typedef struct {
  uint32_t timeOfLastResponse;
  uint16_t jitterTimeSeconds;
  uint16_t  baseTimeSeconds;
  uint8_t failures;
  uint8_t destinationEndpoint;
  bool waitingForResponse;
  sli_zigbee_trust_center_keepalive_state_t state;
  uint8_t lastResponseZclStatus;
  bool tcHasKeepAliveServerCluster;
} sli_zigbee_keepalive_status_struct_t;

static sli_zigbee_keepalive_status_struct_t keepaliveStatusArray[SL_ZIGBEE_SUPPORTED_NETWORKS];

#define keepaliveIsEnabled(statusStruct) (statusStruct->state != STATE_DISABLED)

#define KEEPALIVE_WAIT_TIME_MS (5 * MILLISECOND_TICKS_PER_SECOND)

const sli_zigbee_keepalive_status_struct_t defaultStruct = {
  0xFFFFFFFFUL,
  SL_ZIGBEE_AF_PLUGIN_TRUST_CENTER_KEEPALIVE_DEFAULT_JITTER_PERIOD_SECONDS,
  SL_ZIGBEE_AF_PLUGIN_TRUST_CENTER_KEEPALIVE_DEFAULT_BASE_PERIOD_MINUTES * 60,
  0,      // failures
  0xFF,   // destination endpoint
  false,  // waiting for response
  STATE_NONE,
  SL_ZIGBEE_ZCL_STATUS_SUCCESS,
  true,  // suppose that TC has Keep Alive server cluster
};

//------------------------------------------------------------------------------
// Forward Declarations

static void initiateSearchForNewNetworkWithTrustCenter(void);
static void delayUntilNextKeepalive(void);
static void trustCenterKeepaliveStart(void);

//------------------------------------------------------------------------------

static sli_zigbee_keepalive_status_struct_t* getCurrentStatusStruct(void)
{
  return &(keepaliveStatusArray[sl_zigbee_get_current_network()]);
}

void sl_zigbee_af_trust_center_keepalive_init_cb(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
      sl_zigbee_af_network_event_init(sl_zigbee_af_trust_center_keepalive_tick_network_events,
                                      sl_zigbee_af_trust_center_keepalive_tick_network_event_handler);
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA:
    {
      uint8_t i;
      for (i = 0; i < SL_ZIGBEE_SUPPORTED_NETWORKS; i++) {
        memcpy(&(keepaliveStatusArray[i]), &defaultStruct, sizeof(sli_zigbee_keepalive_status_struct_t));
      }
      break;
    }
  }
}

void sl_zigbee_af_trust_center_keepalive_abort_cb(void)
{
  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();
  // Do not set to standby if not even enabled.
  if (keepaliveIsEnabled(currentStatusStruct)) {
    currentStatusStruct->state = STATE_NONE;
  }
  debug_print("Setting trust center keepalive inactive.");
  sl_zigbee_af_event_set_inactive(sl_zigbee_af_trust_center_keepalive_tick_network_events);
}

void sl_zigbee_af_trust_center_keepalive_stack_status_cb(sl_status_t status)
{
  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();
  if (0 == currentStatusStruct) {
    sl_zigbee_af_security_println("Cannot start keepalive due to invalid memory");
    return;
  }

  if (!keepaliveIsEnabled(currentStatusStruct)) {
    return;
  }

  switch (status) {
    case SL_STATUS_NETWORK_UP:
    case SL_STATUS_ZIGBEE_TRUST_CENTER_SWAP_EUI_HAS_CHANGED:
    case SL_STATUS_ZIGBEE_TRUST_CENTER_SWAP_EUI_HAS_NOT_CHANGED:
      // If we were searching for a network after a succession of keepalive
      // failures, and now we have found the network, reset the keepalive state
      if (currentStatusStruct->state == STATE_INITIATE_NETWORK_SEARCH) {
        currentStatusStruct->state = STATE_NONE;
      }
      trustCenterKeepaliveStart();
      break;
    case SL_STATUS_NETWORK_DOWN:
      sl_zigbee_af_trust_center_keepalive_abort_cb();
      break;
    default:
      break;
  }
}

void sl_zigbee_af_trust_center_keepalive_enable(void)
{
  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();
  if (0 == currentStatusStruct) {
    sl_zigbee_af_security_println("Trust Center Keep Alive: invalid memory");
    return;
  }

  currentStatusStruct->state = STATE_NONE;

  // Start the keep alive process if we are in a joined state already,
  // wait for the stack status callback otherwise.
  sl_status_t status = sl_zigbee_network_state();
  if (status == SL_ZIGBEE_JOINED_NETWORK || status == SL_ZIGBEE_JOINED_NETWORK_NO_PARENT) {
    trustCenterKeepaliveStart();
  }
}

void sl_zigbee_af_trust_center_keepalive_disable(void)
{
  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();
  if (0 == currentStatusStruct) {
    sl_zigbee_af_security_println("Trust Center Keep Alive: invalid memory");
    return;
  }

  currentStatusStruct->state = STATE_DISABLED;
  sl_zigbee_af_trust_center_keepalive_abort_cb();
}

static void trustCenterKeepaliveStart(void)
{
  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();
  uint16_t baseTimeSeconds, jitterTimeSeconds;

  if (sl_zigbee_af_get_node_id() == SL_ZIGBEE_TRUST_CENTER_NODE_ID) {
    // If this code is executing, then the trust center must be alive.
    return;
  }

  if (currentStatusStruct->state != STATE_NONE) {
    return;
  }

  sl_zigbee_current_security_state_t securityState;
  memset(&securityState, 0, sizeof(securityState));
  (void)sl_zigbee_get_current_security_state(&securityState);
  bool in_distributed_network = (securityState.bitmask & SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE);
  if (in_distributed_network) {
    return;
  }

  memcpy(currentStatusStruct, &defaultStruct, sizeof(sli_zigbee_keepalive_status_struct_t));

  // Give a chance to any third-party user to change the check-in times.
  if (sl_zigbee_af_trust_center_keepalive_overwrite_default_timing_cb(&baseTimeSeconds, &jitterTimeSeconds)) {
    currentStatusStruct->baseTimeSeconds = baseTimeSeconds;
    currentStatusStruct->jitterTimeSeconds = jitterTimeSeconds;
  }

  currentStatusStruct->state = STATE_DISCOVER_ENDPOINT;

  delayUntilNextKeepalive();
}

// Here is a hack to randomize without a formal rand(x) function.
// We know that the keepalive jitter can be between 0 and 0x200 seconds.
// Therefore we will just round up to the nearest value that has all 1 bits
// set and thus can give us a mask to our sl_zigbee_get_pseudo_random_number().
// Example:  Round any value between 80 and 179 to 127 (0x007F) and use that
// as our jitter.
// If we go over the jitter amount, then we'll just subtract the random value by
// the jitter amount later.
// Example: Jitter of 80 gets us a random value up to 127. If we get, say, 112,
// we subtract 112 by the max jitter possible (80) to get 42 in this case.

// There is a better way of calculating a good random number but it requires
// several calls to sl_zigbee_get_pseudo_random_number().
// If we get a jitter of 80, first we'll calculate a random number up until its
// next lower mask (63 = 11_1111b), and then add it to a random of the
// the difference (80 - 63 = 17), also masked to the next lowest mask
// (15 = 1111b). Keep repeating that until we get a difference that is
// negligible (like 7 or less).
// Example: jitter is 97
// sl_zigbee_get_pseudo_random_number() & 11_1111b (63) = 47,   difference is 97 - 63 = 34
// sl_zigbee_get_pseudo_random_number() & 1_1111b (31) = 17,    difference is 34 - 31 = 3
// and stop. Sum is 47 + 17 = 64. It's a bit better than the previous method
// but it requires multiple calls to sl_zigbee_get_pseudo_random_number() and it may be
// overkill for what keepalive really needs.

typedef struct {
  uint16_t minValue;
  uint16_t mask;
} sli_zigbee_round_to_mask_struct_t;

static const sli_zigbee_round_to_mask_struct_t roundToMaskArray[] = {
  { 300, (512 - 1) },
  { 180, (256 - 1) },
  { 80, (128 - 1) },
  { 50, (64 -  1) },
  { 20, (32 -  1) },
  { 10, (16 -  1) },
  { 0, (8  -  1) },
};

static uint16_t getInt16uRandomMask(uint16_t value)
{
  uint8_t i;
  for (i = 0; i < sizeof(roundToMaskArray) / sizeof(sli_zigbee_round_to_mask_struct_t); i++) {
    if (value >= roundToMaskArray[i].minValue) {
      return roundToMaskArray[i].mask;
    }
  }
  return 0x000F;
}

static void delayUntilNextKeepalive(void)
{
  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();
  uint32_t randomJitter = (sl_zigbee_get_pseudo_random_number()
                           & getInt16uRandomMask(currentStatusStruct->jitterTimeSeconds));
  // The random mask function may return a value over the value of jitter, since
  // the random function works with masks, so ensure the jitter limit
  if (randomJitter > currentStatusStruct->jitterTimeSeconds) {
    randomJitter -= currentStatusStruct->jitterTimeSeconds;
  }

  uint32_t baseTimeSeconds = currentStatusStruct->baseTimeSeconds;
  uint32_t delayTimeSeconds = baseTimeSeconds + randomJitter;

  sl_zigbee_af_security_println(
    "TC Keep-Alive delay is %d sec (%d sec base + %d sec jitter [%d])",
    delayTimeSeconds,
    currentStatusStruct->baseTimeSeconds,
    randomJitter, currentStatusStruct->jitterTimeSeconds);
  sl_zigbee_af_event_set_delay_ms(sl_zigbee_af_trust_center_keepalive_tick_network_events,
                                  (delayTimeSeconds
                                   * MILLISECOND_TICKS_PER_SECOND));
}

static void messageTimeout(void)
{
  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();

  currentStatusStruct->waitingForResponse = false;
  if (currentStatusStruct->failures != 255) {
    currentStatusStruct->failures++;
  }

  sl_zigbee_af_security_println("ERR: Trust center did not acknowledge "
                                "previous keep-alive signal correctly (timeout"
                                " %d)", currentStatusStruct->failures);

  if (currentStatusStruct->failures >= SL_ZIGBEE_AF_PLUGIN_TRUST_CENTER_KEEPALIVE_FAILURE_LIMIT) {
    sl_zigbee_af_security_println("ERR: keep-alive failure limit reached (%d)",
                                  SL_ZIGBEE_AF_PLUGIN_TRUST_CENTER_KEEPALIVE_FAILURE_LIMIT);

    // The trust-center-keepalive plugin is a multi-purpose plugin used in WWAH as well as on
    // smart energy devices.
    sl_zigbee_beacon_classification_params_t param;
    sl_zigbee_get_beacon_classification_params(&param);
    param.beaconClassificationMask &= ~TC_CONNECTIVITY;   //there is no TC connectivity
    sl_zigbee_set_beacon_classification_params(&param);

    // Another plugin may want to implement its own rejoin algorithm, so see if
    // the callback is consumed
    if (false == sl_zigbee_af_trust_center_keepalive_timeout_cb()) {
      initiateSearchForNewNetworkWithTrustCenter();
    } else {
      delayUntilNextKeepalive();
    }
  } else {
    delayUntilNextKeepalive();
  }
}

static void serviceDiscoveryCallback(const sl_zigbee_af_service_discovery_result_t* result)
{
  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();
  if (currentStatusStruct->state != STATE_DISCOVER_ENDPOINT) {
    return;
  }

  if (result->status == SL_ZIGBEE_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE) {
    const sl_zigbee_af_endpoint_list_t* list = (sl_zigbee_af_endpoint_list_t*)result->responseData;

    // This is the original scenario where we do care the result of the response.
    // Server does support the the Keep-Alive cluster. So we can advance to send
    // the Keep-Alive signals.
    currentStatusStruct->destinationEndpoint = list->list[0];
    sl_zigbee_af_trust_center_keepalive_connectivity_established_cb();
    sli_zigbee_af_send_keepalive_signal();
  } else if (result->status == SL_ZIGBEE_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_EMPTY_RESPONSE) {
    if (sl_zigbee_af_trust_center_keepalive_serverless_is_supported_cb()) {
      // Some third-party users of this mechanism (eg. WWAH) might not care about
      // the actual result of the service discovery only if a response has arrived
      // from the TC.
      // Note, the use case is when the TC cannot include the server side of the
      // cluster for some reason. In this case we just stay with the service
      // discovery (ie. sending Match Descriptor Requests) instead of advancing
      // to send the Keep-Alive signals (ie. Keep-Alive Attribute Read Requests).
      currentStatusStruct->tcHasKeepAliveServerCluster = false;
      currentStatusStruct->failures = 0;
      if (sl_zigbee_af_trust_center_keepalive_serverless_is_enabled_cb()) {
        // Third-party user of this mechanism (eg. WWAH) has the functionality
        // enabled. Verify TC Connectivity and go, setup and delay appropriately.
        sl_zigbee_af_trust_center_keepalive_connectivity_established_cb();
        delayUntilNextKeepalive();
      } else {
        // Third-party user of this mechanism (eg. WWAH) has the functionality
        // disabled at the moment. Not need to do anythin more, go idle.
        sl_zigbee_af_trust_center_keepalive_disable();
      }
    } else {
      // This is the original scenario where we do care the result of the response.
      // Server does not support the Keep-Alive cluster on any endpoints
      // This is not an error until Smart Energy 1.2b is certified and support
      // of the Keep-Alive cluster is made mandatory

      // TC returned no endpoints for those which support Keep-Alive cluster
      // Count this as an error. No islands.
      sl_zigbee_af_security_println("Server not reporting any endpoints that "
                                    "support Keep-Alive cluster");
      messageTimeout();
    }
  } else {
    // This code means we sent a discovery packet but received no response on
    // the channel. Count it as an error. We also classify this as a failure of
    // the keepalive.
    sl_zigbee_af_security_println("Failed to discover Keep-Alive service on "
                                  "Trust Center (0x%02X)", result->status);
    messageTimeout();
  }
}

static void discoveryKeepaliveEndpoint(void)
{
  // Use profile ID wildcard since it's not known what type of endpoint the server will have, unless
  // we're doing BDB, in which case we need to use the HA profile ID
#if defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z3) || defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z4)
  uint16_t profile_id = HA_PROFILE_ID;
#else // SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z3 || SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z4
  uint16_t profile_id = SL_ZIGBEE_WILDCARD_PROFILE_ID;
#endif // SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z3 || SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z4

  sl_status_t status = sl_zigbee_af_find_devices_by_profile_and_cluster(SL_ZIGBEE_TRUST_CENTER_NODE_ID,
                                                                        profile_id,
                                                                        ZCL_KEEPALIVE_CLUSTER_ID,
                                                                        true, // server
                                                                        serviceDiscoveryCallback);
  if (status != SL_STATUS_OK) {
    // We treat even internal errors as a failure of the keepalive.  It is slighly
    // more robust in the belief that if we do trigger a trust center search due to
    // internal failure it is better than never triggering one due to internal failure.
    sl_zigbee_af_security_println("Failed to initiate Keep-Alive service discovery "
                                  "(0x%02X)", status);
    messageTimeout();
    return;
  } // else
    //   Nothing to do.  The service discovery code will return the result or time
    //   out the process.
}

void sl_zigbee_af_trust_center_keepalive_tick_network_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);

  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();
  sl_zigbee_af_event_set_inactive(sl_zigbee_af_trust_center_keepalive_tick_network_events);

  if (currentStatusStruct->waitingForResponse) {
    messageTimeout();
    return;
  }

  switch (currentStatusStruct->state) {
    case STATE_DISCOVER_ENDPOINT:
      discoveryKeepaliveEndpoint();
      break;
    case STATE_SEND_KEEPALIVE_SIGNAL:
      sli_zigbee_af_send_keepalive_signal();
      break;
    default:
      break;
  }
}

void sl_zigbee_af_keepalive_cluster_client_default_response_cb(uint8_t endpoint,
                                                               uint8_t commandId,
                                                               sl_zigbee_af_status_t status)
{
  UNUSED_VAR(endpoint);

  if (commandId == ZCL_READ_ATTRIBUTES_COMMAND_ID) {
    sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();
    currentStatusStruct->waitingForResponse = false;
    currentStatusStruct->lastResponseZclStatus = status;
  }
}

void sli_zigbee_af_send_keepalive_signal(void)
{
  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();
  sl_status_t status;
  uint8_t attributeIds[] = {
    LOW_BYTE(ZCL_KEEPALIVE_BASE_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_KEEPALIVE_BASE_ATTRIBUTE_ID),
    LOW_BYTE(ZCL_KEEPALIVE_JITTER_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_KEEPALIVE_JITTER_ATTRIBUTE_ID)
  };
  uint8_t sourceEndpoint = sl_zigbee_af_primary_endpoint_for_current_network_index();

  sl_zigbee_af_fill_command_global_client_to_server_read_attributes(ZCL_KEEPALIVE_CLUSTER_ID,
                                                                    attributeIds,
                                                                    sizeof(attributeIds));

  // It is possible we will retrieve an undefined endpoint (0xFF) if we rebooted
  // and the TC is not around.  Nonetheless we will still use it as the broadcast
  // endpoint in the hopes that the trust center will respond.
  sl_zigbee_af_set_command_endpoints(sourceEndpoint, currentStatusStruct->destinationEndpoint);

  // The keepalive is an attempt to read a Key Establishment attribute on the
  // trust center.  In general, APS encryption is not required for Key
  // Establishment commands, but it is required by the spec for the keepalive,
  // so the option is explicitly set here.
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_ENCRYPTION;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                             SL_ZIGBEE_TRUST_CENTER_NODE_ID);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_security_println("Failed to send keep-alive signal to trust "
                                  "center endpoint 0x%02X (0x%02X)",
                                  currentStatusStruct->destinationEndpoint,
                                  status);
    // Consider it a failure of the timeout. We can't island ourself.
    messageTimeout();
  }
  currentStatusStruct->state = STATE_SEND_KEEPALIVE_SIGNAL;
  currentStatusStruct->waitingForResponse = (status == SL_STATUS_OK);
  if (status == SL_STATUS_OK) {
    sl_zigbee_af_event_set_delay_ms(sl_zigbee_af_trust_center_keepalive_tick_network_events,
                                    KEEPALIVE_WAIT_TIME_MS);
  } else {
    delayUntilNextKeepalive();
  }
}

void sli_zigbee_af_send_keepalive_now(void)
{
  sl_zigbee_af_event_set_active(sl_zigbee_af_trust_center_keepalive_tick_network_events);
}

void sli_zigbee_af_trust_center_keepalive_read_attributes_response_callback(uint8_t *buffer,
                                                                            uint16_t bufLen)
{
  UNUSED_VAR(bufLen);

  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();
  if (sl_zigbee_af_current_command()->source == SL_ZIGBEE_TRUST_CENTER_NODE_ID
      && currentStatusStruct->state == STATE_SEND_KEEPALIVE_SIGNAL) {
    uint8_t i;
    uint8_t index = 0;
    uint8_t attributesFound = 0;

    currentStatusStruct->waitingForResponse = false;
    currentStatusStruct->lastResponseZclStatus = SL_ZIGBEE_ZCL_STATUS_SUCCESS;

    for (i = 0; i < 2; i++) {
      uint16_t attributeId = HIGH_LOW_TO_INT(buffer[index + 1], buffer[index]);
      uint8_t status = buffer[index + 2];
      // Read 'type' but don't use it
      (void)buffer[index + 3];
      index += (sizeof(attributeId) + sizeof(status) + sizeof(uint8_t));

      if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
        if (attributeId == ZCL_KEEPALIVE_BASE_ATTRIBUTE_ID) {
          currentStatusStruct->baseTimeSeconds = buffer[index] * 60;
          attributesFound++;
          index += sizeof(uint8_t);
        } else if (attributeId == ZCL_KEEPALIVE_JITTER_ATTRIBUTE_ID) {
          currentStatusStruct->jitterTimeSeconds = HIGH_LOW_TO_INT(buffer[index + 1], buffer[index]);
          attributesFound++;
          index += sizeof(currentStatusStruct->jitterTimeSeconds);
        } else {
          sl_zigbee_af_security_println("Keep-Alive readAttribute: unexpected "
                                        "attribute ID 0x%04X", attributeId);
          goto keepaliveReadAttributeFailure;
        }
      } else if (currentStatusStruct->lastResponseZclStatus > SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
        // Only record one of the failures if there are multiple.  This should
        // be good enough as the CLI prints out all the other failures, though
        // only when actively receiving a keepalive.
        currentStatusStruct->lastResponseZclStatus = status;
      }
    }
    if (attributesFound == 2) {
      if (currentStatusStruct->failures != 0) {
        sl_zigbee_af_trust_center_keepalive_connectivity_established_cb();

        // The trust-center-keepalive plugin is a multi-purpose plugin used in WWAH as well as on
        // smart energy devices.
        sl_zigbee_beacon_classification_params_t param;
        sl_zigbee_get_beacon_classification_params(&param);
        param.beaconClassificationMask |= TC_CONNECTIVITY;   //there is now TC connectivity
        sl_zigbee_set_beacon_classification_params(&param);
      }

      currentStatusStruct->failures = 0;
      currentStatusStruct->state = STATE_SEND_KEEPALIVE_SIGNAL;

      // Delay based on the new parameters we got back from the server.
      delayUntilNextKeepalive();
      return;
    }

    keepaliveReadAttributeFailure:
    // Read failures also constitute keepalive errors. We'll just call timeout
    // to handle them for now.
    sl_zigbee_af_security_println("Error in keep-alive response, %d of 2 attributes "
                                  "received.", attributesFound);
    messageTimeout();
  }
}

static void initiateSearchForNewNetworkWithTrustCenter(void)
{
  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();

  sl_status_t status;
  sl_zigbee_af_security_flush();
  sl_zigbee_af_security_println("Initiating trust center search");
  sl_zigbee_af_security_flush();
  status = sl_zigbee_stop_writing_stack_tokens();
  if (status == SL_STATUS_OK) {
    sl_zigbee_beacon_classification_params_t param;
    sl_zigbee_get_beacon_classification_params(&param);
    // Activate TC-connectivity based beacon prioritization, we won't deactivate it later
    param.beaconClassificationMask |= PRIORITIZE_BEACONS_BASED_ON_TC_CONNECTVITY;
    sl_zigbee_set_beacon_classification_params(&param);

    // Allow the upper layer to update the rejoin mask incase needed in this callback.
    // Why do we need this callback - in SE 1.4 CCB 2637 introduced a device type that is slightly
    // different Multi-MAC Selection device called as the Multi-MAC Joining device.
    // The Joining Device shall not change the interface during rejoin but the selection device can.
    // Since this code is in library of the leaf node with the folloiwng callback it would be able
    // update mask for rejoining based on the above device types.
    uint32_t rejoinChannelMask = SL_ZIGBEE_ALL_802_15_4_CHANNELS_MASK;
#ifdef SL_CATALOG_ZIGBEE_PHY_2_4_SUBGHZ_JOINING_END_DEVICE_PRESENT
    rejoinChannelMask = sl_zigbee_internal_update_multi_mac_rejoin_channel_mask_for_selection_or_joining_device_handler(rejoinChannelMask);
#endif //SL_CATALOG_ZIGBEE_PHY_2_4_SUBGHZ_JOINING_END_DEVICE_PRESENT
    status = sl_zigbee_find_and_rejoin_network(false, // TC (unsecured) rejoin
                                               rejoinChannelMask,
                                               SL_ZIGBEE_AF_REJOIN_DUE_TO_TC_KEEPALIVE_FAILURE,
                                               SL_ZIGBEE_DEVICE_TYPE_UNCHANGED);
  } else {
    sl_zigbee_af_security_println("Failed to suspend token writing");
  }

  if (status == SL_STATUS_OK) {
    currentStatusStruct->state = STATE_INITIATE_NETWORK_SEARCH;
  } else {
    sl_zigbee_af_security_println("Could not initiate TC search (0x%02X)", status);
    sl_zigbee_start_writing_stack_tokens();
  }
}

uint8_t sli_zigbee_af_trust_center_keepalive_get_base_time_minutes(void)
{
  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();
  return (uint8_t)(currentStatusStruct->baseTimeSeconds / 60);
}

uint16_t sli_zigbee_af_trust_center_keepalive_get_base_time_seconds(void)
{
  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();
  return currentStatusStruct->baseTimeSeconds;
}

uint16_t sli_zigbee_af_trust_center_keepalive_get_jitter_time_seconds(void)
{
  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();
  return currentStatusStruct->jitterTimeSeconds;
}

bool sli_zigbee_af_trust_center_keepalive_tc_has_server_cluster(void)
{
  sli_zigbee_keepalive_status_struct_t* currentStatusStruct = getCurrentStatusStruct();
  return currentStatusStruct->tcHasKeepAliveServerCluster;
}
