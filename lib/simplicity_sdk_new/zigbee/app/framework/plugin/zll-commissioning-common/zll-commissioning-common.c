/***************************************************************************//**
 * @file
 * @brief Routines for the ZLL Commissioning Common plugin, which defines
 *        functions common to both server and client sides of the ZLL protocol.
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
#include "app/framework/util/common.h"
#include "zll-commissioning-common.h"
#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
#define DEFINETYPES
#endif
#include "stack/config/sl_zigbee_token_defines.h"
#include "stack/include/sl_zigbee_token.h"

// The code below assumes that there is exactly one network and that it is
// ZigBee PRO.
#if SL_ZIGBEE_SUPPORTED_NETWORKS != 1
  #error ZLL is not supported with multiple networks.
#endif

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_NETWORK_CREATOR_PRESENT
#define NETWORK_CREATOR_PRESENT
#include "network-creator.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_CLIENT_PRESENT
#include "zll-commissioning-client-config.h"
#define ZLL_COMMISSIONING_CLIENT_PRESENT
#endif
#ifdef SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_SERVER_PRESENT
#define ZLL_COMMISSIONING_SERVER_PRESENT
#endif
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
#define TEST_HARNESS_Z3_PRESENT
#endif

#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
#include "app/test/pro-compliance.h"
#endif

//------------------------------------------------------------------------------
// Globals

#define isFactoryNew(state) ((state) & SL_ZIGBEE_ZLL_STATE_FACTORY_NEW)

// The target network - used by both client and server sides, the latter mainly for
// the touchlink complete callback to the application.
sl_zigbee_zll_network_t sli_zigbee_af_zll_network;

#ifdef ZLL_COMMISSIONING_CLIENT_PRESENT
// Sub-device info (mainly for client, but server needs to initialize the count)
sl_zigbee_zll_device_info_record_t sli_zigbee_af_zll_sub_devices[SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT_SUB_DEVICE_TABLE_SIZE];
uint8_t sli_zigbee_af_zll_sub_device_count = 0;
#endif // ZLL_COMMISSIONING_CLIENT_PRESENT

// The module state for both client and server.
uint16_t sli_zigbee_af_zll_flags = INITIAL;

// TL Rejoin stage allows a fixed number of retries (in case Beacon responses are missing).
#define INITIATOR_REJOIN_MAX_RETRIES   4
uint8_t sli_zigbee_af_initiator_rejoin_retry_count = 0;

#ifdef PLUGIN_DEBUG
static const uint8_t sli_zigbee_af_zll_commissioning_plugin_name[] = "ZLL Commissioning Common";
#define PLUGIN_NAME sli_zigbee_af_zll_commissioning_plugin_name
#endif

#ifdef TEST_HARNESS_Z3_PRESENT
extern sli_zigbee_event_t sli_zigbee_leave_event;
#endif

// Private ZLL commissioning functions
void sli_zigbee_af_zll_finish_network_formation_for_router(sl_status_t status);
void sli_zigbee_af_zll_abort_touch_link(sl_zigbee_af_zll_commissioning_status_t reason);
void sli_zigbee_af_zll_stack_status(sl_status_t status);
void sli_zigbee_af_zll_initialize_radio(void);
bool sli_zigbee_af_zll_stealing_allowed(void);
bool sli_zigbee_af_zll_remote_reset_allowed(void);

// Forward references
bool sli_zigbee_af_zll_am_factory_new(void);
#ifdef EZSP_HOST
void sl_zigbee_af_zll_commissioning_common_ncp_init_cb(void);
#else
void sl_zigbee_af_zll_commissioning_common_init_cb(uint8_t init_level);
#endif

//------------------------------------------------------------------------------
// Module private functions

static void setProfileInteropState(void)
{
  sl_zigbee_tok_type_stack_zll_data_t token;

  sl_zigbee_zll_get_token_stack_zll_data(&token);
  token.bitmask |= SL_ZIGBEE_ZLL_STATE_PROFILE_INTEROP;
  sl_zigbee_zll_set_token_stack_zll_data(&token);
}

static void initFactoryNew(void)
{
  // We use the node type token to indicate if we have joined a network and thus
  tokTypeStackNodeData tokNode;
  (void)sl_token_manager_get_data(COMMON_TOKEN_STACK_NODE_DATA,
                                  (void *)&tokNode,
                                  sizeof(tokTypeStackNodeData));

  // The initialization is only performed if we are factory new in the BDB sense,
  // i.e. not joined to a centralized or distributed network.
  if (sli_zigbee_af_zll_am_factory_new() && (tokNode.nodeType == SL_ZIGBEE_UNKNOWN_DEVICE)) {
    sl_zigbee_af_app_println("ZllCommInit - device is not joined to a network");

    // Set the default ZLL node type for both client and server, for Scan Request
    // and Scan Response messages respectively.
    sl_zigbee_set_zll_node_type((sli_zigbee_af_current_zigbee_pro_network->nodeType
                                 == SL_ZIGBEE_COORDINATOR)
                                ? SL_ZIGBEE_ROUTER
                                : sli_zigbee_af_current_zigbee_pro_network->nodeType);

#ifdef ZLL_COMMISSIONING_SERVER_PRESENT
    sli_zigbee_af_zll_initialize_radio();
#endif

#ifdef ZLL_COMMISSIONING_CLIENT_PRESENT
    // Set the address assignment capability bit to true for a client in all cases.
    sl_zigbee_set_zll_additional_state(SL_ZIGBEE_ZLL_STATE_ADDRESS_ASSIGNMENT_CAPABLE);
#endif

#ifdef ZIGBEE3_SUPPORT
    // Set interop bit here if we support Zigbee 3.0.
    setProfileInteropState();
#endif

    // Set the security state here, in case of incoming scan requests, to ensure
    // that our security key bitmask matches that of the initiator, or in case
    // we initiate a touchlink after joining by classical commissioning.
    sl_zigbee_af_zll_set_initial_security_state();

    // Set the default power for a new network in all cases.
    sl_zigbee_set_radio_power(SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_RADIO_TX_POWER);
  }
}

static void completeResetToFactoryNew(void)
{
  sl_zigbee_af_app_println("Resetting to factory new");
  sl_zigbee_af_reset_attributes(SL_ZIGBEE_BROADCAST_ENDPOINT);
  sl_zigbee_af_groups_cluster_clear_group_table_cb(SL_ZIGBEE_BROADCAST_ENDPOINT);
  sl_zigbee_af_scenes_cluster_clear_scene_table_cb(SL_ZIGBEE_BROADCAST_ENDPOINT);
  sl_zigbee_clear_binding_table();
#ifdef EZSP_HOST
  sl_zigbee_af_zll_commissioning_common_ncp_init_cb();
#else
  // TODO: fix this once we port the init callback
  sl_zigbee_af_zll_commissioning_common_init_cb(SL_ZIGBEE_INIT_LEVEL_EVENT);
#endif
  sl_zigbee_af_zll_commissioning_common_reset_to_factory_new_cb();
  sli_zigbee_af_zll_flags = INITIAL;
}

//------------------------------------------------------------------------------
// ZLL commissioning private functions

bool sli_zigbee_af_zll_am_factory_new(void)
{
  sl_zigbee_tok_type_stack_zll_data_t token;
  sl_zigbee_zll_get_token_stack_zll_data(&token);
  return isFactoryNew(token.bitmask);
}

void sli_zigbee_af_zll_touch_link_complete(void)
{
  sl_zigbee_node_type_t nodeType;
  sl_zigbee_network_parameters_t parameters;
  sl_zigbee_af_get_network_parameters(&nodeType, &parameters);
  sli_zigbee_af_zll_network.zigbeeNetwork.channel = parameters.radioChannel;
  sli_zigbee_af_zll_network.zigbeeNetwork.panId = parameters.panId;
  memmove(sli_zigbee_af_zll_network.zigbeeNetwork.extendedPanId,
          parameters.extendedPanId,
          EXTENDED_PAN_ID_SIZE);
  sli_zigbee_af_zll_network.zigbeeNetwork.nwkUpdateId = parameters.nwkUpdateId;
#ifdef ZLL_COMMISSIONING_CLIENT_PRESENT
  sl_zigbee_af_zll_commissioning_common_touch_link_complete_cb(&sli_zigbee_af_zll_network,
                                                               sli_zigbee_af_zll_sub_device_count,
                                                               (sli_zigbee_af_zll_sub_device_count == 0
                                                                ? NULL
                                                                : sli_zigbee_af_zll_sub_devices));
#else
  sl_zigbee_af_zll_commissioning_common_touch_link_complete_cb(&sli_zigbee_af_zll_network, 0, NULL);
#endif // ZLL_COMMISSIONING_CLIENT_PRESENT

  // Update module state after the callback call.
  sli_zigbee_af_zll_flags = INITIAL;
}

sl_zigbee_node_type_t sli_zigbee_af_zll_get_logical_node_type(void)
{
  sl_zigbee_node_type_t nodeType;
  sl_status_t status = sl_zigbee_af_get_node_type(&nodeType);

  // Note, we only report as a coordinator if we are a currently
  // coordinator on a centralized network.
  if (status == SL_STATUS_NOT_JOINED) {
    nodeType = sli_zigbee_af_current_zigbee_pro_network->nodeType;
    if (nodeType == SL_ZIGBEE_COORDINATOR) {
      nodeType = SL_ZIGBEE_ROUTER;
    }
  }
  return nodeType;
}

// The following function is not required during standard touchlink commissioning,
// but is retained to support the following CLI command (which may be used
// either client or server side), in particular for ZTT/ZUTH:
//   plugin zll-commissioning form
// Note that it WILL now result in a 'ZLL' distributed network being created, i.e.,
// a network appropriate to a address-assignment-capable touchlink initiator,
// with address and group ranges assigned to it.
sl_status_t sli_zigbee_af_zll_form_network(uint8_t channel, int8_t power, sl_802154_pan_id_t panId)
{
  // Create a distributed commissioning network using the ZLL link key.
  sl_zigbee_zll_network_t network;
  memset(&network, 0, sizeof(sl_zigbee_zll_network_t));
  network.zigbeeNetwork.channel = channel;
  network.zigbeeNetwork.panId = panId;
  sl_zigbee_af_get_form_and_join_extended_pan_id_cb(network.zigbeeNetwork.extendedPanId);
  sl_zigbee_tok_type_stack_zll_data_t token;
  sl_zigbee_zll_get_token_stack_zll_data(&token);
  network.state = token.bitmask & 0xffff;
  network.nodeType = sli_zigbee_af_zll_get_logical_node_type();
  sl_zigbee_af_zll_set_initial_security_state();
  sl_status_t status = sl_zigbee_zll_form_network(&network, power);
  debugPrintln("%s: sl_zigbee_zll_form_network - status = %02X, node type = %d", PLUGIN_NAME, status, network.nodeType);
  if (status == SL_STATUS_OK) {
    sli_zigbee_af_zll_flags |= FORMING_NETWORK;
  }
  return status;
}

sl_zigbee_zll_policy_t sli_zigbee_af_zll_get_policy(void)
{
  sl_zigbee_zll_policy_t policy;
#ifdef EZSP_HOST
  if (sl_zigbee_ezsp_get_policy(SL_ZIGBEE_EZSP_ZLL_POLICY, &policy) != SL_ZIGBEE_EZSP_SUCCESS) {
    policy = SL_ZIGBEE_ZLL_POLICY_DISABLED;
  }
#else
  policy = sl_zigbee_zll_get_policy();
#endif
  return policy;
}

//------------------------------------------------------------------------------
// Public functions

void sl_zigbee_af_zll_commissioning_common_init_cb(uint8_t init_level)
{
#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
  if (isZCPContext() == true) {
    // If this is running as a pro-compliance app, we should return here
    // to avoid overwritting the existing security with ZLL commissioning values
    // e.g the security bitmask, TC link key
    return;
  }
#endif
  (void)init_level;

#ifndef EZSP_HOST
  // Set the policy for both server and client.
  sl_zigbee_zll_policy_t policy = SL_ZIGBEE_ZLL_POLICY_ENABLED;
#ifdef ZLL_COMMISSIONING_SERVER_PRESENT
  policy |= SL_ZIGBEE_ZLL_POLICY_TARGET;
  if (sli_zigbee_af_zll_stealing_allowed()) {
    policy |= SL_ZIGBEE_ZLL_POLICY_STEALING_ENABLED;
  }
  if (sli_zigbee_af_zll_remote_reset_allowed()) {
    policy |= SL_ZIGBEE_ZLL_POLICY_REMOTE_RESET_ENABLED;
  }
#endif
  sl_zigbee_zll_set_policy(policy);

  // Set the primary and secondary channel masks for both server and client.
  sl_zigbee_set_zll_primary_channel_mask(SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_PRIMARY_CHANNEL_MASK);
#ifdef SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_SECONDARY_CHANNEL_MASK
  sl_zigbee_set_zll_secondary_channel_mask(SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_SECONDARY_CHANNEL_MASK);
#endif

  // Factory new initialization
  initFactoryNew();
#endif // #ifndef EZSP_HOST
}

void sl_zigbee_af_zll_commissioning_common_ncp_init_cb(void)
{
#ifdef EZSP_HOST
  // Set the policy for both server and client.
  sl_zigbee_zll_policy_t policy = SL_ZIGBEE_ZLL_POLICY_ENABLED;
#ifdef ZLL_COMMISSIONING_SERVER_PRESENT
  policy |= SL_ZIGBEE_ZLL_POLICY_TARGET;
  if (sli_zigbee_af_zll_stealing_allowed()) {
    policy |= SL_ZIGBEE_ZLL_POLICY_STEALING_ENABLED;
  }
  if (sli_zigbee_af_zll_remote_reset_allowed()) {
    policy |= SL_ZIGBEE_ZLL_POLICY_REMOTE_RESET_ENABLED;
  }
#endif
  sl_zigbee_af_set_ezsp_policy(SL_ZIGBEE_EZSP_ZLL_POLICY,
                               policy,
                               "ZLL policy",
                               "enable");

  // Set the primary and secondary channel masks for both server and client.
  sl_zigbee_set_zll_primary_channel_mask(SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_PRIMARY_CHANNEL_MASK);
#ifdef SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_SECONDARY_CHANNEL_MASK
  sl_zigbee_set_zll_secondary_channel_mask(SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_SECONDARY_CHANNEL_MASK);
#endif

  // Factory new initialization
  initFactoryNew();
#endif // #ifdef EZSP_HOST
}

sl_status_t sl_zigbee_af_zll_set_initial_security_state(void)
{
  sl_zigbee_key_data_t networkKey;
  sl_zigbee_zll_initial_security_state_t securityState = {
    0, // bitmask - unused
    SL_ZIGBEE_ZLL_KEY_INDEX_CERTIFICATION,
    { SL_ZIGBEE_ZLL_CERTIFICATION_ENCRYPTION_KEY },
    { SL_ZIGBEE_ZLL_CERTIFICATION_PRECONFIGURED_LINK_KEY },
  };
  sl_status_t status;

  // We can only initialize security information while not on a network - this
  // also covers the case where we are joined as a coordinator.
  if (sl_zigbee_af_network_state() != SL_ZIGBEE_NO_NETWORK) {
    return SL_STATUS_OK;
  }

  status = sl_zigbee_af_generate_random_key(&networkKey);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_app_println("%s%s failed 0x%02X",
                             "Error: ",
                             "Generating random key",
                             status);
    return status;
  }

  sl_zigbee_af_zll_commissioning_common_initial_security_state_cb(&securityState);
  status = sl_zigbee_zll_set_initial_security_state(&networkKey, &securityState);

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_app_println("%s%s failed 0x%02X",
                             "Error: ",
                             "Initializing security",
                             status);
  }
  return status;
}

void sl_zigbee_af_zll_reset_to_factory_new(void)
{
  // The leave will cause the ZLL state to be set to 'factory new',
  // but after a short delay.
  sli_zigbee_af_zll_flags |= RESETTING_TO_FACTORY_NEW;

  debugPrintln("sl_zigbee_af_zll_reset_to_factory_new - flags = %02X, networkState = %02X", sli_zigbee_af_zll_flags, sl_zigbee_af_network_state());

  // Note that we won't get a network down stack status if we
  // are currently trying to join - the leave will complete silently.
  bool silentLeave = (sl_zigbee_af_network_state() == SL_ZIGBEE_JOINING_NETWORK) ? true : false;
  sl_status_t status = sl_zigbee_leave_network(SL_ZIGBEE_LEAVE_NWK_WITH_NO_OPTION);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_app_println("Error: Failed to leave network, status: 0x%02X", status);
  }

  // Complete the reset immediately if we're not expecting a network down status.
  if (silentLeave || status != SL_STATUS_OK) {
    sl_zigbee_zll_clear_tokens();
    completeResetToFactoryNew();
  }
#ifdef TEST_HARNESS_Z3_PRESENT
  else if (status == SL_STATUS_OK && !touchLinkTarget()) {
    // Complete the leave immediately without the usual delay for a local reset
    // (this is to accommodate ZTT scripts which issue a 'network leave'
    // which is immediately followed by a 'reset').
    sli_zigbee_event_set_active(&sli_zigbee_leave_event);
  }
#endif
}

// TODO: renamed for naming consistency purposes
void sli_zigbee_af_zll_commissioning_common_stack_status_callback(sl_status_t status)
{
  // If we are forming a network for a router initiator, then we handle
  // this status separately.
  // During touch linking, SL_STATUS_NETWORK_UP means the process is complete.  Any
  // other status, unless we're busy joining or rejoining, means that the touch
  // link failed.
  debugPrintln("%s: ZllCommStackStatus: status = %02X, flags = %02X", PLUGIN_NAME, status, sli_zigbee_af_zll_flags);

#if defined(ZLL_COMMISSIONING_CLIENT_PRESENT) && defined(NETWORK_CREATOR_PRESENT)
  if (formingNetwork()) {
    sli_zigbee_af_zll_finish_network_formation_for_router(status);
  } else
#endif

  if (resettingToFactoryNew()) { // either local or remote (TOUCH_LINK_TARGET set)
    // A reset overrides a touchlink in progress.
    if (status == SL_STATUS_NETWORK_DOWN) {
      completeResetToFactoryNew();
    }
  } else if (touchLinkInProgress()) { // including TOUCH_LINK_TARGET
    if (status == SL_STATUS_NETWORK_UP) {
      sli_zigbee_af_zll_touch_link_complete();
    } else if (status == SL_STATUS_NETWORK_DOWN) {
      // We don't do anything here for a network down.
    } else {
      sl_zigbee_af_app_println("%s%s%s: status = %02X, flags = %02X",
                               "Error: ",
                               "Touch linking failed: ",
                               "joining failed",
                               status, sli_zigbee_af_zll_flags);
#ifdef ZLL_COMMISSIONING_CLIENT_PRESENT
      if (!touchLinkTarget()) {
        if ((sl_zigbee_af_network_state() == SL_ZIGBEE_JOINED_NETWORK_NO_PARENT)
            && (sli_zigbee_af_initiator_rejoin_retry_count < INITIATOR_REJOIN_MAX_RETRIES)) {
          // The TL initiator has joined the target's Pan but the final Rejoin
          // has failed (e.g. if the target fails to send a Beacon Response
          // during the rejoin).  We allow a number of Rejoin retries here.
          ++sli_zigbee_af_initiator_rejoin_retry_count;
          sl_zigbee_find_and_rejoin_network((true), 0,
                                            SL_ZIGBEE_REJOIN_DUE_TO_APP_EVENT_1,
                                            SL_ZIGBEE_DEVICE_TYPE_UNCHANGED);
        } else {
          sli_zigbee_af_zll_abort_touch_link(SL_ZIGBEE_AF_ZLL_JOINING_FAILED);
        }
      }
#endif
    }
  } else {
    // Here we catch all fresh non-ZLL network joins, and set the ZLL state accordingly.
    sl_zigbee_tok_type_stack_zll_data_t token;
    sl_zigbee_zll_get_token_stack_zll_data(&token);

    if (status == SL_STATUS_NETWORK_UP && (token.bitmask & SL_ZIGBEE_ZLL_STATE_FACTORY_NEW)) {
      // When either a router or an end device joins a non-ZLL network, it is
      // no longer factory new.  On a non-ZLL network, ZLL devices that are
      // normally address assignment capable do not have free network or group
      // addresses nor do they have a range of group addresses for themselves.
      // (Note, to ensure that ZLL devices will always operate as ZigBee 3.0 applications,
      // we need to set the ZLL profile interop bit even when the application
      // joins a classical ZigBee network. This way, if the device is stolen from
      // a classical ZigBee network to a ZLL network, it will operate as a
      // ZigBee 3.0 device. This is now set at plugin initialization time.)
      token.bitmask &= ~SL_ZIGBEE_ZLL_STATE_FACTORY_NEW;
      token.freeNodeIdMin = token.freeNodeIdMax = SL_ZIGBEE_ZLL_NULL_NODE_ID;
      token.myGroupIdMin = SL_ZIGBEE_ZLL_NULL_GROUP_ID;
      token.freeGroupIdMin = token.freeGroupIdMax = SL_ZIGBEE_ZLL_NULL_GROUP_ID;
      sl_zigbee_zll_set_token_stack_zll_data(&token);
      sl_zigbee_zll_set_non_zll_network();
    }
    // Otherwise, we just ignore the status, for example, a network up
    // from a rejoin or a join at startup.
  }
}

void sl_zigbee_af_zll_unset_factory_new(void)
{
  sl_zigbee_tok_type_stack_zll_data_t token;
  sl_zigbee_zll_get_token_stack_zll_data(&token);
  if (token.bitmask & SL_ZIGBEE_ZLL_STATE_FACTORY_NEW) {
    token.bitmask &= ~SL_ZIGBEE_ZLL_STATE_FACTORY_NEW;
    sl_zigbee_zll_set_token_stack_zll_data(&token);
  }
}

bool sl_zigbee_af_zll_touch_link_in_progress(void)
{
  // Returns true if a touch link is in progress or false otherwise.

  return touchLinkInProgress();
}

// Note, this will cause *all* touchlink interpan messages to be dropped,
// not just touchlink requests, so it will effectively disable touchlink
// initiator, as well as touchlink target.
sl_status_t sl_zigbee_af_zll_disable(void)
{
  sl_zigbee_zll_policy_t policy = sli_zigbee_af_zll_get_policy();
  sl_status_t status;
#ifndef EZSP_HOST
  status = sl_zigbee_zll_set_policy(policy & ~SL_ZIGBEE_ZLL_POLICY_ENABLED);
#else
  status = sl_zigbee_af_set_ezsp_policy(SL_ZIGBEE_EZSP_ZLL_POLICY,
                                        policy & ~SL_ZIGBEE_ZLL_POLICY_ENABLED,
                                        "ZLL processing",
                                        "disable");
#endif
  return status;
}

sl_status_t sl_zigbee_af_zll_enable(void)
{
  sl_zigbee_zll_policy_t policy = sli_zigbee_af_zll_get_policy();
  sl_status_t status;

  // Re-enable stealing and remote reset, if we have server-side, and the plugin option permits it.
  policy |= SL_ZIGBEE_ZLL_POLICY_ENABLED;
#ifdef ZLL_COMMISSIONING_SERVER_PRESENT
  if (sli_zigbee_af_zll_stealing_allowed()) {
    policy |= SL_ZIGBEE_ZLL_POLICY_STEALING_ENABLED;
  }
  if (sli_zigbee_af_zll_remote_reset_allowed()) {
    policy |= SL_ZIGBEE_ZLL_POLICY_REMOTE_RESET_ENABLED;
  }
#endif
#ifndef EZSP_HOST
  status = sl_zigbee_zll_set_policy(policy);
#else
  status = sl_zigbee_af_set_ezsp_policy(SL_ZIGBEE_EZSP_ZLL_POLICY,
                                        policy,
                                        "ZLL processing",
                                        "enable");
#endif
  return status;
}
