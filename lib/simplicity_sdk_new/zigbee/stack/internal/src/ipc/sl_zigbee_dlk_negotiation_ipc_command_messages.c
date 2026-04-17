/***************************************************************************//**
 * @file sl_zigbee_dlk_negotiation_ipc_command_messages.c
 * @brief internal wrappers for 'sl_zigbee_dlk_negotiation' ipc commands
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
// automatically generated from sl_zigbee_dlk_negotiation.h.  Do not manually edit
#include "stack/include/sl_zigbee_dlk_negotiation.h"
#include "stack/internal/inc/sl_zigbee_dlk_negotiation_internal_def.h"
#include "stack/internal/src/ipc/sl_zigbee_dlk_negotiation_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_dlk_close_key_exchange_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_dlk_close_key_exchange(&msg->data.dlk_close_key_exchange.request.dlk_negotiation_ctx);
}

void sli_zigbee_stack_dlk_finish_key_exchange_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.dlk_finish_key_exchange.response.result = sli_zigbee_stack_dlk_finish_key_exchange(&msg->data.dlk_finish_key_exchange.request.dlk_negotiation_ctx,
                                                                                               &msg->data.dlk_finish_key_exchange.request.partner,
                                                                                               msg->data.dlk_finish_key_exchange.request.partner_key,
                                                                                               msg->data.dlk_finish_key_exchange.request.partner_key_length);
}

void sli_zigbee_stack_dlk_open_key_exchange_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.dlk_open_key_exchange.response.result = sli_zigbee_stack_dlk_open_key_exchange(&msg->data.dlk_open_key_exchange.request.dlk_negotiation_ctx,
                                                                                           &msg->data.dlk_open_key_exchange.request.partner,
                                                                                           msg->data.dlk_open_key_exchange.request.keyProtocol,
                                                                                           msg->data.dlk_open_key_exchange.request.secret,
                                                                                           msg->data.dlk_open_key_exchange.request.psk_bytes,
                                                                                           msg->data.dlk_open_key_exchange.request.dlk_start_complete_callback,
                                                                                           msg->data.dlk_open_key_exchange.request.dlk_finish_complete_callback);
}

void sli_zigbee_stack_dlk_start_key_exchange_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.dlk_start_key_exchange.response.result = sli_zigbee_stack_dlk_start_key_exchange(&msg->data.dlk_start_key_exchange.request.dlk_negotiation_ctx,
                                                                                             &msg->data.dlk_start_key_exchange.request.partner);
}

// public entrypoints

void sl_zigbee_dlk_close_key_exchange(sl_zigbee_dlk_negotiation_context_t *dlk_negotiation_ctx)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (dlk_negotiation_ctx != NULL) {
    msg.data.dlk_close_key_exchange.request.dlk_negotiation_ctx = *dlk_negotiation_ctx;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_dlk_close_key_exchange_process_ipc_command, &msg);

  if (dlk_negotiation_ctx != NULL) {
    *dlk_negotiation_ctx = msg.data.dlk_close_key_exchange.request.dlk_negotiation_ctx;
  }
}

sl_status_t sl_zigbee_dlk_finish_key_exchange(sl_zigbee_dlk_negotiation_context_t *dlk_negotiation_ctx,
                                              const sl_zigbee_address_info *partner,
                                              const uint8_t *partner_key,
                                              size_t partner_key_length)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (dlk_negotiation_ctx != NULL) {
    msg.data.dlk_finish_key_exchange.request.dlk_negotiation_ctx = *dlk_negotiation_ctx;
  }

  if (partner != NULL) {
    msg.data.dlk_finish_key_exchange.request.partner = *partner;
  }

  if (partner_key_length > 64) {
    assert(false); // "vector partner_key length exceeds expected maximum
  }

  memmove(msg.data.dlk_finish_key_exchange.request.partner_key, partner_key, sizeof(uint8_t) * partner_key_length);
  msg.data.dlk_finish_key_exchange.request.partner_key_length = partner_key_length;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_dlk_finish_key_exchange_process_ipc_command, &msg);

  if (dlk_negotiation_ctx != NULL) {
    *dlk_negotiation_ctx = msg.data.dlk_finish_key_exchange.request.dlk_negotiation_ctx;
  }

  return msg.data.dlk_finish_key_exchange.response.result;
}

sl_status_t sl_zigbee_dlk_open_key_exchange(sl_zigbee_dlk_negotiation_context_t *dlk_negotiation_ctx,
                                            const sl_zigbee_address_info *partner,
                                            sl_zigbee_dlk_negotiation_method keyProtocol,
                                            sl_zigbee_dlk_negotiation_shared_secret_source secret,
                                            const uint8_t *psk_bytes,
                                            sl_zigbee_dlk_start_complete_callback dlk_start_complete_callback,
                                            sl_zigbee_dlk_finish_complete_callback dlk_finish_complete_callback)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (dlk_negotiation_ctx != NULL) {
    msg.data.dlk_open_key_exchange.request.dlk_negotiation_ctx = *dlk_negotiation_ctx;
  }

  if (partner != NULL) {
    msg.data.dlk_open_key_exchange.request.partner = *partner;
  }

  msg.data.dlk_open_key_exchange.request.keyProtocol = keyProtocol;
  msg.data.dlk_open_key_exchange.request.secret = secret;

  if (psk_bytes != NULL) {
    memmove(msg.data.dlk_open_key_exchange.request.psk_bytes, psk_bytes, sizeof(uint8_t) * 16);
  }

  msg.data.dlk_open_key_exchange.request.dlk_start_complete_callback = dlk_start_complete_callback;
  msg.data.dlk_open_key_exchange.request.dlk_finish_complete_callback = dlk_finish_complete_callback;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_dlk_open_key_exchange_process_ipc_command, &msg);

  if (dlk_negotiation_ctx != NULL) {
    *dlk_negotiation_ctx = msg.data.dlk_open_key_exchange.request.dlk_negotiation_ctx;
  }

  return msg.data.dlk_open_key_exchange.response.result;
}

sl_status_t sl_zigbee_dlk_start_key_exchange(sl_zigbee_dlk_negotiation_context_t *dlk_negotiation_ctx,
                                             const sl_zigbee_address_info *partner)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (dlk_negotiation_ctx != NULL) {
    msg.data.dlk_start_key_exchange.request.dlk_negotiation_ctx = *dlk_negotiation_ctx;
  }

  if (partner != NULL) {
    msg.data.dlk_start_key_exchange.request.partner = *partner;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_dlk_start_key_exchange_process_ipc_command, &msg);

  if (dlk_negotiation_ctx != NULL) {
    *dlk_negotiation_ctx = msg.data.dlk_start_key_exchange.request.dlk_negotiation_ctx;
  }

  return msg.data.dlk_start_key_exchange.response.result;
}
