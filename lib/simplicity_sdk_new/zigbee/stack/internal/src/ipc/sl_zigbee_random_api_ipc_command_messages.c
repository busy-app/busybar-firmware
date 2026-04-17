/***************************************************************************//**
 * @file sl_zigbee_random_api_ipc_command_messages.c
 * @brief internal wrappers for 'sl_zigbee_random_api' ipc commands
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
// automatically generated from sl_zigbee_random_api.h.  Do not manually edit
#include "stack/include/sl_zigbee_random_api.h"
#include "stack/internal/inc/sl_zigbee_random_api_internal_def.h"
#include "stack/internal/src/ipc/sl_zigbee_random_api_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_get_pseudo_random_number_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_pseudo_random_number.response.result = sli_zigbee_stack_get_pseudo_random_number();
}

void sli_zigbee_stack_get_strong_random_entropy_source_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_strong_random_entropy_source.response.result = sli_zigbee_stack_get_strong_random_entropy_source();
}

void sli_zigbee_stack_get_strong_random_number_array_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_strong_random_number_array.response.result = sli_zigbee_stack_get_strong_random_number_array(msg->data.get_strong_random_number_array.request.randomNumber,
                                                                                                             msg->data.get_strong_random_number_array.request.count);
}

// public entrypoints

uint16_t sl_zigbee_get_pseudo_random_number(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_pseudo_random_number_process_ipc_command, &msg);

  return msg.data.get_pseudo_random_number.response.result;
}

sl_zigbee_entropy_source_t sl_zigbee_get_strong_random_entropy_source(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_strong_random_entropy_source_process_ipc_command, &msg);

  return msg.data.get_strong_random_entropy_source.response.result;
}

sl_status_t sl_zigbee_get_strong_random_number_array(uint16_t *randomNumber,
                                                     uint8_t count)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (count > MAX_IPC_VEC_ARG_CAPACITY / sizeof(uint16_t)) {
    assert(false); // "vector randomNumber length exceeds expected maximum
  }

  memmove(msg.data.get_strong_random_number_array.request.randomNumber, randomNumber, sizeof(uint16_t) * count);
  msg.data.get_strong_random_number_array.request.count = count;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_strong_random_number_array_process_ipc_command, &msg);

  if (count > MAX_IPC_VEC_ARG_CAPACITY / sizeof(uint16_t)) {
    assert(false); // "vector randomNumber length exceeds expected maximum
  }

  memmove(randomNumber, msg.data.get_strong_random_number_array.request.randomNumber, sizeof(uint16_t) * count);
  return msg.data.get_strong_random_number_array.response.result;
}
