/***************************************************************************//**
 * @file
 * @brief ZigBee 3.0 touchlink test harness functionality
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

#include "test-harness-z3-core.h"
#include "test-harness-z3-zll.h"
#include "test-harness-z3-mac.h"

#include "app/framework/plugin/zll-commissioning-common/zll-commissioning.h"
#include "app/framework/plugin/interpan/interpan.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_NETWORK_CREATOR_PRESENT
#include "network-creator.h"
#endif

#include "app/framework/util/common.h"

// -----------------------------------------------------------------------------
// Types

typedef struct {
  uint32_t transactionId;
  sl_zigbee_af_profile_id_t profileId;
  uint16_t keyBitmask;
  bool setReservedInformationBits;
  uint8_t networkUpdateId;
  uint8_t numberSubDevices;
  uint8_t totalGroupIds;
  sl_802154_long_addr_t destinationEui64;
} sli_zigbee_scan_response_data_t;

// -----------------------------------------------------------------------------
// Globals
extern sl_status_t sl_zigbee_set_nwk_update_id(uint8_t nwkUpdateId, bool set_when_on_network);
// We need this extern'd for unit tests since the zll-commissioning.h extern
// might not be compiled in.
#if defined(SL_ZIGBEE_SCRIPTED_TEST)
extern uint32_t sli_zigbee_af_zll_secondary_channel_mask;
#endif

sl_zigbee_zll_network_t zllNetwork = { { 0, }, { 0, 0, 0 }, { 0, }, 0, 0, 0, 0, 0, 0 };  // non-static - used by -core.c
#define initZllNetwork(network) \
  do { memmove(&zllNetwork, network, sizeof(sl_zigbee_zll_network_t)); } while (0);
#define deinitZllNetwork() \
  do { zllNetwork.securityAlgorithm.transactionId = 0; } while (0);
#define zllNetworkIsInit() \
  (zllNetwork.securityAlgorithm.transactionId != 0)

#define STATE_NOT_SCANNING     (0x00)
#define STATE_PRIMARY_CHANNELS (0x01)
#define STATE_SCANNING         (0x02)
static uint8_t scanningState;

#define NEGATIVE_BEHAVIOR_IMMEDIATE_RESPONSE (BIT32(0))
#define NEGATIVE_BEHAVIOR_DELAY_RESPONSE     (BIT32(1))
#define NEGATIVE_BEHAVIOR_SUPPRESS_RESPONSE  (BIT32(2))
#define NEGATIVE_BEHAVIOR_POSITIVE_RESPONSE  (BIT32(3))
#define NEGATIVE_BEHAVIOR_BAD_TRANSACTION_ID (BIT32(4))
#define NEGATIVE_BEHAVIOR_BAD_COMMAND_ID     (BIT32(5))
#define NEGATIVE_BEHAVIOR_BAD_SOURCE_ADDRESS (BIT32(6))
#define NEGATIVE_BEHAVIOR_BAD_STATUS         (BIT32(7))
#define NEGATIVE_BEHAVIOR_SET_ZLL_RESERVED_BITS (BIT32(8))
#define NEGATIVE_BEHAVIOR_SET_ZB_RESERVED_BITS  (BIT32(9))
#define NEGATIVE_BEHAVIOR_CLEAR_INITIATOR_BIT   (BIT32(10))
#define NEGATIVE_BEHAVIOR_SET_INITIATOR_BIT     (BIT32(11))
static uint32_t negativeBehaviorMask = 0;
static uint8_t negativeBehaviorCommandId = 0xFF;
static uint32_t responseBehaviorMask = 0;
static uint8_t responseBehaviorCommandId = 0xFF;
static uint32_t scanBehaviorMask = 0;
static uint32_t lastTransactionId = 0;

#define ZLL_RESERVED_BITS (0x4c)
#define ZB_RESERVED_BITS  (0xf8)

uint8_t realNetworkUpdateId = 0xff;

static sli_zigbee_scan_response_data_t globalScanResponseData = { 0, };
static bool isDelayingScanResponse = false;

sl_zigbee_af_event_t sl_zigbee_af_test_harness_z3_zll_stuff_event;
#define z3ZllStuffEventControl (&sl_zigbee_af_test_harness_z3_zll_stuff_event)
void sl_zigbee_af_test_harness_z3_zll_stuff_event_handler(sl_zigbee_af_event_t * event);

#define ZLL_STUFF_EVENT_CONTROL_ACTION_SCAN_RESPONSE (0x00)
#define ZLL_STUFF_EVENT_CONTROL_ACTION_NULL          (0xFF)
static uint8_t zllStuffEventControlAction;

#define RESPONSE_ID 0xACACACAC

#define ENDPOINT       0x01
#define DEVICE_ID      0x1234
#define DEVICE_VERSION 0x01
#define GROUP_COUNT    0x00

// private stack API's
void sli_zigbee_af_zll_set_forced_address_assignment(sl_zigbee_zll_address_assignment_t *assignment);
void sli_zigbee_af_zll_clear_forced_address_assignment(void);
extern void sli_zigbee_set_pan_id(sl_802154_pan_id_t panId);
extern void emRadioEnableAddressMatching(uint8_t enable);

// private plugin API's.
sl_status_t sli_zigbee_af_zll_form_network_for_router_initiator(uint8_t channel, int8_t radioPower, sl_802154_pan_id_t panId);

// MAC header constants (see interpan.c for details)
//ToDo: EMZIGBEE-4854 instead of this we need to use the umac version
#ifdef EZSP_HOST
#define MAC_FRAME_SOURCE_MODE_LONG  0xC000
#define MAC_FRAME_DESTINATION_MODE_LONG    0x0C00
#endif

#define LONG_DEST_FRAME_CONTROL  (MAC_FRAME_TYPE_DATA               \
                                  | MAC_FRAME_DESTINATION_MODE_LONG \
                                  | MAC_FRAME_SOURCE_MODE_LONG)
#define MAC_ACK_REQUIRED              0x0020
#define BROADCAST_PAN_ID 0xffff

// -----------------------------------------------------------------------------
// Util

static sl_status_t startScan(void)
{
  sl_zigbee_node_type_t nodeType;
  if ((sli_zigbee_af_test_harness_z3_device_mode & 0xFE) == EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZR_NOT_ADDRESS_ASSIGNABLE) {
    nodeType = SL_ZIGBEE_ROUTER;
  } else if ((sli_zigbee_af_test_harness_z3_device_mode & 0xFE) == EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZED_NOT_ADDRESS_ASSIGNABLE) {
    nodeType = SL_ZIGBEE_END_DEVICE;
  } else {
    nodeType = SL_ZIGBEE_SLEEPY_END_DEVICE;
  }

  sl_zigbee_af_debug_println("startScan: scanningState = %02X, dev mode = %d, nodeType = %d", scanningState, sli_zigbee_af_test_harness_z3_device_mode, nodeType);
  return sl_zigbee_zll_start_scan((scanningState & STATE_PRIMARY_CHANNELS) ? sl_zigbee_get_zll_primary_channel_mask() : sl_zigbee_get_zll_secondary_channel_mask(),
                                  0, // default power
                                  nodeType);
}

static sl_zigbee_node_type_t getLogicalNodeType(void)
{
  sl_zigbee_node_type_t nodeType;
  sl_status_t status = sl_zigbee_af_get_node_type(&nodeType);

  // Note, we only report as a coordinator if we are a currently
  // coordinator on a centralized network
  if (status == SL_STATUS_NOT_JOINED) {
    nodeType = sli_zigbee_af_current_zigbee_pro_network->nodeType;
    if (nodeType == SL_ZIGBEE_COORDINATOR) {
      nodeType = SL_ZIGBEE_ROUTER;
    }
  }
  sl_zigbee_af_core_println("getLogicalNodeType: node type = %d", nodeType);
  return nodeType;
}

static uint8_t zigbeeInformation(void)
{
  uint8_t byte = 0x00;

  sl_zigbee_node_type_t nodeType = getLogicalNodeType();
  // Set zigbee information based on the node type
  switch (nodeType) {
    case SL_ZIGBEE_COORDINATOR:
      byte = 0x04; //SL_ZIGBEE_COORDINATOR with RX ON When Idle
      break;
    case SL_ZIGBEE_ROUTER:
      byte = (0x01 | 0x04); // SL_ZIGBEE_ROUTER with RX ON When Idle
      break;
    case SL_ZIGBEE_END_DEVICE:
      byte = (0x02 | 0x04);  // ED with RX ON When Idle
      break;
    case SL_ZIGBEE_SLEEPY_END_DEVICE:
      byte = 0x02;  // ED without RX ON When Idle
    default:
      break; // nothing
  }

  if (globalScanResponseData.setReservedInformationBits) {
    byte |= (0xF0 | 0x08);
  }

  return byte;
}

static uint8_t zllInformation(void)
{
  uint8_t byte;
  sl_zigbee_tok_type_stack_zll_data_t token;

  sl_zigbee_zll_get_token_stack_zll_data(&token);
  byte = BYTE_0(token.bitmask);

  switch (sli_zigbee_af_test_harness_z3_device_mode) {
    case EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZR_NOT_ADDRESS_ASSIGNABLE:
    case EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZED_NOT_ADDRESS_ASSIGNABLE:
    case EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_SLEEPY_ZED_NOT_ADDRESS_ASSIGNABLE:
      byte &= ~(SL_ZIGBEE_ZLL_STATE_ADDRESS_ASSIGNMENT_CAPABLE | SL_ZIGBEE_ZLL_STATE_LINK_INITIATOR);
      break;
    case EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZR_ADDRESS_ASSIGNABLE:
    case EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZED_ADDRESS_ASSIGNABLE:
    case EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_SLEEPY_ZED_ADDRESS_ASSIGNABLE:
    default:
      byte |= (SL_ZIGBEE_ZLL_STATE_ADDRESS_ASSIGNMENT_CAPABLE | SL_ZIGBEE_ZLL_STATE_LINK_INITIATOR);
      break;
  }

  if (globalScanResponseData.setReservedInformationBits) {
    byte |= (0x40 | 0x0C);
  }

  return byte;
}

extern uint32_t sli_zigbee_zll_generate_response_id(uint32_t transactionId);
// Uses the global variable globalScanResponseData.
static sl_status_t sendScanResponse(void)
{
  sl_status_t status;
  sl_zigbee_node_type_t nodeType;
  sl_zigbee_network_parameters_t networkParams;
  static uint8_t payload[39]; // scan response payload is 39 bytes with one sub-device

  uint8_t *finger = payload;

  status = sl_zigbee_af_get_network_parameters(&nodeType, &networkParams);
  if (status == SL_STATUS_NOT_JOINED) {
    networkParams.radioChannel = sl_zigbee_get_radio_channel(); // this is our default radio channel
    do {
      sl_zigbee_get_true_random_number_array((uint16_t*)(&networkParams.panId), 1);
    } while (networkParams.panId == 0 || networkParams.panId == 0xffff);  // make sure it is valid
    sl_zigbee_get_true_random_number_array((uint16_t*)(networkParams.extendedPanId), EXTENDED_PAN_ID_SIZE >> 1);

    // Save the (dummy) PAN id for insertion into the MAC header for this message,
    // and for any subsequent touchlink responses.
    sli_zigbee_set_pan_id(networkParams.panId);
  }

  // ZCL.
  *finger++ = (ZCL_CLUSTER_SPECIFIC_COMMAND
               | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
               | ZCL_DISABLE_DEFAULT_RESPONSE_MASK);
  *finger++ = sl_zigbee_af_next_sequence();
  *finger++ = ZCL_SCAN_RESPONSE_COMMAND_ID;

  // ZLL.
  sl_zigbee_af_copy_int32u(finger, 0, globalScanResponseData.transactionId);
  sl_zigbee_af_core_println("sendScanResponse: tr id = %08X", globalScanResponseData.transactionId);
  finger += sizeof(globalScanResponseData.transactionId);
  *finger++ = 0x00; // RSSI correction - whatever
  *finger++ = zigbeeInformation();
  *finger++ = zllInformation();
  sl_zigbee_af_copy_int16u(finger, 0, globalScanResponseData.keyBitmask);
  finger += sizeof(globalScanResponseData.keyBitmask);

  // The purpose of this stack call is really to register 'their' transaction id
  // with the zll library, in case we are expecting a proper network request to
  // follow our mangled scan response (and maybe our mangled dev info responses).
  uint32_t responseId = sli_zigbee_zll_generate_response_id(globalScanResponseData.transactionId);
  sl_zigbee_af_copy_int32u(finger, 0, responseId);
  finger += sizeof(responseId);
  memmove(finger, networkParams.extendedPanId, EXTENDED_PAN_ID_SIZE);
  finger += EXTENDED_PAN_ID_SIZE;
  *finger++ = globalScanResponseData.networkUpdateId;
  *finger++ = networkParams.radioChannel;
  sl_zigbee_af_copy_int16u(finger, 0, networkParams.panId);
  finger += sizeof(networkParams.panId);
  sl_zigbee_af_copy_int16u(finger, 0, sl_zigbee_get_node_id());  // Network address
  finger += sizeof(sl_802154_short_addr_t);
  *finger++ = globalScanResponseData.numberSubDevices;
  *finger++ = globalScanResponseData.totalGroupIds;

  if (globalScanResponseData.numberSubDevices == 1) {
    // Sub-device info goes into scan response.
    *finger++ = 0x01; // endpoint
    sl_zigbee_af_copy_int16u(finger, 0, globalScanResponseData.profileId);
    finger += sizeof(globalScanResponseData.profileId);
    sl_zigbee_af_copy_int16u(finger, 0, DEVICE_ID);
    finger += sizeof(uint16_t);
    *finger++ = DEVICE_VERSION;
    *finger++ = GROUP_COUNT;
  }

  // Calculate actual payload length.
  uint16_t payloadLength = finger - payload;

  sl_802154_short_addr_t node_id;
  (void) sl_zigbee_lookup_node_id_by_eui64(globalScanResponseData.destinationEui64, &node_id);
  status = sl_zigbee_af_send_inter_pan(0xFFFF, // pan ID
                                       globalScanResponseData.destinationEui64,
                                       node_id,
                                       0xFFFF, // multicast id - whatever
                                       ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                                       globalScanResponseData.profileId,
                                       payloadLength,
                                       payload);

  return status;
}

void sendDeviceInfoResponse(const sl_802154_long_addr_t source,
                            uint32_t transaction,
                            uint8_t startIndex)
{
  UNUSED_VAR(source);

  sl_802154_long_addr_t eui64;
  sl_status_t status;
  static uint8_t payload[74]; // device info max payload length = 74 (4 sub-device records)
  uint8_t *finger = payload;

  // ZCL.
  *finger++ = (ZCL_CLUSTER_SPECIFIC_COMMAND
               | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
               | ZCL_DISABLE_DEFAULT_RESPONSE_MASK);
  *finger++ = sl_zigbee_af_next_sequence();
  *finger++ = ZCL_DEVICE_INFORMATION_RESPONSE_COMMAND_ID;

  // ZLL.
  sl_zigbee_af_copy_int32u(finger, 0, transaction);
  finger += sizeof(transaction);
  *finger++ = globalScanResponseData.numberSubDevices;
  *finger++ = startIndex;

  uint8_t *recordCountPtr = finger++;
  *recordCountPtr = 0;

  sl_zigbee_af_core_println("sendDeviceInfoResponse 0x%08X, 0x%02X",
                            transaction,
                            startIndex);

  // Set up some canned data for all sub-records.
  sl_zigbee_af_get_eui64(eui64);
  uint8_t deviceVersion = sl_zigbee_af_device_version_from_index(0);
  sl_zigbee_af_profile_id_t profileId = sl_zigbee_af_profile_id_from_index(0);
  uint16_t deviceId = sl_zigbee_af_device_id_from_index(0);
  uint8_t groupCount = globalScanResponseData.totalGroupIds;

  for (uint8_t i = startIndex; i < globalScanResponseData.numberSubDevices && *recordCountPtr < 4; i++) {
    // Add up to four sub-device records to the message.
    sl_zigbee_af_debug_println("Adding sub-device record: i = %d, recordCount = %d", i, *recordCountPtr);
    memmove(finger, eui64, EUI64_SIZE);
    finger += EUI64_SIZE;
    *finger++ = i + 1; // endpoint (from 1)
    sl_zigbee_af_copy_int16u(finger, 0, profileId);
    finger += sizeof(profileId);
    sl_zigbee_af_copy_int16u(finger, 0, deviceId);
    finger += sizeof(deviceId);
    *finger++ = deviceVersion;
    *finger++ = groupCount;
    *finger++ = i; // sort order
    (*recordCountPtr)++;
  }

  // Calculate actual payload length.
  uint8_t payloadLength = finger - payload;
  sl_802154_short_addr_t node_id;
  (void) sl_zigbee_lookup_node_id_by_eui64(globalScanResponseData.destinationEui64, &node_id);
  status = sl_zigbee_af_send_inter_pan(0xFFFF, // pan ID
                                       globalScanResponseData.destinationEui64,
                                       node_id,
                                       0xFFFF, // multicast id - whatever
                                       ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                                       globalScanResponseData.profileId,
                                       payloadLength,
                                       payload);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_core_println("%s%s failed 0x%02X",
                              "Error: ",
                              "Device information response",
                              status);
  }
}

static uint8_t networkUpdateId(void)
{
  sl_zigbee_node_type_t nodeType;
  sl_zigbee_network_parameters_t networkParams;

  sl_zigbee_af_get_network_parameters(&nodeType, &networkParams);

  return networkParams.nwkUpdateId;
}

// -----------------------------------------------------------------------------
// Touchlink CLI Commands

// plugin test-harness z3 touchlink scan-request-process <linkInitiator:1>
// <unused:1> <options:4>
void sli_zigbee_af_test_harness_z3_touchlink_scan_request_process_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status = SL_STATUS_INVALID_STATE;

#ifndef EZSP_HOST
  uint8_t linkInitiator = sl_cli_get_argument_uint8(arguments, 0);
  uint32_t options      = sl_cli_get_argument_uint32(arguments, 2);
  sl_zigbee_app_debug_println("Scan request process: linkInitiator = %d, options = %08X", linkInitiator, options);
  // Assume we're starting with the primary channel mask.
  scanningState = STATE_NOT_SCANNING | STATE_PRIMARY_CHANNELS;
  if (options & BIT32(1)) {
    // Use current channel for scanning.
    sl_zigbee_set_zll_primary_channel_mask(BIT32(sl_zigbee_af_get_radio_channel()));
    sl_zigbee_set_zll_secondary_channel_mask(0);
  } else if (options & BIT32(4)) {
    // Only use secondary channels for scanning.
    sl_zigbee_set_zll_primary_channel_mask(0);
    sl_zigbee_set_zll_secondary_channel_mask(SL_ZIGBEE_ZLL_SECONDARY_CHANNEL_MASK);
    scanningState &= ~STATE_PRIMARY_CHANNELS;
  }

  // Set a mask for the outgoing message hook function.
  scanBehaviorMask = 0;
  if (options & BIT32(0)) {
    scanBehaviorMask |= NEGATIVE_BEHAVIOR_SET_ZLL_RESERVED_BITS;
  }
  if (options & BIT32(5)) {
    scanBehaviorMask |= NEGATIVE_BEHAVIOR_SET_ZB_RESERVED_BITS;
  }

  // We'll use the outgoing message hook to update the initiator bit,
  // otherwise, we would have to (temporarily) update the stack token.
  if (linkInitiator == 1) {
    scanBehaviorMask |= NEGATIVE_BEHAVIOR_SET_INITIATOR_BIT;
  } else if (linkInitiator == 0) {
    scanBehaviorMask |= NEGATIVE_BEHAVIOR_CLEAR_INITIATOR_BIT;
  }

  // Set the enabled and target bits of the touchlinking policy.
  sl_zigbee_zll_policy_t policy = sl_zigbee_zll_get_policy();  // use Ember API directly
  policy |= SL_ZIGBEE_ZLL_POLICY_ENABLED;
  if (options & BIT32(3)) {
    policy &= ~SL_ZIGBEE_ZLL_POLICY_TARGET;   // reject scan-requests
  } else {
    policy |= SL_ZIGBEE_ZLL_POLICY_TARGET;
  }
  sl_zigbee_zll_set_policy(policy);

  sl_zigbee_app_debug_println("Scan request process: scanningState = %d, scanBehaviorMask = %08X, policy = %02X", scanningState, scanBehaviorMask, policy);

  // Set security for a new scan.
  sl_zigbee_af_zll_set_initial_security_state();

  if ((status  = startScan()) == SL_STATUS_OK) {
    scanningState |= STATE_SCANNING;
  }
#endif /* EZSP_HOST */

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Scan request process",
                            status);
}

