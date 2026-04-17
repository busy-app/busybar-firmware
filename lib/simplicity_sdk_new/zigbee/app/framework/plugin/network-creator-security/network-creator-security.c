/***************************************************************************//**
 * @file
 * @brief Routines for the Network Creator Security plugin, which configures
 *        network permit join state and key policy for a Zigbee 3.0 network.
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
#include "app/framework/util/af-main.h"

#include "network-creator-security.h"
#include "stack/include/zigbee-security-manager.h"
#include "stack/include/network-formation.h"
#include "stack/include/trust-center.h"

#include "app/framework/security/af-security.h"
#include "app/util/zigbee-framework/zigbee-device-common.h" // sl_zigbee_leave_request

#include "network-creator-security-config.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#if (SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SECURITY_ALLOW_HA_DEVICES_TO_STAY == 1)
#define ALLOW_HA_DEVICES_TO_STAY
#endif

bool allowTCRejoinWithWellknownKey = SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SECURITY_ALLOW_TC_REJOIN_WITH_WELL_KNOWN_KEY;
uint16_t allowTCRejoinWithWellknownKeyTimeOut = SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SECURITY_ALLOW_TC_REJOINS_USING_WELL_KNOWN_KEY_TIMEOUT_SEC;

#ifdef EZSP_HOST
  #define setTransientKeyTimeout(timeS) \
  sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_TRANSIENT_KEY_TIMEOUT_S, 2, (uint8_t *)(&timeS));
#else
  #define setTransientKeyTimeout sl_zigbee_set_transient_key_timeout_s
#endif

#ifndef SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SECURITY_NETWORK_OPEN_TIME_S
  #define SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SECURITY_NETWORK_OPEN_TIME_S (300)
#endif

#define NETWORK_OPEN_TIME_S (SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SECURITY_NETWORK_OPEN_TIME_S)

#if defined(ALLOW_HA_DEVICES_TO_STAY)
  #define ALLOW_HA_DEVICES true
#else
  #define ALLOW_HA_DEVICES false
#endif

// -----------------------------------------------------------------------------
// Globals

bool allowHaDevices = ALLOW_HA_DEVICES;
sl_zigbee_key_data_t distributedKey = ZIGBEE_3_DISTRIBUTED_SECURITY_LINK_KEY;
#ifdef SL_CATALOG_ZIGBEE_DIRECT_ZDD_PRESENT
extern void sli_zigbee_direct_zdd_update_keys(sl_zigbee_initial_security_state_t *state);
#endif // SL_CATALOG_ZIGBEE_DIRECT_ZDD_PRESENT

// -----------------------------------------------------------------------------
// Internal Declarations

static void openNetworkNetworkEventHandler(sl_zigbee_af_event_t * event);
static sl_zigbee_af_event_t openNetworkNetworkEvents[SL_ZIGBEE_SUPPORTED_NETWORKS];

static uint16_t openNetworkTimeRemainingS;

// -----------------------------------------------------------------------------
// Framework Callbacks

void sl_zigbee_af_network_creator_security_init_cb(uint8_t init_level)
{
  (void)init_level;

#if defined(EZSP_HOST)
  if (sl_zigbee_get_join_uses_install_code()) {
    sl_status_t status = sl_zigbee_af_set_ezsp_policy(SL_ZIGBEE_EZSP_TRUST_CENTER_POLICY,
                                                      (SL_ZIGBEE_EZSP_DECISION_ALLOW_JOINS | SL_ZIGBEE_EZSP_DECISION_JOINS_USE_INSTALL_CODE_KEY),
                                                      "Trust Center Policy",
                                                      "Joins using install code only");

    if (SL_STATUS_OK != status) {
      sl_zigbee_af_core_println("%s: %s: 0x%02X",
                                SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SECURITY_PLUGIN_NAME,
                                "failed to configure joining using install code only",
                                status);
    }
  }
#endif // EZSP_HOST

#if (SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SECURITY_BDB_JOIN_USES_INSTALL_CODE_KEY == 1) || defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z4)
  sl_zigbee_set_join_uses_install_code(true);
#endif // (SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SECURITY_BDB_JOIN_USES_INSTALL_CODE_KEY == 1) || SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z4

  sl_zigbee_af_network_event_init(openNetworkNetworkEvents,
                                  openNetworkNetworkEventHandler);
}

// TODO: renamed for naming consistency purposes
void sli_zigbee_af_network_creator_security_stack_status_callback(sl_status_t status)
{
#ifdef SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK
  if (status == SL_STATUS_NETWORK_UP
      && sl_zigbee_af_get_node_id() == SL_ZIGBEE_TRUST_CENTER_NODE_ID) {
    sl_zigbee_extended_security_bitmask_t extended;

    // WWAH is what introduced the original requirement for being able to (re)generate arbitrarily many APS link keys.
    // BDB 3.1 should be making it the default behavior
    // unless the TC is intentionally configured to use hashed link key
    if (SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SECURITY_ALLOW_TC_USING_HASHED_LINK_KEY == 1) {
      sl_zigbee_set_trust_center_link_key_request_policy(SL_ZIGBEE_ALLOW_TC_LINK_KEY_REQUEST_AND_SEND_CURRENT_KEY);
    } else {
      sl_zigbee_set_trust_center_link_key_request_policy(SL_ZIGBEE_ALLOW_TC_LINK_KEY_REQUEST_AND_GENERATE_NEW_KEY);
    }

    // This bit is not saved to a token, so make sure that our security bitmask
    // has this bit set on reboot.
    sl_zigbee_get_extended_security_bitmask(&extended);
    extended |= SL_ZIGBEE_NWK_LEAVE_REQUEST_NOT_ALLOWED;
    sl_zigbee_set_extended_security_bitmask(extended);
  }
#else
  (void)status;
#endif /* SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK */
}

