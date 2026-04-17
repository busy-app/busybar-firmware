/***************************************************************************//**
 * @file bootload_ipc_command_messages.c
 * @brief internal wrappers for 'bootload' ipc commands
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
// automatically generated from bootload.h.  Do not manually edit
#include "stack/include/bootload.h"
#include "stack/internal/inc/bootload_internal_def.h"
#include "stack/internal/src/ipc/bootload_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_send_bootload_message_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_bootload_message.response.result = sli_zigbee_stack_send_bootload_message(msg->data.send_bootload_message.request.broadcast,
                                                                                           msg->data.send_bootload_message.request.destEui64,
                                                                                           msg->data.send_bootload_message.request.messageLength,
                                                                                           msg->data.send_bootload_message.request.messageContents);
}

// public entrypoints

sl_status_t sl_zigbee_send_bootload_message(bool broadcast,
                                            sl_802154_long_addr_t destEui64,
                                            uint8_t messageLength,
                                            uint8_t *messageContents)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.send_bootload_message.request.broadcast = broadcast;

  if (destEui64 != NULL) {
    memmove(msg.data.send_bootload_message.request.destEui64, destEui64, sizeof(sl_802154_long_addr_t));
  }

  msg.data.send_bootload_message.request.messageLength = messageLength;

  if (messageLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector messageContents length exceeds expected maximum
  }

  memmove(msg.data.send_bootload_message.request.messageContents, messageContents, sizeof(uint8_t) * messageLength);
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_bootload_message_process_ipc_command, &msg);

  if (destEui64 != NULL) {
    memmove(destEui64, msg.data.send_bootload_message.request.destEui64, sizeof(sl_802154_long_addr_t));
  }

  if (messageLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector messageContents length exceeds expected maximum
  }

  memmove(messageContents, msg.data.send_bootload_message.request.messageContents, sizeof(uint8_t) * messageLength);
  return msg.data.send_bootload_message.response.result;
}
