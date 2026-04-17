/***************************************************************************//**
 * @file zigbee-dynamic-node-type-ipc-command-messages.h
 * @brief defines structured format for 'zigbee-dynamic-node-type' ipc messages
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
// automatically generated from zigbee-dynamic-node-type.h.  Do not manually edit
#ifndef ZIGBEE_DYNAMIC_NODE_TYPE_IPC_COMMAND_MESSAGES_H
#define ZIGBEE_DYNAMIC_NODE_TYPE_IPC_COMMAND_MESSAGES_H

#include "stack/include/zigbee-dynamic-node-type.h"
#include "stack/internal/inc/zigbee-dynamic-node-type-internal-def.h"

typedef struct {
  sl_status_t result;
} slxi_zigbee_stack_switch_role_router_ipc_rsp_t;

typedef struct {
  slxi_zigbee_stack_switch_role_router_ipc_rsp_t response;
} slxi_zigbee_stack_switch_role_router_ipc_msg_t;

typedef struct {
  uint8_t end_device_configuration;
} slxi_zigbee_stack_switch_role_sleepy_end_device_ipc_req_t;

typedef struct {
  sl_status_t result;
} slxi_zigbee_stack_switch_role_sleepy_end_device_ipc_rsp_t;

typedef struct {
  slxi_zigbee_stack_switch_role_sleepy_end_device_ipc_req_t request;
  slxi_zigbee_stack_switch_role_sleepy_end_device_ipc_rsp_t response;
} slxi_zigbee_stack_switch_role_sleepy_end_device_ipc_msg_t;

#endif // ZIGBEE_DYNAMIC_NODE_TYPE_IPC_COMMAND_MESSAGES_H