// -----------------------------------------------------------------------------
// Stack Callbacks

#if (defined(SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK) || defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT))
static bool isWildcardEui64(sl_802154_long_addr_t eui64)
{
  for (uint8_t i = 0; i < EUI64_SIZE; i++) {
    if (eui64[i] != 0xFF) {
      return false;
    }
  }
  return true;
}
#endif // defined(SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK) || defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)

#if defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)
extern uint8_t sli_zigbee_af_test_harness_z3_server_mask_high;
#endif // defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)

void sl_zigbee_af_network_creator_security_zigbee_key_establishment_cb(sl_802154_long_addr_t eui64,
                                                                       sl_zigbee_key_status_t keyStatus)
{
#if (defined(SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK) || defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT))
  // If we are notified that a joining node failed to verify their
  // TCLK properly, then we are going to kick them off the network,
  // as they pose a potential security hazard.

#if defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)
  // Do nothing if we are pretending to be r20 or lower.
  if (sli_zigbee_af_test_harness_z3_server_mask_high == 0) {
    return;
  }
#endif // defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)

  if (!isWildcardEui64(eui64)
      && ((keyStatus == SL_ZIGBEE_TC_REQUESTER_VERIFY_KEY_TIMEOUT && !allowHaDevices)
          || keyStatus == SL_ZIGBEE_TC_REQUESTER_VERIFY_KEY_FAILURE)) {
    sl_status_t status = SL_STATUS_NOT_FOUND;
    sl_802154_short_addr_t destinationId;
    status = sl_zigbee_lookup_node_id_by_eui64(eui64, &destinationId);
    if (status == SL_STATUS_OK) {
      // We do not tell the node to rejoin, since they may be malicious.
      status = sl_zigbee_leave_request(destinationId,
                                       eui64,
                                       0, // no leave flags
                                       (SL_ZIGBEE_APS_OPTION_RETRY
                                        | SL_ZIGBEE_APS_OPTION_ENABLE_ROUTE_DISCOVERY));
    }
    sl_zigbee_af_core_println("%s: Remove node 0x%04X for failed key verification: 0x%02X",
                              SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SECURITY_PLUGIN_NAME,
                              destinationId,
                              status);
  }
#else
  (void)eui64;
  (void)keyStatus;
#endif // defined(SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK) || defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
extern bool sl_zigbee_af_network_creator_security_get_key_callback(sl_zigbee_key_data_t * keyData);
#endif
// -----------------------------------------------------------------------------
// API

sl_status_t sl_zigbee_af_network_creator_security_start(bool centralizedNetwork)
{
  sl_zigbee_initial_security_state_t state;
  sl_zigbee_extended_security_bitmask_t extended;
  sl_status_t status = SL_STATUS_OK;

  memset(&state, 0, sizeof(state));
  state.bitmask = (SL_ZIGBEE_TRUST_CENTER_GLOBAL_LINK_KEY
                   | SL_ZIGBEE_HAVE_PRECONFIGURED_KEY
                   | SL_ZIGBEE_HAVE_NETWORK_KEY
                   | SL_ZIGBEE_NO_FRAME_COUNTER_RESET
                   | SL_ZIGBEE_REQUIRE_ENCRYPTED_KEY);

  extended = SL_ZIGBEE_JOINER_GLOBAL_LINK_KEY | SL_ZIGBEE_R18_STACK_BEHAVIOR;

  if (!centralizedNetwork) {
    memmove(&(state.preconfiguredKey),
            sl_zigbee_key_contents(&distributedKey),
            SL_ZIGBEE_ENCRYPTION_KEY_SIZE);

    // Use distributed trust center mode.
    state.bitmask |= SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE;
  }
#if (defined(SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK) || defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT))
  else { // centralizedNetwork
    // Generate a random global link key.
    // This is the key the trust center will send to a joining node when it
    // updates its link key.
    status = sl_zigbee_af_generate_random_key(&(state.preconfiguredKey));
    if (status != SL_STATUS_OK) {
      goto kickout;
    }

    // Use hashed link keys for improved storage and speed.
    state.bitmask |= SL_ZIGBEE_TRUST_CENTER_USES_HASHED_LINK_KEY;

    // Tell the trust center to ignore leave requests.
    extended |= SL_ZIGBEE_NWK_LEAVE_REQUEST_NOT_ALLOWED;
  }
#else
  else { // centralizedNetwork
    // in case device doesn't support centralized network we should return SL_STATUS_INVALID_STATE immediately
    status = SL_STATUS_INVALID_STATE;
    goto kickout;
  }
#endif // defined(SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK) || defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
  if (!sl_zigbee_af_network_creator_security_get_key_callback(&state.networkKey)) {
    // Generate a random network key if the user did not provide a key
    status = sl_zigbee_af_generate_random_key(&(state.networkKey));
    if (status != SL_STATUS_OK) {
      goto kickout;
    }
  }
#else
  status = sl_zigbee_af_generate_random_key(&(state.networkKey));
  if (status != SL_STATUS_OK) {
    goto kickout;
  }
#endif

#ifdef SL_CATALOG_ZIGBEE_DIRECT_ZDD_PRESENT
  sli_zigbee_direct_zdd_update_keys(&state);
#endif // SL_CATALOG_ZIGBEE_DIRECT_ZDD_PRESENT
  // Set the initial security data.
  status = sl_zigbee_set_initial_security_state(&state);
  if (status != SL_STATUS_OK) {
    goto kickout;
  }
  status = sl_zigbee_set_extended_security_bitmask(extended);

  kickout:
  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SECURITY_PLUGIN_NAME,
                            "Start",
                            status);
  return status;
}

