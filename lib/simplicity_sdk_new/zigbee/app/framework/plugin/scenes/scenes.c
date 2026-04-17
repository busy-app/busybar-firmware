/***************************************************************************//**
 * @file
 * @brief Routines for the Scenes plugin, which implements the server side of
 *        the Scenes cluster.
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
#include "../../util/common.h"
#include "scenes.h"
#include "sl_zigbee_token.h"

#include "zap-cluster-command-parser.h"
#ifdef SL_CATALOG_ZIGBEE_ZLL_SCENES_SERVER_PRESENT
#include "zll-scenes-server.h"
#endif

uint8_t sl_zigbee_af_scenes_server_entries_in_use = 0;
#if (SL_ZIGBEE_AF_PLUGIN_SCENES_USE_TOKENS == 0) || defined(EZSP_HOST)
sl_zigbee_af_scene_table_entry_t sl_zigbee_af_scenes_server_scene_table[SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE];
#endif // (SL_ZIGBEE_AF_PLUGIN_SCENES_USE_TOKENS == 0) || defined(EZSP_HOST)
#if defined(ZCL_USING_ON_OFF_CLUSTER_SERVER)         \
  || defined(ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER) \
  || defined(ZCL_USING_THERMOSTAT_CLUSTER_SERVER)    \
  || defined(ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER) \
  || defined(ZCL_USING_DOOR_LOCK_CLUSTER_SERVER)     \
  || defined(ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER)
static bool readServerAttribute(uint8_t endpoint,
                                sl_zigbee_af_cluster_id_t clusterId,
                                sl_zigbee_af_attribute_id_t attributeId,
                                const char * name,
                                uint8_t *data,
                                uint8_t size)
{
  bool success = false;
  if (sl_zigbee_af_contains_server(endpoint, clusterId)) {
    sl_zigbee_af_status_t status = sl_zigbee_af_read_server_attribute(endpoint,
                                                                      clusterId,
                                                                      attributeId,
                                                                      data,
                                                                      size);
    if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      success = true;
    } else {
      sl_zigbee_af_scenes_cluster_println("ERR: %sing %s 0x%02X", "read", name, status);
    }
  }
  return success;
}
#endif

static sl_zigbee_af_status_t writeServerAttribute(uint8_t endpoint,
                                                  sl_zigbee_af_cluster_id_t clusterId,
                                                  sl_zigbee_af_attribute_id_t attributeId,
                                                  const char * name,
                                                  uint8_t *data,
                                                  sl_zigbee_af_attribute_type_t type)
{
  sl_zigbee_af_status_t status = sl_zigbee_af_write_server_attribute(endpoint,
                                                                     clusterId,
                                                                     attributeId,
                                                                     data,
                                                                     type);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_scenes_cluster_println("ERR: %sing %s 0x%02X", "writ", name, status);
  }
  return status;
}

static sl_zigbee_af_status_t findScene(uint8_t endpoint,
                                       uint16_t groupId,
                                       uint8_t sceneId,
                                       sl_zigbee_af_scene_table_entry_t *sceneEntry)
{
  if (sceneEntry == NULL) {
    return SL_ZIGBEE_ZCL_STATUS_SOFTWARE_FAILURE;
  }

  if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !sl_zigbee_af_groups_cluster_endpoint_in_group_cb(endpoint, groupId)) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  }

  for (uint8_t i = 0; i < SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE; i++) {
    sl_zigbee_af_scenes_server_retrieve_scene_entry((*sceneEntry), i);
    if (sceneEntry->endpoint == endpoint
        && sceneEntry->groupId == groupId
        && sceneEntry->sceneId == sceneId) {
      return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    }
  }

  return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
}

void sl_zigbee_af_scenes_cluster_server_in(uint8_t endpoint)
{
#ifdef NAME_SUPPORT
  {
    // The high bit of Name Support indicates whether scene names are supported.
    uint8_t nameSupport = BIT(7);
    writeServerAttribute(endpoint,
                         ZCL_SCENES_CLUSTER_ID,
                         ZCL_SCENE_NAME_SUPPORT_ATTRIBUTE_ID,
                         "name support",
                         (uint8_t *)&nameSupport,
                         ZCL_BITMAP8_ATTRIBUTE_TYPE);
  }
#endif
#if (SL_ZIGBEE_AF_PLUGIN_SCENES_USE_TOKENS == 0) || defined(EZSP_HOST)
  {
    uint8_t i;
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE; i++) {
      sl_zigbee_af_scene_table_entry_t entry;
      sl_zigbee_af_scenes_server_retrieve_scene_entry(entry, i);
      entry.endpoint = SL_ZIGBEE_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID;
      sl_zigbee_af_scenes_server_save_scene_entry(entry, i);
    }
    sl_zigbee_af_scenes_server_set_num_scene_entries_in_use(0);
  }
#endif // (SL_ZIGBEE_AF_PLUGIN_SCENES_USE_TOKENS == 0) || defined(EZSP_HOST)
  sl_zigbee_af_scenes_set_scene_count_attribute(endpoint,
                                                sl_zigbee_af_scenes_server_num_scene_entries_in_use());
}

sl_zigbee_af_status_t sl_zigbee_af_scenes_set_scene_count_attribute(uint8_t endpoint,
                                                                    uint8_t newCount)
{
  return writeServerAttribute(endpoint,
                              ZCL_SCENES_CLUSTER_ID,
                              ZCL_SCENE_COUNT_ATTRIBUTE_ID,
                              "scene count",
                              (uint8_t *)&newCount,
                              ZCL_INT8U_ATTRIBUTE_TYPE);
}

sl_zigbee_af_status_t sl_zigbee_af_scenes_make_valid(uint8_t endpoint,
                                                     uint8_t sceneId,
                                                     uint16_t groupId)
{
  sl_zigbee_af_status_t status;
  bool valid = true;

  // scene ID
  status = writeServerAttribute(endpoint,
                                ZCL_SCENES_CLUSTER_ID,
                                ZCL_CURRENT_SCENE_ATTRIBUTE_ID,
                                "current scene",
                                (uint8_t *)&sceneId,
                                ZCL_INT8U_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }

  // group ID
  status = writeServerAttribute(endpoint,
                                ZCL_SCENES_CLUSTER_ID,
                                ZCL_CURRENT_GROUP_ATTRIBUTE_ID,
                                "current group",
                                (uint8_t *)&groupId,
                                ZCL_INT16U_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }

  status = writeServerAttribute(endpoint,
                                ZCL_SCENES_CLUSTER_ID,
                                ZCL_SCENE_VALID_ATTRIBUTE_ID,
                                "scene valid",
                                (uint8_t *)&valid,
                                ZCL_BOOLEAN_ATTRIBUTE_TYPE);
  return status;
}

sl_zigbee_af_status_t sl_zigbee_af_scenes_cluster_make_invalid_cb(uint8_t endpoint)
{
  bool valid = false;
  return writeServerAttribute(endpoint,
                              ZCL_SCENES_CLUSTER_ID,
                              ZCL_SCENE_VALID_ATTRIBUTE_ID,
                              "scene valid",
                              (uint8_t *)&valid,
                              ZCL_BOOLEAN_ATTRIBUTE_TYPE);
}

#if defined(SL_CATALOG_CLI_PRESENT)
void sli_plugin_scenes_server_print_info(sl_cli_command_arg_t *arguments)
#else
void sli_zigbee_af_scenes_server_print_info(void)
#endif // SL_CATALOG_CLI_PRESENT
{
  #if defined(SL_CATALOG_CLI_PRESENT)
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  uint8_t i;
  sl_zigbee_af_scene_table_entry_t entry;
  sl_zigbee_af_core_println("using 0x%02X out of 0x%02X table slots",
                            sl_zigbee_af_scenes_server_num_scene_entries_in_use(),
                            SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE);
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE; i++) {
    sl_zigbee_af_scenes_server_retrieve_scene_entry(entry, i);
    sl_zigbee_af_core_print("%02X: ", i);
    if (entry.endpoint != SL_ZIGBEE_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID) {
      sl_zigbee_af_core_print("ep %02X grp %04X scene %02X tt %d",
                              entry.endpoint,
                              entry.groupId,
                              entry.sceneId,
                              entry.transitionTime);
      sl_zigbee_af_core_print(".%d", entry.transitionTime100ms);
#ifdef NAME_SUPPORT
      sl_zigbee_af_core_print(" name(%02X)\"", sl_zigbee_af_string_length(entry.name));
      sl_zigbee_af_core_print_string(entry.name);
      sl_zigbee_af_core_print("\"");
#endif
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
      sl_zigbee_af_core_print(" on/off %02X", entry.onOffValue);
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
      sl_zigbee_af_core_print(" lvl %02X", entry.currentLevelValue);
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
      sl_zigbee_af_core_print(" therm %04X %04X %02X",
                              entry.occupiedCoolingSetpointValue,
                              entry.occupiedHeatingSetpointValue,
                              entry.systemModeValue);
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
      sl_zigbee_af_core_print(" color %04X %04X",
                              entry.currentXValue,
                              entry.currentYValue);
      sl_zigbee_af_core_print(" %04X %02X %02X %02X %04X",
                              entry.enhancedCurrentHueValue,
                              entry.currentSaturationValue,
                              entry.colorLoopActiveValue,
                              entry.colorLoopDirectionValue,
                              entry.colorLoopTimeValue,
                              entry.colorTemperatureMiredsValue);
      sl_zigbee_af_core_flush();
#endif //ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
      sl_zigbee_af_core_print(" door %02X", entry.lockStateValue);
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
      sl_zigbee_af_core_print(" window %02X %02X",
                              entry.currentPositionLiftPercentageValue,
                              entry.currentPositionTiltPercentageValue);
#endif
    }
    sl_zigbee_af_core_println("");
  }
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_scenes_cluster_add_scene_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_scenes_cluster_add_scene_command_t cmd_data;

  if (zcl_decode_scenes_cluster_add_scene_command(cmd, &cmd_data)
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

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_scenes_cluster_view_scene_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_scenes_cluster_view_scene_command_t cmd_data;

  if (zcl_decode_scenes_cluster_view_scene_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  return sl_zigbee_af_scenes_server_parse_view_scene(cmd,
                                                     cmd_data.groupId,
                                                     cmd_data.sceneId);
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_scenes_cluster_remove_scene_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_scenes_cluster_remove_scene_command_t cmd_data;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  sl_status_t sendStatus;

  if (zcl_decode_scenes_cluster_remove_scene_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_scenes_cluster_println("RX: RemoveScene 0x%04X, 0x%02X", cmd_data.groupId, cmd_data.sceneId);

  // If a group id is specified but this endpoint isn't in it, take no action.
  if (cmd_data.groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !sl_zigbee_af_groups_cluster_endpoint_in_group_cb(sl_zigbee_af_current_endpoint(),
                                                           cmd_data.groupId)) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else {
    uint8_t i;
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE; i++) {
      sl_zigbee_af_scene_table_entry_t entry;
      sl_zigbee_af_scenes_server_retrieve_scene_entry(entry, i);
      if (entry.endpoint == sl_zigbee_af_current_endpoint()
          && entry.groupId == cmd_data.groupId
          && entry.sceneId == cmd_data.sceneId) {
        entry.endpoint = SL_ZIGBEE_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID;
        sl_zigbee_af_scenes_server_save_scene_entry(entry, i);
        sl_zigbee_af_scenes_server_decr_num_scene_entries_in_use();
        sl_zigbee_af_scenes_set_scene_count_attribute(sl_zigbee_af_current_endpoint(),
                                                      sl_zigbee_af_scenes_server_num_scene_entries_in_use());
        status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
        break;
      }
    }
  }

  // Remove Scene commands are only responded to when they are addressed to a
  // single device.
  if (sl_zigbee_af_current_command()->type == SL_ZIGBEE_INCOMING_UNICAST
      || sl_zigbee_af_current_command()->type == SL_ZIGBEE_INCOMING_UNICAST_REPLY) {
    sl_zigbee_af_fill_command_scenes_cluster_remove_scene_response(status,
                                                                   cmd_data.groupId,
                                                                   cmd_data.sceneId);
    sendStatus = sl_zigbee_af_send_response();
    if (SL_STATUS_OK != sendStatus) {
      sl_zigbee_af_scenes_cluster_println("Scenes: failed to send %s response: 0x%02X",
                                          "remove_scene",
                                          sendStatus);
    }
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_scenes_cluster_remove_all_scenes_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_scenes_cluster_remove_all_scenes_command_t cmd_data;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  sl_status_t sendStatus;

  if (zcl_decode_scenes_cluster_remove_all_scenes_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_scenes_cluster_println("RX: RemoveAllScenes 0x%04X", cmd_data.groupId);

  if (cmd_data.groupId == ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      || sl_zigbee_af_groups_cluster_endpoint_in_group_cb(sl_zigbee_af_current_endpoint(),
                                                          cmd_data.groupId)) {
    uint8_t i;
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE; i++) {
      sl_zigbee_af_scene_table_entry_t entry;
      sl_zigbee_af_scenes_server_retrieve_scene_entry(entry, i);
      if (entry.endpoint == sl_zigbee_af_current_endpoint()
          && entry.groupId == cmd_data.groupId) {
        entry.endpoint = SL_ZIGBEE_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID;
        sl_zigbee_af_scenes_server_save_scene_entry(entry, i);
        sl_zigbee_af_scenes_server_decr_num_scene_entries_in_use();
      }
    }
    sl_zigbee_af_scenes_set_scene_count_attribute(sl_zigbee_af_current_endpoint(),
                                                  sl_zigbee_af_scenes_server_num_scene_entries_in_use());
  }

  // Remove All Scenes commands are only responded to when they are addressed
  // to a single device.
  if (sl_zigbee_af_current_command()->type == SL_ZIGBEE_INCOMING_UNICAST
      || sl_zigbee_af_current_command()->type == SL_ZIGBEE_INCOMING_UNICAST_REPLY) {
    sl_zigbee_af_fill_command_scenes_cluster_remove_all_scenes_response(status, cmd_data.groupId);
    sendStatus = sl_zigbee_af_send_response();
    if (SL_STATUS_OK != sendStatus) {
      sl_zigbee_af_scenes_cluster_println("Scenes: failed to send %s response: 0x%02X",
                                          "remove_all_scenes",
                                          sendStatus);
    }
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_scenes_cluster_store_scene_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_scenes_cluster_store_scene_command_t cmd_data;
  sl_zigbee_af_status_t status;
  sl_status_t sendStatus;

  if (zcl_decode_scenes_cluster_store_scene_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_scenes_cluster_println("RX: StoreScene 0x%04X, 0x%02X", cmd_data.groupId, cmd_data.sceneId);
  status = sl_zigbee_af_scenes_cluster_store_current_scene_cb(sl_zigbee_af_current_endpoint(),
                                                              cmd_data.groupId,
                                                              cmd_data.sceneId);

  // Store Scene commands are only responded to when they are addressed to a
  // single device.
  if (sl_zigbee_af_current_command()->type == SL_ZIGBEE_INCOMING_UNICAST
      || sl_zigbee_af_current_command()->type == SL_ZIGBEE_INCOMING_UNICAST_REPLY) {
    sl_zigbee_af_fill_command_scenes_cluster_store_scene_response(status, cmd_data.groupId, cmd_data.sceneId);
    sendStatus = sl_zigbee_af_send_response();
    if (SL_STATUS_OK != sendStatus) {
      sl_zigbee_af_scenes_cluster_println("Scenes: failed to send %s response: 0x%02X",
                                          "store_scene",
                                          sendStatus);
    }
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_scenes_cluster_recall_scene_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  // NOTE: TransitionTime field in the RecallScene command is currently
  // ignored. Per Zigbee Alliance ZCL 7 (07-5123-07):
  //
  // "The transition time determines how long the tranition takes from the
  // old cluster state to the new cluster state. It is recommended that, where
  // possible (e.g., it is not possible for attributes with Boolean type),
  // a gradual transition SHOULD take place from the old to the new state
  // over this time. However, the exact transition is manufacturer dependent."
  //
  // The default manufacturer-dependent implementation provided here immediately
  // sets all attributes to their scene-specified values, without regard to the
  // value of TransitionTime. This default treatment is applied if the call
  // to sl_zigbee_af_scenes_server_custom_recall_scene_cb() returns false,
  // indicating that no overriding customization is implemented.
  //
  // A product manufacturer can contribute a different (TransitionTime-aware)
  // implementation of sl_zigbee_af_scenes_server_custom_recall_scene_cb()
  // to customize the behavior of scene transition when the RecallScene

  sl_zcl_scenes_cluster_recall_scene_command_t cmd_data;
  sl_zigbee_af_scene_table_entry_t sceneEntry;
  sl_zigbee_af_status_t status;
  uint16_t payloadOffset = cmd->payloadStartIndex;

  if (zcl_decode_scenes_cluster_recall_scene_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  /** Note: Fields in the payload - 5 bytes in total
   * uint16_t groupId;  // Ver.: always
   * uint8_t sceneId;   // Ver.: always
   * uint16_t transitionTime;  // Ver.: not always present, since version zcl-7.0-07-5123-07 or higer
   * "When the command payload field not present or value equal to 0xFFFF" - from the spec 3.4.2.4.7.2
   * So if Transition field is missing, we set transitionTime to 0xFFFF
   */
  if (cmd->bufLen < payloadOffset + 5) {
    cmd_data.transitionTime = 0xFFFF;
  }

  sl_zigbee_af_scenes_cluster_println("RX: RecallScene 0x%04X, 0x%02X, 0x%04X",
                                      cmd_data.groupId,
                                      cmd_data.sceneId,
                                      cmd_data.transitionTime);
  status = findScene(sl_zigbee_af_current_endpoint(), cmd_data.groupId, cmd_data.sceneId, &sceneEntry);
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    // Valid scene, try custom callback
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
    if (!sl_zigbee_af_scenes_server_custom_recall_scene_cb(&sceneEntry,
                                                           cmd_data.transitionTime,
                                                           &status)) {
      // No custom callback, apply default handling
      status = sl_zigbee_af_scenes_cluster_recall_saved_scene_cb(sl_zigbee_af_current_endpoint(),
                                                                 cmd_data.groupId,
                                                                 cmd_data.sceneId);
    }
  }
