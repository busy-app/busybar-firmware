/***************************************************************************//**
 * @file
 * @brief ZigBee 3.0 zdo test harness functionality
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

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#if defined(SL_COMPONENT_ZIGBEE_EZSP_PRESENT)
#include "app/util/zigbee-framework/zigbee-device-host.h"
#endif // SL_COMPONENT_ZIGBEE_EZSP_PRESENT

#include "app/util/zigbee-framework/zigbee-device-common.h"

#include "test-harness-z3-core.h"

// -----------------------------------------------------------------------------
// Constants
#define TEST_HARNESS_ENDPOINT (1)
#define ZDO_NETWORK_UPDATE_REQUEST (0x0038)
#define ZDO_NODE_DESCRIPTOR_BAND_2400_MHZ (0x40)

#define TH_SIMPLE_DESC_REQ_PAYLOAD_LENGTH (4)
#define TH_USER_DESC_REQ_PAYLOAD_LENGTH (3)
#define TH_IEEE_ADD_REQ_PAYLOAD_LENGTH (3)
#define SERVER_MASK_HIGH (0x2C)  // r22
#define TH_USER_DESC_RSP_PAYLOAD_LENGTH (16)

// Clusters for simple descriptor response
// server
#define IN_CLUSTERS { 0x0000, 0x0003, 0x0006, 0x0008, 0x000a, 0x0019, 0x0020, 0x0025, 0x0101 }
// client
#define OUT_CLUSTERS { 0x0006, 0x0008, 0x0019, 0x0101, 0x0102, 0x0103 }
// -----------------------------------------------------------------------------
// extern functions
extern bool sli_zigbee_am_network_manager(void);

// -----------------------------------------------------------------------------
// Globals
sl_zigbee_af_event_t sl_zigbee_af_test_harness_z3_zdo_send_event_control;

#define NULL_CURRENT_ZDO_NEGATIVE_COMMAND_CLUSTER (0x00FF)
static sl_802154_short_addr_t currentZdoNegativeCommandDestination = SL_ZIGBEE_NULL_NODE_ID;
static uint16_t currentZdoNegativeCommandCluster = NULL_CURRENT_ZDO_NEGATIVE_COMMAND_CLUSTER;
static uint8_t currentZdoNegativeCommandFrame[64];
static uint8_t currentZdoNegativeCommandFrameLength;

uint8_t sli_zigbee_af_test_harness_z3_server_mask_high = SERVER_MASK_HIGH;

// Internal stack API.
uint8_t sli_zigbee_stack_get_nwk_update_id(void);

// -----------------------------------------------------------------------------
// ZDO Commands

// Declared in zdo-cli.c. Used here so CLI commands can be reused.
extern uint16_t sli_zigbee_af_cli_zdo_in_clusters[];
extern uint16_t sli_zigbee_af_cli_zdo_out_clusters[];
extern uint8_t sli_zigbee_af_cli_in_cl_count;
extern uint8_t sli_zigbee_af_cli_out_cl_count;

static uint32_t negativeZdoCommandFlags = 0;
#define NEGATIVE_ZDO_COMMAND_FLAGS_OUR_NWK_ADDRESS        BIT32(0)
#define NEGATIVE_ZDO_COMMAND_FLAGS_MATCH_LIST_ABSENT      BIT32(1)
#define NEGATIVE_ZDO_COMMAND_FLAGS_SEND_POSITIVE_RESPONSE BIT32(2)
#define NEGATIVE_ZDO_COMMAND_FLAGS_NO_SIMPLE_DESCRIPTOR   BIT32(3)
#define NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_KE        BIT32(4)
#define NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_SWAP      BIT32(5)
#define NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_ABSENT    BIT32(6)
#define NEGATIVE_ZDO_COMMAND_FLAGS_NO_ENDPOINT_FIELDS     BIT32(7)
#define NEGATIVE_ZDO_COMMAND_FLAGS_NO_NWK_ADDR            BIT32(8)
#define NEGATIVE_ZDO_COMMAND_FLAGS_NO_NEGATIVE_RESPONSE   BIT32(9)
#define NEGATIVE_ZDO_COMMAND_FLAGS_NO_USER_DESCRIPTOR     BIT32(10)
#define NEGATIVE_ZDO_COMMAND_FLAGS_NO_NODE_DESCRIPTOR     BIT32(11)

#ifdef EZSP_HOST

static sl_status_t sendZdoCommand(sl_802154_short_addr_t destination,
                                  uint16_t clusterId,
                                  uint8_t *frame,
                                  uint8_t frameLength,
                                  uint8_t sequenceNumber)
{
  return SL_STATUS_INVALID_STATE;
}

#else

extern sl_802154_short_addr_t sli_zigbee_current_sender;

static sl_status_t sendZdoCommand(sl_802154_short_addr_t destination,
                                  uint16_t clusterId,
                                  uint8_t *frame,
                                  uint8_t frameLength,
                                  uint8_t sequenceNumber)
{
  sl_status_t status;
  sl_zigbee_aps_frame_t apsFrame = {
    SL_ZIGBEE_ZDO_PROFILE_ID,
    clusterId,
    SL_ZIGBEE_ZDO_ENDPOINT,
    SL_ZIGBEE_ZDO_ENDPOINT,
    (SL_ZIGBEE_APS_OPTION_RETRY),
    0, // group id
    0, // sequence
    0  // radius
  };
  frame[0] = sequenceNumber ? sequenceNumber : sl_zigbee_next_zig_dev_request_sequence();
  sli_buffer_manager_buffer_t message = sl_legacy_buffer_manager_fill_linked_buffers(frame, frameLength);

  if (message == SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  if (sl_zigbee_is_zigbee_broadcast_address(destination)) {
    status = sl_zigbee_send_broadcast(SL_ZIGBEE_NULL_NODE_ID,
                                      destination,
                                      0,
                                      &apsFrame,
                                      SL_ZIGBEE_MAX_HOPS,
                                      0x00, // tag
                                      frameLength,
                                      frame,
                                      NULL);
  } else {
    status = sl_zigbee_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                    destination,
                                    &apsFrame,
                                    0x00, // tag
                                    frameLength,
                                    frame,
                                    NULL);
  }

  return status;
}

#endif /* EZSP_HOST */
// plugin test-harness z3 mgmt permit-joining-req <timeS:2> <dstShort:2> <options:4>
void sli_zigbee_af_test_harness_z3_mgmt_permit_joining_req_command(sl_cli_command_arg_t *arguments)
{
  uint16_t permitDurationS = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_short_addr_t destination  = sl_cli_get_argument_uint16(arguments, 1);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 2);

  sl_status_t status;

  status = sl_zigbee_permit_joining_request(destination,
                                            permitDurationS,
                                            (options == BIT32(0) ? false : true),
                                            (SL_ZIGBEE_APS_OPTION_RETRY));

  sl_zigbee_af_core_println("%s: %s: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Permit joining request",
                            status);
}