// plugin test-harness z3 touchlink start-as-router
void sli_zigbee_af_test_harness_z3_touchlink_start_as_router_command(sl_cli_command_arg_t *arguments)
{
#ifdef SL_CATALOG_ZIGBEE_NETWORK_CREATOR_PRESENT

  sl_802154_pan_id_t panId = sl_cli_get_argument_uint16(arguments, 0);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 1);

  sl_status_t status = SL_STATUS_INVALID_STATE;

  // This options bitmask is currently unused.
  (void)options;

  if (zllNetworkIsInit()) {
    status = sli_zigbee_af_zll_form_network_for_router_initiator(zllNetwork.zigbeeNetwork.channel,
                                                                 SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_RADIO_TX_POWER,
                                                                 panId);
  }

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Start as router",
                            status);
#else
  UNUSED_VAR(arguments);
#endif
}

// plugin test-harness z3 touchlink is-scanning
void sli_zigbee_af_test_harness_z3_touchlink_is_scanning_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  sl_zigbee_af_core_println("scanning:%s",
                            (scanningState & STATE_SCANNING ? "true" : "false"));
}
extern uint8_t sli_mac_lower_mac_get_radio_channel(uint8_t mac_index);
// plugin test-harness z3 touchlink device-information-request
// <startIndex:1> <options:4>
void sli_zigbee_af_test_harness_z3_touchlink_device_information_request_command(sl_cli_command_arg_t *arguments)
{
  uint8_t startIndex = sl_cli_get_argument_uint8(arguments, 0);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 1);

  sl_status_t status = SL_STATUS_INVALID_STATE;

  uint32_t interpanTransactionId = zllNetwork.securityAlgorithm.transactionId;
  if (options & BIT(0)) {
    interpanTransactionId--;
  }

  sl_zigbee_af_debug_println("DeviceInfoRq, zllNetworkIsInit(): %d, tr id = %d", zllNetworkIsInit(), interpanTransactionId);
  if (zllNetworkIsInit()) {
    (void) sl_zigbee_af_fill_external_buffer(EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_CLIENT_TO_SERVER_FRAME_CONTROL,
                                             ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                                             ZCL_DEVICE_INFORMATION_REQUEST_COMMAND_ID,
                                             "wu",
                                             interpanTransactionId,
                                             startIndex);
    status = sl_zigbee_af_send_command_inter_pan(0xFFFF,                // destination pan id
                                                 zllNetwork.eui64,
                                                 SL_ZIGBEE_NULL_NODE_ID, // node id - ignored
                                                 0x0000,       // group id - ignored
                                                 sli_zigbee_af_test_harness_z3_touchlink_profile_id);
  }

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Device information request",
                            status);
}

