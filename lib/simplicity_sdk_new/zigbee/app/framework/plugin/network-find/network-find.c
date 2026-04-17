/***************************************************************************//**
 * @file
 * @brief Routines for finding and joining any viable network via scanning, rather
 * than joining a specific network.
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

#include "sl_common.h"
#include "app/framework/include/af.h"
#include "app/util/common/form-and-join.h"
#include "network-find.h"

#include "app/framework/util/util.h"    // sl_zigbee_af_get_page_from_8bit_encoded_chan_pg() etc

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "zigbee_stack_callback_dispatcher.h"
#ifdef SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT
#include "network-find-sub-ghz-config.h"
#endif
#if (SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_ENABLE_ALL_CHANNELS == 1)
#define ENABLE_ALL_CHANNELS
#endif
#if (SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_RADIO_TX_CALLBACK == 1)
#define RADIO_TX_CALLBACK
#endif

//------------------------------------------------------------------------------
// Forward Declaration

#ifdef SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK
static sl_status_t startSearchForUnusedNetwork(bool allChannels);
#endif
static sl_status_t startSearchForJoinableNetwork(uint32_t channelMask,
                                                 bool allChannels);
static sl_status_t startSearchForJoinableNetworkAllChannels(void);

//------------------------------------------------------------------------------
// Globals

enum {
  NETWORK_FIND_NONE,
#ifdef SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK
  NETWORK_FIND_FORM,
  NETWORK_FIND_FORM_ALL_CHANNELS,
  #ifdef SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT
  // Only with sub-GHz support do we have the secondary interface (even that
  // only on dual-PHY architectures but that if difficult to guard with a macro,
  // especially in a Host-NCP configuration).
  NETWORK_FIND_PHY2,
  NETWORK_FIND_PHY2_ALL_CHANNELS,
  #endif // SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT
#endif // SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK
  // NOTE: Important!
  // In order to resume the right kind of joinable network search, we have two
  // different waiting states. The code (see the two macros below) assumes that
  // they are in this specific order. Keep that in mind when editing states.
  NETWORK_FIND_JOIN,
  NETWORK_FIND_JOIN_ALL_CHANNELS,
  NETWORK_FIND_WAIT,
  NETWORK_FIND_WAIT_ALL_CHANNELS
};

// Macros to switch to the right flavor of join/wait state from the current state.
// See also the comment in the enum list above.
#define JOIN_STATE_TO_WAIT_STATE(state) ((state) - NETWORK_FIND_JOIN + NETWORK_FIND_WAIT)
#define WAIT_STATE_TO_JOIN_STATE(state) ((state) - NETWORK_FIND_WAIT + NETWORK_FIND_JOIN)

#if defined(SL_ZIGBEE_SCRIPTED_TEST)
  #define HIDDEN
#else
  #define HIDDEN static
#endif

HIDDEN uint8_t state = NETWORK_FIND_NONE;

#if defined(SL_ZIGBEE_SCRIPTED_TEST)
  #define SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_DURATION 5
extern uint32_t testFrameworkChannelMask;
  #define page0mask testFrameworkChannelMask

#elif (defined(EMBER_AF_PLUGIN_TEST_HARNESS) || defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT))
const uint32_t testHarnessOriginalChannelMask = SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_CHANNEL_MASK;
uint32_t testHarnessChannelMask = SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_CHANNEL_MASK;
  #define page0mask testHarnessChannelMask

#else
static uint32_t page0mask = SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_CHANNEL_MASK;
#endif

#ifdef SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT
static uint8_t currentPage = 0;
static uint32_t page28to31mask[SL_ZIGBEE_MAX_SUGBHZ_PAGE_NUMBER - SL_ZIGBEE_MIN_SUGBHZ_PAGE_NUMBER + 1] = SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_SUB_GHZ_CHANNEL_MASK;
  #define CHANNEL_28_TO_31_PAGE_MASK_WITH_PARAMETER(page, channelMaskArray) \
  ((((uint32_t)(page)) << SL_ZIGBEE_MAX_CHANNELS_PER_PAGE) | channelMaskArray[(page) - SL_ZIGBEE_MIN_SUGBHZ_PAGE_NUMBER])
  #define CHANNEL_28_TO_31_PAGE_MASK(page) \
  CHANNEL_28_TO_31_PAGE_MASK_WITH_PARAMETER(page, page28to31mask)

static const uint32_t page28to31maskAllChannels[] = SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_SUB_GHZ_ALL_CHANNELS_MASK;
  #define CHANNEL_28_TO_31_PAGE_ALL_CHANNELS_MASK(page) \
  CHANNEL_28_TO_31_PAGE_MASK_WITH_PARAMETER(page, page28to31maskAllChannels)

// Search mode bitmask for "find unused" and "find joinable".
// Permitted values are set by the FIND_AND_JOIN_MODE_... macros.
// The default value is to search all supported channels of all supported pages.
static uint8_t searchMode =
  FIND_AND_JOIN_MODE_ALLOW_2_4_GHZ | FIND_AND_JOIN_MODE_ALLOW_SUB_GHZ;
#endif // SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT

#if defined(RADIO_TX_CALLBACK)
  #define GET_RADIO_TX_POWER(pgChan)    sl_zigbee_af_network_find_get_radio_power_for_channel_cb(pgChan)
#elif defined(SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT)
  #define GET_RADIO_TX_POWER(pgChan)    (sl_zigbee_af_get_page_from_8bit_encoded_chan_pg(pgChan) == 0 \
                                         ? SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_RADIO_TX_POWER            \
                                         : SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_SUB_GHZ_RADIO_TX_POWER)
#else
  #define GET_RADIO_TX_POWER(pgChan)    SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_RADIO_TX_POWER
#endif

static uint8_t extendedPanIds[SL_ZIGBEE_SUPPORTED_NETWORKS][EXTENDED_PAN_ID_SIZE];
sl_zigbee_af_event_t sl_zigbee_af_network_find_tick_event;
#define networkFindTickEventControl (&sl_zigbee_af_network_find_tick_event)
void sl_zigbee_af_network_find_tick_event_handler(sl_zigbee_af_event_t * event);
//------------------------------------------------------------------------------

void sl_zigbee_af_network_find_init_cb(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
      sl_zigbee_af_event_init(networkFindTickEventControl,
                              sl_zigbee_af_network_find_tick_event_handler);
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA:
    {
      uint8_t extendedPanId[EXTENDED_PAN_ID_SIZE] = SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_EXTENDED_PAN_ID;
      uint8_t i;
      for (i = 0; i < SL_ZIGBEE_SUPPORTED_NETWORKS; i++) {
        memmove(extendedPanIds[i], extendedPanId, EXTENDED_PAN_ID_SIZE);
      }
      break;
    }

    default:
      break;
  }
}

void sl_zigbee_af_form_and_join_unused_pan_id_found_cb(sl_802154_pan_id_t panId, uint8_t channel)
{
  sli_zigbee_af_network_find_unused_pan_id_found_callback(panId, channel);
}

void sli_zigbee_af_network_find_unused_pan_id_found_callback(sl_802154_pan_id_t panId, uint8_t channel)
{
  sl_zigbee_network_parameters_t networkParams = { 0 };
  sl_status_t status;

  sl_zigbee_af_get_form_and_join_extended_pan_id_cb(networkParams.extendedPanId);
  networkParams.panId = panId;
  networkParams.radioChannel = channel;
  networkParams.radioTxPower = GET_RADIO_TX_POWER(channel);

  status = sl_zigbee_af_form_network(&networkParams);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_app_println("%s error 0x%02X %s", "Form",
                             status,
                             "aborting");
    sl_zigbee_af_app_flush();
    sli_zigbee_af_network_find_scan_error_callback(status);
  }
}

void sl_zigbee_af_joinable_network_found_cb(sl_zigbee_zigbee_network_t *networkFound,
                                            uint8_t lqi,
                                            int8_t rssi)
{
  sl_status_t status = SL_STATUS_FAIL;

  // NOTE: It's not necessary to check the networkFound->extendedPanId here
  // because the form-and-join utilities ensure this handler is only called
  // when the beacon in the found network has the same EPID as what we asked
  // for when we initiated the scan.  If the scan was requested for the EPID of
  // all zeroes, that's a special case that means any network is OK.  Either
  // way we can trust that it's OK to try joining the network params found in
  // the beacon.

  if (sl_zigbee_af_network_find_join_cb(networkFound, lqi, rssi)) {
    // Now construct the network parameters to join
    sl_zigbee_network_parameters_t networkParams;
    memset(&networkParams, 0, sizeof(sl_zigbee_network_parameters_t));
    memmove(networkParams.extendedPanId,
            networkFound->extendedPanId,
            EXTENDED_PAN_ID_SIZE);
    networkParams.panId = networkFound->panId;
    networkParams.radioChannel = networkFound->channel;
    networkParams.radioTxPower = GET_RADIO_TX_POWER(networkFound->channel);

    sl_zigbee_af_app_println("Nwk found, ch %d, panId 0x%04X, joining",
                             networkFound->channel,
                             networkFound->panId);

    status = sl_zigbee_af_join_network(&networkParams);
  }

  // Note that if the application wants to skip this network or if the join
  // fails, we can't continue the scan from here (by calling
  // sl_zigbee_scan_for_next_joinable_network) because that's the function that called
  // this handler in the first place, and we don't want to recurse.
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_app_println("%s error 0x%02X, %s",
                             "Join",
                             status,
                             "aborting");
    sl_zigbee_af_app_flush();
    sl_zigbee_af_event_set_active(networkFindTickEventControl);
  }
}

void sl_zigbee_af_form_and_join_network_found_cb(sl_zigbee_zigbee_network_t *networkFound,
                                                 uint8_t lqi,
                                                 int8_t rssi)
{
  sl_zigbee_af_joinable_network_found_cb(networkFound, lqi, rssi);
}

void sl_zigbee_af_network_find_tick_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_status_t status = SL_STATUS_OK;
  sl_zigbee_af_event_set_inactive(networkFindTickEventControl);
  if (state == NETWORK_FIND_JOIN || state == NETWORK_FIND_JOIN_ALL_CHANNELS) {
    // If the tick fires while we're searching for a joinable network, it means
    // we need to keep searching.  This can happen if the join fails or if the
    // application decided to leave the network because it was the wrong
    // network.
    sl_zigbee_af_app_println("Continue %s search", "joinable network");
    sl_zigbee_scan_for_next_joinable_network();
#if defined(SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK) && defined(SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT)
  } else if (state == NETWORK_FIND_PHY2 || state == NETWORK_FIND_PHY2_ALL_CHANNELS) {
    state = NETWORK_FIND_NONE;
#endif
  } else {
    // In (almost) all other cases, we're done and can clean up.
    sl_zigbee_af_app_println("Network find complete");
    sl_zigbee_form_and_join_cleanup(SL_STATUS_OK);
#if defined(SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK) && defined(SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT)
    // If the tick has fired while forming AND we are running on a dual-PHY
    // device, then after forming the network on 2.4GHz we need to kick off
    // a new energy scan on sub-GHz bands and start a second interface.
    if ((state == NETWORK_FIND_FORM || state == NETWORK_FIND_FORM_ALL_CHANNELS)
        && currentPage == 0
        && (searchMode & FIND_AND_JOIN_MODE_ALLOW_SUB_GHZ) != 0) {
      const uint8_t phyInterfaceCount = sl_zigbee_get_phy_interface_count();
      if (phyInterfaceCount > 1 && phyInterfaceCount != 0xFF) {
        currentPage = SL_ZIGBEE_MIN_SUGBHZ_PAGE_NUMBER;
        status = sl_zigbee_scan_for_unused_pan_id(CHANNEL_28_TO_31_PAGE_MASK(currentPage),
                                                  SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_DURATION);
        if (status == SL_STATUS_OK) {
          state = NETWORK_FIND_PHY2;
          // We are not quite finished yet, avoid calling the finished callback.
          return;
        }
      }
    }
#endif
    state = NETWORK_FIND_NONE;
    sl_zigbee_af_network_find_finished_cb(status);
  }
}

static void printScanAllChannelsPrompt(void)
{
  sl_zigbee_af_app_println("%s, no suitable primary channel found.",
                           "Network find complete");
  sl_zigbee_af_app_println("Scanning again, using all channels.");
}

void sli_zigbee_af_network_find_scan_error_callback(sl_status_t status)
{
  switch (status) {
    case SL_STATUS_NO_BEACONS:
#ifdef SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT
      // Finished scanning the current page, try the next page if available
      sl_zigbee_af_core_println("%s %s on page %d", "Form", "and join scan done", currentPage);
      if ((searchMode & FIND_AND_JOIN_MODE_ALLOW_SUB_GHZ)
          && currentPage < SL_ZIGBEE_MAX_SUGBHZ_PAGE_NUMBER) {
        bool const allChannels = (state == NETWORK_FIND_JOIN_ALL_CHANNELS);
        uint32_t newPageMask;
        if (currentPage == 0) {
          currentPage = SL_ZIGBEE_MIN_SUGBHZ_PAGE_NUMBER;
        } else {
          currentPage++;
        }
        state = NETWORK_FIND_NONE;        // have to reset state before calling the fn
        newPageMask = allChannels
                      ? CHANNEL_28_TO_31_PAGE_ALL_CHANNELS_MASK(currentPage)
                      : CHANNEL_28_TO_31_PAGE_MASK(currentPage);
        status = startSearchForJoinableNetwork(newPageMask, allChannels);
        if (status == SL_STATUS_OK) {
          return;       // not 'break', we want to bail out immediately
        }
      }
#else
      sl_zigbee_af_core_println("%s %s", "Form", "and join scan done");
#endif
      if (state == NETWORK_FIND_JOIN
          && sl_zigbee_af_network_find_get_enable_scanning_all_channels_cb()) {
        printScanAllChannelsPrompt();
        state = NETWORK_FIND_NONE;        // have to reset state before calling the fn
        status = startSearchForJoinableNetworkAllChannels();
        if (status == SL_STATUS_OK) {
          return;       // not 'break', we want to bail out immediately
        }
      }
      break;
#ifdef SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK
    case SL_STATUS_NOT_FOUND:
      if ((state == NETWORK_FIND_FORM
  #ifdef SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT
           || state == NETWORK_FIND_PHY2
  #endif
           )
          && sl_zigbee_af_network_find_get_enable_scanning_all_channels_cb()) {
        printScanAllChannelsPrompt();
        if (state == NETWORK_FIND_FORM) {
          state = NETWORK_FIND_NONE;      // have to reset state before calling the fn
          status = startSearchForUnusedNetwork(true);
  #ifdef SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT
        } else {
          state = NETWORK_FIND_PHY2_ALL_CHANNELS;
          currentPage = SL_ZIGBEE_MIN_SUGBHZ_PAGE_NUMBER;
          status = sl_zigbee_scan_for_unused_pan_id(CHANNEL_28_TO_31_PAGE_ALL_CHANNELS_MASK(currentPage),
                                                    SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_DURATION);
  #endif
        }
        if (status == SL_STATUS_OK) {
          return;       // not 'break', we want to bail out immediately
        }
      } else {
        // Translate the error status for backwards compatibility...
        status = SL_STATUS_FAIL;
      }
      // ...and deliberately fall through
      SL_FALLTHROUGH
#endif
    // default case to print out error
    default:
      sl_zigbee_af_core_println("%s error 0x%02X", "Scan", status);
  }
  sl_zigbee_af_core_flush();
  state = NETWORK_FIND_NONE;
  sl_zigbee_af_app_println("%s (scan error) 0x%0x",
                           "Network find complete",
                           status);
  sl_zigbee_af_network_find_finished_cb(status);
}

static bool findInitialChannelMask(bool allChannels,
                                   uint32_t *initialChannelMask)
{
#ifdef SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT
  if (searchMode & FIND_AND_JOIN_MODE_ALLOW_2_4_GHZ) {
    currentPage = 0;
    // Continue in the common part after the if statement
  } else if (searchMode & FIND_AND_JOIN_MODE_ALLOW_SUB_GHZ) {
    currentPage = SL_ZIGBEE_MIN_SUGBHZ_PAGE_NUMBER;
    *initialChannelMask = allChannels
                          ? CHANNEL_28_TO_31_PAGE_ALL_CHANNELS_MASK(currentPage)
                          : CHANNEL_28_TO_31_PAGE_MASK(currentPage);
    return true;
  } else {
    sl_zigbee_af_app_print("Invalid search mode");
    return false;
  }
#endif

  // A common part for a 2.4 GHz only mode or no sub-GHz support at all
  *initialChannelMask = allChannels
                        ? SL_ZIGBEE_ALL_802_15_4_CHANNELS_MASK
                        : page0mask;
  return true;
}

#ifdef SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK
static sl_status_t startSearchForUnusedNetwork(bool allChannels)
{
  sl_status_t status;
  uint32_t initialChannelMask;

  if (state != NETWORK_FIND_NONE) {
    sl_zigbee_af_app_println("%sForm and join ongoing, state = %d", "Error: ", state);
    return SL_STATUS_INVALID_STATE;
  }

  if (!findInitialChannelMask(allChannels, &initialChannelMask)) {
    return SL_STATUS_INVALID_STATE;
  }

  sl_zigbee_af_app_println("Search for %s network", "unused");

  status = sl_zigbee_scan_for_unused_pan_id(initialChannelMask,
                                            SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_DURATION);
  if (status == SL_STATUS_OK) {
    state = allChannels
            ? NETWORK_FIND_FORM_ALL_CHANNELS
            : NETWORK_FIND_FORM;
  }
  return status;
}
#endif

sl_status_t sl_zigbee_af_find_unused_pan_id_and_form_cb(void)
{
#ifdef SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK
  return startSearchForUnusedNetwork(false);
#else
  return SL_STATUS_INVALID_STATE;
#endif
}

static sl_status_t startSearchForJoinableNetwork(uint32_t channelMask,
                                                 bool allChannels)
{
  sl_status_t status;
  uint8_t extendedPanId[EXTENDED_PAN_ID_SIZE];

  if (state != NETWORK_FIND_NONE) {
    sl_zigbee_af_app_println("%sForm and join ongoing", "Error: ");
    return SL_STATUS_INVALID_STATE;
  }

  // Set radio power before active scan to configure beacon request
  // Tx power level based on the plugin setting
  // Upper bits of channel mask are used for sub gig page. If this is zero, the search
  // must use 2,4GHz plugin power setting. Else, we use the sub-gig plugin power setting
  #if defined(EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ)
  (void)sl_zigbee_set_radio_power(((channelMask & SL_ZIGBEE_ALL_CHANNEL_PAGE_MASK) == 0) ? SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_RADIO_TX_POWER : SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_SUB_GHZ_RADIO_TX_POWER);
  #else
  (void)sl_zigbee_set_radio_power(SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_RADIO_TX_POWER);
  #endif
  sl_zigbee_af_get_form_and_join_extended_pan_id_cb(extendedPanId);
  status = sl_zigbee_scan_for_joinable_network(channelMask, extendedPanId);
  if (status == SL_STATUS_OK) {
    state = allChannels
            ? NETWORK_FIND_JOIN_ALL_CHANNELS
            : NETWORK_FIND_JOIN;
  }
  return status;
}

void sl_zigbee_af_network_find_stack_status_cb(sl_status_t status)
{
  if (status == SL_STATUS_NETWORK_UP) {
    // If we had been searching for an unused network so that we could form,
    // we're done.  If we were searching for a joinable network and have
    // successfully joined, we give the application some time to determine if
    // this is the correct network.  If so, we'll eventually time out and clean
    // up the state machine.  If not, the application will tell the stack to
    // leave, we'll get an SL_STATUS_NETWORK_DOWN, and we'll continue searching.
    switch (state) {
#ifdef SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK
      case NETWORK_FIND_FORM:
      case NETWORK_FIND_FORM_ALL_CHANNELS:
        sl_zigbee_af_network_find_tick_event_handler(networkFindTickEventControl);
        break;
#endif
      case NETWORK_FIND_JOIN:
      case NETWORK_FIND_JOIN_ALL_CHANNELS:
        state = JOIN_STATE_TO_WAIT_STATE(state);
        sl_zigbee_af_event_set_delay_minutes(networkFindTickEventControl,
                                             SL_ZIGBEE_AF_PLUGIN_NETWORK_FIND_JOINABLE_SCAN_TIMEOUT_MINUTES);
        break;
      default:
        // MISRA requires to have a default case
        break;
    }
  } else {
    // If we get something other than SL_STATUS_NETWORK_UP while trying to join or
    // while waiting for the application to determine if this is the right
    // network, we need to continue searching for a joinable network.
    switch (state) {
      case NETWORK_FIND_WAIT:
      case NETWORK_FIND_WAIT_ALL_CHANNELS:
        state = WAIT_STATE_TO_JOIN_STATE(state);
        // Deliberate fall-through.
        SL_FALLTHROUGH
      case NETWORK_FIND_JOIN:
      case NETWORK_FIND_JOIN_ALL_CHANNELS:
        sl_zigbee_af_network_find_tick_event_handler(networkFindTickEventControl);
        break;
      default:
        // MISRA requires a default case
        break;
    }
  }
}

void sl_zigbee_af_get_form_and_join_extended_pan_id_cb(uint8_t *resultLocation)
{
  uint8_t networkIndex = sl_zigbee_get_current_network();
  memmove(resultLocation, extendedPanIds[networkIndex], EXTENDED_PAN_ID_SIZE);
}

void sl_zigbee_af_set_form_and_join_extended_pan_id_cb(const uint8_t *extendedPanId)
{
  uint8_t networkIndex = sl_zigbee_get_current_network();
  memmove(extendedPanIds[networkIndex], extendedPanId, EXTENDED_PAN_ID_SIZE);
}

// Code to compliantly search for all channels once we've searched on the
// preferred channels.
// It is assumed that appropriate channel masks per page are set prior to calling.
static sl_status_t startSearchForJoinableNetworkCallbackCommon(bool allChannels)
{
  uint32_t initialChannelMask;

  if (!findInitialChannelMask(allChannels,
                              &initialChannelMask)) {
    return SL_STATUS_INVALID_STATE;
  }

  sl_zigbee_af_app_println("Search for %s network", "joinable");

  return startSearchForJoinableNetwork(initialChannelMask, allChannels);
}

sl_status_t sl_zigbee_af_start_search_for_joinable_network_cb(void)
{
  return startSearchForJoinableNetworkCallbackCommon(false);
}

static sl_status_t startSearchForJoinableNetworkAllChannels(void)
{
  return startSearchForJoinableNetworkCallbackCommon(true);
}

sl_status_t sl_zigbee_af_set_form_and_join_channel_mask(uint8_t page, uint32_t mask)
{
  switch (page) {
    case 0:
      // Make sure the bitmask only contains the permitted bits
      if ((mask & ~SL_ZIGBEE_ALL_802_15_4_CHANNELS_MASK) != 0) {
        return SL_STATUS_INVALID_CHANNEL_MASK;
      }
      page0mask = mask;
      break;
#ifdef SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT
    case 28:
    case 30:
    case 31:
      if ((mask & ~SL_ZIGBEE_ALL_SUBGHZ_CHANNELS_MASK_FOR_PAGES_28_30_31) != 0) {
        return SL_STATUS_INVALID_CHANNEL_MASK;
      }
      page28to31mask[page - SL_ZIGBEE_MIN_SUGBHZ_PAGE_NUMBER] = mask;
      break;
    case 29:
      if ((mask & ~SL_ZIGBEE_ALL_SUBGHZ_CHANNELS_MASK_FOR_PAGES_29) != 0) {
        return SL_STATUS_INVALID_CHANNEL_MASK;
      }
      page28to31mask[page - SL_ZIGBEE_MIN_SUGBHZ_PAGE_NUMBER] = mask;
      break;
#endif
    default:
      // It would be nice to have an "invalid page" error, but this will have to do:
      return SL_STATUS_TRANSMIT_INVALID_CHANNEL;
  }

  return SL_STATUS_OK;
}

uint32_t sl_zigbee_af_form_and_join_get_channel_mask(uint8_t page, bool allChannels)
{
  switch (page) {
    case 0:
      return allChannels ? SL_ZIGBEE_ALL_802_15_4_CHANNELS_MASK : page0mask;
      break;
#ifdef SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT
    case 28:
    case 29:
    case 30:
    case 31:
      return allChannels ? page28to31maskAllChannels[page - SL_ZIGBEE_MIN_SUGBHZ_PAGE_NUMBER] : page28to31mask[page - SL_ZIGBEE_MIN_SUGBHZ_PAGE_NUMBER];
      break;
#endif
    default:
      return 0xFFFFFFFFU;
  }
}

sl_status_t sl_zigbee_af_set_form_and_join_search_mode(uint8_t mode)
{
  switch (mode) {
    case FIND_AND_JOIN_MODE_ALLOW_2_4_GHZ:
#ifdef SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT
    case FIND_AND_JOIN_MODE_ALLOW_SUB_GHZ:
    case FIND_AND_JOIN_MODE_ALLOW_BOTH:
      searchMode = mode;
#endif
      break;
    default:
      return SL_STATUS_INVALID_CHANNEL_MASK;
  }

  return SL_STATUS_OK;
}

uint8_t sl_zigbee_af_get_form_and_join_search_mode(void)
{
#ifdef SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT
  return searchMode;
#else
  return FIND_AND_JOIN_MODE_ALLOW_2_4_GHZ;
#endif
}

/** @brief Is the search for an unused network currently in progress scanning
 *         all channels?
 * @return true if yes, false if the current scan is on preferred channels only
 */
