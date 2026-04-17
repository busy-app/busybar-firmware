/***************************************************************************//**
 * @file
 * @brief CLI for the EZ-Mode plugin.
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
#include "ez-mode.h"

#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "ezmode-commissioning-config.h"
#define MAX_CLUSTER_IDS 3
static sl_zigbee_af_cluster_id_t clusterIds[MAX_CLUSTER_IDS];

// plugin ezmode-commissioning client <endpoint:1> <direction:1> <cluster ids:n>
void sl_zigbee_af_ez_mode_commissioning_client_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_ez_mode_commissioning_direction_t direction = (sl_zigbee_af_ez_mode_commissioning_direction_t)sl_cli_get_argument_uint8(arguments, 1);
  uint8_t clusterIdLength;
  uint8_t count = sl_cli_get_argument_count(arguments) - 2;
  if (MAX_CLUSTER_IDS < count) {
    count = MAX_CLUSTER_IDS;
    sl_zigbee_af_app_println("Cluster ids truncated to %d", count);
  }
  for (clusterIdLength = 0; clusterIdLength < count; clusterIdLength++) {
    clusterIds[clusterIdLength]
      = (sl_zigbee_af_cluster_id_t)sl_cli_get_argument_uint16(arguments, clusterIdLength + 2);
  }
  status = sl_zigbee_af_ezmode_client_commission(endpoint,
                                                 direction,
                                                 clusterIds,
                                                 clusterIdLength);
  UNUSED_VAR(status);
  sl_zigbee_af_app_println("%s 0x%02X", "client", status);
}

// plugin ezmode-commissioning server <endpoint:1> <identify time:1>
void sl_zigbee_af_ez_mode_commissioning_server_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  uint16_t identifyTimeS = sl_cli_get_argument_uint16(arguments, 1);
  if (identifyTimeS == 0) {
    identifyTimeS = SL_ZIGBEE_AF_PLUGIN_EZMODE_COMMISSIONING_IDENTIFY_TIMEOUT;
  }
  status = sl_zigbee_af_ezmode_server_commission_with_timeout(endpoint, identifyTimeS);
  UNUSED_VAR(status);
  sl_zigbee_af_app_println("%s 0x%02X", "server", status);
}
