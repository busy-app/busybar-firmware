/***************************************************************************//**
 * @file
 * @brief Routines for the ZLL Scenes Server plugin.
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

#include "../../include/af.h"
#include "../scenes/scenes.h"

#include "zap-cluster-command-parser.h"

#define ZCL_SCENES_CLUSTER_MODE_COPY_ALL_SCENES_MASK BIT(0)

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_scenes_cluster_enhanced_add_scene_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_scenes_cluster_enhanced_add_scene_command_t cmd_data;

  if (zcl_decode_scenes_cluster_enhanced_add_scene_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  return sl_zigbee_af_scenes_server_parse_add_scene(cmd,
                                                    cmd_data.groupId,
                                                    cmd_data.sceneId,
                                                    cmd_data.transitionTime,
                                                    cmd_data.sceneName,
                                                    cmd_data.extensionFieldSets);
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_scenes_cluster_enhanced_view_scene_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_scenes_cluster_enhanced_view_scene_command_t cmd_data;

  if (zcl_decode_scenes_cluster_enhanced_view_scene_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  return sl_zigbee_af_scenes_server_parse_view_scene(cmd,
                                                     cmd_data.groupId,
                                                     cmd_data.sceneId);
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_scenes_cluster_copy_scene_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_scenes_cluster_copy_scene_command_t cmd_data;
  sl_status_t sendStatus;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  bool copyAllScenes;
  uint8_t i;

  if (zcl_decode_scenes_cluster_copy_scene_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  copyAllScenes = (cmd_data.mode & ZCL_SCENES_CLUSTER_MODE_COPY_ALL_SCENES_MASK);

  sl_zigbee_af_scenes_cluster_println("RX: CopyScene 0x%02X, 0x%04X, 0x%02X, 0x%04X, 0x%02X",
                                      cmd_data.mode,
                                      cmd_data.groupIdFrom,
                                      cmd_data.sceneIdFrom,
                                      cmd_data.groupIdTo,
                                      cmd_data.sceneIdTo);

  // If a group id is specified but this endpoint isn't in it, take no action.
  if ((cmd_data.groupIdFrom != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
       && !sl_zigbee_af_groups_cluster_endpoint_in_group_cb(sl_zigbee_af_current_endpoint(),
                                                            cmd_data.groupIdFrom))
      || (cmd_data.groupIdTo != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
          && !sl_zigbee_af_groups_cluster_endpoint_in_group_cb(sl_zigbee_af_current_endpoint(),
                                                               cmd_data.groupIdTo))) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    goto kickout;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE; i++) {
    sl_zigbee_af_scene_table_entry_t from;
    sl_zigbee_af_scenes_server_retrieve_scene_entry(from, i);
    if (from.endpoint == sl_zigbee_af_current_endpoint()
        && from.groupId == cmd_data.groupIdFrom
        && (copyAllScenes || from.sceneId == cmd_data.sceneIdFrom)) {
      uint8_t j, index = SL_ZIGBEE_AF_SCENE_TABLE_NULL_INDEX;
      for (j = 0; j < SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE; j++) {
        sl_zigbee_af_scene_table_entry_t to;
        if (i == j) {
          continue;
        }
        sl_zigbee_af_scenes_server_retrieve_scene_entry(to, j);
        if (to.endpoint == sl_zigbee_af_current_endpoint()
            && to.groupId == cmd_data.groupIdTo
            && to.sceneId == (copyAllScenes ? from.sceneId : cmd_data.sceneIdTo)) {
          index = j;
          break;
        } else if (index == SL_ZIGBEE_AF_SCENE_TABLE_NULL_INDEX
                   && to.endpoint == SL_ZIGBEE_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID) {
          index = j;
        }
      }

      // If the target index is still zero, the table is full.
      if (index == SL_ZIGBEE_AF_SCENE_TABLE_NULL_INDEX) {
        status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
        goto kickout;
      }

      // Save the "from" entry to the "to" index.  This makes a copy of "from"
      // with the correct group and scene ids and leaves the original in tact.
      from.groupId = cmd_data.groupIdTo;
      if (!copyAllScenes) {
        from.sceneId = cmd_data.sceneIdTo;
      }
      sl_zigbee_af_scenes_server_save_scene_entry(from, index);

      if (j != index) {
        sl_zigbee_af_scenes_server_incr_num_scene_entries_in_use();
        sl_zigbee_af_scenes_set_scene_count_attribute(sl_zigbee_af_current_endpoint(),
                                                      sl_zigbee_af_scenes_server_num_scene_entries_in_use());
      }

      // If we aren't copying all scenes, we can stop here.
      status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
      if (!copyAllScenes) {
        goto kickout;
      }
    }
  }

  kickout:
  // Copy Scene commands are only responded to when they are addressed to a
  // single device.
  if (sl_zigbee_af_current_command()->type == SL_ZIGBEE_INCOMING_UNICAST
      || sl_zigbee_af_current_command()->type == SL_ZIGBEE_INCOMING_UNICAST_REPLY) {
    sl_zigbee_af_fill_command_scenes_cluster_copy_scene_response(status,
                                                                 cmd_data.groupIdFrom,
                                                                 cmd_data.sceneIdFrom);
    sendStatus = sl_zigbee_af_send_response();
    if (SL_STATUS_OK != sendStatus) {
      sl_zigbee_af_scenes_cluster_println("Scenes: failed to send %s response: 0x%02X",
                                          "copy_scene",
                                          sendStatus);
    }
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_status_t sl_zigbee_af_zll_scenes_server_recall_scene_zll_extensions(uint8_t endpoint)
{
  bool globalSceneControl = true;
  sl_zigbee_af_status_t status = sl_zigbee_af_write_server_attribute(endpoint,
                                                                     ZCL_ON_OFF_CLUSTER_ID,
                                                                     ZCL_GLOBAL_SCENE_CONTROL_ATTRIBUTE_ID,
                                                                     (uint8_t *)&globalSceneControl,
                                                                     ZCL_BOOLEAN_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_scenes_cluster_println("ERR: writing global scene control %02X", status);
  }
  return status;
}

uint32_t sl_zigbee_af_zll_scenes_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                              sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_COPY_SCENE_COMMAND_ID:
      {
        status = sl_zigbee_af_scenes_cluster_copy_scene_cb(cmd);
        break;
      }
      case ZCL_ENHANCED_ADD_SCENE_COMMAND_ID:
      {
        status = sl_zigbee_af_scenes_cluster_enhanced_add_scene_cb(cmd);
        break;
      }
      case ZCL_ENHANCED_VIEW_SCENE_COMMAND_ID:
      {
        status = sl_zigbee_af_scenes_cluster_enhanced_view_scene_cb(cmd);
        break;
      }
    }
  }

  return status;
}
