/***************************************************************************//**
 * @file library_ipc_command_messages.c
 * @brief internal wrappers for 'library' ipc commands
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
// automatically generated from library.h.  Do not manually edit
#include "stack/include/library.h"
#include "stack/internal/inc/library_internal_def.h"
#include "stack/internal/src/ipc/library_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_get_library_status_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_library_status.response.result = sli_zigbee_stack_get_library_status(msg->data.get_library_status.request.libraryId);
}

// public entrypoints

sl_zigbee_library_status_t sl_zigbee_get_library_status(sl_zigbee_library_id_t libraryId)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_library_status.request.libraryId = libraryId;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_library_status_process_ipc_command, &msg);

  return msg.data.get_library_status.response.result;
}
