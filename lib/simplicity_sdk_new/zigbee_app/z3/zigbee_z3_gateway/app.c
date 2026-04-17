/***************************************************************************//**
 * @file app.c
 * @brief Callbacks implementation and application specific code.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "app/framework/util/util.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "stack/include/trust-center.h"
#include "zap-cluster-command-parser.h"
#include <stdlib.h>
#include "stack/include/zigbee-security-manager.h"

// The number of tokens that can be written using sl_zigbee_ezsp_set_token and read using
// sl_zigbee_ezsp_get_token.
#define MFGSAMP_NUM_EZSP_TOKENS 8
// The size of the tokens that can be written using sl_zigbee_ezsp_set_token and read using
// sl_zigbee_ezsp_get_token.
#define MFGSAMP_EZSP_TOKEN_SIZE 8
// The number of manufacturing tokens.
#define MFGSAMP_NUM_EZSP_MFG_TOKENS 11
// The size of the largest EZSP Mfg token, SL_ZIGBEE_EZSP_MFG_CBKE_DATA please refer to
// app/util/ezsp/ezsp-enum.h.
#define MFGSAMP_EZSP_TOKEN_MFG_MAXSIZE 92

#if defined(SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_UNICAST_PRESENT)    \
  || defined(SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_BROADCAST_PRESENT) \
  || defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)
extern sl_status_t sl_zigbee_af_trust_center_start_network_key_update(void);
#endif // defined(SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_UNICAST_PRESENT) ||
// defined(SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_BROADCAST_PRESENT) ||
// defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)

//----------------------
// ZCL commands handling

static void ias_ace_cluster_arm_command_handler(uint8_t armMode,
                                                uint8_t* armDisarmCode,
                                                uint8_t zoneId)
{
  uint16_t armDisarmCodeLength = sl_zigbee_af_string_length(armDisarmCode);
  uint16_t i;

  sl_zigbee_app_debug_print("IAS ACE Arm Received %04X", armMode);

  // Start i at 1 to skip over leading character in the byte array as it is the
  // length byte
  for (i = 1; i < armDisarmCodeLength; i++) {
    sl_zigbee_app_debug_print("%c", armDisarmCode[i]);
  }
  sl_zigbee_app_debug_println(" %02X", zoneId);

  sl_zigbee_af_fill_command_ias_ace_cluster_arm_response(armMode);
  sl_zigbee_af_send_response();
}

static void ias_ace_cluster_bypass_command_handler(uint8_t numberOfZones,
                                                   uint8_t* zoneIds,
                                                   uint8_t* armDisarmCode)
{
  UNUSED_VAR(armDisarmCode);
  sl_zigbee_app_debug_print("IAS ACE Cluster Bypass for zones ");

  for (uint8_t i = 0; i < numberOfZones; i++) {
    sl_zigbee_app_debug_print("%d ", zoneIds[i]);
  }
  sl_zigbee_app_debug_println("");

  sl_zigbee_af_fill_command_ias_ace_cluster_bypass_response(numberOfZones,
                                                            zoneIds,
                                                            numberOfZones);
  sl_zigbee_af_send_response();
}

static uint32_t zcl_ias_ace_cluster_server_command_handler(sl_service_opcode_t opcode,
                                                           sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;

  switch (cmd->commandId) {
    case ZCL_ARM_COMMAND_ID:
    {
      sl_zcl_ias_ace_cluster_arm_command_t cmd_data;

      if (zcl_decode_ias_ace_cluster_arm_command(cmd, &cmd_data)
          != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
        return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
      }

      ias_ace_cluster_arm_command_handler(cmd_data.armMode,
                                          cmd_data.armDisarmCode,
                                          cmd_data.zoneId);
      return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
    }
    case ZCL_BYPASS_COMMAND_ID:
    {
      sl_zcl_ias_ace_cluster_bypass_command_t cmd_data;

      if (zcl_decode_ias_ace_cluster_bypass_command(cmd, &cmd_data)
          != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
        return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
      }

      ias_ace_cluster_bypass_command_handler(cmd_data.numberOfZones,
                                             cmd_data.zoneIds,
                                             cmd_data.armDisarmCode);

      return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
    }
  }

  return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
}

//----------------------
// Implemented Callbacks

/** @brief Init
 * Application init function
 */