// plugin test-harness z3 touchlink device-information-request-w-target
// <startIndex:1> <shortAddress:2> <options:4>
void sli_zigbee_af_test_harness_z3_touchlink_device_information_request_w_target_command(sl_cli_command_arg_t *arguments)
{
  uint8_t startIndex = sl_cli_get_argument_uint8(arguments, 0);
  uint16_t shortAddress = sl_cli_get_argument_uint16(arguments, 1);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 2);

  sl_status_t status = SL_STATUS_INVALID_STATE;

  uint32_t interpanTransactionId = zllNetwork.securityAlgorithm.transactionId;
  if (options & BIT(0)) {
    interpanTransactionId--;
  }
  (void)options;

  if (zllNetworkIsInit()) {
    (void) sl_zigbee_af_fill_external_buffer(EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_CLIENT_TO_SERVER_FRAME_CONTROL,
                                             ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                                             ZCL_DEVICE_INFORMATION_REQUEST_COMMAND_ID,
                                             "wu",
                                             interpanTransactionId,
                                             startIndex);
    // TODO: It would probably be better to use ZDO to get the target EUI64, but
    // then we would still need to do a scan to start an active transaction.
    status = sl_zigbee_af_send_command_inter_pan(0xFFFF, // destination pan id
                                                 zllNetwork.eui64, // to keep NWK happy
                                                 shortAddress, // node id
                                                 0x0000, // group id - ignored
                                                 sli_zigbee_af_test_harness_z3_touchlink_profile_id);
  }

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Device information request w target",
                            status);
}