// plugin test-harness z3 mgmt leave <dstShort:2> <removeChildren:1>
// <rejoin:1> <optionBitmask:4>
void sli_zigbee_af_test_harness_z3_mgmt_leave_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t destination = sl_cli_get_argument_uint16(arguments, 0);
  // CCB 2047
  // - CCB makes the first step to depracate the 'leave and remove children' functionality.
  // - We were proactive here and deprecated it right away.
  // bool removeChildren     = (bool)sl_zigbee_unsigned_command_argument(1);

  bool rejoin             = (bool)sl_cli_get_argument_uint32(arguments, 2);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 3);

  sl_status_t status = SL_STATUS_INVALID_STATE;
  uint8_t frame[10];
  uint8_t *finger = &frame[1];

  // Long address of destination. Bit 0 means don't put it in payload.
  if (options != BIT(0)) {
    status = sl_zigbee_lookup_eui64_by_node_id(destination, finger);
    if (status != SL_STATUS_OK) {
      goto done;
    }
    finger += EUI64_SIZE;
  }

  // Options.
  if (options != BIT(1)) {
    *finger = 0;
    if (rejoin) {
      *finger |= BIT(7);
    }
    finger++;
  }
  status = sendZdoCommand(destination, LEAVE_REQUEST, frame, finger - &frame[0], 0);

  done:
  sl_zigbee_af_core_println("%s: %s: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Mgmt leave",
                            status);
}

// plugin test-harness z3 mgmt nwk-update-request <scanChannel:2> <scanDuration:2>
// <scanCount:1> <dstShort:2> <option:4>
void sli_zigbee_af_test_harness_z3_mgmt_nwk_update_request_command(sl_cli_command_arg_t *arguments)
{
  uint16_t scanChannel    = sl_cli_get_argument_uint16(arguments, 0);
  uint16_t scanDuration   = sl_cli_get_argument_uint16(arguments, 1);
  uint8_t scanCount       = sl_cli_get_argument_uint8(arguments, 2);
  sl_802154_short_addr_t destination = sl_cli_get_argument_uint16(arguments, 3);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 4);

  sl_status_t status;
  uint8_t frame[9];
  uint8_t *finger = &frame[1];
  sl_zigbee_network_parameters_t networkParameters;
  sl_zigbee_node_type_t nodeType;

  status = sl_zigbee_af_get_network_parameters(&nodeType, &networkParameters);
  if (status != SL_STATUS_OK) {
    goto done;
  }

  // Scan channel.
  sl_zigbee_af_copy_int32u(finger, 0, BIT32(scanChannel));
  finger += sizeof(uint32_t);
  // Scan duration.
  *finger++ = (uint8_t)(0xFF & scanDuration); // ANDREW: Q3.
  // Scan count (table 2.90):
  // "This field shall be present only if the ScanDuration is within the
  // range of 0x00 to 0x05."
  if (scanDuration <= 5) {
    *finger++ = scanCount;
  }
  // Network update id (table 2.90):
  // "This field shall only be present of the ScanDuration is 0xfe or 0xff.
  // If the ScanDuration is 0xff, then the value in the nwkUpdateID shall
  // be ignored." (But it should still be present, unchanged.)

  if (scanDuration == 0xFE || scanDuration == 0xFF) {
    *finger++ = scanDuration == 0xFF
                ? networkParameters.nwkUpdateId
                : options == BIT(1)
                ? 0x55
                : options == BIT(0)
                ? networkParameters.nwkUpdateId - 1
                : networkParameters.nwkUpdateId + 1;
  }
  // Network manager id (table 2.90):
  // "This field shall be present only if the ScanDuration is set to 0xff,
  // and, where present, indicates the NWK address for the device with the
  // Network Manager bit set in its Node Descriptor."
  if (scanDuration == 0xFF) {
    sl_zigbee_af_copy_int16u(finger, 0, networkParameters.nwkManagerId);
    finger += sizeof(uint16_t);
  }

  status = sendZdoCommand(destination,
                          ZDO_NETWORK_UPDATE_REQUEST,
                          frame,
                          finger - &frame[0],
                          0);

  done:
  sl_zigbee_af_core_println("%s: %s: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Network update request",
                            status);
}