void sl_zigbee_af_main_init_cb(void)
{
  // Subscribe to ZCL commands for the IAS_ACE cluster, server side.
  sl_zigbee_subscribe_to_zcl_commands(ZCL_IAS_ACE_CLUSTER_ID,
                                      0xFFFF,
                                      ZCL_DIRECTION_CLIENT_TO_SERVER,
                                      zcl_ias_ace_cluster_server_command_handler);
}

#ifdef SL_CATALOG_CLI_PRESENT
//-------------------------------------
// Custom CLI commands and related code

// The manufacturing tokens are enumerated in app/util/ezsp/ezsp-protocol.h.
// The names are enumerated here to make it easier for the user.
static const char * ezspMfgTokenNames[] =
{
  "SL_ZIGBEE_EZSP_MFG_CUSTOM_VERSION...",
  "SL_ZIGBEE_EZSP_MFG_STRING...........",
  "SL_ZIGBEE_EZSP_MFG_BOARD_NAME.......",
  "SL_ZIGBEE_EZSP_MFG_MANUF_ID.........",
  "SL_ZIGBEE_EZSP_MFG_PHY_CONFIG.......",
  "SL_ZIGBEE_EZSP_MFG_BOOTLOAD_AES_KEY.",
  "SL_ZIGBEE_EZSP_MFG_ASH_CONFIG.......",
  "SL_ZIGBEE_EZSP_MFG_SL_ZIGBEE_EZSP_STORAGE.....",
  "SL_ZIGBEE_EZSP_STACK_CAL_DATA.......",
  "SL_ZIGBEE_EZSP_MFG_CBKE_DATA........",
  "SL_ZIGBEE_EZSP_MFG_INSTALLATION_CODE"
};

// Called to dump all of the tokens. This dumps the indices, the names,
// and the values using sl_zigbee_ezsp_get_token and sl_zigbee_ezsp_get_mfg_token. The indices
// are used for read and write functions below.
void mfgappTokenDump(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  sl_status_t status;
  uint8_t tokenData[MFGSAMP_EZSP_TOKEN_MFG_MAXSIZE];
  uint8_t index, i, tokenLength;

  // first go through the tokens accessed using sl_zigbee_ezsp_get_token
  sl_zigbee_app_debug_println("(data shown little endian)");
  sl_zigbee_app_debug_println("Tokens:");
  sl_zigbee_app_debug_println("idx  value:");
  for (index = 0; index < MFGSAMP_NUM_EZSP_TOKENS; index++) {
    // get the token data here
    status = sl_zigbee_ezsp_get_token(index, tokenData);
    sl_zigbee_app_debug_print("[%d]", index);
    if (status == SL_STATUS_OK) {
      // Print out the token data
      for (i = 0; i < MFGSAMP_EZSP_TOKEN_SIZE; i++) {
        sl_zigbee_app_debug_print(" %02X", tokenData[i]);
      }
      sl_zigbee_app_debug_println("");
    } else {
      // handle when sl_zigbee_ezsp_get_token returns an error
      sl_zigbee_app_debug_println(" ... error 0x%02X ...", status);
    }
  }

  // now go through the tokens accessed using sl_zigbee_ezsp_get_mfg_token
  // the manufacturing tokens are enumerated in app/util/ezsp/ezsp-protocol.h
  // this file contains an array (ezspMfgTokenNames) representing the names.
  sl_zigbee_app_debug_println("Manufacturing Tokens:");
  sl_zigbee_app_debug_println("idx  token name                 len   value");
  for (index = 0; index < MFGSAMP_NUM_EZSP_MFG_TOKENS; index++) {
    // sl_zigbee_ezsp_get_mfg_token returns a length, be careful to only access
    // valid token indices.
    tokenLength = sl_zigbee_ezsp_get_mfg_token(index, tokenData);
    sl_zigbee_app_debug_println("[%02X] %s: 0x%02X:",
                                index, ezspMfgTokenNames[index], tokenLength);

    // Print out the token data
    for (i = 0; i < tokenLength; i++) {
      if ((i != 0) && ((i % 8) == 0)) {
        sl_zigbee_app_debug_println("");
        sl_zigbee_app_debug_print("                                    :");
      }
      sl_zigbee_app_debug_print(" %02X", tokenData[i]);
    }
    sl_zigbee_app_debug_println("");
  }
  sl_zigbee_app_debug_println("");
}

