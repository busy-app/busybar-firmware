/***************************************************************************//**
 * @file zigbee-device-stack-ipc-command-messages.c
 * @brief internal wrappers for 'zigbee-device-stack' ipc commands
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
// automatically generated from zigbee-device-stack.h.  Do not manually edit
#include "stack/include/zigbee-device-stack.h"
#include "stack/internal/inc/zigbee-device-stack-internal-def.h"
#include "stack/internal/src/ipc/zigbee-device-stack-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_energy_scan_request_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.energy_scan_request.response.result = sli_zigbee_stack_energy_scan_request(msg->data.energy_scan_request.request.target,
                                                                                       msg->data.energy_scan_request.request.scanChannels,
                                                                                       msg->data.energy_scan_request.request.scanDuration,
                                                                                       msg->data.energy_scan_request.request.scanCount);
}

void sli_zigbee_stack_get_last_stack_zig_dev_request_sequence_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_last_stack_zig_dev_request_sequence.response.result = sli_zigbee_stack_get_last_stack_zig_dev_request_sequence();
}

void sli_zigbee_stack_ieee_address_request_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.ieee_address_request.response.result = sli_zigbee_stack_ieee_address_request(msg->data.ieee_address_request.request.target,
                                                                                         msg->data.ieee_address_request.request.reportKids,
                                                                                         msg->data.ieee_address_request.request.childStartIndex,
                                                                                         msg->data.ieee_address_request.request.options);
}

void sli_zigbee_stack_ieee_address_request_to_target_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.ieee_address_request_to_target.response.result = sli_zigbee_stack_ieee_address_request_to_target(msg->data.ieee_address_request_to_target.request.discoveryNodeId,
                                                                                                             msg->data.ieee_address_request_to_target.request.reportKids,
                                                                                                             msg->data.ieee_address_request_to_target.request.childStartIndex,
                                                                                                             msg->data.ieee_address_request_to_target.request.options,
                                                                                                             msg->data.ieee_address_request_to_target.request.targetNodeIdOfRequest);
}

void sli_zigbee_stack_network_address_request_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.network_address_request.response.result = sli_zigbee_stack_network_address_request(msg->data.network_address_request.request.target,
                                                                                               msg->data.network_address_request.request.reportKids,
                                                                                               msg->data.network_address_request.request.childStartIndex);
}

void sli_zigbee_stack_send_device_announcement_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_device_announcement.response.result = sli_zigbee_stack_send_device_announcement();
}

void sli_zigbee_stack_send_parent_announcement_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_parent_announcement.response.result = sli_zigbee_stack_send_parent_announcement();
}

void sli_zigbee_stack_set_manufacturer_code_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_manufacturer_code.response.result = sli_zigbee_stack_set_manufacturer_code(msg->data.set_manufacturer_code.request.code);
}

void sli_zigbee_stack_set_pending_network_update_channel_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_pending_network_update_channel(msg->data.set_pending_network_update_channel.request.channel);
}

void sli_zigbee_stack_set_zdo_configuration_mode_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_zdo_configuration_mode(msg->data.set_zdo_configuration_mode.request.enabled);
}

// public entrypoints

sl_status_t sl_zigbee_energy_scan_request(sl_802154_short_addr_t target,
                                          uint32_t scanChannels,
                                          uint8_t scanDuration,
                                          uint16_t scanCount)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.energy_scan_request.request.target = target;
  msg.data.energy_scan_request.request.scanChannels = scanChannels;
  msg.data.energy_scan_request.request.scanDuration = scanDuration;
  msg.data.energy_scan_request.request.scanCount = scanCount;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_energy_scan_request_process_ipc_command, &msg);

  return msg.data.energy_scan_request.response.result;
}

uint8_t sl_zigbee_get_last_stack_zig_dev_request_sequence(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_last_stack_zig_dev_request_sequence_process_ipc_command, &msg);

  return msg.data.get_last_stack_zig_dev_request_sequence.response.result;
}

sl_status_t sl_zigbee_ieee_address_request(sl_802154_short_addr_t target,
                                           bool reportKids,
                                           uint8_t childStartIndex,
                                           sl_zigbee_aps_option_t options)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.ieee_address_request.request.target = target;
  msg.data.ieee_address_request.request.reportKids = reportKids;
  msg.data.ieee_address_request.request.childStartIndex = childStartIndex;
  msg.data.ieee_address_request.request.options = options;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_ieee_address_request_process_ipc_command, &msg);

  return msg.data.ieee_address_request.response.result;
}

sl_status_t sl_zigbee_ieee_address_request_to_target(sl_802154_short_addr_t discoveryNodeId,
                                                     bool reportKids,
                                                     uint8_t childStartIndex,
                                                     sl_zigbee_aps_option_t options,
                                                     sl_802154_short_addr_t targetNodeIdOfRequest)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.ieee_address_request_to_target.request.discoveryNodeId = discoveryNodeId;
  msg.data.ieee_address_request_to_target.request.reportKids = reportKids;
  msg.data.ieee_address_request_to_target.request.childStartIndex = childStartIndex;
  msg.data.ieee_address_request_to_target.request.options = options;
  msg.data.ieee_address_request_to_target.request.targetNodeIdOfRequest = targetNodeIdOfRequest;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_ieee_address_request_to_target_process_ipc_command, &msg);

  return msg.data.ieee_address_request_to_target.response.result;
}

sl_status_t sl_zigbee_network_address_request(sl_802154_long_addr_t target,
                                              bool reportKids,
                                              uint8_t childStartIndex)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (target != NULL) {
    memmove(msg.data.network_address_request.request.target, target, sizeof(sl_802154_long_addr_t));
  }

  msg.data.network_address_request.request.reportKids = reportKids;
  msg.data.network_address_request.request.childStartIndex = childStartIndex;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_network_address_request_process_ipc_command, &msg);

  if (target != NULL) {
    memmove(target, msg.data.network_address_request.request.target, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.network_address_request.response.result;
}

sl_status_t sl_zigbee_send_device_announcement(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_device_announcement_process_ipc_command, &msg);

  return msg.data.send_device_announcement.response.result;
}

sl_status_t sl_zigbee_send_parent_announcement(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_parent_announcement_process_ipc_command, &msg);

  return msg.data.send_parent_announcement.response.result;
}

sl_status_t sl_zigbee_set_manufacturer_code(uint16_t code)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_manufacturer_code.request.code = code;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_manufacturer_code_process_ipc_command, &msg);

  return msg.data.set_manufacturer_code.response.result;
}

void sl_zigbee_set_pending_network_update_channel(uint8_t channel)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_pending_network_update_channel.request.channel = channel;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_pending_network_update_channel_process_ipc_command, &msg);
}

void sl_zigbee_set_zdo_configuration_mode(bool enabled)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_zdo_configuration_mode.request.enabled = enabled;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_zdo_configuration_mode_process_ipc_command, &msg);
}
