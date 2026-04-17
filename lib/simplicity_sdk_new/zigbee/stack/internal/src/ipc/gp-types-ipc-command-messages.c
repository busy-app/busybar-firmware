/***************************************************************************//**
 * @file gp-types-ipc-command-messages.c
 * @brief internal wrappers for 'gp-types' ipc commands
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
// automatically generated from gp-types.h.  Do not manually edit
#include "stack/include/gp-types.h"
#include "stack/internal/inc/gp-types-internal-def.h"
#include "stack/internal/src/ipc/gp-types-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_d_gp_send_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.d_gp_send.response.result = sli_zigbee_stack_d_gp_send(msg->data.d_gp_send.request.action,
                                                                   msg->data.d_gp_send.request.useCca,
                                                                   &msg->data.d_gp_send.request.addr,
                                                                   msg->data.d_gp_send.request.gpdCommandId,
                                                                   msg->data.d_gp_send.request.gpdAsduLength,
                                                                   msg->data.d_gp_send.request.gpdAsdu,
                                                                   msg->data.d_gp_send.request.gpepHandle,
                                                                   msg->data.d_gp_send.request.gpTxQueueEntryLifetimeMs);
}

void sli_zigbee_stack_get_gp_max_tx_q_list_count_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_gp_max_tx_q_list_count.response.result = sli_zigbee_stack_get_gp_max_tx_q_list_count();
}

void sli_zigbee_stack_get_gp_tx_q_list_count_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_gp_tx_q_list_count.response.result = sli_zigbee_stack_get_gp_tx_q_list_count();
}

void sli_zigbee_stack_gp_add_gp_tx_queue_entry_with_payload_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_add_gp_tx_queue_entry_with_payload.response.result = sli_zigbee_stack_gp_add_gp_tx_queue_entry_with_payload(&msg->data.gp_add_gp_tx_queue_entry_with_payload.request.txQueue,
                                                                                                                           msg->data.gp_add_gp_tx_queue_entry_with_payload.request.data,
                                                                                                                           msg->data.gp_add_gp_tx_queue_entry_with_payload.request.dataLength);
}

void sli_zigbee_stack_gp_clear_tx_queue_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_zigbee_stack_gp_clear_tx_queue();
}

void sli_zigbee_stack_gp_get_tx_queue_entry_from_queue_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_get_tx_queue_entry_from_queue.response.result = sli_zigbee_stack_gp_get_tx_queue_entry_from_queue(&msg->data.gp_get_tx_queue_entry_from_queue.request.txQueue,
                                                                                                                 msg->data.gp_get_tx_queue_entry_from_queue.request.data,
                                                                                                                 &msg->data.gp_get_tx_queue_entry_from_queue.request.dataLength,
                                                                                                                 msg->data.gp_get_tx_queue_entry_from_queue.request.allocatedDataLength);
}

void sli_zigbee_stack_gp_get_tx_queue_entry_from_queue_index_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_get_tx_queue_entry_from_queue_index.response.result = sli_zigbee_stack_gp_get_tx_queue_entry_from_queue_index(msg->data.gp_get_tx_queue_entry_from_queue_index.request.index,
                                                                                                                             &msg->data.gp_get_tx_queue_entry_from_queue_index.request.txQueue,
                                                                                                                             msg->data.gp_get_tx_queue_entry_from_queue_index.request.payload,
                                                                                                                             &msg->data.gp_get_tx_queue_entry_from_queue_index.request.payload_len);
}

void sli_zigbee_stack_gp_get_tx_queue_head_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_get_tx_queue_head.response.result = sli_zigbee_stack_gp_get_tx_queue_head();
}

void sli_zigbee_stack_gp_remove_from_tx_queue_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gp_remove_from_tx_queue.response.result = sli_zigbee_stack_gp_remove_from_tx_queue(&msg->data.gp_remove_from_tx_queue.request.txQueue);
}

void sli_zigbee_stack_gp_set_max_tx_queue_entry_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_gp_set_max_tx_queue_entry(msg->data.gp_set_max_tx_queue_entry.request.maxEntries);
}

// public entrypoints

sl_status_t sl_zigbee_d_gp_send(bool action,
                                bool useCca,
                                sl_zigbee_gp_address_t *addr,
                                uint8_t gpdCommandId,
                                uint8_t gpdAsduLength,
                                const uint8_t *gpdAsdu,
                                uint8_t gpepHandle,
                                uint16_t gpTxQueueEntryLifetimeMs)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.d_gp_send.request.action = action;
  msg.data.d_gp_send.request.useCca = useCca;

  if (addr != NULL) {
    msg.data.d_gp_send.request.addr = *addr;
  }

  msg.data.d_gp_send.request.gpdCommandId = gpdCommandId;
  msg.data.d_gp_send.request.gpdAsduLength = gpdAsduLength;

  if (gpdAsduLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector gpdAsdu length exceeds expected maximum
  }

  memmove(msg.data.d_gp_send.request.gpdAsdu, gpdAsdu, sizeof(uint8_t) * gpdAsduLength);
  msg.data.d_gp_send.request.gpepHandle = gpepHandle;
  msg.data.d_gp_send.request.gpTxQueueEntryLifetimeMs = gpTxQueueEntryLifetimeMs;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_d_gp_send_process_ipc_command, &msg);

  if (addr != NULL) {
    *addr = msg.data.d_gp_send.request.addr;
  }

  return msg.data.d_gp_send.response.result;
}

uint16_t sl_zigbee_get_gp_max_tx_q_list_count(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_gp_max_tx_q_list_count_process_ipc_command, &msg);

  return msg.data.get_gp_max_tx_q_list_count.response.result;
}

uint16_t sl_zigbee_get_gp_tx_q_list_count(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_gp_tx_q_list_count_process_ipc_command, &msg);

  return msg.data.get_gp_tx_q_list_count.response.result;
}

sli_buffer_manager_buffer_t sl_zigbee_gp_add_gp_tx_queue_entry_with_payload(sl_zigbee_gp_tx_queue_entry_t *txQueue,
                                                                            uint8_t *data,
                                                                            uint16_t dataLength)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (txQueue != NULL) {
    msg.data.gp_add_gp_tx_queue_entry_with_payload.request.txQueue = *txQueue;
  }

  if (dataLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector data length exceeds expected maximum
  }

  memmove(msg.data.gp_add_gp_tx_queue_entry_with_payload.request.data, data, sizeof(uint8_t) * dataLength);
  msg.data.gp_add_gp_tx_queue_entry_with_payload.request.dataLength = dataLength;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_add_gp_tx_queue_entry_with_payload_process_ipc_command, &msg);

  if (txQueue != NULL) {
    *txQueue = msg.data.gp_add_gp_tx_queue_entry_with_payload.request.txQueue;
  }

  if (dataLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector data length exceeds expected maximum
  }

  memmove(data, msg.data.gp_add_gp_tx_queue_entry_with_payload.request.data, sizeof(uint8_t) * dataLength);
  return msg.data.gp_add_gp_tx_queue_entry_with_payload.response.result;
}

void sl_zigbee_gp_clear_tx_queue(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_clear_tx_queue_process_ipc_command, &msg);
}

sli_buffer_manager_buffer_t sl_zigbee_gp_get_tx_queue_entry_from_queue(sl_zigbee_gp_tx_queue_entry_t *txQueue,
                                                                       uint8_t *data,
                                                                       uint16_t *dataLength,
                                                                       uint16_t allocatedDataLength)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (txQueue != NULL) {
    msg.data.gp_get_tx_queue_entry_from_queue.request.txQueue = *txQueue;
  }

  if (allocatedDataLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector data length exceeds expected maximum
  }

  memmove(msg.data.gp_get_tx_queue_entry_from_queue.request.data, data, sizeof(uint8_t) * allocatedDataLength);

  if (dataLength != NULL) {
    msg.data.gp_get_tx_queue_entry_from_queue.request.dataLength = *dataLength;
  }

  msg.data.gp_get_tx_queue_entry_from_queue.request.allocatedDataLength = allocatedDataLength;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_get_tx_queue_entry_from_queue_process_ipc_command, &msg);

  if (txQueue != NULL) {
    *txQueue = msg.data.gp_get_tx_queue_entry_from_queue.request.txQueue;
  }

  if (allocatedDataLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector data length exceeds expected maximum
  }

  memmove(data, msg.data.gp_get_tx_queue_entry_from_queue.request.data, sizeof(uint8_t) * allocatedDataLength);

  if (dataLength != NULL) {
    *dataLength = msg.data.gp_get_tx_queue_entry_from_queue.request.dataLength;
  }

  return msg.data.gp_get_tx_queue_entry_from_queue.response.result;
}

sl_status_t sl_zigbee_gp_get_tx_queue_entry_from_queue_index(uint8_t index,
                                                             sl_zigbee_gp_tx_queue_entry_t *txQueue,
                                                             uint8_t *payload,
                                                             uint16_t *payload_len)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.gp_get_tx_queue_entry_from_queue_index.request.index = index;

  if (txQueue != NULL) {
    msg.data.gp_get_tx_queue_entry_from_queue_index.request.txQueue = *txQueue;
  }

  if ((*payload_len) > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector payload length exceeds expected maximum
  }

  memmove(msg.data.gp_get_tx_queue_entry_from_queue_index.request.payload, payload, sizeof(uint8_t) * (*payload_len));

  if (payload_len != NULL) {
    msg.data.gp_get_tx_queue_entry_from_queue_index.request.payload_len = *payload_len;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_get_tx_queue_entry_from_queue_index_process_ipc_command, &msg);

  if (txQueue != NULL) {
    *txQueue = msg.data.gp_get_tx_queue_entry_from_queue_index.request.txQueue;
  }

  if ((*payload_len) > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector payload length exceeds expected maximum
  }

  memmove(payload, msg.data.gp_get_tx_queue_entry_from_queue_index.request.payload, sizeof(uint8_t) * (*payload_len));

  if (payload_len != NULL) {
    *payload_len = msg.data.gp_get_tx_queue_entry_from_queue_index.request.payload_len;
  }

  return msg.data.gp_get_tx_queue_entry_from_queue_index.response.result;
}

sli_zigbee_message_buffer_queue_t * sl_zigbee_gp_get_tx_queue_head(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_get_tx_queue_head_process_ipc_command, &msg);

  return msg.data.gp_get_tx_queue_head.response.result;
}

bool sl_zigbee_gp_remove_from_tx_queue(sl_zigbee_gp_tx_queue_entry_t *txQueue)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (txQueue != NULL) {
    msg.data.gp_remove_from_tx_queue.request.txQueue = *txQueue;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_remove_from_tx_queue_process_ipc_command, &msg);

  if (txQueue != NULL) {
    *txQueue = msg.data.gp_remove_from_tx_queue.request.txQueue;
  }

  return msg.data.gp_remove_from_tx_queue.response.result;
}

void sl_zigbee_gp_set_max_tx_queue_entry(uint16_t maxEntries)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.gp_set_max_tx_queue_entry.request.maxEntries = maxEntries;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_gp_set_max_tx_queue_entry_process_ipc_command, &msg);
}