// plugin test-harness z3 zdo zdo-reset
void sli_zigbee_af_test_harness_z3_zdo_zdo_reset_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  currentZdoNegativeCommandCluster = NULL_CURRENT_ZDO_NEGATIVE_COMMAND_CLUSTER;
  negativeZdoCommandFlags = 0;

  sl_zigbee_af_core_println("%s: %s: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "ZDO reset",
                            SL_STATUS_OK);
}

// plugin test-harness z3 zdo node-desc-req <dstShort:2>
void sli_zigbee_af_test_harness_z3_zdo_node_desc_req_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t destination       = sl_cli_get_argument_uint16(arguments, 0);
  sl_status_t status;
  uint8_t frame[1];

  status = sendZdoCommand(destination,
                          NODE_DESCRIPTOR_REQUEST,
                          frame,
                          1, 0);

  sl_zigbee_af_core_println("%s: %s: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Node descriptor request",
                            status);
}

// plugin test-harness z3 zdo zdo-node-desc-rsp-config <options:4>
void sli_zigbee_af_test_harness_z3_zdo_zdo_node_desc_rsp_config_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status = SL_STATUS_INVALID_STATE;

#ifndef EZSP_HOST

  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 0);
  currentZdoNegativeCommandCluster = NODE_DESCRIPTOR_RESPONSE;
  status = SL_STATUS_INVALID_PARAMETER;

  switch (options) {
    case BIT(0):
      negativeZdoCommandFlags = 0;  // negative response only!
      status = SL_STATUS_OK;
      break;
    case BIT(1):
      // node descriptor field absent
      negativeZdoCommandFlags = NEGATIVE_ZDO_COMMAND_FLAGS_NO_NODE_DESCRIPTOR;
      status = SL_STATUS_OK;
      break;
  }

#endif /* EZSP_HOST */

  sl_zigbee_af_core_println("%s: %s: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Node descriptor response config",
                            status);
}

// plugin test-harness z3 zdo active-endpoint-request <dstShort:2>
// <nwkAddrOfInterest:2> <options:4>
void sli_zigbee_af_test_harness_z3_zdo_active_endpoint_request_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t destination       = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_short_addr_t nwkAddrOfInterest = sl_cli_get_argument_uint16(arguments, 1);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 2);

  sl_status_t status;
  uint8_t frame[3], frameLength = 1;

  if (options != BIT(0)) {
    frame[1] = LOW_BYTE(nwkAddrOfInterest);
    frame[2] = HIGH_BYTE(nwkAddrOfInterest);
    frameLength += 2;
  }

  status = sendZdoCommand(destination,
                          ACTIVE_ENDPOINTS_REQUEST,
                          frame,
                          frameLength,
                          0);

  sl_zigbee_af_core_println("%s: %s: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Active endpoint request",
                            status);
}

// plugin test-harness z3 zdo match-desc-req <dstShort:2>
// <nwkAddrOfInterest:2> <profileId:4> <options:4>
void sli_zigbee_af_test_harness_z3_zdo_match_desc_req_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t destination       = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_short_addr_t nwkAddrOfInterest = sl_cli_get_argument_uint16(arguments, 1);
  uint16_t profileId            = sl_cli_get_argument_uint16(arguments, 2);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 3);

  sl_status_t status;
  uint8_t frame[32];
  uint8_t *finger = &frame[1];

  if (options != BIT(0)) {
    // Network address of interest.
    sl_zigbee_af_copy_int16u(finger, 0, nwkAddrOfInterest);
    finger += sizeof(uint16_t);
  }

  // Profile id.
  sl_zigbee_af_copy_int16u(finger, 0, profileId);
  finger += sizeof(uint16_t);

  if (options != BIT(1)) {
    uint8_t reallyInClustersCount
      = (options == BIT(2) ? 1 : sli_zigbee_af_cli_in_cl_count);
    uint8_t reallyOutClustersCount
      = (options == BIT(3) ? 1 : sli_zigbee_af_cli_out_cl_count);

    // In clusters.
    *finger++ = sli_zigbee_af_cli_in_cl_count;
    memmove(finger,
            sli_zigbee_af_cli_zdo_in_clusters,
            reallyInClustersCount * sizeof(uint16_t));
    finger += (reallyInClustersCount * sizeof(uint16_t));

    // Out clusters.
    *finger++ = sli_zigbee_af_cli_out_cl_count;
    memmove(finger,
            sli_zigbee_af_cli_zdo_out_clusters,
            reallyOutClustersCount * sizeof(uint16_t));
    finger += (reallyOutClustersCount * sizeof(uint16_t));
  } else {
    *finger++ = sli_zigbee_af_cli_in_cl_count;  // In clusters.
    *finger++ = sli_zigbee_af_cli_out_cl_count; // Out clusters.
  }

  status = sendZdoCommand(destination,
                          MATCH_DESCRIPTORS_REQUEST,
                          frame,
                          finger - &frame[0], 0);

  sl_zigbee_af_core_println("%s: %s: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Match descriptor request",
                            status);
}

