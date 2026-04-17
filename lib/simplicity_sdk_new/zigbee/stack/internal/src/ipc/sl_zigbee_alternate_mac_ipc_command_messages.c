/***************************************************************************//**
 * @file sl_zigbee_alternate_mac_ipc_command_messages.c
 * @brief internal wrappers for 'sl_zigbee_alternate_mac' ipc commands
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from sl_zigbee_alternate_mac.h.  Do not manually edit
#include "stack/include/sl_zigbee_alternate_mac.h"
#include "stack/internal/inc/sl_zigbee_alternate_mac_internal_def.h"
#include "stack/internal/src/ipc/sl_zigbee_alternate_mac_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_alt_mac_init_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.alt_mac_init.response.result = sli_zigbee_stack_alt_mac_init(&msg->data.alt_mac_init.request.config);
}

// public entrypoints

sl_zigbee_mac_interface_id_t sl_zigbee_alt_mac_init(const sl_zigbee_alt_mac_config_t *config)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (config != NULL) {
    msg.data.alt_mac_init.request.config = *config;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_alt_mac_init_process_ipc_command, &msg);

  return msg.data.alt_mac_init.response.result;
}
