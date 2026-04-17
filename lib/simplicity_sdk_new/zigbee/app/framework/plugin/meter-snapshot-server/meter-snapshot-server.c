/***************************************************************************//**
 * @file
 * @brief Code to handle meter snapshot server behavior.
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
#include "app/framework/util/common.h"
#include "meter-snapshot-server.h"

#include "zap-cluster-command-parser.h"

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_schedule_snapshot_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_schedule_snapshot_command_t cmd_data;

  if (zcl_decode_simple_metering_cluster_schedule_snapshot_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }
  uint8_t responsePayload[2];

  // Attempt to schedule the snapshot
  sl_zigbee_af_meter_snapshot_server_schedule_snapshot_cb(cmd->apsFrame->destinationEndpoint,
                                                          cmd->apsFrame->sourceEndpoint,
                                                          cmd->source,
                                                          cmd_data.snapshotSchedulePayload,
                                                          (uint8_t *)responsePayload);

  sl_zigbee_af_fill_command_simple_metering_cluster_schedule_snapshot_response(cmd_data.issuerEventId,
                                                                               responsePayload,
                                                                               2);

  sl_zigbee_af_send_response();
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_take_snapshot_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_take_snapshot_command_t cmd_data;
  if (zcl_decode_simple_metering_cluster_take_snapshot_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t snapshotConfirmation;
  uint32_t snapshotId;

  // Attempt to take the snapshot
  snapshotId = sl_zigbee_af_meter_snapshot_server_take_snapshot_cb(endpoint,
                                                                   cmd_data.snapshotCause,
                                                                   &snapshotConfirmation);

  sl_zigbee_af_fill_command_simple_metering_cluster_take_snapshot_response(snapshotId,
                                                                           snapshotConfirmation);
  sl_zigbee_af_send_response();
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_get_snapshot_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_get_snapshot_command_t cmd_data;
  if (zcl_decode_simple_metering_cluster_get_snapshot_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint8_t snapshotCriteria[13];

  // Package the snapshot criteria for our callback to process
  sl_zigbee_af_copy_int32u((uint8_t *)snapshotCriteria, 0, cmd_data.earliestStartTime);
  sl_zigbee_af_copy_int32u((uint8_t *)snapshotCriteria, 4, cmd_data.latestEndTime);
  snapshotCriteria[8] = cmd_data.snapshotOffset;
  sl_zigbee_af_copy_int32u((uint8_t *)snapshotCriteria, 9, cmd_data.snapshotCause);
  sl_zigbee_af_core_println("snapshotCause %u", cmd_data.snapshotCause);
  sl_zigbee_af_core_println("snapshotCause %u", cmd_data.snapshotCause);

  sl_zigbee_af_core_println("Start Time %u Endpoint %u snapshot Offset %u SnapShotCause %u", cmd_data.earliestStartTime, cmd_data.latestEndTime, cmd_data.snapshotOffset, cmd_data.snapshotCause);
  // Get / publish the snapshot
  sl_zigbee_af_meter_snapshot_server_get_snapshot_cb(cmd->apsFrame->destinationEndpoint,
                                                     cmd->apsFrame->sourceEndpoint,
                                                     cmd->source,
                                                     (uint8_t *)snapshotCriteria);

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

uint32_t sl_zigbee_af_simple_metering_cluster_server_snapshot_command_parse(sl_service_opcode_t opcode,
                                                                            sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_TAKE_SNAPSHOT_COMMAND_ID:
        status = sl_zigbee_af_simple_metering_cluster_take_snapshot_cb(cmd);
        break;
      case ZCL_GET_SNAPSHOT_COMMAND_ID:
        status = sl_zigbee_af_simple_metering_cluster_get_snapshot_cb(cmd);
        break;
      case ZCL_SCHEDULE_SNAPSHOT_COMMAND_ID:
        status = sl_zigbee_af_simple_metering_cluster_schedule_snapshot_cb(cmd);
        break;
      default:
        break;
    }
  }

  return status;
}
