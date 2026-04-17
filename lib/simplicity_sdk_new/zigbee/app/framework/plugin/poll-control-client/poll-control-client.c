/***************************************************************************//**
 * @file
 * @brief Routines for the Poll Control Client plugin, which implement the
 *        client side of the Poll Control cluster. The Poll Control cluster
 *        provides a means to communicate with an end device with a sleep
 *        schedule.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "app/framework/include/af.h"
#include "poll-control-client.h"

#include "poll-control-client-config.h"
#include "zap-cluster-command-parser.h"

static bool fastPolling = false;
static bool respondToCheckIn = true;
static uint16_t fastPollingTimeout = SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_CLIENT_DEFAULT_FAST_POLL_TIMEOUT;

void sli_zigbee_af_set_fast_polling_mode(bool mode)
{
  fastPolling = mode;
}

void sli_zigbee_af_set_fast_polling_timeout(uint16_t timeout)
{
  fastPollingTimeout = timeout;
}

void sli_zigbee_af_set_response_mode(bool mode)
{
  respondToCheckIn = mode;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_poll_control_cluster_check_in_cb(void)
{
  sl_zigbee_af_poll_control_cluster_println("RX: CheckIn");
  if (respondToCheckIn) {
    sl_zigbee_af_fill_command_poll_control_cluster_check_in_response(fastPolling,
                                                                     fastPollingTimeout);
    sl_zigbee_af_send_response();
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

void sli_zigbee_af_poll_control_client_print(void)
{
  sl_zigbee_af_poll_control_cluster_println("Poll Control Client:\n%s %s\n%s 0x%04X",
                                            "fast polling: ",
                                            fastPolling ? "on" : "off",
                                            "fast polling timeout: ",
                                            fastPollingTimeout);
}

uint32_t sl_zigbee_af_poll_control_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                                sl_service_function_context_t *context)
{
  (void)opcode;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  if (!cmd->mfgSpecific && cmd->commandId == ZCL_CHECK_IN_COMMAND_ID) {
    status = sl_zigbee_af_poll_control_cluster_check_in_cb();
  }

  return status;
}
