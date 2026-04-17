/***************************************************************************//**
 * @file
 * @brief CLI for the Interpan plugin.
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

#include "interpan.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"

#ifndef SL_ZIGBEE_AF_GENERATE_CLI
  #error The Interpan plugin is not compatible with the legacy CLI.
#endif

// Prototypes

void sli_zigbee_af_interpan_enable_command(sl_cli_command_arg_t *args);
void sli_zigbee_af_interpan_disable_command(sl_cli_command_arg_t *args);
void sli_zigbee_af_interpan_fragment_test_command(sl_cli_command_arg_t *args);
void sli_zigbee_af_interpan_set_message_timeout_command(sl_cli_command_arg_t *args);
void sli_zigbee_af_interpan_group_short_command(sl_cli_command_arg_t *args);
void sli_zigbee_af_interpan_long_command(sl_cli_command_arg_t *args);

// Global variables

// The following variables are defined in app/framework/cli/zcl-cli.c
extern uint8_t appZclBuffer[];
extern uint16_t appZclBufferLen;
extern bool zclCmdIsBuilt;
extern sl_zigbee_aps_frame_t globalApsFrame;

// This is large. It may go away or be refactored for future releases.
uint8_t testMessage[SL_ZIGBEE_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_PAYLOAD_SIZE];

// Functions

// Globally enable inter-PAN messages
void sli_zigbee_af_interpan_enable_command(sl_cli_command_arg_t *args)
{
  (void)args;
  sli_zigbee_af_interpan_set_enable_state(true);
}

// Globally disable inter-PAN messages
void sli_zigbee_af_interpan_disable_command(sl_cli_command_arg_t *args)
{
  (void)args;
  sli_zigbee_af_interpan_set_enable_state(false);
}

// Test an inter-PAN fragment transmission with a randomly-filled payload
void sli_zigbee_af_interpan_fragment_test_command(sl_cli_command_arg_t *args)
{
  sl_802154_pan_id_t panId = sl_cli_get_argument_uint16(args, 0);
  sl_802154_long_addr_t eui64;
  sl_zigbee_copy_eui64_arg(args, 1, eui64, true);
  uint16_t clusterId = sl_cli_get_argument_uint16(args, 2);
  uint16_t messageLen = sl_cli_get_argument_uint16(args, 3);

  messageLen = (messageLen > sizeof(testMessage))
               ? sizeof(testMessage) : messageLen;

  uint16_t i;
  for (i = 0; i < messageLen; i++) {
    testMessage[i] = (sl_zigbee_get_pseudo_random_number() & 0xFF);
  }

  sl_zigbee_af_core_print("Sending inter-PAN message of len %d to ", messageLen);
  //sl_zigbee_af_core_debug_exec(sl_zigbee_af_print_big_endian_eui64(eui64));
  sl_zigbee_af_core_print(" with random values: ");

  sl_status_t status = sl_zigbee_af_send_inter_pan(panId,
                                                   eui64,
                                                   SL_ZIGBEE_NULL_NODE_ID,
                                                   0,     // mcast id - unused
                                                   clusterId,
                                                   SE_PROFILE_ID, // GBCS only
                                                   messageLen,
                                                   testMessage);

  sl_zigbee_af_core_println("%s (0x%02X)",
                            (SL_STATUS_OK == status) ? "success" : "failure",
                            status);
  sl_zigbee_af_core_flush();
}

// Set the timeout for inter-PAN fragment transmissions
void sli_zigbee_af_interpan_set_message_timeout_command(sl_cli_command_arg_t *args)
{
#if defined(ALLOW_FRAGMENTATION)
  uint16_t timeout = sl_cli_get_argument_uint16(args, 0);
  if (0 == timeout) {
    sl_zigbee_af_core_println("ERR: inter-PAN message timeout of 0 not allowed");
    sl_zigbee_af_core_flush();
    return;
  }
  interpanPluginSetFragmentMessageTimeout(timeout);
#else
  (void)args;
  sl_zigbee_af_core_println("ERR: inter-PAN fragmentation feature missing");
#endif //ALLOW_FRAGMENTATION
}

// plugin interpan group <groupId:2> <destPAN:2> <destProfileID:2>
// plugin interpan short <shortId:2> <destPAN:2> <destProfileID:2>
void sli_zigbee_af_interpan_group_short_command(sl_cli_command_arg_t *args)
{
  sl_status_t status;
  sl_zigbee_af_interpan_header_t header;
  uint16_t shortOrGroupId;

  if (zclCmdIsBuilt == false) {
    sl_zigbee_af_core_println("no cmd");
    return;
  }

  memset(&header, 0, sizeof(sl_zigbee_af_interpan_header_t));
  shortOrGroupId = (uint16_t)sl_cli_get_argument_uint16(args, 0);
  header.panId = (uint16_t)sl_cli_get_argument_uint16(args, 1);
  header.profileId = (uint16_t)sl_cli_get_argument_uint16(args, 2);
  header.clusterId = globalApsFrame.clusterId;

  if (sl_cli_get_command_string(args, 2)[0] == 'g') {
    header.groupId = shortOrGroupId;
    sl_zigbee_af_debug_println("interpan %s %04x", "group", header.groupId);
  } else {
    header.shortAddress = shortOrGroupId;
    sl_zigbee_af_debug_println("interpan %s %04x", "short", shortOrGroupId);
  }

  status = sl_zigbee_af_interpan_send_message_cb(&header,
                                                 appZclBufferLen,
                                                 appZclBuffer);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_debug_println("ERR: Inter-PAN send failed: 0x%08X", status);
  }
}

// plugin interpan long  <longId:8>  <destPAN:2> <destProfileID:2> <options:2>
//    Options: Bit(0) = encrypt.  Can only encrypt with this CLI command
//    since long address must be present.
void sli_zigbee_af_interpan_long_command(sl_cli_command_arg_t *args)
{
  sl_status_t status;
  sl_zigbee_af_interpan_header_t header;

  if (zclCmdIsBuilt == false) {
    sl_zigbee_af_core_println("no cmd");
    return;
  }

  memset(&header, 0, sizeof(sl_zigbee_af_interpan_header_t));
  header.panId = (uint16_t)sl_cli_get_argument_uint16(args, 1);
  header.profileId = (uint16_t)sl_cli_get_argument_uint16(args, 2);
  header.options = ((uint16_t)sl_cli_get_argument_uint16(args, 3)
                    | SL_ZIGBEE_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS);
  header.clusterId = globalApsFrame.clusterId;

  sl_zigbee_copy_eui64_arg(args, 0, header.longAddress, true);

  status = sl_zigbee_af_interpan_send_message_cb(&header,
                                                 appZclBufferLen,
                                                 appZclBuffer);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_debug_println("ERR: Inter-PAN send failed: 0x%08X", status);
  }
}