sl_status_t sl_zigbee_af_network_creator_security_open_network(void)
{
  if (sl_zigbee_get_join_uses_install_code()) {
    sl_zigbee_af_core_println("open-network not permitted when install code joins are required");
    return SL_STATUS_INVALID_STATE;
  }
  sl_status_t status = SL_STATUS_OK;
  sl_zigbee_current_security_state_t securityState;

  if (sl_zigbee_af_network_state() != SL_ZIGBEE_JOINED_NETWORK) {
    return SL_STATUS_FAIL;
  }

  sl_zigbee_get_current_security_state(&securityState);
  // For a distributed network, don't store the ZIGBEE_3_CENTRALIZED_SECURITY_LINK_KEY
  // in the transient link key table.
  if (!(securityState.bitmask & SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE)) {
    if (allowTCRejoinWithWellknownKey == true) {
      sl_zigbee_set_tc_rejoins_using_well_known_key_timeout_sec(allowTCRejoinWithWellknownKeyTimeOut);
      sl_zigbee_set_tc_rejoins_using_well_known_key_allowed(true);
    }

    #if (defined(SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK) || defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT))
    if (sl_zigbee_af_get_node_id() == SL_ZIGBEE_TRUST_CENTER_NODE_ID) {
      sl_802154_long_addr_t wildcardEui64 = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, };
      sl_zigbee_key_data_t centralizedKey = ZIGBEE_3_CENTRALIZED_SECURITY_LINK_KEY;
      uint16_t transientKeyTimeout = (NETWORK_OPEN_TIME_S > SL_ZIGBEE_TRANSIENT_KEY_TIMEOUT_S)
                                     ? NETWORK_OPEN_TIME_S
                                     : SL_ZIGBEE_TRANSIENT_KEY_TIMEOUT_S;
      setTransientKeyTimeout(transientKeyTimeout);
      status = sl_zigbee_sec_man_import_transient_key(wildcardEui64,
                                                      (sl_zigbee_sec_man_key_t*)&centralizedKey);
    }
  #endif // defined(SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK) || defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)
  }

  if (status == SL_STATUS_OK) {
    openNetworkTimeRemainingS = NETWORK_OPEN_TIME_S;
    sl_zigbee_af_event_set_active(openNetworkNetworkEvents);
  }

  return status;
}

