/***************************************************************************//**
 * @file
 * @brief Common code for Application Test Profile #2.  This was originally part of
 * level-three.c but was extracted and put into a common file.
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

#include PLATFORM_HEADER

// Ember stack and related utilities.
#include "stack/include/sl_zigbee.h"               // Main stack definitions.
#include "stack/include/zigbee-device-stack.h" // ZigBee Device Object.
#include "stack/internal/inc/internal-defs-patch.h"

// HAL.
#include "hal/hal.h"

// Application utilities.
#include "serial/serial.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "app/util/common/common.h"
#include "app/util/zigbee-framework/zigbee-device-library.h"

#include "test-profile.h"
#include "pro-compliance-common.h"
#include "app/framework/plugin/fragmentation/fragmentation.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_SIMPLE_LED_PRESENT
#include "sl_simple_led_instances.h"
#endif // SL_CATALOG_SIMPLE_LED_PRESENT
//------------------------------------------------------------------------------
// External Declarations

//------------------------------------------------------------------------------
// Globals

typedef struct {
  bool inUse;
  uint8_t apsSeqn;
  uint32_t sendTimeMs;
} sli_zigbee_pkt_in_flight_info_t;

#define MAX_IN_FLIGHT_PACKETS     10

static uint8_t pktLength[SL_ZIGBEE_SUPPORTED_NETWORKS];
static uint16_t pktTotalCount[SL_ZIGBEE_SUPPORTED_NETWORKS];
static uint16_t pktInterval[SL_ZIGBEE_SUPPORTED_NETWORKS];
static uint8_t pktMaxInFlight[SL_ZIGBEE_SUPPORTED_NETWORKS];
static uint8_t pktSourceEndpoint[SL_ZIGBEE_SUPPORTED_NETWORKS];
static sl_802154_short_addr_t pktDestination[SL_ZIGBEE_SUPPORTED_NETWORKS];
static uint8_t pktDestEndpoint[SL_ZIGBEE_SUPPORTED_NETWORKS];
static sl_zigbee_aps_option_t pktOptions[SL_ZIGBEE_SUPPORTED_NETWORKS];
static uint8_t pktInFlight[SL_ZIGBEE_SUPPORTED_NETWORKS];
static uint16_t pktRunningCount[SL_ZIGBEE_SUPPORTED_NETWORKS];
static uint32_t pktSendStartTime[SL_ZIGBEE_SUPPORTED_NETWORKS];
static uint32_t totalBytesSent[SL_ZIGBEE_SUPPORTED_NETWORKS];
static uint32_t pktSuccessCount[SL_ZIGBEE_SUPPORTED_NETWORKS];
static sli_zigbee_pkt_in_flight_info_t pktInFlightInfo[SL_ZIGBEE_SUPPORTED_NETWORKS][MAX_IN_FLIGHT_PACKETS];
static uint32_t pktSendTimeSum[SL_ZIGBEE_SUPPORTED_NETWORKS];
static uint32_t pktMinSendTime[SL_ZIGBEE_SUPPORTED_NETWORKS];
static uint32_t pktMaxSendTime[SL_ZIGBEE_SUPPORTED_NETWORKS];

#define MAX_TRANSMIT_MESSAGE_PACKET_LENGTH  255

// For now we leave these variables common to all networks.
static uint16_t pktRxCount;
static uint16_t pktLastRxCounter;
static bool pktRxReset = true;
static uint8_t broadcastRadius = 0xFF;
static uint16_t broadcastAddr = 0;
static bool ongoingSendingPackets = false;
uint8_t maxMessageLength = DONT_FRAGMENT;
uint8_t maxFragmentLength = 30;
sl_zigbee_outgoing_message_type_t outgoingMessageType = SL_ZIGBEE_OUTGOING_DIRECT;

// Up to 4 networks
sl_zigbee_af_event_t realSendEvent[4];
// TODO: Once we fully port the pro-compliance app to UC, we can
// get rid of the sl_zigbee_af_event_t pointer array.
sl_zigbee_af_event_t *sendEvent[4] = {
  &realSendEvent[0],
  &realSendEvent[1],
  &realSendEvent[2],
  &realSendEvent[3]
};

#define TEST_PROFILE_ID 0x7F01  // Per 064166r00
// Need this variable so applications can have access to it.
const uint16_t testProfileId = TEST_PROFILE_ID;

const uint16_t testDriverInClusters[] = { TEST_DRIVER_IN_CLUSTERS };
const uint16_t testDriverOutClusters[] = { TEST_DRIVER_OUT_CLUSTERS };

sl_zigbee_endpoint_description_t const testDriverDescription = {
  TEST_PROFILE_ID,  // Test Profile #2 profile ID.
  0x0000,  // Test Driver device ID.
  0,       // No complex or user descriptor.
  sizeof(testDriverInClusters) >> 1,
  sizeof(testDriverOutClusters) >> 1
};

const uint16_t fullDeviceInClusters[] = { FULL_DEVICE_IN_CLUSTERS };
const uint16_t fullDeviceOutClusters[] = { FULL_DEVICE_OUT_CLUSTERS };

sl_zigbee_endpoint_description_t const fullDeviceDescription = {
  TEST_PROFILE_ID,  // Test Profile #2 profile ID.
  0xFFFF,  // Full Device Under Test device ID.
  0,       // No complex or user descriptor.
  sizeof(fullDeviceInClusters) >> 1,
  sizeof(fullDeviceOutClusters) >> 1
};

uint16_t profileIdForInitiatedMessages = TEST_PROFILE_ID;
static bool gCountPacketsOutOfOrder =
#if defined(COUNT_PACKETS_OUT_OF_ORDER)
  true;
#else
  false;
#endif

bool sli_zigbee_print_incoming_packet_enabled = true;

//------------------------------------------------------------------------------
// Forward declarations.

static void sendEventHandler(uint8_t nwkIndex);

static void printPktsSendStats(uint8_t nwkIndex);

sl_status_t sendMulticast(sl_zigbee_aps_frame_t *apsFrame,
                          uint8_t radius,
                          uint16_t broadcastAddr,
                          sli_buffer_manager_buffer_t message);

//------------------------------------------------------------------------------
// Functions

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
uint8_t sl_zigbee_af_maximum_aps_payload_length_pro_compliance(sl_zigbee_outgoing_message_type_t type,
                                                               uint16_t indexOrDestination,
                                                               sl_zigbee_aps_frame_t *apsFrame)
#else
uint8_t sl_zigbee_af_maximum_aps_payload_length(sl_zigbee_outgoing_message_type_t type,
                                                uint16_t indexOrDestination,
                                                sl_zigbee_aps_frame_t *apsFrame)
#endif
{
  UNUSED_VAR(type);
  UNUSED_VAR(indexOrDestination);
  UNUSED_VAR(apsFrame);
  return maxFragmentLength;
}

void setProfileIdCommand(SL_CLI_COMMAND_ARG)
{
  profileIdForInitiatedMessages = sl_cli_get_argument_uint16(arguments, 0);
}

static bool sendResponse(sl_802154_short_addr_t sender,
                         sl_zigbee_aps_frame_t *inApsFrame,
                         uint16_t clusterId,
                         uint8_t responseLength,
                         uint8_t *responseContents)
{
  sl_zigbee_aps_frame_t outApsFrame;
  uint16_t messageTag;
  outApsFrame.sourceEndpoint = inApsFrame->destinationEndpoint;
  outApsFrame.destinationEndpoint = inApsFrame->sourceEndpoint;
  outApsFrame.clusterId = clusterId;
  outApsFrame.profileId = testProfileId;
  outApsFrame.options = inApsFrame->options;
  // Always respond direct.  Otherwise we would have to lookup the
  // the destination based on the outgoing message type,
  // and that requires too much code.
  return (((maxMessageLength == DONT_FRAGMENT
            || responseLength < maxMessageLength)
           ? sl_zigbee_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                    sender,
                                    &outApsFrame,
                                    0x00,        // tag
                                    responseLength,
                                    (uint8_t*)responseContents,
                                    NULL)
           : sli_zigbee_af_fragmentation_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                                      sender,
                                                      &outApsFrame,
                                                      (uint8_t*)responseContents,
                                                      (uint16_t) responseLength,
                                                      &messageTag))
          == SL_STATUS_OK);
}

//------------------------------------------------------------------------------
sl_status_t transmitMessage(sl_802154_short_addr_t destination,
                            uint8_t messageLength,
                            uint8_t* messageContents,
                            uint8_t sourceEndpoint,
                            uint8_t destEndpoint,
                            uint16_t clusterId,
                            uint8_t *apsSeqn,
                            sl_zigbee_aps_option_t options,
                            uint16_t nwkBroadcastDestination)
{
  sl_zigbee_aps_frame_t apsFrame;
  sl_status_t result;

  apsFrame.sourceEndpoint = sourceEndpoint;
  apsFrame.destinationEndpoint = destEndpoint;
  apsFrame.clusterId = clusterId;
  apsFrame.profileId = profileIdForInitiatedMessages;
  apsFrame.options = options;

  if ((options & ZIGBEE_APS_FRAME_CONTROL_MODE_MASK)
      == ZIGBEE_APS_FRAME_CONTROL_MODE_MULTICAST) {
    apsFrame.groupId = destination;

    // APS multicast
    result = sl_zigbee_send_multicast(&apsFrame,
                                      broadcastRadius,
                                      nwkBroadcastDestination,
                                      0xFFFF,
                                      0,
                                      0x00,        // tag
                                      messageLength,
                                      messageContents,
                                      NULL);
  } else if (SL_ZIGBEE_BROADCAST_ADDRESS <= destination) {
    result = sl_zigbee_send_broadcast(SL_ZIGBEE_NULL_NODE_ID,
                                      destination,
                                      0,
                                      &apsFrame,
                                      broadcastRadius,
                                      0x00,        // tag
                                      messageLength,
                                      messageContents,
                                      NULL);
  } else {
    uint16_t messageTag;
    result = (((maxMessageLength == DONT_FRAGMENT
                || messageLength < maxMessageLength))
              ? sl_zigbee_send_unicast(outgoingMessageType,
                                       destination,
                                       &apsFrame,
                                       0x00,        // tag
                                       messageLength,
                                       messageContents,
                                       NULL)
              : sli_zigbee_af_fragmentation_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                                         destination,
                                                         &apsFrame,
                                                         messageContents,
                                                         messageLength,
                                                         &messageTag));
  }

  if (apsSeqn != NULL) {
    *apsSeqn = apsFrame.sequence;
  }

  return result;
}

//------------------------------------------------------------------------------
void sendAliasMsg(SL_CLI_COMMAND_ARG)
{
  uint8_t seq      = sl_cli_get_argument_uint8(arguments, 0);
  uint16_t alias    = sl_cli_get_argument_uint16(arguments, 1);
  uint8_t data[4] = { 0x1, 0x2, 0x3, 0x4 };
  sl_zigbee_aps_frame_t apsFrame;
  apsFrame.sourceEndpoint = 0xFF;
  apsFrame.destinationEndpoint = 0xFF;
  apsFrame.options = 0;
  apsFrame.profileId = 0x7F01; // test profile ID
  apsFrame.clusterId = 0x0001; // counted packets cluster
  apsFrame.radius =  0x2;
  apsFrame.groupId = 0xFABE;

  sl_zigbee_send_multicast(&apsFrame,
                           apsFrame.radius,
                           7,
                           alias,
                           seq,
                           0x00,        // tag
                           sizeof(data),
                           (uint8_t*) data,
                           NULL);
}
void msgCommand(SL_CLI_COMMAND_ARG)
{
  uint8_t length;
  uint8_t *command = sl_zigbee_cli_get_argument_string_and_length(arguments, -1, &length);
  uint8_t data              = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t sourceEndpoint    = sl_cli_get_argument_uint8(arguments, 1);
  sl_802154_short_addr_t destination = sl_cli_get_argument_uint16(arguments, 2);
  uint8_t destEndpoint      = sl_cli_get_argument_uint8(arguments, 3);
  sl_zigbee_aps_option_t options  = sl_cli_get_argument_uint16(arguments, 4);
  sl_802154_short_addr_t nwkBroadcast = DEFAULT_MULTICAST_NWK_BROADCAST_ADDRESS;
  if (command[4] == 'g' || sl_cli_get_argument_count(arguments) >= 6) {
    nwkBroadcast = sl_cli_get_argument_uint16(arguments, 5);
  }
  uint16_t clusterId = (command[4] == 'b'
                        ? CLUSTER_BUFFER_TEST_REQUEST
                        : (command[4] == 'g'
                           ? CLUSTER_BUFFER_GROUP_REQUEST
                           : CLUSTER_FREEFORM_MSG_REQUEST));

  sl_status_t status = SL_STATUS_FAIL;

  // Just to make life easier, force group addressing for this command.
  if (clusterId == CLUSTER_BUFFER_GROUP_REQUEST) {
    options = ((options & ~ZIGBEE_APS_FRAME_CONTROL_MODE_MASK)
               | ZIGBEE_APS_FRAME_CONTROL_MODE_MULTICAST);
  }

  status = transmitMessage(destination,
                           1,
                           &data,
                           sourceEndpoint,
                           destEndpoint,
                           clusterId,
                           NULL,
                           options,
                           nwkBroadcast);

  printCommandStatus(status, NULL, "Failed to send");
}
void msgCommandPayload(SL_CLI_COMMAND_ARG)
{
  sl_zigbee_aps_frame_t outApsFrame;

  uint8_t data                    = sl_cli_get_argument_uint8(arguments, 0);
  outApsFrame.sourceEndpoint      = sl_cli_get_argument_uint8(arguments, 1);
  sl_802154_short_addr_t destination = sl_cli_get_argument_uint16(arguments, 2);
  outApsFrame.destinationEndpoint = sl_cli_get_argument_uint8(arguments, 3);
  outApsFrame.clusterId           = CLUSTER_BUFFER_TEST_REQUEST;
  outApsFrame.profileId           = testProfileId;
  outApsFrame.options             = sl_cli_get_argument_uint16(arguments, 4);

  #define MAX_MSG_COMMAND_LENGTH 251
  bool success = false;
  uint8_t message[MAX_MSG_COMMAND_LENGTH];

  if (data >= 250) { //cannot be more than 256 bytes
    success = false;
  } else {
    uint16_t messageTag;
    message[0] = data;
    for (int i = 0; i < data; i++) {
      message[i + 1] = i;
    }
    success =  (((maxMessageLength == DONT_FRAGMENT
                  || message[0] + 1 < maxMessageLength)
                 ? sl_zigbee_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                          destination,
                                          &outApsFrame,
                                          0x00,        // tag
                                          data + 1,
                                          (uint8_t*) message,
                                          NULL)
                 : sli_zigbee_af_fragmentation_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                                            destination,
                                                            &outApsFrame,
                                                            (uint8_t*) message,
                                                            data + 1,
                                                            &messageTag))
                == SL_STATUS_OK);
    sl_legacy_buffer_manager_release_message_buffer(message);
  }

  if (!success) {
    printCommandStatus(SL_STATUS_FAIL, NULL, "Failed to send");
  }
}
//------------------------------------------------------------------------------

void spoofedBroadcastMsgCommand(SL_CLI_COMMAND_ARG)
{
  uint8_t length;
  uint8_t *command = sl_zigbee_cli_get_argument_string_and_length(arguments, -1, &length);
  uint8_t data              = sl_cli_get_argument_uint8(arguments, 0);
  sl_802154_short_addr_t source = sl_cli_get_argument_uint16(arguments, 1);
  uint8_t sourceEndpoint    = sl_cli_get_argument_uint8(arguments, 2);
  sl_802154_short_addr_t destination = sl_cli_get_argument_uint16(arguments, 3);
  uint8_t destEndpoint      = sl_cli_get_argument_uint8(arguments, 4);
  sl_zigbee_aps_option_t options  = sl_cli_get_argument_uint16(arguments, 5);
  uint8_t seq    = sl_cli_get_argument_uint8(arguments, 6);
  uint16_t clusterId = (command[4] == 'b'
                        ? CLUSTER_BUFFER_TEST_REQUEST
                        : (command[4] == 'g'
                           ? CLUSTER_BUFFER_GROUP_REQUEST
                           : CLUSTER_FREEFORM_MSG_REQUEST));

  sl_status_t status;

  // Just to make life easier, force group addressing for this command.
  if (clusterId == CLUSTER_BUFFER_GROUP_REQUEST) {
    options = ((options & ~ZIGBEE_APS_FRAME_CONTROL_MODE_MASK)
               | ZIGBEE_APS_FRAME_CONTROL_MODE_MULTICAST);
  }

  sl_zigbee_aps_frame_t apsFrame;

  apsFrame.sourceEndpoint = sourceEndpoint;
  apsFrame.destinationEndpoint = destEndpoint;
  apsFrame.clusterId = clusterId;
  apsFrame.profileId = profileIdForInitiatedMessages;
  apsFrame.options = options;
  status = sl_zigbee_send_broadcast(source,
                                    destination,
                                    seq,
                                    &apsFrame,
                                    broadcastRadius,
                                    0x00,        // tag
                                    1,
                                    &data,
                                    NULL);

  printCommandStatus(status, NULL, "Failed to send proxied broadcast message");
}

//------------------------------------------------------------------------------

void sendPacketsCommand(SL_CLI_COMMAND_ARG)
{
  uint8_t nwkIndex = sl_zigbee_get_current_network();
  uint8_t i;

  pktLength[nwkIndex]         = sl_cli_get_argument_uint8(arguments, 0);
  pktTotalCount[nwkIndex]     = sl_cli_get_argument_uint16(arguments, 1);
  pktInterval[nwkIndex]       = sl_cli_get_argument_uint16(arguments, 2);
  pktMaxInFlight[nwkIndex]    = sl_cli_get_argument_uint8(arguments, 3);
  pktSourceEndpoint[nwkIndex] = sl_cli_get_argument_uint8(arguments, 4);
  pktDestination[nwkIndex]    = sl_cli_get_argument_uint16(arguments, 5);
  pktDestEndpoint[nwkIndex]   = sl_cli_get_argument_uint8(arguments, 6);
  pktOptions[nwkIndex]        = sl_cli_get_argument_uint16(arguments, 7);

  pktRunningCount[nwkIndex] = 0;
  pktInFlight[nwkIndex] = 0;
  pktSendStartTime[nwkIndex] = halCommonGetInt32uMillisecondTick();
  totalBytesSent[nwkIndex] = 0;
  pktSuccessCount[nwkIndex] = 0;
  pktMinSendTime[nwkIndex] = 0xFFFFFFFF;
  pktMaxSendTime[nwkIndex] = 0;
  pktSendTimeSum[nwkIndex] = 0;

  for (i = 0; i < MAX_IN_FLIGHT_PACKETS; i++) {
    pktInFlightInfo[nwkIndex][i].inUse = false;
  }

  ongoingSendingPackets = true;

  sendEventHandler(nwkIndex);
}

//------------------------------------------------------------------------------

void sendPacketCommand(SL_CLI_COMMAND_ARG)
{
  static uint16_t count = 0;
  uint8_t length = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t sourceEndpoint = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t destEndpoint = sl_cli_get_argument_uint8(arguments, 2);
  sl_802154_short_addr_t destination = sl_cli_get_argument_uint16(arguments, 3);
  uint16_t apsOptions = sl_cli_get_argument_uint16(arguments, 4);

  uint8_t message[MAX_TRANSMIT_MESSAGE_PACKET_LENGTH];

  message[0] = length;
  message[1] = LOW_BYTE(count);
  message[2] = HIGH_BYTE(count);
  count++;
  transmitMessage(destination,
                  3,
                  message,
                  sourceEndpoint,
                  destEndpoint,
                  CLUSTER_COUNTED_PACKETS,
                  NULL,
                  apsOptions,
                  DEFAULT_MULTICAST_NWK_BROADCAST_ADDRESS);
}

//------------------------------------------------------------------------------

void packetCountCommand(SL_CLI_COMMAND_ARG)
{
  uint8_t length;
  uint8_t *command = sl_zigbee_cli_get_argument_string_and_length(arguments, -1, &length);
  uint8_t sourceEndpoint    = sl_cli_get_argument_uint8(arguments, 0);
  sl_802154_short_addr_t destination = sl_cli_get_argument_uint16(arguments, 1);
  uint8_t destEndpoint      = sl_cli_get_argument_uint8(arguments, 2);
  sl_zigbee_aps_option_t options = sl_cli_get_argument_uint16(arguments, 3);
  uint8_t clusterId = command[0] == 'r'
                      ? CLUSTER_RESET_PACKET_COUNT
                      : CLUSTER_RETRIEVE_PACKET_COUNT;
  uint8_t message[1] = { 0 };
  transmitMessage(destination,
                  0,
                  message,
                  sourceEndpoint,
                  destEndpoint,
                  clusterId,
                  NULL,
                  options,
                  DEFAULT_MULTICAST_NWK_BROADCAST_ADDRESS);
}

//------------------------------------------------------------------------------

void enableRxPrintCommand(SL_CLI_COMMAND_ARG)
{
  sli_zigbee_print_incoming_packet_enabled = (sl_cli_get_argument_uint32(arguments, 0) > 0);

  sl_zigbee_core_debug_println("Print RX messages %s",
                               (sli_zigbee_print_incoming_packet_enabled ? "enabled" : "disabled"));
}

//------------------------------------------------------------------------------

void setRadiusCommand(SL_CLI_COMMAND_ARG)
{
  broadcastRadius = sl_cli_get_argument_uint8(arguments, 0);
  broadcastAddr = sl_cli_get_argument_uint8(arguments, 1);
}

// NOTE default fragment size is configurable?
#define SL_ZIGBEE_TEST_PROFILE_SET_FRAGMENT_MAX_TRANSFER_SIZE_ON 256
#define SL_ZIGBEE_TEST_PROFILE_SET_FRAGMENT_MAX_TRANSFER_SIZE_OFF 128
// #define SL_ZIGBEE_TEST_PROFILE_DEFAULT_SUPPORTED_FRAGMENTATION_SIZE 256
void setFragmentsCommand(SL_CLI_COMMAND_ARG)
{
  uint8_t maxTotalMessageLength      = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t windowSize                 = sl_cli_get_argument_uint8(arguments, 2);
  uint8_t fragmentDelayMs            = sl_cli_get_argument_uint8(arguments, 3);
  uint8_t missedBlocks               = sl_cli_get_argument_uint8(arguments, 4);
  // For the onboard, this will be an sl_status_t.
  // For the host, this will be an sl_zigbee_ezsp_status_t.
  uint8_t status = initializeFragmentation(maxTotalMessageLength,
                                           windowSize,
                                           fragmentDelayMs,
                                           missedBlocks);
  printCommandStatus(status,
                     NULL,  // success message
                     "Frag. param failure");

  // Common to all platforms.  Stored in the application.
  maxFragmentLength = sl_cli_get_argument_uint8(arguments, 1);

  sl_zigbee_node_descriptor_info_t descriptor;
  descriptor.capability = 0xFF;
  // set the stack maximums to account for larger payloads
  if (maxTotalMessageLength != DONT_FRAGMENT) {
    descriptor.max_incoming_transfer_size = SL_ZIGBEE_TEST_PROFILE_SET_FRAGMENT_MAX_TRANSFER_SIZE_ON;
    descriptor.max_outgoing_transfer_size = SL_ZIGBEE_TEST_PROFILE_SET_FRAGMENT_MAX_TRANSFER_SIZE_ON;
  } else {
    descriptor.max_incoming_transfer_size = SL_ZIGBEE_TEST_PROFILE_SET_FRAGMENT_MAX_TRANSFER_SIZE_OFF;
    descriptor.max_outgoing_transfer_size = SL_ZIGBEE_TEST_PROFILE_SET_FRAGMENT_MAX_TRANSFER_SIZE_OFF;
  }
  (void) sl_zigbee_set_node_descriptor(&descriptor);
}

void changeTxMode(SL_CLI_COMMAND_ARG)
{
  uint8_t mode    = sl_cli_get_argument_uint8(arguments, 0); //signal could be 1: grant a request, or 0: deny a request
  setTxMode(mode);
}

//------------------------------------------------------------------------------
// This function handles all Test Profile Cluster Requests / Responses.
// If the cluster was recognized and handled successfully it will return true.
// Otherwise it returns false.

bool testProfileMessageHandler(sl_802154_short_addr_t sender,
                               sl_zigbee_aps_frame_t *apsFrame,
                               uint8_t messageLength,
                               uint8_t *message)
{
  bool success = false;
  uint8_t i = 0;

  if ((messageLength == 0)
      && (apsFrame->clusterId != CLUSTER_FREEFORM_NO_DATA_RESPONSE)
      && (apsFrame->clusterId != CLUSTER_RESET_PACKET_COUNT)
      && (apsFrame->clusterId != CLUSTER_RETRIEVE_PACKET_COUNT)) {
    if (sli_zigbee_print_incoming_packet_enabled) {
      sl_zigbee_core_debug_println("Empty test profile message received, dropping.");
    }
    return false;
  }

  switch (apsFrame->clusterId) {
    case CLUSTER_BUFFER_TEST_REQUEST:
    case CLUSTER_BUFFER_GROUP_REQUEST: {
      uint16_t clusterId = apsFrame->clusterId + (CLUSTER_BUFFER_TEST_RESPONSE
                                                  - CLUSTER_BUFFER_TEST_REQUEST);
      uint8_t sequenceLength = message[0];
      #define MAX_TEST_RESPONSE_LENGTH 252
      uint8_t response[MAX_TEST_RESPONSE_LENGTH];
      if (sequenceLength + 2 >= MAX_TEST_RESPONSE_LENGTH) {
        success = false;
      } else {
        response[0] = sequenceLength;
        response[1] = 0; // status: success
        for (i = 0; i < sequenceLength; i++) {
          response[i + 2] = i;
        }
        success = sendResponse(sender, apsFrame, clusterId, sequenceLength + 2, (uint8_t*)response);
      }
      if (!success) {
        // XXX:  The message could have been too large to send, or we could
        // have encountered some other problem.
        // The application does not enumerate this properly.
        response[0] = sequenceLength;
        response[1] = 1; // status: fail
        sendResponse(sender, apsFrame, clusterId, 2, (uint8_t*)response);
      } else {
        if (sli_zigbee_print_incoming_packet_enabled) {
          sl_zigbee_core_debug_println("Responded to buffer request, length %02X, status %02X",
                                       sequenceLength,
                                       success ? 0 : 1);
        }
      }
      break;
    }

    case CLUSTER_FREEFORM_MSG_REQUEST: {
    #define MAX_CLUSTER_FREEFORM_MSG_RESPONSE_LENGTH 7
      uint8_t response[MAX_CLUSTER_FREEFORM_MSG_RESPONSE_LENGTH] = { 0 };
      uint8_t responseLength = 0;
      uint8_t requestType = message[0];

      switch (requestType) {
        case REQUEST_TYPE_8_BIT_INT:
          responseLength = 2;
          response[0] = 0x00;
          response[1] = 0x42;
          break;
        case REQUEST_TYPE_CHAR_STRING:
          responseLength = 7;
          response[0] = 0x01;
          response[1] = 0x5A;
          response[2] = 0x69;
          response[3] = 0x67;
          response[4] = 0x42;
          response[5] = 0x65;
          response[6] = 0x65;
          break;
        case REQUEST_TYPE_COORDINATES:
          responseLength = 5;
          response[0] = 0x02;
          response[1] = 0x12;
          response[2] = 0x34;
          response[3] = 0x56;
          response[4] = 0x78;
          break;
        case REQUEST_TYPE_16_BIT_INT:
          responseLength = 3;
          response[0] = 0x03;
          response[1] = 0xCD;
          response[2] = 0xAB;
          break;
        case REQUEST_TYPE_NO_DATA:
          responseLength = 0;
          break;
        case REQUEST_TYPE_RELATIVE_TIME:
          responseLength = 5;
          response[0] = 0x05;
          response[1] = 0x78;
          response[2] = 0x56;
          response[3] = 0x34;
          response[4] = 0x12;
          break;
        case REQUEST_TYPE_ABSOLUTE_TIME:
          responseLength = 5;
          response[0] = 0x06;
          response[1] = 0x0F;
          response[2] = 0xDE;
          response[3] = 0xBC;
          response[4] = 0x9A;
          break;
        default: {
        }
      } // close switch.

      sendResponse(sender,
                   apsFrame,
                   (requestType == REQUEST_TYPE_NO_DATA
                    ? CLUSTER_FREEFORM_NO_DATA_RESPONSE
                    : CLUSTER_FREEFORM_MSG_RESPONSE),
                   responseLength,
                   (uint8_t*)response);
      if (sli_zigbee_print_incoming_packet_enabled) {
        sl_zigbee_core_debug_println("Responded to freeform request, type %02X",
                                     requestType);
      }

      break;
    }

    case CLUSTER_BUFFER_TEST_RESPONSE:
    case CLUSTER_BUFFER_GROUP_RESPONSE: {
      uint8_t sequenceLength = message[0];
      uint8_t status = message[1];
      uint8_t j;
      bool sequenceError = false;
      if (status == 0) {
        for (j = 0; j < sequenceLength; j++) {
          if (message[2 + j] != j) {
            sequenceError = true;
            break;
          }
        }
      }
      if (sequenceError) {
        if (sli_zigbee_print_incoming_packet_enabled) {
          sl_zigbee_core_debug_println("%ssequence error at %02X, length %02X, status %02X",
                                       "Buffer ",
                                       j, sequenceLength, status);
        }
      } else {
        if (sli_zigbee_print_incoming_packet_enabled) {
          sl_zigbee_core_debug_println("%s%slength %02X, status %02X",
                                       "Buffer ",
                                       "response: ",
                                       sequenceLength, status);
        }
      }
      break;
    }

    case CLUSTER_FREEFORM_MSG_RESPONSE: {
      uint8_t requestType = message[0];
      if (sli_zigbee_print_incoming_packet_enabled) {
        sl_zigbee_core_debug_print("Freeform %stype %02X, data: ",
                                   "response: ",
                                   requestType);
      }
      for (i = 1; i < messageLength; i++) {
        sl_zigbee_core_debug_print("%02X ", message[i]);
        (void) sli_legacy_serial_wait_send(serialPort);
      }
      printCarriageReturn();
      break;
    }
    case CLUSTER_COUNTED_PACKETS: {
      uint16_t counter = HIGH_LOW_TO_INT(message[2], message[1]);
      if (sli_zigbee_print_incoming_packet_enabled) {
        sl_zigbee_core_debug_println("Counter %u", counter);
      }
      if (counter > pktLastRxCounter || pktRxReset || gCountPacketsOutOfOrder) {
        pktRxReset = false;
        pktRxCount++;
        pktLastRxCounter = counter;
      }
      break;
    }
    case CLUSTER_RESET_PACKET_COUNT: {
      pktRxReset = true;
      pktRxCount = 0;
      pktLastRxCounter = 0;
      if (sli_zigbee_print_incoming_packet_enabled) {
        sl_zigbee_core_debug_println("Reset packet count");
      }
      break;
    }
    case CLUSTER_RETRIEVE_PACKET_COUNT: {
      uint8_t response[2] = { LOW_BYTE(pktRxCount), HIGH_BYTE(pktRxCount) };
      sendResponse(sender, apsFrame, CLUSTER_PACKET_COUNT_RESPONSE, 2, (uint8_t*)response);

      if (sli_zigbee_print_incoming_packet_enabled) {
        sl_zigbee_core_debug_println("Reporting %u %s (last counter %u)",
                                     pktRxCount,
                                     "packets",
                                     pktLastRxCounter);
      }
      break;
    }
    case CLUSTER_PACKET_COUNT_RESPONSE: {
      uint16_t counter = HIGH_LOW_TO_INT(message[1], message[0]);
      if (sli_zigbee_print_incoming_packet_enabled) {
        sl_zigbee_core_debug_println("Packet count %s%u %s",
                                     "response: ",
                                     counter,
                                     "packets");
      }
      break;
    }
    case CLUSTER_GENERIC_MESSAGE: {
      if (sli_zigbee_print_incoming_packet_enabled) {
        sl_zigbee_core_debug_println("Generic msg received");
      }
      break;
    }
#ifdef SL_CATALOG_SIMPLE_LED_LED1_PRESENT
    case CLUSTER_BOARD_LED_CONTROL:
      if (message[0]) {
        sl_led_turn_on(&sl_led_led1);
      } else {
        sl_led_turn_off(&sl_led_led1);
      }
      break;
#endif // SL_CATALOG_SIMPLE_LED_LED1_PRESENT
    default: {
    }
  } //close switch.

  return success;
}

//------------------------------------------------------------------------------
// This should be called from the sl_zigbee_message_sent_handler so that the test
// profile can update some internal state used by the 'send_packets' command.

void testProfileMessageSentHandler(sl_zigbee_outgoing_message_type_t type,
                                   sl_zigbee_aps_frame_t *apsFrame,
                                   sl_status_t status)
{
  UNUSED_VAR(type);
  uint8_t nwkIndex = sl_zigbee_get_callback_network();
  uint32_t nowMs = halCommonGetInt32uMillisecondTick();
  uint32_t sendTimeMs = 0xFFFFFFFF;

  if (apsFrame->clusterId == CLUSTER_COUNTED_PACKETS
      && ongoingSendingPackets) {
    uint8_t i;
    pktInFlight[nwkIndex]--;

    // Search for a matching entry and compute the send time.
    for (i = 0; i < MAX_IN_FLIGHT_PACKETS; i++) {
      if (pktInFlightInfo[nwkIndex][i].inUse
          && pktInFlightInfo[nwkIndex][i].apsSeqn == apsFrame->sequence) {
        sendTimeMs = elapsedTimeInt32u(pktInFlightInfo[nwkIndex][i].sendTimeMs,
                                       nowMs);
        pktInFlightInfo[nwkIndex][i].inUse = false;
        break;
      }
    }

    if (((apsFrame->options & SL_ZIGBEE_APS_OPTION_FRAGMENT) !=  SL_ZIGBEE_APS_OPTION_FRAGMENT)
        && status == SL_STATUS_OK && ongoingSendingPackets) {
      totalBytesSent[nwkIndex] += pktLength[nwkIndex];
      pktSuccessCount[nwkIndex]++;

      assert(sendTimeMs < 0xFFFFFFFF);
      pktSendTimeSum[nwkIndex] += sendTimeMs;

      if (pktMinSendTime[nwkIndex] > sendTimeMs) {
        pktMinSendTime[nwkIndex] = sendTimeMs;
      }

      if (pktMaxSendTime[nwkIndex] < sendTimeMs) {
        pktMaxSendTime[nwkIndex] = sendTimeMs;
      }
    }

    if (pktRunningCount[nwkIndex] == pktTotalCount[nwkIndex]
        && pktInFlight[nwkIndex] == 0) {
      ongoingSendingPackets = false;
      printPktsSendStats(nwkIndex);
    }
  }
}

//------------------------------------------------------------------------------
// This Application Event handles periodically transmitting messages
// for the 'send_packets' command.  The application must include 'sendEvent'
// in the application events they declare.

static void sendEventHandler(uint8_t nwkIndex)
{
  UNUSED uint8_t savedNetworkIndex = sl_zigbee_get_current_network();

  assert(nwkIndex < SL_ZIGBEE_SUPPORTED_NETWORKS);

  uint8_t max_num_packets_in_flight = sizeof(pktMaxInFlight) / sizeof(pktMaxInFlight[0]);
  uint8_t num_packets_in_flight = sizeof(pktInFlight) / sizeof(pktInFlight[0]);
  if ((nwkIndex >= max_num_packets_in_flight) || (nwkIndex >= num_packets_in_flight)) {
    return;
  }

  if (pktMaxInFlight[nwkIndex]
      && pktInFlight[nwkIndex] >= pktMaxInFlight[nwkIndex]) {
    // Max in flight limit reached, wait and try again.
    sl_zigbee_af_event_set_delay_ms(sendEvent[nwkIndex], 1);
    return;
  }

  uint8_t message[MAX_TRANSMIT_MESSAGE_PACKET_LENGTH];

  uint8_t i;
  uint8_t apsSeqn;
  bool success;
  message[0] = pktLength[nwkIndex];
  message[1] = LOW_BYTE(pktRunningCount[nwkIndex]);
  message[2] = HIGH_BYTE(pktRunningCount[nwkIndex]);
  for (i = 0; i < pktLength[nwkIndex] - 2; i++) {
    message[i + 3] = i;
  }
  (void) sl_zigbee_set_current_network(nwkIndex);

  success = (SL_STATUS_OK
             == transmitMessage(pktDestination[nwkIndex],
                                pktLength[nwkIndex] + 4,
                                (uint8_t*)message,
                                pktSourceEndpoint[nwkIndex],
                                pktDestEndpoint[nwkIndex],
                                CLUSTER_COUNTED_PACKETS,
                                &apsSeqn,
                                pktOptions[nwkIndex],
                                DEFAULT_MULTICAST_NWK_BROADCAST_ADDRESS));

  (void) sl_zigbee_set_current_network(savedNetworkIndex);
  if (success) {
    // Save the time we submitted the packet to the stack (if we have an
    // available entry).
    uint8_t i;
    for (i = 0; i < MAX_IN_FLIGHT_PACKETS; i++) {
      if (!pktInFlightInfo[nwkIndex][i].inUse) {
        pktInFlightInfo[nwkIndex][i].inUse = true;
        pktInFlightInfo[nwkIndex][i].apsSeqn = apsSeqn;
        pktInFlightInfo[nwkIndex][i].sendTimeMs =
          halCommonGetInt32uMillisecondTick();
        break;
      }
    }

    pktRunningCount[nwkIndex]++;
    pktInFlight[nwkIndex]++;
  }

  if (pktTotalCount[nwkIndex] > 0
      && pktRunningCount[nwkIndex] >= pktTotalCount[nwkIndex]) {
    sl_zigbee_af_event_set_inactive(sendEvent[nwkIndex]);
    sl_zigbee_core_debug_println("Done sending %u %s",
                                 pktTotalCount[nwkIndex],
                                 "packets");
  } else {
    sl_zigbee_af_event_set_delay_ms(sendEvent[nwkIndex], pktInterval[nwkIndex]);
  }
}

static void printPktsSendStats(uint8_t nwkIndex)
{
  uint32_t nowMs = halCommonGetInt32uMillisecondTick();
  uint32_t totalSendTimeMs = elapsedTimeInt32u(pktSendStartTime[nwkIndex],
                                               nowMs);
  uint32_t throughput = (totalBytesSent[nwkIndex] * 8 * 1000 / totalSendTimeMs);

  sl_zigbee_core_debug_println("Total time %lums", (unsigned long)totalSendTimeMs);
  sl_zigbee_core_debug_println("Success packets: %lu out of %u",
                               (unsigned long)pktSuccessCount[nwkIndex],
                               pktTotalCount[nwkIndex]);

  if (pktSuccessCount[nwkIndex] == 0) {
    return;
  }

  sl_zigbee_core_debug_println("Avg. send time %ums",
                               pktSendTimeSum[nwkIndex] / pktSuccessCount[nwkIndex]);
  sl_zigbee_core_debug_println("Min send time %ums", pktMinSendTime[nwkIndex]);
  sl_zigbee_core_debug_println("Max send time %ums", pktMaxSendTime[nwkIndex]);
  sl_zigbee_core_debug_println("Throughput: %u bits/s", throughput);
}

void sendEventHandler0(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sendEventHandler(0);
}

void sendEventHandler1(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sendEventHandler(1);
}

void sendEventHandler2(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sendEventHandler(2);
}

void sendEventHandler3(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sendEventHandler(3);
}
