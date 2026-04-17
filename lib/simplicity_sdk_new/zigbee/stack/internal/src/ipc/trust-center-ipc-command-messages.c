/***************************************************************************//**
 * @file trust-center-ipc-command-messages.c
 * @brief internal wrappers for 'trust-center' ipc commands
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
// automatically generated from trust-center.h.  Do not manually edit
#include "stack/include/trust-center.h"
#include "stack/internal/inc/trust-center-internal-def.h"
#include "stack/internal/src/ipc/trust-center-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_broadcast_network_key_switch_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.broadcast_network_key_switch.response.result = sli_zigbee_stack_broadcast_network_key_switch();
}

void sli_zigbee_stack_broadcast_next_network_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.broadcast_next_network_key.response.result = sli_zigbee_stack_broadcast_next_network_key(&msg->data.broadcast_next_network_key.request.key);
}

void sli_zigbee_stack_get_app_link_key_request_policy_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_app_link_key_request_policy.response.result = sli_zigbee_stack_get_app_link_key_request_policy();
}

void sli_zigbee_stack_get_transient_device_table_timeout_ms_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_transient_device_table_timeout_ms.response.result = sli_zigbee_stack_get_transient_device_table_timeout_ms();
}

void sli_zigbee_stack_get_trust_center_link_key_request_policy_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_trust_center_link_key_request_policy.response.result = sli_zigbee_stack_get_trust_center_link_key_request_policy();
}

void sli_zigbee_stack_send_remove_device_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_remove_device.response.result = sli_zigbee_stack_send_remove_device(msg->data.send_remove_device.request.destShort,
                                                                                     msg->data.send_remove_device.request.destLong,
                                                                                     msg->data.send_remove_device.request.deviceToRemoveLong);
}

void sli_zigbee_stack_send_trust_center_link_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_trust_center_link_key.response.result = sli_zigbee_stack_send_trust_center_link_key(msg->data.send_trust_center_link_key.request.destinationNodeId,
                                                                                                     msg->data.send_trust_center_link_key.request.destinationEui64);
}

void sli_zigbee_stack_send_unicast_network_key_update_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_unicast_network_key_update.response.result = sli_zigbee_stack_send_unicast_network_key_update(msg->data.send_unicast_network_key_update.request.targetShort,
                                                                                                               msg->data.send_unicast_network_key_update.request.targetLong,
                                                                                                               &msg->data.send_unicast_network_key_update.request.newKey);
}

void sli_zigbee_stack_set_app_link_key_request_policy_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_app_link_key_request_policy(msg->data.set_app_link_key_request_policy.request.policy);
}

void sli_zigbee_stack_set_transient_device_table_timeout_ms_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_transient_device_table_timeout_ms.response.result = sli_zigbee_stack_set_transient_device_table_timeout_ms(msg->data.set_transient_device_table_timeout_ms.request.timeout);
}

void sli_zigbee_stack_set_trust_center_link_key_request_policy_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_trust_center_link_key_request_policy(msg->data.set_trust_center_link_key_request_policy.request.policy);
}

void sli_zigbee_stack_unicast_current_network_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.unicast_current_network_key.response.result = sli_zigbee_stack_unicast_current_network_key(msg->data.unicast_current_network_key.request.targetShort,
                                                                                                       msg->data.unicast_current_network_key.request.targetLong,
                                                                                                       msg->data.unicast_current_network_key.request.parentShortId);
}

// public entrypoints

sl_status_t sl_zigbee_broadcast_network_key_switch(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_broadcast_network_key_switch_process_ipc_command, &msg);

  return msg.data.broadcast_network_key_switch.response.result;
}

sl_status_t sl_zigbee_broadcast_next_network_key(sl_zigbee_key_data_t *key)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (key != NULL) {
    msg.data.broadcast_next_network_key.request.key = *key;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_broadcast_next_network_key_process_ipc_command, &msg);

  if (key != NULL) {
    *key = msg.data.broadcast_next_network_key.request.key;
  }

  return msg.data.broadcast_next_network_key.response.result;
}

sl_zigbee_app_link_key_request_policy_t sl_zigbee_get_app_link_key_request_policy(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_app_link_key_request_policy_process_ipc_command, &msg);

  return msg.data.get_app_link_key_request_policy.response.result;
}

uint16_t sl_zigbee_get_transient_device_table_timeout_ms(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_transient_device_table_timeout_ms_process_ipc_command, &msg);

  return msg.data.get_transient_device_table_timeout_ms.response.result;
}

sl_zigbee_tc_link_key_request_policy_t sl_zigbee_get_trust_center_link_key_request_policy(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_trust_center_link_key_request_policy_process_ipc_command, &msg);

  return msg.data.get_trust_center_link_key_request_policy.response.result;
}

sl_status_t sl_zigbee_send_remove_device(sl_802154_short_addr_t destShort,
                                         sl_802154_long_addr_t destLong,
                                         sl_802154_long_addr_t deviceToRemoveLong)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.send_remove_device.request.destShort = destShort;

  if (destLong != NULL) {
    memmove(msg.data.send_remove_device.request.destLong, destLong, sizeof(sl_802154_long_addr_t));
  }

  if (deviceToRemoveLong != NULL) {
    memmove(msg.data.send_remove_device.request.deviceToRemoveLong, deviceToRemoveLong, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_remove_device_process_ipc_command, &msg);

  if (destLong != NULL) {
    memmove(destLong, msg.data.send_remove_device.request.destLong, sizeof(sl_802154_long_addr_t));
  }

  if (deviceToRemoveLong != NULL) {
    memmove(deviceToRemoveLong, msg.data.send_remove_device.request.deviceToRemoveLong, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.send_remove_device.response.result;
}

sl_status_t sl_zigbee_send_trust_center_link_key(sl_802154_short_addr_t destinationNodeId,
                                                 sl_802154_long_addr_t destinationEui64)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.send_trust_center_link_key.request.destinationNodeId = destinationNodeId;

  if (destinationEui64 != NULL) {
    memmove(msg.data.send_trust_center_link_key.request.destinationEui64, destinationEui64, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_trust_center_link_key_process_ipc_command, &msg);

  if (destinationEui64 != NULL) {
    memmove(destinationEui64, msg.data.send_trust_center_link_key.request.destinationEui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.send_trust_center_link_key.response.result;
}

sl_status_t sl_zigbee_send_unicast_network_key_update(sl_802154_short_addr_t targetShort,
                                                      sl_802154_long_addr_t targetLong,
                                                      const sl_zigbee_key_data_t *newKey)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.send_unicast_network_key_update.request.targetShort = targetShort;

  if (targetLong != NULL) {
    memmove(msg.data.send_unicast_network_key_update.request.targetLong, targetLong, sizeof(sl_802154_long_addr_t));
  }

  if (newKey != NULL) {
    msg.data.send_unicast_network_key_update.request.newKey = *newKey;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_unicast_network_key_update_process_ipc_command, &msg);

  if (targetLong != NULL) {
    memmove(targetLong, msg.data.send_unicast_network_key_update.request.targetLong, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.send_unicast_network_key_update.response.result;
}

void sl_zigbee_set_app_link_key_request_policy(sl_zigbee_app_link_key_request_policy_t policy)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_app_link_key_request_policy.request.policy = policy;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_app_link_key_request_policy_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_set_transient_device_table_timeout_ms(uint16_t timeout)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_transient_device_table_timeout_ms.request.timeout = timeout;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_transient_device_table_timeout_ms_process_ipc_command, &msg);

  return msg.data.set_transient_device_table_timeout_ms.response.result;
}

void sl_zigbee_set_trust_center_link_key_request_policy(sl_zigbee_tc_link_key_request_policy_t policy)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_trust_center_link_key_request_policy.request.policy = policy;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_trust_center_link_key_request_policy_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_unicast_current_network_key(sl_802154_short_addr_t targetShort,
                                                  sl_802154_long_addr_t targetLong,
                                                  sl_802154_short_addr_t parentShortId)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.unicast_current_network_key.request.targetShort = targetShort;

  if (targetLong != NULL) {
    memmove(msg.data.unicast_current_network_key.request.targetLong, targetLong, sizeof(sl_802154_long_addr_t));
  }

  msg.data.unicast_current_network_key.request.parentShortId = parentShortId;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_unicast_current_network_key_process_ipc_command, &msg);

  if (targetLong != NULL) {
    memmove(targetLong, msg.data.unicast_current_network_key.request.targetLong, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.unicast_current_network_key.response.result;
}
