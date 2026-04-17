/***************************************************************************//**
 * @file raw-message-ipc-command-messages.c
 * @brief internal wrappers for 'raw-message' ipc commands
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
// automatically generated from raw-message.h.  Do not manually edit
#include "stack/include/raw-message.h"
#include "stack/internal/inc/raw-message-internal-def.h"
#include "stack/internal/src/ipc/raw-message-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_get_embernet_passthrough_source_address_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_embernet_passthrough_source_address.response.result = sli_zigbee_stack_get_embernet_passthrough_source_address();
}

void sli_zigbee_stack_get_mac_passthrough_flags_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_mac_passthrough_flags.response.result = sli_zigbee_stack_get_mac_passthrough_flags();
}

void sli_zigbee_stack_send_raw_message_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_raw_message.response.result = sli_zigbee_stack_send_raw_message(msg->data.send_raw_message.request.message,
                                                                                 msg->data.send_raw_message.request.message_length,
                                                                                 msg->data.send_raw_message.request.priority,
                                                                                 msg->data.send_raw_message.request.useCca);
}

void sli_zigbee_stack_send_raw_message_with_tag_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_raw_message_with_tag.response.result = sli_zigbee_stack_send_raw_message_with_tag(msg->data.send_raw_message_with_tag.request.message,
                                                                                                   msg->data.send_raw_message_with_tag.request.message_length,
                                                                                                   msg->data.send_raw_message_with_tag.request.priority,
                                                                                                   msg->data.send_raw_message_with_tag.request.useCca,
                                                                                                   msg->data.send_raw_message_with_tag.request.messageTag);
}

void sli_zigbee_stack_set_embernet_passthrough_source_address_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_embernet_passthrough_source_address(msg->data.set_embernet_passthrough_source_address.request.address);
}

void sli_zigbee_stack_set_mac_filter_match_list_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_mac_filter_match_list.response.result = sli_zigbee_stack_set_mac_filter_match_list(msg->data.set_mac_filter_match_list.request.macFilterMatchList,
                                                                                                   msg->data.set_mac_filter_match_list.request.listLength);
}

void sli_zigbee_stack_set_mac_passthrough_flags_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_mac_passthrough_flags(msg->data.set_mac_passthrough_flags.request.type);
}

// public entrypoints

sl_802154_short_addr_t sl_zigbee_get_embernet_passthrough_source_address(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_embernet_passthrough_source_address_process_ipc_command, &msg);

  return msg.data.get_embernet_passthrough_source_address.response.result;
}

sl_zigbee_mac_passthrough_type_t sl_zigbee_get_mac_passthrough_flags(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_mac_passthrough_flags_process_ipc_command, &msg);

  return msg.data.get_mac_passthrough_flags.response.result;
}

sl_status_t sl_zigbee_send_raw_message(const uint8_t *message,
                                       uint8_t message_length,
                                       sl_zigbee_transmit_priority_t priority,
                                       bool useCca)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (message_length > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector message length exceeds expected maximum
  }

  memmove(msg.data.send_raw_message.request.message, message, sizeof(uint8_t) * message_length);
  msg.data.send_raw_message.request.message_length = message_length;
  msg.data.send_raw_message.request.priority = priority;
  msg.data.send_raw_message.request.useCca = useCca;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_raw_message_process_ipc_command, &msg);

  return msg.data.send_raw_message.response.result;
}

sl_status_t sl_zigbee_send_raw_message_with_tag(const uint8_t *message,
                                                uint8_t message_length,
                                                sl_zigbee_transmit_priority_t priority,
                                                bool useCca,
                                                uint8_t messageTag)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (message_length > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector message length exceeds expected maximum
  }

  memmove(msg.data.send_raw_message_with_tag.request.message, message, sizeof(uint8_t) * message_length);
  msg.data.send_raw_message_with_tag.request.message_length = message_length;
  msg.data.send_raw_message_with_tag.request.priority = priority;
  msg.data.send_raw_message_with_tag.request.useCca = useCca;
  msg.data.send_raw_message_with_tag.request.messageTag = messageTag;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_raw_message_with_tag_process_ipc_command, &msg);

  return msg.data.send_raw_message_with_tag.response.result;
}

void sl_zigbee_set_embernet_passthrough_source_address(sl_802154_short_addr_t address)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_embernet_passthrough_source_address.request.address = address;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_embernet_passthrough_source_address_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_set_mac_filter_match_list(const sl_zigbee_mac_filter_match_data_t *macFilterMatchList,
                                                uint8_t listLength)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (listLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector macFilterMatchList length exceeds expected maximum
  }

  memmove(msg.data.set_mac_filter_match_list.request.macFilterMatchList, macFilterMatchList, sizeof(sl_zigbee_mac_filter_match_data_t) * listLength);
  msg.data.set_mac_filter_match_list.request.listLength = listLength;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_mac_filter_match_list_process_ipc_command, &msg);

  return msg.data.set_mac_filter_match_list.response.result;
}

void sl_zigbee_set_mac_passthrough_flags(sl_zigbee_mac_passthrough_type_t type)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_mac_passthrough_flags.request.type = type;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_mac_passthrough_flags_process_ipc_command, &msg);
}