#ifdef SL_CATALOG_ZIGBEE_ZLL_SCENES_SERVER_PRESENT
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_zll_scenes_server_recall_scene_zll_extensions(sl_zigbee_af_current_endpoint());
  }
#endif
  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_scenes_cluster_get_scene_membership_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_scenes_cluster_get_scene_membership_command_t cmd_data;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  sl_status_t sendStatus;
  uint8_t sceneCount = 0;

  if (zcl_decode_scenes_cluster_get_scene_membership_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_scenes_cluster_println("RX: GetSceneMembership 0x%04X", cmd_data.groupId);

  if (cmd_data.groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !sl_zigbee_af_groups_cluster_endpoint_in_group_cb(sl_zigbee_af_current_endpoint(),
                                                           cmd_data.groupId)) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  }

  // The status, capacity, and group id are always included in the response, but
  // the scene count and scene list are only included if the group id matched.
  (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                            | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                                            | SL_ZIGBEE_AF_DEFAULT_RESPONSE_POLICY_RESPONSES),
                                           ZCL_SCENES_CLUSTER_ID,
                                           ZCL_GET_SCENE_MEMBERSHIP_RESPONSE_COMMAND_ID,
                                           "uuv",
                                           status,
                                           (SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE
                                            - sl_zigbee_af_scenes_server_num_scene_entries_in_use()), // capacity
                                           cmd_data.groupId);
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    uint8_t i, sceneList[SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE];
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE; i++) {
      sl_zigbee_af_scene_table_entry_t entry;
      sl_zigbee_af_scenes_server_retrieve_scene_entry(entry, i);
      if (entry.endpoint == sl_zigbee_af_current_endpoint()
          && entry.groupId == cmd_data.groupId) {
        sceneList[sceneCount] = entry.sceneId;
        sceneCount++;
      }
    }
    (void) sl_zigbee_af_put_int8u_in_resp(sceneCount);
    for (i = 0; i < sceneCount; i++) {
      (void) sl_zigbee_af_put_int8u_in_resp(sceneList[i]);
    }
  }

  // Get Scene Membership commands are only responded to when they are
  // addressed to a single device or when an entry in the table matches.
  if (sl_zigbee_af_current_command()->type == SL_ZIGBEE_INCOMING_UNICAST
      || sl_zigbee_af_current_command()->type == SL_ZIGBEE_INCOMING_UNICAST_REPLY
      || sceneCount != 0) {
    sendStatus = sl_zigbee_af_send_response();
    if (SL_STATUS_OK != sendStatus) {
      sl_zigbee_af_scenes_cluster_println("Scenes: failed to send %s response: 0x%02X",
                                          "get_scene_membership",
                                          sendStatus);
    }
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

void sl_zigbee_af_scenes_cluster_clear_scene_table_cb(uint8_t endpoint)
{
  uint8_t i, networkIndex = sl_zigbee_get_current_network();
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE; i++) {
    sl_zigbee_af_scene_table_entry_t entry;
    sl_zigbee_af_scenes_server_retrieve_scene_entry(entry, i);
    if (entry.endpoint != SL_ZIGBEE_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID
        && (endpoint == entry.endpoint
            || (endpoint == SL_ZIGBEE_BROADCAST_ENDPOINT
                && (networkIndex
                    == sl_zigbee_af_network_index_from_endpoint(entry.endpoint))))) {
      entry.endpoint = SL_ZIGBEE_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID;
      sl_zigbee_af_scenes_server_save_scene_entry(entry, i);
    }
  }
  sl_zigbee_af_scenes_server_set_num_scene_entries_in_use(0);
  if (endpoint == SL_ZIGBEE_BROADCAST_ENDPOINT) {
    for (i = 0; i < sl_zigbee_af_endpoint_count(); i++) {
      if (sl_zigbee_af_network_index_from_endpoint_index(i) == networkIndex) {
        sl_zigbee_af_scenes_set_scene_count_attribute(sl_zigbee_af_endpoint_from_index(i), 0);
      }
    }
  } else {
    sl_zigbee_af_scenes_set_scene_count_attribute(endpoint, 0);
  }
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_scenes_server_parse_add_scene(const sl_zigbee_af_cluster_command_t *cmd,
                                                                             uint16_t groupId,
                                                                             uint8_t sceneId,
                                                                             uint16_t transitionTime,
                                                                             uint8_t *sceneName,
                                                                             uint8_t *extensionFieldSets)
{
  sl_zigbee_af_scene_table_entry_t entry;
  sl_zigbee_af_status_t status;
  sl_status_t sendStatus;
  bool enhanced = (cmd->commandId == ZCL_ENHANCED_ADD_SCENE_COMMAND_ID);
  uint16_t extensionFieldSetsLen = (cmd->bufLen
                                    - (cmd->payloadStartIndex
                                       + sizeof(groupId)
                                       + sizeof(sceneId)
                                       + sizeof(transitionTime)
                                       + sl_zigbee_af_string_length(sceneName) + 1));
  uint16_t extensionFieldSetsIndex = 0;
  uint8_t endpoint = cmd->apsFrame->destinationEndpoint;
  uint8_t i, index = SL_ZIGBEE_AF_SCENE_TABLE_NULL_INDEX;

  sl_zigbee_af_scenes_cluster_print("RX: %sAddScene 0x%04X, 0x%02X, 0x%04X, \"",
                                    (enhanced ? "Enhanced" : ""),
                                    groupId,
                                    sceneId,
                                    transitionTime);
  sl_zigbee_af_scenes_cluster_print_string(sceneName);
  sl_zigbee_af_scenes_cluster_print("\", ");
  sl_zigbee_af_scenes_cluster_print_buffer(extensionFieldSets, extensionFieldSetsLen, false);
  sl_zigbee_af_scenes_cluster_println("");

  // Add Scene commands can only reference groups to which we belong.
  if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !sl_zigbee_af_groups_cluster_endpoint_in_group_cb(endpoint, groupId)) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    goto kickout;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE; i++) {
    sl_zigbee_af_scenes_server_retrieve_scene_entry(entry, i);
    if (entry.endpoint == endpoint
        && entry.groupId == groupId
        && entry.sceneId == sceneId) {
      index = i;
      break;
    } else if (index == SL_ZIGBEE_AF_SCENE_TABLE_NULL_INDEX
               && entry.endpoint == SL_ZIGBEE_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID) {
      index = i;
    }
  }

  // If the target index is still zero, the table is full.
  if (index == SL_ZIGBEE_AF_SCENE_TABLE_NULL_INDEX) {
    status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
    goto kickout;
  }

  sl_zigbee_af_scenes_server_retrieve_scene_entry(entry, index);

  // The transition time is specified in seconds in the regular version of the
  // command and tenths of a second in the enhanced version.
  if (enhanced) {
    entry.transitionTime = transitionTime / 10;
    entry.transitionTime100ms = (uint8_t)(transitionTime - entry.transitionTime * 10);
  } else {
    entry.transitionTime = transitionTime;
    entry.transitionTime100ms = 0;
  }

