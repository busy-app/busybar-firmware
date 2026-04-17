/***************************************************************************//**
 * @file
 * @brief Routines for the ZLL Scenes Client plugin.
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
#include "../scenes-client/scenes-client.h"

#include "zap-cluster-command-parser.h"

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_scenes_cluster_enhanced_add_scene_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_scenes_cluster_enhanced_add_scene_response_command_t cmd_data;

  if (zcl_decode_scenes_cluster_enhanced_add_scene_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  return sl_zigbee_af_scenes_client_parse_add_scene_response(cmd,
                                                             cmd_data.status,
                                                             cmd_data.groupId,
                                                             cmd_data.sceneId);
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_scenes_cluster_enhanced_view_scene_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_scenes_cluster_enhanced_view_scene_response_command_t cmd_data;

  if (zcl_decode_scenes_cluster_enhanced_view_scene_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  return sl_zigbee_af_scenes_client_parse_view_scene_response(cmd,
                                                              cmd_data.status,
                                                              cmd_data.groupId,
                                                              cmd_data.sceneId,
                                                              cmd_data.transitionTime,
                                                              cmd_data.sceneName,
                                                              cmd_data.extensionFieldSets);
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_scenes_cluster_copy_scene_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_scenes_cluster_copy_scene_response_command_t cmd_data;

  if (zcl_decode_scenes_cluster_copy_scene_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_scenes_cluster_println("RX: CopySceneResponse 0x%02X, 0x%04X, 0x%02X",
                                      cmd_data.status,
                                      cmd_data.groupIdFrom,
                                      cmd_data.sceneIdFrom);
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

uint32_t sl_zigbee_af_zll_scenes_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                              sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_COPY_SCENE_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_scenes_cluster_copy_scene_response_cb(cmd);
        break;
      }
      case ZCL_ENHANCED_ADD_SCENE_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_scenes_cluster_enhanced_add_scene_response_cb(cmd);
        break;
      }
      case ZCL_ENHANCED_VIEW_SCENE_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_scenes_cluster_enhanced_view_scene_response_cb(cmd);
        break;
      }
    }
  }

  return status;
}