sl_status_t sl_zigbee_af_network_creator_security_close_network(void)
{
  sl_status_t status = SL_STATUS_FAIL;

  if (sl_zigbee_af_network_state() == SL_ZIGBEE_JOINED_NETWORK) {
    sl_zigbee_clear_transient_link_keys();
    sl_zigbee_af_event_set_inactive(openNetworkNetworkEvents);
    zaTrustCenterSetJoinPolicy(SL_ZIGBEE_ALLOW_REJOINS_ONLY);
    status = sl_zigbee_af_permit_join(0, true); // broadcast
  }

  return status;
}

sl_status_t sl_zigbee_af_network_creator_security_open_network_with_key_pair(sl_802154_long_addr_t eui64,
                                                                             sl_zigbee_key_data_t keyData)
{
  sl_status_t status = SL_STATUS_OK;

  if (sl_zigbee_af_network_state() != SL_ZIGBEE_JOINED_NETWORK) {
    return SL_STATUS_FAIL;
  }

#if (defined(SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK) || defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT))
  if (sl_zigbee_af_get_node_id() == SL_ZIGBEE_TRUST_CENTER_NODE_ID) {
    uint16_t transientKeyTimeout = (NETWORK_OPEN_TIME_S > SL_ZIGBEE_TRANSIENT_KEY_TIMEOUT_S)
                                   ? NETWORK_OPEN_TIME_S
                                   : SL_ZIGBEE_TRANSIENT_KEY_TIMEOUT_S;
    setTransientKeyTimeout(transientKeyTimeout);
    status = sl_zigbee_sec_man_import_transient_key(eui64,
                                                    (sl_zigbee_sec_man_key_t*)&keyData);
  }
#else
  (void)eui64;
  (void)keyData;
#endif // defined(SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK) || defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)

  if (status == SL_STATUS_OK) {
    openNetworkTimeRemainingS = NETWORK_OPEN_TIME_S;
    sl_zigbee_af_event_set_active(openNetworkNetworkEvents);
  }

  return status;
}

// -----------------------------------------------------------------------------
// Internal Definitions

static void openNetworkNetworkEventHandler(sl_zigbee_af_event_t * event)
{
  (void)event;
  sl_status_t status = SL_STATUS_OK;
  uint8_t permitJoinTime;

  sl_zigbee_af_event_set_inactive(openNetworkNetworkEvents);

  // If we have left the network, then we don't need to proceed further.
  if (sl_zigbee_af_network_state() != SL_ZIGBEE_JOINED_NETWORK) {
    return;
  }

  if (openNetworkTimeRemainingS > SL_ZIGBEE_AF_PERMIT_JOIN_MAX_TIMEOUT) {
    permitJoinTime = SL_ZIGBEE_AF_PERMIT_JOIN_MAX_TIMEOUT;
    openNetworkTimeRemainingS -= SL_ZIGBEE_AF_PERMIT_JOIN_MAX_TIMEOUT;
    sl_zigbee_af_event_set_delay_qs(openNetworkNetworkEvents,
                                    (SL_ZIGBEE_AF_PERMIT_JOIN_MAX_TIMEOUT << 2));
  } else {
    permitJoinTime = openNetworkTimeRemainingS;
    openNetworkTimeRemainingS = 0;
  }

#if defined(EZSP_HOST)
  sl_zigbee_ezsp_decision_id_t decision;
  sl_status_t eszpStatus = sl_zigbee_ezsp_get_policy(SL_ZIGBEE_EZSP_TRUST_CENTER_POLICY,
                                                     &decision);
  if (eszpStatus == SL_ZIGBEE_EZSP_SUCCESS && !(decision & SL_ZIGBEE_EZSP_DECISION_DEFER_JOINS)) {
    sl_zigbee_ezsp_decision_bitmask_t policy = (SL_ZIGBEE_EZSP_DECISION_ALLOW_JOINS | SL_ZIGBEE_EZSP_DECISION_ALLOW_UNSECURED_REJOINS);
    if (sl_zigbee_get_join_uses_install_code()) {
      policy |= SL_ZIGBEE_EZSP_DECISION_JOINS_USE_INSTALL_CODE_KEY;
    }
    sl_zigbee_af_set_ezsp_policy(SL_ZIGBEE_EZSP_TRUST_CENTER_POLICY,
                                 policy,
                                 "Trust Center Policy",
                                 "Allow preconfigured key joins");
  }
#else // !EZSP_HOST
  zaTrustCenterSetJoinPolicy(SL_ZIGBEE_USE_PRECONFIGURED_KEY);
#endif // EZSP_HOST
  status = sl_zigbee_af_permit_join(permitJoinTime, true);   // broadcast permit join

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SECURITY_PLUGIN_NAME,
                            "Open network",
                            status);
}