#ifdef NAME_SUPPORT
  sl_zigbee_af_copy_string(entry.name, sceneName, ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH);
#endif

  // When adding a scene, always wipe out all of the extensions before parsing the
  // extension field sets data.
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
  entry.hasOnOffValue = false;
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
  entry.hasCurrentLevelValue = false;
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
  entry.hasOccupiedCoolingSetpointValue = false;
  entry.hasOccupiedHeatingSetpointValue = false;
  entry.hasSystemModeValue = false;
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
  entry.hasCurrentXValue = false;
  entry.hasCurrentYValue = false;
  entry.hasEnhancedCurrentHueValue = false;
  entry.hasCurrentSaturationValue = false;
  entry.hasColorLoopActiveValue = false;
  entry.hasColorLoopDirectionValue = false;
  entry.hasColorLoopTimeValue = false;
  entry.hasColorTemperatureMiredsValue = false;
#endif //ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
  entry.hasLockStateValue = false;
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
  entry.hasCurrentPositionLiftPercentageValue = false;
  entry.hasCurrentPositionTiltPercentageValue = false;
#endif

  while (extensionFieldSetsIndex < extensionFieldSetsLen) {
    sl_zigbee_af_cluster_id_t clusterId;
    uint8_t length;

    // Each extension field set must contain a two-byte cluster id and a one-
    // byte length.  Otherwise, the command is malformed.
    if (extensionFieldSetsLen < extensionFieldSetsIndex + 3) {
      status = SL_ZIGBEE_ZCL_STATUS_MALFORMED_COMMAND;
      goto kickout;
    }

    clusterId = sl_zigbee_af_get_int16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
    extensionFieldSetsIndex += 2;
    length = sl_zigbee_af_get_int8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
    extensionFieldSetsIndex++;

    // If the length is off, the command is also malformed.
    if (length == 0) {
      continue;
    } else if (extensionFieldSetsLen < extensionFieldSetsIndex + length) {
      status = SL_ZIGBEE_ZCL_STATUS_MALFORMED_COMMAND;
      goto kickout;
    }

    switch (clusterId) {
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
      case ZCL_ON_OFF_CLUSTER_ID:
        // We only know of one extension for the On/Off cluster and it is just one
        // byte, which means we can skip some logic for this cluster.  If other
        // extensions are added in this cluster, more logic will be needed here.
        entry.hasOnOffValue = true;
        entry.onOffValue = sl_zigbee_af_get_int8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
        break;
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
      case ZCL_LEVEL_CONTROL_CLUSTER_ID:
        // We only know of one extension for the Level Control cluster and it is
        // just one byte, which means we can skip some logic for this cluster.  If
        // other extensions are added in this cluster, more logic will be needed
        // here.
        entry.hasCurrentLevelValue = true;
        entry.currentLevelValue = sl_zigbee_af_get_int8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
        break;
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
      case ZCL_THERMOSTAT_CLUSTER_ID:
        if (length < 2) {
          break;
        }
        entry.hasOccupiedCoolingSetpointValue = true;
        entry.occupiedCoolingSetpointValue = (int16_t)sl_zigbee_af_get_int16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
        extensionFieldSetsIndex += 2;
        length -= 2;
        if (length < 2) {
          break;
        }
        entry.hasOccupiedHeatingSetpointValue = true;
        entry.occupiedHeatingSetpointValue = (int16_t)sl_zigbee_af_get_int16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
        extensionFieldSetsIndex += 2;
        length -= 2;
        if (length < 1) {
          break;
        }
        entry.hasSystemModeValue = true;
        entry.systemModeValue = sl_zigbee_af_get_int8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
        // If additional Thermostat extensions are added, adjust the index and
        // length variables here.
        break;
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
      case ZCL_COLOR_CONTROL_CLUSTER_ID:
        if (length < 2) {
          break;
        }
        entry.hasCurrentXValue = true;
        entry.currentXValue = sl_zigbee_af_get_int16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
        extensionFieldSetsIndex += 2;
        length -= 2;
        if (length < 2) {
          break;
        }
        entry.hasCurrentYValue = true;
        entry.currentYValue = sl_zigbee_af_get_int16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
        if (enhanced) {
          extensionFieldSetsIndex += 2;
          length -= 2;
          if (length < 2) {
            break;
          }
          entry.hasEnhancedCurrentHueValue = true;
          entry.enhancedCurrentHueValue = sl_zigbee_af_get_int16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
          extensionFieldSetsIndex += 2;
          length -= 2;
          if (length < 1) {
            break;
          }
          entry.hasCurrentSaturationValue = true;
          entry.currentSaturationValue = sl_zigbee_af_get_int8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
          extensionFieldSetsIndex++;
          length--;
          if (length < 1) {
            break;
          }
          entry.hasColorLoopActiveValue = true;
          entry.colorLoopActiveValue = sl_zigbee_af_get_int8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
          extensionFieldSetsIndex++;
          length--;
          if (length < 1) {
            break;
          }
          entry.hasColorLoopDirectionValue = true;
          entry.colorLoopDirectionValue = sl_zigbee_af_get_int8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
          extensionFieldSetsIndex++;
          length--;
          if (length < 2) {
            break;
          }
          entry.hasColorLoopTimeValue = true;
          entry.colorLoopTimeValue = sl_zigbee_af_get_int16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
          extensionFieldSetsIndex += 2;
          length -= 2;
          if (length < 2) {
            break;
          }
          entry.hasColorTemperatureMiredsValue = true;
          entry.colorTemperatureMiredsValue = sl_zigbee_af_get_int16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
        }
        // If additional Color Control extensions are added, adjust the index and
        // length variables here.
        break;
#endif //ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
      case ZCL_DOOR_LOCK_CLUSTER_ID:
        // We only know of one extension for the Door Lock cluster and it is just
        // one byte, which means we can skip some logic for this cluster.  If
        // other extensions are added in this cluster, more logic will be needed
        // here.
        entry.hasLockStateValue = true;
        entry.lockStateValue = sl_zigbee_af_get_int8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
        break;
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
      case ZCL_WINDOW_COVERING_CLUSTER_ID:
        // If we're here, we know we have at least one byte, so we can skip the
        // length check for the first field.
        entry.hasCurrentPositionLiftPercentageValue = true;
        entry.currentPositionLiftPercentageValue = sl_zigbee_af_get_int8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
        extensionFieldSetsIndex++;
        length--;
        if (length < 1) {
          break;
        }
        entry.hasCurrentPositionTiltPercentageValue = true;
        entry.currentPositionTiltPercentageValue = sl_zigbee_af_get_int8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
        // If additional Window Covering extensions are added, adjust the index
        // and length variables here.
        break;
#endif
      default:
        break;
    }

    extensionFieldSetsIndex += length;
  }

  // If we got this far, we either added a new entry or updated an existing one.
  // If we added, store the basic data and increment the scene count.  In either
  // case, save the entry.
  if (i != index) {
    entry.endpoint = endpoint;
    entry.groupId = groupId;
    entry.sceneId = sceneId;
    sl_zigbee_af_scenes_server_incr_num_scene_entries_in_use();
    sl_zigbee_af_scenes_set_scene_count_attribute(endpoint,
                                                  sl_zigbee_af_scenes_server_num_scene_entries_in_use());
  }
  sl_zigbee_af_scenes_server_save_scene_entry(entry, index);
  status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;

  kickout:
  // Add Scene commands are only responded to when they are addressed to a
  // single device.
  if (sl_zigbee_af_current_command()->type != SL_ZIGBEE_INCOMING_UNICAST
      && sl_zigbee_af_current_command()->type != SL_ZIGBEE_INCOMING_UNICAST_REPLY) {
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }
  (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                            | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),
                                           ZCL_SCENES_CLUSTER_ID,
                                           (enhanced
                                            ? ZCL_ENHANCED_ADD_SCENE_RESPONSE_COMMAND_ID
                                            : ZCL_ADD_SCENE_RESPONSE_COMMAND_ID),
                                           "uvu",
                                           status,
                                           groupId,
                                           sceneId);
  sendStatus = sl_zigbee_af_send_response();
  if (SL_STATUS_OK != sendStatus) {
    sl_zigbee_af_scenes_cluster_println("Scenes: failed to send %s response: 0x%02X",
                                        "add_scene",
                                        sendStatus);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_scenes_server_parse_view_scene(const sl_zigbee_af_cluster_command_t *cmd,
                                                                              uint16_t groupId,
                                                                              uint8_t sceneId)
{
  sl_zigbee_af_scene_table_entry_t entry;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  sl_status_t sendStatus;
  bool enhanced = (cmd->commandId == ZCL_ENHANCED_VIEW_SCENE_COMMAND_ID);
  uint8_t endpoint = cmd->apsFrame->destinationEndpoint;

  sl_zigbee_af_scenes_cluster_println("RX: %sViewScene 0x%04X, 0x%02X",
                                      (enhanced ? "Enhanced" : ""),
                                      groupId,
                                      sceneId);

  // View Scene commands can only reference groups which we belong to.
  if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !sl_zigbee_af_groups_cluster_endpoint_in_group_cb(endpoint,
                                                           groupId)) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else {
    uint8_t i;
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE; i++) {
      sl_zigbee_af_scenes_server_retrieve_scene_entry(entry, i);
      if (entry.endpoint == endpoint
          && entry.groupId == groupId
          && entry.sceneId == sceneId) {
        status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
        break;
      }
    }
  }

  // The status, group id, and scene id are always included in the response, but
  // the transition time, name, and extension fields are only included if the
  // scene was found.
  (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                            | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                                            | SL_ZIGBEE_AF_DEFAULT_RESPONSE_POLICY_RESPONSES),
                                           ZCL_SCENES_CLUSTER_ID,
                                           (enhanced
                                            ? ZCL_ENHANCED_VIEW_SCENE_RESPONSE_COMMAND_ID
                                            : ZCL_VIEW_SCENE_RESPONSE_COMMAND_ID),
                                           "uvu",
                                           status,
                                           groupId,
                                           sceneId);
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    // The transition time is returned in seconds in the regular version of the
    // command and tenths of a second in the enhanced version.
    (void) sl_zigbee_af_put_int16u_in_resp(enhanced
                                           ? entry.transitionTime * 10 + entry.transitionTime100ms
                                           : entry.transitionTime);
