/***************************************************************************//**
 * @file sl_zigbee_zdo_configuration_ipc_command_messages.c
 * @brief internal wrappers for 'sl_zigbee_zdo_configuration' ipc commands
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
// automatically generated from sl_zigbee_zdo_configuration.h.  Do not manually edit
#include "stack/include/sl_zigbee_zdo_configuration.h"
#include "stack/internal/inc/sl_zigbee_zdo_configuration_internal_def.h"
#include "stack/internal/src/ipc/sl_zigbee_zdo_configuration_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_zdo_get_configuration_req_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zdo_get_configuration_req.response.result = sli_zigbee_stack_zdo_get_configuration_req(msg->data.zdo_get_configuration_req.request.device_short,
                                                                                                   msg->data.zdo_get_configuration_req.request.encrypt,
                                                                                                   msg->data.zdo_get_configuration_req.request.tag_ids,
                                                                                                   msg->data.zdo_get_configuration_req.request.count);
}

void sli_zigbee_stack_zdo_set_add_configuration_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zdo_set_add_configuration.response.result = sli_zigbee_stack_zdo_set_add_configuration(msg->data.zdo_set_add_configuration.request.tag_id,
                                                                                                   msg->data.zdo_set_add_configuration.request.zdo_config_arg_len,
                                                                                                   msg->data.zdo_set_add_configuration.request.zdo_config_arg);
}

void sli_zigbee_stack_zdo_set_send_configuration_req_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zdo_set_send_configuration_req.response.result = sli_zigbee_stack_zdo_set_send_configuration_req(msg->data.zdo_set_send_configuration_req.request.device_short,
                                                                                                             msg->data.zdo_set_send_configuration_req.request.encrypt);
}

// public entrypoints

sl_status_t sl_zigbee_zdo_get_configuration_req(sl_802154_short_addr_t device_short,
                                                bool encrypt,
                                                uint8_t *tag_ids,
                                                uint8_t count)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.zdo_get_configuration_req.request.device_short = device_short;
  msg.data.zdo_get_configuration_req.request.encrypt = encrypt;

  if (count > 10) {
    assert(false); // "vector tag_ids length exceeds expected maximum
  }

  memmove(msg.data.zdo_get_configuration_req.request.tag_ids, tag_ids, sizeof(uint8_t) * count);
  msg.data.zdo_get_configuration_req.request.count = count;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zdo_get_configuration_req_process_ipc_command, &msg);

  if (count > 10) {
    assert(false); // "vector tag_ids length exceeds expected maximum
  }

  memmove(tag_ids, msg.data.zdo_get_configuration_req.request.tag_ids, sizeof(uint8_t) * count);
  return msg.data.zdo_get_configuration_req.response.result;
}

sl_status_t sl_zigbee_zdo_set_add_configuration(uint8_t tag_id,
                                                size_t zdo_config_arg_len,
                                                const uint8_t *zdo_config_arg)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.zdo_set_add_configuration.request.tag_id = tag_id;
  msg.data.zdo_set_add_configuration.request.zdo_config_arg_len = zdo_config_arg_len;

  if (zdo_config_arg_len > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector zdo_config_arg length exceeds expected maximum
  }

  memmove(msg.data.zdo_set_add_configuration.request.zdo_config_arg, zdo_config_arg, sizeof(uint8_t) * zdo_config_arg_len);
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zdo_set_add_configuration_process_ipc_command, &msg);

  return msg.data.zdo_set_add_configuration.response.result;
}

sl_status_t sl_zigbee_zdo_set_send_configuration_req(sl_802154_short_addr_t device_short,
                                                     bool encrypt)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.zdo_set_send_configuration_req.request.device_short = device_short;
  msg.data.zdo_set_send_configuration_req.request.encrypt = encrypt;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zdo_set_send_configuration_req_process_ipc_command, &msg);

  return msg.data.zdo_set_send_configuration_req.response.result;
}
