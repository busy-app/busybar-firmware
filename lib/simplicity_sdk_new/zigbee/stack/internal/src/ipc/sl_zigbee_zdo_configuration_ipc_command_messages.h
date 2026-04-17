/***************************************************************************//**
 * @file sl_zigbee_zdo_configuration_ipc_command_messages.h
 * @brief defines structured format for 'sl_zigbee_zdo_configuration' ipc messages
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
// automatically generated from sl_zigbee_zdo_configuration.h.  Do not manually edit
#ifndef SL_ZIGBEE_ZDO_CONFIGURATION_IPC_COMMAND_MESSAGES_H
#define SL_ZIGBEE_ZDO_CONFIGURATION_IPC_COMMAND_MESSAGES_H

#include "stack/include/sl_zigbee_zdo_configuration.h"
#include "stack/internal/inc/sl_zigbee_zdo_configuration_internal_def.h"

typedef struct {
  sl_802154_short_addr_t device_short;
  bool encrypt;
  uint8_t tag_ids[10];
  uint8_t count;
} sli_zigbee_stack_zdo_get_configuration_req_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_zdo_get_configuration_req_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_zdo_get_configuration_req_ipc_req_t request;
  sli_zigbee_stack_zdo_get_configuration_req_ipc_rsp_t response;
} sli_zigbee_stack_zdo_get_configuration_req_ipc_msg_t;

typedef struct {
  uint8_t tag_id;
  size_t zdo_config_arg_len;
  uint8_t zdo_config_arg[MAX_IPC_VEC_ARG_CAPACITY];
} sli_zigbee_stack_zdo_set_add_configuration_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_zdo_set_add_configuration_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_zdo_set_add_configuration_ipc_req_t request;
  sli_zigbee_stack_zdo_set_add_configuration_ipc_rsp_t response;
} sli_zigbee_stack_zdo_set_add_configuration_ipc_msg_t;

typedef struct {
  sl_802154_short_addr_t device_short;
  bool encrypt;
} sli_zigbee_stack_zdo_set_send_configuration_req_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_zdo_set_send_configuration_req_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_zdo_set_send_configuration_req_ipc_req_t request;
  sli_zigbee_stack_zdo_set_send_configuration_req_ipc_rsp_t response;
} sli_zigbee_stack_zdo_set_send_configuration_req_ipc_msg_t;

#endif // SL_ZIGBEE_ZDO_CONFIGURATION_IPC_COMMAND_MESSAGES_H