// plugin test-harness z3 zdo match-desc-rsp-config <nwkAddrOfInterest:2>
// <status:1> <options:4>
void sli_zigbee_af_test_harness_z3_zdo_match_desc_rsp_config_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t nwkAddrOfInterest = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t zdoStatus             = sl_cli_get_argument_uint8(arguments, 1);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 2);

  sl_status_t status = SL_STATUS_OK;

  // Global state.
  currentZdoNegativeCommandCluster = MATCH_DESCRIPTORS_RESPONSE;
  currentZdoNegativeCommandFrameLength = 5;

  // Positive behavior.
  currentZdoNegativeCommandFrame[1] = zdoStatus;
  currentZdoNegativeCommandFrame[2] = LOW_BYTE(nwkAddrOfInterest);
  currentZdoNegativeCommandFrame[3] = HIGH_BYTE(nwkAddrOfInterest);

  switch (options) {
    case BIT(0):
      // match list absent
      negativeZdoCommandFlags
        = (NEGATIVE_ZDO_COMMAND_FLAGS_MATCH_LIST_ABSENT);
      break;
    case BIT(1):
      // second response device not found
      negativeZdoCommandFlags
        = (NEGATIVE_ZDO_COMMAND_FLAGS_MATCH_LIST_ABSENT
           | NEGATIVE_ZDO_COMMAND_FLAGS_SEND_POSITIVE_RESPONSE);
      currentZdoNegativeCommandFrame[1] = SL_ZIGBEE_ZDP_DEVICE_NOT_FOUND;
      break;
    case BIT(2):
      // second response no descriptor
      negativeZdoCommandFlags
        = (NEGATIVE_ZDO_COMMAND_FLAGS_MATCH_LIST_ABSENT
           | NEGATIVE_ZDO_COMMAND_FLAGS_SEND_POSITIVE_RESPONSE);
      currentZdoNegativeCommandFrame[1] = SL_ZIGBEE_ZDP_NO_DESCRIPTOR;
      break;
    default:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
  }

  sl_zigbee_af_core_println("%s: %s: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Match descriptor response config",
                            status);
}

// plugin test-harness z3 zdo simple-desc-req <dstShort:2> <dstEndpoint:1>
// <nwkAddrOfInterest:2> <options:4>
void sli_zigbee_af_test_harness_z3_zdo_simple_desc_req_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t destination       = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t endpoint              = sl_cli_get_argument_uint8(arguments, 1);
  sl_802154_short_addr_t nwkAddrOfInterest = sl_cli_get_argument_uint16(arguments, 2);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 3);

  sl_status_t status;
  uint8_t frame[4];
  uint8_t *finger = &frame[1];

  // Network address of interest.
  if (options != BIT(0)) {
    sl_zigbee_af_copy_int16u(finger, 0, nwkAddrOfInterest);
    finger += sizeof(uint16_t);
  }

  // Endpoint.
  if (options != BIT(1)) {
    *finger++ = endpoint;
  }

  status = sendZdoCommand(destination,
                          SIMPLE_DESCRIPTOR_REQUEST,
                          frame,
                          finger - &frame[0],
                          0);

  sl_zigbee_af_core_println("%s: %s: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Simple descriptor request",
                            status);
}

// plugin test-harness z3 simple-desc-rsp-config <nwkAddrOfInterest:2>
// <status:1> <length:1> <options:4>
void sli_zigbee_af_test_harness_z3_zdo_simple_desc_rsp_config_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t nwkAddrOfInterest = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t zdoStatus             = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t length                = sl_cli_get_argument_uint8(arguments, 2);
  uint32_t options              = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 3);

  sl_status_t status = SL_STATUS_OK;

  // Global state.
  currentZdoNegativeCommandCluster = SIMPLE_DESCRIPTOR_RESPONSE;
  currentZdoNegativeCommandFrameLength = 5;

  // Positive behavior.
  currentZdoNegativeCommandFrame[1] = zdoStatus;
  sl_zigbee_af_copy_int16u(currentZdoNegativeCommandFrame, 2, nwkAddrOfInterest);
  currentZdoNegativeCommandFrame[4] = length;

  // Negative behavior.
  switch (options) {
    case BIT(0):
      // simple descriptor field absent
      negativeZdoCommandFlags = (NEGATIVE_ZDO_COMMAND_FLAGS_NO_SIMPLE_DESCRIPTOR);
      break;
    case BIT(1):
      // do not forward to the nwk addr of interest
      break;
    case BIT(2):
      // cluster list key establishment
      negativeZdoCommandFlags = (NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_KE);
      break;
    case BIT(3):
      // swap support for client/server clusters
      negativeZdoCommandFlags = (NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_SWAP);
      break;
    case BIT(4):
      // do not send any cluster lists
      negativeZdoCommandFlags = (NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_ABSENT);
      break;
    case BIT(5):
      // send a positive command alongside a negative command
      negativeZdoCommandFlags
        = (NEGATIVE_ZDO_COMMAND_FLAGS_SEND_POSITIVE_RESPONSE
           | NEGATIVE_ZDO_COMMAND_FLAGS_NO_SIMPLE_DESCRIPTOR);
      currentZdoNegativeCommandFrame[1] = SL_ZIGBEE_ZDP_DEVICE_NOT_FOUND;
      currentZdoNegativeCommandFrame[4] = 0; // length
      break;
    case BIT(6):
      // do not include endpoints fields in the variable payload
      negativeZdoCommandFlags = (NEGATIVE_ZDO_COMMAND_FLAGS_NO_ENDPOINT_FIELDS);
      break;
    case BIT(7):
      // do not include the network address of interest
      negativeZdoCommandFlags = (NEGATIVE_ZDO_COMMAND_FLAGS_NO_NWK_ADDR);
      currentZdoNegativeCommandFrameLength -= 2;
      break;
    default:
      ; // options of 0x00 is ok
  }

  sl_zigbee_af_core_println("%s: %s: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Simple descriptor response config",
                            status);
}

