/***************************************************************************//**
 * @file mac-layer-ipc-command-messages.c
 * @brief internal wrappers for 'mac-layer' ipc commands
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
// automatically generated from mac-layer.h.  Do not manually edit
#include "stack/include/mac-layer.h"
#include "stack/internal/inc/mac-layer-internal-def.h"
#include "stack/internal/src/ipc/mac-layer-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_force_tx_after_failed_cca_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.force_tx_after_failed_cca.response.result = sli_zigbee_stack_force_tx_after_failed_cca(msg->data.force_tx_after_failed_cca.request.csmaAttempts);
}

void sli_zigbee_stack_get_max_mac_retries_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_max_mac_retries.response.result = sli_zigbee_stack_get_max_mac_retries();
}

// public entrypoints

sl_status_t sl_zigbee_force_tx_after_failed_cca(uint8_t csmaAttempts)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.force_tx_after_failed_cca.request.csmaAttempts = csmaAttempts;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_force_tx_after_failed_cca_process_ipc_command, &msg);

  return msg.data.force_tx_after_failed_cca.response.result;
}

uint8_t sl_zigbee_get_max_mac_retries(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_max_mac_retries_process_ipc_command, &msg);

  return msg.data.get_max_mac_retries.response.result;
}
