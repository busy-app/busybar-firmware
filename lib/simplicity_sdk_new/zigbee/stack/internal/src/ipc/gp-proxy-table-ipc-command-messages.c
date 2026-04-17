/***************************************************************************//**
 * @file gp-proxy-table-ipc-command-messages.c
 * @brief internal wrappers for 'gp-proxy-table' ipc commands
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
// automatically generated from gp-proxy-table.h.  Do not manually edit
#include "stack/include/gp-proxy-table.h"
#include "stack/internal/inc/gp-proxy-table-internal-def.h"
#include "stack/internal/src/ipc/gp-proxy-table-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_gp_clear_proxy_table_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_zigbee_stack_gp_clear_proxy_table();
}

void sli_zigbee_stack_gp_proxy_table_get_entry_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_proxy_table_get_entry.response.result = sli_zigbee_stack_gp_proxy_table_get_entry(msg->data.gp_proxy_table_get_entry.request.proxyIndex,
                                                                                                 &msg->data.gp_proxy_table_get_entry.request.entry);
}

void sli_zigbee_stack_gp_proxy_table_lookup_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_proxy_table_lookup.response.result = sli_zigbee_stack_gp_proxy_table_lookup(&msg->data.gp_proxy_table_lookup.request.addr);
}

void sli_zigbee_stack_gp_proxy_table_process_gp_pairing_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_proxy_table_process_gp_pairing.response.result = sli_zigbee_stack_gp_proxy_table_process_gp_pairing(msg->data.gp_proxy_table_process_gp_pairing.request.options,
                                                                                                                   &msg->data.gp_proxy_table_process_gp_pairing.request.addr,
                                                                                                                   msg->data.gp_proxy_table_process_gp_pairing.request.commMode,
                                                                                                                   msg->data.gp_proxy_table_process_gp_pairing.request.sinkNwkAddress,
                                                                                                                   msg->data.gp_proxy_table_process_gp_pairing.request.sinkGroupId,
                                                                                                                   msg->data.gp_proxy_table_process_gp_pairing.request.assignedAlias,
                                                                                                                   msg->data.gp_proxy_table_process_gp_pairing.request.sinkIeeeAddress,
                                                                                                                   &msg->data.gp_proxy_table_process_gp_pairing.request.gpdKey,
                                                                                                                   msg->data.gp_proxy_table_process_gp_pairing.request.gpdSecurityFrameCounter,
                                                                                                                   msg->data.gp_proxy_table_process_gp_pairing.request.forwardingRadius);
}

void sli_zigbee_stack_gp_proxy_table_remove_entry_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_gp_proxy_table_remove_entry(msg->data.gp_proxy_table_remove_entry.request.index);
}

// public entrypoints

void sl_zigbee_gp_clear_proxy_table(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_clear_proxy_table_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_gp_proxy_table_get_entry(uint8_t proxyIndex,
                                               sl_zigbee_gp_proxy_table_entry_t *entry)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.gp_proxy_table_get_entry.request.proxyIndex = proxyIndex;

  if (entry != NULL) {
    msg.data.gp_proxy_table_get_entry.request.entry = *entry;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_proxy_table_get_entry_process_ipc_command, &msg);

  if (entry != NULL) {
    *entry = msg.data.gp_proxy_table_get_entry.request.entry;
  }

  return msg.data.gp_proxy_table_get_entry.response.result;
}

uint8_t sl_zigbee_gp_proxy_table_lookup(sl_zigbee_gp_address_t *addr)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (addr != NULL) {
    msg.data.gp_proxy_table_lookup.request.addr = *addr;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_proxy_table_lookup_process_ipc_command, &msg);

  if (addr != NULL) {
    *addr = msg.data.gp_proxy_table_lookup.request.addr;
  }

  return msg.data.gp_proxy_table_lookup.response.result;
}

bool sl_zigbee_gp_proxy_table_process_gp_pairing(uint32_t options,
                                                 sl_zigbee_gp_address_t *addr,
                                                 uint8_t commMode,
                                                 uint16_t sinkNwkAddress,
                                                 uint16_t sinkGroupId,
                                                 uint16_t assignedAlias,
                                                 uint8_t *sinkIeeeAddress,
                                                 sl_zigbee_key_data_t *gpdKey,
                                                 uint32_t gpdSecurityFrameCounter,
                                                 uint8_t forwardingRadius)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.gp_proxy_table_process_gp_pairing.request.options = options;

  if (addr != NULL) {
    msg.data.gp_proxy_table_process_gp_pairing.request.addr = *addr;
  }

  msg.data.gp_proxy_table_process_gp_pairing.request.commMode = commMode;
  msg.data.gp_proxy_table_process_gp_pairing.request.sinkNwkAddress = sinkNwkAddress;
  msg.data.gp_proxy_table_process_gp_pairing.request.sinkGroupId = sinkGroupId;
  msg.data.gp_proxy_table_process_gp_pairing.request.assignedAlias = assignedAlias;

  if (sinkIeeeAddress != NULL) {
    memmove(msg.data.gp_proxy_table_process_gp_pairing.request.sinkIeeeAddress, sinkIeeeAddress, sizeof(uint8_t) * EUI64_SIZE);
  }

  if (gpdKey != NULL) {
    msg.data.gp_proxy_table_process_gp_pairing.request.gpdKey = *gpdKey;
  }

  msg.data.gp_proxy_table_process_gp_pairing.request.gpdSecurityFrameCounter = gpdSecurityFrameCounter;
  msg.data.gp_proxy_table_process_gp_pairing.request.forwardingRadius = forwardingRadius;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_proxy_table_process_gp_pairing_process_ipc_command, &msg);

  if (addr != NULL) {
    *addr = msg.data.gp_proxy_table_process_gp_pairing.request.addr;
  }

  if (sinkIeeeAddress != NULL) {
    memmove(sinkIeeeAddress, msg.data.gp_proxy_table_process_gp_pairing.request.sinkIeeeAddress, sizeof(uint8_t) * EUI64_SIZE);
  }

  if (gpdKey != NULL) {
    *gpdKey = msg.data.gp_proxy_table_process_gp_pairing.request.gpdKey;
  }

  return msg.data.gp_proxy_table_process_gp_pairing.response.result;
}

void sl_zigbee_gp_proxy_table_remove_entry(uint8_t index)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.gp_proxy_table_remove_entry.request.index = index;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_proxy_table_remove_entry_process_ipc_command, &msg);
}
