/***************************************************************************//**
 * @file sl_zigbee_alternate_mac_ipc_command_messages.h
 * @brief defines structured format for 'sl_zigbee_alternate_mac' ipc messages
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
// automatically generated from sl_zigbee_alternate_mac.h.  Do not manually edit
#ifndef SL_ZIGBEE_ALTERNATE_MAC_IPC_COMMAND_MESSAGES_H
#define SL_ZIGBEE_ALTERNATE_MAC_IPC_COMMAND_MESSAGES_H

#include "stack/include/sl_zigbee_alternate_mac.h"
#include "stack/internal/inc/sl_zigbee_alternate_mac_internal_def.h"

typedef struct {
  sl_zigbee_alt_mac_config_t config;
} sli_zigbee_stack_alt_mac_init_ipc_req_t;

typedef struct {
  sl_zigbee_mac_interface_id_t result;
} sli_zigbee_stack_alt_mac_init_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_alt_mac_init_ipc_req_t request;
  sli_zigbee_stack_alt_mac_init_ipc_rsp_t response;
} sli_zigbee_stack_alt_mac_init_ipc_msg_t;

#endif // SL_ZIGBEE_ALTERNATE_MAC_IPC_COMMAND_MESSAGES_H