// plugin test-harness z3 touchlink identify-request <duration:2> <options:4>
void sli_zigbee_af_test_harness_z3_touchlink_identify_request_command(sl_cli_command_arg_t *arguments)
{
  uint16_t duration = sl_cli_get_argument_uint16(arguments, 0);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 1);

  sl_status_t status = SL_STATUS_INVALID_STATE;

  uint32_t interpanTransactionId = zllNetwork.securityAlgorithm.transactionId;
  if (options & BIT(0)) {
    interpanTransactionId--;
  }

  if (zllNetworkIsInit()) {
    (void) sl_zigbee_af_fill_external_buffer(EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_CLIENT_TO_SERVER_FRAME_CONTROL,
                                             ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                                             ZCL_IDENTIFY_REQUEST_COMMAND_ID,
                                             "wv",
                                             interpanTransactionId,
                                             duration);
    status = sl_zigbee_af_send_command_inter_pan(0xFFFF,     // destination pan id
                                                 zllNetwork.eui64,
                                                 SL_ZIGBEE_NULL_NODE_ID, // node id - ignored
                                                 0x0000,  // group id - ignored
                                                 sli_zigbee_af_test_harness_z3_touchlink_profile_id);
  }

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Identify request",
                            status);
}

// plugin test-harness z3 touchlink reset-to-factory-new-request <options:4>
void sli_zigbee_af_test_harness_z3_touchlink_rtfn_request_command(sl_cli_command_arg_t *arguments)
{
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 0);
  sl_status_t status = SL_STATUS_INVALID_STATE;

  // We currently do not have a way to use the option bitmask.
  (void)options;

  if (zllNetworkIsInit()) {
    (void) sl_zigbee_af_fill_external_buffer(EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_CLIENT_TO_SERVER_FRAME_CONTROL,
                                             ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                                             ZCL_RESET_TO_FACTORY_NEW_REQUEST_COMMAND_ID,
                                             "w",
                                             zllNetwork.securityAlgorithm.transactionId);
    if (options & BIT(0)) {
      status = sl_zigbee_af_send_command_inter_pan(0xFFFF,     // destination pan id
                                                   NULL, // long id - ignored
                                                   SL_ZIGBEE_BROADCAST_ADDRESS,// bcast
                                                   0x0000, // group id - ignored
                                                   sli_zigbee_af_test_harness_z3_touchlink_profile_id);
    } else {
      status = sl_zigbee_af_send_command_inter_pan(0xFFFF,     // destination pan id
                                                   zllNetwork.eui64,
                                                   SL_ZIGBEE_NULL_NODE_ID,// node id- ignored
                                                   0x0000, // group id - ignored
                                                   sli_zigbee_af_test_harness_z3_touchlink_profile_id);
    }
  }

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Reset to factory new request",
                            status);
}

