/***************************************************************************//**
 * @file multi-network-ipc-command-messages.c
 * @brief internal wrappers for 'multi-network' ipc commands
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
// automatically generated from multi-network.h.  Do not manually edit
#include "stack/include/multi-network.h"
#include "stack/internal/inc/multi-network-internal-def.h"
#include "stack/internal/src/ipc/multi-network-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_get_callback_network_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_callback_network.response.result = sli_zigbee_stack_get_callback_network();
}

void sli_zigbee_stack_get_current_network_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_current_network.response.result = sli_zigbee_stack_get_current_network();
}

void sli_zigbee_stack_set_current_network_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_current_network.response.result = sli_zigbee_stack_set_current_network(msg->data.set_current_network.request.index);
}

// public entrypoints

uint8_t sl_zigbee_get_callback_network(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_callback_network_process_ipc_command, &msg);

  return msg.data.get_callback_network.response.result;
}

uint8_t sl_zigbee_get_current_network(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_current_network_process_ipc_command, &msg);

  return msg.data.get_current_network.response.result;
}

sl_status_t sl_zigbee_set_current_network(uint8_t index)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_current_network.request.index = index;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_current_network_process_ipc_command, &msg);

  return msg.data.set_current_network.response.result;
}