#if defined(SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_UNICAST_PRESENT)    \
  || defined(SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_BROADCAST_PRESENT) \
  || defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)
void changeNwkKeyCommand(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  sl_status_t status = sl_zigbee_af_trust_center_start_network_key_update();

  if (status != SL_STATUS_OK) {
    sl_zigbee_app_debug_println("Change Key Error %02X", status);
  } else {
    sl_zigbee_app_debug_println("Change Key Success");
  }
}
#endif // defined(SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_UNICAST_PRESENT) ||
// defined(SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_BROADCAST_PRESENT) ||
// defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)

static void dcPrintKey(uint8_t label, uint8_t *key)
{
  uint8_t i;
  sl_zigbee_app_debug_println("key %02X: ", label);
  for (i = 0; i < SL_ZIGBEE_ENCRYPTION_KEY_SIZE; i++) {
    sl_zigbee_app_debug_print("%02X", key[i]);
  }
  sl_zigbee_app_debug_println("");
}

void printNextKeyCommand(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  sl_status_t status;
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_key_t plaintext_key;

  sl_zigbee_sec_man_init_context(&context);

  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_NETWORK;
  context.key_index = 1;

  status = sl_zigbee_sec_man_export_key(&context, &plaintext_key);

  if (status != SL_STATUS_OK) {
    sl_zigbee_app_debug_println("Error getting key");
  } else {
    dcPrintKey(1, plaintext_key.key);
  }
}

void versionCommand(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  sl_zigbee_app_debug_print("Version:  0.1 Alpha\n");
  sl_zigbee_app_debug_println(" %s", __DATE__);
  sl_zigbee_app_debug_println(" %s", __TIME__);
  sl_zigbee_app_debug_println("");
#ifdef SL_ZIGBEE_TEST
  sl_zigbee_app_debug_println("Print formatter test : 0x%02X=0x12, 0x%02x=0x1234 0x%04x=0x12345678",
                              0x12, 0x1234, 0x12345678);
#endif
}

void setTxPowerCommand(sl_cli_command_arg_t *arguments)
{
  int8_t dBm = sl_cli_get_argument_int8(arguments, 0);

  sl_zigbee_set_radio_power(dBm);
}

#ifdef SL_CATALOG_ZIGBEE_EZSP_SPI_PRESENT
void setSleepMode(SL_CLI_COMMAND_ARG)
{
  uint8_t sleep_mode = sl_cli_get_argument_uint8(arguments, 0);
  if (sleep_mode <= EZSP_FRAME_CONTROL_RESERVED_SLEEP) {
    ezspSleepMode = sleep_mode;
    sl_zigbee_ezsp_nop();
  }
}

void wakeNcpUp(SL_CLI_COMMAND_ARG)
{
  (void)arguments;
  // Wake ncp up then put it into idle state.
  // This command is useful when sleepy SPI NCP is in deep sleep
  // or power down mode that require an external interrupt to wake up
  sl_zigbee_ezsp_wake_up();
  ezspSleepMode = EZSP_FRAME_CONTROL_IDLE;
  sl_zigbee_ezsp_nop();
}
#endif // SL_CATALOG_ZIGBEE_EZSP_SPI_PRESENT

#endif

#ifdef SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT
bool sl_zigbee_af_get_endpoint_info_cb(int8u endpoint,
                                       int8u* returnNetworkIndex,
                                       sl_zigbee_af_endpoint_info_struct_t* returnEndpointInfo)
{
  // In case GP endpoint is located on the NCP, the host has no way
  // to know what networkIndex and profileId that endpoint is configured.
  // User has to manually provide that data.
  if (endpoint == 242) {
    *returnNetworkIndex = 0;
    returnEndpointInfo->profileId = 0xA1E0;
    return true;
  }
  return false;
}
#endif // SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT
