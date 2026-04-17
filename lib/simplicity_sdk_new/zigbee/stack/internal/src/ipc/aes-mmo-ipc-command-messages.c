/***************************************************************************//**
 * @file aes-mmo-ipc-command-messages.c
 * @brief internal wrappers for 'aes-mmo' ipc commands
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
// automatically generated from aes-mmo.h.  Do not manually edit
#include "stack/include/aes-mmo.h"
#include "stack/internal/inc/aes-mmo-internal-def.h"
#include "stack/internal/src/ipc/aes-mmo-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_aes_hash_simple_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.aes_hash_simple.response.result = sli_zigbee_stack_aes_hash_simple(msg->data.aes_hash_simple.request.totalLength,
                                                                               msg->data.aes_hash_simple.request.data,
                                                                               msg->data.aes_hash_simple.request.result);
}

void sli_zigbee_stack_aes_mmo_hash_final_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.aes_mmo_hash_final.response.result = sli_zigbee_stack_aes_mmo_hash_final(&msg->data.aes_mmo_hash_final.request.context,
                                                                                     msg->data.aes_mmo_hash_final.request.length,
                                                                                     msg->data.aes_mmo_hash_final.request.finalData);
}

void sli_zigbee_stack_aes_mmo_hash_init_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_aes_mmo_hash_init(&msg->data.aes_mmo_hash_init.request.context);
}

void sli_zigbee_stack_aes_mmo_hash_update_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.aes_mmo_hash_update.response.result = sli_zigbee_stack_aes_mmo_hash_update(&msg->data.aes_mmo_hash_update.request.context,
                                                                                       msg->data.aes_mmo_hash_update.request.length,
                                                                                       msg->data.aes_mmo_hash_update.request.data);
}

// public entrypoints

sl_status_t sl_zigbee_aes_hash_simple(uint8_t totalLength,
                                      const uint8_t *data,
                                      uint8_t *result)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.aes_hash_simple.request.totalLength = totalLength;

  if (totalLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector data length exceeds expected maximum
  }

  memmove(msg.data.aes_hash_simple.request.data, data, sizeof(uint8_t) * totalLength);

  if (SL_ZIGBEE_AES_HASH_BLOCK_SIZE > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector result length exceeds expected maximum
  }

  memmove(msg.data.aes_hash_simple.request.result, result, sizeof(uint8_t) * SL_ZIGBEE_AES_HASH_BLOCK_SIZE);
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_aes_hash_simple_process_ipc_command, &msg);

  if (SL_ZIGBEE_AES_HASH_BLOCK_SIZE > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector result length exceeds expected maximum
  }

  memmove(result, msg.data.aes_hash_simple.request.result, sizeof(uint8_t) * SL_ZIGBEE_AES_HASH_BLOCK_SIZE);
  return msg.data.aes_hash_simple.response.result;
}

sl_status_t sl_zigbee_aes_mmo_hash_final(sl_zigbee_aes_mmo_hash_context_t *context,
                                         uint32_t length,
                                         const uint8_t *finalData)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (context != NULL) {
    msg.data.aes_mmo_hash_final.request.context = *context;
  }

  msg.data.aes_mmo_hash_final.request.length = length;

  if (length > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector finalData length exceeds expected maximum
  }

  memmove(msg.data.aes_mmo_hash_final.request.finalData, finalData, sizeof(uint8_t) * length);
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_aes_mmo_hash_final_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.aes_mmo_hash_final.request.context;
  }

  return msg.data.aes_mmo_hash_final.response.result;
}

void sl_zigbee_aes_mmo_hash_init(sl_zigbee_aes_mmo_hash_context_t *context)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (context != NULL) {
    msg.data.aes_mmo_hash_init.request.context = *context;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_aes_mmo_hash_init_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.aes_mmo_hash_init.request.context;
  }
}

sl_status_t sl_zigbee_aes_mmo_hash_update(sl_zigbee_aes_mmo_hash_context_t *context,
                                          uint32_t length,
                                          const uint8_t *data)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (context != NULL) {
    msg.data.aes_mmo_hash_update.request.context = *context;
  }

  msg.data.aes_mmo_hash_update.request.length = length;

  if (length > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector data length exceeds expected maximum
  }

  memmove(msg.data.aes_mmo_hash_update.request.data, data, sizeof(uint8_t) * length);
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_aes_mmo_hash_update_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.aes_mmo_hash_update.request.context;
  }

  return msg.data.aes_mmo_hash_update.response.result;
}
