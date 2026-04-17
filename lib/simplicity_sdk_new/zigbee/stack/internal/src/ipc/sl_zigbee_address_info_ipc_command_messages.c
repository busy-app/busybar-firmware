/***************************************************************************//**
 * @file sl_zigbee_address_info_ipc_command_messages.c
 * @brief internal wrappers for 'sl_zigbee_address_info' ipc commands
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
// automatically generated from sl_zigbee_address_info.h.  Do not manually edit
#include "stack/include/sl_zigbee_address_info.h"
#include "stack/internal/inc/sl_zigbee_address_info_internal_def.h"
#include "stack/internal/src/ipc/sl_zigbee_address_info_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_device_id_matches_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.device_id_matches.response.result = sli_zigbee_stack_device_id_matches(&msg->data.device_id_matches.request.set_a,
                                                                                   &msg->data.device_id_matches.request.set_b);
}

void sli_zigbee_stack_make_device_id_pair_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.make_device_id_pair.response.result = sli_zigbee_stack_make_device_id_pair(msg->data.make_device_id_pair.request.short_id,
                                                                                       msg->data.make_device_id_pair.request.long_id);
}

// public entrypoints

bool sl_zigbee_device_id_matches(const sl_zigbee_address_info *set_a,
                                 const sl_zigbee_address_info *set_b)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (set_a != NULL) {
    msg.data.device_id_matches.request.set_a = *set_a;
  }

  if (set_b != NULL) {
    msg.data.device_id_matches.request.set_b = *set_b;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_device_id_matches_process_ipc_command, &msg);

  return msg.data.device_id_matches.response.result;
}

sl_zigbee_address_info sl_zigbee_make_device_id_pair(const sl_802154_short_addr_t short_id,
                                                     const sl_802154_long_addr_t long_id)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.make_device_id_pair.request.short_id = short_id;

  if (long_id != NULL) {
    memmove(msg.data.make_device_id_pair.request.long_id, long_id, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_make_device_id_pair_process_ipc_command, &msg);

  return msg.data.make_device_id_pair.response.result;
}
