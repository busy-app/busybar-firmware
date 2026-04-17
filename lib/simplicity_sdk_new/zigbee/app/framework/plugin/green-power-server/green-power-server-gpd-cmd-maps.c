/***************************************************************************//**
 * @file
 * @brief Routines for the Green Power Server GPD command Cluster and Device Id maps.
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
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "app/framework/include/af.h"
#include "app/framework/util/util.h"
#else // !SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "green-power-adapter.h"
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

#include "green-power-common.h"
#include "green-power-server.h"
#include "green-power-server-config.h"

// Green Power Server configuration for user having the translation table
#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_USER_HAS_DEFAULT_TRANSLATION_TABLE == 1)
#define USER_HAS_DEFAULT_TRANSLATION_TABLE
#endif //SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_USER_HAS_DEFAULT_TRANSLATION_TABLE

// The green power server is capable of a pre-defined default translation and
// forwarding of GPDF commands without the support of the translation table
// component. This header is needed to provide some of the data structure
// definitions when in absence of the translation table component. Hence
// added without any condition.
#include "green-power-translation-table.h"
#ifdef SL_CATALOG_ZIGBEE_GREEN_POWER_TRANSLATION_TABLE_PRESENT
#include "green-power-translation-table-config.h"
// Green Power Translation Table configuration for user having the translation table
#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USER_HAS_DEFAULT_TRANSLATION_TABLE == 1)
#define USER_HAS_DEFAULT_TRANSLATION_TABLE
#endif // SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USER_HAS_DEFAULT_TRANSLATION_TABLE
#define TRANSLATION_TABLE_PRESENT
#endif // SL_CATALOG_ZIGBEE_GREEN_POWER_TRANSLATION_TABLE_PRESENT

// super/protocol/zigbee/app/framework/util/util.h for the sl_zigbee_af_append_to_external_buffer

// This file has three map tables
// 1. Default translation table, maps the gpd command with a zigbee cluster command. This is called as a default or
//    generic translation table. This is used to forward GP operational commands to an application endpoint with or without
//    use of Translation Table plugin.
//    The macro SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_USER_HAS_DEFAULT_TRANSLATION_TABLE allows the user to define this table
//    if needed.
//
// 2. Cluster mapping from Device Id. In GP spec, the device Id specefies certain clusters to be included mandatorily.
//    During GPD commissioning (with only device Id is present and no other information included in commissioning frame)
//    the functionality matching is done by looking up the device Id, finding the cluster and then matching that with
//    Sink application end point that is in commissioning.
//
// 3. Command mapping from Device Id. In GP spec, the device Id also means certain commands to be included by default.
//    This table is used to look up the commands list from device Id while commissioning.
//
// Example of the sink functionality matching is based on the following flow when just device Id and no other infomation is
// present in the commissioning.
// Device Id -> command Id list -> Cluster list
// Device Id --------------------> Cluster List
//                                             |--> Cluster list matching with Sink application endpoint that
//                                                  is currently commissioning.

#ifndef USER_HAS_DEFAULT_TRANSLATION_TABLE
const sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t sl_zigbee_gp_default_translation_table[] =
{
  { true, SL_ZIGBEE_ZCL_GP_GPDF_IDENTIFY, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_IDENTIFY_CLUSTER_ID, 1, ZCL_IDENTIFY_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x02, 0x00, 0x3C } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_RECALL_SCENE0, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_RECALL_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x00 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_RECALL_SCENE1, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_RECALL_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x01 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_RECALL_SCENE2, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_RECALL_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x02 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_RECALL_SCENE3, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_RECALL_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x03 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_RECALL_SCENE4, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_RECALL_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x04 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_RECALL_SCENE5, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_RECALL_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x05 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_RECALL_SCENE6, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_RECALL_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x06 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_RECALL_SCENE7, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_RECALL_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x07 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STORE_SCENE0, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_STORE_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x00 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STORE_SCENE1, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_STORE_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x01 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STORE_SCENE2, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_STORE_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x02 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STORE_SCENE3, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_STORE_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x03 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STORE_SCENE4, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_STORE_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x04 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STORE_SCENE5, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_STORE_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x05 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STORE_SCENE6, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_STORE_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x06 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STORE_SCENE7, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_SCENES_CLUSTER_ID, 1, ZCL_STORE_SCENE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x03, 0xFF, 0xFF, 0x07 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_OFF, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_OFF_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_ON, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_TOGGLE, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_TOGGLE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_RELEASE, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_MOVE_UP, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_MOVE_MODE_UP } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_MOVE_DOWN, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_MOVE_MODE_DOWN } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STEP_UP, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STEP_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_STEP_MODE_UP } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STEP_DOWN, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STEP_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_STEP_MODE_DOWN } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_LEVEL_CONTROL_STOP, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_MOVE_UP_WITH_ON_OFF, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_WITH_ON_OFF_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_MOVE_MODE_UP } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_MOVE_DOWN_WITH_ON_OFF, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_WITH_ON_OFF_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_MOVE_MODE_DOWN } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STEP_UP_WITH_ON_OFF, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STEP_WITH_ON_OFF_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_STEP_MODE_UP } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STEP_DOWN_WITH_ON_OFF, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STEP_WITH_ON_OFF_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_STEP_MODE_DOWN } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_MOVE_HUE_STOP, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_COLOR_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_HUE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x02, SL_ZIGBEE_ZCL_HUE_MOVE_MODE_STOP, 0xFF } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_MOVE_HUE_UP, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_COLOR_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_HUE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_HUE_MOVE_MODE_UP } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_MOVE_HUE_DOWN, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_COLOR_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_HUE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_HUE_MOVE_MODE_DOWN } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STEP_HUE_UP, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_COLOR_CONTROL_CLUSTER_ID, 1, ZCL_STEP_HUE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_HUE_STEP_MODE_UP } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STEP_HUE_DOWN, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_COLOR_CONTROL_CLUSTER_ID, 1, ZCL_STEP_HUE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_HUE_STEP_MODE_DOWN } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_MOVE_SATURATION_STOP, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_COLOR_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_SATURATION_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED, { 0x01, SL_ZIGBEE_ZCL_SATURATION_MOVE_MODE_STOP } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_MOVE_SATURATION_UP, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_COLOR_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_SATURATION_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_SATURATION_MOVE_MODE_UP } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_MOVE_SATURATION_DOWN, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_COLOR_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_SATURATION_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_SATURATION_MOVE_MODE_DOWN } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STEP_SATURATION_UP, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_COLOR_CONTROL_CLUSTER_ID, 1, ZCL_STEP_SATURATION_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_SATURATION_STEP_MODE_UP } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STEP_SATURATION_DOWN, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_COLOR_CONTROL_CLUSTER_ID, 1, ZCL_STEP_SATURATION_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_SATURATION_STEP_MODE_DOWN } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_MOVE_COLOR, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_COLOR_CONTROL_CLUSTER_ID, 1, ZCL_STEP_SATURATION_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_STEP_COLOR, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_COLOR_CONTROL_CLUSTER_ID, 1, ZCL_STEP_COLOR_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_LOCK_DOOR, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_DOOR_LOCK_CLUSTER_ID, 1, ZCL_LOCK_DOOR_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_UNLOCK_DOOR, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_DOOR_LOCK_CLUSTER_ID, 1, ZCL_UNLOCK_DOOR_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },

  // User configurable as the command does not have a ZCL mapping
  { true, SL_ZIGBEE_ZCL_GP_GPDF_PRESS1_OF1, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_RELEASE1_OF1, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_OFF_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_SHORT_PRESS1_OF1, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_TOGGLE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_PRESS1_OF2, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_RELEASE1_OF2, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_OFF_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_PRESS2_OF2, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_RELEASE2_OF2, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_OFF_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_SHORT_PRESS1_OF2, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_TOGGLE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_SHORT_PRESS2_OF2, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_TOGGLE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } },

  // Gereral ZCL command mapping - for which the cluster should be present in
  // gpd commissioning session to validate if it can be supported by sink
  { true, SL_ZIGBEE_ZCL_GP_GPDF_REQUEST_ATTRIBUTE, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, 0xFFFF, 1, ZCL_READ_ATTRIBUTES_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0xFF } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_READ_ATTR_RESPONSE, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, 0xFFFF, 0, ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0xFF } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_ZCL_TUNNELING_WITH_PAYLOAD, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, 0xFFFF, 0, 0xFF, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0xFF } },
  { true, SL_ZIGBEE_ZCL_GP_GPDF_ANY_GPD_SENSOR_CMD, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, 0xFFFF, 0, ZCL_REPORT_ATTRIBUTES_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0xFF } },

// Following command is only available if CAR/MS is implemenetd, which requires translation table support.
#ifdef TRANSLATION_TABLE_PRESENT
  // for compact attribute report, if ZbEndpoint=0xFC then ZbClu + ZbCmd + ZbPayload have No Meaning
  { true, SL_ZIGBEE_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING, SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT, HA_PROFILE_ID, 0xFFFF, 0, ZCL_REPORT_ATTRIBUTES_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0xFE } },
#endif
};

uint16_t sli_zigbee_af_gp_default_table_size = (sizeof(sl_zigbee_gp_default_translation_table) / sizeof(sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t));

static uint16_t defaultTableGpdCmdIndex(uint16_t startIndex, uint8_t gpdCommandId)
{
  for (int i = startIndex; i < sli_zigbee_af_gp_default_table_size; i++) {
    if (gpdCommandId == sl_zigbee_gp_default_translation_table[i].gpdCommand) {
      return i;
    }
  }
  return 0xFFFF;
}
// Simply builds a list of all the gpd cluster supported by the standard deviceId
// consumed by formGpdClusterListFromIncommingCommReq
bool sli_zigbee_af_get_cluster_list_from_cmd_id_lookup(uint8_t gpdCommandId,
                                                       sli_zigbee_zigbee_cluster_t *gpdCluster)
{
  if (gpdCluster != NULL) {
    uint16_t index = defaultTableGpdCmdIndex(0, gpdCommandId);
    if (0xFFFF != index) {
      gpdCluster->clusterId = sl_zigbee_gp_default_translation_table[index].zigbeeCluster;
      gpdCluster->serverClient = sl_zigbee_gp_default_translation_table[index].serverClient;
      return true;
    }
  }
  return false;
}
#endif

static void fillExternalBufferGpdCommandA6Payload(uint8_t *gpdCommandPayload)
{
  // A6 - GPD ZCL Tunneling gpdCommandPayload format :
  //  -------------------------------------------------------------------------------------------------------------
  // | Total Length(1) | Option(1) | Manf Id (0/2) | ClusterId(2) | ZCommand(1) | Length(1) | Payload(0/Variable) |
  //  -------------------------------------------------------------------------------------------------------------
  // total payload length is given in gpdCommandPayload[0]
  uint8_t frameControl = gpdCommandPayload[1];
  uint8_t *payload = &gpdCommandPayload[2];

  uint16_t manufacturerCode = SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE;
  if (frameControl & ZCL_MANUFACTURER_SPECIFIC_MASK) {
    manufacturerCode = ((*(payload + 1)) << 8) + (*payload);
    payload += 2;
  }
  sl_zigbee_af_cluster_id_t clusterId = ((*(payload + 1)) << 8) + (*payload);
  payload += 2;
  uint8_t zigbeeCommand = *payload;
  payload += 1;
  sl_zigbee_af_fill_external_manufacturer_specific_buffer(frameControl,
                                                          clusterId,
                                                          manufacturerCode,
                                                          zigbeeCommand,
                                                          "");
  uint8_t length = *payload;
  payload += 1;
  sl_zigbee_af_append_to_external_buffer(payload, length);
}

static void fillExternalBufferGpdCommandA0A1Payload(uint8_t gpdCommandId,
                                                    uint8_t *gpdCommandPayload,
                                                    uint8_t zigbeeCommandId,
                                                    bool direction)
{
  // total payload length is given in gpdCommandPayload[0]
  uint8_t *payload = &gpdCommandPayload[1];
  uint8_t frameControl = (ZCL_GLOBAL_COMMAND                                          \
                          | (direction ? ZCL_FRAME_CONTROL_CLIENT_TO_SERVER           \
                             : ZCL_FRAME_CONTROL_SERVER_TO_CLIENT)                    \
                          | ((gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_MFR_SP_ATTR_RPTG) \
                             ? ZCL_MANUFACTURER_SPECIFIC_MASK : 0));

  uint16_t manufacturerCode = SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE;
  if (frameControl & ZCL_MANUFACTURER_SPECIFIC_MASK) {
    manufacturerCode = ((*(payload + 1)) << 8) + (*payload);
    payload += 2;
  }
  sl_zigbee_af_cluster_id_t clusterId = ((*(payload + 1)) << 8) + (*payload);
  payload += 2;
  uint8_t length = (payload - (&(gpdCommandPayload[1])));
  sl_zigbee_af_fill_external_manufacturer_specific_buffer(frameControl,
                                                          clusterId,
                                                          manufacturerCode,
                                                          zigbeeCommandId,
                                                          "");
  // Remaining payload to copy = gpdCommandPayload[0] - what ever populated
  sl_zigbee_af_append_to_external_buffer(payload, (gpdCommandPayload[0] - length));
}

static sl_status_t handleA2A3MultiClusterReportForwarding(uint8_t gpdCommandId,
                                                          uint8_t *gpdCommandPayload,
                                                          uint8_t zigbeeCommandId,
                                                          bool direction,
                                                          uint8_t zbEndpoint)
{
  // To handle the Multi Cluster Attribute reporting, as there is no equivalent ZCL command
  // it needed to be broken down to individual packets as A0/A1 and sent.
  // There are two approaches, break down and send in a loop here OR store it and
  // send it with help of a timer.
  uint8_t *payloadFinger = &gpdCommandPayload[1];
  uint8_t clusterReport[30];
  uint8_t startIndex = 1;
  sl_status_t status;

  if (gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_MFR_SP_MULTI_CLUSTER_RPTG) {
    memcpy(&clusterReport[startIndex], &gpdCommandPayload[1], 2); // ManufactureId copied
    payloadFinger += 2; // source pointer incremented
    startIndex += 2;     // Destination pointer incremented
  }
  // Copy cluster Report fields for each cluster in incoming payload
  do {
    uint8_t destIndex = startIndex;
    memcpy(&clusterReport[destIndex], payloadFinger, 4); // ClusterId + Attribute Id copied
    payloadFinger += 4; // source pointer incremented for copied cluster and attribute Id
    destIndex += 4;     // Dest pointer Incremented for copied cluster and attribute Id
    clusterReport[destIndex] = *payloadFinger; //AttributeDataType copied
    uint8_t dataSize = sl_zigbee_af_get_data_size(*payloadFinger); // get the dataSize with the dataType
    payloadFinger += 1; // source pointer incremented for dataType
    destIndex += 1;     // Dest pointer Incremented for dataType
    memcpy(&clusterReport[destIndex], payloadFinger, dataSize); // Attribuetdata copied
    payloadFinger += dataSize; // source pointer incremented for copied data
    destIndex += dataSize;     // Dest pointer incremented for copied data
    clusterReport[0] = (destIndex - 1);  // finally copy the length in index 0;

    fillExternalBufferGpdCommandA0A1Payload((gpdCommandId - 2), // A0/A1 is exact replica of A2/A3 respectively
                                            clusterReport,
                                            zigbeeCommandId,
                                            direction);
    sl_zigbee_af_set_command_endpoints(zbEndpoint,
                                       zbEndpoint);
    status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, sl_zigbee_af_get_node_id());
    if (status != SL_STATUS_OK) {
      return status;
    }
  } while ((payloadFinger - (&gpdCommandPayload[1])) < gpdCommandPayload[0]);
  return status;
}

static void fillExternalBufferGpdCommandScenesPayload(sl_zigbee_gp_address_t *gpdAddr,
                                                      uint8_t gpdCommandId,
                                                      sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t const *genericTranslationTable,
                                                      uint8_t endpoint)
{
  //If the sink implements the Translation Table, it derives the groupId as alias
  uint16_t groupId = ((uint16_t)genericTranslationTable->zclPayloadDefault[2] << 8) + genericTranslationTable->zclPayloadDefault[1];
  if (groupId == 0xFFFF) {
    groupId = sli_zigbee_gpd_alias(gpdAddr);
  }
  // General command filling.
  (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND             \
                                            | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),   \
                                           genericTranslationTable->zigbeeCluster,   \
                                           genericTranslationTable->zigbeeCommandId, \
                                           "");
  sl_zigbee_af_append_to_external_buffer((uint8_t *)(&groupId), sizeof(groupId));
  sl_zigbee_af_append_to_external_buffer((uint8_t *)(&genericTranslationTable->zclPayloadDefault[3]), sizeof(uint8_t));

  if (gpdCommandId & SL_ZIGBEE_ZCL_GP_GPDF_STORE_SCENE0) {
    // Store Scenes commands from 0x18 to 0x1F
    // Add the endpoint to the group in the store
    sl_zigbee_af_status_t status = sli_zigbee_af_gp_add_to_aps_group(endpoint, groupId);
    sl_zigbee_af_core_println("Store Scene : Added endpoint %d to Group %d Status = %d", endpoint, groupId, status);
  } else {
    // Recall Scenes commands from 0x10 to 0x17 - the payload is already ready
  }
}

sl_status_t sli_zigbee_af_gp_forward_gpd_to_mapped_endpoint(sl_zigbee_gp_address_t *addr,
                                                            uint8_t gpdCommandId,
                                                            uint8_t *gpdCommandPayload,
                                                            const sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *genericTranslationTable,
                                                            uint8_t endpoint)
{
  // Start to fill and send the commands
  if (gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_MULTI_CLUSTER_RPTG
      || gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_MFR_SP_MULTI_CLUSTER_RPTG) {
    // This will fill up and send out multiple UCAST to the dest app endpoint.
    return handleA2A3MultiClusterReportForwarding(gpdCommandId,
                                                  gpdCommandPayload,
                                                  genericTranslationTable->zigbeeCommandId,
                                                  (bool)genericTranslationTable->serverClient,
                                                  endpoint);
  } else if (gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_ZCL_TUNNELING_WITH_PAYLOAD) {
    fillExternalBufferGpdCommandA6Payload(gpdCommandPayload);
  } else if (gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_ATTRIBUTE_REPORTING
             || gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_MFR_SP_ATTR_RPTG) {
    fillExternalBufferGpdCommandA0A1Payload(gpdCommandId,
                                            gpdCommandPayload,
                                            genericTranslationTable->zigbeeCommandId,
                                            (bool)genericTranslationTable->serverClient);
  } else if (gpdCommandId >= SL_ZIGBEE_ZCL_GP_GPDF_RECALL_SCENE0
             && gpdCommandId <= SL_ZIGBEE_ZCL_GP_GPDF_STORE_SCENE7) {
    // All scenes commands 0x10 to 0x1F is prepared and handled here
    fillExternalBufferGpdCommandScenesPayload(addr,
                                              gpdCommandId,
                                              genericTranslationTable,
                                              endpoint);
  } else {
    // General command filling.
    (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND             \
                                              | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),   \
                                             genericTranslationTable->zigbeeCluster,   \
                                             genericTranslationTable->zigbeeCommandId, \
                                             "");
    // First copy the pre-configured source
    if (genericTranslationTable->payloadSrc & SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED) {
      sl_zigbee_af_append_to_external_buffer(&genericTranslationTable->zclPayloadDefault[1],
                                             genericTranslationTable->zclPayloadDefault[0]);
    }
    if (gpdCommandId >= SL_ZIGBEE_ZCL_GP_GPDF_MOVE_UP
        && gpdCommandId <= SL_ZIGBEE_ZCL_GP_GPDF_MOVE_DOWN
        && gpdCommandPayload != NULL) {
      if (gpdCommandPayload[0] == 0) {
        // If the rate field is not present
        // we set it to 0xff so the device shall move at the default rate
        gpdCommandPayload[0] = 1;
        gpdCommandPayload[1] = 0xff;
      }
    }
    // Overwrite the payload from gpd command source - as this is the priority
    if ((genericTranslationTable->payloadSrc & SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD)
        && gpdCommandPayload != NULL) {
      sl_zigbee_af_append_to_external_buffer(&gpdCommandPayload[1],
                                             gpdCommandPayload[0]);
    }
  }
#if defined(SL_ZIGBEE_AF_NCP) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT)
  uint8_t appEp = sl_zigbee_get_endpoint(0);
#else
  uint8_t appEp = sli_zigbee_af_endpoints[0].endpoint;
#endif
  sl_zigbee_af_set_command_endpoints(appEp, endpoint);
  return sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, sl_zigbee_af_get_node_id());
}

void sli_zigbee_af_gp_forward_gpd_command_default(sl_zigbee_gp_address_t *addr,
                                                  uint8_t gpdCommandId,
                                                  uint8_t *gpdCommandPayload)
{
  uint16_t tableIndex = defaultTableGpdCmdIndex(0, gpdCommandId);
  if (0xFFFF == tableIndex) {
    return; // Can not be forwarded without preconfigured map.
  }
  const sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *genericTranslationTable = &sl_zigbee_gp_default_translation_table[tableIndex];
  sli_zigbee_af_gp_forward_gpd_to_mapped_endpoint(addr,
                                                  gpdCommandId,
                                                  gpdCommandPayload,
                                                  genericTranslationTable,
                                                  SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ZCL_MESSAGE_DST_ENDPOINT); // Application Endpoint
}

const uint16_t deviceIdCluster_SL_ZIGBEE_GP_DEVICE_ID_GPD_SIMPLE_SENSOR_SWITCH[]          = { 0x000F };
const uint16_t deviceIdCluster_SL_ZIGBEE_GP_DEVICE_ID_GPD_LIGHT_SENSOR_SWITCH[]           = { 0x0400 };
const uint16_t deviceIdCluster_SL_ZIGBEE_GP_DEVICE_ID_GPD_OCCUPANCY_SENSOR_SWITCH[]       = { 0x0406 };
const uint16_t deviceIdCluster_SL_ZIGBEE_GP_DEVICE_ID_GPD_TEMPERATURE_SENSOR_SWITCH[]     = { 0x0402 };
const uint16_t deviceIdCluster_SL_ZIGBEE_GP_DEVICE_ID_GPD_PRESSURE_SENSOR_SWITCH[]        = { 0x0403 };
const uint16_t deviceIdCluster_SL_ZIGBEE_GP_DEVICE_ID_GPD_FLOW_SENSOR_SWITCH[]            = { 0x0404 };
const uint16_t deviceIdCluster_SL_ZIGBEE_GP_DEVICE_ID_GPD_INDOOR_ENVIRONMENT_SENSOR[]     = { 0x0402, 0x0405, 0x0400, 0x040D };

#define DEVICEID_CLUSTER_LOOKUP(num)  { num, (sizeof(deviceIdCluster_##num) / sizeof(uint16_t)), deviceIdCluster_##num }
#define DEVICE_ID_MAP_CLUSTER_TABLE_SIZE (sizeof(gpdDeviceClusterMap) / sizeof(sli_zigbee_gp_device_id_and_cluster_map_t))
const sli_zigbee_gp_device_id_and_cluster_map_t gpdDeviceClusterMap[] = {
  DEVICEID_CLUSTER_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_SIMPLE_SENSOR_SWITCH),             //4
  DEVICEID_CLUSTER_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_LIGHT_SENSOR_SWITCH),              //11
  DEVICEID_CLUSTER_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_OCCUPANCY_SENSOR_SWITCH),          //12
  DEVICEID_CLUSTER_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_TEMPERATURE_SENSOR_SWITCH),        //30
  DEVICEID_CLUSTER_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_PRESSURE_SENSOR_SWITCH),           //31
  DEVICEID_CLUSTER_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_FLOW_SENSOR_SWITCH),               //32
  DEVICEID_CLUSTER_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_INDOOR_ENVIRONMENT_SENSOR),        //33
};

// Simply builds a list of all the gpd cluster supported by the standard deviceId
// consumed by formGpdClusterListFromIncommingCommReq
uint8_t sli_zigbee_af_get_cluster_list_from_device_id_lookup(uint8_t gpdDeviceId,
                                                             sli_zigbee_zigbee_cluster_t *gpdClusterList)
{
  for (uint8_t mapTableIndex = 0; mapTableIndex < DEVICE_ID_MAP_CLUSTER_TABLE_SIZE; mapTableIndex++) {
    if (gpdDeviceClusterMap[mapTableIndex].deviceId == gpdDeviceId) {
      for (uint8_t clusterIndex = 0; clusterIndex < gpdDeviceClusterMap[mapTableIndex].numberOfClusters; clusterIndex++) {
        // cluster supported
        gpdClusterList[clusterIndex].clusterId = gpdDeviceClusterMap[mapTableIndex].cluster[clusterIndex];
        // GPD announce this cluster as server,
        // this cluster is recorded as client side to evaluate it presence on the sink
        gpdClusterList[clusterIndex].serverClient = 0; // reverse for the match
      }
      return gpdDeviceClusterMap[mapTableIndex].numberOfClusters;
    }
  }
  return 0;
}

// all define to map CmdID with DeviceID
const uint8_t deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_SIMPLE_GENERIC_ONE_STATE_SWITCH[]   = { 2, 0x60, 0x61 };                                                                         //0
const uint8_t deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_SIMPLE_GENERIC_TWO_STATE_SWITCH[]   = { 4, 0x62, 0x63, 0x64, 0x65 };                                                             //1
#ifdef SL_ZIGBEE_TEST
const uint8_t deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_ON_OFF_SWITCH[]                     = { 8, 0x00, 0x20, 0x21, 0x22, 0x13, 0x1B, 0x40, 0x30 };                                     //2
#else
const uint8_t deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_ON_OFF_SWITCH[]                     = { 3, 0x20, 0x21, 0x22 };                                                                   //2
#endif
const uint8_t deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_LEVEL_CONTROL_SWITCH[]              = { 9, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38 };                               //3
const uint8_t deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_ADVANCED_GENERIC_ONE_STATE_SWITCH[] = { 3, 0x60, 0x61, 0x66 };                                                                   //5
const uint8_t deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_ADVANCED_GENERIC_TWO_STATE_SWITCH[] = { 7, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68 };                                           //6
const uint8_t deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_GENERIC_SWITCH[]                    = { 2, 0x69, 0x6A };                                                                         //7
const uint8_t deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_COLOR_DIMMER_SWITCH[]               = { 12, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B };            //10
const uint8_t deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_LIGHT_SENSOR_SWITCH[]               = { 2, 0xAF, 0xA6 }; // 0xAF for any command from 0xA0 to 0xA3 to compact TT                 //11,12,30,31,32,33
const uint8_t deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_DOOR_LOCK_CONTROLLER_SWITCH[]       = { 2, 0x50, 0x51 };                                                                         //20
const uint8_t deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_SCENCES[]                           = { 16, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F }; //unspecified

#define deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_TEMPERATURE_SENSOR_SWITCH       deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_LIGHT_SENSOR_SWITCH
#define deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_PRESSURE_SENSOR_SWITCH          deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_LIGHT_SENSOR_SWITCH
#define deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_FLOW_SENSOR_SWITCH              deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_LIGHT_SENSOR_SWITCH
#define deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_INDOOR_ENVIRONMENT_SENSOR       deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_LIGHT_SENSOR_SWITCH
#define deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_OCCUPANCY_SENSOR_SWITCH         deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_LIGHT_SENSOR_SWITCH
#define deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_SIMPLE_SENSOR_SWITCH            deviceIdCmds_SL_ZIGBEE_GP_DEVICE_ID_GPD_LIGHT_SENSOR_SWITCH

#define DEVICEID_CMDS_LOOKUP(num)  { num, deviceIdCmds_##num }
#define DEVICE_ID_MAP_TABLE_SIZE (sizeof(gpdDeviceCmdMap) / sizeof(sli_zigbee_gp_device_id_and_command_map_t))
const sli_zigbee_gp_device_id_and_command_map_t gpdDeviceCmdMap[] = {
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_SIMPLE_GENERIC_ONE_STATE_SWITCH),  //0
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_SIMPLE_GENERIC_TWO_STATE_SWITCH),  //1
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_ON_OFF_SWITCH),                    //2
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_LEVEL_CONTROL_SWITCH),             //3
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_SIMPLE_SENSOR_SWITCH),             //4
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_ADVANCED_GENERIC_ONE_STATE_SWITCH),//5
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_ADVANCED_GENERIC_TWO_STATE_SWITCH),//6
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_GENERIC_SWITCH),                   //7
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_COLOR_DIMMER_SWITCH),              //10
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_LIGHT_SENSOR_SWITCH),              //11
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_OCCUPANCY_SENSOR_SWITCH),          //12
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_DOOR_LOCK_CONTROLLER_SWITCH),      //20
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_TEMPERATURE_SENSOR_SWITCH),        //30
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_PRESSURE_SENSOR_SWITCH),           //31
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_FLOW_SENSOR_SWITCH),               //32
  DEVICEID_CMDS_LOOKUP(SL_ZIGBEE_GP_DEVICE_ID_GPD_INDOOR_ENVIRONMENT_SENSOR),        //33
};

// Simply builds a list of all the gpd commands supported by the standard deviceId
// consumed by formGpdCommandListFromIncommingCommReq
uint8_t sli_zigbee_af_get_command_list_from_device_id_lookup(uint8_t gpdDeviceId,
                                                             uint8_t *gpdCommandList)
{
  for (uint8_t index = 0; index < DEVICE_ID_MAP_TABLE_SIZE; index++) {
    if (gpdDeviceCmdMap[index].deviceId == gpdDeviceId) {
      memcpy(gpdCommandList,
             &gpdDeviceCmdMap[index].cmd[1],  // commands supported
             gpdDeviceCmdMap[index].cmd[0]);  // length
      return gpdDeviceCmdMap[index].cmd[0];
    }
  }
  return 0;
}
