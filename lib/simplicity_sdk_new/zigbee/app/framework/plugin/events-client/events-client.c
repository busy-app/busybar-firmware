/***************************************************************************//**
 * @file
 * @brief CLI for the Events plugin, based on the Events cluster.
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

#define LOG_PAYLOAD_CONTROL_MASK          0x0F
#define NUMBER_OF_EVENTS_MASK             0xF0
#define eventCrossesFrameBoundary(lpc)    ((lpc) & EventCrossesFrameBoundary)
#define numberOfEvents(lpc)               (((lpc) & NUMBER_OF_EVENTS_MASK) >> 4)

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "zap-cluster-command-parser.h"

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_events_cluster_publish_event_cb(sl_zigbee_af_cluster_command_t *cmd)
{
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
  sl_zcl_events_cluster_publish_event_command_t cmd_data;

  if (zcl_decode_events_cluster_publish_event_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_events_cluster_print("RX: PublishEvent 0x%02X, 0x%04X, 0x%08X, 0x%02X, 0x%02X",
                                    cmd_data.logId,
                                    cmd_data.eventId,
                                    cmd_data.eventTime,
                                    cmd_data.eventControl,
                                    *cmd_data.eventData);

  uint8_t eventDataLen = sl_zigbee_af_string_length(cmd_data.eventData);
  if (eventDataLen > 0) {
    sl_zigbee_af_events_cluster_print(", ");
    sl_zigbee_af_events_cluster_print_string(cmd_data.eventData);
  }
  sl_zigbee_af_events_cluster_println("");
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_events_cluster_publish_event_log_cb(sl_zigbee_af_cluster_command_t *cmd)
{
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
  sl_zcl_events_cluster_publish_event_log_command_t cmd_data;

  if (zcl_decode_events_cluster_publish_event_log_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_events_cluster_print("RX: PublishEventLog 0x%04X, 0x%02X, 0x%02X, 0x%02X",
                                    cmd_data.totalNumberOfEvents,
                                    cmd_data.commandIndex,
                                    cmd_data.totalCommands,
                                    cmd_data.logPayloadControl);

  uint16_t logPayloadLen = (cmd->bufLen
                            - (cmd->payloadStartIndex
                               + sizeof(cmd_data.totalNumberOfEvents)
                               + sizeof(cmd_data.commandIndex)
                               + sizeof(cmd_data.totalCommands)
                               + sizeof(cmd_data.logPayloadControl)));
  uint16_t logPayloadIndex = 0;
  uint8_t i;
  if (NULL != cmd_data.logPayload) {
    for (i = 0; i < numberOfEvents(cmd_data.logPayloadControl); i++) {
      uint8_t logId;
      uint16_t eventId;
      uint32_t eventTime;
      uint8_t *eventData;
      uint8_t eventDataLen;
      logId = sl_zigbee_af_get_int8u(cmd_data.logPayload, logPayloadIndex, logPayloadLen);
      logPayloadIndex++;
      eventId = sl_zigbee_af_get_int16u(cmd_data.logPayload, logPayloadIndex, logPayloadLen);
      logPayloadIndex += 2;
      eventTime = sl_zigbee_af_get_int32u(cmd_data.logPayload, logPayloadIndex, logPayloadLen);
      logPayloadIndex += 4;
      eventData = cmd_data.logPayload + logPayloadIndex;
      eventDataLen = sl_zigbee_af_get_int8u(cmd_data.logPayload, logPayloadIndex, logPayloadLen);
      logPayloadIndex += (1 + eventDataLen);
      sl_zigbee_af_events_cluster_print(" [");
      sl_zigbee_af_events_cluster_print("0x%02X, 0x%04X, 0x%08X, 0x%02X", logId, eventId, eventTime, eventDataLen);
      if (eventDataLen > 0) {
        sl_zigbee_af_events_cluster_print(", ");
        sl_zigbee_af_events_cluster_print_string(eventData);
      }
      sl_zigbee_af_events_cluster_print("]");
    }
  }
  sl_zigbee_af_events_cluster_println("");
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_events_cluster_clear_event_log_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
  sl_zcl_events_cluster_clear_event_log_response_command_t cmd_data;

  if (zcl_decode_events_cluster_clear_event_log_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_events_cluster_println("RX: ClearEventLogResponse 0x%02X",
                                      cmd_data.clearedEventsLogs);
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

uint32_t sl_zigbee_af_events_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                          sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_PUBLISH_EVENT_COMMAND_ID:
      {
        status = sl_zigbee_af_events_cluster_publish_event_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_EVENT_LOG_COMMAND_ID:
      {
        status = sl_zigbee_af_events_cluster_publish_event_log_cb(cmd);
        break;
      }
      case ZCL_CLEAR_EVENT_LOG_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_events_cluster_clear_event_log_response_cb(cmd);
        break;
      }
    }
  }

  return status;
}
