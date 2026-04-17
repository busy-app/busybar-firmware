/***************************************************************************//**
 * @file
 * @brief Routines for the ZLL Utility Server plugin.
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
#include "app/framework/util/common.h"

#ifdef SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_SERVER_PRESENT
#include "app/framework/plugin/zll-commissioning-common/zll-commissioning.h"
#include "app/framework/plugin/zll-commissioning-server/zll-commissioning-server.h"
#endif // SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_SERVER_PRESENT

#include "zap-cluster-command-parser.h"

static uint16_t getMaxLength(void)
{
  // Capturing the value of sl_zigbee_af_maximum_aps_payload_length in an uint16_t
  // even though sl_zigbee_af_maximum_aps_payload_length returns an uint8_t because
  // it needs to be comapred with SL_ZIGBEE_AF_RESPONSE_BUFFER_LEN(ranging from 74-10000)
  // to set the maxLength.
  uint16_t maxLength = sl_zigbee_af_maximum_aps_payload_length(SL_ZIGBEE_OUTGOING_DIRECT,
                                                               sl_zigbee_af_response_destination,
                                                               &sl_zigbee_af_response_aps_frame);
  if (SL_ZIGBEE_AF_RESPONSE_BUFFER_LEN < maxLength) {
    maxLength = SL_ZIGBEE_AF_RESPONSE_BUFFER_LEN;
  }
  return maxLength;
}

#ifndef SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_SERVER_PRESENT
WEAK(uint8_t sl_zigbee_af_zll_commissioning_server_group_identifier_count_cb(uint8_t endpoint))
{
  (void)endpoint;
  return 0x00;
}

WEAK(bool sl_zigbee_af_zll_commissioning_server_group_identifier_cb(uint8_t endpoint,
                                                                    uint8_t index,
                                                                    sl_zigbee_af_plugin_zll_commissioning_group_information_record_t *record))
{
  (void)endpoint;
  (void)index;
  (void)record;
  return false;
}

WEAK(uint8_t sl_zigbee_af_zll_commissioning_server_endpoint_information_count_cb(uint8_t endpoint))
{
  (void)endpoint;
  return 0x00;
}

WEAK(bool sl_zigbee_af_zll_commissioning_server_endpoint_information_cb(uint8_t endpoint,
                                                                        uint8_t index,
                                                                        sl_zigbee_af_plugin_zll_commissioning_endpoint_information_record_t *record))
{
  (void)endpoint;
  (void)index;
  (void)record;
  return false;
}
#endif // !SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_SERVER_PRESENT

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_zll_commissioning_cluster_get_group_identifiers_request_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_zll_commissioning_cluster_get_group_identifiers_request_command_t cmd_data;

  sl_status_t sendStatus;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t total = sl_zigbee_af_zll_commissioning_server_group_identifier_count_cb(endpoint);
  uint8_t i;
  uint8_t *count;
  uint16_t maxLength;

  if (zcl_decode_zll_commissioning_cluster_get_group_identifiers_request_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_zll_commissioning_cluster_println("RX: GetGroupIdentifiersRequest 0x%02X",
                                                 cmd_data.startIndex);

  (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                            | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                                            | SL_ZIGBEE_AF_DEFAULT_RESPONSE_POLICY_RESPONSES),
                                           ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                                           ZCL_GET_GROUP_IDENTIFIERS_RESPONSE_COMMAND_ID,
                                           "uu",
                                           total,
                                           cmd_data.startIndex);

  count = &appResponseData[appResponseLength];
  (void) sl_zigbee_af_put_int8u_in_resp(0); // temporary count

  maxLength = getMaxLength();
  for (i = cmd_data.startIndex; i < total && appResponseLength + 3 <= maxLength; i++) {
    sl_zigbee_af_plugin_zll_commissioning_group_information_record_t record;
    if (sl_zigbee_af_zll_commissioning_server_group_identifier_cb(endpoint,
                                                                  i,
                                                                  &record)) {
      (void) sl_zigbee_af_put_int16u_in_resp(record.groupId);
      (void) sl_zigbee_af_put_int8u_in_resp(record.groupType);
      (*count)++;
    }
  }

  sendStatus = sl_zigbee_af_send_response();
  if (SL_STATUS_OK != sendStatus) {
    sl_zigbee_af_zll_commissioning_cluster_println("ZLL: failed to send %s response: "
                                                   "0x%02X",
                                                   "group_identifiers",
                                                   sendStatus);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_zll_commissioning_cluster_get_endpoint_list_request_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_zll_commissioning_cluster_get_endpoint_list_request_command_t cmd_data;

  sl_status_t sendStatus;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t total = sl_zigbee_af_zll_commissioning_server_endpoint_information_count_cb(endpoint);
  uint8_t i;
  uint8_t *count;
  uint16_t maxLength;

  if (zcl_decode_zll_commissioning_cluster_get_endpoint_list_request_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_zll_commissioning_cluster_println("RX: GetEndpointListRequest 0x%02X",
                                                 cmd_data.startIndex);

  (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                            | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                                            | SL_ZIGBEE_AF_DEFAULT_RESPONSE_POLICY_RESPONSES),
                                           ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                                           ZCL_GET_ENDPOINT_LIST_RESPONSE_COMMAND_ID,
                                           "uu",
                                           total,
                                           cmd_data.startIndex);

  count = &appResponseData[appResponseLength];
  (void) sl_zigbee_af_put_int8u_in_resp(0); // temporary count

  maxLength = getMaxLength();
  for (i = cmd_data.startIndex; i < total && appResponseLength + 8 <= maxLength; i++) {
    sl_zigbee_af_plugin_zll_commissioning_endpoint_information_record_t record;
    if (sl_zigbee_af_zll_commissioning_server_endpoint_information_cb(endpoint,
                                                                      i,
                                                                      &record)) {
      (void) sl_zigbee_af_put_int16u_in_resp(record.networkAddress);
      (void) sl_zigbee_af_put_int8u_in_resp(record.endpointId);
      (void) sl_zigbee_af_put_int16u_in_resp(record.profileId);
      (void) sl_zigbee_af_put_int16u_in_resp(record.deviceId);
      (void) sl_zigbee_af_put_int8u_in_resp(record.version);
      (*count)++;
    }
  }

  sendStatus = sl_zigbee_af_send_response();
  if (SL_STATUS_OK != sendStatus) {
    sl_zigbee_af_zll_commissioning_cluster_println("ZLL: failed to send %s response: "
                                                   "0x%02X",
                                                   "endpoint_list",
                                                   sendStatus);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

uint32_t sl_zigbee_af_zll_commissioning_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                                     sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_GET_ENDPOINT_LIST_REQUEST_COMMAND_ID:
      {
        status = sl_zigbee_af_zll_commissioning_cluster_get_endpoint_list_request_cb(cmd);
        break;
      }
      case ZCL_GET_GROUP_IDENTIFIERS_REQUEST_COMMAND_ID:
      {
        status = sl_zigbee_af_zll_commissioning_cluster_get_group_identifiers_request_cb(cmd);
        break;
      }
    }
  }

  return status;
}
