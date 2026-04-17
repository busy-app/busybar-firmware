/***************************************************************************//**
 * @file zll-api-ipc-command-messages.c
 * @brief internal wrappers for 'zll-api' ipc commands
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
// automatically generated from zll-api.h.  Do not manually edit
#include "stack/include/zll-api.h"
#include "stack/internal/inc/zll-api-internal-def.h"
#include "stack/internal/src/ipc/zll-api-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_get_zll_primary_channel_mask_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_zll_primary_channel_mask.response.result = sli_zigbee_stack_get_zll_primary_channel_mask();
}

void sli_zigbee_stack_get_zll_secondary_channel_mask_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_zll_secondary_channel_mask.response.result = sli_zigbee_stack_get_zll_secondary_channel_mask();
}

void sli_zigbee_stack_is_zll_network_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.is_zll_network.response.result = sli_zigbee_stack_is_zll_network();
}

void sli_zigbee_stack_set_zll_additional_state_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_zll_additional_state(msg->data.set_zll_additional_state.request.mask);
}

void sli_zigbee_stack_set_zll_node_type_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_zll_node_type(msg->data.set_zll_node_type.request.nodeType);
}

void sli_zigbee_stack_set_zll_primary_channel_mask_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_zll_primary_channel_mask(msg->data.set_zll_primary_channel_mask.request.mask);
}

void sli_zigbee_stack_set_zll_secondary_channel_mask_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_zll_secondary_channel_mask(msg->data.set_zll_secondary_channel_mask.request.mask);
}

void sli_zigbee_stack_zll_clear_tokens_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_zigbee_stack_zll_clear_tokens();
}

void sli_zigbee_stack_zll_form_network_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zll_form_network.response.result = sli_zigbee_stack_zll_form_network(&msg->data.zll_form_network.request.networkInfo,
                                                                                 msg->data.zll_form_network.request.radioTxPower);
}

void sli_zigbee_stack_zll_get_policy_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zll_get_policy.response.result = sli_zigbee_stack_zll_get_policy();
}

void sli_zigbee_stack_zll_get_radio_idle_mode_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zll_get_radio_idle_mode.response.result = sli_zigbee_stack_zll_get_radio_idle_mode();
}

void sli_zigbee_stack_zll_get_token_stack_zll_data_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_zll_get_token_stack_zll_data(&msg->data.zll_get_token_stack_zll_data.request.token);
}

void sli_zigbee_stack_zll_get_token_stack_zll_security_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_zll_get_token_stack_zll_security(&msg->data.zll_get_token_stack_zll_security.request.token);
}

void sli_zigbee_stack_zll_get_tokens_stack_zll_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_zll_get_tokens_stack_zll(&msg->data.zll_get_tokens_stack_zll.request.data,
                                            &msg->data.zll_get_tokens_stack_zll.request.security);
}

void sli_zigbee_stack_zll_join_target_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zll_join_target.response.result = sli_zigbee_stack_zll_join_target(&msg->data.zll_join_target.request.targetNetworkInfo);
}

void sli_zigbee_stack_zll_operation_in_progress_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zll_operation_in_progress.response.result = sli_zigbee_stack_zll_operation_in_progress();
}

void sli_zigbee_stack_zll_rx_on_when_idle_get_active_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zll_rx_on_when_idle_get_active.response.result = sli_zigbee_stack_zll_rx_on_when_idle_get_active();
}

void sli_zigbee_stack_zll_scanning_complete_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_zigbee_stack_zll_scanning_complete();
}

void sli_zigbee_stack_zll_set_initial_security_state_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zll_set_initial_security_state.response.result = sli_zigbee_stack_zll_set_initial_security_state(&msg->data.zll_set_initial_security_state.request.networkKey,
                                                                                                             &msg->data.zll_set_initial_security_state.request.securityState);
}

void sli_zigbee_stack_zll_set_non_zll_network_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_zigbee_stack_zll_set_non_zll_network();
}

void sli_zigbee_stack_zll_set_policy_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zll_set_policy.response.result = sli_zigbee_stack_zll_set_policy(msg->data.zll_set_policy.request.policy);
}

void sli_zigbee_stack_zll_set_radio_idle_mode_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_zll_set_radio_idle_mode(msg->data.zll_set_radio_idle_mode.request.mode);
}

void sli_zigbee_stack_zll_set_rx_on_when_idle_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zll_set_rx_on_when_idle.response.result = sli_zigbee_stack_zll_set_rx_on_when_idle(msg->data.zll_set_rx_on_when_idle.request.durationMs);
}

void sli_zigbee_stack_zll_set_security_state_without_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zll_set_security_state_without_key.response.result = sli_zigbee_stack_zll_set_security_state_without_key(&msg->data.zll_set_security_state_without_key.request.securityState);
}

void sli_zigbee_stack_zll_set_token_stack_zll_data_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_zll_set_token_stack_zll_data(&msg->data.zll_set_token_stack_zll_data.request.token);
}

void sli_zigbee_stack_zll_start_scan_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zll_start_scan.response.result = sli_zigbee_stack_zll_start_scan(msg->data.zll_start_scan.request.channelMask,
                                                                             msg->data.zll_start_scan.request.radioPowerForScan,
                                                                             msg->data.zll_start_scan.request.nodeType);
}

// public entrypoints

uint32_t sl_zigbee_get_zll_primary_channel_mask(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_zll_primary_channel_mask_process_ipc_command, &msg);

  return msg.data.get_zll_primary_channel_mask.response.result;
}

uint32_t sl_zigbee_get_zll_secondary_channel_mask(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_zll_secondary_channel_mask_process_ipc_command, &msg);

  return msg.data.get_zll_secondary_channel_mask.response.result;
}

bool sl_zigbee_is_zll_network(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_is_zll_network_process_ipc_command, &msg);

  return msg.data.is_zll_network.response.result;
}

void sl_zigbee_set_zll_additional_state(uint16_t mask)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_zll_additional_state.request.mask = mask;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_zll_additional_state_process_ipc_command, &msg);
}

void sl_zigbee_set_zll_node_type(sl_zigbee_node_type_t nodeType)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_zll_node_type.request.nodeType = nodeType;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_zll_node_type_process_ipc_command, &msg);
}

void sl_zigbee_set_zll_primary_channel_mask(uint32_t mask)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_zll_primary_channel_mask.request.mask = mask;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_zll_primary_channel_mask_process_ipc_command, &msg);
}

void sl_zigbee_set_zll_secondary_channel_mask(uint32_t mask)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_zll_secondary_channel_mask.request.mask = mask;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_zll_secondary_channel_mask_process_ipc_command, &msg);
}

void sl_zigbee_zll_clear_tokens(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_clear_tokens_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_zll_form_network(sl_zigbee_zll_network_t *networkInfo,
                                       int8_t radioTxPower)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (networkInfo != NULL) {
    msg.data.zll_form_network.request.networkInfo = *networkInfo;
  }

  msg.data.zll_form_network.request.radioTxPower = radioTxPower;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_form_network_process_ipc_command, &msg);

  if (networkInfo != NULL) {
    *networkInfo = msg.data.zll_form_network.request.networkInfo;
  }

  return msg.data.zll_form_network.response.result;
}

sl_zigbee_zll_policy_t sl_zigbee_zll_get_policy(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_get_policy_process_ipc_command, &msg);

  return msg.data.zll_get_policy.response.result;
}

uint8_t sl_zigbee_zll_get_radio_idle_mode(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_get_radio_idle_mode_process_ipc_command, &msg);

  return msg.data.zll_get_radio_idle_mode.response.result;
}

void sl_zigbee_zll_get_token_stack_zll_data(sl_zigbee_tok_type_stack_zll_data_t *token)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (token != NULL) {
    msg.data.zll_get_token_stack_zll_data.request.token = *token;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_get_token_stack_zll_data_process_ipc_command, &msg);

  if (token != NULL) {
    *token = msg.data.zll_get_token_stack_zll_data.request.token;
  }
}

void sl_zigbee_zll_get_token_stack_zll_security(sl_zigbee_tok_type_stack_zll_security_t *token)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (token != NULL) {
    msg.data.zll_get_token_stack_zll_security.request.token = *token;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_get_token_stack_zll_security_process_ipc_command, &msg);

  if (token != NULL) {
    *token = msg.data.zll_get_token_stack_zll_security.request.token;
  }
}

void sl_zigbee_zll_get_tokens_stack_zll(sl_zigbee_tok_type_stack_zll_data_t *data,
                                        sl_zigbee_tok_type_stack_zll_security_t *security)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (data != NULL) {
    msg.data.zll_get_tokens_stack_zll.request.data = *data;
  }

  if (security != NULL) {
    msg.data.zll_get_tokens_stack_zll.request.security = *security;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_get_tokens_stack_zll_process_ipc_command, &msg);

  if (data != NULL) {
    *data = msg.data.zll_get_tokens_stack_zll.request.data;
  }

  if (security != NULL) {
    *security = msg.data.zll_get_tokens_stack_zll.request.security;
  }
}

sl_status_t sl_zigbee_zll_join_target(const sl_zigbee_zll_network_t *targetNetworkInfo)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (targetNetworkInfo != NULL) {
    msg.data.zll_join_target.request.targetNetworkInfo = *targetNetworkInfo;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_join_target_process_ipc_command, &msg);

  return msg.data.zll_join_target.response.result;
}

bool sl_zigbee_zll_operation_in_progress(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_operation_in_progress_process_ipc_command, &msg);

  return msg.data.zll_operation_in_progress.response.result;
}

bool sl_zigbee_zll_rx_on_when_idle_get_active(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_rx_on_when_idle_get_active_process_ipc_command, &msg);

  return msg.data.zll_rx_on_when_idle_get_active.response.result;
}

void sl_zigbee_zll_scanning_complete(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_scanning_complete_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_zll_set_initial_security_state(const sl_zigbee_key_data_t *networkKey,
                                                     const sl_zigbee_zll_initial_security_state_t *securityState)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (networkKey != NULL) {
    msg.data.zll_set_initial_security_state.request.networkKey = *networkKey;
  }

  if (securityState != NULL) {
    msg.data.zll_set_initial_security_state.request.securityState = *securityState;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_set_initial_security_state_process_ipc_command, &msg);

  return msg.data.zll_set_initial_security_state.response.result;
}

void sl_zigbee_zll_set_non_zll_network(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_set_non_zll_network_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_zll_set_policy(sl_zigbee_zll_policy_t policy)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.zll_set_policy.request.policy = policy;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_set_policy_process_ipc_command, &msg);

  return msg.data.zll_set_policy.response.result;
}

void sl_zigbee_zll_set_radio_idle_mode(sl_zigbee_radio_power_mode_t mode)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.zll_set_radio_idle_mode.request.mode = mode;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_set_radio_idle_mode_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_zll_set_rx_on_when_idle(uint32_t durationMs)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.zll_set_rx_on_when_idle.request.durationMs = durationMs;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_set_rx_on_when_idle_process_ipc_command, &msg);

  return msg.data.zll_set_rx_on_when_idle.response.result;
}

sl_status_t sl_zigbee_zll_set_security_state_without_key(const sl_zigbee_zll_initial_security_state_t *securityState)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (securityState != NULL) {
    msg.data.zll_set_security_state_without_key.request.securityState = *securityState;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_set_security_state_without_key_process_ipc_command, &msg);

  return msg.data.zll_set_security_state_without_key.response.result;
}

void sl_zigbee_zll_set_token_stack_zll_data(sl_zigbee_tok_type_stack_zll_data_t *token)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (token != NULL) {
    msg.data.zll_set_token_stack_zll_data.request.token = *token;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_set_token_stack_zll_data_process_ipc_command, &msg);

  if (token != NULL) {
    *token = msg.data.zll_set_token_stack_zll_data.request.token;
  }
}

sl_status_t sl_zigbee_zll_start_scan(uint32_t channelMask,
                                     int8_t radioPowerForScan,
                                     sl_zigbee_node_type_t nodeType)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.zll_start_scan.request.channelMask = channelMask;
  msg.data.zll_start_scan.request.radioPowerForScan = radioPowerForScan;
  msg.data.zll_start_scan.request.nodeType = nodeType;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zll_start_scan_process_ipc_command, &msg);

  return msg.data.zll_start_scan.response.result;
}