// plugin test-harness z3 touchlink network-start-request <dstAddress:2>
// <freeAddrBegin:2> <freeAddrEnd:2> <groupIdBegin:2> <groupIdEnd:2> <options:4>
// plugin test-harness z3 touchlink network-join-router-request <dstAddress:2>
// <freeAddrBegin:2> <freeAddrEnd:2> <groupIdBegin:2> <groupIdEnd:2> <options:4>
// plugin test-harness z3 touchlink network-join-end-device-request <dstAddress:2>
// <freeAddrBegin:2> <freeAddrEnd:2> <groupIdBegin:2> <groupIdEnd:2> <options:4>
void sli_zigbee_af_test_harness_z3_touchlink_network_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t nodeId              = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_short_addr_t freeAddrBegin       = sl_cli_get_argument_uint16(arguments, 1);
  sl_802154_short_addr_t freeAddrEnd         = sl_cli_get_argument_uint16(arguments, 2);
  sl_zigbee_multicast_id_t freeGroupBegin = sl_cli_get_argument_uint16(arguments, 3);
  sl_zigbee_multicast_id_t freeGroupEnd   = sl_cli_get_argument_uint16(arguments, 4);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 5);

  uint8_t command = 0;
  sl_zigbee_tok_type_stack_zll_data_t token;
  sl_status_t status = SL_STATUS_INVALID_STATE;
  sl_zigbee_zll_network_t mangledNetwork;

  if (!zllNetworkIsInit()) {
    goto done;
  }

  switch (sl_zigbee_cli_get_argument_string_and_length(arguments, -1, NULL)[13]) {
    case '-':
      command = ZCL_NETWORK_START_REQUEST_COMMAND_ID;
      break;
    case 'r':
      command = ZCL_NETWORK_JOIN_ROUTER_REQUEST_COMMAND_ID;
      break;
    case 'e':
      command = ZCL_NETWORK_JOIN_END_DEVICE_REQUEST_COMMAND_ID;
      break;
    default:
      status = SL_STATUS_INVALID_PARAMETER;
      goto done;
  }

  memmove(&mangledNetwork, &zllNetwork, sizeof(sl_zigbee_zll_network_t));

  sl_zigbee_zll_get_token_stack_zll_data(&token);

  if (command == ZCL_NETWORK_START_REQUEST_COMMAND_ID) {
    // Say that we are FN so that we will indeed send a network start.
    token.bitmask |= SL_ZIGBEE_ZLL_STATE_FACTORY_NEW;
  } else {
    // Say that we are not FN so we will send a network join.
    token.bitmask &= ~SL_ZIGBEE_ZLL_STATE_FACTORY_NEW;
    // Mangle the target device type so that we get the command we want.
    mangledNetwork.nodeType
      = (command == ZCL_NETWORK_JOIN_ROUTER_REQUEST_COMMAND_ID
         ? SL_ZIGBEE_ROUTER
         : SL_ZIGBEE_END_DEVICE);
  }

  // Update the node id with the node id specified in the command line,
  mangledNetwork.nodeId = nodeId;

  // Pretend the remote node is always address capable, to force the stack
  // to include the address and group range fields in the command.
  mangledNetwork.state |= SL_ZIGBEE_ZLL_STATE_ADDRESS_ASSIGNMENT_CAPABLE;

  sl_zigbee_zll_set_token_stack_zll_data(&token);

  if (command == ZCL_NETWORK_START_REQUEST_COMMAND_ID) {
    if (options & BIT32(0)) {
      mangledNetwork.securityAlgorithm.transactionId--;
    }
    if (options & BIT32(1)) {
      sl_802154_long_addr_t address[EUI64_SIZE] = { {
                                                      0xDD, 0xDE, 0xDD, 0xED, 0xDE, 0xDD, 0xED, 0xDE
                                                    } };
      memmove(mangledNetwork.eui64, address, EUI64_SIZE);
    }
  } else { // command == network join
    switch (options) {
      case BIT32(0):
        // TODO: no encryption.
        break;
      case BIT32(1):
        mangledNetwork.securityAlgorithm.bitmask = SL_ZIGBEE_ZLL_KEY_MASK_DEVELOPMENT;
        break;
      case BIT32(2):
        //mangledNetwork.eui64 = NULL;
        mangledNetwork.nodeId = SL_ZIGBEE_BROADCAST_ADDRESS;
        break;
      case BIT32(3):
        mangledNetwork.securityAlgorithm.transactionId--;
        break;
      case BIT32(4): {
        sl_802154_long_addr_t address[EUI64_SIZE] = { {
                                                        0xDD, 0xDE, 0xDD, 0xED, 0xDE, 0xDD, 0xED, 0xDE
                                                      } };
        memmove(mangledNetwork.eui64, address, EUI64_SIZE);
        break;
      }
      default:
        ; // whatever
    }
  }

  // We need to override the Stack's address and group range assignments
  sl_zigbee_zll_address_assignment_t forcedAssignment;
  memset(&forcedAssignment, 0, sizeof(sl_zigbee_zll_address_assignment_t));
  forcedAssignment.freeNodeIdMin  = freeAddrBegin;
  forcedAssignment.freeNodeIdMax  = freeAddrEnd;
  forcedAssignment.freeGroupIdMin = freeGroupBegin;
  forcedAssignment.freeGroupIdMax = freeGroupEnd;
  sli_zigbee_af_zll_set_forced_address_assignment(&forcedAssignment);

  sl_zigbee_af_zll_set_initial_security_state();
  status = sl_zigbee_zll_join_target(&mangledNetwork);

  sli_zigbee_af_zll_clear_forced_address_assignment();

  done:
  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            (command == ZCL_NETWORK_START_REQUEST_COMMAND_ID
                             ? "Network start request"
                             : (command == ZCL_NETWORK_JOIN_ROUTER_REQUEST_COMMAND_ID
                                ? "Network join router request"
                                : "Network join end device request")),
                            status);
}

// plugin test-harness z3 touchlink network-update-request <options:4>
void sli_zigbee_af_test_harness_z3_touchlink_network_update_request_command(sl_cli_command_arg_t *arguments)
{
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 0);

  sl_status_t status = SL_STATUS_INVALID_STATE;
  sl_zigbee_node_type_t nodeType;
  sl_zigbee_network_parameters_t networkParameters;
  uint8_t mangledNwkUpdateId;

  sl_zigbee_af_get_network_parameters(&nodeType, &networkParameters);

  mangledNwkUpdateId = networkParameters.nwkUpdateId;
  if (options == BIT32(0)) {
    mangledNwkUpdateId += 5;
  }

  if (zllNetworkIsInit()) {
    (void) sl_zigbee_af_fill_external_buffer(EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_CLIENT_TO_SERVER_FRAME_CONTROL,
                                             ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                                             ZCL_NETWORK_UPDATE_REQUEST_COMMAND_ID,
                                             "w8uuvv",
                                             zllNetwork.securityAlgorithm.transactionId,
                                             networkParameters.extendedPanId,
                                             mangledNwkUpdateId,
                                             networkParameters.radioChannel,
                                             networkParameters.panId,
                                             sl_zigbee_af_get_node_id());
    status = sl_zigbee_af_send_command_inter_pan(0xFFFF, // destination pan id
                                                 zllNetwork.eui64,
                                                 SL_ZIGBEE_NULL_NODE_ID, // node id - ignored
                                                 0x0000,   // group id - ignored
                                                 sli_zigbee_af_test_harness_z3_touchlink_profile_id);
  }

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Network update request",
                            status);
}

