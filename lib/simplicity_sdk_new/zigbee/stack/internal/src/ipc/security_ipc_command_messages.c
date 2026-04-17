/***************************************************************************//**
 * @file security_ipc_command_messages.c
 * @brief internal wrappers for 'security' ipc commands
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
// automatically generated from security.h.  Do not manually edit
#include "stack/include/security.h"
#include "stack/internal/inc/security_internal_def.h"
#include "stack/internal/src/ipc/security_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_aps_crypt_message_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.aps_crypt_message.response.result = sli_zigbee_stack_aps_crypt_message(msg->data.aps_crypt_message.request.encrypt,
                                                                                   msg->data.aps_crypt_message.request.length,
                                                                                   msg->data.aps_crypt_message.request.message,
                                                                                   msg->data.aps_crypt_message.request.apsHeaderEndIndex,
                                                                                   msg->data.aps_crypt_message.request.remoteEui64);
}

void sli_zigbee_stack_clear_key_table_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.clear_key_table.response.result = sli_zigbee_stack_clear_key_table();
}

void sli_zigbee_stack_clear_transient_link_keys_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_zigbee_stack_clear_transient_link_keys();
}

void sli_zigbee_stack_erase_key_table_entry_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.erase_key_table_entry.response.result = sli_zigbee_stack_erase_key_table_entry(msg->data.erase_key_table_entry.request.index);
}

void sli_zigbee_stack_find_key_table_entry_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.find_key_table_entry.response.result = sli_zigbee_stack_find_key_table_entry(msg->data.find_key_table_entry.request.address,
                                                                                         msg->data.find_key_table_entry.request.linkKey);
}

void sli_zigbee_stack_generate_random_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.generate_random_key.response.result = sli_zigbee_stack_generate_random_key(&msg->data.generate_random_key.request.keyAddress);
}

void sli_zigbee_stack_get_aps_frame_counter_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_aps_frame_counter.response.result = sli_zigbee_stack_get_aps_frame_counter();
}

void sli_zigbee_stack_get_current_security_state_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_current_security_state.response.result = sli_zigbee_stack_get_current_security_state(&msg->data.get_current_security_state.request.state);
}

void sli_zigbee_stack_get_extended_security_bitmask_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_extended_security_bitmask.response.result = sli_zigbee_stack_get_extended_security_bitmask(&msg->data.get_extended_security_bitmask.request.mask);
}

void sli_zigbee_stack_get_incoming_tc_link_key_frame_counter_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_incoming_tc_link_key_frame_counter.response.result = sli_zigbee_stack_get_incoming_tc_link_key_frame_counter();
}

void sli_zigbee_stack_get_mfg_security_config_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_mfg_security_config.response.result = sli_zigbee_stack_get_mfg_security_config(&msg->data.get_mfg_security_config.request.settings);
}

void sli_zigbee_stack_get_rejoin_mode_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_rejoin_mode.response.result = sli_zigbee_stack_get_rejoin_mode();
}

void sli_zigbee_stack_get_security_frame_counter_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_security_frame_counter.response.result = sli_zigbee_stack_get_security_frame_counter();
}

void sli_zigbee_stack_get_transient_key_timeout_s_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_transient_key_timeout_s.response.result = sli_zigbee_stack_get_transient_key_timeout_s();
}

void sli_zigbee_stack_request_link_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.request_link_key.response.result = sli_zigbee_stack_request_link_key(msg->data.request_link_key.request.partner);
}

void sli_zigbee_stack_set_extended_security_bitmask_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_extended_security_bitmask.response.result = sli_zigbee_stack_set_extended_security_bitmask(msg->data.set_extended_security_bitmask.request.mask);
}

void sli_zigbee_stack_set_incoming_tc_link_key_frame_counter_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_incoming_tc_link_key_frame_counter(msg->data.set_incoming_tc_link_key_frame_counter.request.frameCounter);
}

void sli_zigbee_stack_set_initial_security_state_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_initial_security_state.response.result = sli_zigbee_stack_set_initial_security_state(&msg->data.set_initial_security_state.request.state);
}

void sli_zigbee_stack_set_mfg_security_config_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_mfg_security_config.response.result = sli_zigbee_stack_set_mfg_security_config(msg->data.set_mfg_security_config.request.magicNumber,
                                                                                               &msg->data.set_mfg_security_config.request.settings);
}

void sli_zigbee_stack_set_outgoing_aps_frame_counter_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_outgoing_aps_frame_counter.response.result = sli_zigbee_stack_set_outgoing_aps_frame_counter(msg->data.set_outgoing_aps_frame_counter.request.desiredValue);
}

void sli_zigbee_stack_set_outgoing_nwk_frame_counter_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_outgoing_nwk_frame_counter.response.result = sli_zigbee_stack_set_outgoing_nwk_frame_counter(msg->data.set_outgoing_nwk_frame_counter.request.desiredValue);
}

void sli_zigbee_stack_set_rejoin_mode_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_rejoin_mode(msg->data.set_rejoin_mode.request.mode);
}

void sli_zigbee_stack_set_transient_key_timeout_s_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_transient_key_timeout_s(msg->data.set_transient_key_timeout_s.request.seconds);
}

void sli_zigbee_stack_start_writing_stack_tokens_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.start_writing_stack_tokens.response.result = sli_zigbee_stack_start_writing_stack_tokens();
}

void sli_zigbee_stack_stop_writing_stack_tokens_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.stop_writing_stack_tokens.response.result = sli_zigbee_stack_stop_writing_stack_tokens();
}

void sli_zigbee_stack_terminate_app_link_key_request_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.terminate_app_link_key_request.response.result = sli_zigbee_stack_terminate_app_link_key_request();
}

void sli_zigbee_stack_update_app_link_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.update_app_link_key.response.result = sli_zigbee_stack_update_app_link_key(msg->data.update_app_link_key.request.partnerEui64);
}

void sli_zigbee_stack_update_tc_link_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.update_tc_link_key.response.result = sli_zigbee_stack_update_tc_link_key(msg->data.update_tc_link_key.request.maxAttempts);
}

void sli_zigbee_stack_verify_partner_link_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.verify_partner_link_key.response.result = sli_zigbee_stack_verify_partner_link_key(msg->data.verify_partner_link_key.request.shortID,
                                                                                               msg->data.verify_partner_link_key.request.eui);
}

void sli_zigbee_stack_writing_stack_tokens_enabled_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.writing_stack_tokens_enabled.response.result = sli_zigbee_stack_writing_stack_tokens_enabled();
}

// public entrypoints

sl_status_t sl_zigbee_aps_crypt_message(bool encrypt,
                                        uint8_t length,
                                        const uint8_t *message,
                                        uint8_t apsHeaderEndIndex,
                                        sl_802154_long_addr_t remoteEui64)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.aps_crypt_message.request.encrypt = encrypt;
  msg.data.aps_crypt_message.request.length = length;

  if (length > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector message length exceeds expected maximum
  }

  memmove(msg.data.aps_crypt_message.request.message, message, sizeof(uint8_t) * length);
  msg.data.aps_crypt_message.request.apsHeaderEndIndex = apsHeaderEndIndex;

  if (remoteEui64 != NULL) {
    memmove(msg.data.aps_crypt_message.request.remoteEui64, remoteEui64, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_aps_crypt_message_process_ipc_command, &msg);

  if (remoteEui64 != NULL) {
    memmove(remoteEui64, msg.data.aps_crypt_message.request.remoteEui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.aps_crypt_message.response.result;
}

sl_status_t sl_zigbee_clear_key_table(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_clear_key_table_process_ipc_command, &msg);

  return msg.data.clear_key_table.response.result;
}

void sl_zigbee_clear_transient_link_keys(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_clear_transient_link_keys_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_erase_key_table_entry(uint8_t index)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.erase_key_table_entry.request.index = index;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_erase_key_table_entry_process_ipc_command, &msg);

  return msg.data.erase_key_table_entry.response.result;
}

uint8_t sl_zigbee_find_key_table_entry(sl_802154_long_addr_t address,
                                       bool linkKey)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (address != NULL) {
    memmove(msg.data.find_key_table_entry.request.address, address, sizeof(sl_802154_long_addr_t));
  }

  msg.data.find_key_table_entry.request.linkKey = linkKey;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_find_key_table_entry_process_ipc_command, &msg);

  if (address != NULL) {
    memmove(address, msg.data.find_key_table_entry.request.address, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.find_key_table_entry.response.result;
}

sl_status_t sl_zigbee_generate_random_key(sl_zigbee_key_data_t *keyAddress)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (keyAddress != NULL) {
    msg.data.generate_random_key.request.keyAddress = *keyAddress;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_generate_random_key_process_ipc_command, &msg);

  if (keyAddress != NULL) {
    *keyAddress = msg.data.generate_random_key.request.keyAddress;
  }

  return msg.data.generate_random_key.response.result;
}

uint32_t sl_zigbee_get_aps_frame_counter(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_aps_frame_counter_process_ipc_command, &msg);

  return msg.data.get_aps_frame_counter.response.result;
}

sl_status_t sl_zigbee_get_current_security_state(sl_zigbee_current_security_state_t *state)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (state != NULL) {
    msg.data.get_current_security_state.request.state = *state;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_current_security_state_process_ipc_command, &msg);

  if (state != NULL) {
    *state = msg.data.get_current_security_state.request.state;
  }

  return msg.data.get_current_security_state.response.result;
}

sl_status_t sl_zigbee_get_extended_security_bitmask(sl_zigbee_extended_security_bitmask_t *mask)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (mask != NULL) {
    msg.data.get_extended_security_bitmask.request.mask = *mask;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_extended_security_bitmask_process_ipc_command, &msg);

  if (mask != NULL) {
    *mask = msg.data.get_extended_security_bitmask.request.mask;
  }

  return msg.data.get_extended_security_bitmask.response.result;
}

uint32_t sl_zigbee_get_incoming_tc_link_key_frame_counter(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_incoming_tc_link_key_frame_counter_process_ipc_command, &msg);

  return msg.data.get_incoming_tc_link_key_frame_counter.response.result;
}

sl_status_t sl_zigbee_get_mfg_security_config(sl_zigbee_mfg_security_struct_t *settings)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (settings != NULL) {
    msg.data.get_mfg_security_config.request.settings = *settings;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_mfg_security_config_process_ipc_command, &msg);

  if (settings != NULL) {
    *settings = msg.data.get_mfg_security_config.request.settings;
  }

  return msg.data.get_mfg_security_config.response.result;
}

sl_zigbee_aps_rejoin_mode_t sl_zigbee_get_rejoin_mode(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_rejoin_mode_process_ipc_command, &msg);

  return msg.data.get_rejoin_mode.response.result;
}

uint32_t sl_zigbee_get_security_frame_counter(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_security_frame_counter_process_ipc_command, &msg);

  return msg.data.get_security_frame_counter.response.result;
}

uint16_t sl_zigbee_get_transient_key_timeout_s(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_transient_key_timeout_s_process_ipc_command, &msg);

  return msg.data.get_transient_key_timeout_s.response.result;
}

sl_status_t sl_zigbee_request_link_key(sl_802154_long_addr_t partner)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (partner != NULL) {
    memmove(msg.data.request_link_key.request.partner, partner, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_request_link_key_process_ipc_command, &msg);

  if (partner != NULL) {
    memmove(partner, msg.data.request_link_key.request.partner, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.request_link_key.response.result;
}

sl_status_t sl_zigbee_set_extended_security_bitmask(sl_zigbee_extended_security_bitmask_t mask)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_extended_security_bitmask.request.mask = mask;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_extended_security_bitmask_process_ipc_command, &msg);

  return msg.data.set_extended_security_bitmask.response.result;
}

void sl_zigbee_set_incoming_tc_link_key_frame_counter(uint32_t frameCounter)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_incoming_tc_link_key_frame_counter.request.frameCounter = frameCounter;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_incoming_tc_link_key_frame_counter_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_set_initial_security_state(sl_zigbee_initial_security_state_t *state)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (state != NULL) {
    msg.data.set_initial_security_state.request.state = *state;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_initial_security_state_process_ipc_command, &msg);

  if (state != NULL) {
    *state = msg.data.set_initial_security_state.request.state;
  }

  return msg.data.set_initial_security_state.response.result;
}

sl_status_t sl_zigbee_set_mfg_security_config(uint32_t magicNumber,
                                              const sl_zigbee_mfg_security_struct_t *settings)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_mfg_security_config.request.magicNumber = magicNumber;

  if (settings != NULL) {
    msg.data.set_mfg_security_config.request.settings = *settings;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_mfg_security_config_process_ipc_command, &msg);

  return msg.data.set_mfg_security_config.response.result;
}

sl_status_t sl_zigbee_set_outgoing_aps_frame_counter(uint32_t desiredValue)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_outgoing_aps_frame_counter.request.desiredValue = desiredValue;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_outgoing_aps_frame_counter_process_ipc_command, &msg);

  return msg.data.set_outgoing_aps_frame_counter.response.result;
}

sl_status_t sl_zigbee_set_outgoing_nwk_frame_counter(uint32_t desiredValue)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_outgoing_nwk_frame_counter.request.desiredValue = desiredValue;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_outgoing_nwk_frame_counter_process_ipc_command, &msg);

  return msg.data.set_outgoing_nwk_frame_counter.response.result;
}

void sl_zigbee_set_rejoin_mode(sl_zigbee_aps_rejoin_mode_t mode)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_rejoin_mode.request.mode = mode;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_rejoin_mode_process_ipc_command, &msg);
}

void sl_zigbee_set_transient_key_timeout_s(uint16_t seconds)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_transient_key_timeout_s.request.seconds = seconds;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_transient_key_timeout_s_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_start_writing_stack_tokens(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_start_writing_stack_tokens_process_ipc_command, &msg);

  return msg.data.start_writing_stack_tokens.response.result;
}

sl_status_t sl_zigbee_stop_writing_stack_tokens(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_stop_writing_stack_tokens_process_ipc_command, &msg);

  return msg.data.stop_writing_stack_tokens.response.result;
}

sl_status_t sl_zigbee_terminate_app_link_key_request(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_terminate_app_link_key_request_process_ipc_command, &msg);

  return msg.data.terminate_app_link_key_request.response.result;
}

sl_status_t sl_zigbee_update_app_link_key(sl_802154_long_addr_t partnerEui64)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (partnerEui64 != NULL) {
    memmove(msg.data.update_app_link_key.request.partnerEui64, partnerEui64, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_update_app_link_key_process_ipc_command, &msg);

  if (partnerEui64 != NULL) {
    memmove(partnerEui64, msg.data.update_app_link_key.request.partnerEui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.update_app_link_key.response.result;
}

sl_status_t sl_zigbee_update_tc_link_key(uint8_t maxAttempts)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.update_tc_link_key.request.maxAttempts = maxAttempts;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_update_tc_link_key_process_ipc_command, &msg);

  return msg.data.update_tc_link_key.response.result;
}

sl_status_t sl_zigbee_verify_partner_link_key(sl_802154_short_addr_t shortID,
                                              sl_802154_long_addr_t eui)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.verify_partner_link_key.request.shortID = shortID;

  if (eui != NULL) {
    memmove(msg.data.verify_partner_link_key.request.eui, eui, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_verify_partner_link_key_process_ipc_command, &msg);

  if (eui != NULL) {
    memmove(eui, msg.data.verify_partner_link_key.request.eui, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.verify_partner_link_key.response.result;
}

bool sl_zigbee_writing_stack_tokens_enabled(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_writing_stack_tokens_enabled_process_ipc_command, &msg);

  return msg.data.writing_stack_tokens_enabled.response.result;
}
