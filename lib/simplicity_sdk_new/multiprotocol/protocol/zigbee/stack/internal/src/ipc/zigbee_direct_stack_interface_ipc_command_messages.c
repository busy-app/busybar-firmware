/***************************************************************************//**
 * @file zigbee_direct_stack_interface_ipc_command_messages.c
 * @brief internal wrappers for 'zigbee_direct_stack_interface' ipc commands
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
#include "stack/include/zigbee_direct_stack_interface.h"
#include "stack/internal/inc/zigbee_direct_stack_interface_internal_def.h"
#include "stack/internal/src/ipc/zigbee_direct_stack_interface_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_direct_send_commissioning_response_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_direct_send_commissioning_response(msg->data.direct_send_commissioning_response.request.status,
                                                      msg->data.direct_send_commissioning_response.request.dst,
                                                      msg->data.direct_send_commissioning_response.request.longDest);
}

void sli_zigbee_stack_direct_send_ephemeral_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.direct_send_ephemeral_key.response.result = sli_zigbee_stack_direct_send_ephemeral_key(msg->data.direct_send_ephemeral_key.request.destinationShortId,
                                                                                                   msg->data.direct_send_ephemeral_key.request.destinationLongId,
                                                                                                   msg->data.direct_send_ephemeral_key.request.sourceOrPartnerLongId,
                                                                                                   msg->data.direct_send_ephemeral_key.request.keyType,
                                                                                                   &msg->data.direct_send_ephemeral_key.request.keyData,
                                                                                                   msg->data.direct_send_ephemeral_key.request.options);
}

void sli_zigbee_stack_get_nwk_update_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_nwk_update_id.response.result = sli_zigbee_stack_get_nwk_update_id();
}

void sli_zigbee_stack_transient_device_mgmt_finish_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.transient_device_mgmt_finish.response.result = sli_zigbee_stack_transient_device_mgmt_finish(&msg->data.transient_device_mgmt_finish.request.device_ids);
}

// public entrypoints

void sl_zigbee_direct_send_commissioning_response(uint8_t status,
                                                  sl_802154_short_addr_t dst,
                                                  sl_802154_long_addr_t longDest)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.direct_send_commissioning_response.request.status = status;
  msg.data.direct_send_commissioning_response.request.dst = dst;

  if (longDest != NULL) {
    memmove(msg.data.direct_send_commissioning_response.request.longDest, longDest, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_direct_send_commissioning_response_process_ipc_command, &msg);

  if (longDest != NULL) {
    memmove(longDest, msg.data.direct_send_commissioning_response.request.longDest, sizeof(sl_802154_long_addr_t));
  }
}

bool sl_zigbee_direct_send_ephemeral_key(sl_802154_short_addr_t destinationShortId,
                                         sl_802154_long_addr_t destinationLongId,
                                         sl_802154_long_addr_t sourceOrPartnerLongId,
                                         uint8_t keyType,
                                         const sl_zigbee_key_data_t *keyData,
                                         uint8_t options)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.direct_send_ephemeral_key.request.destinationShortId = destinationShortId;

  if (destinationLongId != NULL) {
    memmove(msg.data.direct_send_ephemeral_key.request.destinationLongId, destinationLongId, sizeof(sl_802154_long_addr_t));
  }

  if (sourceOrPartnerLongId != NULL) {
    memmove(msg.data.direct_send_ephemeral_key.request.sourceOrPartnerLongId, sourceOrPartnerLongId, sizeof(sl_802154_long_addr_t));
  }

  msg.data.direct_send_ephemeral_key.request.keyType = keyType;

  if (keyData != NULL) {
    msg.data.direct_send_ephemeral_key.request.keyData = *keyData;
  }

  msg.data.direct_send_ephemeral_key.request.options = options;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_direct_send_ephemeral_key_process_ipc_command, &msg);

  if (destinationLongId != NULL) {
    memmove(destinationLongId, msg.data.direct_send_ephemeral_key.request.destinationLongId, sizeof(sl_802154_long_addr_t));
  }

  if (sourceOrPartnerLongId != NULL) {
    memmove(sourceOrPartnerLongId, msg.data.direct_send_ephemeral_key.request.sourceOrPartnerLongId, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.direct_send_ephemeral_key.response.result;
}

uint8_t sl_zigbee_get_nwk_update_id(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_nwk_update_id_process_ipc_command, &msg);

  return msg.data.get_nwk_update_id.response.result;
}

sl_status_t sl_zigbee_transient_device_mgmt_finish(const sl_zigbee_address_info *device_ids)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (device_ids != NULL) {
    msg.data.transient_device_mgmt_finish.request.device_ids = *device_ids;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_transient_device_mgmt_finish_process_ipc_command, &msg);

  return msg.data.transient_device_mgmt_finish.response.result;
}
