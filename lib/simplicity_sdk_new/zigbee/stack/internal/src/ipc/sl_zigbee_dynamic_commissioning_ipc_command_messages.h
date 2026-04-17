/***************************************************************************//**
 * @file sl_zigbee_dynamic_commissioning_ipc_command_messages.h
 * @brief defines structured format for 'sl_zigbee_dynamic_commissioning' ipc messages
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
// automatically generated from sl_zigbee_dynamic_commissioning.h.  Do not manually edit
#ifndef SL_ZIGBEE_DYNAMIC_COMMISSIONING_IPC_COMMAND_MESSAGES_H
#define SL_ZIGBEE_DYNAMIC_COMMISSIONING_IPC_COMMAND_MESSAGES_H

#include "stack/include/sl_zigbee_dynamic_commissioning.h"
#include "stack/internal/inc/sl_zigbee_dynamic_commissioning_internal_def.h"

typedef struct {
  sl_zigbee_address_info ids;
  sl_zigbee_dynamic_commissioning_event_t event;
} sli_zigbee_stack_device_interview_status_update_ipc_req_t;

typedef struct {
  sli_zigbee_stack_device_interview_status_update_ipc_req_t request;
} sli_zigbee_stack_device_interview_status_update_ipc_msg_t;

typedef struct {
  bool result;
} sli_zigbee_stack_dynamic_commissioning_is_open_for_interview_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_dynamic_commissioning_is_open_for_interview_ipc_rsp_t response;
} sli_zigbee_stack_dynamic_commissioning_is_open_for_interview_ipc_msg_t;

typedef struct {
  bool open_interview;
} sli_zigbee_stack_dynamic_commissioning_set_open_for_interview_ipc_req_t;

typedef struct {
  sli_zigbee_stack_dynamic_commissioning_set_open_for_interview_ipc_req_t request;
} sli_zigbee_stack_dynamic_commissioning_set_open_for_interview_ipc_msg_t;

#endif // SL_ZIGBEE_DYNAMIC_COMMISSIONING_IPC_COMMAND_MESSAGES_H
