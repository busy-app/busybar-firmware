/***************************************************************************//**
 * @file sl_zigbee_zdo_management_ipc_command_messages.c
 * @brief internal wrappers for 'sl_zigbee_zdo_management' ipc commands
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
// automatically generated from sl_zigbee_zdo_management.h.  Do not manually edit
#include "stack/include/sl_zigbee_zdo_management.h"
#include "stack/internal/inc/sl_zigbee_zdo_management_internal_def.h"
#include "stack/internal/src/ipc/sl_zigbee_zdo_management_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_request_beacon_survey_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.request_beacon_survey.response.result = sli_zigbee_stack_request_beacon_survey(msg->data.request_beacon_survey.request.dest,
                                                                                           msg->data.request_beacon_survey.request.channel_page_count,
                                                                                           msg->data.request_beacon_survey.request.channel_masks,
                                                                                           msg->data.request_beacon_survey.request.scan_config_mask);
}

// public entrypoints

sl_status_t sl_zigbee_request_beacon_survey(sl_802154_short_addr_t dest,
                                            uint8_t channel_page_count,
                                            uint32_t *channel_masks,
                                            uint8_t scan_config_mask)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.request_beacon_survey.request.dest = dest;
  msg.data.request_beacon_survey.request.channel_page_count = channel_page_count;

  if (channel_page_count > 5) {
    assert(false); // "vector channel_masks length exceeds expected maximum
  }

  memmove(msg.data.request_beacon_survey.request.channel_masks, channel_masks, sizeof(uint32_t) * channel_page_count);
  msg.data.request_beacon_survey.request.scan_config_mask = scan_config_mask;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_request_beacon_survey_process_ipc_command, &msg);

  if (channel_page_count > 5) {
    assert(false); // "vector channel_masks length exceeds expected maximum
  }

  memmove(channel_masks, msg.data.request_beacon_survey.request.channel_masks, sizeof(uint32_t) * channel_page_count);
  return msg.data.request_beacon_survey.response.result;
}