#ifdef NAME_SUPPORT
    (void) sl_zigbee_af_put_string_in_resp(entry.name);
#else
    (void) sl_zigbee_af_put_int8u_in_resp(0); // name length
#endif
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
    if (entry.hasOnOffValue) {
      (void) sl_zigbee_af_put_int16u_in_resp(ZCL_ON_OFF_CLUSTER_ID);
      (void) sl_zigbee_af_put_int8u_in_resp(1); // length
      (void) sl_zigbee_af_put_int8u_in_resp(entry.onOffValue);
    }
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
    if (entry.hasCurrentLevelValue) {
      (void) sl_zigbee_af_put_int16u_in_resp(ZCL_LEVEL_CONTROL_CLUSTER_ID);
      (void) sl_zigbee_af_put_int8u_in_resp(1); // length
      (void) sl_zigbee_af_put_int8u_in_resp(entry.currentLevelValue);
    }
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
    if (entry.hasOccupiedCoolingSetpointValue) {
      uint8_t *length;
      (void) sl_zigbee_af_put_int16u_in_resp(ZCL_THERMOSTAT_CLUSTER_ID);
      length = &appResponseData[appResponseLength];
      (void) sl_zigbee_af_put_int8u_in_resp(0); // temporary length
      (void) sl_zigbee_af_put_int16u_in_resp(entry.occupiedCoolingSetpointValue);
      *length += 2;
      if (entry.hasOccupiedHeatingSetpointValue) {
        (void) sl_zigbee_af_put_int16u_in_resp(entry.occupiedHeatingSetpointValue);
        *length += 2;
        if (entry.hasSystemModeValue) {
          (void) sl_zigbee_af_put_int8u_in_resp(entry.systemModeValue);
          (*length)++;
        }
      }
    }
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
    {
      if (entry.hasCurrentXValue) {
        uint8_t *length;
        (void) sl_zigbee_af_put_int16u_in_resp(ZCL_COLOR_CONTROL_CLUSTER_ID);
        length = &appResponseData[appResponseLength];
        (void) sl_zigbee_af_put_int8u_in_resp(0); // temporary length
        // Color Control defines three color modes, a device might not support all of them.
        // Populate all attribute fields, substitute placeholder value for those not supported.
        // Attribute IDs are not encoded in the scene extension; identity of attribute values are
        // determined by position in the sequence of concatentated values. Extension fields
        // corresponding to unused attributes (say, Current X and Y) must be populated to allow
        // correct positioning and parsing of subsequent ones (say, colorTemperatureMireds).
        (void) sl_zigbee_af_put_int16u_in_resp(entry.hasCurrentXValue ? entry.currentXValue : 0xFFFF);
        *length += 2;
        if (entry.hasCurrentYValue) {
          (void) sl_zigbee_af_put_int16u_in_resp(entry.hasCurrentYValue ? entry.currentYValue : 0xFFFF);
          *length += 2;
        }
        if (entry.hasEnhancedCurrentHueValue) {
          (void) sl_zigbee_af_put_int16u_in_resp(entry.hasEnhancedCurrentHueValue ? entry.enhancedCurrentHueValue : 0xFFFF);
          *length += 2;
        }
        if (entry.hasCurrentSaturationValue) {
          (void) sl_zigbee_af_put_int8u_in_resp(entry.hasCurrentSaturationValue ? entry.currentSaturationValue : 0xFF);
          (*length)++;
        }
        if (entry.hasColorLoopActiveValue) {
          (void) sl_zigbee_af_put_int8u_in_resp(entry.hasColorLoopActiveValue ? entry.colorLoopActiveValue : 0x00);
          (*length)++;
        }
        if (entry.hasColorLoopDirectionValue) {
          (void) sl_zigbee_af_put_int8u_in_resp(entry.hasColorLoopDirectionValue ? entry.colorLoopDirectionValue : 0x00);
          (*length)++;
        }
        if (entry.colorLoopTimeValue) {
          (void) sl_zigbee_af_put_int16u_in_resp(entry.hasColorLoopTimeValue ? entry.colorLoopTimeValue : 0x0000);
          *length += 2;
        }
        if (entry.hasColorTemperatureMiredsValue) {
          (void) sl_zigbee_af_put_int16u_in_resp(entry.hasColorTemperatureMiredsValue ? entry.colorTemperatureMiredsValue : 0xFFFF);
          *length += 2;
        }
      }
    }
