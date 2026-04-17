/***************************************************************************//**
 * @file
 * @brief Routines for the Groups Server plugin, the server implementation of
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
// * groups-server.c
// *
// *
// * Copyright 2010 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/framework/plugin/groups-server/groups-server.h"
#include "zap-cluster-command-parser.h"
#include "groups-server-common.h"

#define GROUP_NAME_SUPPORT_ENABLED_BIT (0x80)
#define GROUP_ID_MIN                   (0x0001U)
#define GROUP_ID_MAX                   (0xfff7U)
#define isInvalidGroupId(groupId) ((groupId) < GROUP_ID_MIN || (groupId) > GROUP_ID_MAX)

static uint8_t findGroupIndex(uint8_t endpoint, uint16_t groupId);

void sl_zigbee_af_groups_cluster_server_init_cb(uint8_t endpoint)
{
  // The high bit of Name Support indicates whether group names are supported.
  // Group names are not supported by this plugin.
  sl_zigbee_af_status_t status;
  uint8_t nameSupport = 0;
  if (sl_zigbee_af_groups_server_group_names_supported_cb(endpoint)) {
    nameSupport |= GROUP_NAME_SUPPORT_ENABLED_BIT;
  }
  status = sl_zigbee_af_write_attribute(endpoint,
                                        ZCL_GROUPS_CLUSTER_ID,
                                        ZCL_GROUP_NAME_SUPPORT_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER,
                                        (uint8_t *)&nameSupport,
                                        ZCL_BITMAP8_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_groups_cluster_println("ERR: writing name support %02X", status);
  }
}

// --------------------------
// Internal functions used to maintain the group table within the context
// of the binding table.
//
// In the binding:
// The first two bytes of the identifier is set to the groupId
// The local endpoint is set to the endpoint that is mapped to this group
// --------------------------
static sl_zigbee_af_status_t addEntryToGroupTable(uint8_t endpoint, uint16_t groupId, uint8_t *groupName)
{
  uint8_t i;

  // Validate groupId
  if (isInvalidGroupId(groupId)) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }
  // Check for duplicates.
  if (sli_zigbee_af_groups_cluster_is_group_present(endpoint, groupId)) {
    // Even if the group already exists, tell the application about the name,
    // so it can cope with renames.
    sl_zigbee_af_groups_server_set_group_name_cb(endpoint,
                                                 groupId,
                                                 groupName);
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  // Look for an empty binding slot.
  for (i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
    sl_zigbee_binding_table_entry_t binding;
    if (sl_zigbee_get_binding(i, &binding) == SL_STATUS_OK
        && binding.type == SL_ZIGBEE_UNUSED_BINDING) {
      sl_status_t status;
      binding.type = SL_ZIGBEE_MULTICAST_BINDING;
      binding.identifier[0] = LOW_BYTE(groupId);
      binding.identifier[1] = HIGH_BYTE(groupId);
      binding.local = endpoint;

      status = sl_zigbee_set_binding(i, &binding);
      if (status == SL_STATUS_OK) {
        // Set the group name, if supported
        sl_zigbee_af_groups_server_set_group_name_cb(endpoint,
                                                     groupId,
                                                     groupName);
        return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
      } else {
        sl_zigbee_af_groups_cluster_println("ERR: Failed to create binding (0x%02X)",
                                            status);
      }
    }
  }
  sl_zigbee_af_groups_cluster_println("ERR: Binding table is full");
  return SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
}

static sl_zigbee_af_status_t removeEntryFromGroupTable(uint8_t endpoint, uint16_t groupId)
{
  // Validate groupId
  if (isInvalidGroupId(groupId)) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }
  if (sli_zigbee_af_groups_cluster_is_group_present(endpoint, groupId)) {
    uint8_t bindingIndex = findGroupIndex(endpoint, groupId);
    sl_status_t status = sl_zigbee_delete_binding(bindingIndex);
    if (status == SL_STATUS_OK) {
      uint8_t groupName[ZCL_GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH + 1] = { 0 };
      sl_zigbee_af_groups_server_set_group_name_cb(endpoint,
                                                   groupId,
                                                   groupName);
      return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    } else {
      sl_zigbee_af_groups_cluster_println("ERR: Failed to delete binding (0x%02X)",
                                          status);
      return SL_ZIGBEE_ZCL_STATUS_FAILURE;
    }
  }
  return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_groups_cluster_add_group_cb(sl_zigbee_af_cluster_command_t * cmd)
{
  sl_zcl_groups_cluster_add_group_command_t cmd_data;
  sl_zigbee_af_status_t status;

  if (zcl_decode_groups_cluster_add_group_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_groups_cluster_print("RX: AddGroup 0x%04X, \"", cmd_data.groupId);
  sl_zigbee_af_groups_cluster_print_string(cmd_data.groupName);
  sl_zigbee_af_groups_cluster_println("\"");

  status = addEntryToGroupTable(sl_zigbee_af_current_endpoint(), cmd_data.groupId, cmd_data.groupName);

  // For all networks, Add Group commands are only responded to when
  // they are addressed to a single device.
  if (sl_zigbee_af_current_command()->type != SL_ZIGBEE_INCOMING_UNICAST
      && sl_zigbee_af_current_command()->type != SL_ZIGBEE_INCOMING_UNICAST_REPLY) {
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }
  sl_zigbee_af_fill_command_groups_cluster_add_group_response(status, cmd_data.groupId);
  sl_zigbee_af_send_response();
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_groups_cluster_view_group_cb(sl_zigbee_af_cluster_command_t * cmd)
{
  sl_zcl_groups_cluster_view_group_command_t cmd_data;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  sl_status_t sendStatus;
  uint8_t groupName[ZCL_GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH + 1] = { 0 };

  if (zcl_decode_groups_cluster_view_group_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_groups_cluster_println("RX: ViewGroup 0x%04X", cmd_data.groupId);

  // For all networks, View Group commands can only be addressed to a
  // single device.
  if (sl_zigbee_af_current_command()->type != SL_ZIGBEE_INCOMING_UNICAST
      && sl_zigbee_af_current_command()->type != SL_ZIGBEE_INCOMING_UNICAST_REPLY) {
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  // Validate groupId
  if (isInvalidGroupId(cmd_data.groupId)) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  } else {
    if (sli_zigbee_af_groups_cluster_is_group_present(sl_zigbee_af_current_endpoint(), cmd_data.groupId)) {
      status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
      // Get the group name, if supported
      sl_zigbee_af_groups_server_get_group_name_cb(sl_zigbee_af_current_endpoint(),
                                                   cmd_data.groupId,
                                                   groupName);
    }
  }

  sl_zigbee_af_fill_command_groups_cluster_view_group_response(status, cmd_data.groupId, groupName);
  sendStatus = sl_zigbee_af_send_response();
  if (SL_STATUS_OK != sendStatus) {
    sl_zigbee_af_groups_cluster_println("Groups: failed to send %s response: 0x%02X",
                                        "view_group",
                                        sendStatus);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_groups_cluster_get_group_membership_cb(sl_zigbee_af_cluster_command_t * cmd)
{
  sl_zcl_groups_cluster_get_group_membership_command_t cmd_data;
  sl_status_t status;
  uint8_t i, j;
  uint8_t count = 0;
  uint8_t list[SL_ZIGBEE_BINDING_TABLE_SIZE << 1];
  uint8_t listLen = 0;

  if (zcl_decode_groups_cluster_get_group_membership_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_groups_cluster_print("RX: GetGroupMembership 0x%02X,", cmd_data.groupCount);
  for (i = 0; i < cmd_data.groupCount; i++) {
    sl_zigbee_af_groups_cluster_print(" [0x%04X]",
                                      sl_zigbee_af_get_int16u(cmd_data.groupList + (i << 1), 0, 2));
  }
  sl_zigbee_af_groups_cluster_println("");

  // For all networks, Get Group Membership commands may be sent either
  // unicast or broadcast (removing the ZLL-specific limitation to unicast).

  // When Group Count is zero, respond with a list of all active groups.
  // Otherwise, respond with a list of matches.
  if (cmd_data.groupCount == 0) {
    for (i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
      sl_zigbee_binding_table_entry_t entry;
      status = sl_zigbee_get_binding(i, &entry);
      if ((status == SL_STATUS_OK)
          && (entry.type == SL_ZIGBEE_MULTICAST_BINDING)
          && (entry.local == sl_zigbee_af_current_endpoint())) {
        list[listLen]     = entry.identifier[0];
        list[listLen + 1] = entry.identifier[1];
        listLen += 2;
        count++;
      }
    }
  } else {
    for (i = 0; i < cmd_data.groupCount; i++) {
      uint16_t groupId = sl_zigbee_af_get_int16u(cmd_data.groupList + (i << 1), 0, 2);
      for (j = 0; j < SL_ZIGBEE_BINDING_TABLE_SIZE; j++) {
        sl_zigbee_binding_table_entry_t entry;
        status = sl_zigbee_get_binding(j, &entry);
        if ((status == SL_STATUS_OK)
            && (entry.type == SL_ZIGBEE_MULTICAST_BINDING)) {
          if (entry.local == sl_zigbee_af_current_endpoint()
              && entry.identifier[0] == LOW_BYTE(groupId)
              && entry.identifier[1] == HIGH_BYTE(groupId)) {
            list[listLen]     = entry.identifier[0];
            list[listLen + 1] = entry.identifier[1];
            listLen += 2;
            count++;
          }
        }
      }
    }
  }

  // Only send a response if the Group Count was zero or if one or more active
  // groups matched.  Otherwise, a Default Response is sent.
  if (cmd_data.groupCount == 0 || count != 0) {
    // A capacity of 0xFF means that it is unknown if any further groups may be
    // added.  Each group requires a binding and, because the binding table is
    // used for other purposes besides groups, we can't be sure what the
    // capacity will be in the future.
    sl_zigbee_af_fill_command_groups_cluster_get_group_membership_response(0xFF, // capacity
                                                                           count,
                                                                           list,
                                                                           listLen);
    status = sl_zigbee_af_send_response();
    if (SL_STATUS_OK != status) {
      sl_zigbee_af_groups_cluster_println("Groups: failed to send %s: 0x%02X",
                                          (cmd_data.groupCount == 0 || count != 0)
                                          ? "get_group_membership response"
                                          : "default_response",
                                          status);
    }
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }
  return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_groups_cluster_remove_group_cb(sl_zigbee_af_cluster_command_t * cmd)
{
  sl_zcl_groups_cluster_remove_group_command_t cmd_data;
  sl_zigbee_af_status_t status;
  sl_status_t sendStatus;

  if (zcl_decode_groups_cluster_remove_group_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_groups_cluster_println("RX: RemoveGroup 0x%04X", cmd_data.groupId);

  status = removeEntryFromGroupTable(sl_zigbee_af_current_endpoint(),
                                     cmd_data.groupId);

  // For all networks, Remove Group commands are only responded to when
  // they are addressed to a single device.
  if (sl_zigbee_af_current_command()->type != SL_ZIGBEE_INCOMING_UNICAST
      && sl_zigbee_af_current_command()->type != SL_ZIGBEE_INCOMING_UNICAST_REPLY) {
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  // EMAPPFWKV2-1414: if we remove a group, we should remove any scene
  // associated with it. ZCL6: 3.6.2.3.5: "Note that if a group is
  // removed the scenes associated with that group SHOULD be removed."
  sl_zigbee_af_scenes_cluster_remove_scenes_in_group_cb(sl_zigbee_af_current_endpoint(),
                                                        cmd_data.groupId);

  sl_zigbee_af_fill_command_groups_cluster_remove_group_response(status, cmd_data.groupId);
  sendStatus = sl_zigbee_af_send_response();
  if (SL_STATUS_OK != sendStatus) {
    sl_zigbee_af_groups_cluster_println("Groups: failed to send %s response: 0x%02X",
                                        "remove_group",
                                        sendStatus);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_groups_cluster_remove_all_groups_cb(void)
{
  uint8_t i, endpoint = sl_zigbee_af_current_endpoint();
  bool success = true;

  sl_zigbee_af_groups_cluster_println("RX: RemoveAllGroups");

  for (i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
    sl_zigbee_binding_table_entry_t binding;
    if (sl_zigbee_get_binding(i, &binding) == SL_STATUS_OK) {
      if (binding.type == SL_ZIGBEE_MULTICAST_BINDING
          && endpoint == binding.local) {
        sl_status_t status = sl_zigbee_delete_binding(i);
        if (status != SL_STATUS_OK) {
          success = false;
          sl_zigbee_af_groups_cluster_println("ERR: Failed to delete binding (0x%02X)",
                                              status);
        } else {
          uint8_t groupName[ZCL_GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH + 1] = { 0 };
          uint16_t groupId = HIGH_LOW_TO_INT(binding.identifier[1],
                                             binding.identifier[0]);
          sl_zigbee_af_groups_server_set_group_name_cb(endpoint,
                                                       groupId,
                                                       groupName);
          success = true && success;

          // EMAPPFWKV2-1414: if we remove a group, we should remove any scene
          // associated with it. ZCL6: 3.6.2.3.5: "Note that if a group is
          // removed the scenes associated with that group SHOULD be removed."
          sl_zigbee_af_scenes_cluster_remove_scenes_in_group_cb(endpoint, groupId);
        }
      }
    }
  }

  sl_zigbee_af_scenes_cluster_remove_scenes_in_group_cb(sl_zigbee_af_current_endpoint(),
                                                        ZCL_SCENES_GLOBAL_SCENE_GROUP_ID);

  return success ? SL_ZIGBEE_ZCL_STATUS_SUCCESS : SL_ZIGBEE_ZCL_STATUS_FAILURE;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_groups_cluster_add_group_if_identifying_cb(sl_zigbee_af_cluster_command_t * cmd)
{
  sl_zcl_groups_cluster_add_group_if_identifying_command_t cmd_data;
  sl_zigbee_af_status_t status;

  if (zcl_decode_groups_cluster_add_group_if_identifying_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS ) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_groups_cluster_print("RX: AddGroupIfIdentifying 0x%04X, \"", cmd_data.groupId);
  sl_zigbee_af_groups_cluster_print_string(cmd_data.groupName);
  sl_zigbee_af_groups_cluster_println("\"");

  if (!sl_zigbee_af_is_device_identifying(sl_zigbee_af_current_endpoint())) {
    // If not identifying, ignore add group -> success; not a failure.
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  } else {
    status = addEntryToGroupTable(sl_zigbee_af_current_endpoint(),
                                  cmd_data.groupId,
                                  cmd_data.groupName);
  }

  return status;
}

void sl_zigbee_af_groups_cluster_clear_group_table_cb(uint8_t endpoint)
{
  uint8_t i, networkIndex = sl_zigbee_get_current_network();
  for (i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
    sl_zigbee_binding_table_entry_t binding;
    if (sl_zigbee_get_binding(i, &binding) == SL_STATUS_OK
        && binding.type == SL_ZIGBEE_MULTICAST_BINDING
        && (endpoint == binding.local
            || (endpoint == SL_ZIGBEE_BROADCAST_ENDPOINT
                && networkIndex == binding.networkIndex))) {
      sl_status_t status = sl_zigbee_delete_binding(i);
      if (status != SL_STATUS_OK) {
        sl_zigbee_af_groups_cluster_println("ERR: Failed to delete binding (0x%02X)",
                                            status);
      }
    }
  }
}

static uint8_t findGroupIndex(uint8_t endpoint, uint16_t groupId)
{
  sl_status_t status;
  uint8_t i;

  for (i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
    sl_zigbee_binding_table_entry_t entry;
    status = sl_zigbee_get_binding(i, &entry);
    if ((status == SL_STATUS_OK)
        && sli_zigbee_af_groups_cluster_binding_group_match(endpoint, groupId, &entry)) {
      return i;
    }
  }
  return SL_ZIGBEE_AF_GROUP_TABLE_NULL_INDEX;
}

uint32_t sl_zigbee_af_groups_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                          sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_ADD_GROUP_COMMAND_ID:
      {
        status = sl_zigbee_af_groups_cluster_add_group_cb(cmd);
        break;
      }
      case ZCL_ADD_GROUP_IF_IDENTIFYING_COMMAND_ID:
      {
        status = sl_zigbee_af_groups_cluster_add_group_if_identifying_cb(cmd);
        break;
      }
      case ZCL_GET_GROUP_MEMBERSHIP_COMMAND_ID:
      {
        status = sl_zigbee_af_groups_cluster_get_group_membership_cb(cmd);
        break;
      }
      case ZCL_REMOVE_ALL_GROUPS_COMMAND_ID:
      {
        status = sl_zigbee_af_groups_cluster_remove_all_groups_cb();
        break;
      }
      case ZCL_REMOVE_GROUP_COMMAND_ID:
      {
        status = sl_zigbee_af_groups_cluster_remove_group_cb(cmd);
        break;
      }
      case ZCL_VIEW_GROUP_COMMAND_ID:
      {
        status = sl_zigbee_af_groups_cluster_view_group_cb(cmd);
        break;
      }
    }
  }

  return status;
}
