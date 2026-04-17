/***************************************************************************//**
 * @file zigbee-dynamic-node-type-ipc-command-messages.c
 * @brief internal wrappers for 'zigbee-dynamic-node-type' ipc commands
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
// automatically generated from zigbee-dynamic-node-type.h.  Do not manually edit
#include "stack/include/zigbee-dynamic-node-type.h"
#include "stack/internal/inc/zigbee-dynamic-node-type-internal-def.h"
#include "stack/internal/src/ipc/zigbee-dynamic-node-type-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void slxi_zigbee_stack_switch_role_router_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.switch_role_router.response.result = slxi_zigbee_stack_switch_role_router();
}

void slxi_zigbee_stack_switch_role_sleepy_end_device_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.switch_role_sleepy_end_device.response.result = slxi_zigbee_stack_switch_role_sleepy_end_device(msg->data.switch_role_sleepy_end_device.request.end_device_configuration);
}

// public entrypoints

sl_status_t slx_zigbee_switch_role_router(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_switch_role_router_process_ipc_command, &msg);

  return msg.data.switch_role_router.response.result;
}

sl_status_t slx_zigbee_switch_role_sleepy_end_device(uint8_t end_device_configuration)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.switch_role_sleepy_end_device.request.end_device_configuration = end_device_configuration;
  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_switch_role_sleepy_end_device_process_ipc_command, &msg);

  return msg.data.switch_role_sleepy_end_device.response.result;
}