// plugin test-harness z3 touchlink get-group-identifiers-request
// <startIndex:1>
void sli_zigbee_af_test_harness_z3_touchlink_get_group_identifiers_request_command(sl_cli_command_arg_t *arguments)
{
  uint8_t startIndex = sl_cli_get_argument_uint8(arguments, 0);
  sl_status_t status = SL_STATUS_INVALID_STATE;
  sl_802154_short_addr_t destination;

  if (zllNetworkIsInit()) {
    if ((destination = zllNetwork.nodeId) == SL_ZIGBEE_NULL_NODE_ID) {
      (void) sl_zigbee_lookup_node_id_by_eui64(zllNetwork.eui64, &destination);
    }
    sl_zigbee_af_fill_command_zll_commissioning_cluster_get_group_identifiers_request(startIndex);
    sl_zigbee_af_set_command_endpoints(1, 1); // endpoints?
    status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, destination);
  }

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Get group identifiers request",
                            status);
}

// plugin test-harness z3 touchlink get-endpoint-list-request
// <startIndex:1>
void sli_zigbee_af_test_harness_z3_touchlink_get_endpoint_list_request_command(sl_cli_command_arg_t *arguments)
{
  uint8_t startIndex = sl_cli_get_argument_uint8(arguments, 0);
  sl_status_t status = SL_STATUS_INVALID_STATE;
  sl_802154_short_addr_t destination;

  if (zllNetworkIsInit()) {
    if ((destination = zllNetwork.nodeId) == SL_ZIGBEE_NULL_NODE_ID) {
      (void) sl_zigbee_lookup_node_id_by_eui64(zllNetwork.eui64, &destination);
    }
    sl_zigbee_af_fill_command_zll_commissioning_cluster_get_endpoint_list_request(startIndex);
    sl_zigbee_af_set_command_endpoints(1, 1); // endpoints?
    status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, destination);
  }

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Get endpoint list request",
                            status);
}

// plugin test-harness z3 touchlink scan-response-config <options:4>
void sli_zigbee_af_test_harness_z3_touchlink_scan_response_config_command(sl_cli_command_arg_t *arguments)
{
  uint32_t options      = sl_cli_get_argument_uint32(arguments, 0);

  globalScanResponseData.keyBitmask = SL_ZIGBEE_ZLL_KEY_MASK_CERTIFICATION;
  globalScanResponseData.profileId = HA_PROFILE_ID; // use HA profile for endpoint info
  globalScanResponseData.setReservedInformationBits = false;
  globalScanResponseData.networkUpdateId = networkUpdateId();
  globalScanResponseData.totalGroupIds = 0;
  globalScanResponseData.numberSubDevices = 1;

  negativeBehaviorMask = 0;
  negativeBehaviorCommandId = 0xff;

  if (options) {
    if (options & BIT32(0)) {
      negativeBehaviorMask |= NEGATIVE_BEHAVIOR_BAD_TRANSACTION_ID;
    }
    if (options & BIT32(1)) {
      negativeBehaviorMask |= NEGATIVE_BEHAVIOR_POSITIVE_RESPONSE;
    }
    if (options & BIT32(2)) {
      negativeBehaviorMask |= NEGATIVE_BEHAVIOR_DELAY_RESPONSE;
    }
    if (options & BIT32(3)) {
      negativeBehaviorMask |= NEGATIVE_BEHAVIOR_IMMEDIATE_RESPONSE;
      globalScanResponseData.keyBitmask = 0x0000;
    }
    if (options & BIT32(4)) {
      negativeBehaviorMask |= NEGATIVE_BEHAVIOR_IMMEDIATE_RESPONSE;
      globalScanResponseData.keyBitmask = 0x0001;
    }
    if (options & BIT32(5)) {
      negativeBehaviorMask |= NEGATIVE_BEHAVIOR_IMMEDIATE_RESPONSE;
      globalScanResponseData.setReservedInformationBits = true;
    }
    if (options & BIT32(6)) {
      negativeBehaviorMask |= NEGATIVE_BEHAVIOR_IMMEDIATE_RESPONSE;
      // As per the nwkUpdateId roll over logic defined in the ZCL6.
      // 13.3.4.9 Frequency Agility, the following will set it low.
      globalScanResponseData.networkUpdateId += 201;
    }
    if (options & BIT32(7)) {
      negativeBehaviorMask |= NEGATIVE_BEHAVIOR_IMMEDIATE_RESPONSE;
      globalScanResponseData.networkUpdateId = 0x50;
    }
    if (options & BIT32(8)) {
      negativeBehaviorMask |= NEGATIVE_BEHAVIOR_SUPPRESS_RESPONSE;
    }
    if (options & BIT32(9)) {
      negativeBehaviorMask |= NEGATIVE_BEHAVIOR_IMMEDIATE_RESPONSE;
      globalScanResponseData.numberSubDevices = 10;
    }
    if (options & BIT32(10)) {
      negativeBehaviorMask |= NEGATIVE_BEHAVIOR_IMMEDIATE_RESPONSE;
      globalScanResponseData.networkUpdateId += 0x5A;
    }
    if (options & BIT32(11)) {
      negativeBehaviorMask |= NEGATIVE_BEHAVIOR_IMMEDIATE_RESPONSE;
      globalScanResponseData.keyBitmask = 0x0020;
    }
  }
  if (negativeBehaviorMask) {
    if (realNetworkUpdateId == 0xff) {
      // Save the real network update id.
      realNetworkUpdateId = networkUpdateId();
    }
    // Set the actual update id to match, in case we get a beacon request
    // when a ZED initiator updates its own update id in response to a scan
    // response with a higher update id.
    sl_zigbee_set_nwk_update_id(globalScanResponseData.networkUpdateId, true);

    negativeBehaviorCommandId = ZCL_SCAN_REQUEST_COMMAND_ID;
  } else if (realNetworkUpdateId != 0xff) {
    // Restore the real network update id on a config reset.
    sl_zigbee_set_nwk_update_id(realNetworkUpdateId, true);
    realNetworkUpdateId = 0xff;
  }
  // Set the network update Id so that the scan responses to
  // multiple scan request produces the same id
  sl_zigbee_set_nwk_update_id(globalScanResponseData.networkUpdateId, true);
  negativeBehaviorCommandId = ZCL_SCAN_REQUEST_COMMAND_ID;

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Scan response config",
                            SL_STATUS_OK);
}
// plugin test-harness z3 touchlink network-start-response-config <options:4>
void sli_zigbee_af_test_harness_z3_touchlink_network_start_response_config(sl_cli_command_arg_t *arguments)
{
  uint32_t options = sl_cli_get_argument_uint32(arguments, 0);

  responseBehaviorMask = 0;
  responseBehaviorCommandId = 0xff;
  if (options) {
    if (options & BIT32(0)) {
      responseBehaviorMask |= NEGATIVE_BEHAVIOR_SUPPRESS_RESPONSE;
    }
    if (options & BIT32(1)) {
      responseBehaviorMask |= NEGATIVE_BEHAVIOR_BAD_COMMAND_ID;
    }
    if (options & BIT32(2)) {
      responseBehaviorMask |= NEGATIVE_BEHAVIOR_BAD_TRANSACTION_ID;
    }
    if (options & BIT32(3)) {
      responseBehaviorMask |= NEGATIVE_BEHAVIOR_BAD_SOURCE_ADDRESS;
    }
    if (options & BIT32(4)) {
      responseBehaviorMask |= NEGATIVE_BEHAVIOR_BAD_STATUS;
    }
  }
  if (responseBehaviorMask) {
    responseBehaviorCommandId = ZCL_NETWORK_START_RESPONSE_COMMAND_ID;
  }

  // All options are handled in the negative behaviour callbacks
  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Network start response config",
                            SL_STATUS_OK);
}

