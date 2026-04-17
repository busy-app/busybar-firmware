/***************************************************************************//**
 * @file sl_zigbee_zdo_security_ipc_command_messages.h
 * @brief defines structured format for 'sl_zigbee_zdo_security' ipc messages
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef SL_ZIGBEE_ZDO_SECURITY_IPC_COMMAND_MESSAGES_H
#define SL_ZIGBEE_ZDO_SECURITY_IPC_COMMAND_MESSAGES_H

#include "stack/include/sl_zigbee_zdo_security.h"
#include "stack/internal/inc/sl_zigbee_zdo_security_internal_def.h"

typedef struct {
  sl_802154_short_addr_t dest;
  sl_802154_long_addr_t target;
} sli_zigbee_stack_get_authentication_level_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_get_authentication_level_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_get_authentication_level_ipc_req_t request;
  sli_zigbee_stack_get_authentication_level_ipc_rsp_t response;
} sli_zigbee_stack_get_authentication_level_ipc_msg_t;

typedef struct {
  sl_802154_long_addr_t eui64;
  sl_802154_short_addr_t short_id;
  uint8_t passphrase[16];
} sli_zigbee_stack_get_symmetric_passphrase_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_get_symmetric_passphrase_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_get_symmetric_passphrase_ipc_req_t request;
  sli_zigbee_stack_get_symmetric_passphrase_ipc_rsp_t response;
} sli_zigbee_stack_get_symmetric_passphrase_ipc_msg_t;

typedef struct {
  sl_802154_short_addr_t partnerNodeId;
  sl_802154_long_addr_t partnerLong;
  uint8_t keyIndex;
} sli_zigbee_stack_initiate_security_challenge_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_initiate_security_challenge_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_initiate_security_challenge_ipc_req_t request;
  sli_zigbee_stack_initiate_security_challenge_ipc_rsp_t response;
} sli_zigbee_stack_initiate_security_challenge_ipc_msg_t;

typedef struct {
  sl_802154_short_addr_t destination;
  sl_zigbee_aps_option_t options;
} sli_zigbee_stack_retrieve_authentication_token_ipc_req_t;

typedef struct {
  sli_zigbee_stack_retrieve_authentication_token_ipc_req_t request;
} sli_zigbee_stack_retrieve_authentication_token_ipc_msg_t;

#endif // SL_ZIGBEE_ZDO_SECURITY_IPC_COMMAND_MESSAGES_H
