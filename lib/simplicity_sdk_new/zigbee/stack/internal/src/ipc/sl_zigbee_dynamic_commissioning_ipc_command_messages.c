/***************************************************************************//**
 * @file sl_zigbee_dynamic_commissioning_ipc_command_messages.c
 * @brief internal wrappers for 'sl_zigbee_dynamic_commissioning' ipc commands
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
// automatically generated from sl_zigbee_dynamic_commissioning.h.  Do not manually edit
#include "stack/include/sl_zigbee_dynamic_commissioning.h"
#include "stack/internal/inc/sl_zigbee_dynamic_commissioning_internal_def.h"
#include "stack/internal/src/ipc/sl_zigbee_dynamic_commissioning_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_device_interview_status_update_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_device_interview_status_update(&msg->data.device_interview_status_update.request.ids,
                                                  msg->data.device_interview_status_update.request.event);
}

void sli_zigbee_stack_dynamic_commissioning_is_open_for_interview_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.dynamic_commissioning_is_open_for_interview.response.result = sli_zigbee_stack_dynamic_commissioning_is_open_for_interview();
}

void sli_zigbee_stack_dynamic_commissioning_set_open_for_interview_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_dynamic_commissioning_set_open_for_interview(msg->data.dynamic_commissioning_set_open_for_interview.request.open_interview);
}

// public entrypoints

void sl_zigbee_device_interview_status_update(sl_zigbee_address_info *ids,
                                              sl_zigbee_dynamic_commissioning_event_t event)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (ids != NULL) {
    msg.data.device_interview_status_update.request.ids = *ids;
  }

  msg.data.device_interview_status_update.request.event = event;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_device_interview_status_update_process_ipc_command, &msg);

  if (ids != NULL) {
    *ids = msg.data.device_interview_status_update.request.ids;
  }
}

bool sl_zigbee_dynamic_commissioning_is_open_for_interview(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_dynamic_commissioning_is_open_for_interview_process_ipc_command, &msg);

  return msg.data.dynamic_commissioning_is_open_for_interview.response.result;
}

void sl_zigbee_dynamic_commissioning_set_open_for_interview(bool open_interview)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.dynamic_commissioning_set_open_for_interview.request.open_interview = open_interview;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_dynamic_commissioning_set_open_for_interview_process_ipc_command, &msg);
}
