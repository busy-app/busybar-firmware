/***************************************************************************//**
 * @file sl_zigbee_security_manager_dlk_ecc_ipc_command_messages.c
 * @brief internal wrappers for 'sl_zigbee_security_manager_dlk_ecc' ipc commands
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
// automatically generated from sl_zigbee_security_manager_dlk_ecc.h.  Do not manually edit
#include "stack/include/sl_zigbee_security_manager_dlk_ecc.h"
#include "stack/internal/inc/sl_zigbee_security_manager_dlk_ecc_internal_def.h"
#include "stack/internal/src/ipc/sl_zigbee_security_manager_dlk_ecc_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_sec_man_ecc_derive_link_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_ecc_derive_link_key.response.result = sli_zigbee_stack_sec_man_ecc_derive_link_key(&msg->data.sec_man_ecc_derive_link_key.request.dlk_ecc_ctx);
}

void sli_zigbee_stack_sec_man_ecc_expand_shared_secret_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_ecc_expand_shared_secret.response.result = sli_zigbee_stack_sec_man_ecc_expand_shared_secret(&msg->data.sec_man_ecc_expand_shared_secret.request.dlk_ecc_ctx,
                                                                                                                 msg->data.sec_man_ecc_expand_shared_secret.request.our_eui,
                                                                                                                 msg->data.sec_man_ecc_expand_shared_secret.request.their_eui);
}

void sli_zigbee_stack_sec_man_ecc_extract_shared_secret_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_ecc_extract_shared_secret.response.result = sli_zigbee_stack_sec_man_ecc_extract_shared_secret(&msg->data.sec_man_ecc_extract_shared_secret.request.dlk_ecc_ctx,
                                                                                                                   msg->data.sec_man_ecc_extract_shared_secret.request.peer_public_key,
                                                                                                                   msg->data.sec_man_ecc_extract_shared_secret.request.peer_key_len);
}

void sli_zigbee_stack_sec_man_ecc_free_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_sec_man_ecc_free(&msg->data.sec_man_ecc_free.request.dlk_ecc_ctx);
}

void sli_zigbee_stack_sec_man_ecc_generate_keypair_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_ecc_generate_keypair.response.result = sli_zigbee_stack_sec_man_ecc_generate_keypair(&msg->data.sec_man_ecc_generate_keypair.request.dlk_ecc_ctx,
                                                                                                         msg->data.sec_man_ecc_generate_keypair.request.public_key_buff,
                                                                                                         &msg->data.sec_man_ecc_generate_keypair.request.key_len_out);
}

void sli_zigbee_stack_sec_man_ecc_init_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_ecc_init.response.result = sli_zigbee_stack_sec_man_ecc_init(&msg->data.sec_man_ecc_init.request.dlk_ecc_ctx,
                                                                                 &msg->data.sec_man_ecc_init.request.config,
                                                                                 msg->data.sec_man_ecc_init.request.psk);
}

void sli_zigbee_stack_sec_man_speke_expand_shared_secret_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_speke_expand_shared_secret.response.result = sli_zigbee_stack_sec_man_speke_expand_shared_secret(&msg->data.sec_man_speke_expand_shared_secret.request.dlk_ecc_ctx,
                                                                                                                     msg->data.sec_man_speke_expand_shared_secret.request.our_eui,
                                                                                                                     msg->data.sec_man_speke_expand_shared_secret.request.their_eui);
}

// public entrypoints

sl_status_t sl_zigbee_sec_man_ecc_derive_link_key(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (dlk_ecc_ctx != NULL) {
    msg.data.sec_man_ecc_derive_link_key.request.dlk_ecc_ctx = *dlk_ecc_ctx;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_ecc_derive_link_key_process_ipc_command, &msg);

  if (dlk_ecc_ctx != NULL) {
    *dlk_ecc_ctx = msg.data.sec_man_ecc_derive_link_key.request.dlk_ecc_ctx;
  }

  return msg.data.sec_man_ecc_derive_link_key.response.result;
}

sl_status_t sl_zigbee_sec_man_ecc_expand_shared_secret(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                       const uint8_t *our_eui,
                                                       const uint8_t *their_eui)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (dlk_ecc_ctx != NULL) {
    msg.data.sec_man_ecc_expand_shared_secret.request.dlk_ecc_ctx = *dlk_ecc_ctx;
  }

  if (our_eui != NULL) {
    memmove(msg.data.sec_man_ecc_expand_shared_secret.request.our_eui, our_eui, sizeof(uint8_t) * EUI64_SIZE);
  }

  if (their_eui != NULL) {
    memmove(msg.data.sec_man_ecc_expand_shared_secret.request.their_eui, their_eui, sizeof(uint8_t) * EUI64_SIZE);
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_ecc_expand_shared_secret_process_ipc_command, &msg);

  if (dlk_ecc_ctx != NULL) {
    *dlk_ecc_ctx = msg.data.sec_man_ecc_expand_shared_secret.request.dlk_ecc_ctx;
  }

  return msg.data.sec_man_ecc_expand_shared_secret.response.result;
}

sl_status_t sl_zigbee_sec_man_ecc_extract_shared_secret(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                        const uint8_t *peer_public_key,
                                                        size_t peer_key_len)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (dlk_ecc_ctx != NULL) {
    msg.data.sec_man_ecc_extract_shared_secret.request.dlk_ecc_ctx = *dlk_ecc_ctx;
  }

  if (peer_key_len > DLK_ECC_MAX_PUBLIC_KEY_SIZE) {
    assert(false); // "vector peer_public_key length exceeds expected maximum
  }

  memmove(msg.data.sec_man_ecc_extract_shared_secret.request.peer_public_key, peer_public_key, sizeof(uint8_t) * peer_key_len);
  msg.data.sec_man_ecc_extract_shared_secret.request.peer_key_len = peer_key_len;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_ecc_extract_shared_secret_process_ipc_command, &msg);

  if (dlk_ecc_ctx != NULL) {
    *dlk_ecc_ctx = msg.data.sec_man_ecc_extract_shared_secret.request.dlk_ecc_ctx;
  }

  return msg.data.sec_man_ecc_extract_shared_secret.response.result;
}

void sl_zigbee_sec_man_ecc_free(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (dlk_ecc_ctx != NULL) {
    msg.data.sec_man_ecc_free.request.dlk_ecc_ctx = *dlk_ecc_ctx;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_ecc_free_process_ipc_command, &msg);

  if (dlk_ecc_ctx != NULL) {
    *dlk_ecc_ctx = msg.data.sec_man_ecc_free.request.dlk_ecc_ctx;
  }
}

sl_status_t sl_zigbee_sec_man_ecc_generate_keypair(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                   uint8_t *public_key_buff,
                                                   size_t *key_len_out)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (dlk_ecc_ctx != NULL) {
    msg.data.sec_man_ecc_generate_keypair.request.dlk_ecc_ctx = *dlk_ecc_ctx;
  }

  if (public_key_buff != NULL) {
    memmove(msg.data.sec_man_ecc_generate_keypair.request.public_key_buff, public_key_buff, sizeof(uint8_t) * DLK_ECC_MAX_PUBLIC_KEY_SIZE);
  }

  if (key_len_out != NULL) {
    msg.data.sec_man_ecc_generate_keypair.request.key_len_out = *key_len_out;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_ecc_generate_keypair_process_ipc_command, &msg);

  if (dlk_ecc_ctx != NULL) {
    *dlk_ecc_ctx = msg.data.sec_man_ecc_generate_keypair.request.dlk_ecc_ctx;
  }

  if (public_key_buff != NULL) {
    memmove(public_key_buff, msg.data.sec_man_ecc_generate_keypair.request.public_key_buff, sizeof(uint8_t) * DLK_ECC_MAX_PUBLIC_KEY_SIZE);
  }

  if (key_len_out != NULL) {
    *key_len_out = msg.data.sec_man_ecc_generate_keypair.request.key_len_out;
  }

  return msg.data.sec_man_ecc_generate_keypair.response.result;
}

sl_status_t sl_zigbee_sec_man_ecc_init(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                       sl_zb_dlk_ecc_config_t *config,
                                       const uint8_t *psk)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (dlk_ecc_ctx != NULL) {
    msg.data.sec_man_ecc_init.request.dlk_ecc_ctx = *dlk_ecc_ctx;
  }

  if (config != NULL) {
    msg.data.sec_man_ecc_init.request.config = *config;
  }

  if (psk != NULL) {
    memmove(msg.data.sec_man_ecc_init.request.psk, psk, sizeof(uint8_t) * 16);
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_ecc_init_process_ipc_command, &msg);

  if (dlk_ecc_ctx != NULL) {
    *dlk_ecc_ctx = msg.data.sec_man_ecc_init.request.dlk_ecc_ctx;
  }

  if (config != NULL) {
    *config = msg.data.sec_man_ecc_init.request.config;
  }

  return msg.data.sec_man_ecc_init.response.result;
}

sl_status_t sl_zigbee_sec_man_speke_expand_shared_secret(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                         const uint8_t *our_eui,
                                                         const uint8_t *their_eui)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (dlk_ecc_ctx != NULL) {
    msg.data.sec_man_speke_expand_shared_secret.request.dlk_ecc_ctx = *dlk_ecc_ctx;
  }

  if (our_eui != NULL) {
    memmove(msg.data.sec_man_speke_expand_shared_secret.request.our_eui, our_eui, sizeof(uint8_t) * EUI64_SIZE);
  }

  if (their_eui != NULL) {
    memmove(msg.data.sec_man_speke_expand_shared_secret.request.their_eui, their_eui, sizeof(uint8_t) * EUI64_SIZE);
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_speke_expand_shared_secret_process_ipc_command, &msg);

  if (dlk_ecc_ctx != NULL) {
    *dlk_ecc_ctx = msg.data.sec_man_speke_expand_shared_secret.request.dlk_ecc_ctx;
  }

  return msg.data.sec_man_speke_expand_shared_secret.response.result;
}
