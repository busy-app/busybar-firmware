/***************************************************************************//**
 * @file zigbee_direct_stack_interface_ipc_command_messages.h
 * @brief defines structured format for 'zigbee_direct_stack_interface' ipc messages
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
// automatically generated from zigbee_direct_stack_interface.h.  Do not manually edit
#ifndef ZIGBEE_DIRECT_STACK_INTERFACE_IPC_COMMAND_MESSAGES_H
#define ZIGBEE_DIRECT_STACK_INTERFACE_IPC_COMMAND_MESSAGES_H

#include "stack/include/zigbee_direct_stack_interface.h"
#include "stack/internal/inc/zigbee_direct_stack_interface_internal_def.h"

typedef struct {
  uint8_t status;
  sl_802154_short_addr_t dst;
  sl_802154_long_addr_t longDest;
} sli_zigbee_stack_direct_send_commissioning_response_ipc_req_t;

typedef struct {
  sli_zigbee_stack_direct_send_commissioning_response_ipc_req_t request;
} sli_zigbee_stack_direct_send_commissioning_response_ipc_msg_t;

typedef struct {
  sl_802154_short_addr_t destinationShortId;
  sl_802154_long_addr_t destinationLongId;
  sl_802154_long_addr_t sourceOrPartnerLongId;
  uint8_t keyType;
  sl_zigbee_key_data_t keyData;
  uint8_t options;
} sli_zigbee_stack_direct_send_ephemeral_key_ipc_req_t;

typedef struct {
  bool result;
} sli_zigbee_stack_direct_send_ephemeral_key_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_direct_send_ephemeral_key_ipc_req_t request;
  sli_zigbee_stack_direct_send_ephemeral_key_ipc_rsp_t response;
} sli_zigbee_stack_direct_send_ephemeral_key_ipc_msg_t;

typedef struct {
  uint8_t result;
} sli_zigbee_stack_get_nwk_update_id_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_get_nwk_update_id_ipc_rsp_t response;
} sli_zigbee_stack_get_nwk_update_id_ipc_msg_t;

typedef struct {
  sl_zigbee_address_info device_ids;
} sli_zigbee_stack_transient_device_mgmt_finish_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_transient_device_mgmt_finish_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_transient_device_mgmt_finish_ipc_req_t request;
  sli_zigbee_stack_transient_device_mgmt_finish_ipc_rsp_t response;
} sli_zigbee_stack_transient_device_mgmt_finish_ipc_msg_t;

#endif // ZIGBEE_DIRECT_STACK_INTERFACE_IPC_COMMAND_MESSAGES_H
