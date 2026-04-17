/***************************************************************************//**
 * @file force_sleep_wakeup_stack_interface_ipc_command_messages.c
 * @brief internal wrappers for 'force_sleep_wakeup_stack_interface' ipc commands
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
// automatically generated from force_sleep_wakeup_stack_interface.h.  Do not manually edit
#include "stack/include/force_sleep_wakeup_stack_interface.h"
#include "stack/internal/inc/force_sleep_wakeup_stack_interface_internal_def.h"
#include "stack/internal/src/ipc/force_sleep_wakeup_stack_interface_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_802154_stack_cancel_polls_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_802154_stack_cancel_polls();
}

void sli_802154_stack_purge_incoming_queue_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_802154_stack_purge_incoming_queue();
}

void sli_802154_stack_purge_transmit_queue_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_802154_stack_purge_transmit_queue();
}

void sli_mac_stack_lower_mac_force_sleep_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_mac_stack_lower_mac_force_sleep(msg->data.lower_mac_force_sleep.request.sleep);
}

// public entrypoints

void sl_802154_cancel_polls(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_802154_stack_cancel_polls_process_ipc_command, &msg);
}

void sl_802154_purge_incoming_queue(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_802154_stack_purge_incoming_queue_process_ipc_command, &msg);
}

void sl_802154_purge_transmit_queue(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_802154_stack_purge_transmit_queue_process_ipc_command, &msg);
}

void sl_mac_lower_mac_force_sleep(bool sleep)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.lower_mac_force_sleep.request.sleep = sleep;
  sli_zigbee_send_ipc_cmd(sli_mac_stack_lower_mac_force_sleep_process_ipc_command, &msg);
}
