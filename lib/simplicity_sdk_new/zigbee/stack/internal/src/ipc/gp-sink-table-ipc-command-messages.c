/***************************************************************************//**
 * @file gp-sink-table-ipc-command-messages.c
 * @brief internal wrappers for 'gp-sink-table' ipc commands
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
// automatically generated from gp-sink-table.h.  Do not manually edit
#include "stack/include/gp-sink-table.h"
#include "stack/internal/inc/gp-sink-table-internal-def.h"
#include "stack/internal/src/ipc/gp-sink-table-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_gp_sink_table_add_group_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_gp_sink_table_add_group(msg->data.gp_sink_table_add_group.request.index,
                                           msg->data.gp_sink_table_add_group.request.sinkGroupId,
                                           msg->data.gp_sink_table_add_group.request.assignedAlias);
}

void sli_zigbee_stack_gp_sink_table_clear_all_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_zigbee_stack_gp_sink_table_clear_all();
}

void sli_zigbee_stack_gp_sink_table_entry_in_use_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_sink_table_entry_in_use.response.result = sli_zigbee_stack_gp_sink_table_entry_in_use(msg->data.gp_sink_table_entry_in_use.request.sinkTableIndex);
}

void sli_zigbee_stack_gp_sink_table_find_or_allocate_entry_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_sink_table_find_or_allocate_entry.response.result = sli_zigbee_stack_gp_sink_table_find_or_allocate_entry(&msg->data.gp_sink_table_find_or_allocate_entry.request.addr);
}

void sli_zigbee_stack_gp_sink_table_get_entry_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_sink_table_get_entry.response.result = sli_zigbee_stack_gp_sink_table_get_entry(msg->data.gp_sink_table_get_entry.request.sinkTableIndex,
                                                                                               &msg->data.gp_sink_table_get_entry.request.entry);
}

void sli_zigbee_stack_gp_sink_table_get_free_entry_index_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_sink_table_get_free_entry_index.response.result = sli_zigbee_stack_gp_sink_table_get_free_entry_index();
}

void sli_zigbee_stack_gp_sink_table_get_number_of_active_entries_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_sink_table_get_number_of_active_entries.response.result = sli_zigbee_stack_gp_sink_table_get_number_of_active_entries();
}

void sli_zigbee_stack_gp_sink_table_init_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_zigbee_stack_gp_sink_table_init();
}

void sli_zigbee_stack_gp_sink_table_lookup_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_sink_table_lookup.response.result = sli_zigbee_stack_gp_sink_table_lookup(&msg->data.gp_sink_table_lookup.request.addr);
}

void sli_zigbee_stack_gp_sink_table_remove_entry_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_gp_sink_table_remove_entry(msg->data.gp_sink_table_remove_entry.request.index);
}

void sli_zigbee_stack_gp_sink_table_remove_group_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_sink_table_remove_group.response.result = sli_zigbee_stack_gp_sink_table_remove_group(msg->data.gp_sink_table_remove_group.request.index,
                                                                                                     msg->data.gp_sink_table_remove_group.request.sinkGroupId,
                                                                                                     msg->data.gp_sink_table_remove_group.request.assignedAlias);
}

void sli_zigbee_stack_gp_sink_table_set_entry_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_sink_table_set_entry.response.result = sli_zigbee_stack_gp_sink_table_set_entry(msg->data.gp_sink_table_set_entry.request.sinkTableIndex,
                                                                                               &msg->data.gp_sink_table_set_entry.request.entry);
}

void sli_zigbee_stack_gp_sink_table_set_security_frame_counter_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_gp_sink_table_set_security_frame_counter(msg->data.gp_sink_table_set_security_frame_counter.request.index,
                                                            msg->data.gp_sink_table_set_security_frame_counter.request.sfc);
}

// public entrypoints

void sl_zigbee_gp_sink_table_add_group(uint8_t index,
                                       uint16_t sinkGroupId,
                                       uint16_t assignedAlias)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.gp_sink_table_add_group.request.index = index;
  msg.data.gp_sink_table_add_group.request.sinkGroupId = sinkGroupId;
  msg.data.gp_sink_table_add_group.request.assignedAlias = assignedAlias;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_sink_table_add_group_process_ipc_command, &msg);
}

void sl_zigbee_gp_sink_table_clear_all(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_sink_table_clear_all_process_ipc_command, &msg);
}

uint8_t sl_zigbee_gp_sink_table_entry_in_use(uint8_t sinkTableIndex)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.gp_sink_table_entry_in_use.request.sinkTableIndex = sinkTableIndex;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_sink_table_entry_in_use_process_ipc_command, &msg);

  return msg.data.gp_sink_table_entry_in_use.response.result;
}

uint8_t sl_zigbee_gp_sink_table_find_or_allocate_entry(sl_zigbee_gp_address_t *addr)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (addr != NULL) {
    msg.data.gp_sink_table_find_or_allocate_entry.request.addr = *addr;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_sink_table_find_or_allocate_entry_process_ipc_command, &msg);

  if (addr != NULL) {
    *addr = msg.data.gp_sink_table_find_or_allocate_entry.request.addr;
  }

  return msg.data.gp_sink_table_find_or_allocate_entry.response.result;
}

sl_status_t sl_zigbee_gp_sink_table_get_entry(uint8_t sinkTableIndex,
                                              sl_zigbee_gp_sink_table_entry_t *entry)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.gp_sink_table_get_entry.request.sinkTableIndex = sinkTableIndex;

  if (entry != NULL) {
    msg.data.gp_sink_table_get_entry.request.entry = *entry;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_sink_table_get_entry_process_ipc_command, &msg);

  if (entry != NULL) {
    *entry = msg.data.gp_sink_table_get_entry.request.entry;
  }

  return msg.data.gp_sink_table_get_entry.response.result;
}

uint8_t sl_zigbee_gp_sink_table_get_free_entry_index(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_sink_table_get_free_entry_index_process_ipc_command, &msg);

  return msg.data.gp_sink_table_get_free_entry_index.response.result;
}

uint8_t sl_zigbee_gp_sink_table_get_number_of_active_entries(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_sink_table_get_number_of_active_entries_process_ipc_command, &msg);

  return msg.data.gp_sink_table_get_number_of_active_entries.response.result;
}

void sl_zigbee_gp_sink_table_init(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_sink_table_init_process_ipc_command, &msg);
}

uint8_t sl_zigbee_gp_sink_table_lookup(sl_zigbee_gp_address_t *addr)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (addr != NULL) {
    msg.data.gp_sink_table_lookup.request.addr = *addr;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_sink_table_lookup_process_ipc_command, &msg);

  if (addr != NULL) {
    *addr = msg.data.gp_sink_table_lookup.request.addr;
  }

  return msg.data.gp_sink_table_lookup.response.result;
}

void sl_zigbee_gp_sink_table_remove_entry(uint8_t index)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.gp_sink_table_remove_entry.request.index = index;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_sink_table_remove_entry_process_ipc_command, &msg);
}

bool sl_zigbee_gp_sink_table_remove_group(uint8_t index,
                                          uint16_t sinkGroupId,
                                          uint16_t assignedAlias)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.gp_sink_table_remove_group.request.index = index;
  msg.data.gp_sink_table_remove_group.request.sinkGroupId = sinkGroupId;
  msg.data.gp_sink_table_remove_group.request.assignedAlias = assignedAlias;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_sink_table_remove_group_process_ipc_command, &msg);

  return msg.data.gp_sink_table_remove_group.response.result;
}

sl_status_t sl_zigbee_gp_sink_table_set_entry(uint8_t sinkTableIndex,
                                              sl_zigbee_gp_sink_table_entry_t *entry)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.gp_sink_table_set_entry.request.sinkTableIndex = sinkTableIndex;

  if (entry != NULL) {
    msg.data.gp_sink_table_set_entry.request.entry = *entry;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_sink_table_set_entry_process_ipc_command, &msg);

  if (entry != NULL) {
    *entry = msg.data.gp_sink_table_set_entry.request.entry;
  }

  return msg.data.gp_sink_table_set_entry.response.result;
}

void sl_zigbee_gp_sink_table_set_security_frame_counter(uint8_t index,
                                                        uint32_t sfc)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.gp_sink_table_set_security_frame_counter.request.index = index;
  msg.data.gp_sink_table_set_security_frame_counter.request.sfc = sfc;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_sink_table_set_security_frame_counter_process_ipc_command, &msg);
}
