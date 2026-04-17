/***************************************************************************//**
 * @file sl_zigbee_zdo_dlk_negotiation_ipc_command_messages.c
 * @brief internal wrappers for 'sl_zigbee_zdo_dlk_negotiation' ipc commands
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
// automatically generated from sl_zigbee_zdo_dlk_negotiation.h.  Do not manually edit
#include "stack/include/sl_zigbee_zdo_dlk_negotiation.h"
#include "stack/internal/inc/sl_zigbee_zdo_dlk_negotiation_internal_def.h"
#include "stack/internal/src/ipc/sl_zigbee_zdo_dlk_negotiation_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_zdo_dlk_enabled_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zdo_dlk_enabled.response.result = sli_zigbee_stack_zdo_dlk_enabled();
}

void sli_zigbee_stack_zdo_dlk_get_supported_negotiation_parameters_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_zdo_dlk_get_supported_negotiation_parameters(&msg->data.zdo_dlk_get_supported_negotiation_parameters.request.method_mask,
                                                                &msg->data.zdo_dlk_get_supported_negotiation_parameters.request.secret_mask);
}

void sli_zigbee_stack_zdo_dlk_start_key_negotiation_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zdo_dlk_start_key_negotiation.response.result = sli_zigbee_stack_zdo_dlk_start_key_negotiation(&msg->data.zdo_dlk_start_key_negotiation.request.partner,
                                                                                                           msg->data.zdo_dlk_start_key_negotiation.request.selected_method,
                                                                                                           msg->data.zdo_dlk_start_key_negotiation.request.selected_secret);
}

void sli_zigbee_stack_zdo_dlk_start_key_update_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zdo_dlk_start_key_update.response.result = sli_zigbee_stack_zdo_dlk_start_key_update(&msg->data.zdo_dlk_start_key_update.request.target,
                                                                                                 msg->data.zdo_dlk_start_key_update.request.selected_method,
                                                                                                 msg->data.zdo_dlk_start_key_update.request.selected_secret);
}

void slxi_zigbee_stack_gu_zdo_dlk_mangle_packet_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gu_zdo_dlk_mangle_packet.response.result = slxi_zigbee_stack_gu_zdo_dlk_mangle_packet(&msg->data.gu_zdo_dlk_mangle_packet.request.buffer);
}

void slxi_zigbee_stack_gu_zdo_dlk_override_psk_fetch_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.gu_zdo_dlk_override_psk_fetch.response.result = slxi_zigbee_stack_gu_zdo_dlk_override_psk_fetch(msg->data.gu_zdo_dlk_override_psk_fetch.request.key_buffer);
}

void slxi_zigbee_stack_gu_zdo_dlk_override_supported_params_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  slxi_zigbee_stack_gu_zdo_dlk_override_supported_params(&msg->data.gu_zdo_dlk_override_supported_params.request.method_mask,
                                                         &msg->data.gu_zdo_dlk_override_supported_params.request.secret_mask);
}

// public entrypoints

bool sl_zigbee_zdo_dlk_enabled(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zdo_dlk_enabled_process_ipc_command, &msg);

  return msg.data.zdo_dlk_enabled.response.result;
}

void sl_zigbee_zdo_dlk_get_supported_negotiation_parameters(sl_zigbee_dlk_supported_negotiation_method *method_mask,
                                                            sl_zigbee_dlk_negotiation_supported_shared_secret_source *secret_mask)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (method_mask != NULL) {
    msg.data.zdo_dlk_get_supported_negotiation_parameters.request.method_mask = *method_mask;
  }

  if (secret_mask != NULL) {
    msg.data.zdo_dlk_get_supported_negotiation_parameters.request.secret_mask = *secret_mask;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zdo_dlk_get_supported_negotiation_parameters_process_ipc_command, &msg);

  if (method_mask != NULL) {
    *method_mask = msg.data.zdo_dlk_get_supported_negotiation_parameters.request.method_mask;
  }

  if (secret_mask != NULL) {
    *secret_mask = msg.data.zdo_dlk_get_supported_negotiation_parameters.request.secret_mask;
  }
}

sl_status_t sl_zigbee_zdo_dlk_start_key_negotiation(sl_zigbee_address_info *partner,
                                                    sl_zigbee_dlk_negotiation_method selected_method,
                                                    sl_zigbee_dlk_negotiation_shared_secret_source selected_secret)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (partner != NULL) {
    msg.data.zdo_dlk_start_key_negotiation.request.partner = *partner;
  }

  msg.data.zdo_dlk_start_key_negotiation.request.selected_method = selected_method;
  msg.data.zdo_dlk_start_key_negotiation.request.selected_secret = selected_secret;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zdo_dlk_start_key_negotiation_process_ipc_command, &msg);

  if (partner != NULL) {
    *partner = msg.data.zdo_dlk_start_key_negotiation.request.partner;
  }

  return msg.data.zdo_dlk_start_key_negotiation.response.result;
}

sl_status_t sl_zigbee_zdo_dlk_start_key_update(sl_zigbee_address_info *target,
                                               sl_zigbee_dlk_negotiation_method selected_method,
                                               sl_zigbee_dlk_negotiation_shared_secret_source selected_secret)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (target != NULL) {
    msg.data.zdo_dlk_start_key_update.request.target = *target;
  }

  msg.data.zdo_dlk_start_key_update.request.selected_method = selected_method;
  msg.data.zdo_dlk_start_key_update.request.selected_secret = selected_secret;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zdo_dlk_start_key_update_process_ipc_command, &msg);

  if (target != NULL) {
    *target = msg.data.zdo_dlk_start_key_update.request.target;
  }

  return msg.data.zdo_dlk_start_key_update.response.result;
}

bool slx_zigbee_gu_zdo_dlk_mangle_packet(sli_buffer_manager_buffer_t *buffer)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (buffer != NULL) {
    msg.data.gu_zdo_dlk_mangle_packet.request.buffer = *buffer;
  }

  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_gu_zdo_dlk_mangle_packet_process_ipc_command, &msg);

  if (buffer != NULL) {
    *buffer = msg.data.gu_zdo_dlk_mangle_packet.request.buffer;
  }

  return msg.data.gu_zdo_dlk_mangle_packet.response.result;
}

bool slx_zigbee_gu_zdo_dlk_override_psk_fetch(uint8_t *key_buffer)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (key_buffer != NULL) {
    memmove(msg.data.gu_zdo_dlk_override_psk_fetch.request.key_buffer, key_buffer, sizeof(uint8_t) * SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  }

  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_gu_zdo_dlk_override_psk_fetch_process_ipc_command, &msg);

  if (key_buffer != NULL) {
    memmove(key_buffer, msg.data.gu_zdo_dlk_override_psk_fetch.request.key_buffer, sizeof(uint8_t) * SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  }

  return msg.data.gu_zdo_dlk_override_psk_fetch.response.result;
}

void slx_zigbee_gu_zdo_dlk_override_supported_params(sl_zigbee_dlk_supported_negotiation_method *method_mask,
                                                     sl_zigbee_dlk_negotiation_supported_shared_secret_source *secret_mask)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (method_mask != NULL) {
    msg.data.gu_zdo_dlk_override_supported_params.request.method_mask = *method_mask;
  }

  if (secret_mask != NULL) {
    msg.data.gu_zdo_dlk_override_supported_params.request.secret_mask = *secret_mask;
  }

  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_gu_zdo_dlk_override_supported_params_process_ipc_command, &msg);

  if (method_mask != NULL) {
    *method_mask = msg.data.gu_zdo_dlk_override_supported_params.request.method_mask;
  }

  if (secret_mask != NULL) {
    *secret_mask = msg.data.gu_zdo_dlk_override_supported_params.request.secret_mask;
  }
}
