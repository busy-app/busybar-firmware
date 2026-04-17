/***************************************************************************//**
 * @file sl_zigbee_zdo_security_ipc_command_messages.c
 * @brief internal wrappers for 'sl_zigbee_zdo_security' ipc commands
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
// automatically generated from sl_zigbee_zdo_security.h.  Do not manually edit
#include "stack/include/sl_zigbee_zdo_security.h"
#include "stack/internal/inc/sl_zigbee_zdo_security_internal_def.h"
#include "stack/internal/src/ipc/sl_zigbee_zdo_security_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_get_authentication_level_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_authentication_level.response.result = sli_zigbee_stack_get_authentication_level(msg->data.get_authentication_level.request.dest,
                                                                                                 msg->data.get_authentication_level.request.target);
}

void sli_zigbee_stack_get_symmetric_passphrase_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_symmetric_passphrase.response.result = sli_zigbee_stack_get_symmetric_passphrase(msg->data.get_symmetric_passphrase.request.eui64,
                                                                                                 msg->data.get_symmetric_passphrase.request.short_id,
                                                                                                 msg->data.get_symmetric_passphrase.request.passphrase);
}

void sli_zigbee_stack_initiate_security_challenge_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.initiate_security_challenge.response.result = sli_zigbee_stack_initiate_security_challenge(msg->data.initiate_security_challenge.request.partnerNodeId,
                                                                                                       msg->data.initiate_security_challenge.request.partnerLong,
                                                                                                       msg->data.initiate_security_challenge.request.keyIndex);
}

void sli_zigbee_stack_retrieve_authentication_token_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_retrieve_authentication_token(msg->data.retrieve_authentication_token.request.destination,
                                                 msg->data.retrieve_authentication_token.request.options);
}

// public entrypoints

sl_status_t sl_zigbee_get_authentication_level(sl_802154_short_addr_t dest,
                                               sl_802154_long_addr_t target)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_authentication_level.request.dest = dest;

  if (target != NULL) {
    memmove(msg.data.get_authentication_level.request.target, target, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_authentication_level_process_ipc_command, &msg);

  if (target != NULL) {
    memmove(target, msg.data.get_authentication_level.request.target, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.get_authentication_level.response.result;
}

sl_status_t sl_zigbee_get_symmetric_passphrase(sl_802154_long_addr_t eui64,
                                               sl_802154_short_addr_t short_id,
                                               uint8_t *passphrase)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (eui64 != NULL) {
    memmove(msg.data.get_symmetric_passphrase.request.eui64, eui64, sizeof(sl_802154_long_addr_t));
  }

  msg.data.get_symmetric_passphrase.request.short_id = short_id;

  if (passphrase != NULL) {
    memmove(msg.data.get_symmetric_passphrase.request.passphrase, passphrase, sizeof(uint8_t) * 16);
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_symmetric_passphrase_process_ipc_command, &msg);

  if (eui64 != NULL) {
    memmove(eui64, msg.data.get_symmetric_passphrase.request.eui64, sizeof(sl_802154_long_addr_t));
  }

  if (passphrase != NULL) {
    memmove(passphrase, msg.data.get_symmetric_passphrase.request.passphrase, sizeof(uint8_t) * 16);
  }

  return msg.data.get_symmetric_passphrase.response.result;
}

sl_status_t sl_zigbee_initiate_security_challenge(sl_802154_short_addr_t partnerNodeId,
                                                  sl_802154_long_addr_t partnerLong,
                                                  uint8_t keyIndex)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.initiate_security_challenge.request.partnerNodeId = partnerNodeId;

  if (partnerLong != NULL) {
    memmove(msg.data.initiate_security_challenge.request.partnerLong, partnerLong, sizeof(sl_802154_long_addr_t));
  }

  msg.data.initiate_security_challenge.request.keyIndex = keyIndex;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_initiate_security_challenge_process_ipc_command, &msg);

  if (partnerLong != NULL) {
    memmove(partnerLong, msg.data.initiate_security_challenge.request.partnerLong, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.initiate_security_challenge.response.result;
}

void sl_zigbee_retrieve_authentication_token(sl_802154_short_addr_t destination,
                                             sl_zigbee_aps_option_t options)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.retrieve_authentication_token.request.destination = destination;
  msg.data.retrieve_authentication_token.request.options = options;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_retrieve_authentication_token_process_ipc_command, &msg);
}
