/***************************************************************************//**
 * @file
 * @brief General Procedure
 *
 * 1. Setup stack security with an Install code key (if available)
 *    If not available, skip to step 9.
 * 2. Set the channel mask to the Primary set of channels.
 * 3. Perform a scan of open networks on a single channel.
 * 4. For all beacons collected on that channel, filter out PAN IDs that are not open
 *    for joining or don't have the right stack profile (2), or are duplicates.
 * 5. For each discovered network, try to join.
 * 6. Repeat steps 3-5 until all channels in the mask are tried.
 * 7. If no networks are successfully joined, setup secondary channel mask
 *    (all channels) and repeat steps 3-6 with.
 * 8. If no networks are successfully joined, change the security
 *    to use the default well-known "ZigbeeAlliance09" key.
 * 9. Repeat steps 2-7
 *    (This will again try primary channel mask first, then secondary).
 * 10. If no networks are successsully joined, setup security to use
 *     distributed link key.
 * 11. Repeat steps 2.7.
 * 12. If no networks are joined successfully fail the whole process and return
 *     control to the application.
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

#include "app/framework/security/af-security.h" // sli_zigbee_af_clear_link_key_table()
#include "network-steering.h"
#include "network-steering-internal.h"

#include "stack/include/zigbee-security-manager.h"

#ifndef OPTIMIZE_SCANS

#include "app/framework/plugin/scan-dispatch/scan-dispatch.h"
#include "app/framework/plugin/update-tc-link-key/update-tc-link-key.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
extern sl_zigbee_event_control_t sl_af_test_harness_z3_opent_network_event_control;
#endif

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
  // These next two states are only run if explicitly configured to do so
  // See sli_zigbee_af_network_steering_set_configured_key()
  "Scan Primary Channels and use Configured Key",
  "Scan Secondary Channels and use Configured Key",
  "Scan Primary Channels and use Install Code",
  "Scan Secondary Channels and use Install Code",
  "Scan Primary Channels and Use Centralized Key",
  "Scan Secondary Channels and Use Centralized Key",
  "Scan Primary Channels and Use Distributed Key",
  "Scan Secondary Channels and Use Distributed Key",
};

#define LAST_JOINING_STATE \
  SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_DISTRIBUTED

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

#define SECONDARY_CHANNEL_MASK SL_ZIGBEE_ALL_802_15_4_CHANNELS_MASK

#define REQUIRED_STACK_PROFILE 2

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

static bool printedMaxPanIdsWarning = false;
uint8_t sli_zigbee_af_network_steering_pan_id_index = 0;
uint8_t sli_zigbee_af_network_steering_current_channel;

// We make these into variables so that they can be changed at run time.
// This is very useful for unit and interop tests.
uint32_t sli_zigbee_af_network_steering_primary_channel_mask
  = SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_CHANNEL_MASK;
uint32_t sli_zigbee_af_network_steering_secondary_channel_mask
  = SECONDARY_CHANNEL_MASK;

uint8_t sli_zigbee_af_network_steering_total_beacons = 0;
uint8_t sli_zigbee_af_network_steering_join_attempts = 0;
sl_zigbee_key_data_t sl_zigbee_plugin_network_steering_distributed_key;

static uint32_t currentChannelMask = 0;

sl_zigbee_af_event_t sl_zigbee_af_network_steering_finish_steering_event[SL_ZIGBEE_SUPPORTED_NETWORKS];
#define finishSteeringEvent (sl_zigbee_af_network_steering_finish_steering_event)
void sl_zigbee_af_network_steering_finish_steering_event_handler(sl_zigbee_af_event_t * event);

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

#define MAX_NETWORKS 16

static uint16_t storedNetworks[MAX_NETWORKS];
static bool memoryCleared = false;

//============================================================================
// Forward Declarations

static void cleanupAndStop(sl_status_t status);
static sl_status_t stateMachineRun(void);
static uint8_t getNextChannel(void);
static sl_status_t tryNextMethod(void);
static sl_status_t setupSecurity(void);
static uint32_t jitterTimeDelayMs();
HIDDEN void scanResultsHandler(sl_zigbee_af_plugin_scan_dispatch_scan_results_t *results);

bool sl_zigbee_stack_is_up(void);

// Callback declarations for the sake of the compiler during unit tests.
int8_t sl_zigbee_af_network_steering_get_power_for_radio_channel_cb(uint8_t channel);
bool sl_zigbee_af_network_steering_get_distributed_key_cb(sl_zigbee_key_data_t * key);
sl_zigbee_node_type_t sl_zigbee_af_network_steering_get_node_type_cb(sl_zigbee_af_plugin_network_steering_joining_state_t state);
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

static bool addPanIdCandidate(uint16_t panId)
{
  uint16_t* panIdPointer = sli_zigbee_af_network_steering_get_stored_pan_id_pointer(0);
  if (panIdPointer == NULL) {
    sl_zigbee_af_core_println("Error: %s could not get memory pointer for stored PAN IDs",
                              PLUGIN_NAME);
    return false;
  }
  uint8_t maxNetworks = sli_zigbee_af_network_steering_get_max_possible_pan_ids();
  uint8_t i;
  for (i = 0; i < maxNetworks; i++) {
    if (panId == *panIdPointer) {
      // We already know about this PAN, no point in recording it twice.
      debugPrintln("Ignoring duplicate PAN ID 0x%04X", panId);
      return true;
    } else if (*panIdPointer == SL_ZIGBEE_AF_INVALID_PAN_ID) {
      *panIdPointer = panId;
      debugPrintln("Stored PAN ID 0x%04X", *panIdPointer);
      return true;
    }
    panIdPointer++;
  }

  if (!printedMaxPanIdsWarning) {
    sl_zigbee_af_core_println("Warning: %s Max PANs reached (%d)",
                              PLUGIN_NAME,
                              maxNetworks);
    printedMaxPanIdsWarning = true;
  }
  return true;
}

static void clearPanIdCandidates(void)
{
  printedMaxPanIdsWarning = false;
  sli_zigbee_af_network_steering_clear_stored_pan_ids();
  sli_zigbee_af_network_steering_pan_id_index = 0;
}

static uint16_t getNextCandidate(void)
{
  debugPrintln("Getting candidate at index %d", sli_zigbee_af_network_steering_pan_id_index);
  uint16_t* pointer =
    sli_zigbee_af_network_steering_get_stored_pan_id_pointer(sli_zigbee_af_network_steering_pan_id_index);
  if (pointer == NULL) {
    debugPrintln("Error: %s could not get pointer to stored PANs", PLUGIN_NAME);
    return SL_ZIGBEE_AF_INVALID_PAN_ID;
  }
  sli_zigbee_af_network_steering_pan_id_index++;
  return *pointer;
}

static void gotoNextChannel(void)
{
  sl_zigbee_af_plugin_scan_dispatch_scan_data_t scanData;
  sl_status_t status;

  sli_zigbee_af_network_steering_current_channel = getNextChannel();
  if (sli_zigbee_af_network_steering_current_channel == 0) {
    debugPrintln("No more channels");
    tryNextMethod();
    return;
  }
  clearPanIdCandidates();

  // Set radio power before active scan to configure beacon request
  // Tx power level based on the plugin setting
  (void)sl_zigbee_set_radio_power(SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_RADIO_TX_POWER);

  scanData.scanType = SL_ZIGBEE_ACTIVE_SCAN;
  scanData.channelMask = BIT32(sli_zigbee_af_network_steering_current_channel);
  scanData.duration = SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_SCAN_DURATION;
  scanData.handler = scanResultsHandler;
  status = sl_zigbee_af_scan_dispatch_schedule_scan(&scanData);

  if (SL_STATUS_OK != status) {
    sl_zigbee_af_core_println("Error: %s start scan failed: 0x%02X", PLUGIN_NAME, status);
    cleanupAndStop(status);
  } else {
    sl_zigbee_af_core_println("Starting scan on channel %d",
                              sli_zigbee_af_network_steering_current_channel);
  }
}

void tryToJoinNetwork(void)
{
  sl_zigbee_network_parameters_t networkParams;
  sl_status_t status;
  sl_zigbee_node_type_t nodeType;

  memset(&networkParams, 0, sizeof(sl_zigbee_network_parameters_t));

  networkParams.panId = getNextCandidate();
  if (networkParams.panId == SL_ZIGBEE_AF_INVALID_PAN_ID) {
    debugPrintln("No networks to join on channel %d", sli_zigbee_af_network_steering_current_channel);
    gotoNextChannel();
    return;
  }

  sl_zigbee_af_core_println("%s joining 0x%04X", PLUGIN_NAME, networkParams.panId);
  networkParams.radioChannel = sli_zigbee_af_network_steering_current_channel;
  networkParams.radioTxPower = GET_RADIO_TX_POWER(sli_zigbee_af_network_steering_current_channel);
  nodeType = sl_zigbee_af_network_steering_get_node_type_cb(sli_zigbee_af_network_steering_state);
  status = sl_zigbee_join_network(nodeType, &networkParams);
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

// TODO: renamed for naming consistency purposes
void sli_zigbee_af_network_steering_stack_status_callback(sl_status_t status)
{
  if (sli_zigbee_af_network_steering_state
      == SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_NONE) {
    if (status == SL_STATUS_NETWORK_UP && sl_zigbee_af_update_tc_link_key_is_tclk_key_default()) {
      sl_zigbee_af_update_tc_link_key_set_delay(jitterTimeDelayMs());
    } else if (status == SL_STATUS_NETWORK_DOWN) {
      sl_zigbee_af_update_tc_link_key_set_inactive();
    }
    return;
  } else if (status == SL_STATUS_NETWORK_UP) {
    sl_zigbee_af_core_println("%s network joined.", PLUGIN_NAME);
    if (!sli_zigbee_af_network_steering_state_uses_distributed_key()
        && !(sli_zigbee_af_network_steering_options_mask
             & SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_OPTIONS_NO_TCLK_UPDATE)) {
#ifdef SL_CATALOG_ZIGBEE_DYNAMIC_COMMISSIONING_PRESENT
      // If we are network up after a successful DLK, we do not need to update our TCLK
      if (sl_zigbee_get_stack_compliance_revision() == R23_COMPLIANCE_REVISION && sl_zigbee_zdo_dlk_enabled()) {
        uint32_t tok;
        if (SL_STATUS_OK == sl_zigbee_get_trust_center_additional_info(&tok) && (tok & EXTENDED_BIT_MASK_DERIVED_KEY_DLK)) {
          sl_zigbee_af_core_println("%s: Retrieving authentication token for DLK", PLUGIN_NAME);
          sl_zigbee_retrieve_authentication_token(SL_ZIGBEE_TRUST_CENTER_NODE_ID, (SL_ZIGBEE_APS_OPTION_ENCRYPTION | SL_ZIGBEE_APS_OPTION_RETRY));
          sl_zigbee_af_event_set_delay_ms(finishSteeringEvent, randomJitterMS());
          return;
        }
      }
#endif // SL_CATALOG_ZIGBEE_DYNAMIC_COMMISSIONING_PRESENT
      sli_zigbee_af_network_steering_state_set_update_tclk();
    }
    sl_zigbee_af_event_set_delay_ms(finishSteeringEvent, randomJitterMS());
    return;
  } else if (!sl_zigbee_stack_is_up()) {
    tryToJoinNetwork();
  } else {
  }
}

static void scanCompleteCallback(uint8_t channel, sl_status_t status)
{
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_core_println("Error: Scan complete handler returned 0x%02X on %d", status, channel);
  } else {   // when scan is over, we always return with success
    // EMAPPFWKV2-1462 - make sure we didn't cleanupAndStop() above.
    if (sli_zigbee_af_network_steering_state
        != SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_NONE) {
      tryToJoinNetwork();
    }
  }
}

static void networkFoundCallback(sl_zigbee_zigbee_network_t *networkFound,
                                 uint8_t lqi,
                                 int8_t rssi)
{
  UNUSED_VAR(lqi);
  UNUSED_VAR(rssi);

  sli_zigbee_af_network_steering_total_beacons++;

  if (!(networkFound->allowingJoin
        && networkFound->stackProfile == REQUIRED_STACK_PROFILE)) {
    return;
  }

  debugPrint("%s nwk found ch: %d, panID 0x%04X, xpan: ",
             PLUGIN_NAME,
             networkFound->channel,
             networkFound->panId);
  debugExec(sl_zigbee_af_print_big_endian_eui64(networkFound->extendedPanId));

  // If we were told to filter by extended PAN ID, do so now
  if (gFilterByExtendedPanId) {
    if (0 != memcmp(networkFound->extendedPanId,
                    gExtendedPanIdToFilterOn,
                    COUNTOF(gExtendedPanIdToFilterOn))) {
      debugPrint(". Skipping since we are looking for xpan: ");
      debugExec(sl_zigbee_af_print_big_endian_eui64(gExtendedPanIdToFilterOn));
      debugPrintln("");
      return;
    }
  }

  debugPrintln("");

  if (!addPanIdCandidate(networkFound->panId)) {
    sl_zigbee_af_core_println("Error: %s could not add candidate network.",
                              PLUGIN_NAME);
    cleanupAndStop(SL_STATUS_FAIL);
    return;
  }
}

HIDDEN void scanResultsHandler(sl_zigbee_af_plugin_scan_dispatch_scan_results_t *results)
{
  if (sl_zigbee_af_scan_dispatch_scan_results_are_complete(results)
      || sl_zigbee_af_scan_dispatch_scan_results_are_failure(results)) {
    scanCompleteCallback(results->channel, results->status);
  } else if (results->network && results->network->panId != SL_ZIGBEE_AF_INVALID_PAN_ID) {
    networkFoundCallback(results->network,
                         results->lqi,
                         results->rssi);
  }
}

static sl_status_t tryNextMethod(void)
{
  sli_zigbee_af_network_steering_state++;
  if (sli_zigbee_af_network_steering_state > LAST_JOINING_STATE) {
    sl_status_t status = (sli_zigbee_af_network_steering_total_beacons > 0
                          ? SL_STATUS_NOT_JOINED
                          : SL_STATUS_NO_BEACONS);
    cleanupAndStop(status);
    return status;
  }
  return stateMachineRun();
}

static void cleanupAndStop(sl_status_t status)
{
  sl_zigbee_af_core_println("%s Stop.  Cleaning up.", PLUGIN_NAME);
  sl_zigbee_af_network_steering_complete_cb(status,
                                            sli_zigbee_af_network_steering_total_beacons,
                                            sli_zigbee_af_network_steering_join_attempts,
                                            sli_zigbee_af_network_steering_state);
  sli_zigbee_af_network_steering_clear_stored_pan_ids();
  sli_zigbee_af_network_steering_state = SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_NONE;
  sli_zigbee_af_network_steering_pan_id_index = 0;
  sli_zigbee_af_network_steering_join_attempts = 0;
  sli_zigbee_af_network_steering_total_beacons = 0;
}

static uint8_t getNextChannel(void)
{
  if (sli_zigbee_af_network_steering_current_channel == 0) {
    sli_zigbee_af_network_steering_current_channel = (sl_zigbee_get_pseudo_random_number() & 0x0F)
                                                     + SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER;
    debugPrintln("Randomly choosing a starting channel %d.", sli_zigbee_af_network_steering_current_channel);
  } else {
    sli_zigbee_af_network_steering_current_channel++;
  }
  while (currentChannelMask != 0) {
    if (BIT32(sli_zigbee_af_network_steering_current_channel) & currentChannelMask) {
      currentChannelMask &= ~(BIT32(sli_zigbee_af_network_steering_current_channel));
      return sli_zigbee_af_network_steering_current_channel;
    }
    sli_zigbee_af_network_steering_current_channel++;
    if (sli_zigbee_af_network_steering_current_channel > SL_ZIGBEE_MAX_802_15_4_CHANNEL_NUMBER) {
      sli_zigbee_af_network_steering_current_channel = SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER;
    }
  }
  return 0;
}

static sl_status_t stateMachineRun(void)
{
  sl_status_t status = SL_STATUS_OK;
  sl_zigbee_af_core_println("%s State: %s",
                            PLUGIN_NAME,
                            sli_zigbee_af_network_steering_stateNames[sli_zigbee_af_network_steering_state]);

  if ((status = setupSecurity()) != SL_STATUS_OK) {
    sl_zigbee_af_core_println("Error: %s could not setup security: 0x%02X",
                              PLUGIN_NAME,
                              status);
    return tryNextMethod();
  }

  switch (sli_zigbee_af_network_steering_state) {
    case SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CONFIGURED:
    case SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE:
    case SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CENTRALIZED:
    case SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_DISTRIBUTED:
      currentChannelMask = sli_zigbee_af_network_steering_primary_channel_mask;
      break;
    default:
      currentChannelMask = sli_zigbee_af_network_steering_secondary_channel_mask;
      break;
  }

  debugPrintln("Channel Mask: 0x%08X", currentChannelMask);
  gotoNextChannel();
  return status;
}

static sl_status_t setupSecurity(void)
{
  sl_status_t status;
  sl_zigbee_initial_security_state_t state;
  sl_zigbee_extended_security_bitmask_t extended;

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

  if ((status = sl_zigbee_set_initial_security_state(&state))
      != SL_STATUS_OK) {
    if (sli_zigbee_af_network_steering_state_uses_install_codes()
        && (status == SL_STATUS_INVALID_CONFIGURATION)) {
      sl_zigbee_af_core_println("Error: install code setup failed. Is an install "
                                "code flashed onto the device?");
    }
    goto done;
  }

  extended = (SL_ZIGBEE_JOINER_GLOBAL_LINK_KEY
              | SL_ZIGBEE_EXT_NO_FRAME_COUNTER_RESET);

  if ((status = sl_zigbee_set_extended_security_bitmask(extended)) != SL_STATUS_OK) {
    goto done;
  }

  sli_zigbee_af_clear_link_key_table();

  done:
  return status;
}

sl_status_t sl_zigbee_af_network_steering_start(void)
{
  sl_status_t status = SL_STATUS_INVALID_STATE;
  if (sli_zigbee_af_pro_is_current_network()
      && (sli_zigbee_af_network_steering_state
          == SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_NONE)) {
    if (sl_zigbee_af_network_state() == SL_ZIGBEE_NO_NETWORK) {
      sl_zigbee_af_add_to_current_app_tasks_cb(SL_ZIGBEE_AF_WAITING_FOR_TC_KEY_UPDATE);     // to force sleepy device do short poll
      sli_zigbee_af_network_steering_state = gUseConfiguredKey
                                             ? SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CONFIGURED
                                             : SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE;

      // Stop any previous trust center link key update.
      sl_zigbee_af_update_tc_link_key_stop();

      status = stateMachineRun();
    } else {
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

// =============================================================================
// Finish Steering

// At the end of the network steering process, we need to update the
// trust center link key (if we are in a centralized network) and broadcast
// a permit join to extend the network. This process needs to happen after
// we send our device announce and possibly our network timeout request if we
// are an end device.
void sl_zigbee_af_network_steering_finish_steering_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);

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
        // If the trust center is pre-r21, then we don't update the link key.
        // If the key status is that the link key has been verified, then we
        // have successfully updated our trust center link key and we are done!
        sl_zigbee_af_event_set_delay_ms(finishSteeringEvent, randomJitterMS());
        break;
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

void sli_zigbee_af_network_steering_set_configured_key(const uint8_t *key,
                                                       bool useConfiguredKey)
{
  if (!key) {
    return;
  }
  memcpy(gConfiguredKey.contents, key, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  gUseConfiguredKey = useConfiguredKey;
}

void sli_zigbee_af_network_steering_cleanup(sl_status_t status)
{
  cleanupAndStop(status);
}

uint8_t sli_zigbee_af_network_steering_get_current_channel()
{
  return sli_zigbee_af_network_steering_current_channel;
}

uint8_t sli_zigbee_af_network_steering_get_max_possible_pan_ids(void)
{
  return MAX_NETWORKS;
}

void sli_zigbee_af_network_steering_clear_stored_pan_ids(void)
{
  memoryCleared = false;
}

uint16_t* sli_zigbee_af_network_steering_get_stored_pan_id_pointer(uint8_t index)
{
  if (index >= MAX_NETWORKS) {
    return NULL;
  }
  if (!memoryCleared) {
    memset(storedNetworks, 0xFF, sizeof(uint16_t) * MAX_NETWORKS);
    memoryCleared = true;
  }
  return &(storedNetworks[index]);
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

#endif // OPTIMIZE_SCANS
