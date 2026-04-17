/***************************************************************************//**
 * @file sl_zigbee_token_ipc_command_messages.c
 * @brief internal wrappers for 'sl_zigbee_token' ipc commands
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
// automatically generated from sl_zigbee_token.h.  Do not manually edit
#include "stack/include/sl_zigbee_token.h"
#include "stack/internal/inc/sl_zigbee_token_internal_def.h"
#include "stack/internal/src/ipc/sl_zigbee_token_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_get_token_count_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_token_count.response.result = sli_zigbee_stack_get_token_count();
}

void sli_zigbee_stack_get_token_data_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_token_data.response.result = sli_zigbee_stack_get_token_data(msg->data.get_token_data.request.token,
                                                                             msg->data.get_token_data.request.index,
                                                                             &msg->data.get_token_data.request.tokenData);
}

void sli_zigbee_stack_get_token_info_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_token_info.response.result = sli_zigbee_stack_get_token_info(msg->data.get_token_info.request.index,
                                                                             &msg->data.get_token_info.request.tokenInfo);
}

void sli_zigbee_stack_set_token_data_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_token_data.response.result = sli_zigbee_stack_set_token_data(msg->data.set_token_data.request.token,
                                                                             msg->data.set_token_data.request.index,
                                                                             &msg->data.set_token_data.request.tokenData);
}

// public entrypoints

uint32_t sl_zigbee_get_token_count(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_token_count_process_ipc_command, &msg);

  return msg.data.get_token_count.response.result;
}

sl_status_t sl_zigbee_get_token_data(uint32_t token,
                                     uint32_t index,
                                     sl_zigbee_token_data_t *tokenData)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_token_data.request.token = token;
  msg.data.get_token_data.request.index = index;

  if (tokenData != NULL) {
    msg.data.get_token_data.request.tokenData = *tokenData;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_token_data_process_ipc_command, &msg);

  if (tokenData != NULL) {
    *tokenData = msg.data.get_token_data.request.tokenData;
  }

  return msg.data.get_token_data.response.result;
}

sl_status_t sl_zigbee_get_token_info(uint8_t index,
                                     sl_zigbee_token_info_t *tokenInfo)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_token_info.request.index = index;

  if (tokenInfo != NULL) {
    msg.data.get_token_info.request.tokenInfo = *tokenInfo;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_token_info_process_ipc_command, &msg);

  if (tokenInfo != NULL) {
    *tokenInfo = msg.data.get_token_info.request.tokenInfo;
  }

  return msg.data.get_token_info.response.result;
}

sl_status_t sl_zigbee_set_token_data(uint32_t token,
                                     uint32_t index,
                                     sl_zigbee_token_data_t *tokenData)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_token_data.request.token = token;
  msg.data.set_token_data.request.index = index;

  if (tokenData != NULL) {
    msg.data.set_token_data.request.tokenData = *tokenData;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_token_data_process_ipc_command, &msg);

  if (tokenData != NULL) {
    *tokenData = msg.data.set_token_data.request.tokenData;
  }

  return msg.data.set_token_data.response.result;
}