#endif //ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
    if (entry.hasLockStateValue) {
      (void) sl_zigbee_af_put_int16u_in_resp(ZCL_DOOR_LOCK_CLUSTER_ID);
      (void) sl_zigbee_af_put_int8u_in_resp(1); // length
      (void) sl_zigbee_af_put_int8u_in_resp(entry.lockStateValue);
    }
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
    if (entry.hasCurrentPositionLiftPercentageValue) {
      uint8_t *length;
      (void) sl_zigbee_af_put_int16u_in_resp(ZCL_WINDOW_COVERING_CLUSTER_ID);
      length = &appResponseData[appResponseLength];
      (void) sl_zigbee_af_put_int8u_in_resp(0); // temporary length
      (void) sl_zigbee_af_put_int8u_in_resp(entry.currentPositionLiftPercentageValue);
      (*length)++;
      if (entry.hasCurrentPositionTiltPercentageValue) {
        (void) sl_zigbee_af_put_int8u_in_resp(entry.currentPositionTiltPercentageValue);
        (*length)++;
      }
    }
#endif
  }

  // View Scene commands are only responded to when they are addressed to a
  // single device.
  if (sl_zigbee_af_current_command()->type != SL_ZIGBEE_INCOMING_UNICAST
      && sl_zigbee_af_current_command()->type != SL_ZIGBEE_INCOMING_UNICAST_REPLY) {
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }
  sendStatus = sl_zigbee_af_send_response();
  if (SL_STATUS_OK != sendStatus) {
    sl_zigbee_af_scenes_cluster_println("Scenes: failed to send %s response: 0x%02X",
                                        "view_scene",
                                        sendStatus);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

void sl_zigbee_af_scenes_cluster_remove_scenes_in_group_cb(uint8_t endpoint,
                                                           uint16_t groupId)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE; i++) {
    sl_zigbee_af_scene_table_entry_t entry;
    sl_zigbee_af_scenes_server_retrieve_scene_entry(entry, i);
    if (entry.endpoint == endpoint
        && entry.groupId == groupId) {
      entry.groupId = ZCL_SCENES_GLOBAL_SCENE_GROUP_ID;
      entry.endpoint = SL_ZIGBEE_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID;
      sl_zigbee_af_scenes_server_save_scene_entry(entry, i);
      sl_zigbee_af_scenes_server_decr_num_scene_entries_in_use();
      sl_zigbee_af_scenes_set_scene_count_attribute(sl_zigbee_af_current_endpoint(),
                                                    sl_zigbee_af_scenes_server_num_scene_entries_in_use());
    }
  }
}

