/***************************************************************************//**
 * @file child_ipc_command_messages.c
 * @brief internal wrappers for 'child' ipc commands
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
// automatically generated from child.h.  Do not manually edit
#include "stack/include/child.h"
#include "stack/internal/inc/child_internal_def.h"
#include "stack/internal/src/ipc/child_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_add_child_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.add_child.response.result = sli_zigbee_stack_add_child(msg->data.add_child.request.shortId,
                                                                   msg->data.add_child.request.longId,
                                                                   msg->data.add_child.request.nodeType);
}

void sli_zigbee_stack_child_count_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.child_count.response.result = sli_zigbee_stack_child_count();
}

void sli_zigbee_stack_child_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.child_id.response.result = sli_zigbee_stack_child_id(msg->data.child_id.request.childIndex);
}

void sli_zigbee_stack_child_index_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.child_index.response.result = sli_zigbee_stack_child_index(msg->data.child_index.request.childId);
}

void sli_zigbee_stack_child_power_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.child_power.response.result = sli_zigbee_stack_child_power(msg->data.child_power.request.childIndex);
}

void sli_zigbee_stack_clear_message_flag_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.clear_message_flag.response.result = sli_zigbee_stack_clear_message_flag(msg->data.clear_message_flag.request.childId);
}

void sli_zigbee_stack_current_stack_tasks_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.current_stack_tasks.response.result = sli_zigbee_stack_current_stack_tasks();
}

void sli_zigbee_stack_get_avg_parent_rssi_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_avg_parent_rssi.response.result = sli_zigbee_stack_get_avg_parent_rssi();
}

void sli_zigbee_stack_get_child_data_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_child_data.response.result = sli_zigbee_stack_get_child_data(msg->data.get_child_data.request.index,
                                                                             &msg->data.get_child_data.request.childData);
}

void sli_zigbee_stack_get_keep_alive_mode_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_keep_alive_mode.response.result = sli_zigbee_stack_get_keep_alive_mode();
}

void sli_zigbee_stack_get_max_end_device_children_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_max_end_device_children.response.result = sli_zigbee_stack_get_max_end_device_children();
}

void sli_zigbee_stack_get_parent_eui64_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_parent_eui64.response.result = sli_zigbee_stack_get_parent_eui64();
}

void sli_zigbee_stack_get_parent_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_parent_id.response.result = sli_zigbee_stack_get_parent_id();
}

void sli_zigbee_stack_get_parent_incoming_nwk_frame_counter_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_parent_incoming_nwk_frame_counter.response.result = sli_zigbee_stack_get_parent_incoming_nwk_frame_counter();
}

void sli_zigbee_stack_get_parent_node_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_parent_node_id.response.result = sli_zigbee_stack_get_parent_node_id();
}

void sli_zigbee_stack_parent_token_set_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.parent_token_set.response.result = sli_zigbee_stack_parent_token_set();
}

void sli_zigbee_stack_poll_for_data_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.poll_for_data.response.result = sli_zigbee_stack_poll_for_data();
}

void sli_zigbee_stack_remove_child_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.remove_child.response.result = sli_zigbee_stack_remove_child(msg->data.remove_child.request.childEui64);
}

void sli_zigbee_stack_remove_neighbor_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_remove_neighbor(msg->data.remove_neighbor.request.shortId,
                                   msg->data.remove_neighbor.request.longId);
}

void sli_zigbee_stack_send_link_power_delta_request_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_link_power_delta_request.response.result = sli_zigbee_stack_send_link_power_delta_request();
}

void sli_zigbee_stack_set_child_data_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_child_data.response.result = sli_zigbee_stack_set_child_data(msg->data.set_child_data.request.index,
                                                                             &msg->data.set_child_data.request.childData);
}

void sli_zigbee_stack_set_child_power_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_child_power(msg->data.set_child_power.request.childIndex,
                                   msg->data.set_child_power.request.newPower);
}

void sli_zigbee_stack_set_child_timeout_option_mask_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_child_timeout_option_mask.response.result = sli_zigbee_stack_set_child_timeout_option_mask(msg->data.set_child_timeout_option_mask.request.mask);
}

void sli_zigbee_stack_set_keep_alive_mode_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_keep_alive_mode.response.result = sli_zigbee_stack_set_keep_alive_mode(msg->data.set_keep_alive_mode.request.mode);
}

void sli_zigbee_stack_set_mac_poll_failure_wait_time_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_mac_poll_failure_wait_time(msg->data.set_mac_poll_failure_wait_time.request.waitBeforeRetryIntervalMs);
}

void sli_zigbee_stack_set_max_end_device_children_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_max_end_device_children.response.result = sli_zigbee_stack_set_max_end_device_children(msg->data.set_max_end_device_children.request.max);
}

void sli_zigbee_stack_set_message_flag_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_message_flag.response.result = sli_zigbee_stack_set_message_flag(msg->data.set_message_flag.request.childId);
}

void sli_zigbee_stack_set_parent_incoming_nwk_frame_counter_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_parent_incoming_nwk_frame_counter.response.result = sli_zigbee_stack_set_parent_incoming_nwk_frame_counter(msg->data.set_parent_incoming_nwk_frame_counter.request.value);
}

void sli_zigbee_stack_stack_power_down_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_zigbee_stack_stack_power_down();
}

void sli_zigbee_stack_stack_power_up_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_zigbee_stack_stack_power_up();
}

// public entrypoints

sl_status_t sl_zigbee_add_child(sl_802154_short_addr_t shortId,
                                sl_802154_long_addr_t longId,
                                sl_zigbee_node_type_t nodeType)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.add_child.request.shortId = shortId;

  if (longId != NULL) {
    memmove(msg.data.add_child.request.longId, longId, sizeof(sl_802154_long_addr_t));
  }

  msg.data.add_child.request.nodeType = nodeType;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_add_child_process_ipc_command, &msg);

  if (longId != NULL) {
    memmove(longId, msg.data.add_child.request.longId, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.add_child.response.result;
}

uint8_t sl_zigbee_child_count(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_child_count_process_ipc_command, &msg);

  return msg.data.child_count.response.result;
}

sl_802154_short_addr_t sl_zigbee_child_id(uint8_t childIndex)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.child_id.request.childIndex = childIndex;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_child_id_process_ipc_command, &msg);

  return msg.data.child_id.response.result;
}

uint8_t sl_zigbee_child_index(sl_802154_short_addr_t childId)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.child_index.request.childId = childId;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_child_index_process_ipc_command, &msg);

  return msg.data.child_index.response.result;
}

int8_t sl_zigbee_child_power(uint8_t childIndex)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.child_power.request.childIndex = childIndex;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_child_power_process_ipc_command, &msg);

  return msg.data.child_power.response.result;
}

sl_status_t sl_zigbee_clear_message_flag(sl_802154_short_addr_t childId)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.clear_message_flag.request.childId = childId;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_clear_message_flag_process_ipc_command, &msg);

  return msg.data.clear_message_flag.response.result;
}

uint16_t sl_zigbee_current_stack_tasks(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_current_stack_tasks_process_ipc_command, &msg);

  return msg.data.current_stack_tasks.response.result;
}

int8_t sl_zigbee_get_avg_parent_rssi(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_avg_parent_rssi_process_ipc_command, &msg);

  return msg.data.get_avg_parent_rssi.response.result;
}

sl_status_t sl_zigbee_get_child_data(uint8_t index,
                                     sl_zigbee_child_data_t *childData)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_child_data.request.index = index;

  if (childData != NULL) {
    msg.data.get_child_data.request.childData = *childData;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_child_data_process_ipc_command, &msg);

  if (childData != NULL) {
    *childData = msg.data.get_child_data.request.childData;
  }

  return msg.data.get_child_data.response.result;
}

sl_zigbee_keep_alive_mode_t sl_zigbee_get_keep_alive_mode(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_keep_alive_mode_process_ipc_command, &msg);

  return msg.data.get_keep_alive_mode.response.result;
}

uint8_t sl_zigbee_get_max_end_device_children(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_max_end_device_children_process_ipc_command, &msg);

  return msg.data.get_max_end_device_children.response.result;
}

uint8_t * sl_zigbee_get_parent_eui64(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_parent_eui64_process_ipc_command, &msg);

  return msg.data.get_parent_eui64.response.result;
}

sl_802154_short_addr_t sl_zigbee_get_parent_id(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_parent_id_process_ipc_command, &msg);

  return msg.data.get_parent_id.response.result;
}

uint32_t sl_zigbee_get_parent_incoming_nwk_frame_counter(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_parent_incoming_nwk_frame_counter_process_ipc_command, &msg);

  return msg.data.get_parent_incoming_nwk_frame_counter.response.result;
}

sl_802154_short_addr_t sl_zigbee_get_parent_node_id(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_parent_node_id_process_ipc_command, &msg);

  return msg.data.get_parent_node_id.response.result;
}

bool sl_zigbee_parent_token_set(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_parent_token_set_process_ipc_command, &msg);

  return msg.data.parent_token_set.response.result;
}

sl_status_t sl_zigbee_poll_for_data(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_poll_for_data_process_ipc_command, &msg);

  return msg.data.poll_for_data.response.result;
}

sl_status_t sl_zigbee_remove_child(sl_802154_long_addr_t childEui64)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (childEui64 != NULL) {
    memmove(msg.data.remove_child.request.childEui64, childEui64, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_remove_child_process_ipc_command, &msg);

  if (childEui64 != NULL) {
    memmove(childEui64, msg.data.remove_child.request.childEui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.remove_child.response.result;
}

void sl_zigbee_remove_neighbor(sl_802154_short_addr_t shortId,
                               sl_802154_long_addr_t longId)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.remove_neighbor.request.shortId = shortId;

  if (longId != NULL) {
    memmove(msg.data.remove_neighbor.request.longId, longId, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_remove_neighbor_process_ipc_command, &msg);

  if (longId != NULL) {
    memmove(longId, msg.data.remove_neighbor.request.longId, sizeof(sl_802154_long_addr_t));
  }
}

sl_status_t sl_zigbee_send_link_power_delta_request(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_link_power_delta_request_process_ipc_command, &msg);

  return msg.data.send_link_power_delta_request.response.result;
}

sl_status_t sl_zigbee_set_child_data(uint8_t index,
                                     const sl_zigbee_child_data_t *childData)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_child_data.request.index = index;

  if (childData != NULL) {
    msg.data.set_child_data.request.childData = *childData;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_child_data_process_ipc_command, &msg);

  return msg.data.set_child_data.response.result;
}

void sl_zigbee_set_child_power(uint8_t childIndex,
                               int8_t newPower)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_child_power.request.childIndex = childIndex;
  msg.data.set_child_power.request.newPower = newPower;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_child_power_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_set_child_timeout_option_mask(uint16_t mask)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_child_timeout_option_mask.request.mask = mask;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_child_timeout_option_mask_process_ipc_command, &msg);

  return msg.data.set_child_timeout_option_mask.response.result;
}

sl_status_t sl_zigbee_set_keep_alive_mode(sl_zigbee_keep_alive_mode_t mode)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_keep_alive_mode.request.mode = mode;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_keep_alive_mode_process_ipc_command, &msg);

  return msg.data.set_keep_alive_mode.response.result;
}

void sl_zigbee_set_mac_poll_failure_wait_time(uint32_t waitBeforeRetryIntervalMs)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_mac_poll_failure_wait_time.request.waitBeforeRetryIntervalMs = waitBeforeRetryIntervalMs;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_mac_poll_failure_wait_time_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_set_max_end_device_children(uint8_t max)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_max_end_device_children.request.max = max;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_max_end_device_children_process_ipc_command, &msg);

  return msg.data.set_max_end_device_children.response.result;
}

sl_status_t sl_zigbee_set_message_flag(sl_802154_short_addr_t childId)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_message_flag.request.childId = childId;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_message_flag_process_ipc_command, &msg);

  return msg.data.set_message_flag.response.result;
}

sl_status_t sl_zigbee_set_parent_incoming_nwk_frame_counter(uint32_t value)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_parent_incoming_nwk_frame_counter.request.value = value;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_parent_incoming_nwk_frame_counter_process_ipc_command, &msg);

  return msg.data.set_parent_incoming_nwk_frame_counter.response.result;
}

void sl_zigbee_stack_power_down(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_stack_power_down_process_ipc_command, &msg);
}

void sl_zigbee_stack_power_up(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_stack_power_up_process_ipc_command, &msg);
}
