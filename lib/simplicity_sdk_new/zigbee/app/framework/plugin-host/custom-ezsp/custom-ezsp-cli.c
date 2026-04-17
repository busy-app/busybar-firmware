/***************************************************************************//**
 * @file
 * @brief A sample of custom EZSP protocol.
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

#include "custom-ezsp.h"

#include "app/framework/include/af.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "app/xncp/xncp-sample-custom-ezsp-protocol.h"
void sl_zigbee_af_custom_ezsp_set_power_mode_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  // Custom command ID (1 byte) + power mode (1 byte)
  uint8_t message[2];
  uint8_t mode = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t replyLength = 0;

  message[0] = SL_ZIGBEE_CUSTOM_EZSP_COMMAND_SET_POWER_MODE;
  message[1] = (mode == 0)
               ? SL_ZIGBEE_XNCP_NORMAL_MODE
               : SL_ZIGBEE_XNCP_LOW_POWER_MODE;

  if (mode > 1) {
    sl_zigbee_af_core_println("Invalid mode, allowed values: {0,1}");
    return;
  }

  status = sl_zigbee_ezsp_custom_frame(2, message, &replyLength, NULL);

  if (status == SL_STATUS_OK) {
    sl_zigbee_af_core_println("Power mode set");
  } else {
    sl_zigbee_af_core_println("Power mode set failed, status:0x%02X", status);
  }

  assert(replyLength == 0);
}

// plugin custom-ezsp get_mode
void sl_zigbee_af_custom_ezsp_get_power_mode_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_status_t status;
  // Custom command ID (1 byte)
  uint8_t message[1];
  uint8_t replyLength = 1;
  uint8_t reply[1];

  message[0] = SL_ZIGBEE_CUSTOM_EZSP_COMMAND_GET_POWER_MODE;

  status = sl_zigbee_ezsp_custom_frame(1, message, &replyLength, reply);

  if (status == SL_STATUS_OK) {
    assert(replyLength == 1);

    if (reply[0] == SL_ZIGBEE_XNCP_NORMAL_MODE) {
      sl_zigbee_af_core_println("Power mode: NORMAL");
    } else if (reply[0] == SL_ZIGBEE_XNCP_LOW_POWER_MODE) {
      sl_zigbee_af_core_println("Power mode: LOW_POWER");
    } else {
      sl_zigbee_af_core_println("Power mode: UNDEFINED");
    }
  } else {
    sl_zigbee_af_core_println("Command failed, status:0x%02X", status);
  }
}

// plugin custom-ezsp add_cluster <clusterId:2>
void sl_zigbee_af_custom_ezsp_add_cluster_to_filtering_list_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  // Custom command ID (1 byte) + cluster ID (2 bytes)
  uint8_t message[3];
  uint16_t clusterId = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t replyLength = 0;

  message[0] = SL_ZIGBEE_CUSTOM_EZSP_COMMAND_ADD_CLUSTER_TO_FILTERING_LIST;
  message[1] = LOW_BYTE(clusterId);
  message[2] = HIGH_BYTE(clusterId);

  status = sl_zigbee_ezsp_custom_frame(3, message, &replyLength, NULL);

  if (status == SL_STATUS_OK) {
    assert(replyLength == 0);
    sl_zigbee_af_core_println("Cluster ADD success");
  } else {
    sl_zigbee_af_core_println("Cluster ADD failed, status:0x%02X", status);
  }
}

// plugin custom-ezsp rem_cluster <clusterId:2>
void sl_zigbee_af_custom_ezsp_remove_cluster_to_filtering_list_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  // Custom command ID (1 byte) + cluster ID (2 bytes)
  uint8_t message[3];
  uint16_t clusterId = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t replyLength = 0;

  message[0] = SL_ZIGBEE_CUSTOM_EZSP_COMMAND_REMOVE_CLUSTER_TO_FILTERING_LIST;
  message[1] = LOW_BYTE(clusterId);
  message[2] = HIGH_BYTE(clusterId);

  status = sl_zigbee_ezsp_custom_frame(3, message, &replyLength, NULL);

  if (status == SL_STATUS_OK) {
    assert(replyLength == 0);
    sl_zigbee_af_core_println("Cluster REMOVE success");
  } else {
    sl_zigbee_af_core_println("Cluster REMOVE failed, status:0x%02X", status);
  }
}

// plugin custom-ezsp print_list
void sl_zigbee_af_custom_ezsp_print_cluster_filtering_list_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_status_t status;
  // Custom command ID (1 byte)
  uint8_t message[1];
  uint8_t replyLength = SL_ZIGBEE_MAX_CUSTOM_EZSP_MESSAGE_PAYLOAD;
  uint8_t reply[SL_ZIGBEE_MAX_CUSTOM_EZSP_MESSAGE_PAYLOAD];
  uint8_t i;

  message[0] = SL_ZIGBEE_CUSTOM_EZSP_COMMAND_GET_CLUSTER_FILTERING_LIST;

  status = sl_zigbee_ezsp_custom_frame(1, message, &replyLength, reply);

  if (status == SL_STATUS_OK) {
    sl_zigbee_af_core_println("Cluster filtering list, size=0x%02X", reply[0]);

    // First byte of the reply payload is the number of entries
    for (i = 1; i < replyLength; i += 2) {
      sl_zigbee_af_core_println("Cluster filtering entry -> Cluster ID 0x%04X",
                                HIGH_LOW_TO_INT(reply[i + 1], reply[i]));
    }
  } else {
    sl_zigbee_af_core_println("Command failed, status:0x%02X", status);
  }
}

// plugin custom-ezsp reports_on <time:2>
void sl_zigbee_af_custom_ezsp_enable_reports_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  // Custom command ID (1 byte) + report time (seconds) (2 bytes)
  uint8_t message[3];
  uint16_t time = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t replyLength = 0;

  message[0] = SL_ZIGBEE_CUSTOM_EZSP_COMMAND_ENABLE_PERIODIC_REPORTS;
  message[1] = LOW_BYTE(time);
  message[2] = HIGH_BYTE(time);

  status = sl_zigbee_ezsp_custom_frame(3, message, &replyLength, NULL);

  if (status == SL_STATUS_OK) {
    assert(replyLength == 0);
    sl_zigbee_af_core_println("Reports enabled");
  } else {
    sl_zigbee_af_core_println("Reports enabling failed, status:0x%02X", status);
  }
}

// plugin custom-ezsp reports_off
void sl_zigbee_af_custom_ezsp_disable_reports_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_status_t status;
  // Custom command ID (1 byte)
  uint8_t message[1];
  uint8_t replyLength = 0;

  message[0] = SL_ZIGBEE_CUSTOM_EZSP_COMMAND_DISABLE_PERIODIC_REPORTS;

  status = sl_zigbee_ezsp_custom_frame(1, message, &replyLength, NULL);

  if (status == SL_STATUS_OK) {
    assert(replyLength == 0);
    sl_zigbee_af_core_println("Reports disabled");
  } else {
    sl_zigbee_af_core_println("Reports disabling failed, status:0x%02X", status);
  }
}

// plugin custom-ezsp set_token <nodeType:1> <nodeId:2> <panId:2>
void sl_zigbee_af_custom_ezsp_set_custom_token_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  // Custom command ID (1 byte) + nodeType (1 byte) + nodeID (2 bytes) +
  // + panID (2 bytes)
  uint8_t message[6];
  uint8_t nodeType = sl_cli_get_argument_uint8(arguments, 0);
  uint16_t nodeId = sl_cli_get_argument_uint16(arguments, 1);
  uint16_t panId = sl_cli_get_argument_uint16(arguments, 2);
  uint8_t replyLength = 0;

  message[0] = SL_ZIGBEE_CUSTOM_EZSP_COMMAND_SET_CUSTOM_TOKEN;
  message[1] = nodeType;
  message[2] = LOW_BYTE(nodeId);
  message[3] = HIGH_BYTE(nodeId);
  message[4] = LOW_BYTE(panId);
  message[5] = HIGH_BYTE(panId);

  status = sl_zigbee_ezsp_custom_frame(6, message, &replyLength, NULL);

  if (status == SL_STATUS_OK) {
    assert(replyLength == 0);
    sl_zigbee_af_core_println("Custom token set");
  } else {
    sl_zigbee_af_core_println("Custom token set failed, status:0x%02X", status);
  }
}

// plugin custom-ezsp get_token
void sl_zigbee_af_custom_ezsp_get_custom_token_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_status_t status;
  // Custom command ID (1 byte)
  uint8_t message[1];
  uint8_t replyLength = 5;
  // nodeType (1 byte) + nodeID (2 bytes) + panID (2 bytes)
  uint8_t reply[5];

  message[0] = SL_ZIGBEE_CUSTOM_EZSP_COMMAND_GET_CUSTOM_TOKEN;

  status = sl_zigbee_ezsp_custom_frame(1, message, &replyLength, reply);

  if (status == SL_STATUS_OK) {
    assert(replyLength == 5);
    sl_zigbee_af_core_println("Custom token - nodeType:0x%02X nodeId:0x%04X panId:0x%04X",
                              reply[0],
                              HIGH_LOW_TO_INT(reply[1], reply[2]),
                              HIGH_LOW_TO_INT(reply[3], reply[4]));
  } else {
    sl_zigbee_af_core_println("Custom token read failed, status:0x%02X", status);
  }
}

// plugin custom-ezsp get_info
void sl_zigbee_af_custom_ezsp_get_x_ncp_info_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_status_t status;
  uint16_t manufacturerId;
  uint16_t versionNumber;

  status = sl_zigbee_ezsp_get_xncp_info(&manufacturerId, &versionNumber);

  if (status == SL_STATUS_OK) {
    sl_zigbee_af_core_println("XNCP library present, manufacturer ID:0x%04X version:0x%04X",
                              manufacturerId,
                              versionNumber);
  } else {
    sl_zigbee_af_core_println("XNCP library not present");
  }
}
