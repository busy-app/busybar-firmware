/***************************************************************************//**
 * @file sl_zigbee_stack_specific_tlv_ipc_command_messages.c
 * @brief internal wrappers for 'sl_zigbee_stack_specific_tlv' ipc commands
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
// automatically generated from sl_zigbee_stack_specific_tlv.h.  Do not manually edit
#include "stack/include/sl_zigbee_stack_specific_tlv.h"
#include "stack/internal/inc/sl_zigbee_stack_specific_tlv_internal_def.h"
#include "stack/internal/src/ipc/sl_zigbee_stack_specific_tlv_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_global_tlv_add_configurations_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_add_configurations.response.result = sli_zigbee_stack_global_tlv_add_configurations(&msg->data.global_tlv_add_configurations.request.buffer,
                                                                                                           msg->data.global_tlv_add_configurations.request.index,
                                                                                                           msg->data.global_tlv_add_configurations.request.tag_c,
                                                                                                           msg->data.global_tlv_add_configurations.request.tag_v);
}

void sli_zigbee_stack_global_tlv_beacon_appendix_encapsulation_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_beacon_appendix_encapsulation.response.result = sli_zigbee_stack_global_tlv_beacon_appendix_encapsulation(&msg->data.global_tlv_beacon_appendix_encapsulation.request.encapTlv);
}

void sli_zigbee_stack_global_tlv_configuration_params_from_bitmask_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_configuration_params_from_bitmask.response.result = sli_zigbee_stack_global_tlv_configuration_params_from_bitmask(msg->data.global_tlv_configuration_params_from_bitmask.request.mask);
}

void sli_zigbee_stack_global_tlv_configuration_params_get_bitmask_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_configuration_params_get_bitmask.response.result = sli_zigbee_stack_global_tlv_configuration_params_get_bitmask(&msg->data.global_tlv_configuration_params_get_bitmask.request.tlv);
}

void sli_zigbee_stack_global_tlv_device_capability_extension_param_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_device_capability_extension_param.response.result = sli_zigbee_stack_global_tlv_device_capability_extension_param(msg->data.global_tlv_device_capability_extension_param.request.bitmask);
}

void sli_zigbee_stack_global_tlv_fragmentation_param_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_fragmentation_param.response.result = sli_zigbee_stack_global_tlv_fragmentation_param();
}

void sli_zigbee_stack_global_tlv_get_configurations_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_get_configurations.response.result = sli_zigbee_stack_global_tlv_get_configurations(msg->data.global_tlv_get_configurations.request.tag_c,
                                                                                                           msg->data.global_tlv_get_configurations.request.tag_v);
}

void sli_zigbee_stack_global_tlv_joiner_encapsulation_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_joiner_encapsulation.response.result = sli_zigbee_stack_global_tlv_joiner_encapsulation(&msg->data.global_tlv_joiner_encapsulation.request.encapTlv);
}

void sli_zigbee_stack_global_tlv_next_channel_change_get_bitmask_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_next_channel_change_get_bitmask.response.result = sli_zigbee_stack_global_tlv_next_channel_change_get_bitmask(&msg->data.global_tlv_next_channel_change_get_bitmask.request.tlv);
}

void sli_zigbee_stack_global_tlv_next_channel_from_pg_ch_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_next_channel_from_pg_ch.response.result = sli_zigbee_stack_global_tlv_next_channel_from_pg_ch(msg->data.global_tlv_next_channel_from_pg_ch.request.page,
                                                                                                                     msg->data.global_tlv_next_channel_from_pg_ch.request.channel);
}

void sli_zigbee_stack_global_tlv_next_pan_from_pan_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_next_pan_from_pan.response.result = sli_zigbee_stack_global_tlv_next_pan_from_pan(msg->data.global_tlv_next_pan_from_pan.request.pan_id);
}

void sli_zigbee_stack_global_tlv_next_pan_get_pan_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_next_pan_get_pan.response.result = sli_zigbee_stack_global_tlv_next_pan_get_pan(&msg->data.global_tlv_next_pan_get_pan.request.tlv);
}

void sli_zigbee_stack_global_tlv_pan_id_conflict_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_pan_id_conflict.response.result = sli_zigbee_stack_global_tlv_pan_id_conflict();
}

void sli_zigbee_stack_global_tlv_router_information_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_router_information.response.result = sli_zigbee_stack_global_tlv_router_information();
}

void sli_zigbee_stack_global_tlv_router_information_get_bitmask_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_router_information_get_bitmask.response.result = sli_zigbee_stack_global_tlv_router_information_get_bitmask(&msg->data.global_tlv_router_information_get_bitmask.request.tlv);
}

void sli_zigbee_stack_global_tlv_supp_key_negotiation_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_supp_key_negotiation.response.result = sli_zigbee_stack_global_tlv_supp_key_negotiation();
}

void sli_zigbee_stack_global_tlv_symmetric_passphrase_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.global_tlv_symmetric_passphrase.response.result = sli_zigbee_stack_global_tlv_symmetric_passphrase(&msg->data.global_tlv_symmetric_passphrase.request.passphrase);
}

// public entrypoints

sl_status_t sl_zigbee_global_tlv_add_configurations(sli_buffer_manager_buffer_t *buffer,
                                                    uint16_t index,
                                                    uint8_t tag_c,
                                                    const uint8_t *tag_v)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (buffer != NULL) {
    msg.data.global_tlv_add_configurations.request.buffer = *buffer;
  }

  msg.data.global_tlv_add_configurations.request.index = index;
  msg.data.global_tlv_add_configurations.request.tag_c = tag_c;

  if (tag_c > 10) {
    assert(false); // "vector tag_v length exceeds expected maximum
  }

  memmove(msg.data.global_tlv_add_configurations.request.tag_v, tag_v, sizeof(uint8_t) * tag_c);
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_add_configurations_process_ipc_command, &msg);

  if (buffer != NULL) {
    *buffer = msg.data.global_tlv_add_configurations.request.buffer;
  }

  return msg.data.global_tlv_add_configurations.response.result;
}

sl_status_t sl_zigbee_global_tlv_beacon_appendix_encapsulation(sl_zigbee_global_tlv_beacon_appendix_encap_t *encapTlv)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (encapTlv != NULL) {
    msg.data.global_tlv_beacon_appendix_encapsulation.request.encapTlv = *encapTlv;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_beacon_appendix_encapsulation_process_ipc_command, &msg);

  if (encapTlv != NULL) {
    *encapTlv = msg.data.global_tlv_beacon_appendix_encapsulation.request.encapTlv;
  }

  return msg.data.global_tlv_beacon_appendix_encapsulation.response.result;
}

sl_zigbee_global_tlv_configuration_params_t sl_zigbee_global_tlv_configuration_params_from_bitmask(uint16_t mask)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.global_tlv_configuration_params_from_bitmask.request.mask = mask;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_configuration_params_from_bitmask_process_ipc_command, &msg);

  return msg.data.global_tlv_configuration_params_from_bitmask.response.result;
}

uint16_t sl_zigbee_global_tlv_configuration_params_get_bitmask(sl_zigbee_global_tlv_configuration_params_t *tlv)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (tlv != NULL) {
    msg.data.global_tlv_configuration_params_get_bitmask.request.tlv = *tlv;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_configuration_params_get_bitmask_process_ipc_command, &msg);

  if (tlv != NULL) {
    *tlv = msg.data.global_tlv_configuration_params_get_bitmask.request.tlv;
  }

  return msg.data.global_tlv_configuration_params_get_bitmask.response.result;
}

sl_zigbee_global_tlv_device_capability_extension_t sl_zigbee_global_tlv_device_capability_extension_param(uint16_t bitmask)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.global_tlv_device_capability_extension_param.request.bitmask = bitmask;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_device_capability_extension_param_process_ipc_command, &msg);

  return msg.data.global_tlv_device_capability_extension_param.response.result;
}

sl_zigbee_global_tlv_fragmentation_param_t sl_zigbee_global_tlv_fragmentation_param(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_fragmentation_param_process_ipc_command, &msg);

  return msg.data.global_tlv_fragmentation_param.response.result;
}

sli_buffer_manager_buffer_t sl_zigbee_global_tlv_get_configurations(uint8_t tag_c,
                                                                    const uint8_t *tag_v)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.global_tlv_get_configurations.request.tag_c = tag_c;

  if (tag_c > 10) {
    assert(false); // "vector tag_v length exceeds expected maximum
  }

  memmove(msg.data.global_tlv_get_configurations.request.tag_v, tag_v, sizeof(uint8_t) * tag_c);
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_get_configurations_process_ipc_command, &msg);

  return msg.data.global_tlv_get_configurations.response.result;
}

sl_status_t sl_zigbee_global_tlv_joiner_encapsulation(sl_zigbee_global_tlv_joiner_encap_t *encapTlv)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (encapTlv != NULL) {
    msg.data.global_tlv_joiner_encapsulation.request.encapTlv = *encapTlv;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_joiner_encapsulation_process_ipc_command, &msg);

  if (encapTlv != NULL) {
    *encapTlv = msg.data.global_tlv_joiner_encapsulation.request.encapTlv;
  }

  return msg.data.global_tlv_joiner_encapsulation.response.result;
}

uint32_t sl_zigbee_global_tlv_next_channel_change_get_bitmask(sl_zigbee_global_tlv_next_channel_change_t *tlv)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (tlv != NULL) {
    msg.data.global_tlv_next_channel_change_get_bitmask.request.tlv = *tlv;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_next_channel_change_get_bitmask_process_ipc_command, &msg);

  if (tlv != NULL) {
    *tlv = msg.data.global_tlv_next_channel_change_get_bitmask.request.tlv;
  }

  return msg.data.global_tlv_next_channel_change_get_bitmask.response.result;
}

sl_zigbee_global_tlv_next_channel_change_t sl_zigbee_global_tlv_next_channel_from_pg_ch(uint8_t page,
                                                                                        uint8_t channel)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.global_tlv_next_channel_from_pg_ch.request.page = page;
  msg.data.global_tlv_next_channel_from_pg_ch.request.channel = channel;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_next_channel_from_pg_ch_process_ipc_command, &msg);

  return msg.data.global_tlv_next_channel_from_pg_ch.response.result;
}

sl_zigbee_global_tlv_next_pan_id_t sl_zigbee_global_tlv_next_pan_from_pan(sl_802154_pan_id_t pan_id)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.global_tlv_next_pan_from_pan.request.pan_id = pan_id;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_next_pan_from_pan_process_ipc_command, &msg);

  return msg.data.global_tlv_next_pan_from_pan.response.result;
}

sl_802154_pan_id_t sl_zigbee_global_tlv_next_pan_get_pan(sl_zigbee_global_tlv_next_pan_id_t *tlv)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (tlv != NULL) {
    msg.data.global_tlv_next_pan_get_pan.request.tlv = *tlv;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_next_pan_get_pan_process_ipc_command, &msg);

  if (tlv != NULL) {
    *tlv = msg.data.global_tlv_next_pan_get_pan.request.tlv;
  }

  return msg.data.global_tlv_next_pan_get_pan.response.result;
}

sl_zigbee_global_tlv_pan_id_conflict_t sl_zigbee_global_tlv_pan_id_conflict(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_pan_id_conflict_process_ipc_command, &msg);

  return msg.data.global_tlv_pan_id_conflict.response.result;
}

sl_zigbee_global_tlv_router_information_t sl_zigbee_global_tlv_router_information(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_router_information_process_ipc_command, &msg);

  return msg.data.global_tlv_router_information.response.result;
}

uint16_t sl_zigbee_global_tlv_router_information_get_bitmask(sl_zigbee_global_tlv_router_information_t *tlv)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (tlv != NULL) {
    msg.data.global_tlv_router_information_get_bitmask.request.tlv = *tlv;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_router_information_get_bitmask_process_ipc_command, &msg);

  if (tlv != NULL) {
    *tlv = msg.data.global_tlv_router_information_get_bitmask.request.tlv;
  }

  return msg.data.global_tlv_router_information_get_bitmask.response.result;
}

sl_zigbee_global_tlv_supp_key_negotiation_t sl_zigbee_global_tlv_supp_key_negotiation(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_supp_key_negotiation_process_ipc_command, &msg);

  return msg.data.global_tlv_supp_key_negotiation.response.result;
}

sl_zigbee_global_tlv_symmetric_passphrase_t sl_zigbee_global_tlv_symmetric_passphrase(const uint8_t *passphrase)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (passphrase != NULL) {
    msg.data.global_tlv_symmetric_passphrase.request.passphrase = *passphrase;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_global_tlv_symmetric_passphrase_process_ipc_command, &msg);

  return msg.data.global_tlv_symmetric_passphrase.response.result;
}
