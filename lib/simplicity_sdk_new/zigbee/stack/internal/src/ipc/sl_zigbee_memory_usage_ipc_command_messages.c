/***************************************************************************//**
 * @file sl_zigbee_memory_usage_ipc_command_messages.c
 * @brief internal wrappers for 'sl_zigbee_memory_usage' ipc commands
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from sl_zigbee_memory_usage.h.  Do not manually edit
#include "stack/include/sl_zigbee_memory_usage.h"
#include "stack/internal/inc/sl_zigbee_memory_usage_internal_def.h"
#include "stack/internal/src/ipc/sl_zigbee_memory_usage_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_get_memory_usage_data_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_memory_usage_data.response.result = sli_zigbee_stack_get_memory_usage_data(msg->data.get_memory_usage_data.request.type);
}

void sli_zigbee_stack_memory_usage_init_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_zigbee_stack_memory_usage_init();
}

// public entrypoints

uint32_t sl_zigbee_get_memory_usage_data(sl_zigbee_memory_usage_data_t type)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_memory_usage_data.request.type = type;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_memory_usage_data_process_ipc_command, &msg);

  return msg.data.get_memory_usage_data.response.result;
}

void sl_zigbee_memory_usage_init(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_memory_usage_init_process_ipc_command, &msg);
}
