/***************************************************************************//**
 * @file sl_zigbee_zdo_dlk_negotiation_ipc_command_messages.h
 * @brief defines structured format for 'sl_zigbee_zdo_dlk_negotiation' ipc messages
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
#ifndef SL_ZIGBEE_ZDO_DLK_NEGOTIATION_IPC_COMMAND_MESSAGES_H
#define SL_ZIGBEE_ZDO_DLK_NEGOTIATION_IPC_COMMAND_MESSAGES_H

#include "stack/include/sl_zigbee_zdo_dlk_negotiation.h"
#include "stack/internal/inc/sl_zigbee_zdo_dlk_negotiation_internal_def.h"

typedef struct {
  bool result;
} sli_zigbee_stack_zdo_dlk_enabled_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_zdo_dlk_enabled_ipc_rsp_t response;
} sli_zigbee_stack_zdo_dlk_enabled_ipc_msg_t;

typedef struct {
  sl_zigbee_dlk_supported_negotiation_method method_mask;
  sl_zigbee_dlk_negotiation_supported_shared_secret_source secret_mask;
} sli_zigbee_stack_zdo_dlk_get_supported_negotiation_parameters_ipc_req_t;

typedef struct {
  sli_zigbee_stack_zdo_dlk_get_supported_negotiation_parameters_ipc_req_t request;
} sli_zigbee_stack_zdo_dlk_get_supported_negotiation_parameters_ipc_msg_t;

typedef struct {
  sl_zigbee_address_info partner;
  sl_zigbee_dlk_negotiation_method selected_method;
  sl_zigbee_dlk_negotiation_shared_secret_source selected_secret;
} sli_zigbee_stack_zdo_dlk_start_key_negotiation_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_zdo_dlk_start_key_negotiation_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_zdo_dlk_start_key_negotiation_ipc_req_t request;
  sli_zigbee_stack_zdo_dlk_start_key_negotiation_ipc_rsp_t response;
} sli_zigbee_stack_zdo_dlk_start_key_negotiation_ipc_msg_t;

typedef struct {
  sl_zigbee_address_info target;
  sl_zigbee_dlk_negotiation_method selected_method;
  sl_zigbee_dlk_negotiation_shared_secret_source selected_secret;
} sli_zigbee_stack_zdo_dlk_start_key_update_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_zdo_dlk_start_key_update_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_zdo_dlk_start_key_update_ipc_req_t request;
  sli_zigbee_stack_zdo_dlk_start_key_update_ipc_rsp_t response;
} sli_zigbee_stack_zdo_dlk_start_key_update_ipc_msg_t;

typedef struct {
  sli_buffer_manager_buffer_t buffer;
} slxi_zigbee_stack_gu_zdo_dlk_mangle_packet_ipc_req_t;

typedef struct {
  bool result;
} slxi_zigbee_stack_gu_zdo_dlk_mangle_packet_ipc_rsp_t;

typedef struct {
  slxi_zigbee_stack_gu_zdo_dlk_mangle_packet_ipc_req_t request;
  slxi_zigbee_stack_gu_zdo_dlk_mangle_packet_ipc_rsp_t response;
} slxi_zigbee_stack_gu_zdo_dlk_mangle_packet_ipc_msg_t;

typedef struct {
  uint8_t key_buffer[SL_ZIGBEE_ENCRYPTION_KEY_SIZE];
} slxi_zigbee_stack_gu_zdo_dlk_override_psk_fetch_ipc_req_t;

typedef struct {
  bool result;
} slxi_zigbee_stack_gu_zdo_dlk_override_psk_fetch_ipc_rsp_t;

typedef struct {
  slxi_zigbee_stack_gu_zdo_dlk_override_psk_fetch_ipc_req_t request;
  slxi_zigbee_stack_gu_zdo_dlk_override_psk_fetch_ipc_rsp_t response;
} slxi_zigbee_stack_gu_zdo_dlk_override_psk_fetch_ipc_msg_t;

typedef struct {
  sl_zigbee_dlk_supported_negotiation_method method_mask;
  sl_zigbee_dlk_negotiation_supported_shared_secret_source secret_mask;
} slxi_zigbee_stack_gu_zdo_dlk_override_supported_params_ipc_req_t;

typedef struct {
  slxi_zigbee_stack_gu_zdo_dlk_override_supported_params_ipc_req_t request;
} slxi_zigbee_stack_gu_zdo_dlk_override_supported_params_ipc_msg_t;

#endif // SL_ZIGBEE_ZDO_DLK_NEGOTIATION_IPC_COMMAND_MESSAGES_H
