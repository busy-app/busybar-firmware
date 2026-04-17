/***************************************************************************//**
 * @file
 * @brief Routines for the Groups Client plugin, the client implementation of
 *        the Groups cluster.
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

// *******************************************************************
// * groups-client.c
// *
// *
// * Copyright 2010 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "../../include/af.h"

#include "zap-cluster-command-parser.h"

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_groups_cluster_add_group_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;

#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
  sl_zcl_groups_cluster_add_group_response_command_t cmd_data;

  if (zcl_decode_groups_cluster_add_group_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS ) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_groups_cluster_println("RX: AddGroupResponse 0x%02X, 0x%04X",
                                      cmd_data.status,
                                      cmd_data.groupId);
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_groups_cluster_view_group_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;

#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
  sl_zcl_groups_cluster_view_group_response_command_t cmd_data;

  if (zcl_decode_groups_cluster_view_group_response_command(cmd, &cmd_data) != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_groups_cluster_print("RX: ViewGroupResponse 0x%02X, 0x%04X, \"",
                                    cmd_data.status,
                                    cmd_data.groupId);
  sl_zigbee_af_groups_cluster_print_string(cmd_data.groupName);
  sl_zigbee_af_groups_cluster_println("\"");
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_groups_cluster_get_group_membership_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;

#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
  sl_zcl_groups_cluster_get_group_membership_response_command_t cmd_data;
  uint8_t i;

  if (zcl_decode_groups_cluster_get_group_membership_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_groups_cluster_print("RX: GetGroupMembershipResponse 0x%02X, 0x%02X,",
                                    cmd_data.capacity,
                                    cmd_data.groupCount);
  for (i = 0; i < cmd_data.groupCount; i++) {
    sl_zigbee_af_groups_cluster_print(" [0x%04X]",
                                      sl_zigbee_af_get_int16u(cmd_data.groupList + (i << 1), 0, 2));
  }
  sl_zigbee_af_groups_cluster_println("");
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_groups_cluster_remove_group_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;

#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
  sl_zcl_groups_cluster_remove_group_response_command_t cmd_data;

  if (zcl_decode_groups_cluster_remove_group_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_groups_cluster_println("RX: RemoveGroupResponse 0x%02X, 0x%04X",
                                      cmd_data.status,
                                      cmd_data.groupId);
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

uint32_t sl_zigbee_af_groups_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                          sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_ADD_GROUP_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_groups_cluster_add_group_response_cb(cmd);
        break;
      }
      case ZCL_GET_GROUP_MEMBERSHIP_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_groups_cluster_get_group_membership_response_cb(cmd);
        break;
      }
      case ZCL_REMOVE_GROUP_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_groups_cluster_remove_group_response_cb(cmd);
        break;
      }
      case ZCL_VIEW_GROUP_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_groups_cluster_view_group_response_cb(cmd);
        break;
      }
    }
  }

  return status;
}
