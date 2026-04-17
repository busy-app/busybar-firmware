/***************************************************************************//**
 * @file sl_zigbee_security_manager_dlk_ecc_ipc_command_messages.h
 * @brief defines structured format for 'sl_zigbee_security_manager_dlk_ecc' ipc messages
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
// automatically generated from sl_zigbee_security_manager_dlk_ecc.h.  Do not manually edit
#ifndef SL_ZIGBEE_SECURITY_MANAGER_DLK_ECC_IPC_COMMAND_MESSAGES_H
#define SL_ZIGBEE_SECURITY_MANAGER_DLK_ECC_IPC_COMMAND_MESSAGES_H

#include "stack/include/sl_zigbee_security_manager_dlk_ecc.h"
#include "stack/internal/inc/sl_zigbee_security_manager_dlk_ecc_internal_def.h"

typedef struct {
  sl_zigbee_sec_man_dlk_ecc_context_t dlk_ecc_ctx;
} sli_zigbee_stack_sec_man_ecc_derive_link_key_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_sec_man_ecc_derive_link_key_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_sec_man_ecc_derive_link_key_ipc_req_t request;
  sli_zigbee_stack_sec_man_ecc_derive_link_key_ipc_rsp_t response;
} sli_zigbee_stack_sec_man_ecc_derive_link_key_ipc_msg_t;

typedef struct {
  sl_zigbee_sec_man_dlk_ecc_context_t dlk_ecc_ctx;
  uint8_t our_eui[EUI64_SIZE];
  uint8_t their_eui[EUI64_SIZE];
} sli_zigbee_stack_sec_man_ecc_expand_shared_secret_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_sec_man_ecc_expand_shared_secret_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_sec_man_ecc_expand_shared_secret_ipc_req_t request;
  sli_zigbee_stack_sec_man_ecc_expand_shared_secret_ipc_rsp_t response;
} sli_zigbee_stack_sec_man_ecc_expand_shared_secret_ipc_msg_t;

typedef struct {
  sl_zigbee_sec_man_dlk_ecc_context_t dlk_ecc_ctx;
  uint8_t peer_public_key[DLK_ECC_MAX_PUBLIC_KEY_SIZE];
  size_t peer_key_len;
} sli_zigbee_stack_sec_man_ecc_extract_shared_secret_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_sec_man_ecc_extract_shared_secret_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_sec_man_ecc_extract_shared_secret_ipc_req_t request;
  sli_zigbee_stack_sec_man_ecc_extract_shared_secret_ipc_rsp_t response;
} sli_zigbee_stack_sec_man_ecc_extract_shared_secret_ipc_msg_t;

typedef struct {
  sl_zigbee_sec_man_dlk_ecc_context_t dlk_ecc_ctx;
} sli_zigbee_stack_sec_man_ecc_free_ipc_req_t;

typedef struct {
  sli_zigbee_stack_sec_man_ecc_free_ipc_req_t request;
} sli_zigbee_stack_sec_man_ecc_free_ipc_msg_t;

typedef struct {
  sl_zigbee_sec_man_dlk_ecc_context_t dlk_ecc_ctx;
  uint8_t public_key_buff[DLK_ECC_MAX_PUBLIC_KEY_SIZE];
  size_t key_len_out;
} sli_zigbee_stack_sec_man_ecc_generate_keypair_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_sec_man_ecc_generate_keypair_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_sec_man_ecc_generate_keypair_ipc_req_t request;
  sli_zigbee_stack_sec_man_ecc_generate_keypair_ipc_rsp_t response;
} sli_zigbee_stack_sec_man_ecc_generate_keypair_ipc_msg_t;

typedef struct {
  sl_zigbee_sec_man_dlk_ecc_context_t dlk_ecc_ctx;
  sl_zb_dlk_ecc_config_t config;
  uint8_t psk[16];
} sli_zigbee_stack_sec_man_ecc_init_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_sec_man_ecc_init_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_sec_man_ecc_init_ipc_req_t request;
  sli_zigbee_stack_sec_man_ecc_init_ipc_rsp_t response;
} sli_zigbee_stack_sec_man_ecc_init_ipc_msg_t;

typedef struct {
  sl_zigbee_sec_man_dlk_ecc_context_t dlk_ecc_ctx;
  uint8_t our_eui[EUI64_SIZE];
  uint8_t their_eui[EUI64_SIZE];
} sli_zigbee_stack_sec_man_speke_expand_shared_secret_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_sec_man_speke_expand_shared_secret_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_sec_man_speke_expand_shared_secret_ipc_req_t request;
  sli_zigbee_stack_sec_man_speke_expand_shared_secret_ipc_rsp_t response;
} sli_zigbee_stack_sec_man_speke_expand_shared_secret_ipc_msg_t;

#endif // SL_ZIGBEE_SECURITY_MANAGER_DLK_ECC_IPC_COMMAND_MESSAGES_H
