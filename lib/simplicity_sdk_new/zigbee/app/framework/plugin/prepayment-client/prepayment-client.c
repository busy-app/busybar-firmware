/***************************************************************************//**
 * @file
 * @brief Routines for the Prepayment Client plugin, which implements the client
 *        side of the Prepayment cluster.
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

// *****************************************************************************
// * prepayment-client.c
// *
// * Implemented routines for prepayment client.
// *
// * Copyright 2014 by Silicon Laboratories, Inc.
// *****************************************************************************

#include "app/framework/include/af.h"
#include "prepayment-client.h"

#include "zap-cluster-command-parser.h"

void sl_zigbee_af_prepayment_client_change_payment_mode(sl_802154_short_addr_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint, uint32_t providerId, uint32_t issuerEventId, uint32_t implementationDateTime, uint16_t proposedPaymentControlConfiguration, uint32_t cutOffValue)
{
  sl_status_t status;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(srcEndpoint, ZCL_PREPAYMENT_CLUSTER_ID);
  if ( ep == 0xFF ) {
    sl_zigbee_af_app_println("==== NO PREPAYMENT CLIENT ENDPOINT");
    return;
  }

  sl_zigbee_af_fill_command_prepayment_cluster_change_payment_mode(providerId, issuerEventId, implementationDateTime,
                                                                   proposedPaymentControlConfiguration, cutOffValue);
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
  sl_zigbee_af_app_println("=====   SEND PAYMENT MODE stat=0x%02X", status);
}

//-----------------------
// ZCL commands callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_zcl_prepayment_cluster_change_payment_mode_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;

  sl_zigbee_af_app_println("RX: Change Payment Mode Response Callback");
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_publish_prepay_snapshot_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;

  sl_zigbee_af_app_println("RX: Publish Prepay Snapshot Callback");
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_publish_top_up_log_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;

  sl_zigbee_af_app_println("RX: Publish TopUp Log Callback");
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_publish_debt_log_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;

#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
  sl_zcl_prepayment_cluster_publish_debt_log_command_t cmd_data;

  if (zcl_decode_prepayment_cluster_publish_debt_log_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_app_println("RX: Publish Debt Log Callback");
  sl_zigbee_af_prepayment_cluster_println("  commandIndex=%d", cmd_data.commandIndex);
  sl_zigbee_af_prepayment_cluster_println("  totalNumberOfCommands=%d", cmd_data.totalNumberOfCommands);
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

uint32_t sl_zigbee_af_prepayment_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                              sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_PUBLISH_PREPAY_SNAPSHOT_COMMAND_ID:
      {
        status = sl_zigbee_af_prepayment_cluster_publish_prepay_snapshot_cb(cmd);
        break;
      }
      case ZCL_CHANGE_PAYMENT_MODE_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_zcl_prepayment_cluster_change_payment_mode_response_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_TOP_UP_LOG_COMMAND_ID:
      {
        status = sl_zigbee_af_prepayment_cluster_publish_top_up_log_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_DEBT_LOG_COMMAND_ID:
      {
        status = sl_zigbee_af_prepayment_cluster_publish_debt_log_cb(cmd);
        break;
      }
    }
  }

  return status;
}