sl_zigbee_af_status_t sl_zigbee_af_scenes_cluster_store_current_scene_cb(uint8_t endpoint,
                                                                         uint16_t groupId,
                                                                         uint8_t sceneId)
{
  sl_zigbee_af_scene_table_entry_t entry;
  uint8_t i, index = SL_ZIGBEE_AF_SCENE_TABLE_NULL_INDEX;

  // If a group id is specified but this endpoint isn't in it, take no action.
  if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !sl_zigbee_af_groups_cluster_endpoint_in_group_cb(endpoint, groupId)) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE; i++) {
    sl_zigbee_af_scenes_server_retrieve_scene_entry(entry, i);
    if (entry.endpoint == endpoint
        && entry.groupId == groupId
        && entry.sceneId == sceneId) {
      index = i;
      break;
    } else if (index == SL_ZIGBEE_AF_SCENE_TABLE_NULL_INDEX
               && entry.endpoint == SL_ZIGBEE_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID) {
      index = i;
    }
  }

  // If the target index is still zero, the table is full.
  if (index == SL_ZIGBEE_AF_SCENE_TABLE_NULL_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
  }

  sl_zigbee_af_scenes_server_retrieve_scene_entry(entry, index);

  // When creating a new entry or refreshing an existing one, the extension
  // fields are updated with the current state of other clusters on the device.
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
  entry.hasOnOffValue = readServerAttribute(endpoint,
                                            ZCL_ON_OFF_CLUSTER_ID,
                                            ZCL_ON_OFF_ATTRIBUTE_ID,
                                            "on/off",
                                            (uint8_t *)&entry.onOffValue,
                                            sizeof(entry.onOffValue));
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
  entry.hasCurrentLevelValue = readServerAttribute(endpoint,
                                                   ZCL_LEVEL_CONTROL_CLUSTER_ID,
                                                   ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                                                   "current level",
                                                   (uint8_t *)&entry.currentLevelValue,
                                                   sizeof(entry.currentLevelValue));
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
  entry.hasOccupiedCoolingSetpointValue = readServerAttribute(endpoint,
                                                              ZCL_THERMOSTAT_CLUSTER_ID,
                                                              ZCL_OCCUPIED_COOLING_SETPOINT_ATTRIBUTE_ID,
                                                              "occupied cooling setpoint",
                                                              (uint8_t *)&entry.occupiedCoolingSetpointValue,
                                                              sizeof(entry.occupiedCoolingSetpointValue));
  entry.hasOccupiedHeatingSetpointValue = readServerAttribute(endpoint,
                                                              ZCL_THERMOSTAT_CLUSTER_ID,
                                                              ZCL_OCCUPIED_HEATING_SETPOINT_ATTRIBUTE_ID,
                                                              "occupied heating setpoint",
                                                              (uint8_t *)&entry.occupiedHeatingSetpointValue,
                                                              sizeof(entry.occupiedHeatingSetpointValue));
  entry.hasSystemModeValue = readServerAttribute(endpoint,
                                                 ZCL_THERMOSTAT_CLUSTER_ID,
                                                 ZCL_SYSTEM_MODE_ATTRIBUTE_ID,
                                                 "system mode",
                                                 (uint8_t *)&entry.systemModeValue,
                                                 sizeof(entry.systemModeValue));
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
  entry.hasCurrentXValue = readServerAttribute(endpoint,
                                               ZCL_COLOR_CONTROL_CLUSTER_ID,
                                               ZCL_COLOR_CONTROL_CURRENT_X_ATTRIBUTE_ID,
                                               "current x",
                                               (uint8_t *)&entry.currentXValue,
                                               sizeof(entry.currentXValue));
  entry.hasCurrentYValue = readServerAttribute(endpoint,
                                               ZCL_COLOR_CONTROL_CLUSTER_ID,
                                               ZCL_COLOR_CONTROL_CURRENT_Y_ATTRIBUTE_ID,
                                               "current y",
                                               (uint8_t *)&entry.currentYValue,
                                               sizeof(entry.currentYValue));
  entry.hasEnhancedCurrentHueValue = readServerAttribute(endpoint,
                                                         ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                         ZCL_COLOR_CONTROL_ENHANCED_CURRENT_HUE_ATTRIBUTE_ID,
                                                         "enhanced current hue",
                                                         (uint8_t *)&entry.enhancedCurrentHueValue,
                                                         sizeof(entry.enhancedCurrentHueValue));
  entry.hasCurrentSaturationValue = readServerAttribute(endpoint,
                                                        ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                        ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                                                        "current saturation",
                                                        (uint8_t *)&entry.currentSaturationValue,
                                                        sizeof(entry.currentSaturationValue));
  entry.hasColorLoopActiveValue = readServerAttribute(endpoint,
                                                      ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                      ZCL_COLOR_CONTROL_COLOR_LOOP_ACTIVE_ATTRIBUTE_ID,
                                                      "color loop active",
                                                      (uint8_t *)&entry.colorLoopActiveValue,
                                                      sizeof(entry.colorLoopActiveValue));
  entry.hasColorLoopDirectionValue = readServerAttribute(endpoint,
                                                         ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                         ZCL_COLOR_CONTROL_COLOR_LOOP_DIRECTION_ATTRIBUTE_ID,
                                                         "color loop direction",
                                                         (uint8_t *)&entry.colorLoopDirectionValue,
                                                         sizeof(entry.colorLoopDirectionValue));
  entry.hasColorLoopTimeValue = readServerAttribute(endpoint,
                                                    ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                    ZCL_COLOR_CONTROL_COLOR_LOOP_TIME_ATTRIBUTE_ID,
                                                    "color loop time",
                                                    (uint8_t *)&entry.colorLoopTimeValue,
                                                    sizeof(entry.colorLoopTimeValue));
  entry.hasColorTemperatureMiredsValue = readServerAttribute(endpoint,
                                                             ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                             ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID,
                                                             "color temp mireds",
                                                             (uint8_t *)&entry.colorTemperatureMiredsValue,
                                                             sizeof(entry.colorTemperatureMiredsValue));

  // If attributes for more than one color mode are supported, this stored scene entry should
  // should enable only the attributes pertaining to the currently selected color mode.
  // Just need to check one attribute relating to each mode, and any other atrributes for that
  // mode would be supported also.
  uint8_t modeCount = (entry.hasCurrentYValue ? 1 : 0)
                      + (entry.hasCurrentSaturationValue ? 1 : 0)
                      + (entry.hasColorTemperatureMiredsValue ? 1 : 0);
  if (modeCount > 1) {
    uint8_t currentColorMode = 0;
    if (readServerAttribute(endpoint,
                            ZCL_COLOR_CONTROL_CLUSTER_ID,
                            ZCL_COLOR_CONTROL_COLOR_MODE_ATTRIBUTE_ID,
                            "current mode",
                            (uint8_t *)&currentColorMode,
                            sizeof(currentColorMode))) {
      switch (currentColorMode) {
        case SL_ZIGBEE_ZCL_COLOR_MODE_CURRENT_HUE_AND_CURRENT_SATURATION:
          entry.hasCurrentXValue = false;
          entry.hasCurrentYValue = false;
          entry.hasColorTemperatureMiredsValue = false;
          break;

        case SL_ZIGBEE_ZCL_COLOR_MODE_CURRENT_X_AND_CURRENT_Y:
          entry.hasEnhancedCurrentHueValue = false;
          entry.hasCurrentSaturationValue = false;
          entry.hasColorTemperatureMiredsValue = false;
          break;

        case SL_ZIGBEE_ZCL_COLOR_MODE_COLOR_TEMPERATURE:
          entry.hasEnhancedCurrentHueValue = false;
          entry.hasCurrentSaturationValue = false;
          entry.hasCurrentXValue = false;
          entry.hasCurrentYValue = false;
          break;

        default:
          // leave unchanged
          break;
      }
    }
  }