// plugin test-harness z3 zdo bind-group <shortAddress:2> <srcEndpoint:1>
// <dstEndpoint:1> <dstAddress:2> <cluster:2> <srcIeee:8>
void sli_zigbee_af_test_harness_z3_zdo_bind_group_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t shortAddress = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t srcEndpoint      = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t dstEndpoint      = sl_cli_get_argument_uint8(arguments, 2);
  uint16_t groupId         = sl_cli_get_argument_uint16(arguments, 3);
  sl_zigbee_af_cluster_id_t cluster = sl_cli_get_argument_uint16(arguments, 4);
  sl_status_t status;
  uint8_t frame[16];
  uint8_t *finger = &frame[1];

  // Not currently used.
  (void)dstEndpoint;

  // SrcAddress
  finger += sl_zigbee_copy_eui64_arg(arguments, 5, finger, true);

  // SrcEndp
  *finger++ = srcEndpoint;

  // ClusterID
  sl_zigbee_af_copy_int16u(finger, 0, cluster);
  finger += sizeof(cluster);

  // DstAddrMode
  *finger++ = 0x01; // multicast

  // DstAddress
  sl_zigbee_af_copy_int16u(finger, 0, groupId);
  finger += sizeof(groupId);

  status = sendZdoCommand(shortAddress,
                          BIND_REQUEST,
                          frame,
                          finger - &frame[0],
                          0);

  sl_zigbee_af_core_println("%s: %s: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "ZDO bind group",
                            status);
}

// plugin test-harness z3 zdo nwk-addr-req <ieee:8> <requestType:1>
// <startIndex:1> <dstShort:2> <options:4>
void sli_zigbee_af_test_harness_z3_zdo_nwk_addr_req_command(sl_cli_command_arg_t *arguments)
{
  uint8_t requestType          = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t startIndex           = sl_cli_get_argument_uint8(arguments, 2);
  sl_802154_short_addr_t destinationShort = sl_cli_get_argument_uint16(arguments, 3);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 4);

  sl_status_t status;
  uint8_t frame[11];
  uint8_t *finger = &frame[1];

  if (options != BIT(0)) {
    // IEEE address.
    sl_zigbee_copy_eui64_arg(arguments, 0, finger, true);
    finger += EUI64_SIZE;

    // Request type.
    *finger++ = requestType;

    // Start index.
    *finger++ = startIndex;
  }

  status = sendZdoCommand(destinationShort,
                          NETWORK_ADDRESS_REQUEST,
                          frame,
                          finger - &frame[0],
                          0);

  sl_zigbee_af_core_println("%s: %ss: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Network address request",
                            status);
}

// plugin test-harness z3 zdo ieee-addr-req <nwkAddrOfInterest:2>
// <requestType:1> <startIndex:1> <dstShort:2> <options:4>
void sli_zigbee_af_test_harness_z3_zdo_ieee_addr_req_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t nwkAddrOfInterest = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t requestType           = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t startIndex            = sl_cli_get_argument_uint8(arguments, 2);
  sl_802154_short_addr_t destination       = sl_cli_get_argument_uint16(arguments, 3);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 4);

  sl_status_t status;
  uint8_t frame[5];
  uint8_t *finger = &frame[1];

  if (options != BIT(0)) {
    // Network address of interest.
    sl_zigbee_af_copy_int16u(finger, 0, nwkAddrOfInterest);
    finger += sizeof(uint16_t);

    // Request type.
    *finger++ = requestType;

    // Start index.
    *finger++ = startIndex;
  }

  status = sendZdoCommand(destination,
                          IEEE_ADDRESS_REQUEST,
                          frame,
                          finger - &frame[0],
                          0);

  sl_zigbee_af_core_println("%s: %s: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "IEEE address request",
                            status);
}

// plugin test-harness z3 nwk ieee-addr-rsp-config reset
// plugin test-harness z3 nwk ieee-addr-rsp-config issuer-nwk-address-remote-dev
// plugin test-harness z3 nwk ieee-addr-rsp-config status-device-not-found
void sli_zigbee_af_test_harness_z3_zdo_ieee_addr_rsp_config_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status = SL_STATUS_OK;
  char firstChar = sl_cli_get_command_string(arguments, 5)[0];

  // Global state.
  currentZdoNegativeCommandCluster = IEEE_ADDRESS_RESPONSE;
  currentZdoNegativeCommandFrameLength = 12;

  // Positive behavior.
  currentZdoNegativeCommandFrame[1] = SL_ZIGBEE_ZDP_SUCCESS;

  // Negative behavior.
  switch (firstChar) {
    case 'r':
      // reset
      negativeZdoCommandFlags = 0;
      currentZdoNegativeCommandCluster = NULL_CURRENT_ZDO_NEGATIVE_COMMAND_CLUSTER;
      break;
    case 'i':
      // issuer-nwk-address-remote-dev
      negativeZdoCommandFlags |= NEGATIVE_ZDO_COMMAND_FLAGS_OUR_NWK_ADDRESS;
      break;
    case 's':
      // status-device-not-found
      currentZdoNegativeCommandFrame[1] = SL_ZIGBEE_ZDP_DEVICE_NOT_FOUND;
      break;
    default:
      status = SL_STATUS_INVALID_PARAMETER;
  }

  sl_zigbee_af_core_println("%s: %s: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "IEEE address response config",
                            status);
}