// plugin test-harness z3 touchlink network-join-router-response-config <options:4>
void sli_zigbee_af_test_harness_z3_touchlink_network_join_router_response_config(sl_cli_command_arg_t *arguments)
{
  uint32_t options = sl_cli_get_argument_uint32(arguments, 0);

  responseBehaviorMask = 0;
  responseBehaviorCommandId = 0xff;
  if (options) {
    if (options & BIT32(0)) {
      responseBehaviorMask |= NEGATIVE_BEHAVIOR_SUPPRESS_RESPONSE;
    }
    if (options & BIT32(1)) {
      responseBehaviorMask |= NEGATIVE_BEHAVIOR_BAD_COMMAND_ID;
    }
    if (options & BIT32(2)) {
      responseBehaviorMask |= NEGATIVE_BEHAVIOR_BAD_TRANSACTION_ID;
    }
    if (options & BIT32(3)) {
      responseBehaviorMask |= NEGATIVE_BEHAVIOR_BAD_SOURCE_ADDRESS;
    }
    if (options & BIT32(4)) {
      responseBehaviorMask |= NEGATIVE_BEHAVIOR_BAD_STATUS;
    }
  }
  if (responseBehaviorMask) {
    responseBehaviorCommandId = ZCL_NETWORK_JOIN_ROUTER_RESPONSE_COMMAND_ID;
  }

  // All options are handled in the negative behaviour callbacks
  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Network join router response config",
                            SL_STATUS_OK);
}

// plugin test-harness z3 touchlink device-information-response-config
// <options:4>
void sli_zigbee_af_test_harness_z3_touchlink_device_information_response_config_command(sl_cli_command_arg_t *arguments)
{
  uint32_t options = sl_cli_get_argument_uint32(arguments, 0);

  negativeBehaviorMask = 0;
  negativeBehaviorCommandId = 0xff;
  if (options & BIT32(0)) {
    negativeBehaviorMask |= NEGATIVE_BEHAVIOR_SUPPRESS_RESPONSE;
  }

  if (negativeBehaviorMask) {
    negativeBehaviorCommandId = ZCL_DEVICE_INFORMATION_REQUEST_COMMAND_ID;
  }
  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Device information response config",
                            SL_STATUS_OK);
}

// -----------------------------------------------------------------------------
// Framework callbacks

void sli_zigbee_af_test_harness_z3_zll_network_found_callback(const sl_zigbee_zll_network_t *networkInfo)
{
  sl_zigbee_af_core_println("%s: %s: node type = %d, zll state = 0x%04X, node id = 0x%04X, pan id = 0x%04X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Network found",
                            networkInfo->nodeType,
                            networkInfo->state,
                            networkInfo->nodeId,
                            networkInfo->zigbeeNetwork.panId);
  initZllNetwork(networkInfo);
}

bool sli_zigbee_af_test_harness_z3_zll_scan_complete_callback(sl_status_t status)
{
  bool handled = false;
  if (scanningState & STATE_SCANNING) {
    // If this was a test scan, then we'll handle the scan complete, and
    // initiate a secondary channel scan if required.
    if (!zllNetworkIsInit()
        && (scanningState & STATE_PRIMARY_CHANNELS)
        && sl_zigbee_get_zll_secondary_channel_mask() != 0) {
      scanningState &= ~STATE_PRIMARY_CHANNELS;
      startScan();
    } else {
      sl_zigbee_af_core_println("%s: %s: 0x%02X",
                                TEST_HARNESS_Z3_PRINT_NAME,
                                "Scan Complete",
                                SL_STATUS_OK,
                                status);
      scanningState = STATE_NOT_SCANNING;
      if (status != SL_STATUS_OK) {
        deinitZllNetwork();
      }
      // Tell the API that we have finished scanning, so that it won't
      // attempt to re-use our old transaction code.
      sl_zigbee_zll_scanning_complete();
    }
    // Do NOT cancel any negative mode here on the scan request.

    // Tell zll commissioning client we've already processed the scan complete.
    handled = true;
  }
  return handled;
}

sl_zigbee_packet_action_t sli_zigbee_af_test_harness_z3_zll_command_callback(uint8_t  *command,
                                                                             sl_802154_long_addr_t sourceEui64)
{
  // The start index points at the beginning of the ZCL frame.
  // FRAME_CONTROL:1 | SEQUENCE:1 | COMMAND_ID:1 | start of transaction...
  uint8_t transactionIdIndex = 3;
  uint8_t commandId = command[2];
  uint32_t realTransactionId = sl_util_fetch_low_high_int32u(command + transactionIdIndex);
  sl_zigbee_packet_action_t act = SL_ZIGBEE_ACCEPT_PACKET;
  sl_zigbee_af_debug_println("emAfPluginTestHarnessZ3ZllCommandCallback: commandId = %02X", commandId);

  // Print the source EUI64 for certain commands only
  if (sourceEui64 != NULL) {
    if (commandId == ZCL_NETWORK_START_RESPONSE_COMMAND_ID
        || commandId == ZCL_NETWORK_JOIN_ROUTER_RESPONSE_COMMAND_ID
        || commandId == ZCL_SCAN_RESPONSE_COMMAND_ID) {
      sl_zigbee_af_core_print("sourceEui64: ");
      sl_zigbee_af_core_print_buffer(sourceEui64, 8, true);   // spaces?
      sl_zigbee_af_core_println("");
    }
  } else {
    return SL_ZIGBEE_ACCEPT_PACKET;
  }

  if (commandId == ZCL_NETWORK_START_RESPONSE_COMMAND_ID
      || commandId == ZCL_NETWORK_JOIN_ROUTER_RESPONSE_COMMAND_ID
      || commandId == ZCL_NETWORK_JOIN_END_DEVICE_RESPONSE_COMMAND_ID ) {
    // Disable address matching until we join, in order to detect device
    // announce message from target.
    emRadioEnableAddressMatching(false);
  }
  // Ignore any negative behavior if the command doesn't match up, except
  // if the previous scan config option forces a device info request.
  // (negativeBehaviorCommandId will still be ZCL_SCAN_REQUEST_COMMAND_ID)
  if (commandId != negativeBehaviorCommandId
      && (commandId != ZCL_DEVICE_INFORMATION_REQUEST_COMMAND_ID
          || globalScanResponseData.numberSubDevices == 1)) {
    return act;
  }

  globalScanResponseData.transactionId = realTransactionId;
  memmove(globalScanResponseData.destinationEui64, sourceEui64, EUI64_SIZE);

  if (negativeBehaviorMask
      & (NEGATIVE_BEHAVIOR_SUPPRESS_RESPONSE
         | NEGATIVE_BEHAVIOR_DELAY_RESPONSE
         | NEGATIVE_BEHAVIOR_IMMEDIATE_RESPONSE)) {
    if (!(negativeBehaviorMask & NEGATIVE_BEHAVIOR_POSITIVE_RESPONSE)) {
      // Overwrite the transaction ID to be 0. This will make us not respond
      // to the command.
      sl_util_store_low_high_int32u(command + transactionIdIndex, 0);
      act = SL_ZIGBEE_MANGLE_PACKET;
    }

    if (negativeBehaviorMask & NEGATIVE_BEHAVIOR_DELAY_RESPONSE) {
      // Delay 8 seconds, since this is the transaction ID lifetime. But
      // not if we are already delaying, e.g. for multiple scan requests
      // on first primary channel.
      if (!isDelayingScanResponse) {
        isDelayingScanResponse = true;
        sl_zigbee_af_event_set_delay_ms(z3ZllStuffEventControl,
                                        MILLISECOND_TICKS_PER_SECOND * 8);
        zllStuffEventControlAction = ZLL_STUFF_EVENT_CONTROL_ACTION_SCAN_RESPONSE;
      }
    } else if (negativeBehaviorMask & NEGATIVE_BEHAVIOR_IMMEDIATE_RESPONSE) {
      if (commandId == ZCL_DEVICE_INFORMATION_REQUEST_COMMAND_ID) {
        // Send the next chunk of device info.
        uint8_t startIndex = command[transactionIdIndex + 4];
        sendDeviceInfoResponse(sourceEui64, realTransactionId, startIndex);
      } else {   // Must be a scan request
        // Send our own response immediately, but only to the first scan request.
        if (realTransactionId != lastTransactionId) {
          lastTransactionId = realTransactionId;
          sendScanResponse();
        } else {
          sl_zigbee_af_debug_println("Dropping scan req w/dupl trans id = %08X", realTransactionId);
        }
      }
    }
  } else if (negativeBehaviorMask & NEGATIVE_BEHAVIOR_BAD_TRANSACTION_ID) {
    sl_util_store_low_high_int32u(command + transactionIdIndex, realTransactionId - 1);
    act = SL_ZIGBEE_MANGLE_PACKET;
  }

  return act;
}