#endif //ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
  entry.hasLockStateValue = readServerAttribute(endpoint,
                                                ZCL_DOOR_LOCK_CLUSTER_ID,
                                                ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                                "lock state",
                                                (uint8_t *)&entry.lockStateValue,
                                                sizeof(entry.lockStateValue));
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
  entry.hasCurrentPositionLiftPercentageValue = readServerAttribute(endpoint,
                                                                    ZCL_WINDOW_COVERING_CLUSTER_ID,
                                                                    ZCL_CURRENT_LIFT_PERCENTAGE_ATTRIBUTE_ID,
                                                                    "current position lift percentage",
                                                                    (uint8_t *)&entry.currentPositionLiftPercentageValue,
                                                                    sizeof(entry.currentPositionLiftPercentageValue));
  entry.hasCurrentPositionTiltPercentageValue = readServerAttribute(endpoint,
                                                                    ZCL_WINDOW_COVERING_CLUSTER_ID,
                                                                    ZCL_CURRENT_TILT_PERCENTAGE_ATTRIBUTE_ID,
                                                                    "current position tilt percentage",
                                                                    (uint8_t *)&entry.currentPositionTiltPercentageValue,
                                                                    sizeof(entry.currentPositionTiltPercentageValue));
#endif

  // When creating a new entry, the name is set to the null string (i.e., the
  // length is set to zero) and the transition time is set to zero.  The scene
  // count must be increased and written to the attribute table when adding a
  // new scene.  Otherwise, these fields and the count are left alone.
  if (i != index) {
    entry.endpoint = endpoint;
    entry.groupId = groupId;
    entry.sceneId = sceneId;
#ifdef NAME_SUPPORT
    entry.name[0] = 0;
#endif
    entry.transitionTime = 0;
    entry.transitionTime100ms = 0;
    sl_zigbee_af_scenes_server_incr_num_scene_entries_in_use();
    sl_zigbee_af_scenes_set_scene_count_attribute(endpoint,
                                                  sl_zigbee_af_scenes_server_num_scene_entries_in_use());
  }

  // Save the scene entry and mark is as valid by storing its scene and group
  // ids in the attribute table and setting valid to true.
  sl_zigbee_af_scenes_server_save_scene_entry(entry, index);
  sl_zigbee_af_scenes_make_valid(endpoint, sceneId, groupId);
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_status_t sl_zigbee_af_scenes_cluster_recall_saved_scene_cb(uint8_t endpoint,
                                                                        uint16_t groupId,
                                                                        uint8_t sceneId)
{
  if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !sl_zigbee_af_groups_cluster_endpoint_in_group_cb(endpoint, groupId)) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else {
    uint8_t i;
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE; i++) {
      sl_zigbee_af_scene_table_entry_t entry;
      sl_zigbee_af_scenes_server_retrieve_scene_entry(entry, i);
      if (entry.endpoint == endpoint
          && entry.groupId == groupId
          && entry.sceneId == sceneId) {
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
        if (entry.hasOnOffValue) {
          writeServerAttribute(endpoint,
                               ZCL_ON_OFF_CLUSTER_ID,
                               ZCL_ON_OFF_ATTRIBUTE_ID,
                               "on/off",
                               (uint8_t *)&entry.onOffValue,
                               ZCL_BOOLEAN_ATTRIBUTE_TYPE);
        }
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
        if (entry.hasCurrentLevelValue) {
          writeServerAttribute(endpoint,
                               ZCL_LEVEL_CONTROL_CLUSTER_ID,
                               ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                               "current level",
                               (uint8_t *)&entry.currentLevelValue,
                               ZCL_INT8U_ATTRIBUTE_TYPE);
        }
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
        if (entry.hasOccupiedCoolingSetpointValue) {
          writeServerAttribute(endpoint,
                               ZCL_THERMOSTAT_CLUSTER_ID,
                               ZCL_OCCUPIED_COOLING_SETPOINT_ATTRIBUTE_ID,
                               "occupied cooling setpoint",
                               (uint8_t *)&entry.occupiedCoolingSetpointValue,
                               ZCL_INT16S_ATTRIBUTE_TYPE);
        }
        if (entry.hasOccupiedHeatingSetpointValue) {
          writeServerAttribute(endpoint,
                               ZCL_THERMOSTAT_CLUSTER_ID,
                               ZCL_OCCUPIED_HEATING_SETPOINT_ATTRIBUTE_ID,
                               "occupied heating setpoint",
                               (uint8_t *)&entry.occupiedHeatingSetpointValue,
                               ZCL_INT16S_ATTRIBUTE_TYPE);
        }
        if (entry.hasSystemModeValue) {
          writeServerAttribute(endpoint,
                               ZCL_THERMOSTAT_CLUSTER_ID,
                               ZCL_SYSTEM_MODE_ATTRIBUTE_ID,
                               "system mode",
                               (uint8_t *)&entry.systemModeValue,
                               ZCL_INT8U_ATTRIBUTE_TYPE);
        }
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
        if (entry.hasCurrentXValue) {
          writeServerAttribute(endpoint,
                               ZCL_COLOR_CONTROL_CLUSTER_ID,
                               ZCL_COLOR_CONTROL_CURRENT_X_ATTRIBUTE_ID,
                               "current x",
                               (uint8_t *)&entry.currentXValue,
                               ZCL_INT16U_ATTRIBUTE_TYPE);
        }
        if (entry.hasCurrentYValue) {
          writeServerAttribute(endpoint,
                               ZCL_COLOR_CONTROL_CLUSTER_ID,
                               ZCL_COLOR_CONTROL_CURRENT_Y_ATTRIBUTE_ID,
                               "current y",
                               (uint8_t *)&entry.currentYValue,
                               ZCL_INT16U_ATTRIBUTE_TYPE);
        }

        if (entry.hasEnhancedCurrentHueValue) {
          writeServerAttribute(endpoint,
                               ZCL_COLOR_CONTROL_CLUSTER_ID,
                               ZCL_COLOR_CONTROL_ENHANCED_CURRENT_HUE_ATTRIBUTE_ID,
                               "enhanced current hue",
                               (uint8_t *)&entry.enhancedCurrentHueValue,
                               ZCL_INT16U_ATTRIBUTE_TYPE);
        }
        if (entry.hasCurrentSaturationValue) {
          writeServerAttribute(endpoint,
                               ZCL_COLOR_CONTROL_CLUSTER_ID,
                               ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                               "current saturation",
                               (uint8_t *)&entry.currentSaturationValue,
                               ZCL_INT8U_ATTRIBUTE_TYPE);
        }
        if (entry.hasColorLoopActiveValue) {
          writeServerAttribute(endpoint,
                               ZCL_COLOR_CONTROL_CLUSTER_ID,
                               ZCL_COLOR_CONTROL_COLOR_LOOP_ACTIVE_ATTRIBUTE_ID,
                               "color loop active",
                               (uint8_t *)&entry.colorLoopActiveValue,
                               ZCL_INT8U_ATTRIBUTE_TYPE);
        }
        if (entry.hasColorLoopDirectionValue) {
          writeServerAttribute(endpoint,
                               ZCL_COLOR_CONTROL_CLUSTER_ID,
                               ZCL_COLOR_CONTROL_COLOR_LOOP_DIRECTION_ATTRIBUTE_ID,
                               "color loop direction",
                               (uint8_t *)&entry.colorLoopDirectionValue,
                               ZCL_INT8U_ATTRIBUTE_TYPE);
        }
        if (entry.hasColorLoopTimeValue) {
          writeServerAttribute(endpoint,
                               ZCL_COLOR_CONTROL_CLUSTER_ID,
                               ZCL_COLOR_CONTROL_COLOR_LOOP_TIME_ATTRIBUTE_ID,
                               "color loop time",
                               (uint8_t *)&entry.colorLoopTimeValue,
                               ZCL_INT16U_ATTRIBUTE_TYPE);
        }
        if (entry.hasColorTemperatureMiredsValue) {
          writeServerAttribute(endpoint,
                               ZCL_COLOR_CONTROL_CLUSTER_ID,
                               ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID,
                               "color temp mireds",
                               (uint8_t *)&entry.colorTemperatureMiredsValue,
                               ZCL_INT16U_ATTRIBUTE_TYPE);
        }
#endif //ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
        if (entry.hasLockStateValue) {
          writeServerAttribute(endpoint,
                               ZCL_DOOR_LOCK_CLUSTER_ID,
                               ZCL_LOCK_STATE_ATTRIBUTE_ID,
                               "lock state",
                               (uint8_t *)&entry.lockStateValue,
                               ZCL_INT8U_ATTRIBUTE_TYPE);
        }
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
        if (entry.hasCurrentPositionLiftPercentageValue) {
          writeServerAttribute(endpoint,
                               ZCL_WINDOW_COVERING_CLUSTER_ID,
                               ZCL_CURRENT_LIFT_PERCENTAGE_ATTRIBUTE_ID,
                               "current position lift percentage",
                               (uint8_t *)&entry.currentPositionLiftPercentageValue,
                               ZCL_INT8U_ATTRIBUTE_TYPE);
        }
        if (entry.hasCurrentPositionTiltPercentageValue) {
          writeServerAttribute(endpoint,
                               ZCL_WINDOW_COVERING_CLUSTER_ID,
                               ZCL_CURRENT_TILT_PERCENTAGE_ATTRIBUTE_ID,
                               "current position tilt percentage",
                               (uint8_t *)&entry.currentPositionTiltPercentageValue,
                               ZCL_INT8U_ATTRIBUTE_TYPE);
        }
#endif
        sl_zigbee_af_scenes_make_valid(endpoint, sceneId, groupId);
        return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
      }
    }
  }

  return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
}

