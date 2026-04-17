/***************************************************************************//**
 * @file multi-phy-ipc-command-messages.c
 * @brief internal wrappers for 'multi-phy' ipc commands
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
// automatically generated from multi-phy.h.  Do not manually edit
#include "stack/include/multi-phy.h"
#include "stack/internal/inc/multi-phy-internal-def.h"
#include "stack/internal/src/ipc/multi-phy-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_multi_phy_set_radio_channel_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.multi_phy_set_radio_channel.response.result = sli_zigbee_stack_multi_phy_set_radio_channel(msg->data.multi_phy_set_radio_channel.request.phyIndex,
                                                                                                       msg->data.multi_phy_set_radio_channel.request.page,
                                                                                                       msg->data.multi_phy_set_radio_channel.request.channel);
}

void sli_zigbee_stack_multi_phy_set_radio_power_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.multi_phy_set_radio_power.response.result = sli_zigbee_stack_multi_phy_set_radio_power(msg->data.multi_phy_set_radio_power.request.phyIndex,
                                                                                                   msg->data.multi_phy_set_radio_power.request.power);
}

void sli_zigbee_stack_multi_phy_start_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.multi_phy_start.response.result = sli_zigbee_stack_multi_phy_start(msg->data.multi_phy_start.request.phyIndex,
                                                                               msg->data.multi_phy_start.request.page,
                                                                               msg->data.multi_phy_start.request.channel,
                                                                               msg->data.multi_phy_start.request.power,
                                                                               msg->data.multi_phy_start.request.bitmask);
}

void sli_zigbee_stack_multi_phy_stop_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.multi_phy_stop.response.result = sli_zigbee_stack_multi_phy_stop(msg->data.multi_phy_stop.request.phyIndex);
}

// public entrypoints

sl_status_t sl_zigbee_multi_phy_set_radio_channel(uint8_t phyIndex,
                                                  uint8_t page,
                                                  uint8_t channel)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.multi_phy_set_radio_channel.request.phyIndex = phyIndex;
  msg.data.multi_phy_set_radio_channel.request.page = page;
  msg.data.multi_phy_set_radio_channel.request.channel = channel;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_multi_phy_set_radio_channel_process_ipc_command, &msg);

  return msg.data.multi_phy_set_radio_channel.response.result;
}

sl_status_t sl_zigbee_multi_phy_set_radio_power(uint8_t phyIndex,
                                                int8_t power)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.multi_phy_set_radio_power.request.phyIndex = phyIndex;
  msg.data.multi_phy_set_radio_power.request.power = power;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_multi_phy_set_radio_power_process_ipc_command, &msg);

  return msg.data.multi_phy_set_radio_power.response.result;
}

sl_status_t sl_zigbee_multi_phy_start(uint8_t phyIndex,
                                      uint8_t page,
                                      uint8_t channel,
                                      int8_t power,
                                      sl_zigbee_multi_phy_nwk_config_t bitmask)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.multi_phy_start.request.phyIndex = phyIndex;
  msg.data.multi_phy_start.request.page = page;
  msg.data.multi_phy_start.request.channel = channel;
  msg.data.multi_phy_start.request.power = power;
  msg.data.multi_phy_start.request.bitmask = bitmask;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_multi_phy_start_process_ipc_command, &msg);

  return msg.data.multi_phy_start.response.result;
}

sl_status_t sl_zigbee_multi_phy_stop(uint8_t phyIndex)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.multi_phy_stop.request.phyIndex = phyIndex;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_multi_phy_stop_process_ipc_command, &msg);

  return msg.data.multi_phy_stop.response.result;
}
