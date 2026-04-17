/***************************************************************************//**
 * @file binding-table-ipc-command-messages.c
 * @brief internal wrappers for 'binding-table' ipc commands
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
// automatically generated from binding-table.h.  Do not manually edit
#include "stack/include/binding-table.h"
#include "stack/internal/inc/binding-table-internal-def.h"
#include "stack/internal/src/ipc/binding-table-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_binding_is_active_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.binding_is_active.response.result = sli_zigbee_stack_binding_is_active(msg->data.binding_is_active.request.index);
}

void sli_zigbee_stack_clear_binding_table_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.clear_binding_table.response.result = sli_zigbee_stack_clear_binding_table();
}

void sli_zigbee_stack_clear_binding_table_on_leave_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_clear_binding_table_on_leave(msg->data.clear_binding_table_on_leave.request.clear);
}

void sli_zigbee_stack_delete_binding_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.delete_binding.response.result = sli_zigbee_stack_delete_binding(msg->data.delete_binding.request.index);
}

void sli_zigbee_stack_get_binding_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_binding.response.result = sli_zigbee_stack_get_binding(msg->data.get_binding.request.index,
                                                                       &msg->data.get_binding.request.result);
}

void sli_zigbee_stack_get_binding_remote_node_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_binding_remote_node_id.response.result = sli_zigbee_stack_get_binding_remote_node_id(msg->data.get_binding_remote_node_id.request.index);
}

void sli_zigbee_stack_get_binding_table_size_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_binding_table_size.response.result = sli_zigbee_stack_get_binding_table_size();
}

void sli_zigbee_stack_note_senders_binding_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.note_senders_binding.response.result = sli_zigbee_stack_note_senders_binding(msg->data.note_senders_binding.request.index,
                                                                                         msg->data.note_senders_binding.request.source);
}

void sli_zigbee_stack_set_binding_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_binding.response.result = sli_zigbee_stack_set_binding(msg->data.set_binding.request.index,
                                                                       &msg->data.set_binding.request.value);
}

void sli_zigbee_stack_set_binding_remote_node_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_binding_remote_node_id(msg->data.set_binding_remote_node_id.request.index,
                                              msg->data.set_binding_remote_node_id.request.id);
}

void sli_zigbee_stack_set_binding_table_size_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_binding_table_size.response.result = sli_zigbee_stack_set_binding_table_size(msg->data.set_binding_table_size.request.size);
}

void sli_zigbee_stack_set_reply_binding_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_reply_binding.response.result = sli_zigbee_stack_set_reply_binding(msg->data.set_reply_binding.request.index,
                                                                                   &msg->data.set_reply_binding.request.entry,
                                                                                   msg->data.set_reply_binding.request.source);
}

// public entrypoints

bool sl_zigbee_binding_is_active(uint8_t index)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.binding_is_active.request.index = index;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_binding_is_active_process_ipc_command, &msg);

  return msg.data.binding_is_active.response.result;
}

sl_status_t sl_zigbee_clear_binding_table(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_clear_binding_table_process_ipc_command, &msg);

  return msg.data.clear_binding_table.response.result;
}

void sl_zigbee_clear_binding_table_on_leave(bool clear)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.clear_binding_table_on_leave.request.clear = clear;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_clear_binding_table_on_leave_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_delete_binding(uint8_t index)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.delete_binding.request.index = index;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_delete_binding_process_ipc_command, &msg);

  return msg.data.delete_binding.response.result;
}

sl_status_t sl_zigbee_get_binding(uint8_t index,
                                  sl_zigbee_binding_table_entry_t *result)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_binding.request.index = index;

  if (result != NULL) {
    msg.data.get_binding.request.result = *result;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_binding_process_ipc_command, &msg);

  if (result != NULL) {
    *result = msg.data.get_binding.request.result;
  }

  return msg.data.get_binding.response.result;
}

sl_802154_short_addr_t sl_zigbee_get_binding_remote_node_id(uint8_t index)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_binding_remote_node_id.request.index = index;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_binding_remote_node_id_process_ipc_command, &msg);

  return msg.data.get_binding_remote_node_id.response.result;
}

uint8_t sl_zigbee_get_binding_table_size(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_binding_table_size_process_ipc_command, &msg);

  return msg.data.get_binding_table_size.response.result;
}

sl_status_t sl_zigbee_note_senders_binding(uint8_t index,
                                           sl_802154_short_addr_t source)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.note_senders_binding.request.index = index;
  msg.data.note_senders_binding.request.source = source;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_note_senders_binding_process_ipc_command, &msg);

  return msg.data.note_senders_binding.response.result;
}

sl_status_t sl_zigbee_set_binding(uint8_t index,
                                  sl_zigbee_binding_table_entry_t *value)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_binding.request.index = index;

  if (value != NULL) {
    msg.data.set_binding.request.value = *value;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_binding_process_ipc_command, &msg);

  if (value != NULL) {
    *value = msg.data.set_binding.request.value;
  }

  return msg.data.set_binding.response.result;
}

void sl_zigbee_set_binding_remote_node_id(uint8_t index,
                                          sl_802154_short_addr_t id)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_binding_remote_node_id.request.index = index;
  msg.data.set_binding_remote_node_id.request.id = id;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_binding_remote_node_id_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_set_binding_table_size(uint8_t size)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_binding_table_size.request.size = size;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_binding_table_size_process_ipc_command, &msg);

  return msg.data.set_binding_table_size.response.result;
}

sl_status_t sl_zigbee_set_reply_binding(uint8_t index,
                                        sl_zigbee_binding_table_entry_t *entry,
                                        sl_802154_short_addr_t source)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_reply_binding.request.index = index;

  if (entry != NULL) {
    msg.data.set_reply_binding.request.entry = *entry;
  }

  msg.data.set_reply_binding.request.source = source;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_reply_binding_process_ipc_command, &msg);

  if (entry != NULL) {
    *entry = msg.data.set_reply_binding.request.entry;
  }

  return msg.data.set_reply_binding.response.result;
}