uint32_t sl_zigbee_af_scenes_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                          sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_ADD_SCENE_COMMAND_ID:
      {
        status = sl_zigbee_af_scenes_cluster_add_scene_cb(cmd);
        break;
      }
      case ZCL_GET_SCENE_MEMBERSHIP_COMMAND_ID:
      {
        status = sl_zigbee_af_scenes_cluster_get_scene_membership_cb(cmd);
        break;
      }
      case ZCL_RECALL_SCENE_COMMAND_ID:
      {
        status = sl_zigbee_af_scenes_cluster_recall_scene_cb(cmd);
        break;
      }
      case ZCL_REMOVE_ALL_SCENES_COMMAND_ID:
      {
        status = sl_zigbee_af_scenes_cluster_remove_all_scenes_cb(cmd);
        break;
      }
      case ZCL_REMOVE_SCENE_COMMAND_ID:
      {
        status = sl_zigbee_af_scenes_cluster_remove_scene_cb(cmd);
        break;
      }
      case ZCL_STORE_SCENE_COMMAND_ID:
      {
        status = sl_zigbee_af_scenes_cluster_store_scene_cb(cmd);
        break;
      }
      case ZCL_VIEW_SCENE_COMMAND_ID:
      {
        status = sl_zigbee_af_scenes_cluster_view_scene_cb(cmd);
        break;
      }
    }
  }

  return status;
}

sl_status_t sl_zigbee_af_scenes_token_init(void)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t tok_scenes_num_entries_default = TOKEN_SCENES_NUM_ENTRIES_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_SCENES_NUM_ENTRIES, &tok_scenes_num_entries_default, sizeof(uint8_t));
  assert(status == SL_STATUS_OK);
  sl_zigbee_af_scene_table_entry_t scene_table_entry_default = TOKEN_SCENES_TABLE_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_SCENES_TABLE, &scene_table_entry_default, sizeof(sl_zigbee_af_scene_table_entry_t), SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE);
  assert(status == SL_STATUS_OK);
  return status;
}

void sl_zigbee_af_scenes_init(uint8_t init_level)
{
  (void)init_level;
  assert(SL_STATUS_OK == sl_zigbee_af_scenes_token_init());
}