bool sli_zigbee_af_is_current_search_for_unused_network_scanning_all_channels(void)
{
#ifdef SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK
  #ifdef SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT
  return state == NETWORK_FIND_FORM_ALL_CHANNELS
         || state == NETWORK_FIND_PHY2_ALL_CHANNELS;
  #else
  return state == NETWORK_FIND_FORM_ALL_CHANNELS;
  #endif
#else
  return false;
#endif
}

/** @brief Returns the channel mask for the current scan.
 * This function  may return the configured
 * channel mask or all channels mask, depending on the current scan state.
 */
uint32_t sli_zigbee_af_get_unused_network_channel_mask(uint8_t page)
{
  bool allChannels = false;
  if (sli_zigbee_af_is_current_search_for_unused_network_scanning_all_channels()) {
    allChannels = true;
  }

  return sl_zigbee_af_form_and_join_get_channel_mask(page, allChannels);
}

#ifdef SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT
/** @brief Secondary interface formed callback.
 * Called after forming the network to notify the plugin that the secondary
 * interface (in case of a dual-PHY implementation) has been formed. It is used
 * by the plugin to reset its internal state. Strictly for internal use only.
 */
void sli_zigbee_af_secondary_interface_formed_callback(sl_status_t status)
{
  (void) status;        // unreferenced parameter
  #ifdef SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK
  if (state == NETWORK_FIND_PHY2 || state == NETWORK_FIND_PHY2_ALL_CHANNELS) {
    // It should be enough to reset the find status, but we may still have
    // the timer running so let's delegate the job to the tick handler which
    // will also take care of killing the timer.
    sl_zigbee_af_network_find_tick_event_handler(networkFindTickEventControl);
  }
  #endif
}
#endif // SL_CATALOG_ZIGBEE_NETWORK_FIND_SUB_GHZ_PRESENT
