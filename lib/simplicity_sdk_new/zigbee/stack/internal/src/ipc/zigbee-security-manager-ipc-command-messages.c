/***************************************************************************//**
 * @file zigbee-security-manager-ipc-command-messages.c
 * @brief internal wrappers for 'zigbee-security-manager' ipc commands
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
// automatically generated from zigbee-security-manager.h.  Do not manually edit
#include "stack/include/zigbee-security-manager.h"
#include "stack/internal/inc/zigbee-security-manager-internal-def.h"
#include "stack/internal/src/ipc/zigbee-security-manager-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_get_key_from_install_code_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_key_from_install_code.response.result = sli_zigbee_stack_get_key_from_install_code(&msg->data.get_key_from_install_code.request.key);
}

void sli_zigbee_stack_sec_man_aes_128_crypt_block_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_aes_128_crypt_block.response.result = sli_zigbee_stack_sec_man_aes_128_crypt_block(msg->data.sec_man_aes_128_crypt_block.request.encrypt,
                                                                                                       msg->data.sec_man_aes_128_crypt_block.request.input,
                                                                                                       msg->data.sec_man_aes_128_crypt_block.request.output);
}

void sli_zigbee_stack_sec_man_aes_ccm_extended_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_aes_ccm_extended.response.result = sli_zigbee_stack_sec_man_aes_ccm_extended(msg->data.sec_man_aes_ccm_extended.request.nonce,
                                                                                                 msg->data.sec_man_aes_ccm_extended.request.encrypt,
                                                                                                 msg->data.sec_man_aes_ccm_extended.request.input,
                                                                                                 msg->data.sec_man_aes_ccm_extended.request.encryption_start_index,
                                                                                                 msg->data.sec_man_aes_ccm_extended.request.length,
                                                                                                 msg->data.sec_man_aes_ccm_extended.request.mic_length,
                                                                                                 msg->data.sec_man_aes_ccm_extended.request.output);
}

void sli_zigbee_stack_sec_man_check_key_context_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_check_key_context.response.result = sli_zigbee_stack_sec_man_check_key_context(&msg->data.sec_man_check_key_context.request.context);
}

void sli_zigbee_stack_sec_man_compare_key_to_value_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_compare_key_to_value.response.result = sli_zigbee_stack_sec_man_compare_key_to_value(&msg->data.sec_man_compare_key_to_value.request.context,
                                                                                                         &msg->data.sec_man_compare_key_to_value.request.test_key);
}

void sli_zigbee_stack_sec_man_delete_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_delete_key.response.result = sli_zigbee_stack_sec_man_delete_key(&msg->data.sec_man_delete_key.request.context);
}

void sli_zigbee_stack_sec_man_delete_key_table_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_delete_key_table_key.response.result = sli_zigbee_stack_sec_man_delete_key_table_key(&msg->data.sec_man_delete_key_table_key.request.context);
}

void sli_zigbee_stack_sec_man_delete_transient_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_delete_transient_key.response.result = sli_zigbee_stack_sec_man_delete_transient_key(&msg->data.sec_man_delete_transient_key.request.context);
}

void sli_zigbee_stack_sec_man_export_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_export_key.response.result = sli_zigbee_stack_sec_man_export_key(&msg->data.sec_man_export_key.request.context,
                                                                                     &msg->data.sec_man_export_key.request.plaintext_key);
}

void sli_zigbee_stack_sec_man_export_link_key_by_eui_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_export_link_key_by_eui.response.result = sli_zigbee_stack_sec_man_export_link_key_by_eui(msg->data.sec_man_export_link_key_by_eui.request.eui,
                                                                                                             &msg->data.sec_man_export_link_key_by_eui.request.context,
                                                                                                             &msg->data.sec_man_export_link_key_by_eui.request.plaintext_key,
                                                                                                             &msg->data.sec_man_export_link_key_by_eui.request.key_data);
}

void sli_zigbee_stack_sec_man_export_link_key_by_index_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_export_link_key_by_index.response.result = sli_zigbee_stack_sec_man_export_link_key_by_index(msg->data.sec_man_export_link_key_by_index.request.index,
                                                                                                                 &msg->data.sec_man_export_link_key_by_index.request.context,
                                                                                                                 &msg->data.sec_man_export_link_key_by_index.request.plaintext_key,
                                                                                                                 &msg->data.sec_man_export_link_key_by_index.request.key_data);
}

void sli_zigbee_stack_sec_man_export_symmetric_passphrase_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_export_symmetric_passphrase.response.result = sli_zigbee_stack_sec_man_export_symmetric_passphrase(msg->data.sec_man_export_symmetric_passphrase.request.address,
                                                                                                                       &msg->data.sec_man_export_symmetric_passphrase.request.key_data);
}

void sli_zigbee_stack_sec_man_export_transient_key_by_eui_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_export_transient_key_by_eui.response.result = sli_zigbee_stack_sec_man_export_transient_key_by_eui(msg->data.sec_man_export_transient_key_by_eui.request.eui64,
                                                                                                                       &msg->data.sec_man_export_transient_key_by_eui.request.context,
                                                                                                                       &msg->data.sec_man_export_transient_key_by_eui.request.plaintext_key,
                                                                                                                       &msg->data.sec_man_export_transient_key_by_eui.request.key_data);
}

void sli_zigbee_stack_sec_man_export_transient_key_by_index_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_export_transient_key_by_index.response.result = sli_zigbee_stack_sec_man_export_transient_key_by_index(msg->data.sec_man_export_transient_key_by_index.request.index,
                                                                                                                           &msg->data.sec_man_export_transient_key_by_index.request.context,
                                                                                                                           &msg->data.sec_man_export_transient_key_by_index.request.plaintext_key,
                                                                                                                           &msg->data.sec_man_export_transient_key_by_index.request.key_data);
}

void sli_zigbee_stack_sec_man_find_symmetric_passphrase_key_table_index_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_find_symmetric_passphrase_key_table_index.response.result = sli_zigbee_stack_sec_man_find_symmetric_passphrase_key_table_index(msg->data.sec_man_find_symmetric_passphrase_key_table_index.request.address);
}

void sli_zigbee_stack_sec_man_get_aps_key_info_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_get_aps_key_info.response.result = sli_zigbee_stack_sec_man_get_aps_key_info(&msg->data.sec_man_get_aps_key_info.request.context,
                                                                                                 &msg->data.sec_man_get_aps_key_info.request.key_data);
}

void sli_zigbee_stack_sec_man_get_network_key_info_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_get_network_key_info.response.result = sli_zigbee_stack_sec_man_get_network_key_info(&msg->data.sec_man_get_network_key_info.request.network_key_info);
}

void sli_zigbee_stack_sec_man_have_link_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_have_link_key.response.result = sli_zigbee_stack_sec_man_have_link_key(msg->data.sec_man_have_link_key.request.eui);
}

void sli_zigbee_stack_sec_man_hmac_aes_mmo_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_sec_man_hmac_aes_mmo(msg->data.sec_man_hmac_aes_mmo.request.input,
                                        msg->data.sec_man_hmac_aes_mmo.request.data_length,
                                        msg->data.sec_man_hmac_aes_mmo.request.output);
}

void sli_zigbee_stack_sec_man_import_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_import_key.response.result = sli_zigbee_stack_sec_man_import_key(&msg->data.sec_man_import_key.request.context,
                                                                                     &msg->data.sec_man_import_key.request.plaintext_key);
}

void sli_zigbee_stack_sec_man_import_link_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_import_link_key.response.result = sli_zigbee_stack_sec_man_import_link_key(msg->data.sec_man_import_link_key.request.index,
                                                                                               msg->data.sec_man_import_link_key.request.address,
                                                                                               &msg->data.sec_man_import_link_key.request.plaintext_key);
}

void sli_zigbee_stack_sec_man_import_symmetric_passphrase_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_import_symmetric_passphrase.response.result = sli_zigbee_stack_sec_man_import_symmetric_passphrase(msg->data.sec_man_import_symmetric_passphrase.request.index,
                                                                                                                       msg->data.sec_man_import_symmetric_passphrase.request.address,
                                                                                                                       &msg->data.sec_man_import_symmetric_passphrase.request.key_data);
}

void sli_zigbee_stack_sec_man_import_transient_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_import_transient_key.response.result = sli_zigbee_stack_sec_man_import_transient_key(msg->data.sec_man_import_transient_key.request.eui64,
                                                                                                         &msg->data.sec_man_import_transient_key.request.plaintext_key);
}

void sli_zigbee_stack_sec_man_init_context_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_sec_man_init_context(&msg->data.sec_man_init_context.request.context);
}

void sli_zigbee_stack_sec_man_link_key_slot_available_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_link_key_slot_available.response.result = sli_zigbee_stack_sec_man_link_key_slot_available(msg->data.sec_man_link_key_slot_available.request.eui64);
}

void sli_zigbee_stack_sec_man_load_key_context_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_load_key_context.response.result = sli_zigbee_stack_sec_man_load_key_context(&msg->data.sec_man_load_key_context.request.context);
}

void sli_zigbee_stack_sec_man_set_context_aes_ecb_alg_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_sec_man_set_context_aes_ecb_alg(&msg->data.sec_man_set_context_aes_ecb_alg.request.context);
}

void sli_zigbee_stack_sec_man_set_context_extended_ccm_alg_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_sec_man_set_context_extended_ccm_alg(&msg->data.sec_man_set_context_extended_ccm_alg.request.context);
}

void sli_zigbee_stack_sec_man_update_symmetric_passphrase_eui_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_update_symmetric_passphrase_eui.response.result = sli_zigbee_stack_sec_man_update_symmetric_passphrase_eui(msg->data.sec_man_update_symmetric_passphrase_eui.request.old_eui64,
                                                                                                                               msg->data.sec_man_update_symmetric_passphrase_eui.request.new_eui64);
}

void slxi_zigbee_stack_get_key_table_additional_info_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_key_table_additional_info.response.result = slxi_zigbee_stack_get_key_table_additional_info(msg->data.get_key_table_additional_info.request.index,
                                                                                                            &msg->data.get_key_table_additional_info.request.tok);
}

void slxi_zigbee_stack_get_trust_center_additional_info_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_trust_center_additional_info.response.result = slxi_zigbee_stack_get_trust_center_additional_info(&msg->data.get_trust_center_additional_info.request.tok);
}

void slxi_zigbee_stack_set_key_table_additional_info_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_key_table_additional_info.response.result = slxi_zigbee_stack_set_key_table_additional_info(msg->data.set_key_table_additional_info.request.index,
                                                                                                            msg->data.set_key_table_additional_info.request.tok);
}

void slxi_zigbee_stack_set_trust_center_additional_info_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_trust_center_additional_info.response.result = slxi_zigbee_stack_set_trust_center_additional_info(msg->data.set_trust_center_additional_info.request.tok);
}

// public entrypoints

sl_status_t sl_zigbee_get_key_from_install_code(sl_zigbee_key_data_t *key)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (key != NULL) {
    msg.data.get_key_from_install_code.request.key = *key;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_key_from_install_code_process_ipc_command, &msg);

  if (key != NULL) {
    *key = msg.data.get_key_from_install_code.request.key;
  }

  return msg.data.get_key_from_install_code.response.result;
}

sl_status_t sl_zigbee_sec_man_aes_128_crypt_block(bool encrypt,
                                                  const uint8_t *input,
                                                  uint8_t *output)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.sec_man_aes_128_crypt_block.request.encrypt = encrypt;

  if (input != NULL) {
    memmove(msg.data.sec_man_aes_128_crypt_block.request.input, input, sizeof(uint8_t) * 16);
  }

  if (output != NULL) {
    memmove(msg.data.sec_man_aes_128_crypt_block.request.output, output, sizeof(uint8_t) * 16);
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_aes_128_crypt_block_process_ipc_command, &msg);

  if (output != NULL) {
    memmove(output, msg.data.sec_man_aes_128_crypt_block.request.output, sizeof(uint8_t) * 16);
  }

  return msg.data.sec_man_aes_128_crypt_block.response.result;
}

sl_status_t sl_zigbee_sec_man_aes_ccm_extended(uint8_t *nonce,
                                               bool encrypt,
                                               const uint8_t *input,
                                               uint8_t encryption_start_index,
                                               uint8_t length,
                                               uint8_t mic_length,
                                               uint8_t *output)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (nonce != NULL) {
    memmove(msg.data.sec_man_aes_ccm_extended.request.nonce, nonce, sizeof(uint8_t) * NONCE_LENGTH);
  }

  msg.data.sec_man_aes_ccm_extended.request.encrypt = encrypt;

  if ((length + mic_length) > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector input length exceeds expected maximum
  }

  memmove(msg.data.sec_man_aes_ccm_extended.request.input, input, sizeof(uint8_t) * (length + mic_length));
  msg.data.sec_man_aes_ccm_extended.request.encryption_start_index = encryption_start_index;
  msg.data.sec_man_aes_ccm_extended.request.length = length;
  msg.data.sec_man_aes_ccm_extended.request.mic_length = mic_length;

  if ((length + mic_length) > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector output length exceeds expected maximum
  }

  memmove(msg.data.sec_man_aes_ccm_extended.request.output, output, sizeof(uint8_t) * (length + mic_length));
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_aes_ccm_extended_process_ipc_command, &msg);

  if (nonce != NULL) {
    memmove(nonce, msg.data.sec_man_aes_ccm_extended.request.nonce, sizeof(uint8_t) * NONCE_LENGTH);
  }

  if ((length + mic_length) > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector output length exceeds expected maximum
  }

  memmove(output, msg.data.sec_man_aes_ccm_extended.request.output, sizeof(uint8_t) * (length + mic_length));
  return msg.data.sec_man_aes_ccm_extended.response.result;
}

sl_status_t sl_zigbee_sec_man_check_key_context(sl_zigbee_sec_man_context_t *context)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (context != NULL) {
    msg.data.sec_man_check_key_context.request.context = *context;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_check_key_context_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.sec_man_check_key_context.request.context;
  }

  return msg.data.sec_man_check_key_context.response.result;
}

bool sl_zigbee_sec_man_compare_key_to_value(sl_zigbee_sec_man_context_t *context,
                                            const sl_zigbee_sec_man_key_t *test_key)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (context != NULL) {
    msg.data.sec_man_compare_key_to_value.request.context = *context;
  }

  if (test_key != NULL) {
    msg.data.sec_man_compare_key_to_value.request.test_key = *test_key;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_compare_key_to_value_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.sec_man_compare_key_to_value.request.context;
  }

  return msg.data.sec_man_compare_key_to_value.response.result;
}

sl_status_t sl_zigbee_sec_man_delete_key(sl_zigbee_sec_man_context_t *context)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (context != NULL) {
    msg.data.sec_man_delete_key.request.context = *context;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_delete_key_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.sec_man_delete_key.request.context;
  }

  return msg.data.sec_man_delete_key.response.result;
}

sl_status_t sl_zigbee_sec_man_delete_key_table_key(sl_zigbee_sec_man_context_t *context)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (context != NULL) {
    msg.data.sec_man_delete_key_table_key.request.context = *context;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_delete_key_table_key_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.sec_man_delete_key_table_key.request.context;
  }

  return msg.data.sec_man_delete_key_table_key.response.result;
}

sl_status_t sl_zigbee_sec_man_delete_transient_key(sl_zigbee_sec_man_context_t *context)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (context != NULL) {
    msg.data.sec_man_delete_transient_key.request.context = *context;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_delete_transient_key_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.sec_man_delete_transient_key.request.context;
  }

  return msg.data.sec_man_delete_transient_key.response.result;
}

sl_status_t sl_zigbee_sec_man_export_key(sl_zigbee_sec_man_context_t *context,
                                         sl_zigbee_sec_man_key_t *plaintext_key)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (context != NULL) {
    msg.data.sec_man_export_key.request.context = *context;
  }

  if (plaintext_key != NULL) {
    msg.data.sec_man_export_key.request.plaintext_key = *plaintext_key;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_export_key_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.sec_man_export_key.request.context;
  }

  if (plaintext_key != NULL) {
    *plaintext_key = msg.data.sec_man_export_key.request.plaintext_key;
  }

  return msg.data.sec_man_export_key.response.result;
}

sl_status_t sl_zigbee_sec_man_export_link_key_by_eui(sl_802154_long_addr_t eui,
                                                     sl_zigbee_sec_man_context_t *context,
                                                     sl_zigbee_sec_man_key_t *plaintext_key,
                                                     sl_zigbee_sec_man_aps_key_metadata_t *key_data)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (eui != NULL) {
    memmove(msg.data.sec_man_export_link_key_by_eui.request.eui, eui, sizeof(sl_802154_long_addr_t));
  }

  if (context != NULL) {
    msg.data.sec_man_export_link_key_by_eui.request.context = *context;
  }

  if (plaintext_key != NULL) {
    msg.data.sec_man_export_link_key_by_eui.request.plaintext_key = *plaintext_key;
  }

  if (key_data != NULL) {
    msg.data.sec_man_export_link_key_by_eui.request.key_data = *key_data;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_export_link_key_by_eui_process_ipc_command, &msg);

  if (eui != NULL) {
    memmove(eui, msg.data.sec_man_export_link_key_by_eui.request.eui, sizeof(sl_802154_long_addr_t));
  }

  if (context != NULL) {
    *context = msg.data.sec_man_export_link_key_by_eui.request.context;
  }

  if (plaintext_key != NULL) {
    *plaintext_key = msg.data.sec_man_export_link_key_by_eui.request.plaintext_key;
  }

  if (key_data != NULL) {
    *key_data = msg.data.sec_man_export_link_key_by_eui.request.key_data;
  }

  return msg.data.sec_man_export_link_key_by_eui.response.result;
}

sl_status_t sl_zigbee_sec_man_export_link_key_by_index(uint8_t index,
                                                       sl_zigbee_sec_man_context_t *context,
                                                       sl_zigbee_sec_man_key_t *plaintext_key,
                                                       sl_zigbee_sec_man_aps_key_metadata_t *key_data)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.sec_man_export_link_key_by_index.request.index = index;

  if (context != NULL) {
    msg.data.sec_man_export_link_key_by_index.request.context = *context;
  }

  if (plaintext_key != NULL) {
    msg.data.sec_man_export_link_key_by_index.request.plaintext_key = *plaintext_key;
  }

  if (key_data != NULL) {
    msg.data.sec_man_export_link_key_by_index.request.key_data = *key_data;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_export_link_key_by_index_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.sec_man_export_link_key_by_index.request.context;
  }

  if (plaintext_key != NULL) {
    *plaintext_key = msg.data.sec_man_export_link_key_by_index.request.plaintext_key;
  }

  if (key_data != NULL) {
    *key_data = msg.data.sec_man_export_link_key_by_index.request.key_data;
  }

  return msg.data.sec_man_export_link_key_by_index.response.result;
}

sl_status_t sl_zigbee_sec_man_export_symmetric_passphrase(sl_802154_long_addr_t address,
                                                          sl_zigbee_key_data_t *key_data)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (address != NULL) {
    memmove(msg.data.sec_man_export_symmetric_passphrase.request.address, address, sizeof(sl_802154_long_addr_t));
  }

  if (key_data != NULL) {
    msg.data.sec_man_export_symmetric_passphrase.request.key_data = *key_data;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_export_symmetric_passphrase_process_ipc_command, &msg);

  if (address != NULL) {
    memmove(address, msg.data.sec_man_export_symmetric_passphrase.request.address, sizeof(sl_802154_long_addr_t));
  }

  if (key_data != NULL) {
    *key_data = msg.data.sec_man_export_symmetric_passphrase.request.key_data;
  }

  return msg.data.sec_man_export_symmetric_passphrase.response.result;
}

sl_status_t sl_zigbee_sec_man_export_transient_key_by_eui(sl_802154_long_addr_t eui64,
                                                          sl_zigbee_sec_man_context_t *context,
                                                          sl_zigbee_sec_man_key_t *plaintext_key,
                                                          sl_zigbee_sec_man_aps_key_metadata_t *key_data)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (eui64 != NULL) {
    memmove(msg.data.sec_man_export_transient_key_by_eui.request.eui64, eui64, sizeof(sl_802154_long_addr_t));
  }

  if (context != NULL) {
    msg.data.sec_man_export_transient_key_by_eui.request.context = *context;
  }

  if (plaintext_key != NULL) {
    msg.data.sec_man_export_transient_key_by_eui.request.plaintext_key = *plaintext_key;
  }

  if (key_data != NULL) {
    msg.data.sec_man_export_transient_key_by_eui.request.key_data = *key_data;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_export_transient_key_by_eui_process_ipc_command, &msg);

  if (eui64 != NULL) {
    memmove(eui64, msg.data.sec_man_export_transient_key_by_eui.request.eui64, sizeof(sl_802154_long_addr_t));
  }

  if (context != NULL) {
    *context = msg.data.sec_man_export_transient_key_by_eui.request.context;
  }

  if (plaintext_key != NULL) {
    *plaintext_key = msg.data.sec_man_export_transient_key_by_eui.request.plaintext_key;
  }

  if (key_data != NULL) {
    *key_data = msg.data.sec_man_export_transient_key_by_eui.request.key_data;
  }

  return msg.data.sec_man_export_transient_key_by_eui.response.result;
}

sl_status_t sl_zigbee_sec_man_export_transient_key_by_index(uint8_t index,
                                                            sl_zigbee_sec_man_context_t *context,
                                                            sl_zigbee_sec_man_key_t *plaintext_key,
                                                            sl_zigbee_sec_man_aps_key_metadata_t *key_data)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.sec_man_export_transient_key_by_index.request.index = index;

  if (context != NULL) {
    msg.data.sec_man_export_transient_key_by_index.request.context = *context;
  }

  if (plaintext_key != NULL) {
    msg.data.sec_man_export_transient_key_by_index.request.plaintext_key = *plaintext_key;
  }

  if (key_data != NULL) {
    msg.data.sec_man_export_transient_key_by_index.request.key_data = *key_data;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_export_transient_key_by_index_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.sec_man_export_transient_key_by_index.request.context;
  }

  if (plaintext_key != NULL) {
    *plaintext_key = msg.data.sec_man_export_transient_key_by_index.request.plaintext_key;
  }

  if (key_data != NULL) {
    *key_data = msg.data.sec_man_export_transient_key_by_index.request.key_data;
  }

  return msg.data.sec_man_export_transient_key_by_index.response.result;
}

uint8_t sl_zigbee_sec_man_find_symmetric_passphrase_key_table_index(sl_802154_long_addr_t address)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (address != NULL) {
    memmove(msg.data.sec_man_find_symmetric_passphrase_key_table_index.request.address, address, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_find_symmetric_passphrase_key_table_index_process_ipc_command, &msg);

  if (address != NULL) {
    memmove(address, msg.data.sec_man_find_symmetric_passphrase_key_table_index.request.address, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.sec_man_find_symmetric_passphrase_key_table_index.response.result;
}

sl_status_t sl_zigbee_sec_man_get_aps_key_info(sl_zigbee_sec_man_context_t *context,
                                               sl_zigbee_sec_man_aps_key_metadata_t *key_data)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (context != NULL) {
    msg.data.sec_man_get_aps_key_info.request.context = *context;
  }

  if (key_data != NULL) {
    msg.data.sec_man_get_aps_key_info.request.key_data = *key_data;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_get_aps_key_info_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.sec_man_get_aps_key_info.request.context;
  }

  if (key_data != NULL) {
    *key_data = msg.data.sec_man_get_aps_key_info.request.key_data;
  }

  return msg.data.sec_man_get_aps_key_info.response.result;
}

sl_status_t sl_zigbee_sec_man_get_network_key_info(sl_zigbee_sec_man_network_key_info_t *network_key_info)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (network_key_info != NULL) {
    msg.data.sec_man_get_network_key_info.request.network_key_info = *network_key_info;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_get_network_key_info_process_ipc_command, &msg);

  if (network_key_info != NULL) {
    *network_key_info = msg.data.sec_man_get_network_key_info.request.network_key_info;
  }

  return msg.data.sec_man_get_network_key_info.response.result;
}

bool sl_zigbee_sec_man_have_link_key(sl_802154_long_addr_t eui)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (eui != NULL) {
    memmove(msg.data.sec_man_have_link_key.request.eui, eui, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_have_link_key_process_ipc_command, &msg);

  if (eui != NULL) {
    memmove(eui, msg.data.sec_man_have_link_key.request.eui, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.sec_man_have_link_key.response.result;
}

void sl_zigbee_sec_man_hmac_aes_mmo(const uint8_t *input,
                                    const uint8_t data_length,
                                    uint8_t *output)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (data_length > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector input length exceeds expected maximum
  }

  memmove(msg.data.sec_man_hmac_aes_mmo.request.input, input, sizeof(uint8_t) * data_length);
  msg.data.sec_man_hmac_aes_mmo.request.data_length = data_length;

  if (16 > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector output length exceeds expected maximum
  }

  memmove(msg.data.sec_man_hmac_aes_mmo.request.output, output, sizeof(uint8_t) * 16);
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_hmac_aes_mmo_process_ipc_command, &msg);

  if (16 > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector output length exceeds expected maximum
  }

  memmove(output, msg.data.sec_man_hmac_aes_mmo.request.output, sizeof(uint8_t) * 16);
}

sl_status_t sl_zigbee_sec_man_import_key(sl_zigbee_sec_man_context_t *context,
                                         const sl_zigbee_sec_man_key_t *plaintext_key)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (context != NULL) {
    msg.data.sec_man_import_key.request.context = *context;
  }

  if (plaintext_key != NULL) {
    msg.data.sec_man_import_key.request.plaintext_key = *plaintext_key;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_import_key_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.sec_man_import_key.request.context;
  }

  return msg.data.sec_man_import_key.response.result;
}

sl_status_t sl_zigbee_sec_man_import_link_key(uint8_t index,
                                              sl_802154_long_addr_t address,
                                              sl_zigbee_sec_man_key_t *plaintext_key)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.sec_man_import_link_key.request.index = index;

  if (address != NULL) {
    memmove(msg.data.sec_man_import_link_key.request.address, address, sizeof(sl_802154_long_addr_t));
  }

  if (plaintext_key != NULL) {
    msg.data.sec_man_import_link_key.request.plaintext_key = *plaintext_key;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_import_link_key_process_ipc_command, &msg);

  if (address != NULL) {
    memmove(address, msg.data.sec_man_import_link_key.request.address, sizeof(sl_802154_long_addr_t));
  }

  if (plaintext_key != NULL) {
    *plaintext_key = msg.data.sec_man_import_link_key.request.plaintext_key;
  }

  return msg.data.sec_man_import_link_key.response.result;
}

sl_status_t sl_zigbee_sec_man_import_symmetric_passphrase(uint8_t index,
                                                          sl_802154_long_addr_t address,
                                                          const sl_zigbee_key_data_t *key_data)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.sec_man_import_symmetric_passphrase.request.index = index;

  if (address != NULL) {
    memmove(msg.data.sec_man_import_symmetric_passphrase.request.address, address, sizeof(sl_802154_long_addr_t));
  }

  if (key_data != NULL) {
    msg.data.sec_man_import_symmetric_passphrase.request.key_data = *key_data;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_import_symmetric_passphrase_process_ipc_command, &msg);

  if (address != NULL) {
    memmove(address, msg.data.sec_man_import_symmetric_passphrase.request.address, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.sec_man_import_symmetric_passphrase.response.result;
}

sl_status_t sl_zigbee_sec_man_import_transient_key(sl_802154_long_addr_t eui64,
                                                   sl_zigbee_sec_man_key_t *plaintext_key)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (eui64 != NULL) {
    memmove(msg.data.sec_man_import_transient_key.request.eui64, eui64, sizeof(sl_802154_long_addr_t));
  }

  if (plaintext_key != NULL) {
    msg.data.sec_man_import_transient_key.request.plaintext_key = *plaintext_key;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_import_transient_key_process_ipc_command, &msg);

  if (eui64 != NULL) {
    memmove(eui64, msg.data.sec_man_import_transient_key.request.eui64, sizeof(sl_802154_long_addr_t));
  }

  if (plaintext_key != NULL) {
    *plaintext_key = msg.data.sec_man_import_transient_key.request.plaintext_key;
  }

  return msg.data.sec_man_import_transient_key.response.result;
}

void sl_zigbee_sec_man_init_context(sl_zigbee_sec_man_context_t *context)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (context != NULL) {
    msg.data.sec_man_init_context.request.context = *context;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_init_context_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.sec_man_init_context.request.context;
  }
}

bool sl_zigbee_sec_man_link_key_slot_available(sl_802154_long_addr_t eui64)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (eui64 != NULL) {
    memmove(msg.data.sec_man_link_key_slot_available.request.eui64, eui64, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_link_key_slot_available_process_ipc_command, &msg);

  if (eui64 != NULL) {
    memmove(eui64, msg.data.sec_man_link_key_slot_available.request.eui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.sec_man_link_key_slot_available.response.result;
}

sl_status_t sl_zigbee_sec_man_load_key_context(sl_zigbee_sec_man_context_t *context)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (context != NULL) {
    msg.data.sec_man_load_key_context.request.context = *context;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_load_key_context_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.sec_man_load_key_context.request.context;
  }

  return msg.data.sec_man_load_key_context.response.result;
}

void sl_zigbee_sec_man_set_context_aes_ecb_alg(sl_zigbee_sec_man_context_t *context)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (context != NULL) {
    msg.data.sec_man_set_context_aes_ecb_alg.request.context = *context;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_set_context_aes_ecb_alg_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.sec_man_set_context_aes_ecb_alg.request.context;
  }
}

void sl_zigbee_sec_man_set_context_extended_ccm_alg(sl_zigbee_sec_man_context_t *context)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (context != NULL) {
    msg.data.sec_man_set_context_extended_ccm_alg.request.context = *context;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_set_context_extended_ccm_alg_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.sec_man_set_context_extended_ccm_alg.request.context;
  }
}

sl_status_t sl_zigbee_sec_man_update_symmetric_passphrase_eui(sl_802154_long_addr_t old_eui64,
                                                              sl_802154_long_addr_t new_eui64)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (old_eui64 != NULL) {
    memmove(msg.data.sec_man_update_symmetric_passphrase_eui.request.old_eui64, old_eui64, sizeof(sl_802154_long_addr_t));
  }

  if (new_eui64 != NULL) {
    memmove(msg.data.sec_man_update_symmetric_passphrase_eui.request.new_eui64, new_eui64, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_update_symmetric_passphrase_eui_process_ipc_command, &msg);

  if (old_eui64 != NULL) {
    memmove(old_eui64, msg.data.sec_man_update_symmetric_passphrase_eui.request.old_eui64, sizeof(sl_802154_long_addr_t));
  }

  if (new_eui64 != NULL) {
    memmove(new_eui64, msg.data.sec_man_update_symmetric_passphrase_eui.request.new_eui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.sec_man_update_symmetric_passphrase_eui.response.result;
}

sl_status_t slx_zigbee_get_key_table_additional_info(uint8_t index,
                                                     uint32_t *tok)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_key_table_additional_info.request.index = index;

  if (tok != NULL) {
    msg.data.get_key_table_additional_info.request.tok = *tok;
  }

  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_get_key_table_additional_info_process_ipc_command, &msg);

  if (tok != NULL) {
    *tok = msg.data.get_key_table_additional_info.request.tok;
  }

  return msg.data.get_key_table_additional_info.response.result;
}

sl_status_t slx_zigbee_get_trust_center_additional_info(uint32_t *tok)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (tok != NULL) {
    msg.data.get_trust_center_additional_info.request.tok = *tok;
  }

  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_get_trust_center_additional_info_process_ipc_command, &msg);

  if (tok != NULL) {
    *tok = msg.data.get_trust_center_additional_info.request.tok;
  }

  return msg.data.get_trust_center_additional_info.response.result;
}

sl_status_t slx_zigbee_set_key_table_additional_info(uint8_t index,
                                                     uint32_t tok)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_key_table_additional_info.request.index = index;
  msg.data.set_key_table_additional_info.request.tok = tok;
  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_set_key_table_additional_info_process_ipc_command, &msg);

  return msg.data.set_key_table_additional_info.response.result;
}

sl_status_t slx_zigbee_set_trust_center_additional_info(uint32_t tok)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_trust_center_additional_info.request.tok = tok;
  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_set_trust_center_additional_info_process_ipc_command, &msg);

  return msg.data.set_trust_center_additional_info.response.result;
}
