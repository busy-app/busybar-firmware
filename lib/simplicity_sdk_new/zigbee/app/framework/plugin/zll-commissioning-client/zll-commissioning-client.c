/***************************************************************************//**
 * @file
 * @brief Routines for the ZLL Commissioning Client plugin, which implements the
 *        client side of the ZLL protocol.
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
#include  "mac-phy.h"

#include "sl_signature_decode.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "zll-commissioning.h"
#include "zll-commissioning-client-config.h"
#ifdef SL_CATALOG_ZIGBEE_NETWORK_CREATOR_PRESENT
#include "network-creator.h"
#define NETWORK_CREATOR_PRESENT
#endif
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
#include "test-harness-z3-zll.h"
#define TEST_HARNESS_Z3_PRESENT
#endif

// The code below assumes that there is exactly one network and that it is
// ZigBee PRO.
#if SL_ZIGBEE_SUPPORTED_NETWORKS != 1
  #error ZLL is not supported with multiple networks.
#endif

//------------------------------------------------------------------------------
// Globals

#define ZLL_DEVICE_INFORMATION_RECORD_SIZE 16

#define isRequestingPriority(state) ((state) & SL_ZIGBEE_ZLL_STATE_LINK_PRIORITY_REQUEST)

static bool radioOnForRequestOnly = false;

static sl_zigbee_af_event_t touchLinkEvent;

static uint8_t currentChannel;
static int8_t rssi;

uint16_t sli_zigbee_af_zll_identify_duration_sec = SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT_IDENTIFY_DURATION;

#ifdef PLUGIN_DEBUG
static const uint8_t sli_zigbee_af_zll_commissioning_plugin_name[] = "ZLL Commissioning Client";
#define PLUGIN_NAME sli_zigbee_af_zll_commissioning_plugin_name
#endif

static bool scanningPrimaryChannels = false;

// Module state
static enum {
  CLIENT_INITIAL,
  CLIENT_SCANNING,
  CLIENT_REQUESTING_INFO,
  CLIENT_AWAITING_RESPONSE,
  CLIENT_REQUESTING_IDENTITY,
  CLIENT_TOUCHLINKING,
} moduleState = CLIENT_INITIAL;

static uint8_t nextSubDeviceIndex = 0;

#ifdef SL_ZIGBEE_TEST
// Limit this delay for scripted tests, to prevent the transaction timing out,
// especially for a host side test.
  #define SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_TOUCH_LINK_MILLISECONDS_DELAY 2500
#else
  #define SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_TOUCH_LINK_MILLISECONDS_DELAY \
  (SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT_TOUCH_LINK_SECONDS_DELAY * MILLISECOND_TICKS_PER_SECOND)
#endif

#define JOIN_RESPONSE_TIMEOUT_QS 20

// Parameters for Network Creator callbacks - note, for a client, we use the
// radio power defined for this plugin, not the power defined for the Network
// Creator plugin, in order that both client and server use the same power.
static sl_802154_pan_id_t networkCreatorPanId = 0xFFFF;
static int8_t networkCreatorRadioPower = SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_RADIO_TX_POWER;

// Note, currently we set a random extended PAN ID for both client and server created
// networks during standard touchlink commissioning (even if the client wants otherwise.)

// Private ZLL commissioning functions
bool sli_zigbee_af_zll_am_factory_new(void);
sl_zigbee_node_type_t sli_zigbee_af_zll_get_logical_node_type(void);
void sli_zigbee_af_zll_touch_link_complete(void);

// Forward references
void sli_zigbee_af_zll_abort_touch_link(sl_zigbee_af_zll_commissioning_status_t reason);

//------------------------------------------------------------------------------
// Module private functions

static uint32_t getChannelMask(void)
{
  uint32_t channelMask = 0;
  if (scanningPrimaryChannels) {
    channelMask = sl_zigbee_get_zll_primary_channel_mask();
#ifdef SCAN_SECONDARY_CHANNELS
  } else {
    channelMask = sl_zigbee_get_zll_secondary_channel_mask();
#endif
  }
  debugPrintln("getChannelMask: mask = %08X, scanningPrimary = %d", channelMask, scanningPrimaryChannels);
  return channelMask;
}

static sl_status_t sendDeviceInformationRequest(uint8_t startIndex)
{
  sl_status_t status;
  (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                            | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                            | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                           ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                                           ZCL_DEVICE_INFORMATION_REQUEST_COMMAND_ID,
                                           "wu",
                                           sli_zigbee_af_zll_network.securityAlgorithm.transactionId,
                                           startIndex);
  status = sl_zigbee_af_send_command_inter_pan(0xFFFF,                // destination pan id
                                               sli_zigbee_af_zll_network.eui64,
                                               SL_ZIGBEE_NULL_NODE_ID, // node id - ignored
                                               0x0000,       // group id - ignored
                                               SL_ZIGBEE_ZLL_PROFILE_ID);
  if (status == SL_STATUS_OK) {
    moduleState = CLIENT_AWAITING_RESPONSE;
    sl_zigbee_af_event_set_delay_ms(&touchLinkEvent,
                                    SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_TOUCH_LINK_MILLISECONDS_DELAY);
  } else {
    sl_zigbee_af_app_println("%s%s failed 0x%02X",
                             "Error: ",
                             "Device information request",
                             status);
  }
  return status;
}

static sl_status_t sendIdentifyRequest(uint16_t identifyDuration)
{
  sl_status_t status;
  (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                            | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                            | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                           ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                                           ZCL_IDENTIFY_REQUEST_COMMAND_ID,
                                           "wv",
                                           sli_zigbee_af_zll_network.securityAlgorithm.transactionId,
                                           identifyDuration);
  status = sl_zigbee_af_send_command_inter_pan(0xFFFF,                // destination pan id
                                               sli_zigbee_af_zll_network.eui64,
                                               SL_ZIGBEE_NULL_NODE_ID, // node id - ignored
                                               0x0000,       // group id - ignored
                                               SL_ZIGBEE_ZLL_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_app_println("%s%s failed 0x%02X",
                             "Error: ",
                             "Identify request",
                             status);
  }
  return status;
}

static sl_status_t sendResetToFactoryNewRequest(void)
{
  sl_status_t status;
  (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                            | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                            | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                           ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                                           ZCL_RESET_TO_FACTORY_NEW_REQUEST_COMMAND_ID,
                                           "w",
                                           sli_zigbee_af_zll_network.securityAlgorithm.transactionId);
  status = sl_zigbee_af_send_command_inter_pan(0xFFFF,                // destination pan id
                                               sli_zigbee_af_zll_network.eui64,
                                               SL_ZIGBEE_NULL_NODE_ID, // node id - ignored
                                               0x0000,       // group id - ignored
                                               SL_ZIGBEE_ZLL_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_app_println("%s%s failed 0x%02X",
                             "Error: ",
                             "Reset to factory new request",
                             status);
  }
  return status;
}

static const sl_signature_t sl_zll_device_info_record_signature = {
  6,
  SL_SIGNATURE_FIELD_MASK_BLOB | EUI64_SIZE, offsetof(sl_zigbee_zll_device_info_record_t, ieeeAddress),
  SL_SIGNATURE_FIELD_1_BYTE, offsetof(sl_zigbee_zll_device_info_record_t, endpointId),
  SL_SIGNATURE_FIELD_2_BYTES, offsetof(sl_zigbee_zll_device_info_record_t, profileId),
  SL_SIGNATURE_FIELD_2_BYTES, offsetof(sl_zigbee_zll_device_info_record_t, deviceId),
  SL_SIGNATURE_FIELD_1_BYTE, offsetof(sl_zigbee_zll_device_info_record_t, version),
  SL_SIGNATURE_FIELD_1_BYTE, offsetof(sl_zigbee_zll_device_info_record_t, groupIdCount),
};
static void deviceInformationResponseHandler(const sl_802154_long_addr_t source,
                                             uint32_t transaction,
                                             uint8_t numberOfSubDevices,
                                             uint8_t startIndex,
                                             uint8_t deviceInformationRecordCount,
                                             uint8_t *deviceInformationRecordList)
{
  sl_zigbee_af_event_set_inactive(&touchLinkEvent);

  uint16_t deviceInformationRecordListLen = (deviceInformationRecordCount
                                             * ZLL_DEVICE_INFORMATION_RECORD_SIZE);
  uint8_t i;
  bool validResponse = ((sli_zigbee_af_zll_network.securityAlgorithm.transactionId == transaction)
                        && memcmp(sli_zigbee_af_zll_network.eui64, source, EUI64_SIZE) == 0);

  sl_zigbee_af_zll_commissioning_cluster_flush();
  sl_zigbee_af_zll_commissioning_cluster_print("RX: DeviceInformationResponse 0x%08X, 0x%02X, 0x%02X, 0x%02X,",
                                               transaction,
                                               numberOfSubDevices,
                                               startIndex,
                                               deviceInformationRecordCount);
  sl_zigbee_af_zll_commissioning_cluster_flush();
  for (i = 0; i < deviceInformationRecordCount; i++) {
    sl_zigbee_zll_device_info_record_t tempDeviceInfoRecord;
    uint8_t sort;
    sl_signature_decode(deviceInformationRecordList,
                        deviceInformationRecordListLen,
                        i * (8 + EUI64_SIZE),
                        sl_zll_device_info_record_signature,
                        (uint8_t*)&tempDeviceInfoRecord);
    sort = sl_zigbee_af_get_int8u(deviceInformationRecordList, (i + 1) * (8 + EUI64_SIZE) - 1, deviceInformationRecordListLen);
    sl_zigbee_af_zll_commissioning_cluster_print(" [");
    sl_zigbee_af_zll_commissioning_cluster_debug_exec(sl_zigbee_af_print_big_endian_eui64(tempDeviceInfoRecord.ieeeAddress));
    sl_zigbee_af_zll_commissioning_cluster_print(" 0x%02X 0x%04X 0x%04X 0x%02X 0x%02X 0x%02X",
                                                 tempDeviceInfoRecord.endpointId,
                                                 tempDeviceInfoRecord.profileId,
                                                 tempDeviceInfoRecord.deviceId,
                                                 tempDeviceInfoRecord.version,
                                                 tempDeviceInfoRecord.groupIdCount,
                                                 sort);
    sl_zigbee_af_zll_commissioning_cluster_flush();

    if (validResponse
        && (sli_zigbee_af_zll_sub_device_count
            < SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT_SUB_DEVICE_TABLE_SIZE)) {
      memcpy(&(sli_zigbee_af_zll_sub_devices[sli_zigbee_af_zll_sub_device_count]), &tempDeviceInfoRecord, sizeof(sl_zigbee_zll_device_info_record_t));
      sli_zigbee_af_zll_sub_device_count++;
    } else {
      sl_zigbee_af_zll_commissioning_cluster_print(" (ignored)");
    }
    sl_zigbee_af_zll_commissioning_cluster_print("]");
    sl_zigbee_af_zll_commissioning_cluster_flush();
  }
  sl_zigbee_af_zll_commissioning_cluster_println("");

  if (validResponse
      && (sli_zigbee_af_zll_sub_device_count
          < SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT_SUB_DEVICE_TABLE_SIZE)
      && sli_zigbee_af_zll_sub_device_count < numberOfSubDevices) {
    nextSubDeviceIndex = startIndex + deviceInformationRecordCount;
    moduleState = CLIENT_REQUESTING_INFO;
  } else {
    moduleState = CLIENT_REQUESTING_IDENTITY;

    if (radioOnForRequestOnly) {
      sl_zigbee_zll_set_radio_idle_mode(SL_ZIGBEE_RADIO_POWER_MODE_OFF);
      radioOnForRequestOnly = false;
    }
  }
  sl_zigbee_af_event_set_active(&touchLinkEvent);
}

static bool amInitiator(void)
{
  sl_zigbee_tok_type_stack_zll_data_t token;
  sl_zigbee_zll_get_token_stack_zll_data(&token);
  return token.bitmask & SL_ZIGBEE_ZLL_STATE_ADDRESS_ASSIGNMENT_CAPABLE;
}

static sl_status_t startScan(uint8_t purpose)
{
  sl_status_t status = SL_STATUS_INVALID_STATE;
  if (touchLinkInProgress()) {
    sl_zigbee_af_app_println("%sTouch linking in progress", "Error: ");
  } else if (!amInitiator()) {
    sl_zigbee_af_app_println("%sDevice is not an initiator", "Error: ");
  } else {
    sl_zigbee_af_zll_set_initial_security_state();

    // Use the Ember API, not the framework API, otherwise host-side returns zero if network down
    currentChannel = sl_zigbee_get_radio_channel();
    debugPrintln("startScan: purpose = %02X, flags = %04X, current channel = %d", purpose, sli_zigbee_af_zll_flags, currentChannel);

    scanningPrimaryChannels = true;
    uint32_t channelMask = getChannelMask();
    if (channelMask == 0) {
      // If there are no primary channels, then get the secondary mask.
      scanningPrimaryChannels = false;
      channelMask = getChannelMask();
    }

    if (channelMask != 0) {
      status = sl_zigbee_zll_start_scan(channelMask,
                                        SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT_SCAN_POWER_LEVEL,
                                        sli_zigbee_af_zll_get_logical_node_type());
      if (status == SL_STATUS_OK) {
        sli_zigbee_af_zll_flags = purpose;
        moduleState = CLIENT_SCANNING;
      } else {
        sl_zigbee_af_app_println("%s%s%s0x%02X",
                                 "Error: ",
                                 "Touch linking failed: ",
                                 "could not start scan: ",
                                 status);
      }
    } else {
      sl_zigbee_af_app_println("%s%s0x%02X",
                               "Error: ",
                               "Touch linking failed: ",
                               "no scan channels configured");
    }
  }
  return status;
}

static bool isSameNetwork(const sl_zigbee_zll_network_t *network)
{
  sl_zigbee_node_type_t nodeType;
  sl_zigbee_network_parameters_t parameters;
  sl_status_t status = sl_zigbee_af_get_network_parameters(&nodeType, &parameters);

  // Note, we only check the Extended Pan Id, not the Short Pan Id, nor the
  // Network Update Id.
  return (status == SL_STATUS_OK
          && (memcmp(parameters.extendedPanId,
                     network->zigbeeNetwork.extendedPanId,
                     EXTENDED_PAN_ID_SIZE) == 0));
}

static bool isTouchlinkPermitted(const sl_zigbee_zll_network_t *networkInfo)
{
  // Basic checks for a Z3.0 touchlink initiator.
  if (networkInfo->nodeType == SL_ZIGBEE_UNKNOWN_DEVICE) {
    // The target must be a known device type.
    debugPrintln("isTouchlinkPermitted: Error - Target is unknown node type %d", networkInfo->nodeType);
    return false;
  }

  // Get current run-time network status. (Note that getting the node type may
  // report an actual node type even if a previous touchlink failed, e.g.
  // if the initiator was orphaned and was unable to rejoin)
  sl_zigbee_network_status_t networkStatus = sl_zigbee_af_network_state();
  sl_zigbee_current_security_state_t securityState;
  bool networkUp;

  if (networkStatus == SL_ZIGBEE_JOINED_NETWORK
      || networkStatus == SL_ZIGBEE_JOINED_NETWORK_NO_PARENT) {
    networkUp = true;
    if (sl_zigbee_get_current_security_state(&securityState) != SL_STATUS_OK) {
      // It is possible that we're orphaned but attempting to rejoin here, but we'll
      // fail the touchlink anyway, since it would probably collide with the rejoining
      // procedure. (sl_zigbee_get_current_security_state() fails if we are rejoining even
      // though sl_zigbee_af_network_state() reports SL_ZIGBEE_JOINED_NETWORK_NO_PARENT)
      debugPrintln("sl_zigbee_get_current_security_state failure");
      return false;
    } else {
      // We're either joined or orphaned and not attempting to rejoin; in
      // either case we can proceed with the next stage of validation.
      debugPrintln("isTouchlinkPermitted: security bitmask = %04X,flags = %04X",
                   securityState.bitmask, sli_zigbee_af_zll_flags);
    }
  } else {
    networkUp = false;
    debugPrintln("isTouchlinkPermitted: initiator is not joined");
  }

  if (scanForReset() || scanForTouchLink()) {
    // When scanning for touch linking or reset, the initiator need not be
    // on a network, but if it is, then it may not be on a different centralized
    // network, unless the (non-compliant) centralized network support is enabled.
    if (networkUp
        && (securityState.bitmask & SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE) == 0
        && !isSameNetwork(networkInfo)) {
      debugPrintln("isTouchlinkPermitted: Error - Initiator on centralized network");
      return false;
    }
  }

  if (scanForTouchLink() && !networkUp) {
    // When scanning for touch linking, and if the initiator is not on a network,
    // either initiator or target must be able to form a network. If centralized
    // network touchlink initiating is supported, the initiating device must
    // be coordinator-capable.
    if (sli_zigbee_af_zll_get_logical_node_type() != SL_ZIGBEE_ROUTER && networkInfo->nodeType != SL_ZIGBEE_ROUTER) {
      debugPrintln("isTouchlinkPermitted: Error - Neither end can form a network");
      debugPrintln("our type = %d, their type = %d", sli_zigbee_af_zll_get_logical_node_type(), networkInfo->nodeType);
      return false;
    }
  }
  return true;
}

// Returns an integer greater than, equal to, or less than zero, according to
// whether target t1 is better than, equal to, or worse than target t2 in terms
// of requested priority with corrected signal strength serving as tiebreaker.
static int8_t targetCompare(const sl_zigbee_zll_network_t *t1,
                            int8_t r1,
                            const sl_zigbee_zll_network_t *t2,
                            int8_t r2)
{
  // When considering two targets, if only one has requested priority, that one
  // is chosen.
  if (isRequestingPriority(t1->state)
      && !isRequestingPriority(t2->state)) {
    return 1;
  } else if (!isRequestingPriority(t1->state)
             && isRequestingPriority(t2->state)) {
    return -1;
  }

  // If the priority of both targets is the same, the correct signal strength
  // is considered.  The strongest corrected signal wins.
  sl_zigbee_af_debug_println("Signal strengths: r1 = %d, t1->corr = %d, r2 = %d, t2->corr = %d",
                             r1, t1->rssiCorrection, r2, t2->rssiCorrection);
  if (r1 + t1->rssiCorrection < r2 + t2->rssiCorrection) {
    return -1;
  } else if (r2 + t2->rssiCorrection < r1 + t1->rssiCorrection) {
    return 1;
  }

  // If we got here, both targets are considered equal.
  return 0;
}

static void processScanComplete(sl_status_t scanStatus)
{
  sl_status_t status;
  debugPrintln("processScanComplete - status = %02X", scanStatus);

  if (scanStatus != SL_STATUS_OK) {
    sl_zigbee_af_app_println("%s%s0x%02X",
                             "Error: ",
                             "Touch linking failed due to preemption: ",
                             scanStatus);

    // Factory new initiator preempted by an NFN initiator.
    sli_zigbee_af_zll_abort_touch_link(SL_ZIGBEE_AF_ZLL_PREEMPTED_BY_STACK);

    if (sl_zigbee_zll_get_radio_idle_mode() == SL_ZIGBEE_RADIO_POWER_MODE_OFF) {
      // Start a short rx-on period to cover an incoming network request.
      sl_zigbee_zll_set_rx_on_when_idle(JOIN_RESPONSE_TIMEOUT_QS * 250 * 2);
    }
  } else if (abortingTouchLink()) {
    debugPrintln("processScanComplete: aborted by application");
    sli_zigbee_af_zll_abort_touch_link(SL_ZIGBEE_AF_ZLL_ABORTED_BY_APPLICATION);
  }

  if (touchLinkInProgress()) {
#ifdef SCAN_SECONDARY_CHANNELS
    if (scanningPrimaryChannels) {
      scanningPrimaryChannels = false;
      uint32_t channelMask = getChannelMask();
      if (channelMask != 0) {
        status = sl_zigbee_zll_start_scan(channelMask,
                                          SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT_SCAN_POWER_LEVEL,
                                          sli_zigbee_af_zll_get_logical_node_type());
        if (status == SL_STATUS_OK) {
          // We will continue on the second scan complete callback.
          return;
        }
        sl_zigbee_af_app_println("%s%s0x%02X",
                                 "Error: ",
                                 "could not start secondary channel scan: ",
                                 status);
      }
      debugPrintln("processScanComplete: no secondary channels, dropping thru");
    }
#endif // SCAN_SECONDARY_CHANNELS
    sli_zigbee_af_zll_flags |= SCAN_COMPLETE;

    // Inform the API that scanning is complete for this transaction.
    sl_zigbee_zll_scanning_complete();

    if (targetNetworkFound()) {
      status = sl_zigbee_set_logical_and_radio_channel(sli_zigbee_af_zll_network.zigbeeNetwork.channel);
      if (status != SL_STATUS_OK) {
        sl_zigbee_af_app_println("%s%s%s0x%02X",
                                 "Error: ",
                                 "Touch linking failed: ",
                                 "could not change channel: ",
                                 status);
        sli_zigbee_af_zll_abort_touch_link(SL_ZIGBEE_AF_ZLL_CHANNEL_CHANGE_FAILED);
        return;
      }

      // When scanning for the purposes of touch linking or requesting device
      // information and the target has more than one sub device, turn the
      // receiver on (so we can actually receive the response) and send out the
      // first request.  If the target only has one sub device, its data will
      // have already been received in the ScanRequest.
      if ((scanForTouchLink() || scanForDeviceInformation())
          && sli_zigbee_af_zll_network.numberSubDevices != 1) {
        debugPrintln("processScanComplete: sending first device info request");

        // Turn receiver on if it is currently off so we can receive
        // the Device Info Response.
        if (sl_zigbee_zll_get_radio_idle_mode() == SL_ZIGBEE_RADIO_POWER_MODE_OFF) {
          if (sli_zigbee_af_zll_am_factory_new()) {
            // If we're factory new, start a short rx-on period to cover the request.
            sl_zigbee_zll_set_rx_on_when_idle(JOIN_RESPONSE_TIMEOUT_QS * 250);
          } else {
            // We're joined as a sleepy end-device - rx-on may be active and
            // suspended, or expired, or never started in the first place.
            sl_zigbee_zll_set_radio_idle_mode(SL_ZIGBEE_RADIO_POWER_MODE_RX_ON);
            radioOnForRequestOnly = true;
          }
        }
        nextSubDeviceIndex = 0;
        moduleState = CLIENT_REQUESTING_INFO;
      } else {
        moduleState = CLIENT_REQUESTING_IDENTITY;
      }
      sl_zigbee_af_event_set_active(&touchLinkEvent);
    } else {
      sl_zigbee_af_app_println("%s%s%s",
                               "Error: ",
                               "Touch linking failed: ",
                               "no networks were found");
      sli_zigbee_af_zll_abort_touch_link(SL_ZIGBEE_AF_ZLL_NO_NETWORKS_FOUND);
    }
  } else {
    debugPrintln("processScanComplete: no touchlink in progress");
  }
}

//------------------------------------------------------------------------------
// ZLL commissioning private functions

void sli_zigbee_af_zll_abort_touch_link(sl_zigbee_af_zll_commissioning_status_t reason)
{
  sli_zigbee_af_zll_flags = INITIAL;
  moduleState = CLIENT_INITIAL;
  debugPrintln("sli_zigbee_af_zll_abort_touch_link: reason = %d", reason);
  if (sl_zigbee_af_event_is_scheduled(&touchLinkEvent)) {
    sl_zigbee_af_event_set_inactive(&touchLinkEvent);
    if (radioOnForRequestOnly) {
      sl_zigbee_zll_set_radio_idle_mode(SL_ZIGBEE_RADIO_POWER_MODE_OFF);
      radioOnForRequestOnly = false;
    }
    sendIdentifyRequest(0x0000); // exit identify mode
  }
  sl_status_t status;
  if (sl_zigbee_af_network_state() == SL_ZIGBEE_JOINED_NETWORK
      || sl_zigbee_af_network_state() == SL_ZIGBEE_JOINED_NETWORK_NO_PARENT) {
    status = sl_zigbee_set_radio_channel(currentChannel);
  } else {
    status = sl_zigbee_set_logical_and_radio_channel(currentChannel);
  }
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_app_println("%s0x%02X%s%d",
                             "Error: ",
                             status,
                             " - could not restore channel to: ",
                             currentChannel);
  }
  if (reason == SL_ZIGBEE_AF_ZLL_NETWORK_UPDATE_OPERATION) {
    sli_zigbee_af_zll_touch_link_complete();
  } else {
    sl_zigbee_af_zll_commissioning_touch_link_failed_cb(reason);
  }

  // Inform the API that scanning is complete for this transaction.
  sl_zigbee_zll_scanning_complete();
}

#ifdef NETWORK_CREATOR_PRESENT
void sli_zigbee_af_zll_finish_network_formation_for_router(sl_status_t status)
{
  if (status == SL_STATUS_NETWORK_UP) {
    // Make ourselves not factory new, and reset the ZLL address and group space
    // we are using node id 0x0001 ourselves.
    sl_zigbee_tok_type_stack_zll_data_t token;
    sl_zigbee_zll_get_token_stack_zll_data(&token);
    token.bitmask &= ~SL_ZIGBEE_ZLL_STATE_FACTORY_NEW;
    token.freeNodeIdMin = SL_ZIGBEE_ZLL_MIN_NODE_ID + 1;  // start with 0x0002
    token.freeNodeIdMax = SL_ZIGBEE_ZLL_MAX_NODE_ID;
    token.myGroupIdMin = (SL_ZIGBEE_ZLL_GROUP_ADDRESSES == 0
                          ? SL_ZIGBEE_ZLL_NULL_GROUP_ID
                          : SL_ZIGBEE_ZLL_MIN_GROUP_ID);
    token.freeGroupIdMin = SL_ZIGBEE_ZLL_MIN_GROUP_ID + SL_ZIGBEE_ZLL_GROUP_ADDRESSES;
    token.freeGroupIdMax = SL_ZIGBEE_ZLL_MAX_GROUP_ID;
    sl_zigbee_zll_set_token_stack_zll_data(&token);

    if (touchLinkInProgress()) {
      // Save the new channel.
      // (Use the Ember API, not the framework API, otherwise host-side returns zero if network down)
      currentChannel = sl_zigbee_get_radio_channel();

      // Switch back to the target's current channel to send the network request,
      // in case we formed on a different one.
      sl_zigbee_set_logical_and_radio_channel(sli_zigbee_af_zll_network.zigbeeNetwork.channel);

      // Set the target's new channel to our current channel.
      sli_zigbee_af_zll_network.zigbeeNetwork.channel = currentChannel;

      debugPrintln("Zll Comm stack status: our chan = %d, target chan = %d", currentChannel, sli_zigbee_af_zll_network.zigbeeNetwork.channel);

      // Kick the client touchlinking event.
#ifdef SL_ZIGBEE_TEST
      // Short delay to prevent beacon collision.
      sl_zigbee_af_event_set_delay_ms(&touchLinkEvent, 100);
#else
      sl_zigbee_af_event_set_active(&touchLinkEvent);
#endif // SL_ZIGBEE_TEST
    }
  } else if (touchLinkInProgress()) {
    sl_zigbee_af_app_println("%s%s%s0x%02X",
                             "Error: ",
                             "Touch linking failed: ",
                             "could not form network: ",
                             status);
    sli_zigbee_af_zll_abort_touch_link(SL_ZIGBEE_AF_ZLL_NETWORK_FORMATION_FAILED);
  }
  sli_zigbee_af_zll_flags &= ~FORMING_NETWORK;
}

extern uint32_t sli_zigbee_af_network_creator_primary_channel_mask;
extern uint32_t sli_zigbee_af_network_creator_secondary_channel_mask;
sl_status_t sli_zigbee_af_zll_form_network_for_router_initiator(uint8_t channel, int8_t radioPower, sl_802154_pan_id_t panId)
{
  sl_status_t status;

  // Initialize ZLL security.
  status = sl_zigbee_af_zll_set_initial_security_state();
  if (status != SL_STATUS_OK) {
    debugPrintln("sli_zigbee_af_zll_form_network_for_router_initiator: unable to initialize security, status = %02X", status);
    return status;
  }

  // Form network with a random extended PAN ID, if no channel is specified, otherwise
  // assume the caller has set the extended PAN ID. (The sli_zigbee_af_extended_pan_id value
  // is global to the framework and used in the network-creator plugin.)
  if (channel == 0) {
    memset(sli_zigbee_af_extended_pan_id, 0xFF, EXTENDED_PAN_ID_SIZE);
  }

  // Per the BDB spec, we scan on the ZLL primary channels, unless a channel
  // has been specified (e.g. for a 'plugin test-harness z3 start-as-router')
  sli_zigbee_af_network_creator_primary_channel_mask = channel ? (uint32_t) (1 << channel) : sl_zigbee_get_zll_primary_channel_mask();
  sli_zigbee_af_network_creator_secondary_channel_mask = 0;

  // Set node id for sl_zigbee_form_network, to override random id generation.
  sl_zigbee_set_node_id(SL_ZIGBEE_ZLL_MIN_NODE_ID);  // our node id = 0x0001

  // Set the radio transmit power and PAN id for the Network Creator callbacks.
  networkCreatorRadioPower = radioPower;
  networkCreatorPanId = panId;

  debugPrintln("sli_zigbee_af_zll_form_network_for_router_initiator: chan mask = %08X", sli_zigbee_af_network_creator_primary_channel_mask);

  status = sl_zigbee_af_network_creator_start(false); // distributed network

  if (status == SL_STATUS_OK) {
    sli_zigbee_af_zll_flags |= FORMING_NETWORK;
  }
  return status;
}
#endif // NETWORK_CREATOR_PRESENT

// This function is required to support the following CLI commands:
//   plugin test-harness z3 set-pan-id
//   plugin test-harness z3 start-as-router
void sli_zigbee_af_zll_set_pan_id(sl_802154_pan_id_t panId)
{
  networkCreatorPanId = panId;
}

bool sli_zigbee_af_zll_commissioning_client_interpan_pre_message_received_callback(const sl_zigbee_af_interpan_header_t *header,
                                                                                   uint8_t msgLen,
                                                                                   uint8_t *message)
{
  uint32_t transaction;
  uint8_t frameControl, commandId, msgIndex;

  // If the message isn't for the ZLL Commissioning cluster, drop it with an
  // indication that we didn't handle it.
  if (header->profileId != SL_ZIGBEE_ZLL_PROFILE_ID
      || header->clusterId != ZCL_ZLL_COMMISSIONING_CLUSTER_ID) {
    return false;
  }

  if (header->messageType != SL_ZIGBEE_AF_INTER_PAN_UNICAST
      || !(header->options & SL_ZIGBEE_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS)
      || msgLen < ZLL_HEADER_OVERHEAD) {
    return false;
  }

  // Verify that the frame control byte makes sense.  Accept only the legacy
  // format or simple client-to-server or server-to-client messages (i.e., no
  // manufacturer-specific commands, etc.)  For non-legacy messages, check that
  // the frame control is correct for the command.  The check is based on
  // DeviceInformationResponse because it is the only server-to-client command
  // we care about.
  frameControl = message[ZLL_HEADER_FRAME_CONTROL_OFFSET];
  commandId = message[ZLL_HEADER_COMMAND_ID_OFFSET];
  if (frameControl != ZLL_FRAME_CONTROL_LEGACY
      && frameControl != ZLL_FRAME_CONTROL_SERVER_TO_CLIENT) {
    return false;
  }

  msgIndex = ZLL_HEADER_TRANSACTION_ID_OFFSET;
  transaction = sl_zigbee_af_get_int32u(message, msgIndex, msgLen);
  msgIndex += 4;

  if (commandId == ZCL_DEVICE_INFORMATION_RESPONSE_COMMAND_ID) {
    if (msgIndex + 3 <= msgLen) {
      uint8_t numberOfSubDevices, startIndex, deviceInformationRecordCount;
      numberOfSubDevices = sl_zigbee_af_get_int8u(message, msgIndex, msgLen);
      msgIndex++;
      startIndex = sl_zigbee_af_get_int8u(message, msgIndex, msgLen);
      msgIndex++;
      deviceInformationRecordCount = sl_zigbee_af_get_int8u(message, msgIndex, msgLen);
      msgIndex++;
      if ((msgIndex
           + deviceInformationRecordCount * ZLL_DEVICE_INFORMATION_RECORD_SIZE)
          <= msgLen) {
        uint8_t *deviceInformationRecordList = message + msgIndex;
        deviceInformationResponseHandler(header->longAddress,
                                         transaction,
                                         numberOfSubDevices,
                                         startIndex,
                                         deviceInformationRecordCount,
                                         deviceInformationRecordList);
      }
    }
    return true;
  }

  // Return false for unhandled messages, since we are multiplexing the pre-message handler.
  return false;
}

//------------------------------------------------------------------------------
// Public functions

sl_status_t sl_zigbee_af_zll_initiate_touch_link(void)
{
  return startScan(SCAN_FOR_TOUCH_LINK);
}

sl_status_t sl_zigbee_af_zll_device_information_request(void)
{
  return startScan(SCAN_FOR_DEVICE_INFORMATION);
}

sl_status_t sl_zigbee_af_zll_identify_request(void)
{
  return startScan(SCAN_FOR_IDENTIFY);
}

sl_status_t sl_zigbee_af_zll_reset_to_factory_new_request(void)
{
  return startScan(SCAN_FOR_RESET);
}

void sl_zigbee_af_zll_abort_touch_link(void)
{
  if (touchLinkInProgress()) {
    // If the scanning portion of touch linking is already finished, we can
    // abort right away.  If not, we need to stop the scan and wait for the
    // stack to inform us when the scan is done.
    sl_zigbee_af_app_println("%s%s%s",
                             "Error: ",
                             "Touch linking failed: ",
                             "aborted by application");
    if (scanComplete()) {
      sli_zigbee_af_zll_abort_touch_link(SL_ZIGBEE_AF_ZLL_ABORTED_BY_APPLICATION);
    } else {
      sli_zigbee_af_zll_flags |= ABORTING_TOUCH_LINK;
      sl_zigbee_stop_scan();
    }
  }
}

void sli_zigbee_af_zll_network_found_callback(sl_zigbee_zll_network_t *networkInfo,
                                              bool isDeviceInfoNull,
                                              sl_zigbee_zll_device_info_record_t *deviceInfo,
                                              sl_zigbee_rx_packet_info_t *packetInfo)
{
  sl_zigbee_app_debug_println("ZllNwkFound: node type = %d, flags = %0x2X", networkInfo->nodeType, sli_zigbee_af_zll_flags);
  if (touchLinkInProgress()) {
    if (isTouchlinkPermitted(networkInfo)) {
      if (!targetNetworkFound()
          || 0 < targetCompare(networkInfo, packetInfo->last_hop_rssi, &sli_zigbee_af_zll_network, rssi)) {
        memmove(&sli_zigbee_af_zll_network, networkInfo, sizeof(sl_zigbee_zll_network_t));
        sli_zigbee_af_zll_sub_device_count = 0;
        if (!isDeviceInfoNull) {
          memmove(&sli_zigbee_af_zll_sub_devices[0], deviceInfo, sizeof(sl_zigbee_zll_device_info_record_t));
          sli_zigbee_af_zll_sub_device_count++;
        }
        rssi = packetInfo->last_hop_rssi;
        sli_zigbee_af_zll_flags |= TARGET_NETWORK_FOUND;
      }
    } else {
      sl_zigbee_app_debug_println("ezspZllNwkFound: touchlink is not permitted");
    }
  }
#ifdef TEST_HARNESS_Z3_PRESENT
  sli_zigbee_af_test_harness_z3_zll_network_found_callback(networkInfo);
#endif
}

void sli_zigbee_af_zll_scan_complete_callback(sl_status_t scanStatus)
{
  debugPrintln(" scan complete");
 #ifdef TEST_HARNESS_Z3_PRESENT
  // If this was a test scan, then let the test harness handle the scan complete.
  if (sli_zigbee_af_test_harness_z3_zll_scan_complete_callback(scanStatus)) {
    return;
  }
 #endif
  processScanComplete(scanStatus);
}

void sli_zigbee_af_zll_address_assignment_callback(sl_zigbee_zll_address_assignment_t *addressInfo,
                                                   sl_zigbee_rx_packet_info_t *packetInfo)
{
  UNUSED_VAR(packetInfo);
  if (touchLinkInProgress()) {
    debugPrintln("sl_zigbee_zll_address_assignment_handler: node id = %04X", addressInfo->nodeId);
    sli_zigbee_af_zll_network.nodeId = addressInfo->nodeId;
  }
}

void touchLinkEventHandler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_status_t status = SL_STATUS_OK;
  sl_zigbee_af_event_set_inactive(&touchLinkEvent);
  debugPrintln("touchlinkEventHandler: flags = %02X", sli_zigbee_af_zll_flags);

  if (!touchLinkInProgress()) {
    return;
  }

  switch (moduleState) {
    case CLIENT_REQUESTING_INFO:
      debugPrintln("EH: Sending device info request");
      status = sendDeviceInformationRequest(nextSubDeviceIndex);
      if (status != SL_STATUS_OK) {
        sl_zigbee_af_app_println("%s%s%s0x%02X",
                                 "Error: ",
                                 "Touch linking failed: ",
                                 "could not send device information request: ",
                                 status);
        sli_zigbee_af_zll_abort_touch_link(SL_ZIGBEE_AF_ZLL_SENDING_DEVICE_INFORMATION_REQUEST_FAILED);
      }
      break;

    case CLIENT_AWAITING_RESPONSE:
      // Timeout - give up.
      sl_zigbee_af_app_println("%s%s%s0x%02X",
                               "Error: ",
                               "Touch linking failed: ",
                               "no reply to device information request: ",
                               status);
      sli_zigbee_af_zll_abort_touch_link(SL_ZIGBEE_AF_ZLL_SENDING_DEVICE_INFORMATION_REQUEST_FAILED);
      break;

    case CLIENT_REQUESTING_IDENTITY:
      debugPrintln("EH - Sending identify request");
      status = sendIdentifyRequest(sli_zigbee_af_zll_identify_duration_sec);
      if (status == SL_STATUS_OK) {
        moduleState = CLIENT_TOUCHLINKING;
        sl_zigbee_af_event_set_delay_ms(&touchLinkEvent,
                                        SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_TOUCH_LINK_MILLISECONDS_DELAY);
      } else {
        sl_zigbee_af_app_println("%s%s%s0x%02X",
                                 "Error: ",
                                 "Touch linking failed: ",
                                 "could not send identify request: ",
                                 status);
        sli_zigbee_af_zll_abort_touch_link(SL_ZIGBEE_AF_ZLL_SENDING_IDENTIFY_REQUEST_FAILED);
      }
      break;

    case CLIENT_TOUCHLINKING:
      status = sendIdentifyRequest(0);
      if (scanForTouchLink()) {
        debugPrintln("EH - Proceeding with touchlink");
        // If we are not factory new, we want to bring the target into our
        // existing network, so we set the channel to our own channel, either
        // our original channel, or the channel we formed on, if we are (were)
        // a factory-new router initiator.
        // Otherwise, we'll use whatever channel the target is on presently.
        if (!sli_zigbee_af_zll_am_factory_new()) {
          sli_zigbee_af_zll_network.zigbeeNetwork.channel = currentChannel;
        }
#ifdef NETWORK_CREATOR_PRESENT
        else if (sli_zigbee_af_current_zigbee_pro_network != NULL
                 && sli_zigbee_af_current_zigbee_pro_network->nodeType <= SL_ZIGBEE_ROUTER) {
          // If we are factory new and a router, then we need to form a network
          // to become non-factory new. See the BDB spec's touchlink procedure
          // for an initiator, steps 13, 21 and 22.
          // Form on the target's channel when valid so the target does not need
          // to switch channels (avoids channel mismatch and join failures).
          uint8_t formChannel = sli_zigbee_af_zll_network.zigbeeNetwork.channel;
          if (formChannel < SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER
              || formChannel > SL_ZIGBEE_MAX_802_15_4_CHANNEL_NUMBER) {
            formChannel = 0;  // use primary channel mask
          }
          status = sli_zigbee_af_zll_form_network_for_router_initiator(formChannel,
                                                                       SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_RADIO_TX_POWER,
                                                                       0xffff);
          if (status != SL_STATUS_OK) {
            sl_zigbee_af_app_println("%s%s%s0x%02X",
                                     "Error: ",
                                     "Touch linking failed: ",
                                     "could not start network formation: ",
                                     status);
            sli_zigbee_af_zll_abort_touch_link(SL_ZIGBEE_AF_ZLL_NETWORK_FORMATION_FAILED);
          }
          return;
        }
#endif // NETWORK_CREATOR_PRESENT
        else {
          // If we are a factory new (end device) initiator, then let the
          // target chose the channel.
          sli_zigbee_af_zll_network.zigbeeNetwork.channel = 0;
        }
        sl_zigbee_af_zll_set_initial_security_state();
        sl_zigbee_af_app_println("%s%s%s",
                                 "Sending ",
                                 sli_zigbee_af_zll_am_factory_new() ? "start" : "join",
                                 " request to target");
        sli_zigbee_af_initiator_rejoin_retry_count = 0;
        status = sl_zigbee_zll_join_target(&sli_zigbee_af_zll_network);
        if (status == SL_STATUS_OK) {
          // We're finished here; the touchlink completion is handled by
          // the stack status handler in zll-commissioning-common.c
          moduleState = CLIENT_INITIAL;
        } else {
          // Check that it's not a non-fatal status, for example, if
          // sl_zigbee_zll_join_target() is sending a network update.
          if (status == SL_STATUS_IN_PROGRESS) {
            // Abort, but report touchlink complete to application.
            sli_zigbee_af_zll_abort_touch_link(SL_ZIGBEE_AF_ZLL_NETWORK_UPDATE_OPERATION);
          } else {
            sl_zigbee_af_app_println("%s%s%s0x%02X",
                                     "Error: ",
                                     "Touch linking failed: ",
                                     "could not send start/join: ",
                                     status);
            sli_zigbee_af_zll_abort_touch_link(SL_ZIGBEE_AF_ZLL_SENDING_START_JOIN_FAILED);
          }
        }
      } else {
        if (scanForReset()) {
          status = sendResetToFactoryNewRequest();
          if (status != SL_STATUS_OK) {
            sl_zigbee_af_app_println("%s%s%s0x%02X",
                                     "Error: ",
                                     "Touch linking failed: ",
                                     "could not send reset: ",
                                     status);
            sli_zigbee_af_zll_abort_touch_link(SL_ZIGBEE_AF_ZLL_SENDING_RESET_TO_FACTORY_NEW_REQUEST_FAILED);
          }
        }

        // For all non-link scan types.
        if (status == SL_STATUS_OK) {
          sl_zigbee_set_radio_channel(currentChannel);
          sli_zigbee_af_zll_touch_link_complete();
          moduleState = CLIENT_INITIAL;
        }
      }
      break;

    default:
      break;
  }
}

void sli_zigbee_af_zll_commissioning_client_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_event_init(&touchLinkEvent,
                          touchLinkEventHandler);
}

/** @brief Get Pan Id
 *
 * This callback is called when the Network Creator plugin needs the PAN ID for
 * the network it is about to create. By default, the callback will return a
 * random 16-bit value.
 *
 */
sl_802154_pan_id_t sl_zigbee_af_network_creator_get_pan_id_cb(void)
{
  return (networkCreatorPanId == 0xFFFF
          ? sl_zigbee_get_pseudo_random_number()
          : networkCreatorPanId);
}

/** @brief Get Power For Radio Channel
 *
 * This callback is called when the Network Creator plugin needs the radio power for
 * the network it is about to create. By default, the callback will use the radio
 * power specified in the relevant plugin option.
 *
 */
int8_t sl_zigbee_af_network_creator_get_radio_power_cb(void)
{
  return networkCreatorRadioPower;
}
