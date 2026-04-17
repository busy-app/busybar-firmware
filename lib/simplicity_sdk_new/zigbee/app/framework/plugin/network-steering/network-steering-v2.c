/***************************************************************************//**
 * @file
 * @brief This file implements the Zigbee 3.0 network steering procedure using
 * only one 15.4 scan.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "app/framework/security/af-security.h" // sli_zigbee_af_clear_link_key_table()
#include "network-steering.h"
#include "network-steering-internal.h"
#include "stack/include/network-formation.h"
#include "stack/include/zigbee-security-manager.h"

#ifdef OPTIMIZE_SCANS

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_SCAN_DISPATCH_PRESENT
#include "scan-dispatch.h"
#else
#include "app/framework/plugin/scan-dispatch/scan-dispatch.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_UPDATE_TC_LINK_KEY_PRESENT
#include "update-tc-link-key.h"
#else
#include "app/framework/plugin/update-tc-link-key/update-tc-link-key.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
#include "app/test/pro-compliance.h"
#endif //SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT

#ifdef SL_CATALOG_ZIGBEE_DYNAMIC_COMMISSIONING_PRESENT
#include "stack/include/sl_zigbee_zdo_dlk_negotiation.h"
#include "stack/include/sl_zigbee_zdo_security.h"
#include "stack/zigbee/aps-keys-full.h"
#endif

#ifdef SL_ZIGBEE_TEST
  #define HIDDEN
  #define SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_RADIO_TX_POWER 3
#else
  #define HIDDEN static
#endif

//============================================================================
// Globals

#if !defined(SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_CHANNEL_MASK)
  #define SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_CHANNEL_MASK \
  (BIT32(11) | BIT32(14))
#endif

#if !defined(SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_SCAN_DURATION)
  #define SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_SCAN_DURATION 5
#endif

#if !defined(SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_COMMISSIONING_TIME_S)
  #define SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_COMMISSIONING_TIME_S (180)
#endif

#ifdef RADIO_TX_CALLBACK
  #define GET_RADIO_TX_POWER(channel) sl_zigbee_af_network_steering_get_power_for_radio_channel_cb(channel)
#else
  #define GET_RADIO_TX_POWER(channel) SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_RADIO_TX_POWER
#endif

const char * sli_zigbee_af_network_steering_stateNames[] = {
  "None",
  "Scan Primary Channels and use Configured Key",
  "Scan Primary Channels and use Install Code",
  "Scan Primary Channels and Use Centralized Key",
  "Scan Primary Channels and Use Distributed Key",
  "Scan Secondary Channels and use Configured Key",
  "Scan Secondary Channels and use Install Code",
  "Scan Secondary Channels and Use Centralized Key",
  "Scan Secondary Channels and Use Distributed Key",
  // The Use All Keys states are run only if the plugin option is selected, in
  // which case the other states are not run
  "Scan Primary Channels and Use All Keys (install code, centralized, distributed)",
  "Scan Secondary Channels and Use All Keys (install code, centralized, distributed)",
};

sl_zigbee_af_plugin_network_steering_joining_state_t sli_zigbee_af_network_steering_state
  = SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_NONE;

const uint8_t sli_zigbee_af_network_steering_plugin_name[] = "NWK Steering";
#define PLUGIN_NAME sli_zigbee_af_network_steering_plugin_name

// #define PLUGIN_DEBUG
#if defined(PLUGIN_DEBUG)
  #define debugPrintln(...) sl_zigbee_af_core_println(__VA_ARGS__)
  #define debugPrint(...)   sl_zigbee_af_core_print(__VA_ARGS__)
  #define debugExec(x) do { x; } while (0)
#else
  #define debugPrintln(...)
  #define debugPrint(...)
  #define debugExec(x)
#endif

#define SECONDARY_CHANNEL_MASK                                                                \
  ((SL_ZIGBEE_ALL_802_15_4_CHANNELS_MASK ^ SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_CHANNEL_MASK) \
   & SL_ZIGBEE_ALL_802_15_4_CHANNELS_MASK)

#define WILDCARD_EUI  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }

static const sl_zigbee_key_data_t defaultLinkKey = {
  { 0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C,
    0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 }
};
static const sl_zigbee_key_data_t distributedTestKey = {
  { 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF }
};

// These parameters allow for filtering which networks to find or which specific
// key to use
static bool gFilterByExtendedPanId = false;
static uint8_t gExtendedPanIdToFilterOn[8];
static bool gUseConfiguredKey = false;
static sl_zigbee_key_data_t gConfiguredKey = {
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

bool sl_zigbee_stack_is_up(void);

// We make these into variables so that they can be changed at run time.
// This is very useful for unit and interop tests.
uint32_t sli_zigbee_af_network_steering_primary_channel_mask
  = SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_CHANNEL_MASK;
uint32_t sli_zigbee_af_network_steering_secondary_channel_mask
  = SECONDARY_CHANNEL_MASK;

uint8_t sli_zigbee_af_network_steering_total_beacons = 0;
uint8_t sli_zigbee_af_network_steering_join_attempts = 0;
sl_zigbee_key_data_t sl_zigbee_plugin_network_steering_distributed_key;

static sl_zigbee_beacon_data_t beacon;
static uint8_t beacon_number = 0;
static uint32_t current_channel_mask = 0;

sl_zigbee_af_event_t sl_zigbee_af_network_steering_finish_steering_event[SL_ZIGBEE_SUPPORTED_NETWORKS];
#define finishSteeringEvent (sl_zigbee_af_network_steering_finish_steering_event)

// TODO: good value for this?
// Let's try jittering our TCLK update and permit join broadcast to cut down
// on commission-time traffic.
#define FINISH_STEERING_JITTER_MIN_MS (MILLISECOND_TICKS_PER_SECOND << 1)
#define FINISH_STEERING_JITTER_MAX_MS (MILLISECOND_TICKS_PER_SECOND << 2)
#define randomJitterMS()                                               \
  ((sl_zigbee_get_pseudo_random_number()                               \
    % (FINISH_STEERING_JITTER_MAX_MS - FINISH_STEERING_JITTER_MIN_MS)) \
   + FINISH_STEERING_JITTER_MIN_MS)
#define UPDATE_TC_LINK_KEY_JITTER_MIN_MS (MILLISECOND_TICKS_PER_SECOND * 10)
#define UPDATE_TC_LINK_KEY_JITTER_MAX_MS (MILLISECOND_TICKS_PER_SECOND * 40)

sl_zigbee_af_plugin_network_steering_options_t sli_zigbee_af_network_steering_options_mask
  = SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_OPTIONS_NONE;

//============================================================================
// Externs

#ifdef EZSP_HOST
 #define setMaxBeaconsToStore(n)  { \
    uint8_t val = n;                \
    (void)sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_MAX_BEACONS_TO_STORE, 1, &val); }
#else // EZSP_HOST
 #define setMaxBeaconsToStore(n)  { \
    (void)sl_zigbee_set_num_beacons_to_store(n); }
#endif // EZSP_HOST
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
extern sl_zigbee_af_event_t sl_zigbee_af_test_harness_z3_open_network_event;
#endif
//============================================================================
// Forward Declarations

static void cleanupAndStop(sl_status_t status);
static void stateMachineRun(void);
static void tryToJoinNetwork(void);
static sl_status_t goToNextState(void);
static sl_status_t setupSecurity(void);
static uint32_t jitterTimeDelayMs();
HIDDEN void scanResultsHandler(sl_zigbee_af_plugin_scan_dispatch_scan_results_t *results);
static sl_status_t scheduleScan(uint32_t channelMask);

// Callback declarations for the sake of the compiler during unit tests.
int8_t sl_zigbee_af_network_steering_get_power_for_radio_channel_cb(uint8_t channel);
bool sl_zigbee_af_network_steering_get_distributed_key_cb(sl_zigbee_key_data_t * key);
sl_zigbee_node_type_t sl_zigbee_af_network_steering_get_node_type_cb(sl_zigbee_af_plugin_network_steering_joining_state_t state);

void sl_zigbee_af_network_steering_finish_steering_event_handler(sl_zigbee_af_event_t * event);

static sl_zigbee_af_plugin_network_steering_joining_state_t getFirstPrimaryState(void);
static sl_zigbee_af_plugin_network_steering_joining_state_t getLastPrimaryState(void);
static sl_zigbee_af_plugin_network_steering_joining_state_t getFirstSecondaryState(void);

//============================================================================
// Internal callbacks

void sli_zigbee_af_network_steering_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_network_event_init(finishSteeringEvent,
                                  sl_zigbee_af_network_steering_finish_steering_event_handler);

#if defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z4)
  // Zigbee 4.0 mandates the binding table to be cleared when leaving a network
  // See CN-Reset-TC-01 and DN-Reset-TC-01
  // The Network Steering component is used by all device roles (ZC, ZR, ZED) so this covers everything
  sl_zigbee_clear_binding_table_on_leave(true);
#endif // SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z4
}

//============================================================================
// State Machine

void tryToJoinNetwork()
{
  sl_status_t status = SL_STATUS_OK;
  sl_zigbee_node_type_t nodeType;
  int8_t radioTxPower;

  sl_zigbee_af_core_println("Examining beacon on channel %d with panId 0x%04X",
                            beacon.channel,
                            beacon.panId);

  if (!(current_channel_mask & BIT32(beacon.channel))) {
    sl_zigbee_af_core_println("This beacon is not part of the current "
                              "channel mask (0x%08X)."
                              " Getting next beacon whose channel bitmask is set.",
                              current_channel_mask);
  }

  bool filteredOut = false;
  if (gFilterByExtendedPanId) {
    if (0 != memcmp(beacon.extendedPanId,
                    gExtendedPanIdToFilterOn,
                    COUNTOF(gExtendedPanIdToFilterOn))) {
      debugPrint(". Skipping since we are looking for xpan: ");
      debugExec(sl_zigbee_af_print_big_endian_eui64(gExtendedPanIdToFilterOn));
      debugPrintln("");
      filteredOut = true;
    }
  }

  // If the retrieved beacon channel is not set in the mask or we've filtered it
  // out, get the next valid beacon
  while ((!(current_channel_mask & BIT32(beacon.channel)))
         || filteredOut) {
    beacon_number++;
    status = sl_zigbee_get_stored_beacon(beacon_number, &beacon);

    if (status != SL_STATUS_OK) {
      break;
    }

    filteredOut = (gFilterByExtendedPanId && memcmp(beacon.extendedPanId,
                                                    gExtendedPanIdToFilterOn,
                                                    COUNTOF(gExtendedPanIdToFilterOn)));
  }

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_core_println("No beacons left on current mask. Moving to next state");

    status = goToNextState();
    if (status != SL_STATUS_OK) {
      return;
    }

    stateMachineRun();
    return;
  }

  sl_zigbee_af_core_println("%s joining 0x%04X on channel %d",
                            PLUGIN_NAME,
                            beacon.panId,
                            beacon.channel);

  nodeType = sl_zigbee_af_network_steering_get_node_type_cb(sli_zigbee_af_network_steering_state);
  radioTxPower = GET_RADIO_TX_POWER(beacon.channel);
#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
  if (isForceAssociationUsed()) {
    beacon.extended_beacon = false;
  }
#endif //SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
  status = sl_zigbee_join_network_directly(nodeType,
                                           &beacon,
                                           radioTxPower,
                                           true); // clearBeaconsAfterNetworkUp
  sli_zigbee_af_network_steering_join_attempts++;
  if (SL_STATUS_OK != status) {
    sl_zigbee_af_core_println("Error: %s could not attempt join: 0x%02X",
                              PLUGIN_NAME,
                              status);
    cleanupAndStop(status);
    return;
  }
}

//Description: Generates a random number between 10000-40000.
static uint32_t jitterTimeDelayMs()
{
  uint32_t jitterDelayMs = (sl_zigbee_get_pseudo_random_number() % (UPDATE_TC_LINK_KEY_JITTER_MAX_MS - UPDATE_TC_LINK_KEY_JITTER_MIN_MS + 1)) + UPDATE_TC_LINK_KEY_JITTER_MIN_MS;
  return jitterDelayMs;
}

static bool joinedToDistributedNetwork(void)
{
  sl_zigbee_current_security_state_t securityState;
  sl_status_t status = sl_zigbee_get_current_security_state(&securityState);
  if (status == SL_STATUS_OK) {
    return (securityState.bitmask & SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE);
  }
  return false;
}

static sl_zigbee_af_plugin_network_steering_joining_state_t getFirstPrimaryState(void)
{
#ifdef TRY_ALL_KEYS
  return SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_USE_ALL_KEYS;
#else
  return (gUseConfiguredKey
          ? SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CONFIGURED
          : SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE);
#endif
}

static sl_zigbee_af_plugin_network_steering_joining_state_t getLastPrimaryState(void)
{
#ifdef TRY_ALL_KEYS
  return SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_USE_ALL_KEYS;
#else
  return SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_DISTRIBUTED;
#endif
}

static sl_zigbee_af_plugin_network_steering_joining_state_t getFirstSecondaryState(void)
{
#ifdef TRY_ALL_KEYS
  return SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_USE_ALL_KEYS;
#else
  return (gUseConfiguredKey
          ? SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_CONFIGURED
          : SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_INSTALL_CODE);
#endif
}

void sli_zigbee_af_network_steering_stack_status_callback(sl_status_t status)
{
  sl_zigbee_af_core_println("%s stack status 0x%02X", PLUGIN_NAME, status);
  if (sli_zigbee_af_network_steering_state
      == SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_NONE) {
    if (status == SL_STATUS_NETWORK_UP && sl_zigbee_af_update_tc_link_key_is_tclk_key_default()) {
      sl_zigbee_af_update_tc_link_key_set_delay(jitterTimeDelayMs());
    } else if (status == SL_STATUS_NETWORK_DOWN) {
      sl_zigbee_af_update_tc_link_key_set_inactive();
    }
  } else if (status == SL_STATUS_NETWORK_UP) {
    sl_zigbee_af_core_println("%s network joined.", PLUGIN_NAME);
    if (!sli_zigbee_af_network_steering_state_uses_distributed_key()
        && !(sli_zigbee_af_network_steering_options_mask
             & SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_OPTIONS_NO_TCLK_UPDATE)
        && !joinedToDistributedNetwork()) {
#ifdef SL_CATALOG_ZIGBEE_DYNAMIC_COMMISSIONING_PRESENT
      // If we are network up after a successful DLK, we do not need to update our TCLK
      if (sl_zigbee_get_stack_compliance_revision() == R23_COMPLIANCE_REVISION && sl_zigbee_zdo_dlk_enabled()) {
        uint32_t tok;
        if (SL_STATUS_OK == slx_zigbee_get_trust_center_additional_info(&tok) && (tok & EXTENDED_BIT_MASK_DERIVED_KEY_DLK)) {
          sl_zigbee_af_event_set_delay_ms(finishSteeringEvent, randomJitterMS());
          return;
        }
      }
#endif // SL_CATALOG_ZIGBEE_DYNAMIC_COMMISSIONING_PRESENT
      sli_zigbee_af_network_steering_state_set_update_tclk();
    }
    sl_zigbee_af_event_set_delay_ms(finishSteeringEvent, randomJitterMS());
  } else if (!sl_zigbee_stack_is_up()) {
    if (sli_zigbee_af_network_steering_state > SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_NONE
        && sli_zigbee_af_network_steering_state < SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_FINISHED) {
      sl_zigbee_af_core_println("%s trying next network.", PLUGIN_NAME);

      beacon_number++;
      status = sl_zigbee_get_stored_beacon(beacon_number, &beacon);
      if (status == SL_STATUS_OK) {
        tryToJoinNetwork();
        return;
      }
      // status != SL_STATUS_OK
      sl_zigbee_af_core_println("No beacons left on current mask. "
                                "Moving to next state");

      status = goToNextState();
      if (status != SL_STATUS_OK) {
        return;
      }
      stateMachineRun();
    } else {
      cleanupAndStop(SL_STATUS_ZIGBEE_NO_LINK_KEY_RECEIVED);
    }
  } else {
  }
}

static sl_status_t scheduleScan(uint32_t channelMask)
{
  sl_status_t status;
  sl_zigbee_node_type_t nodeType;
  sl_zigbee_af_plugin_scan_dispatch_scan_data_t scanData;

  // Kick off a scan and record beacons
  setMaxBeaconsToStore(SL_ZIGBEE_MAX_BEACONS_TO_STORE);

  memset(&beacon, 0, sizeof(beacon));
  current_channel_mask = 0;

  nodeType = sl_zigbee_af_network_steering_get_node_type_cb(sli_zigbee_af_network_steering_state);

  // If the local nodeType is a router, ensure that a router scan is initated
  // This will enable the beacon-handling code will properly filter for
  // good beacons with router capacity
  if ( SL_ZIGBEE_ROUTER == nodeType ) {
    scanData.scanType = SL_ZIGBEE_ACTIVE_SCAN_ROUTER;
  } else {
    scanData.scanType = SL_ZIGBEE_ACTIVE_SCAN;
  }

  // Set radio power before active scan to configure beacon request
  // Tx power level based on the plugin setting
  (void)sl_zigbee_set_radio_power(SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_RADIO_TX_POWER);

  scanData.channelMask = channelMask;
  scanData.duration = SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_SCAN_DURATION;
  scanData.handler = scanResultsHandler;
  status = sl_zigbee_af_scan_dispatch_schedule_scan(&scanData);
  sl_zigbee_af_core_println("%s: issuing scan on %s channels (mask 0x%08X)",
                            PLUGIN_NAME,
                            channelMask == sli_zigbee_af_network_steering_primary_channel_mask
                            ? "primary"
                            : "secondary",
                            channelMask);
  return status;
}

// This function is called after a channel mask has been scanned
HIDDEN void scanResultsHandler(sl_zigbee_af_plugin_scan_dispatch_scan_results_t *results)
{
  // If steering-stop was issued using a CLI command, ignore the results of the previously
  // started scan
  if (sli_zigbee_af_network_steering_state == SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_NONE) {
    return;
  }

  if (sl_zigbee_af_scan_dispatch_scan_results_are_complete(results)
      || sl_zigbee_af_scan_dispatch_scan_results_are_failure(results)) {
    if (results->status != SL_STATUS_OK) {
      sl_zigbee_af_core_println("Error: Scan complete handler returned 0x%02X",
                                results->status);
      return;
    }
    sli_zigbee_af_network_steering_total_beacons = sl_zigbee_get_num_stored_beacons();

    sl_zigbee_af_core_println("%s scan complete. Beacons heard: %d",
                              PLUGIN_NAME,
                              sli_zigbee_af_network_steering_total_beacons);

    if (sli_zigbee_af_network_steering_total_beacons) {
      stateMachineRun();
    } else {
      switch (sli_zigbee_af_network_steering_state) {
        case SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_CONFIGURED:
        case SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_INSTALL_CODE:
        case SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_CENTRALIZED:
        case SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_DISTRIBUTED:
        case SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_USE_ALL_KEYS:
          cleanupAndStop(SL_STATUS_NO_BEACONS);
          return;
          break;
        default:
          break;
      }

      sli_zigbee_af_network_steering_state = getFirstSecondaryState();

      (void)scheduleScan(sli_zigbee_af_network_steering_secondary_channel_mask);

      return;
    }
  }
}

static sl_status_t goToNextState(void)
{
  if (sli_zigbee_af_network_steering_state == getLastPrimaryState()) {
    // Skip over SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_CONFIGURED
    // unless a preconfigured key has been set
    sli_zigbee_af_network_steering_state = getFirstSecondaryState();
  } else {
    sli_zigbee_af_network_steering_state++;
  }

  // If there are no more states, return error
  if ((sli_zigbee_af_network_steering_state
       == SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_USE_ALL_KEYS)
      || (sli_zigbee_af_network_steering_state
          == SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_FINISHED)) {
    cleanupAndStop(SL_STATUS_NOT_JOINED);
    return SL_STATUS_NOT_JOINED;
  }

  // When we're on the final scan-primary-channels state,
  // SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_DISTRIBUTED,
  // goToNextState simply increments sli_zigbee_af_network_steering_state and then we
  // call stateMachineRun. We take this opportunity to check if we need to scan
  // the secondary mask now
  if (sli_zigbee_af_network_steering_state == getFirstSecondaryState()) {
    if (sli_zigbee_af_network_steering_secondary_channel_mask == 0) {
      cleanupAndStop(SL_STATUS_NOT_JOINED);
      return SL_STATUS_NOT_JOINED;
    }

    (void)scheduleScan(sli_zigbee_af_network_steering_secondary_channel_mask);

    // We need to return any error code, so that the code that called this
    // function doesn't continue doing its routine
    return SL_STATUS_MAC_SCANNING;
  }

  return SL_STATUS_OK;
}

static void cleanupAndStop(sl_status_t status)
{
  sl_zigbee_af_core_println("%s Stop.  Cleaning up.", PLUGIN_NAME);
  sl_zigbee_af_network_steering_complete_cb(status,
                                            sli_zigbee_af_network_steering_total_beacons,
                                            sli_zigbee_af_network_steering_join_attempts,
                                            sli_zigbee_af_network_steering_state);
  sli_zigbee_af_network_steering_state = SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_NONE;
  sli_zigbee_af_network_steering_join_attempts = 0;
  sli_zigbee_af_network_steering_total_beacons = 0;
  sl_zigbee_af_event_set_inactive(finishSteeringEvent);
}

// This function is called only on state transitions. This function sets up
// security, configures parameters, and then makes a call to start joining
// networks
static void stateMachineRun(void)
{
  sl_status_t status;
  sl_zigbee_af_core_println("%s State: %s",
                            PLUGIN_NAME,
                            sli_zigbee_af_network_steering_stateNames[sli_zigbee_af_network_steering_state]);

  status = setupSecurity();

  while (status != SL_STATUS_OK) {
    sl_zigbee_af_core_println("Error: %s could not setup security: 0x%02X",
                              PLUGIN_NAME,
                              status);

    status = goToNextState();
    if (status != SL_STATUS_OK) {
      return;
    }

    sl_zigbee_af_core_println("%s State: %s",
                              PLUGIN_NAME,
                              sli_zigbee_af_network_steering_stateNames[sli_zigbee_af_network_steering_state]);

    status = setupSecurity();
  }

  // Set current_channel_mask properly so that we can iterate over beacons
  // correctly
  switch (sli_zigbee_af_network_steering_state) {
    case SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CONFIGURED:
    case SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE:
    case SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CENTRALIZED:
    case SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_DISTRIBUTED:
    case SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_USE_ALL_KEYS:
      current_channel_mask = sli_zigbee_af_network_steering_primary_channel_mask;
      break;
    default:
      current_channel_mask = sli_zigbee_af_network_steering_secondary_channel_mask;
      break;
  }

  // Clear our stored beacon so we can iterate from the start
  beacon_number = 0;
  status = sl_zigbee_get_stored_beacon(beacon_number, &beacon);

  if (status != SL_STATUS_OK) {
    cleanupAndStop(SL_STATUS_FAIL);
    return;
  }

  tryToJoinNetwork();
}

static sl_status_t setupSecurity(void)
{
  sl_status_t status;
  sl_zigbee_initial_security_state_t state;
  sl_zigbee_extended_security_bitmask_t extended;
  bool tryAllKeys = ((sli_zigbee_af_network_steering_state
                      == SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_USE_ALL_KEYS)
                     || (sli_zigbee_af_network_steering_state
                         == SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_USE_ALL_KEYS));

  state.bitmask = (SL_ZIGBEE_TRUST_CENTER_GLOBAL_LINK_KEY
                   | SL_ZIGBEE_HAVE_PRECONFIGURED_KEY
                   | SL_ZIGBEE_REQUIRE_ENCRYPTED_KEY
                   | SL_ZIGBEE_NO_FRAME_COUNTER_RESET
#ifndef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
                   // When running with a Test Harness, the Test Harness will write the install-code
                   // derived link key directly to the Transient Key Table.
                   | (sli_zigbee_af_network_steering_state_uses_install_codes()
                      ? SL_ZIGBEE_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE
                      : 0)
#endif
                   | (sli_zigbee_af_network_steering_state_uses_distributed_key()
                      ? SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE
                      : 0)
                   );

  if (!sl_zigbee_af_network_steering_get_distributed_key_cb(&sl_zigbee_plugin_network_steering_distributed_key)) {
    memmove(sl_zigbee_key_contents(&sl_zigbee_plugin_network_steering_distributed_key),
            sl_zigbee_key_contents(&distributedTestKey),
            SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  }
  memmove(sl_zigbee_key_contents(&(state.preconfiguredKey)),
          gUseConfiguredKey ? sl_zigbee_key_contents(&(gConfiguredKey))
          : (sli_zigbee_af_network_steering_state_uses_distributed_key()
             ? sl_zigbee_key_contents(&sl_zigbee_plugin_network_steering_distributed_key)
             : sl_zigbee_key_contents(&defaultLinkKey)),
          SL_ZIGBEE_ENCRYPTION_KEY_SIZE);

  status = sl_zigbee_set_initial_security_state(&state);

  // If we're trying all keys and the install code wasn't burned, setting
  // security will fail with SL_STATUS_INVALID_CONFIGURATION. As a
  // consequence, we ignore the install code key and try again
  if (status == SL_STATUS_INVALID_CONFIGURATION) {
    if (tryAllKeys) {
      sl_zigbee_af_core_println("%s: WARNING: sl_zigbee_set_initial_security_state failed. "
                                "Continuing without install code.",
                                sli_zigbee_af_network_steering_plugin_name);
      state.bitmask &= ((uint16_t) ~((uint16_t)SL_ZIGBEE_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE));
      status = sl_zigbee_set_initial_security_state(&state);
    } else {
      if (sli_zigbee_af_network_steering_state_uses_install_codes()) {
        sl_zigbee_af_core_println("Error: install code setup failed. Is an install "
                                  "code flashed onto the device?");
      }
    }
  }

  if (status != SL_STATUS_OK) {
    goto done;
  }

  extended = (SL_ZIGBEE_JOINER_GLOBAL_LINK_KEY
              | SL_ZIGBEE_EXT_NO_FRAME_COUNTER_RESET
              | SL_ZIGBEE_R18_STACK_BEHAVIOR);

  if ((status = sl_zigbee_set_extended_security_bitmask(extended)) != SL_STATUS_OK) {
    goto done;
  }

  sli_zigbee_af_clear_link_key_table();

  if (tryAllKeys) {
    sl_802154_long_addr_t wildcardEui = WILDCARD_EUI;

    // The install code derived key is handled by setting the
    // SL_ZIGBEE_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE bit above
    // Add the centralized and distributed keys into the transient key table
    sl_status_t import_status;
    import_status = sl_zigbee_sec_man_import_transient_key(wildcardEui,
                                                           (sl_zigbee_sec_man_key_t*)&defaultLinkKey);
    if (import_status == SL_STATUS_OK) {
      import_status = sl_zigbee_sec_man_import_transient_key(wildcardEui,
                                                             (sl_zigbee_sec_man_key_t*)&sl_zigbee_plugin_network_steering_distributed_key);
    }
    status = ((import_status == SL_STATUS_OK) ? SL_STATUS_OK : SL_STATUS_ALLOCATION_FAILED);
  }

  done:
  return status;
}

sl_status_t sl_zigbee_af_network_steering_start(void)
{
  sl_status_t status = SL_STATUS_INVALID_STATE;
  uint32_t channelsToScan;

  if ((0 == sli_zigbee_af_network_steering_primary_channel_mask)
      && (0 == sli_zigbee_af_network_steering_secondary_channel_mask)) {
    return SL_STATUS_INVALID_STATE;
  }

  if (sli_zigbee_af_pro_is_current_network()
      && (sli_zigbee_af_network_steering_state
          == SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_NONE)) {
    if (sl_zigbee_af_network_state() == SL_ZIGBEE_NO_NETWORK) {
      sl_zigbee_af_add_to_current_app_tasks_cb(SL_ZIGBEE_AF_WAITING_FOR_TC_KEY_UPDATE);
      if (sli_zigbee_af_network_steering_primary_channel_mask) {
        sli_zigbee_af_network_steering_state = getFirstPrimaryState();
        channelsToScan = sli_zigbee_af_network_steering_primary_channel_mask;
      } else {
        sli_zigbee_af_network_steering_state = getFirstSecondaryState();
        channelsToScan = sli_zigbee_af_network_steering_secondary_channel_mask;
      }

      // Stop any previous trust center link key update.
      sl_zigbee_af_update_tc_link_key_stop();

      status = scheduleScan(channelsToScan);
    } else {
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
      // The test harness needs to set up the correct link key
      sl_zigbee_af_event_set_active(&sl_zigbee_af_test_harness_z3_open_network_event);
#endif
      status = sl_zigbee_af_permit_join(SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_COMMISSIONING_TIME_S,
                                        true); // Broadcast permit join?
    }
  } else {
    sl_zigbee_af_core_println("%s is already in progress",
                              sli_zigbee_af_network_steering_plugin_name);
  }

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            sli_zigbee_af_network_steering_plugin_name,
                            "Start",
                            status);

  return status;
}

sl_status_t sl_zigbee_af_network_steering_stop(void)
{
  if (sli_zigbee_af_network_steering_state
      == SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_NONE) {
    return SL_STATUS_INVALID_STATE;
  }
  cleanupAndStop(SL_STATUS_NOT_JOINED);
  return SL_STATUS_OK;
}

uint32_t sl_zigbee_af_network_steering_get_channel_mask(bool primary_mask)
{
  return primary_mask ? sli_zigbee_af_network_steering_primary_channel_mask
         : sli_zigbee_af_network_steering_secondary_channel_mask;
}

void sli_zigbee_af_network_steering_set_configured_key(const uint8_t *key,
                                                       bool useConfiguredKey)
{
  if (!key) {
    return;
  }
  memcpy(gConfiguredKey.contents, key, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  gUseConfiguredKey = useConfiguredKey;
}

// =============================================================================
// Finish Steering

// At the end of the network steering process, we need to update the
// trust center link key (if we are in a centralized network) and broadcast
// a permit join to extend the network. This process needs to happen after
// we send our device announce and possibly our network timeout request if we
// are an end device.

void sl_zigbee_af_network_steering_finish_steering_event_handler(sl_zigbee_af_event_t * event)
{
  (void)event;
  sl_status_t status;
  sl_zigbee_af_event_set_inactive(finishSteeringEvent);

  if (sli_zigbee_af_network_steering_state_update_tclk()) {
    // Start the process to update the TC link key. We will set another event
    // for the broadcast permit join.
    // Attempt a TC link key update now.
    sl_zigbee_af_update_tc_link_key_set_delay(0);
  } else {
    // Broadcast permit join to extend the network.
    // We are done!
    status = sl_zigbee_af_permit_join(SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_COMMISSIONING_TIME_S,
                                      true); // Broadcast permit join?
    sl_zigbee_af_core_println("%s: %s: 0x%02X",
                              PLUGIN_NAME,
                              "Broadcasting permit join",
                              status);
    cleanupAndStop(status);
    sl_zigbee_af_remove_from_current_app_tasks(SL_ZIGBEE_AF_WAITING_FOR_TC_KEY_UPDATE);
  }
}

void sl_zigbee_af_update_tc_link_key_status_cb(sl_zigbee_key_status_t keyStatus)
{
  if (sli_zigbee_af_network_steering_state_update_tclk()) {
    sl_zigbee_af_core_println("%s: %s: 0x%02X",
                              PLUGIN_NAME,
                              "Trust center link key update status",
                              keyStatus);
    switch (keyStatus) {
      case SL_ZIGBEE_TRUST_CENTER_LINK_KEY_ESTABLISHED:
        // Success! But we should still wait to make sure we verify the key.
        return;
      case SL_ZIGBEE_TRUST_CENTER_IS_PRE_R21:
      case SL_ZIGBEE_VERIFY_LINK_KEY_SUCCESS:
      {
        // If the trust center is pre-r21, then we don't update the link key.
        // If the key status is that the link key has been verified, then we
        // have successfully updated our trust center link key and we are done!
        uint32_t ms = randomJitterMS();
        sl_zigbee_af_event_set_delay_ms(finishSteeringEvent, ms);
        break;
      }
      default:
        // Failure!
        sl_zigbee_leave_network(SL_ZIGBEE_LEAVE_NWK_WITH_NO_OPTION);
        cleanupAndStop(SL_STATUS_ZIGBEE_NO_LINK_KEY_RECEIVED);
    }
    sli_zigbee_af_network_steering_state_clear_update_tclk();
  }

  return;
}

void sli_zigbee_af_network_steering_set_channel_mask(uint32_t mask, bool secondaryMask)
{
  if (secondaryMask) {
    sli_zigbee_af_network_steering_secondary_channel_mask = mask;
  } else {
    sli_zigbee_af_network_steering_primary_channel_mask = mask;
  }
}

void sli_zigbee_af_network_steering_set_extended_pan_id_filter(uint8_t* extendedPanId,
                                                               bool turnFilterOn)
{
  if (!extendedPanId) {
    return;
  }
  memcpy(gExtendedPanIdToFilterOn,
         extendedPanId,
         COUNTOF(gExtendedPanIdToFilterOn));
  gFilterByExtendedPanId = turnFilterOn;
}

void sli_zigbee_af_network_steering_cleanup(sl_status_t status)
{
  cleanupAndStop(status);
}

uint8_t sli_zigbee_af_network_steering_get_current_channel()
{
  return beacon.channel;
}

void sl_zigbee_af_network_steering_autostart(void)
{
#ifdef ENABLE_STEERING_AUTOSTART
  sl_status_t status = sl_zigbee_af_network_steering_start();
  sl_zigbee_af_core_println("%s network %s: 0x%02X", "Join", "start", status);
#else
  sl_zigbee_af_core_println("Network steering: auto-start disabled by configuration");
#endif
  return;
}

#endif //  OPTIMIZE_SCANS

//------------------------------------------------------------------------------
// Weak callback definitions.

WEAK(bool sl_zigbee_af_network_steering_get_distributed_key_cb(sl_zigbee_key_data_t * key))
{
  (void)key;
  return false;
}

WEAK(sl_zigbee_node_type_t sl_zigbee_af_network_steering_get_node_type_cb(sl_zigbee_af_plugin_network_steering_joining_state_t state))
{
  (void)state;
  return ((sli_zigbee_af_current_zigbee_pro_network->nodeType == SL_ZIGBEE_COORDINATOR)
          ? SL_ZIGBEE_ROUTER
          : sli_zigbee_af_current_zigbee_pro_network->nodeType);
}
