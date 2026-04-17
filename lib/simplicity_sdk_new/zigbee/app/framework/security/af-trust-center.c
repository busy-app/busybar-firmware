/***************************************************************************//**
 * @file
 * @brief Security code for a Trust Center node.
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
#define USE_REAL_SECURITY_PROTOTYPES
#include "app/framework/security/af-security.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/attribute-storage.h"
#include "stack/include/zigbee-security-manager.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_DELAYED_JOIN_PRESENT
#define DELAYED_JOIN_PRESENT
#endif
#ifdef SL_CATALOG_ZIGBEE_NETWORK_CREATOR_SECURITY_PRESENT
#include "network-creator-security-config.h"
#endif

#if !defined (EZSP_HOST)
#include "stack/internal/inc/network-formation-internal-def.h"
#endif
//------------------------------------------------------------------------------
// Globals

#if !defined (EZSP_HOST)
#if defined(EMBER_AF_PRINT_SECURITY)  || defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT)
// Corresponds to the sl_zigbee_join_decision_t status codes
static const char * joinDecisionText[] = {
  SL_ZIGBEE_JOIN_DECISION_STRINGS
};

// Corresponds to the sl_zigbee_device_update_t status codes
static const char * deviceUpdateText[] = {
  SL_ZIGBEE_DEVICE_UPDATE_STRINGS
};

#endif // SL_ZIGBEE_AF_PRINT_APP
#endif // EZSP_HOST

static sl_status_t permitRequestingTrustCenterLinkKey(void);
static sl_status_t permitRequestingApplicationLinkKey(void);
static sl_status_t setJoinPolicy(sl_zigbee_join_decision_t decision);

//----------------------------------------------------------------
// Helper Functions

// Reverse the bits in a byte
static uint8_t reverse(uint8_t b)
{
#if defined(EZSP_HOST) || defined(BOARD_SIMULATION)
  return ((b * 0x0802UL & 0x22110UL) | (b * 0x8020UL & 0x88440UL)) * 0x10101UL >> 16;
#else
  return (__RBIT((uint32_t)b) >> 24); // Cortex function __RBIT uses uint32_t
#endif // EZSP_HOST
}

//------------------------------------------------------------------------------

sl_status_t zaTrustCenterSetJoinPolicy(sl_zigbee_join_decision_t decision)
{
  // Call the platform specific method to do this.
  return setJoinPolicy(decision);
}

sl_status_t zaTrustCenterSecurityPolicyInit(void)
{
  sl_status_t status = permitRequestingTrustCenterLinkKey();
  if (status == SL_STATUS_OK) {
    status = permitRequestingApplicationLinkKey();
  }
  if (status != SL_STATUS_OK) {
    return status;
  }

  // We always set the join policy to SL_ZIGBEE_USE_PRECONFIGURED_KEY
  // even in the case where the joining device doesn't have one.  This
  // is the safest thing to do.
  // In cases where devices don't have a preconfigured key, the TC must be
  // explicitly told to allow joining by calling the function below with
  // SL_ZIGBEE_SEND_KEY_IN_THE_CLEAR.  Once the device finishes joining the TC
  // should set the policy back to SL_ZIGBEE_USE_PRECONFIGURED_KEY.
  return setJoinPolicy(SL_ZIGBEE_USE_PRECONFIGURED_KEY);
}

sl_status_t zaTrustCenterSecurityInit(bool centralizedNetwork)
{
  sl_zigbee_initial_security_state_t state;
  sl_zigbee_extended_security_bitmask_t extended;
  sl_status_t status;
  const sl_zigbee_af_security_profile_data_t *data = sli_zigbee_af_get_current_security_profile_data();

  if (data == NULL) {
    return SL_STATUS_FAIL;
  }

  memset(&state, 0, sizeof(sl_zigbee_initial_security_state_t));
  state.bitmask = data->tcBitmask;
  extended = data->tcExtendedBitmask;
  memmove(sl_zigbee_key_contents(&state.preconfiguredKey),
          sl_zigbee_key_contents(&data->preconfiguredKey),
          SL_ZIGBEE_ENCRYPTION_KEY_SIZE);

  // Random network key (highly recommended)
  status = sl_zigbee_af_generate_random_key(&(state.networkKey));
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Check for distributed network.
  if (!centralizedNetwork) {
    state.bitmask |= SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE;
  }

#if defined(ZA_CLI_FULL) || defined(SL_CATALOG_CLI_PRESENT)
  // These functions will only be called if the full CLI is enabled and key
  // values have been previously set on the CLI via the "changekey" command.
  getLinkKeyFromCli(&(state.preconfiguredKey));
  getNetworkKeyFromCli(&(state.networkKey));
#endif

  sl_zigbee_af_security_init_cb(&state, &extended, true); // trust center?

  sl_zigbee_af_security_println("set state to: 0x%04X", state.bitmask);
  status = sl_zigbee_set_initial_security_state(&state);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_security_println("security init TC: 0x%02X", status);
    return status;
  }

  // Don't need to check on the status here, sl_zigbee_set_extended_security_bitmask
  // always returns SL_STATUS_OK.
  sl_zigbee_af_security_println("set extended security to: 0x%04X", extended);
  sl_zigbee_set_extended_security_bitmask(extended);

  status = zaTrustCenterSecurityPolicyInit();
  if (status != SL_STATUS_OK) {
    return status;
  }

  sli_zigbee_af_clear_link_key_table();

  return SL_STATUS_OK;
}

sl_status_t sli_zigbee_af_install_code_to_key(uint8_t* installCode, uint8_t length,
                                              sl_zigbee_key_data_t *key)
{
  sl_status_t status;
  uint8_t index;
  bool validInstallCodeSize = false;
  uint16_t crc = 0xFFFF;
  uint8_t installCodeSizes[SL_ZIGBEE_NUM_INSTALL_CODE_SIZES] =
    SL_ZIGBEE_INSTALL_CODE_SIZES;

  if (!installCode || !key) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  for (index = 0; index < SL_ZIGBEE_NUM_INSTALL_CODE_SIZES; index++) {
    if (length == installCodeSizes[index]) {
      validInstallCodeSize = true;
      break;
    }
  }

  if (!validInstallCodeSize) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Compute the CRC and verify that it matches.  The bit reversals, byte swap,
  // and ones' complement are due to differences between halCommonCrc16 and the
  // Smart Energy version.
  for (index = 0; index < length - SL_ZIGBEE_INSTALL_CODE_CRC_SIZE; index++) {
    crc = halCommonCrc16(reverse(installCode[index]), crc);
  }
  crc = (uint16_t)(~HIGH_LOW_TO_INT(reverse(LOW_BYTE(crc)), reverse(HIGH_BYTE(crc))));
  if (installCode[length - SL_ZIGBEE_INSTALL_CODE_CRC_SIZE] != LOW_BYTE(crc)
      || installCode[length - SL_ZIGBEE_INSTALL_CODE_CRC_SIZE + 1] != HIGH_BYTE(crc)) {
    return SL_STATUS_INVALID_CONFIGURATION;
  }

  // Compute the key from the install code and CRC.
  status = sl_zigbee_aes_hash_simple(length, installCode, sl_zigbee_key_contents(key));

  return status;
}

//==============================================================================
// Platform Dependent Implementations

//this callback is moving onto stack side only (untangling decision making from reporting)
#if defined (EZSP_HOST)

static const char * ezspJoinPolicyText[] = {
  "Disallow all joins and rejoins",
  "Allow preconfigured key joins",
  "Allow Joins, rejoins have link key",
  "Allow rejoins only",
};

static sl_status_t setJoinPolicy(sl_zigbee_join_decision_t decision)
{
  sl_zigbee_ezsp_decision_bitmask_t policy = SL_ZIGBEE_EZSP_DECISION_BITMASK_DEFAULT_CONFIGURATION;
  uint8_t textId = 0;

  if (decision == SL_ZIGBEE_USE_PRECONFIGURED_KEY) {
    policy = (SL_ZIGBEE_EZSP_DECISION_ALLOW_JOINS | SL_ZIGBEE_EZSP_DECISION_ALLOW_UNSECURED_REJOINS);
    textId = 1;
  } else if (decision == SL_ZIGBEE_SEND_KEY_IN_THE_CLEAR) {
    policy = (SL_ZIGBEE_EZSP_DECISION_ALLOW_JOINS | SL_ZIGBEE_EZSP_DECISION_ALLOW_UNSECURED_REJOINS | SL_ZIGBEE_EZSP_DECISION_SEND_KEY_IN_CLEAR);
    textId = 2;
  } else if (decision == SL_ZIGBEE_ALLOW_REJOINS_ONLY) {
    policy = SL_ZIGBEE_EZSP_DECISION_ALLOW_UNSECURED_REJOINS;
    textId = 3;
  }
  return sl_zigbee_af_set_ezsp_policy(SL_ZIGBEE_EZSP_TRUST_CENTER_POLICY,
                                      policy,
                                      "Trust Center Policy",
                                      ezspJoinPolicyText[textId]);
}

static sl_status_t permitRequestingTrustCenterLinkKey(void)
{
  const sl_zigbee_af_security_profile_data_t *data = sli_zigbee_af_get_current_security_profile_data();
  if (data == NULL) {
    return SL_STATUS_FAIL;
  }

  return sl_zigbee_af_set_ezsp_policy(SL_ZIGBEE_EZSP_TC_KEY_REQUEST_POLICY,
                                      data->tcLinkKeyRequestPolicy,
                                      "TC Key Request",
                                      (data->tcLinkKeyRequestPolicy
                                       !=  SL_ZIGBEE_AF_DENY_TC_KEY_REQUESTS
                                       ? "Allow"
                                       : "Deny"));
}

static sl_status_t permitRequestingApplicationLinkKey(void)
{
  const sl_zigbee_af_security_profile_data_t *data = sli_zigbee_af_get_current_security_profile_data();
  if (data == NULL) {
    return SL_STATUS_FAIL;
  }
  return sl_zigbee_af_set_ezsp_policy(SL_ZIGBEE_EZSP_APP_KEY_REQUEST_POLICY,
                                      data->appLinkKeyRequestPolicy,
                                      "App. Key Request",
                                      (data->appLinkKeyRequestPolicy
                                       != SL_ZIGBEE_AF_DENY_APP_KEY_REQUESTS
                                       ? "Allow"
                                       : "Deny"));
}

//------------------------------------------------------------------------------
// System-on-a-chip

#else

static sl_zigbee_join_decision_t defaultDecision = SL_ZIGBEE_USE_PRECONFIGURED_KEY;
sl_zigbee_join_decision_t sli_zigbee_af_trust_center_pre_join_callback(sl_802154_short_addr_t newNodeId,
                                                                       sl_802154_long_addr_t newNodeEui64,
                                                                       sl_zigbee_device_update_t status,
                                                                       sl_802154_short_addr_t parentOfNewNode)
{
  (void) parentOfNewNode;
  (void) sl_zigbee_af_push_callback_network_index();
  sl_zigbee_join_decision_t joinDecision = defaultDecision;
  sl_zigbee_current_security_state_t securityState;
  sl_status_t securityStatus = sl_zigbee_get_current_security_state(&securityState);
  bool in_distributed_mode = ((SL_STATUS_OK == securityStatus) && (securityState.bitmask & SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE));

  if (status == SL_ZIGBEE_STANDARD_SECURITY_SECURED_REJOIN
      || status == SL_ZIGBEE_DEVICE_LEFT
      // 4.6.3.3.2 - TC rejoins rejected in distributed TC mode
      || (in_distributed_mode && (status == SL_ZIGBEE_STANDARD_SECURITY_UNSECURED_REJOIN))) {
    joinDecision = SL_ZIGBEE_NO_ACTION;
  } else if (sli_zigbee_stack_get_join_uses_install_code() && !in_distributed_mode) {
    sl_zigbee_sec_man_context_t context;
    sl_zigbee_sec_man_aps_key_metadata_t key_info;
    sl_zigbee_sec_man_init_context(&context);
    context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK_WITH_TIMEOUT;
    memmove(context.eui64, newNodeEui64, EUI64_SIZE);
    context.flags |= ZB_SEC_MAN_FLAG_EUI_IS_VALID;
    // Check in transient key table
    if (SL_STATUS_OK != sl_zigbee_sec_man_get_aps_key_info(&context, &key_info)) {
      // For a join we'll deny it, but for a rejoin we won't send the network key
      if (SL_ZIGBEE_STANDARD_SECURITY_UNSECURED_JOIN == status) {
        joinDecision = SL_ZIGBEE_DENY_JOIN;
      } else if (status == SL_ZIGBEE_STANDARD_SECURITY_SECURED_REJOIN) {
        joinDecision = SL_ZIGBEE_NO_ACTION;
      }
    }
  }
#if defined(DELAYED_JOIN_PRESENT)
  if (sl_zigbee_delayed_join_is_activated()) {
    joinDecision = (status == SL_ZIGBEE_STANDARD_SECURITY_SECURED_REJOIN
                    || status == SL_ZIGBEE_STANDARD_SECURITY_UNSECURED_REJOIN)
                   ? SL_ZIGBEE_USE_PRECONFIGURED_KEY
                   : SL_ZIGBEE_NO_ACTION;
  }
#endif

  // EMZIGBEE-9283 Allow unsecured rejoin if default decision is SL_ZIGBEE_ALLOW_REJOINS_ONLY
  if (status == SL_ZIGBEE_STANDARD_SECURITY_UNSECURED_REJOIN
      && joinDecision == SL_ZIGBEE_ALLOW_REJOINS_ONLY ) {
    joinDecision = SL_ZIGBEE_USE_PRECONFIGURED_KEY;
  }

  (void) sl_zigbee_af_pop_network_index();
  #if !defined (EZSP_HOST)
  #if defined(EMBER_AF_PRINT_SECURITY)  || defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT)
  uint8_t numDeviceUpdateText = sizeof(deviceUpdateText) / sizeof(deviceUpdateText[0]);
  if (status >= numDeviceUpdateText) {
    return SL_ZIGBEE_NO_ACTION;
  }

  sl_zigbee_af_security_println("Trust Center Join Handler: status = %s, decision = %s (%02X), shortid 0x%04X",
                                deviceUpdateText[status],
                                joinDecisionText[joinDecision],
                                joinDecision,
                                newNodeId);
  #endif // SL_ZIGBEE_AF_PRINT_APP
  #endif // EZSP_HOST
  sl_zigbee_af_security_flush();
  return joinDecision;
}

static sl_status_t setJoinPolicy(sl_zigbee_join_decision_t decision)
{
  defaultDecision = decision;
  return SL_STATUS_OK;
}

static sl_status_t permitRequestingTrustCenterLinkKey(void)
{
  const sl_zigbee_af_security_profile_data_t *data = sli_zigbee_af_get_current_security_profile_data();
  if (data == NULL) {
    return SL_STATUS_FAIL;
  }
  sl_zigbee_set_trust_center_link_key_request_policy(data->tcLinkKeyRequestPolicy);

  return SL_STATUS_OK;
}

static sl_status_t permitRequestingApplicationLinkKey(void)
{
  const sl_zigbee_af_security_profile_data_t *data = sli_zigbee_af_get_current_security_profile_data();
  if (data == NULL) {
    return SL_STATUS_FAIL;
  }
  sl_zigbee_set_app_link_key_request_policy(data->appLinkKeyRequestPolicy);
  return SL_STATUS_OK;
}

#endif // EZSP_HOST
