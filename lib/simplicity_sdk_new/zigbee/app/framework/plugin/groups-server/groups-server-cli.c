/***************************************************************************//**
 * @file
 * @brief CLI for the Groups Server plugin.
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

#include "app/framework/plugin/groups-server/groups-server.h"

// plugin groups-server print
void sli_zigbee_af_groups_server_cli_print(sl_cli_command_arg_t *args)
{
  (void)args;
  sl_status_t status;
  uint8_t i;

  for (i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
    sl_zigbee_binding_table_entry_t entry;
    status = sl_zigbee_get_binding(i, &entry);
    if ((status == SL_STATUS_OK) && (entry.type == SL_ZIGBEE_MULTICAST_BINDING)) {
      sl_zigbee_af_core_println("ep[%02X] id[%04X]", entry.local,
                                HIGH_LOW_TO_INT(entry.identifier[1], entry.identifier[0]));
    }
  }
}

// plugin groups-server clear
void sli_zigbee_af_groups_server_cli_clear(sl_cli_command_arg_t *args)
{
  (void)args;
  sl_zigbee_af_core_println("Clearing all groups.");
  sl_zigbee_af_groups_cluster_clear_group_table_cb(SL_ZIGBEE_BROADCAST_ENDPOINT);
}
