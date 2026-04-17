/***************************************************************************//**
 * @file
 * @brief Definitions for the Update TC Link Key plugin, which provides a way
 *        for joining devices to request a new link key after joining a Zigbee
 *        3.0 network.
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
#include "update-tc-link-key.h"
#include "update-tc-link-key-config.h"
#include "app/framework/plugin/network-steering/network-steering-internal.h"
#include "stack/include/zigbee-security-manager.h"

#ifdef SL_CATALOG_ZIGBEE_DYNAMIC_COMMISSIONING_PRESENT
#include "stack/include/sl_zigbee_zdo_dlk_negotiation.h"
#include "stack/include/sl_zigbee_zdo_security.h"
#include "stack/zigbee/aps-keys-full.h"
#endif

#define R21_COMPLIANCE_REVISION 21
static bool inRequest = false;

static sl_zigbee_af_event_t beginTcLinkKeyUpdateEvents[SL_ZIGBEE_SUPPORTED_NETWORKS];

static uint32_t TCLKUpdateRetryBackoffTimeMs = MILLISECOND_TICKS_PER_HOUR;
// Setting the default timer of periodic TCLK update to a day.
static uint32_t LinkKeyUpdateTimerMs = SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_TIMER_SEC * MILLISECOND_TICKS_PER_SECOND;
static uint8_t TCLKUpdateMaxIteration = SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_MAX_ITERATIONS;
static uint8_t TCLKUpdateIteration = 1;
static uint8_t TCLKUpdateMaxAttemptsPerIteration = SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_MAX_ATTEMPTS;
extern uint8_t sl_zigbee_get_stack_compliance_revision(void);
static void update_auth_token(void);

// #define PLUGIN_DEBUG
#if defined(PLUGIN_DEBUG)
  #define debug_print(...) sl_zigbee_af_core_println(__VA_ARGS__)
#else
  #define debug_print(...)
#endif
// -----------------------------------------------------------------------------
// Public API

sl_status_t sl_zigbee_af_update_tc_link_key_start(void)
{
  if (sl_zigbee_get_stack_compliance_revision() < R21_COMPLIANCE_REVISION) {
    // If the stack is pre-R21, we cannot update the TC link key.
    sl_zigbee_af_core_println("%s: %s",
                              SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_PLUGIN_NAME,
                              "Stack revision is pre-R21, cannot update TC link key");
    return SL_STATUS_NOT_SUPPORTED;
  }
  sl_status_t status;

  status = sl_zigbee_update_tc_link_key(TCLKUpdateMaxAttemptsPerIteration);
  if (status == SL_STATUS_OK) {
    inRequest = true;
  }

  return status;
}

bool sl_zigbee_af_update_tc_link_key_stop(void)
{
  bool wasInRequest = inRequest;
  inRequest = false;
  return wasInRequest;
}

void sl_zigbee_af_update_tc_link_key_set_delay(uint32_t delayMs)
{
  sl_zigbee_af_event_set_delay_ms(beginTcLinkKeyUpdateEvents, delayMs);
}

void sl_zigbee_af_update_tc_link_key_set_inactive(void)
{
  sl_zigbee_af_event_set_inactive(beginTcLinkKeyUpdateEvents);
}

void sl_zigbee_af_update_tc_link_key_zigbee_key_establishment_cb(sl_802154_long_addr_t partner,
                                                                 sl_zigbee_key_status_t status)
{
  if (inRequest) {
    sl_zigbee_af_core_print("%s:", SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_PLUGIN_NAME);

    if ((status == SL_ZIGBEE_TRUST_CENTER_LINK_KEY_ESTABLISHED)
        || (status == SL_ZIGBEE_VERIFY_LINK_KEY_SUCCESS)) {
      sl_zigbee_af_core_print(" New key established:");
    } else if (status != SL_ZIGBEE_APP_LINK_KEY_ESTABLISHED) {
      sl_zigbee_af_core_print(" Error:");
    }
    sl_zigbee_af_core_println(" 0x%02X", status);
    sl_zigbee_af_core_print("Partner: ");
    sl_zigbee_af_core_print_buffer(partner, EUI64_SIZE, true); // withSpace?
    sl_zigbee_af_core_println("");

    // Anything greater than SL_ZIGBEE_TRUST_CENTER_LINK_KEY_ESTABLISHED is the
    // final state for a joining device
    if (status > SL_ZIGBEE_TRUST_CENTER_LINK_KEY_ESTABLISHED) {
      inRequest = false;
    }

    // Upon a verify key success, try to get an authentication token
    // if we haven't gotten one already
    if (status == SL_ZIGBEE_VERIFY_LINK_KEY_SUCCESS) {
      update_auth_token();
    }

    if (status == SL_ZIGBEE_TRUST_CENTER_LINK_KEY_ESTABLISHED) {
      sl_zigbee_af_update_tc_link_key_status_cb(status);
      return;
    }
    // Backoff timer
    if (TCLKUpdateIteration < TCLKUpdateMaxIteration
        && status != SL_ZIGBEE_VERIFY_LINK_KEY_SUCCESS) {
      TCLKUpdateIteration++;
      // start backoff timer with the timer value based in current timer value and max backoff time
      sl_zigbee_af_update_tc_link_key_set_delay(TCLKUpdateRetryBackoffTimeMs);
    } else {
      TCLKUpdateIteration = 0;
      // If either the max iteration is reached or the TCLK update is successful,
      // fire sl_zigbee_af_update_tc_link_key_status_cb,
      sl_zigbee_af_update_tc_link_key_status_cb(status);
      // and schedule the next TCLK Update
      if (LinkKeyUpdateTimerMs) {
        sl_zigbee_af_update_tc_link_key_set_delay(LinkKeyUpdateTimerMs);
      } else {
        sl_zigbee_af_event_set_inactive(beginTcLinkKeyUpdateEvents);
      }
    }
  }
}

bool sl_zigbee_af_update_tc_link_key_is_tclk_key_default(void)
{
  sl_status_t status;
  sl_zigbee_key_data_t install_code_key;

  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_init_context(&context);

  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;

  // Get our current APS key
  status = sl_zigbee_sec_man_check_key_context(&context);
  if (status != SL_STATUS_OK) {
    return false;
  }

  // Does it match the default key, ZA09?
  const sl_zigbee_key_data_t default_link_key = {
    { 0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C,
      0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 }
  };

  if (sl_zigbee_sec_man_compare_key_to_value(&context, (const sl_zigbee_sec_man_key_t*)&default_link_key)) {
    return true;
  }

  status = sl_zigbee_get_key_from_install_code(&install_code_key);

  // Does it match our install code derived key?
  if (status == SL_STATUS_OK && sl_zigbee_sec_man_compare_key_to_value(&context, (const sl_zigbee_sec_man_key_t*)&install_code_key)) {
    return true;
  }

  return false;
}

// =============================================================================
// Begin Update of TC Link Key

// The TC link key may be updated on a timely basis, if desired. For instance,
// if a device is joining a network where the Trust Center is pre-R21 and the
// device knows that the TC will be upgraded to post-R21 at some point, the
// joining device may choose to update its Trust Center link key on a timely
// basis such that, eventually, the update completes successfully.
// Other applications may choose to regularly update their TC link key for
// security reasons.
static void beginTcLinkKeyUpdateEventHandler(sl_zigbee_af_event_t * event)
{
  (void)event;

  if (!inRequest) {
    sl_zigbee_af_event_set_inactive(beginTcLinkKeyUpdateEvents);

    sl_status_t status = sl_zigbee_af_update_tc_link_key_start();
    sl_zigbee_af_core_println("%s: %s: 0x%02X",
                              SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_PLUGIN_NAME,
                              "Starting update trust center link key process",
                              status);
    if (status != SL_STATUS_OK) {
      // If Update TCLK failed and network-steering is in progress, leave network
      // Else, this might be one of the periodic link key updates that does not
      // require that the node leave the network. This will be retried in the
      // next attempt to update TCLK
      if ( sli_zigbee_af_network_steering_state_update_tclk() ) {
        sl_zigbee_leave_network(SL_ZIGBEE_LEAVE_NWK_WITH_NO_OPTION);
        sli_zigbee_af_network_steering_cleanup(status);
      }
    }
  }
}

void sl_zigbee_af_set_tc_link_key_update_timer_ms(uint32_t timeInMilliseconds)
{
  LinkKeyUpdateTimerMs = timeInMilliseconds;
}

void sl_zigbee_af_set_tc_link_key_update_retry_backoff_timer_ms(uint32_t timeInMilliseconds)
{
  TCLKUpdateRetryBackoffTimeMs = timeInMilliseconds;
}

void sl_zigbee_af_set_tc_link_key_update_max_attempts_per_iteration(uint8_t maxAttemptsPerIteration)
{
  TCLKUpdateMaxAttemptsPerIteration = maxAttemptsPerIteration;
}

void sl_zigbee_af_set_tc_link_key_update_max_iterations(uint8_t maxIterations)
{
  TCLKUpdateMaxIteration = maxIterations;
}

void sli_zigbee_af_update_tc_link_key_begin_tc_link_key_update_init(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_network_event_init(beginTcLinkKeyUpdateEvents,
                                  beginTcLinkKeyUpdateEventHandler);
}

void sli_zigbee_af_update_tc_link_key_stack_status_callback(sl_status_t status)
{
  if (status == SL_STATUS_NETWORK_UP) {
    // If we are network up, go check if we need to request an authentication token
    update_auth_token();
  }
}

sl_status_t sl_zigbee_af_tc_link_key_update_now(void)
{
  // If the stack is pre-R21, we cannot update the TC link key.
  if (sl_zigbee_get_stack_compliance_revision() < R21_COMPLIANCE_REVISION) {
    sl_zigbee_af_core_println("%s: %s",
                              SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_PLUGIN_NAME,
                              "Stack revision is pre-R21, cannot update TC link key");
    return SL_STATUS_NOT_SUPPORTED;
  }
  // If we're in request, wait until that's done
  if (inRequest) {
    return SL_STATUS_IN_PROGRESS;
  }
  sl_zigbee_af_event_set_active(beginTcLinkKeyUpdateEvents);
  return SL_STATUS_OK;
}

static void update_auth_token(void)
{
#ifdef SL_CATALOG_ZIGBEE_DYNAMIC_COMMISSIONING_PRESENT
  // Fetch an authentication token if we haven't already
  sl_zigbee_key_data_t auth_token;
  sl_802154_long_addr_t tc_eui;
  sl_status_t status;
  uint32_t tok;

  status = slx_zigbee_get_trust_center_additional_info(&tok);
  bool dlk_key_established = ((status == SL_STATUS_OK) && (tok & EXTENDED_BIT_MASK_DERIVED_KEY_DLK));

  status = sl_zigbee_lookup_eui64_by_node_id(SL_ZIGBEE_ZIGBEE_COORDINATOR_ADDRESS, tc_eui);
  if ((status == SL_STATUS_OK)
      && sl_zigbee_get_stack_compliance_revision() == R23_COMPLIANCE_REVISION
      && sl_zigbee_zdo_dlk_enabled()
      && dlk_key_established) {
    status = sl_zigbee_sec_man_export_symmetric_passphrase(tc_eui, &auth_token);
    if (status != SL_STATUS_OK) {
      sl_zigbee_af_core_println("%s: retrieving authentication token for DLK", SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_PLUGIN_NAME);
      sl_zigbee_retrieve_authentication_token(SL_ZIGBEE_TRUST_CENTER_NODE_ID, (SL_ZIGBEE_APS_OPTION_ENCRYPTION | SL_ZIGBEE_APS_OPTION_RETRY));
    }
  } else {
    debug_print("%s: not fetching auth token (DLK: done:%d enabled:%d, EUI: %d, rev:%d)",
                SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_PLUGIN_NAME,
                dlk_key_established,
                sl_zigbee_zdo_dlk_enabled(),
                status,
                sl_zigbee_get_stack_compliance_revision());
  }
#endif // SL_CATALOG_ZIGBEE_DYNAMIC_COMMISSIONING_PRESENT
}
