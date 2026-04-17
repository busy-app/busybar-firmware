/***************************************************************************//**
 * @file sl_zigbee_dlk_negotiation_ipc_command_messages.h
 * @brief defines structured format for 'sl_zigbee_dlk_negotiation' ipc messages
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
// automatically generated from sl_zigbee_dlk_negotiation.h.  Do not manually edit
#ifndef SL_ZIGBEE_DLK_NEGOTIATION_IPC_COMMAND_MESSAGES_H
#define SL_ZIGBEE_DLK_NEGOTIATION_IPC_COMMAND_MESSAGES_H

#include "stack/include/sl_zigbee_dlk_negotiation.h"
#include "stack/internal/inc/sl_zigbee_dlk_negotiation_internal_def.h"

typedef struct {
  sl_zigbee_dlk_negotiation_context_t dlk_negotiation_ctx;
} sli_zigbee_stack_dlk_close_key_exchange_ipc_req_t;

typedef struct {
  sli_zigbee_stack_dlk_close_key_exchange_ipc_req_t request;
} sli_zigbee_stack_dlk_close_key_exchange_ipc_msg_t;

typedef struct {
  sl_zigbee_dlk_negotiation_context_t dlk_negotiation_ctx;
  sl_zigbee_address_info partner;
  uint8_t partner_key[64];
  size_t partner_key_length;
} sli_zigbee_stack_dlk_finish_key_exchange_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_dlk_finish_key_exchange_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_dlk_finish_key_exchange_ipc_req_t request;
  sli_zigbee_stack_dlk_finish_key_exchange_ipc_rsp_t response;
} sli_zigbee_stack_dlk_finish_key_exchange_ipc_msg_t;

typedef struct {
  sl_zigbee_dlk_negotiation_context_t dlk_negotiation_ctx;
  sl_zigbee_address_info partner;
  sl_zigbee_dlk_negotiation_method keyProtocol;
  sl_zigbee_dlk_negotiation_shared_secret_source secret;
  uint8_t psk_bytes[16];
  sl_zigbee_dlk_start_complete_callback dlk_start_complete_callback;
  sl_zigbee_dlk_finish_complete_callback dlk_finish_complete_callback;
} sli_zigbee_stack_dlk_open_key_exchange_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_dlk_open_key_exchange_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_dlk_open_key_exchange_ipc_req_t request;
  sli_zigbee_stack_dlk_open_key_exchange_ipc_rsp_t response;
} sli_zigbee_stack_dlk_open_key_exchange_ipc_msg_t;

typedef struct {
  sl_zigbee_dlk_negotiation_context_t dlk_negotiation_ctx;
  sl_zigbee_address_info partner;
} sli_zigbee_stack_dlk_start_key_exchange_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_dlk_start_key_exchange_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_dlk_start_key_exchange_ipc_req_t request;
  sli_zigbee_stack_dlk_start_key_exchange_ipc_rsp_t response;
} sli_zigbee_stack_dlk_start_key_exchange_ipc_msg_t;

#endif // SL_ZIGBEE_DLK_NEGOTIATION_IPC_COMMAND_MESSAGES_H