// plugin test-harness z3 zdo bc-device-announce <shortAddress:2> <ieeeAddress:8>
void sli_zigbee_af_test_harness_z3_zdo_bc_device_announce_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t shortAddress = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_long_addr_t eui64;
  uint8_t frame[12];
  sl_status_t status;

  sl_zigbee_copy_eui64_arg(arguments, 1, eui64, true);

  sl_zigbee_af_copy_int16u(frame, 1, shortAddress);
  memmove(frame + 3, eui64, EUI64_SIZE);
  frame[11] = 0xE0; // capability: fake like we are a router

  status = sendZdoCommand(SL_ZIGBEE_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS,
                          END_DEVICE_ANNOUNCE,
                          frame,
                          sizeof(frame), 0);

  sl_zigbee_af_core_println("%s: %s: 0x%08X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Broadcast device announce",
                            status);
}

#ifndef EZSP_HOST
sl_status_t sli_zigbee_af_test_harness_z3_zdo_command_response_handler(uint8_t *requestBuffer,
                                                                       uint8_t length,
                                                                       sl_zigbee_aps_frame_t *apsFrame)
{
  sl_zigbee_af_debug_println("sli_zigbee_af_test_harness_z3_zdo_command_response_handler: cluster = %04X", apsFrame->clusterId);
  sl_status_t status = SL_STATUS_OK;
  uint16_t clusterId = apsFrame->clusterId;
  sl_802154_short_addr_t node_id = sl_zigbee_af_get_node_id();

  // Make sure the cluster id matches the negative zdo command's request.
  if (clusterId != (currentZdoNegativeCommandCluster & ~0x8000)) {
    // EMZIGBEE-1317 -The simple descriptor req to reply with a short list.
    // If no cluster is configured for negative test response
    // and a simple descriptor request is received - then handle it with
    // a shorter list of clusters so that the payload does not exhaust.
    if (clusterId == SIMPLE_DESCRIPTOR_REQUEST
        || clusterId == USER_DESCRIPTOR_REQUEST) { // No config command specified yet
      negativeZdoCommandFlags = 0;
    } else {
      return SL_STATUS_INVALID_STATE;
    }
  }

  if (clusterId == IEEE_ADDRESS_REQUEST) {
    if (length <  TH_IEEE_ADD_REQ_PAYLOAD_LENGTH ) {
      return SL_STATUS_INVALID_STATE;
    }
    // Perform some last second mangling of the frame.
    sl_802154_short_addr_t nwkAddressOfInterest
      = HIGH_LOW_TO_INT(requestBuffer[2],
                        requestBuffer[1]);
    if (negativeZdoCommandFlags & NEGATIVE_ZDO_COMMAND_FLAGS_OUR_NWK_ADDRESS) {
      currentZdoNegativeCommandFrame[10] = LOW_BYTE(node_id);
      currentZdoNegativeCommandFrame[11] = HIGH_BYTE(node_id);
    } else {
      currentZdoNegativeCommandFrame[10] = LOW_BYTE(nwkAddressOfInterest);
      currentZdoNegativeCommandFrame[11] = HIGH_BYTE(nwkAddressOfInterest);
    }
    status = sl_zigbee_lookup_eui64_by_node_id(nwkAddressOfInterest,
                                               &currentZdoNegativeCommandFrame[2]);
  } else if (clusterId == NODE_DESCRIPTOR_REQUEST && !(negativeZdoCommandFlags
                                                       & NEGATIVE_ZDO_COMMAND_FLAGS_NO_NODE_DESCRIPTOR)) {
    sl_zigbee_node_type_t nodetype;
    uint16_t manufacturer_code = sl_zigbee_get_manufacturer_code();
    sl_zigbee_node_descriptor_info_t descriptor;
    (void) sl_zigbee_get_node_descriptor(&descriptor);
    sl_zigbee_get_network_parameters(&nodetype, NULL);
    currentZdoNegativeCommandFrame[1] = SL_ZIGBEE_ZDP_SUCCESS;
    currentZdoNegativeCommandFrame[2] = LOW_BYTE(node_id);
    currentZdoNegativeCommandFrame[3] = HIGH_BYTE(node_id);
    currentZdoNegativeCommandFrame[4] = nodetype;
    currentZdoNegativeCommandFrame[5] = ZDO_NODE_DESCRIPTOR_BAND_2400_MHZ;
    currentZdoNegativeCommandFrame[6] = descriptor.capability;
    currentZdoNegativeCommandFrame[7] = LOW_BYTE(manufacturer_code);
    currentZdoNegativeCommandFrame[8] = HIGH_BYTE(manufacturer_code);
    currentZdoNegativeCommandFrame[9] = sl_zigbee_maximum_aps_payload_length();
    currentZdoNegativeCommandFrame[10] = LOW_BYTE(descriptor.max_incoming_transfer_size);
    currentZdoNegativeCommandFrame[11] = HIGH_BYTE(descriptor.max_incoming_transfer_size);

    uint8_t mask_lowbyte = 0;
    if (sl_zigbee_get_node_id() == 0) {
      mask_lowbyte |= 0x01;
    }
    if (sli_zigbee_am_network_manager()) {
      mask_lowbyte |= 0x40;
    }
    currentZdoNegativeCommandFrame[12] = mask_lowbyte;
    currentZdoNegativeCommandFrame[13] = 0x00;

    // Set a global for the Network Creator Security key establishment callback
    sli_zigbee_af_test_harness_z3_server_mask_high = 0;

    currentZdoNegativeCommandFrame[14] = LOW_BYTE(descriptor.max_outgoing_transfer_size);
    currentZdoNegativeCommandFrame[15] = HIGH_BYTE(descriptor.max_outgoing_transfer_size);
    currentZdoNegativeCommandFrame[16] = 0x00;
    currentZdoNegativeCommandFrameLength = 17;
  } else if (clusterId == MATCH_DESCRIPTORS_REQUEST) {
    if (negativeZdoCommandFlags
        & NEGATIVE_ZDO_COMMAND_FLAGS_MATCH_LIST_ABSENT) {
      // Set the match length to 0.
      currentZdoNegativeCommandFrame[4] = 0;
      currentZdoNegativeCommandFrameLength = 5;
    } else {
      // TODO: fill the match desc response.
    }
  } else if (clusterId == SIMPLE_DESCRIPTOR_REQUEST
             && !(negativeZdoCommandFlags
                  & NEGATIVE_ZDO_COMMAND_FLAGS_NO_SIMPLE_DESCRIPTOR)) {
    // TODO: do we need these to be legit?
    // Servers Basic,Identify,Level, On/Off and OTA
    // Use pre-defined cluster lists.
    uint16_t inCluster[] = IN_CLUSTERS;
    uint8_t inClusterCount = sizeof(inCluster) / sizeof(uint16_t);
    uint16_t outCluster[] = OUT_CLUSTERS;
    uint8_t outClusterCount = sizeof(outCluster) / sizeof(uint16_t);
    // Simple Descriptor Request has - seq(1), short address(2),ep(1)
    if (length < TH_SIMPLE_DESC_REQ_PAYLOAD_LENGTH ) {
      // short length - return.
      return SL_STATUS_INVALID_STATE;
    }
    uint8_t endpoint = requestBuffer[3];
    uint8_t endpointIndex
      = sl_zigbee_af_index_from_endpoint(endpoint);
    if (endpointIndex != 0xFF) {
      // If the end point is not the test end point - let ZDO handler handle it
      if (endpoint != TEST_HARNESS_ENDPOINT) {
        return SL_STATUS_INVALID_STATE;
      }
      currentZdoNegativeCommandFrameLength = 0;
      currentZdoNegativeCommandCluster = SIMPLE_DESCRIPTOR_RESPONSE;
      // Sequence number is picked up from request - index 0
      // Account for the field length, increment the counter
      currentZdoNegativeCommandFrameLength++;
      currentZdoNegativeCommandFrame[currentZdoNegativeCommandFrameLength++]
        = SL_ZIGBEE_ZDP_SUCCESS; //Status - Index 1
      if (!(negativeZdoCommandFlags & NEGATIVE_ZDO_COMMAND_FLAGS_NO_NWK_ADDR)) {
        currentZdoNegativeCommandFrame[currentZdoNegativeCommandFrameLength++]
          = LOW_BYTE(node_id); // Index 2
        currentZdoNegativeCommandFrame[currentZdoNegativeCommandFrameLength++]
          = HIGH_BYTE(node_id); // Index 3
      }
      // Length field index 2 or 4 depending the above.
      // Increment the length to account the length field itself.
      currentZdoNegativeCommandFrameLength++;

      if (!(negativeZdoCommandFlags
            & NEGATIVE_ZDO_COMMAND_FLAGS_NO_ENDPOINT_FIELDS)) {
        // endpoint
        currentZdoNegativeCommandFrame[currentZdoNegativeCommandFrameLength++]
          = endpoint;
      }

      // profile id
      sl_zigbee_af_copy_int16u(currentZdoNegativeCommandFrame,
                               currentZdoNegativeCommandFrameLength,
                               sl_zigbee_af_profile_id_from_index(endpointIndex));
      currentZdoNegativeCommandFrameLength += sizeof(uint16_t);

      // device id
      sl_zigbee_af_copy_int16u(currentZdoNegativeCommandFrame,
                               currentZdoNegativeCommandFrameLength,
                               sl_zigbee_af_device_id_from_index(endpointIndex));
      currentZdoNegativeCommandFrameLength += sizeof(uint16_t);

      // device version
      currentZdoNegativeCommandFrame[currentZdoNegativeCommandFrameLength++]
        = sl_zigbee_af_device_version_from_index(endpointIndex);

      if (negativeZdoCommandFlags & NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_KE) {
        inCluster[0] = outCluster[0] = ZCL_KEY_ESTABLISHMENT_CLUSTER_ID;
        inClusterCount = outClusterCount = 1;
      }

      // Initialise copy order for in and out clusters
      uint8_t firstCount = inClusterCount;
      uint16_t *firstList  = inCluster;
      uint8_t secondCount = outClusterCount;
      uint16_t *secondList  = outCluster;
      // Swap flag ? - Yes, then swap it
      if (negativeZdoCommandFlags
          & NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_SWAP) {
        firstCount = outClusterCount;
        firstList  = outCluster;
        secondCount = inClusterCount;
        secondList  = inCluster;
      }
      // Copy in order only if cluster list is not absent
      if (!(negativeZdoCommandFlags
            & NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_ABSENT)) {
        currentZdoNegativeCommandFrame[currentZdoNegativeCommandFrameLength++]
          = firstCount;
        for (int i = 0; i < firstCount; i++ ) {
          sl_zigbee_af_copy_int16u(currentZdoNegativeCommandFrame,
                                   currentZdoNegativeCommandFrameLength,
                                   firstList[i]);
          currentZdoNegativeCommandFrameLength += sizeof(uint16_t);
        }
        currentZdoNegativeCommandFrame[currentZdoNegativeCommandFrameLength++]
          = secondCount;
        for (int i = 0; i < secondCount; i++ ) {
          sl_zigbee_af_copy_int16u(currentZdoNegativeCommandFrame,
                                   currentZdoNegativeCommandFrameLength,
                                   secondList[i]);
          currentZdoNegativeCommandFrameLength += sizeof(uint16_t);
        }
      }

      if (negativeZdoCommandFlags & NEGATIVE_ZDO_COMMAND_FLAGS_NO_NWK_ADDR) {
        currentZdoNegativeCommandFrame[2] // simple descriptor length
          = (currentZdoNegativeCommandFrameLength - 3);
      } else {
        currentZdoNegativeCommandFrame[4] // simple descriptor length
          = (currentZdoNegativeCommandFrameLength - 5);
      }
    } else {
      status = SL_STATUS_FAIL;
    }
  } else if (clusterId == USER_DESCRIPTOR_REQUEST
             && !(negativeZdoCommandFlags
                  & NEGATIVE_ZDO_COMMAND_FLAGS_NO_USER_DESCRIPTOR)) {
    // User Descriptor Request has - seq(1), short address(2)
    if (length < TH_USER_DESC_REQ_PAYLOAD_LENGTH) {
      // short length - return.
      return SL_STATUS_INVALID_STATE;
    }
    // Note, this is currently just sends a good response for testing -
    // there are no negative command options at present.
    currentZdoNegativeCommandFrameLength = 0;
    currentZdoNegativeCommandCluster = USER_DESCRIPTOR_RESPONSE;
    // Sequence number is picked up from request - index 0
    // Account for the field length, increment the counter
    currentZdoNegativeCommandFrameLength++;
    currentZdoNegativeCommandFrame[currentZdoNegativeCommandFrameLength++]
      = SL_ZIGBEE_ZDP_SUCCESS; //Status - Index 1
    currentZdoNegativeCommandFrame[currentZdoNegativeCommandFrameLength++]
      = LOW_BYTE(sl_zigbee_af_get_node_id()); // Index 2
    currentZdoNegativeCommandFrame[currentZdoNegativeCommandFrameLength++]
      = HIGH_BYTE(sl_zigbee_af_get_node_id()); // Index 3

    // Increment the length to account the length field itself.
    currentZdoNegativeCommandFrameLength++;

    // Copy a sample user descriptor.
    static const char userDescriptor[] = "Test Harness App";
    memcpy(&currentZdoNegativeCommandFrame[currentZdoNegativeCommandFrameLength], userDescriptor, TH_USER_DESC_RSP_PAYLOAD_LENGTH);
    currentZdoNegativeCommandFrameLength += TH_USER_DESC_RSP_PAYLOAD_LENGTH;

    currentZdoNegativeCommandFrame[4] // user descriptor length
      = TH_USER_DESC_RSP_PAYLOAD_LENGTH;
  }

  if (status == SL_STATUS_OK) {
    // We use an event for this command so that the positive stuff will go
    // out of the radio first.
    currentZdoNegativeCommandDestination = sli_zigbee_current_sender;
    // Copy sequence number to the response frame, to keep AF discovery happy.
    currentZdoNegativeCommandFrame[0] = requestBuffer[0];
    sl_zigbee_af_event_set_active(&sl_zigbee_af_test_harness_z3_zdo_send_event_control);
  }

  // Maybe change the cluster id to an invalid cluster so we don't process the
  // ZDO command.
  if (!(negativeZdoCommandFlags
        & NEGATIVE_ZDO_COMMAND_FLAGS_SEND_POSITIVE_RESPONSE)) {
    apsFrame->clusterId = 0xFFFF;
  }

  return status;
}
#endif /* EZSP_HOST */

void sl_zigbee_af_test_harness_z3_zdo_send_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);

  sl_zigbee_af_event_set_inactive(&sl_zigbee_af_test_harness_z3_zdo_send_event_control);

  if (!(negativeZdoCommandFlags
        & NEGATIVE_ZDO_COMMAND_FLAGS_NO_NEGATIVE_RESPONSE)) {
    sl_zigbee_af_core_println("%s: %s: 0x%02X",
                              TEST_HARNESS_Z3_PRINT_NAME,
                              "ZDO send event handler",
                              sendZdoCommand(currentZdoNegativeCommandDestination,
                                             currentZdoNegativeCommandCluster,
                                             currentZdoNegativeCommandFrame,
                                             currentZdoNegativeCommandFrameLength,
                                             currentZdoNegativeCommandFrame[0]));
  }
}

// plugin test-harness z3 change-channel <newChannel:1>
void sli_zigbeee_af_test_harness_z3_change_channel(SL_CLI_COMMAND_ARG)
{
  uint8_t newChannel = (uint8_t)sl_cli_get_argument_uint8(arguments, 0);

  // Just do it, no delays.
  sl_zigbee_set_radio_channel(newChannel);
}
