/***************************************************************************//**
 * @file
 * @brief APIs and defines for the Network Creator plugin, which handles forming
 *        a network per the Zigbee Base Device Behavior specification.
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
#include "app/framework/util/util.h"

#include "network-creator.h"
#include "network-creator-composite.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_NETWORK_CREATOR_PRESENT
#include "network-creator-config.h"
#else
#include "app/framework/plugin/network-creator/config/network-creator-config.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_SCAN_DISPATCH_PRESENT
#include "scan-dispatch.h"
#else
#include "app/framework/plugin/scan-dispatch/scan-dispatch.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_NETWORK_CREATOR_SECURITY_PRESENT
#include "network-creator-security.h"
#else
#include "app/framework/plugin/network-creator-security/network-creator-security.h"
#endif

#ifdef SL_ZIGBEE_TEST
  #ifndef SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SCAN_DURATION
    #define SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SCAN_DURATION 0x04
  #endif
  #ifndef SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_MASK
    #define SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_MASK 0x02108800
  #endif
  #ifndef SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_RADIO_POWER
    #define SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_RADIO_POWER 3
  #endif
  #define HIDDEN
#else
  #define HIDDEN static
#endif

//#define EM_AF_PLUGIN_NETWORK_CREATOR_DEBUG
#ifdef EM_AF_PLUGIN_NETWORK_CREATOR_DEBUG
  #define debugPrintln(...) sl_zigbee_af_core_println(__VA_ARGS__)
#else
  #define debugPrintln(...)
#endif

// -----------------------------------------------------------------------------
// Globals

uint32_t sli_zigbee_af_network_creator_primary_channel_mask = SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_MASK;

// The Base Device spec (13-0402) says, by default, define the secondary
// channel mask to be all channels XOR the primary mask. See 6.2, table 2.
#define SECONDARY_CHANNEL_MASK \
  (SL_ZIGBEE_ALL_802_15_4_CHANNELS_MASK ^ SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_MASK)
uint32_t sli_zigbee_af_network_creator_secondary_channel_mask = (uint32_t)SECONDARY_CHANNEL_MASK;

static uint32_t currentChannelMask;

static sli_zigbee_af_network_creator_channel_composite channelComposites[SL_ZIGBEE_NUM_802_15_4_CHANNELS];

static bool doFormCentralizedNetwork = true;

static uint8_t stateFlags = 0;
#define clearStateFlags() (stateFlags = 0)

#define STATE_FLAGS_WAITING_FOR_SCAN (0x01)
#define waitingForScan() (stateFlags & STATE_FLAGS_WAITING_FOR_SCAN)

#define STATE_FLAGS_MASK_IS_SECONDARY (0x02)
#define maskIsSecondary() (stateFlags & STATE_FLAGS_MASK_IS_SECONDARY)

#define FINAL_SCAN_TYPE SL_ZIGBEE_ACTIVE_SCAN

// -----------------------------------------------------------------------------
// Declarations

// For the sake of the compiler in unit tests.
void sl_zigbee_af_network_creator_complete_cb(const sl_zigbee_network_parameters_t *network,
                                              bool usedSecondaryChannels);
sl_802154_pan_id_t sl_zigbee_af_network_creator_get_pan_id_cb(void);
int8_t sl_zigbee_af_network_creator_get_radio_power_cb(void);

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static sl_status_t scheduleScans(void);

static void cleanChannelComposites(void);
static void updateChannelComposites(int8_t rssi,
                                    sli_zigbee_af_network_creator_channel_compositeMetric metric,
                                    uint8_t channel);
static void maybeClearChannelBitOfMaxRssiReading(uint8_t channel);

static void cleanupAndStop(sl_status_t status);
static void fillExtendedPanId(uint8_t *extendedPanId);
static uint32_t makeLeastWorstChannelMask(void);

// -----------------------------------------------------------------------------
// Public API Definitions

sl_status_t sl_zigbee_af_network_creator_start(bool centralizedNetwork)
{
  if (waitingForScan() || (sl_zigbee_af_network_state() != SL_ZIGBEE_NO_NETWORK)) {
    sl_zigbee_af_core_println("%s: %s: 0x%02X",
                              SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                              "Cannot start. State",
                              stateFlags);
    return SL_STATUS_INVALID_STATE;
  }

  doFormCentralizedNetwork = centralizedNetwork;

  // Reset channel masks and composites and state.
  currentChannelMask = sli_zigbee_af_network_creator_primary_channel_mask;
  cleanChannelComposites();
  clearStateFlags();

  return scheduleScans();
}

void sl_zigbee_af_network_creator_stop(void)
{
  cleanupAndStop(SL_STATUS_FAIL);
}

sl_status_t sl_zigbee_af_network_creator_network_form(bool centralizedNetwork,
                                                      sl_802154_pan_id_t panId,
                                                      int8_t radioTxPower,
                                                      uint8_t channel)
{
  sl_status_t status;
  sl_zigbee_network_parameters_t networkParameters;

  networkParameters.panId = panId;
  networkParameters.radioTxPower = radioTxPower;
  fillExtendedPanId(networkParameters.extendedPanId);
  networkParameters.radioChannel = channel;

  if ((channel > SL_ZIGBEE_MAX_802_15_4_CHANNEL_NUMBER)
      || (channel < SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER)) {
    sl_zigbee_af_core_println("%s: Error: Channel %d is out of range",
                              SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                              channel);
    return SL_STATUS_INVALID_PARAMETER;
  }

  doFormCentralizedNetwork = centralizedNetwork;

  status = sl_zigbee_af_network_creator_security_start(centralizedNetwork);
  if (status == SL_STATUS_OK) {
    status = sl_zigbee_af_form_network(&networkParameters);
    sl_zigbee_af_core_println("%s: Form. Channel: %d. Status: 0x%02X",
                              SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                              channel,
                              status);
    sl_zigbee_af_network_creator_complete_cb((const sl_zigbee_network_parameters_t *) &networkParameters,
                                             false);
  }

  return status;
}

// -----------------------------------------------------------------------------
// Private API Definitions

static void fillExtendedPanId(uint8_t *extendedPanId)
{
  uint8_t i;
  bool invalid = true;

  for (i = 0; i < EXTENDED_PAN_ID_SIZE && invalid; i++) {
    invalid = (sli_zigbee_af_extended_pan_id[i] == 0x00 || sli_zigbee_af_extended_pan_id[i] == 0xFF);
  }

  if (invalid) {
    for (i = 0; i < EXTENDED_PAN_ID_SIZE; i++) {
      extendedPanId[i] = sl_zigbee_get_pseudo_random_number();
    }
  } else {
    memmove(extendedPanId, sli_zigbee_af_extended_pan_id, EXTENDED_PAN_ID_SIZE);
  }
}

static sl_status_t tryToFormNetwork(void)
{
  sl_zigbee_network_parameters_t networkParameters;
  sl_status_t status;
  uint8_t channel = (sl_zigbee_get_pseudo_random_number() & 0x0F)
                    + SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER;
  sl_802154_pan_id_t panId = sl_zigbee_af_network_creator_get_pan_id_cb();

  networkParameters.panId = panId;
  networkParameters.radioTxPower = sl_zigbee_af_network_creator_get_radio_power_cb();
  fillExtendedPanId(networkParameters.extendedPanId);

  // If we are on our secondary channel mask and we didn't find any channels that
  // were below our channel composite thresholds, then we pick from the "least
  // worst channels" so that network formation is still possible.
  if (maskIsSecondary() && currentChannelMask == 0) {
    currentChannelMask = makeLeastWorstChannelMask();
  }

  status = sl_zigbee_af_network_creator_security_start(doFormCentralizedNetwork);
  if (status == SL_STATUS_OK) {
    status = SL_STATUS_FAIL;
    while (currentChannelMask != 0 && status != SL_STATUS_OK) {
      // Find the next channel on which to try forming a network.
      channel = (channel == SL_ZIGBEE_MAX_802_15_4_CHANNEL_NUMBER
                 ? SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER
                 : channel + 1);
      if (!READBIT(currentChannelMask, channel)) {
        continue;
      }

      // Try to form the network.
      networkParameters.radioChannel = channel;
      status = sl_zigbee_af_form_network(&networkParameters);
      sl_zigbee_af_core_println("%s: Form. Channel: %d. Status: 0x%02X",
                                SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                                channel,
                                status);

      // If you pass, then tell the user.
      // Else, clear the channel bit that you just tried.
      if (status == SL_STATUS_OK) {
        sl_zigbee_af_network_creator_complete_cb(&networkParameters,
                                                 (maskIsSecondary()
                                                  ? true
                                                  : false));
      } else {
        CLEARBIT(currentChannelMask, channel);
      }
    }
  }

  return status;
}

static void handleScanComplete(sl_zigbee_af_plugin_scan_dispatch_scan_results_t *results)
{
  sl_zigbee_network_scan_type_t scanType
    = sl_zigbee_af_scan_dispatch_scan_results_get_scan_type(results);

  // If then scan was unsuccessful...
  if (results->status != SL_STATUS_OK) {
    // ...just turn off the channel on which the scan failed. The
    // network-creator will disregard this channel in the network
    // formation process.
    CLEARBIT(currentChannelMask, results->channel);
    return;
  }

  // If we're done, try to form
  if (scanType == FINAL_SCAN_TYPE) {
    results->status = tryToFormNetwork();
    // If we were not successful...
    if (results->status != SL_STATUS_OK) {
      // ...then try the secondary mask if we were on the primary...
      // ...else fail because we tried both masks.
      if (!maskIsSecondary()) {
        currentChannelMask = sli_zigbee_af_network_creator_secondary_channel_mask;
        SETBITS(stateFlags, STATE_FLAGS_MASK_IS_SECONDARY);
        scheduleScans();
      } else {
        cleanupAndStop(results->status);
      }
    } else {
      // If we were successful, then all done!
      cleanupAndStop(results->status);
    }
  }
}

HIDDEN void scanHandler(sl_zigbee_af_plugin_scan_dispatch_scan_results_t *results)
{
  sl_zigbee_network_scan_type_t scanType
    = sl_zigbee_af_scan_dispatch_scan_results_get_scan_type(results);

  if (sl_zigbee_af_scan_dispatch_scan_results_are_failure(results)) {
    // If we are here, that means the call to sl_zigbee_start_scan was a failure
    // in the scan-dispatch plugin (see scan-dispatch.h). So fail.
    cleanupAndStop(results->status);
  } else { // success
    if (sl_zigbee_af_scan_dispatch_scan_results_are_complete(results)) {
      debugPrintln("Scan complete. Channel: %d. Status: 0x%02X",
                   results->channel,
                   results->status);
      handleScanComplete(results);
    } else { // results
      if (scanType == SL_ZIGBEE_ACTIVE_SCAN) {
        debugPrintln("Found network!");
        debugPrintln("  PanId: 0x%04X, Channel: %d, PJoin: %s",
                     results->network->panId,
                     results->network->channel,
                     (results->network->allowingJoin ? "YES" : "NO"));
        debugPrintln("  lqi:  %d", results->lqi);
        debugPrintln("  rssi: %d", results->rssi);

        updateChannelComposites(results->rssi,
                                EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_METRIC_BEACONS,
                                results->network->channel);
      } else if (scanType == SL_ZIGBEE_ENERGY_SCAN) {
        debugPrintln("Energy scan results.");
        debugPrintln("%s: Channel: %d. Rssi: %d",
                     SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                     results->channel,
                     results->rssi);

        updateChannelComposites(results->rssi,
                                EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_METRIC_RSSI,
                                results->channel);
      }
    }
  }
}

static sl_status_t scheduleScans()
{
  sl_status_t status;
  sl_zigbee_af_plugin_scan_dispatch_scan_data_t data = {
    .channelMask = currentChannelMask,
    .duration    = SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_SCAN_DURATION,
    .handler     = scanHandler,
  };

  // Set Radio Power prior to active scan so the beacon request
  // tx power level is configurable using the plugin radio power value
  (void)sl_zigbee_set_radio_power(SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_RADIO_POWER);

  // Energy first
  data.scanType = SL_ZIGBEE_ENERGY_SCAN;
  status = sl_zigbee_af_scan_dispatch_schedule_scan(&data);

  // Active second
  data.scanType = SL_ZIGBEE_ACTIVE_SCAN;
  if (status == SL_STATUS_OK) {
    status = sl_zigbee_af_scan_dispatch_schedule_scan(&data);
  }

  return status;
}

static void updateChannelComposites(int8_t rssi,
                                    sli_zigbee_af_network_creator_channel_compositeMetric metric,
                                    uint8_t channel)
{
  uint8_t channelIndex = channel - SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER;

  // Even if the channel bit is off, we still update its channel composite. This
  // is so we can use the channel composites to find a "least worst channel" if
  // necessary.

  // Update the network composite value for this channel.
  switch (metric) {
    case EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_METRIC_BEACONS:
      channelComposites[channelIndex].beaconsHeard++;
      break;

    case EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_METRIC_RSSI:
      if (rssi > channelComposites[channelIndex].maxRssiHeard) {
        channelComposites[channelIndex].maxRssiHeard = rssi;
        maybeClearChannelBitOfMaxRssiReading(channel);
      }
      break;

    default:
      debugPrintln("Unknown metric value: %d", metric);
  }

  // If the channel is over the composite threshold, disregard the channel.
  if (sli_zigbee_af_network_creator_channel_composite_is_above_threshold(channelComposites[channelIndex])) {
    CLEARBIT(currentChannelMask, channel);
  }
}

static void cleanupAndStop(sl_status_t status)
{
  sl_zigbee_af_core_println("%s: Stop. Status: 0x%02X. State: 0x%02X",
                            SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                            status,
                            stateFlags);
}

static void cleanChannelComposites(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_NUM_802_15_4_CHANNELS; i++) {
    channelComposites[i].beaconsHeard
      = 0;
    channelComposites[i].maxRssiHeard
      = EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_INVALID_RSSI;
  }
}

static void maybeClearChannelBitOfMaxRssiReading(uint8_t channel)
{
  uint8_t i, channelsConsideredSoFar, maxIndex;

  // Find max RSSI index and how many channels have been considered so far.
  for (i = 0, channelsConsideredSoFar = 0, maxIndex = 0;
       i < SL_ZIGBEE_NUM_802_15_4_CHANNELS;
       i++) {
    // If we have received an RSSI reading from this channel,
    // and we are still considering this channel...
    if ((channelComposites[i].maxRssiHeard
         != EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_INVALID_RSSI)
        && (READBIT(currentChannelMask,
                    i + SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER))) {
      // ...increment the channelsConsideredSoFar by 1 and see if this
      // channel has the maximum RSSI.
      channelsConsideredSoFar++;
      if (channelComposites[i].maxRssiHeard
          > channelComposites[maxIndex].maxRssiHeard) {
        maxIndex = i;
      }
    }
  }

  // If the number of channels considered so far is more than the number of
  // channels that you want to consider, then remove the channel with the
  // maximum RSSI. Also reset the analogous channel composite so that we remember
  // that we are throwing away this channel.
  if ((channelsConsideredSoFar
       > EM_AF_PLUGIN_NETWORK_CREATOR_CHANNELS_TO_CONSIDER)) {
    uint8_t channelToThrowAway
      = ((channelComposites[channel - SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER].maxRssiHeard
          > channelComposites[maxIndex].maxRssiHeard)
         ? channel
         : maxIndex + SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER);
    CLEARBIT(currentChannelMask, channelToThrowAway);
    channelComposites[channelToThrowAway - SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER].maxRssiHeard
      = EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_INVALID_RSSI;
  }
}

static uint32_t makeLeastWorstChannelMask(void)
{
  uint32_t mask = 0;;
  for (uint8_t i = 0; i < SL_ZIGBEE_NUM_802_15_4_CHANNELS; i++) {
    if (channelComposites[i].maxRssiHeard
        != EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_INVALID_RSSI) {
      SETBIT(mask, i + SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER);
    }
  }
  return mask;
}

//------------------------------------------------------------------------------
// Weak callback definitions.

WEAK(sl_802154_pan_id_t sl_zigbee_af_network_creator_get_pan_id_cb(void))
{
  return sl_zigbee_get_pseudo_random_number();
}

WEAK(int8_t sl_zigbee_af_network_creator_get_radio_power_cb(void))
{
  return SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_RADIO_POWER;
}
