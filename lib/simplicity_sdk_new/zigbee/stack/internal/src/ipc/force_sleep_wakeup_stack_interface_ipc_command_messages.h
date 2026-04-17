/***************************************************************************//**
 * @file force_sleep_wakeup_stack_interface_ipc_command_messages.h
 * @brief defines structured format for 'force_sleep_wakeup_stack_interface' ipc messages
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
// automatically generated from force_sleep_wakeup_stack_interface.h.  Do not manually edit
#ifndef FORCE_SLEEP_WAKEUP_STACK_INTERFACE_IPC_COMMAND_MESSAGES_H
#define FORCE_SLEEP_WAKEUP_STACK_INTERFACE_IPC_COMMAND_MESSAGES_H

#include "stack/include/force_sleep_wakeup_stack_interface.h"
#include "stack/internal/inc/force_sleep_wakeup_stack_interface_internal_def.h"

typedef struct {
  bool sleep;
} sli_mac_stack_lower_mac_force_sleep_ipc_req_t;

typedef struct {
  sli_mac_stack_lower_mac_force_sleep_ipc_req_t request;
} sli_mac_stack_lower_mac_force_sleep_ipc_msg_t;

#endif // FORCE_SLEEP_WAKEUP_STACK_INTERFACE_IPC_COMMAND_MESSAGES_H
