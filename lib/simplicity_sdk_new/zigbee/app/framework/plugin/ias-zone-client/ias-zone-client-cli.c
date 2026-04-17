/***************************************************************************//**
 * @file
 * @brief CLI for the IAS Zone Client plugin.
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

#include "app/util/serial/sl_zigbee_command_interpreter.h"

#include "ias-zone-client.h"

#include "ias-zone-client-config.h"

void sli_zigbee_af_ias_zone_client_print_servers_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  uint8_t i;
  sl_zigbee_af_ias_zone_cluster_println("Index IEEE                 EP   Type   Status State ID");
  sl_zigbee_af_ias_zone_cluster_println("---------------------------------------------------");
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES; i++) {
    if (i < 10) {
      sl_zigbee_af_ias_zone_cluster_print(" ");
    }
    sl_zigbee_af_ias_zone_cluster_print("%d    (>)%02X%02X%02X%02X%02X%02X%02X%02X  ",
                                        i,
                                        sl_zigbee_af_ias_zone_client_known_servers[i].ieeeAddress[7],
                                        sl_zigbee_af_ias_zone_client_known_servers[i].ieeeAddress[6],
                                        sl_zigbee_af_ias_zone_client_known_servers[i].ieeeAddress[5],
                                        sl_zigbee_af_ias_zone_client_known_servers[i].ieeeAddress[4],
                                        sl_zigbee_af_ias_zone_client_known_servers[i].ieeeAddress[3],
                                        sl_zigbee_af_ias_zone_client_known_servers[i].ieeeAddress[2],
                                        sl_zigbee_af_ias_zone_client_known_servers[i].ieeeAddress[1],
                                        sl_zigbee_af_ias_zone_client_known_servers[i].ieeeAddress[0]);
    if (sl_zigbee_af_ias_zone_client_known_servers[i].endpoint < 10) {
      sl_zigbee_af_ias_zone_cluster_print(" ");
    }
    if (sl_zigbee_af_ias_zone_client_known_servers[i].endpoint < 100) {
      sl_zigbee_af_ias_zone_cluster_print(" ");
    }
    sl_zigbee_af_ias_zone_cluster_print("%d  ", sl_zigbee_af_ias_zone_client_known_servers[i].endpoint);
    sl_zigbee_af_ias_zone_cluster_println("0x%04X 0x%04X 0x%02X 0x%02X",
                                          sl_zigbee_af_ias_zone_client_known_servers[i].zoneType,
                                          sl_zigbee_af_ias_zone_client_known_servers[i].zoneStatus,
                                          sl_zigbee_af_ias_zone_client_known_servers[i].zoneState,
                                          sl_zigbee_af_ias_zone_client_known_servers[i].zoneId);
  }
}

void sli_zigbee_af_ias_zone_client_clear_all_servers_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sli_zigbee_af_clear_servers();
}
