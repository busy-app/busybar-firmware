/***************************************************************************//**
 * @file
 * @brief CLI for the Scenes plugin.
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
#include "scenes.h"

void sli_plugin_scenes_server_clear(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_af_core_println("Clearing all scenes.");
  sl_zigbee_af_scenes_cluster_clear_scene_table_cb(SL_ZIGBEE_BROADCAST_ENDPOINT);
}
