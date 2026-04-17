/***************************************************************************//**
 * @file
 * @brief ZigBee 3.0 zcl test harness functionality
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

// -----------------------------------------------------------------------------
// ZCL CLI commands

// plugin test-harness z3 read <cluster:2> <attribute:2> <dstShort:2>
// <srcEndpoint:1> <dstEndpoint:1> <profileId:2>
void sli_zigbee_af_test_harness_z3_zcl_read_command(SL_CLI_COMMAND_ARG)
{
  sl_status_t status = SL_STATUS_INVALID_STATE;

#ifndef EZSP_HOST

  uint16_t clusterId      = sl_cli_get_argument_uint16(arguments, 0);
  uint16_t attributeId    = sl_cli_get_argument_uint16(arguments, 1);
  sl_802154_short_addr_t destination = sl_cli_get_argument_uint16(arguments, 2);
  uint8_t srcEndpoint     = sl_cli_get_argument_uint8(arguments, 3);
  uint8_t dstEndpoint     = sl_cli_get_argument_uint8(arguments, 4);
  uint16_t profileId      = sl_cli_get_argument_uint16(arguments, 5);
  uint8_t frame[5];
  sl_zigbee_aps_frame_t apsFrame;

  // Frame control
  // global command, client to server
  frame[0] = (ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER);
  // sequence, whatever
  frame[1] = 0xAC;
  // command id: read attributes
  frame[2] = ZCL_READ_ATTRIBUTES_COMMAND_ID;
  // payload: attribute id
  frame[3] = LOW_BYTE(attributeId);
  frame[4] = HIGH_BYTE(attributeId);

  apsFrame.profileId = profileId;
  apsFrame.clusterId = clusterId;
  apsFrame.sourceEndpoint = srcEndpoint;
  apsFrame.destinationEndpoint = dstEndpoint;
  apsFrame.options = (SL_ZIGBEE_APS_OPTION_RETRY);

  status = sl_zigbee_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                  destination,
                                  &apsFrame,
                                  0x00, // tag
                                  sizeof(frame),
                                  frame,
                                  NULL);
#endif /* EZSP_HOST */

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Read",
                            status);
}