void sl_zigbee_af_test_harness_z3_zll_stuff_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_status_t status;

  sl_zigbee_af_event_set_inactive(z3ZllStuffEventControl);
  sl_zigbee_af_core_println("sl_zigbee_af_test_harness_z3_zll_stuff_event_handler: entry");

  switch (zllStuffEventControlAction) {
    case ZLL_STUFF_EVENT_CONTROL_ACTION_SCAN_RESPONSE:
      isDelayingScanResponse = false;
      status = sendScanResponse();
      sl_zigbee_af_core_println("emZllSendScanResponse, status = %02X", status);
      break;
    default:
      status = SL_STATUS_INVALID_PARAMETER;
  }

  sl_zigbee_af_core_println("%s: %s (0x%02X): 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "ZLL event handler",
                            zllStuffEventControlAction,
                            status);

  zllStuffEventControlAction = ZLL_STUFF_EVENT_CONTROL_ACTION_NULL;
}

sl_zigbee_packet_action_t sli_zigbee_af_test_harness_z3_zll_modify_interpan_command(uint8_t *commandData,
                                                                                    uint8_t macHeaderLength)
{
  static const sl_802154_long_addr_t badEui64 = { 0xde, 0xed, 0xdd, 0xde, 0xed, 0xdd, 0xde, 0xdd };
  sl_zigbee_packet_action_t act = SL_ZIGBEE_ACCEPT_PACKET;

  uint8_t *apsPointer = commandData + macHeaderLength + 7;
  uint8_t cmdId = *(apsPointer + 2);
  sl_zigbee_af_debug_println("Modify Interpan: cmd id = %d, mask = %08X", cmdId, responseBehaviorMask);

  // Ignore any negative behavior if the command doesn't match up.
  if (cmdId == responseBehaviorCommandId) {
    if (responseBehaviorMask & NEGATIVE_BEHAVIOR_SUPPRESS_RESPONSE) {
      return SL_ZIGBEE_DROP_PACKET;
    }

    // This is the only option which affects the MAC header
    if (responseBehaviorMask & NEGATIVE_BEHAVIOR_BAD_SOURCE_ADDRESS) {
      memmove(commandData + (macHeaderLength - 8), badEui64, EUI64_SIZE);
    }

    // The remaining options pertain to the APS payload
    if (responseBehaviorMask & NEGATIVE_BEHAVIOR_BAD_COMMAND_ID) {
      *(apsPointer + 2) = ZCL_NETWORK_JOIN_END_DEVICE_RESPONSE_COMMAND_ID;
    }
    if (responseBehaviorMask & NEGATIVE_BEHAVIOR_BAD_TRANSACTION_ID) {
      memset(apsPointer + 3, 0, 4);
    }
    if (responseBehaviorMask & NEGATIVE_BEHAVIOR_BAD_STATUS) {
      *(apsPointer + 7) = SL_STATUS_FAIL;
    }

    if (responseBehaviorMask) {
      act = SL_ZIGBEE_MANGLE_PACKET;
    }
  } else if (cmdId == ZCL_SCAN_RESPONSE_COMMAND_ID) {
    // Ensure the zigbee and touchlink information fields are set
    // according to our device mode - this is particularly important
    // for the address assignment capability bit.
    *(apsPointer + 8) = zigbeeInformation();
    *(apsPointer + 9) = zllInformation();
    act = SL_ZIGBEE_MANGLE_PACKET;
  } else if (cmdId == ZCL_SCAN_REQUEST_COMMAND_ID) {
    // This is the most convenient place to set and clear bits.
    sl_zigbee_af_debug_println("Modify Interpan: scanBehaviorMask = %08X", scanBehaviorMask);
    if (scanBehaviorMask & NEGATIVE_BEHAVIOR_SET_ZB_RESERVED_BITS) {
      *(apsPointer + 7) |= ZB_RESERVED_BITS;
    }
    if (scanBehaviorMask & NEGATIVE_BEHAVIOR_SET_ZLL_RESERVED_BITS) {
      *(apsPointer + 8) |= ZLL_RESERVED_BITS;
    }
    if (scanBehaviorMask & NEGATIVE_BEHAVIOR_CLEAR_INITIATOR_BIT) {
      *(apsPointer + 8) &= ~0x10;
    }
    if (scanBehaviorMask & NEGATIVE_BEHAVIOR_SET_INITIATOR_BIT) {
      *(apsPointer + 8) |= 0x10;
    }
    act = SL_ZIGBEE_MANGLE_PACKET;
  }
  return act;
}

// plugin zll-commissioning set-radio-idle-mode
void sli_zigbee_test_harness_set_radio_idle_mode(sl_cli_command_arg_t *arguments)
{
  uint8_t mode = (uint8_t)sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_zll_set_radio_idle_mode(mode);
}

// plugin zll-commissioning get-radio-idle-mode
void sli_zigbee_test_harness_get_radio_idle_mode(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  uint8_t mode = sl_zigbee_zll_get_radio_idle_mode();
  sl_zigbee_af_app_println("%s %s", "MAC Radio Idle Mode: ", mode ? "off" : "rx-on");
}
