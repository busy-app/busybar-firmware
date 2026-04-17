/***************************************************************************//**
 * @file
 * @brief Routines for the Gas Proxy Function plugin.
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

#include "app/framework/plugin/gbz-message-controller/gbz-message-controller.h"
#include "app/framework/plugin/gas-proxy-function/gas-proxy-function.h"
#include "app/framework/plugin/price-server/price-server.h"
#include "app/framework/plugin/messaging-server/messaging-server.h"
#include "app/framework/plugin/calendar-common/calendar-common.h"
#include "app/framework/plugin/device-management-server/device-management-server.h"
#include "app/framework/plugin/meter-mirror/meter-mirror.h"
#include "gpf-structured-data.h"
#include "app/framework/plugin/events-server/events-server.h"

#include "zap-cluster-command-parser.h"

extern sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_get_notified_message_cb(sl_zigbee_af_cluster_command_t *cmd);
extern sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_get_sampled_data_cb(sl_zigbee_af_cluster_command_t *cmd);
extern sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_publish_snapshot_cb(sl_zigbee_af_cluster_command_t *cmd);
extern sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_get_snapshot_cb(sl_zigbee_af_cluster_command_t *cmd);
extern sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_get_sampled_data_response_cb(sl_zigbee_af_cluster_command_t *cmd);
extern sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_publish_prepay_snapshot_cb(sl_zigbee_af_cluster_command_t *cmd);
extern sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_get_prepay_snapshot_cb(sl_zigbee_af_cluster_command_t *cmd);
extern sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_publish_top_up_log_cb(sl_zigbee_af_cluster_command_t *cmd);
extern sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_get_top_up_log_cb(sl_zigbee_af_cluster_command_t *cmd);
extern sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_publish_debt_log_cb(sl_zigbee_af_cluster_command_t *cmd);
extern sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_get_debt_repayment_log_cb(sl_zigbee_af_cluster_command_t *cmd);

// default configurations
#define DEFAULT_TABLE_SET_INDEX (0)
static bool nonTomHandlingActive = false;
static sl_zigbee_af_gbz_message_parser_state_t nonTomGbzRequestParser;
static sl_zigbee_af_gbz_message_creator_state_t nonTomGbzResponseCreator;
static uint8_t nonTomExpectedZclResps = 0;  // per command
static uint8_t nonTomResponsesCount = 0;    // per command
static uint32_t nonTomGbzStartTime = 0;
static sl_zigbee_af_gpf_message_t currentGpfMessage;

static void hideEndpoint(uint8_t endpoint)
{
#ifdef EZSP_HOST
  sl_status_t status = sl_zigbee_ezsp_set_endpoint_flags(endpoint,
                                                         SL_ZIGBEE_EZSP_ENDPOINT_DISABLED);
  sl_zigbee_af_gas_proxy_function_println("GPF: hiding endpoint status: 0x%02X", status);
#else
  // we need to standarize disabling/hiding/reset endpoints.
  UNUSED_VAR(endpoint);
  sl_zigbee_af_gas_proxy_function_println("GPF: hiding endpoint not supported on Soc!");
#endif
}

sl_zigbee_af_event_t sl_zigbee_af_gas_proxy_function_gsme_sync_endpoint_events[ZCL_FIXED_ENDPOINT_COUNT];
sl_zigbee_af_event_t sl_zigbee_af_gas_proxy_function_catchup_event;

extern void sl_zigbee_af_gas_proxy_function_gsme_sync_endpoint_event_handler(uint8_t endpoint);
extern void sl_zigbee_af_gas_proxy_function_catchup_event_handler(sl_zigbee_af_event_t * event);

void sl_zigbee_af_gas_proxy_function_init_cb(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
      uint8_t endpoint_array[ZCL_FIXED_ENDPOINT_COUNT] = ZCL_FIXED_ENDPOINT_ARRAY;
      uint8_t i;

      for (i = 0; i < ZCL_FIXED_ENDPOINT_COUNT; i++) {
        sl_zigbee_af_endpoint_event_init(&sl_zigbee_af_gas_proxy_function_gsme_sync_endpoint_events[i],
                                         sl_zigbee_af_gas_proxy_function_gsme_sync_endpoint_event_handler,
                                         endpoint_array[i]);
      }

      sl_zigbee_af_event_init(&sl_zigbee_af_gas_proxy_function_catchup_event,
                              sl_zigbee_af_gas_proxy_function_catchup_event_handler);
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA:
    {
      sl_zigbee_af_gas_proxy_function_init_structured_data();
      memset(&currentGpfMessage, 0x00, sizeof(sl_zigbee_af_gpf_message_t));
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_DONE:
    {
      // try to hide hidden endpoint from service discovery.
      hideEndpoint(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT);
      break;
    }
  }
}

/*  @brief  Log all GPF commands to GPF Event Log.
 *
 *  In GBCS v0.81, the GPF shall be capable of logging all Commands
 *  received and Outcomes in the GPF Event Log(4.6.3.8). If the message is
 *  too long, the data will be truncated.
 *  */
static void log_event(uint8_t * gbzCmd,
                      uint16_t  gbzCmdLen,
                      uint8_t * gbzResp,
                      uint16_t  gbzRespLen,
                      uint16_t eventId,
                      sl_zigbee_af_g_p_f_message_type_t cmdType,
                      uint16_t messageCode)
{
  UNUSED_VAR(gbzCmd);
  UNUSED_VAR(gbzCmdLen);
  UNUSED_VAR(gbzResp);
  UNUSED_VAR(gbzRespLen);
  UNUSED_VAR(cmdType);

  sl_zigbee_af_zcl_event_t event;
  uint8_t * logMsg = event.eventData;
  uint8_t endpoint;

  logMsg[0] = 0x00;

  // EMAPPFWKV2-1315 - "For any Event Log entries relating to Event Codes
  // 0x0054 and 0x0055, the Device shall record the Commands received on the
  // Network Interface by including the Message Code in the Event Log"
  if (eventId == GBCS_EVENT_ID_IMM_HAN_CMD_RXED_ACTED
      || eventId == GBCS_EVENT_ID_IMM_HAN_CMD_RXED_NOT_ACTED) {
    logMsg[0] = 0x02;
    logMsg[1] = HIGH_BYTE(messageCode);
    logMsg[2] = LOW_BYTE(messageCode);
  }

  event.eventId = eventId;
  event.eventTime = sl_zigbee_af_get_current_time();

  if ((messageCode == GCS01a_MESSAGE_CODE)
      || (messageCode == GCS01b_MESSAGE_CODE)
      || (messageCode == GCS05_MESSAGE_CODE)
      || (messageCode == GCS07_MESSAGE_CODE)
      || (messageCode == GCS23_MESSAGE_CODE)
      || (messageCode == GCS25_MESSAGE_CODE)
      || (messageCode == GCS44_MESSAGE_CODE)
      ) {
    endpoint = SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT;
  } else {
    endpoint = SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT;
  }
  sl_zigbee_af_events_server_add_event(endpoint,
                                       SL_ZIGBEE_ZCL_EVENT_LOG_ID_GENERAL_EVENT_LOG,
                                       &event);
}

static sl_status_t sl_zigbee_af_gas_proxy_function_replay_zcl_command(uint8_t srcEndpoint,
                                                                      uint8_t dstEndpoint,
                                                                      sl_zigbee_af_gbz_zcl_command_t * cmd)
{
  sl_802154_short_addr_t dstId = sl_zigbee_af_get_node_id();
  sl_status_t status;

  sl_zigbee_af_gas_proxy_function_println("GPF: Replaying following command from endpoint(%d) to (%d)", srcEndpoint, dstEndpoint);
  sl_zigbee_af_gbz_message_controller_print_command_info(cmd);

  if (dstEndpoint == SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT
      && !sl_zigbee_af_is_device_enabled(dstEndpoint)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: Cannot replay command. Mirror is not available.");
    (void) sl_zigbee_af_fill_external_buffer(((cmd->direction == ZCL_DIRECTION_SERVER_TO_CLIENT)
                                              ? ZCL_FRAME_CONTROL_CLIENT_TO_SERVER : ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),
                                             cmd->clusterId,                  \
                                             ZCL_DEFAULT_RESPONSE_COMMAND_ID, \
                                             "uu",                            \
                                             cmd->commandId,                  \
                                             SL_ZIGBEE_ZCL_STATUS_FAILURE);
    sl_zigbee_af_set_command_endpoints(srcEndpoint, srcEndpoint);
  } else {
    (void) sl_zigbee_af_fill_external_buffer(cmd->frameControl, \
                                             cmd->clusterId,    \
                                             cmd->commandId,    \
                                             "");
    sl_zigbee_af_append_to_external_buffer(cmd->payload, cmd->payloadLength);
    sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  }

  appResponseData[1] = cmd->transactionSequenceNumber;

  // We assume that one response will be sent per command.  If more than one
  // response is to be received the expected response count will be updated.
  // Refer to sl_zigbee_af_calendar_server_publish_info_cb.
  nonTomExpectedZclResps = 1;
  nonTomResponsesCount = 0;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstId);
  return status;
}

/* @brief check if a GBZ ZCL command matches the corresponding command
 * response
 *
 * return true if pair matches. Otherwise, false.
 * */
static bool matchingZclCommands(sl_zigbee_af_gbz_zcl_command_t * cmd, sl_zigbee_af_gbz_zcl_command_t  * cmdResp)
{
  bool result = true;

  // check command direction
  if ((cmd->frameControl & ZCL_FRAME_CONTROL_DIRECTION_MASK) == (cmdResp->frameControl & ZCL_FRAME_CONTROL_DIRECTION_MASK)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: No matching ZCL command direction (0x%02X/0x%02X) in ZCL cmd/cmd resp pair!",
                                            cmd->frameControl & ZCL_FRAME_CONTROL_DIRECTION_MASK,
                                            cmdResp->frameControl & ZCL_FRAME_CONTROL_DIRECTION_MASK);
    result = false;
  }

  if (cmd->transactionSequenceNumber != cmdResp->transactionSequenceNumber) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: No matching ZCL tran seq number (0x%02X/0x%02X) in ZCL cmd/cmd resp pair!", cmd->transactionSequenceNumber, cmdResp->transactionSequenceNumber);
    result = false;
  }

  if (cmd->clusterId != cmdResp->clusterId) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: No matching ZCL cluster id (%04X/%04X) in ZCL cmd/cmd resp pair!", cmd->clusterId, cmdResp->clusterId);
    result = false;
  }

  // the command id doesn't always matches in value (e.g. read attr(0x00) vs read attr resp (0x01)).
  // this is not being checked for now.
  /*if (cmd->commandId != cmdResp->commandId) {*/
  /*sl_zigbee_af_gas_proxy_function_println("GPF: ERR: No matching ZCL command id (%02X/%02X) in ZCL cmd/cmd resp pair!", cmd->commandId, cmdResp->commandId);*/
  /*result = false;*/
  /*}*/

  return result;
}

/*
 * @brief Send a Tap Off Message (TOM) to the GPF
 *
 * This function can be used to send a Tap Off Message (TOM) to the Gas
 * Proxy Function (GPF) for processing. The message has been tapped off
 * and validated by the gas meter (GSME), so that now it can be applied to the GPF.
 *
 * @param gbzCommands  a pointer to the complete GBZ command data.
 * @param gbzCommandsLength the length of the GBZ command.
 * @param gbzCommandsResponse a pointer to all GBZ responses generated by the GSME.
 * @param gbzCommandsResponseLength the length of the GBZ responses.
 *
 * @return SL_STATUS_OK if all messages have been successfully parsed, a non-zero
 *   status code on error.
 */
sl_status_t sl_zigbee_af_gas_proxy_function_tap_off_message_handler(uint8_t * gbzCommands,
                                                                    uint16_t  gbzCommandsLength,
                                                                    uint8_t * gbzCommandsResponse,
                                                                    uint16_t  gbzCommandsResponseLength,
                                                                    uint16_t  messageCode)
{
  sl_zigbee_af_gbz_message_parser_state_t gbzCommandHandler;
  sl_zigbee_af_gbz_message_parser_state_t gbzCommandResponseHandler;
  sl_status_t status;

  sl_zigbee_af_gbz_message_controller_parser_init(&gbzCommandHandler,
                                                  SL_ZIGBEE_AF_GBZ_MESSAGE_COMMAND,
                                                  gbzCommands,
                                                  gbzCommandsLength,
                                                  false,
                                                  messageCode);
  sl_zigbee_af_gbz_message_controller_parser_init(&gbzCommandResponseHandler,
                                                  SL_ZIGBEE_AF_GBZ_MESSAGE_RESPONSE,
                                                  gbzCommandsResponse,
                                                  gbzCommandsResponseLength,
                                                  false,
                                                  messageCode);

  if (sl_zigbee_af_gbz_message_controller_get_component_size(&gbzCommandHandler)
      != sl_zigbee_af_gbz_message_controller_get_component_size(&gbzCommandResponseHandler)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: GBZ command / command response does not have same number of components!");
    status = SL_STATUS_INVALID_PARAMETER;
    goto kickout;
  }

  currentGpfMessage.gbzCommands = gbzCommands;
  currentGpfMessage.gbzCommandsLength = gbzCommandsLength;
  currentGpfMessage.gbzCommandsResponse = gbzCommandsResponse;
  currentGpfMessage.gbzCommandsResponseLength = gbzCommandsResponseLength;
  currentGpfMessage.messageCode = messageCode;

  while (sl_zigbee_af_gbz_message_controller_has_next_command(&gbzCommandHandler)) {
    sl_zigbee_af_gbz_zcl_command_t cmd;
    sl_zigbee_af_gbz_zcl_command_t cmdResp;
    sl_zigbee_af_gbz_message_controller_next_command(&gbzCommandHandler, &cmd);
    sl_zigbee_af_gbz_message_controller_next_command(&gbzCommandResponseHandler, &cmdResp);

    if (matchingZclCommands(&cmd, &cmdResp)
        && sl_zigbee_af_gbz_message_controller_get_zcl_default_response(&cmdResp) == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_aps_frame_t apsFrame;
      sl_zigbee_af_cluster_command_t clusterCmd;
      memset(&apsFrame, 0x00, sizeof(sl_zigbee_aps_frame_t));
      memset(&clusterCmd, 0x00, sizeof(sl_zigbee_af_cluster_command_t));
      sl_zigbee_af_gas_proxy_function_println("GPF: Updating ESI endpoint with following info: ");
      sl_zigbee_af_gbz_message_controller_print_command_info(&cmd);

      apsFrame.profileId = gbzCommandHandler.profileId;
      apsFrame.clusterId = cmd.clusterId;
      clusterCmd.commandId = cmd.commandId;

      // we mangle the zcl dst ep to a proxy endpoint to intercept the server to client side
      // commands. however, certain commands (Events: ClearEventLogRequest)
      // doesn't need to be redirected to the proxy ep thanks to the correct
      // zcl direction.
      if ((cmd.clusterId == ZCL_EVENTS_CLUSTER_ID)
          && (cmd.commandId == ZCL_CLEAR_EVENT_LOG_REQUEST_COMMAND_ID)) {
        // this TOM cmd is directed at the GSME
        apsFrame.destinationEndpoint = (SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT);
      } else {
        apsFrame.destinationEndpoint = (SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT);
      }

      clusterCmd.apsFrame = &apsFrame;
      clusterCmd.seqNum = cmd.transactionSequenceNumber;
      clusterCmd.clusterSpecific = cmd.clusterSpecific;
      clusterCmd.mfgSpecific = cmd.mfgSpecific;
      clusterCmd.direction = cmd.direction;
      clusterCmd.buffer = cmd.payload;
      clusterCmd.bufLen = cmd.payloadLength;
      clusterCmd.payloadStartIndex = 0;

      status = sl_zigbee_af_gas_proxy_function_validate_incoming_zcl_command_cb(&clusterCmd,
                                                                                messageCode);
      if (status == SL_ZIGBEE_AF_GPF_ZCL_COMMAND_PERMISSION_ALLOWED) {
        sl_zigbee_af_zcl_request_status_t parsingStatus;
        sl_zigbee_af_current_command() = &clusterCmd;
        sl_zigbee_af_gas_proxy_function_println("GPF: Passing cmd to cluster parser");
        parsingStatus = sl_zigbee_af_cluster_specific_command_parse(&clusterCmd);
        if (parsingStatus != SL_ZIGBEE_ZCL_STATUS_SUCCESS
            && parsingStatus != SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED) {
          sl_zigbee_af_gas_proxy_function_println("GPF: ERR: Unable to apply ZCL command (error: 0x%02X)!", parsingStatus);
          status = SL_STATUS_FAIL;
          goto kickout;
        }
      } else if (status == SL_ZIGBEE_AF_GPF_ZCL_COMMAND_PERMISSION_IGNORED) {
        sl_zigbee_af_gas_proxy_function_println("GPF: Info: Command ignored: ZCL command(clus 0x%04X, cmd 0x%02X) embedded within Tap Off Message.", clusterCmd.apsFrame->clusterId, clusterCmd.commandId);
      } else {
        // only SL_ZIGBEE_AF_GPF_ZCL_COMMAND_PERMISSION_NOT_ALLOWED is expected to be
        // here.
        sl_zigbee_af_gas_proxy_function_println("GPF: ERR: Unsupported ZCL command(clus 0x%04X, cmd 0x%02X) embedded within Tap Off Message!", clusterCmd.apsFrame->clusterId, clusterCmd.commandId);
        status = SL_STATUS_FAIL;
        goto kickout;
      }
    } else {
      sl_zigbee_af_gas_proxy_function_println("GPF: ERR: Unable to process ZCL cmd/resp pair with trans seq number (%d)",
                                              cmd.transactionSequenceNumber);
      status = SL_STATUS_FAIL;
      goto kickout;
    }
  }

  status = SL_STATUS_OK;

  kickout:

  sl_zigbee_af_gbz_message_controller_parser_cleanup(&gbzCommandHandler);
  sl_zigbee_af_gbz_message_controller_parser_cleanup(&gbzCommandResponseHandler);

  memset(&currentGpfMessage, 0x00, sizeof(sl_zigbee_af_gpf_message_t));

  if (status == SL_STATUS_OK) {
    sl_zigbee_af_gas_proxy_function_println("GPF: TOM message has been successfully processed and will be logged to Event cluster on ep(%d).",
                                            SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT);
  }

  // time to log these lovely TOM commands to the Event log.
  log_event(gbzCommands,
            gbzCommandsLength,
            gbzCommandsResponse,
            gbzCommandsResponseLength,
            (status == SL_STATUS_OK)
            ?  GBCS_EVENT_ID_IMM_HAN_CMD_RXED_ACTED
            : GBCS_EVENT_ID_IMM_HAN_CMD_RXED_NOT_ACTED,
            SL_ZIGBEE_AF_GPF_MESSAGE_TYPE_TOM,
            messageCode);

  sl_zigbee_af_current_command() = NULL;
  return status;
}

static bool sendNextNonTomZclCmd(void)
{
  sl_zigbee_af_status_t status;
  sl_zigbee_af_gbz_zcl_command_t cmd;

  if (!sl_zigbee_af_gbz_message_controller_has_next_command(&nonTomGbzRequestParser)) {
    return false;
  }

  sl_zigbee_af_gbz_message_controller_next_command(&nonTomGbzRequestParser, &cmd);
  nonTomGbzStartTime = (cmd.hasFromDateTime) ? cmd.fromDateTime : 0;
  if (cmd.clusterId == ZCL_PRICE_CLUSTER_ID
      || cmd.clusterId == ZCL_EVENTS_CLUSTER_ID
      || cmd.clusterId == ZCL_CALENDAR_CLUSTER_ID) {
    status = sl_zigbee_af_gas_proxy_function_replay_zcl_command(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT,
                                                                SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                                                &cmd);
  } else if (cmd.clusterId == ZCL_SIMPLE_METERING_CLUSTER_ID
             || cmd.clusterId == ZCL_PREPAYMENT_CLUSTER_ID) {
    status = sl_zigbee_af_gas_proxy_function_replay_zcl_command(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT,
                                                                SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT,
                                                                &cmd);
  } else if (cmd.clusterId == ZCL_BASIC_CLUSTER_ID) {
    // EMAPPFWKV2-1308 - GCS21e (messageCode = 0x009E) wants us to read the
    // Basic cluster data from the mirror.
    status = sl_zigbee_af_gas_proxy_function_replay_zcl_command(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT,
                                                                (nonTomGbzRequestParser.messageCode == 0x009E
                                                                 ? SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT
                                                                 : SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT),
                                                                &cmd);
  } else if (cmd.clusterId == ZCL_DEVICE_MANAGEMENT_CLUSTER_ID ) {
    if ((cmd.commandId == ZCL_PUBLISH_CHANGE_OF_TENANCY_COMMAND_ID)
        && (cmd.direction == ZCL_DIRECTION_SERVER_TO_CLIENT)) {
      status = sl_zigbee_af_gas_proxy_function_replay_zcl_command(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT,
                                                                  SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT,
                                                                  &cmd);
    } else {
      status = sl_zigbee_af_gas_proxy_function_replay_zcl_command(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT,
                                                                  SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                                                  &cmd);
    }
  } else {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: Replaying following command");
    sl_zigbee_af_gbz_message_controller_print_command_info(&cmd);
    status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: Unable to replay ZCL command (error: 0x%02X)!", status);
    return false;
  }

  return true;
}

uint32_t sli_zigbee_af_gas_proxy_function_get_gbz_start_time(void)
{
  return (!nonTomHandlingActive) ? 0 : nonTomGbzStartTime;
}

void sli_zigbee_af_gas_proxy_function_alert(uint16_t alertCode,
                                            sl_zigbee_af_cluster_command_t *cmd,
                                            uint16_t messageCode)
{
  sl_zigbee_af_gbz_zcl_command_t gbzZclCmd = { 0 };
  sl_zigbee_af_gbz_message_creator_state_t creator;
  sl_zigbee_af_gbz_message_creator_result_t *gbzResponse;

  gbzZclCmd.clusterId = cmd->apsFrame->clusterId;
  gbzZclCmd.frameControl = cmd->buffer[0]; // first byte of the aps payload is the frame control
  gbzZclCmd.transactionSequenceNumber = 0;
  gbzZclCmd.commandId = cmd->commandId;
  gbzZclCmd.payload = &cmd->buffer[cmd->payloadStartIndex];
  gbzZclCmd.payloadLength = cmd->bufLen - cmd->payloadStartIndex;
  gbzZclCmd.direction = cmd->direction;
  gbzZclCmd.clusterSpecific = cmd->clusterSpecific;
  gbzZclCmd.mfgSpecific = cmd->mfgSpecific;
  gbzZclCmd.hasFromDateTime = false;
  gbzZclCmd.encryption = false;

  sl_zigbee_af_gbz_message_controller_creator_init(&creator,
                                                   SL_ZIGBEE_AF_GBZ_MESSAGE_ALERT,
                                                   alertCode,
                                                   sl_zigbee_af_get_current_time(),
                                                   messageCode,
                                                   NULL,
                                                   0);
  sl_zigbee_af_gbz_message_controller_append_command(&creator,
                                                     &gbzZclCmd);
  gbzResponse = sl_zigbee_af_gbz_message_controller_creator_assemble(&creator);

  sl_zigbee_af_gas_proxy_function_println("GPF: Calling Alert WAN callback with the following ZCL command");
  sl_zigbee_af_gbz_message_controller_print_command_info(&gbzZclCmd);

  sl_zigbee_af_gas_proxy_function_alert_w_a_n_cb(alertCode, gbzResponse->payload, gbzResponse->payloadLength);
  gbzResponse->freeRequired = false;

  sl_zigbee_af_gbz_message_controller_creator_cleanup(&creator);
}

/**
 * @brief Send a Non Tap Off Message (Non-TOM) to the GPF
 *
 * This function can be used to send a Non Tap Off Message (Non-TOM) to the Gas
 * Proxy Function (GPF) for processing. Each embedded ZCL command, within the
 * GBZ message, will be sent to the local mirror and the corresponding responses
 * will be written out to the provided response buffer. Once all responses have been collected,
 * sl_zigbee_af_gas_proxy_function_non_tap_off_message_handler_completed_cb() will be invoked.
 *
 * @param gbzCommands       a pointer to the complete GBZ command data.
 * @param gbzCommandsLength the length of the GBZ command.
 * @param messageCode       "Message Code" for the corresponding Non TOM command.
 *
 * @return SL_STATUS_OK if all messages have been successfully parsed, a non-zero
 *                       status code on error.
 *
 */
sl_status_t sl_zigbee_af_gas_proxy_function_non_tap_off_message_handler(uint8_t * gbzCommands,
                                                                        uint16_t  gbzCommandsLength,
                                                                        uint16_t  messageCode)
{
  sl_status_t status = SL_STATUS_OK;

  if (nonTomHandlingActive) {
    sl_zigbee_af_gas_proxy_function_println("GPF: cannot process two non tap off messages at the same time");
    // we are returning directly so no internal bookkeeping state is changed.
    return SL_STATUS_INVALID_STATE;
  }
  if (!sl_zigbee_af_gbz_message_controller_parser_init(&nonTomGbzRequestParser,
                                                       SL_ZIGBEE_AF_GBZ_MESSAGE_COMMAND,
                                                       gbzCommands,
                                                       gbzCommandsLength,
                                                       true,
                                                       messageCode)) {
    status = SL_STATUS_FAIL;
    goto kickout;
  }

  if (!sl_zigbee_af_gbz_message_controller_creator_init(&nonTomGbzResponseCreator,
                                                        SL_ZIGBEE_AF_GBZ_MESSAGE_RESPONSE,
                                                        0,
                                                        0,
                                                        messageCode,
                                                        NULL,
                                                        0)) {
    status = SL_STATUS_FAIL;
    goto kickout;
  }

  nonTomHandlingActive = true;
  status = sendNextNonTomZclCmd() ? SL_STATUS_OK : SL_STATUS_FAIL;

  kickout:
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_gbz_message_controller_parser_cleanup(&nonTomGbzRequestParser);
    sl_zigbee_af_gbz_message_controller_creator_cleanup(&nonTomGbzResponseCreator);
    nonTomExpectedZclResps = 0;
    nonTomResponsesCount = 0;
    nonTomHandlingActive = false;

    // Log Non-Actioned Non-TOM
    log_event(gbzCommands,
              gbzCommandsLength,
              NULL,
              0,
              GBCS_EVENT_ID_IMM_HAN_CMD_RXED_NOT_ACTED,
              SL_ZIGBEE_AF_GPF_MESSAGE_TYPE_NON_TOM,
              messageCode);
  }
  return status;
}

/*
 * @ brief Stores captured Zcl Command Responses internally.
 *
 *   If the response payloads contain user sensitive material,
 *   the payloads are encrypted before being stored.
 *
 */
static void captureNonTomZclCmdResp(sl_zigbee_af_cluster_command_t * cmd)
{
  sl_zigbee_af_gbz_zcl_command_t gbzZclCmd = { 0 };
  sl_zigbee_af_gbz_message_creator_result_t *gbzResponse;
  sl_zigbee_af_gas_proxy_function_println("GPF: Intercepting ZCL cmd(0x%02X) on endpoint: %d", cmd->commandId, sl_zigbee_af_current_endpoint());

  if (!nonTomHandlingActive) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: Not expecting a Non-TOM response.");
    return;
  }

  gbzZclCmd.clusterId = cmd->apsFrame->clusterId;
  gbzZclCmd.frameControl = cmd->buffer[0]; // first byte of the aps payload is the frame control
  gbzZclCmd.transactionSequenceNumber = cmd->seqNum;
  gbzZclCmd.commandId = cmd->commandId;
  gbzZclCmd.payload = &cmd->buffer[cmd->payloadStartIndex];
  gbzZclCmd.payloadLength = cmd->bufLen - cmd->payloadStartIndex;
  gbzZclCmd.direction = cmd->direction;
  gbzZclCmd.clusterSpecific = cmd->clusterSpecific;
  gbzZclCmd.mfgSpecific = cmd->mfgSpecific;
  gbzZclCmd.hasFromDateTime = false;
  gbzZclCmd.encryption = false;

  sl_zigbee_af_gas_proxy_function_println("GPF: Saving following ZCL response for Non-TOM message response.");
  sl_zigbee_af_gbz_message_controller_print_command_info(&gbzZclCmd);

  sl_zigbee_af_gbz_message_controller_append_command(&nonTomGbzResponseCreator,
                                                     &gbzZclCmd);

  // allocate final GBZ Non-TOM response buffer
  if (nonTomExpectedZclResps == ++nonTomResponsesCount
      && !sendNextNonTomZclCmd()) {
    gbzResponse = sl_zigbee_af_gbz_message_controller_creator_assemble(&nonTomGbzResponseCreator);

    sl_zigbee_af_gas_proxy_function_println("GPF: Total length for GBZ response: %d", gbzResponse->payloadLength);
    sl_zigbee_af_gas_proxy_function_print("GPF: GBZ response payload: ");
    sl_zigbee_af_gas_proxy_function_print_buffer(gbzResponse->payload, gbzResponse->payloadLength, true);
    sl_zigbee_af_gas_proxy_function_println("");
    sl_zigbee_af_gas_proxy_function_non_tap_off_message_handler_completed_cb(gbzResponse->payload, gbzResponse->payloadLength);
    gbzResponse->freeRequired = false;

    // Log Actioned Non TOM
    log_event(nonTomGbzRequestParser.command,
              nonTomGbzRequestParser.length,
              gbzResponse->payload,
              gbzResponse->payloadLength,
              GBCS_EVENT_ID_IMM_HAN_CMD_RXED_ACTED,
              SL_ZIGBEE_AF_GPF_MESSAGE_TYPE_NON_TOM,
              nonTomGbzRequestParser.messageCode);

    sl_zigbee_af_gbz_message_controller_parser_cleanup(&nonTomGbzRequestParser);
    sl_zigbee_af_gbz_message_controller_creator_cleanup(&nonTomGbzResponseCreator);
    nonTomExpectedZclResps = 0;
    nonTomResponsesCount = 0;
    nonTomHandlingActive = false;
  }
}

/* @brief Capture ZCL response from replayed Non-TOM messages. The captured responses
 * is packed up and delivered to the application via a GBZ message buffer
 */
bool sli_zigbee_af_gas_proxy_function_pre_command_received_callback(sl_zigbee_af_cluster_command_t* cmd)
{
  if (!nonTomHandlingActive
      || (cmd->source != sl_zigbee_get_node_id())) {
    return false;
  }

  if (cmd->apsFrame->destinationEndpoint == SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT) {
    if ((cmd->apsFrame->sourceEndpoint == SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT)
        || (cmd->apsFrame->sourceEndpoint == SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT)) {
      captureNonTomZclCmdResp(cmd);
      return true;
    } else if ((cmd->apsFrame->sourceEndpoint == SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT)
               && (cmd->commandId == ZCL_DEFAULT_RESPONSE_COMMAND_ID)) {
      captureNonTomZclCmdResp(cmd);
      return true;
    }
  }
  return false;
}

// ZCL command callbacks

// Calendar Client commands

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_calendar_cluster_publish_calendar_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_calendar_cluster_publish_calendar_command_t cmd_data;
  bool status;

  if (zcl_decode_calendar_cluster_publish_calendar_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: PublishCalendar 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%02X, \"",
                                          cmd_data.providerId,
                                          cmd_data.issuerEventId,
                                          cmd_data.issuerCalendarId,
                                          cmd_data.startTime,
                                          cmd_data.calendarType);
  sl_zigbee_af_gas_proxy_function_print_string(cmd_data.calendarName);
  sl_zigbee_af_gas_proxy_function_println("\", %d, %d, %d",
                                          cmd_data.numberOfSeasons,
                                          cmd_data.numberOfWeekProfiles,
                                          cmd_data.numberOfDayProfiles);

  if (sl_zigbee_af_gas_proxy_function_ignore_future_command(cmd_data.startTime)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Ignoring future dated command.");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  status = sl_zigbee_af_calendar_common_add_cal_info(cmd_data.providerId,
                                                     cmd_data.issuerEventId,
                                                     cmd_data.issuerCalendarId,
                                                     cmd_data.startTime,
                                                     cmd_data.calendarType,
                                                     cmd_data.calendarName,
                                                     cmd_data.numberOfSeasons,
                                                     cmd_data.numberOfWeekProfiles,
                                                     cmd_data.numberOfDayProfiles);

  if (status) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Updated: Calendar");
    sl_zigbee_af_gas_proxy_function_println("GPF:          providerId: 0x%08X", cmd_data.providerId);
    sl_zigbee_af_gas_proxy_function_println("GPF:          issuerEventId: 0x%08X", cmd_data.issuerEventId);
    sl_zigbee_af_gas_proxy_function_println("GPF:          issuerCalendarId: 0x%08X", cmd_data.issuerCalendarId);
    sl_zigbee_af_gas_proxy_function_println("GPF:          startTimeUtc: 0x%08X", cmd_data.startTime);
    sl_zigbee_af_gas_proxy_function_println("GPF:          calendarType: 0x%02X", cmd_data.calendarType);
    sl_zigbee_af_gas_proxy_function_print("GPF:          calendarName: ");
    sl_zigbee_af_gas_proxy_function_print_string(cmd_data.calendarName);
    sl_zigbee_af_gas_proxy_function_println("");
    sl_zigbee_af_gas_proxy_function_println("GPF:          numberOfSeasons: 0x%02X", cmd_data.numberOfSeasons);
    sl_zigbee_af_gas_proxy_function_println("GPF:          numberOfWeekProfiles: 0x%02X", cmd_data.numberOfWeekProfiles);
    sl_zigbee_af_gas_proxy_function_println("GPF:          numberOfDayProfiles: 0x%02X", cmd_data.numberOfDayProfiles);
  }

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_calendar_cluster_publish_day_profile_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_calendar_cluster_publish_day_profile_command_t cmd_data;
  uint16_t dayScheduleEntriesLength;

  if (zcl_decode_calendar_cluster_publish_day_profile_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  dayScheduleEntriesLength = fieldLength(cmd_data.dayScheduleEntries);

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: PublishDayProfile 0x%08X, 0x%08X, 0x%08X, %d, %d, %d, %d, 0x%02X",
                                          cmd_data.providerId,
                                          cmd_data.issuerEventId,
                                          cmd_data.issuerCalendarId,
                                          cmd_data.dayId,
                                          cmd_data.totalNumberOfScheduleEntries,
                                          cmd_data.commandIndex,
                                          cmd_data.totalNumberOfCommands,
                                          cmd_data.calendarType);

  sl_zigbee_af_calendar_common_add_day_prof_info(cmd_data.issuerCalendarId,
                                                 cmd_data.dayId,
                                                 cmd_data.dayScheduleEntries,
                                                 dayScheduleEntriesLength);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_calendar_cluster_publish_seasons_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_calendar_cluster_publish_seasons_command_t cmd_data;
  bool success;
  uint16_t seasonEntriesLength;
  uint8_t unknownWeekIdSeasonsMask = 0;

  if (zcl_decode_calendar_cluster_publish_seasons_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  seasonEntriesLength = fieldLength(cmd_data.seasonEntries);

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: PublishSeasons 0x%08X, 0x%08X, 0x%08X, %d, %d",
                                          cmd_data.providerId,
                                          cmd_data.issuerEventId,
                                          cmd_data.issuerCalendarId,
                                          cmd_data.commandIndex,
                                          cmd_data.totalNumberOfCommands);

  success = sl_zigbee_af_calendar_server_add_seasons_info(cmd_data.issuerCalendarId,
                                                          cmd_data.seasonEntries,
                                                          seasonEntriesLength,
                                                          &unknownWeekIdSeasonsMask);

  if (!success && unknownWeekIdSeasonsMask != 0) {
    sl_zigbee_af_gas_proxy_function_unknown_season_week_id_cb(cmd_data.issuerCalendarId,
                                                              cmd_data.seasonEntries,
                                                              seasonEntriesLength,
                                                              unknownWeekIdSeasonsMask);
  }

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_calendar_cluster_publish_special_days_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_calendar_cluster_publish_special_days_command_t cmd_data;
  uint16_t speicalDaysEntriesLength;
  uint8_t unknownSpecialDaysMask = 0;
  bool success;

  if (zcl_decode_calendar_cluster_publish_special_days_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  speicalDaysEntriesLength = fieldLength(cmd_data.specialDayEntries);

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: PublishSpecialDays 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%02X, %d, %d, %d, [",
                                          cmd_data.providerId,
                                          cmd_data.issuerEventId,
                                          cmd_data.issuerCalendarId,
                                          cmd_data.startTime,
                                          cmd_data.calendarType,
                                          cmd_data.totalNumberOfSpecialDays,
                                          cmd_data.commandIndex,
                                          cmd_data.totalNumberOfCommands);
  // TODO: print specialDayEntries
  sl_zigbee_af_gas_proxy_function_println("]");

  if (sl_zigbee_af_gas_proxy_function_ignore_future_command(cmd_data.startTime)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Ignoring future dated command.");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  success = sl_zigbee_af_calendar_common_add_special_days_info(cmd_data.issuerCalendarId,
                                                               cmd_data.totalNumberOfSpecialDays,
                                                               cmd_data.specialDayEntries,
                                                               speicalDaysEntriesLength,
                                                               &unknownSpecialDaysMask);
  if (!success && unknownSpecialDaysMask != 0) {
    sl_zigbee_af_gas_proxy_function_unknown_special_days_day_id_cb(cmd_data.issuerCalendarId,
                                                                   cmd_data.specialDayEntries,
                                                                   speicalDaysEntriesLength,
                                                                   unknownSpecialDaysMask);
  }

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_calendar_cluster_publish_week_profile_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_calendar_cluster_publish_week_profile_command_t cmd_data;

  if (zcl_decode_calendar_cluster_publish_week_profile_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: PublishWeekProfile 0x%08X, 0x%08X, 0x%08X, %d, %d, %d, %d, %d, %d, %d, %d",
                                          cmd_data.providerId,
                                          cmd_data.issuerEventId,
                                          cmd_data.issuerCalendarId,
                                          cmd_data.weekId,
                                          cmd_data.dayIdRefMonday,
                                          cmd_data.dayIdRefTuesday,
                                          cmd_data.dayIdRefWednesday,
                                          cmd_data.dayIdRefThursday,
                                          cmd_data.dayIdRefFriday,
                                          cmd_data.dayIdRefSaturday,
                                          cmd_data.dayIdRefSunday);

  sl_zigbee_af_calendar_server_add_week_prof_info(cmd_data.issuerCalendarId,
                                                  cmd_data.weekId,
                                                  cmd_data.dayIdRefMonday,
                                                  cmd_data.dayIdRefTuesday,
                                                  cmd_data.dayIdRefWednesday,
                                                  cmd_data.dayIdRefThursday,
                                                  cmd_data.dayIdRefFriday,
                                                  cmd_data.dayIdRefSaturday,
                                                  cmd_data.dayIdRefSunday);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

// Device management commands

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_device_management_cluster_publish_change_of_supplier_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_device_management_cluster_publish_change_of_supplier_command_t cmd_data;
  sl_zigbee_af_device_management_supplier_t supplier;

  if (zcl_decode_device_management_cluster_publish_change_of_supplier_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: PublishChangeOfSupplier: 0x%08X, 0x%08X, 0x%02X, 0x%08X, 0x%08X, 0x%08X, ",
                                          cmd_data.currentProviderId,
                                          cmd_data.issuerEventId,
                                          cmd_data.tariffType,
                                          cmd_data.proposedProviderId,
                                          cmd_data.providerChangeImplementationTime,
                                          cmd_data.providerChangeControl);
  sl_zigbee_af_gas_proxy_function_print_string(cmd_data.proposedProviderName);
  sl_zigbee_af_gas_proxy_function_println(", ");
  sl_zigbee_af_gas_proxy_function_print_string(cmd_data.proposedProviderContactDetails);
  sl_zigbee_af_gas_proxy_function_println("");

  if (sl_zigbee_af_gas_proxy_function_ignore_future_command(cmd_data.providerChangeImplementationTime)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Ignoring future dated command.");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  supplier.proposedProviderId = cmd_data.proposedProviderId;
  supplier.implementationDateTime = cmd_data.providerChangeImplementationTime;
  supplier.providerChangeControl = cmd_data.providerChangeControl;
  sl_zigbee_af_copy_string(supplier.proposedProviderName,
                           cmd_data.proposedProviderName,
                           SL_ZIGBEE_AF_DEVICE_MANAGEMENT_MAXIMUM_PROPOSED_PROVIDER_NAME_LENGTH);
  sl_zigbee_af_copy_string(supplier.proposedProviderContactDetails,
                           cmd_data.proposedProviderContactDetails,
                           SL_ZIGBEE_AF_DEVICE_MANAGEMENT_MAXIMUM_PROPOSED_PROVIDER_CONTACT_DETAILS_LENGTH);

  sl_zigbee_af_device_management_set_supplier(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT, &supplier);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_device_management_cluster_publish_change_of_tenancy_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_device_management_cluster_publish_change_of_tenancy_command_t cmd_data;
  sl_zigbee_af_device_management_tenancy_t tenancy;
  bool result;
  sl_zigbee_af_status_t status;

  if (zcl_decode_device_management_cluster_publish_change_of_tenancy_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_gas_proxy_function_println("RX: PublishChangeOfTenancy: 0x%08X, 0x%08X, 0x%02X, 0x%08X, 0x%08X",
                                          cmd_data.providerId,
                                          cmd_data.issuerEventId,
                                          cmd_data.tariffType,
                                          cmd_data.implementationDateTime,
                                          cmd_data.proposedTenancyChangeControl);

  if (sl_zigbee_af_gas_proxy_function_ignore_future_command(cmd_data.implementationDateTime)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Ignoring future dated command.");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  tenancy.implementationDateTime = cmd_data.implementationDateTime;
  tenancy.tenancy = cmd_data.proposedTenancyChangeControl;
  tenancy.providerId = cmd_data.providerId;
  tenancy.issuerEventId = cmd_data.issuerEventId;
  tenancy.tariffType = cmd_data.tariffType;

  result = sl_zigbee_af_device_management_set_tenancy(&tenancy, true);

  if (result) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Updated: Tenancy");
    sl_zigbee_af_gas_proxy_function_println("              implementationTime: 0x%08X", tenancy.implementationDateTime);
    sl_zigbee_af_gas_proxy_function_println("              tenancy: 0x%08X", tenancy.tenancy);
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  } else {
    sl_zigbee_af_gas_proxy_function_println("GPF: Unable to update tenancy due to mismatching information.");
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
  }

  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_calorific_value_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_calorific_value_command_t cmd_data;
  sl_zigbee_af_status_t status;

  if (zcl_decode_price_cluster_publish_calorific_value_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: PublishCalorificValue 0x%08X, 0x%08X, 0x%08X, 0x%02X, 0x%02X",
                                          cmd_data.issuerEventId,
                                          cmd_data.startTime,
                                          cmd_data.calorificValue,
                                          cmd_data.calorificValueUnit,
                                          cmd_data.calorificValueTrailingDigit);

  if (sl_zigbee_af_gas_proxy_function_ignore_future_command(cmd_data.startTime)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Ignoring future dated command.");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  status = sl_zigbee_af_price_server_calorific_value_add(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                                         cmd_data.issuerEventId,
                                                         cmd_data.startTime,
                                                         cmd_data.calorificValue,
                                                         cmd_data.calorificValueUnit,
                                                         cmd_data.calorificValueTrailingDigit);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: Unable to update calorific value (status:0x%02X).", status);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_conversion_factor_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_conversion_factor_command_t cmd_data;
  sl_status_t status;

  if (zcl_decode_price_cluster_publish_conversion_factor_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: PublishConversionFactor 0x%08X, 0x%08X, 0x%08X, 0x%02X",
                                          cmd_data.issuerEventId,
                                          cmd_data.startTime,
                                          cmd_data.conversionFactor,
                                          cmd_data.conversionFactorTrailingDigit);

  if (sl_zigbee_af_gas_proxy_function_ignore_future_command(cmd_data.startTime)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Ignoring future dated command.");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  status = sl_zigbee_af_price_server_conversion_factor_add(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                                           cmd_data.issuerEventId,
                                                           cmd_data.startTime,
                                                           cmd_data.conversionFactor,
                                                           cmd_data.conversionFactorTrailingDigit);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: Unable to update conversion factor (status:0x%02X).", status);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_block_thresholds_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_block_thresholds_command_t cmd_data;

  if (zcl_decode_price_cluster_publish_block_thresholds_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: PublishBlockThresholds 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%02X, 0x%02X, 0x%02X",
                                          cmd_data.providerId,
                                          cmd_data.issuerEventId,
                                          cmd_data.startTime,
                                          cmd_data.issuerTariffId,
                                          cmd_data.commandIndex,
                                          cmd_data.numberOfCommands,
                                          cmd_data.subPayloadControl);

  if (sl_zigbee_af_gas_proxy_function_ignore_future_command(cmd_data.startTime)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Ignoring future dated command.");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  sl_zigbee_af_price_add_block_thresholds_table_entry(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                                      cmd_data.providerId,
                                                      cmd_data.issuerEventId,
                                                      cmd_data.startTime,
                                                      cmd_data.issuerTariffId,
                                                      cmd_data.commandIndex,
                                                      cmd_data.numberOfCommands,
                                                      cmd_data.subPayloadControl,
                                                      cmd_data.payload);

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_tariff_information_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_tariff_information_command_t cmd_data;
  sl_zigbee_af_price_common_info_t info;
  sl_zigbee_af_scheduled_tariff_t tariff;

  if (zcl_decode_price_cluster_publish_tariff_information_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: PublishTariffInformationReceived");
  sl_zigbee_af_price_cluster_print("RX: PublishTariffInformation 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%02X, \"",
                                   cmd_data.providerId,
                                   cmd_data.issuerEventId,
                                   cmd_data.issuerTariffId,
                                   cmd_data.startTime,
                                   cmd_data.tariffTypeChargingScheme);

  sl_zigbee_af_price_cluster_print_string(cmd_data.tariffLabel);
  sl_zigbee_af_price_cluster_print("\"");
  sl_zigbee_af_price_cluster_print(", 0x%02X, 0x%02X, 0x%02X, 0x%04X, 0x%02X",
                                   cmd_data.numberOfPriceTiersInUse,
                                   cmd_data.numberOfBlockThresholdsInUse,
                                   cmd_data.unitOfMeasure,
                                   cmd_data.currency,
                                   cmd_data.priceTrailingDigit);
  sl_zigbee_af_price_cluster_println(", 0x%08X, 0x%02X, 0x%08X, 0x%08X",
                                     cmd_data.standingCharge,
                                     cmd_data.tierBlockMode,
                                     cmd_data.blockThresholdMultiplier,
                                     cmd_data.blockThresholdDivisor);
  sl_zigbee_af_price_cluster_flush();

  if (sl_zigbee_af_gas_proxy_function_ignore_future_command(cmd_data.startTime)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Ignoring future dated command.");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  info.startTime = cmd_data.startTime;
  info.issuerEventId = cmd_data.issuerEventId;
  tariff.providerId = cmd_data.providerId;
  tariff.issuerTariffId = cmd_data.issuerTariffId;
  tariff.tariffTypeChargingScheme = cmd_data.tariffTypeChargingScheme;
  sl_zigbee_af_copy_string(tariff.tariffLabel, cmd_data.tariffLabel, ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH);
  tariff.numberOfPriceTiersInUse = cmd_data.numberOfPriceTiersInUse;
  tariff.numberOfBlockThresholdsInUse = cmd_data.numberOfBlockThresholdsInUse;
  tariff.unitOfMeasure = cmd_data.unitOfMeasure;
  tariff.currency = cmd_data.currency;
  tariff.priceTrailingDigit = cmd_data.priceTrailingDigit;
  tariff.standingCharge = cmd_data.standingCharge;
  tariff.tierBlockMode = cmd_data.tierBlockMode;
  tariff.blockThresholdMultiplier = cmd_data.blockThresholdMultiplier;
  tariff.blockThresholdDivisor = cmd_data.blockThresholdDivisor;
  tariff.status |= FUTURE;

  sl_zigbee_af_price_add_tariff_table_entry(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                            &info,
                                            &tariff);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_price_matrix_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_price_matrix_command_t cmd_data;

  if (zcl_decode_price_cluster_publish_price_matrix_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: PublishPriceMatrix 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%02X, 0x%02X, 0x%02X",
                                          cmd_data.providerId,
                                          cmd_data.issuerEventId,
                                          cmd_data.startTime,
                                          cmd_data.issuerTariffId,
                                          cmd_data.commandIndex,
                                          cmd_data.numberOfCommands,
                                          cmd_data.subPayloadControl);

  if (sl_zigbee_af_gas_proxy_function_ignore_future_command(cmd_data.startTime)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Ignoring future dated command.");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  sl_zigbee_af_price_add_price_matrix_raw(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                          cmd_data.providerId,
                                          cmd_data.issuerEventId,
                                          cmd_data.startTime,
                                          cmd_data.issuerTariffId,
                                          cmd_data.commandIndex,
                                          cmd_data.numberOfCommands,
                                          cmd_data.subPayloadControl,
                                          cmd_data.payload);

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_messaging_cluster_display_message_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_messaging_cluster_display_message_command_t cmd_data;
  sl_zigbee_af_plugin_messaging_server_message_t message;
  uint8_t msgLength;

  if (zcl_decode_messaging_cluster_display_message_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  msgLength = sl_zigbee_af_string_length(cmd_data.message) + 1;

  if (msgLength > SL_ZIGBEE_AF_PLUGIN_MESSAGING_SERVER_MESSAGE_SIZE) {
    sl_zigbee_af_gas_proxy_function_print("GPF: ERR: Message too long for messaging server message buffer.");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  if (sl_zigbee_af_gas_proxy_function_ignore_future_command(cmd_data.startTime)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Ignoring future dated command.");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  message.messageId = cmd_data.messageId;
  message.messageControl = cmd_data.messageControl;
  message.startTime = cmd_data.startTime;
  message.durationInMinutes = cmd_data.durationInMinutes;
  memcpy(message.message, cmd_data.message, msgLength);
  message.extendedMessageControl = cmd_data.optionalExtendedMessageControl;

  sl_zigbee_af_gas_proxy_function_print("GPF: RX: DisplayMessage"
                                        " 0x%08X, 0x%02X, 0x%08X, 0x%04X, \"",
                                        cmd_data.messageId,
                                        cmd_data.messageControl,
                                        cmd_data.startTime,
                                        cmd_data.durationInMinutes);
  sl_zigbee_af_gas_proxy_function_print_string(cmd_data.message);
  sl_zigbee_af_gas_proxy_function_print(", 0x%02X", cmd_data.optionalExtendedMessageControl);
  sl_zigbee_af_gas_proxy_function_println("\"");

  sl_zigbee_af_messaging_server_set_message(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                            &message);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_block_period_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_block_period_command_t cmd_data;
  uint32_t thresholdMultiplier;
  uint32_t thresholdDivisor;

  if (zcl_decode_price_cluster_publish_block_period_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: PublishBlockPeriod 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%02X, 0x%02X, 0x%02X, 0x%02X",
                                          cmd_data.providerId,
                                          cmd_data.issuerEventId,
                                          cmd_data.blockPeriodStartTime,
                                          cmd_data.blockPeriodDuration,
                                          cmd_data.blockPeriodControl,
                                          cmd_data.blockPeriodDurationType,
                                          cmd_data.tariffType,
                                          cmd_data.tariffResolutionPeriod);

  if (sl_zigbee_af_gas_proxy_function_ignore_future_command(cmd_data.blockPeriodStartTime)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Ignoring future dated command.");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  sl_zigbee_af_read_attribute(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                              ZCL_PRICE_CLUSTER_ID,
                              ZCL_THRESHOLD_MULTIPLIER_ATTRIBUTE_ID,
                              CLUSTER_MASK_SERVER,
                              (uint8_t *)&(thresholdMultiplier),
                              sl_zigbee_af_get_data_size(ZCL_INT24U_ATTRIBUTE_TYPE),
                              NULL);

  sl_zigbee_af_read_attribute(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                              ZCL_PRICE_CLUSTER_ID,
                              ZCL_THRESHOLD_DIVISOR_ATTRIBUTE_ID,
                              CLUSTER_MASK_SERVER,
                              (uint8_t *)&(thresholdDivisor),
                              sl_zigbee_af_get_data_size(ZCL_INT24U_ATTRIBUTE_TYPE),
                              NULL);

  sl_zigbee_af_price_server_block_period_add(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                             cmd_data.providerId,
                                             cmd_data.issuerEventId,
                                             cmd_data.blockPeriodStartTime,
                                             cmd_data.blockPeriodDuration,
                                             cmd_data.blockPeriodControl,
                                             cmd_data.blockPeriodDurationType,
                                             thresholdMultiplier,
                                             thresholdDivisor,
                                             cmd_data.tariffType,
                                             cmd_data.tariffResolutionPeriod);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_billing_period_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_billing_period_command_t cmd_data;

  if (zcl_decode_price_cluster_publish_billing_period_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: PublishBillingPeriod 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%02X, 0x%02X",
                                          cmd_data.providerId,
                                          cmd_data.issuerEventId,
                                          cmd_data.billingPeriodStartTime,
                                          cmd_data.billingPeriodDuration,
                                          cmd_data.billingPeriodDurationType,
                                          cmd_data.tariffType);

  if (sl_zigbee_af_gas_proxy_function_ignore_future_command(cmd_data.billingPeriodStartTime)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Ignoring future dated command.");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  sl_zigbee_af_price_server_billing_period_add(SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT,
                                               cmd_data.billingPeriodStartTime,
                                               cmd_data.issuerEventId,
                                               cmd_data.providerId,
                                               cmd_data.billingPeriodDuration,
                                               cmd_data.billingPeriodDurationType,
                                               cmd_data.tariffType);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_request_mirror_cb(void)
{
  sl_802154_long_addr_t otaEui;
  uint16_t endpointId;

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: RequestMirror");

  if (sl_zigbee_lookup_eui64_by_node_id(sl_zigbee_af_response_destination, otaEui) == SL_STATUS_OK) {
    endpointId = sl_zigbee_af_meter_mirror_request_mirror(otaEui);
    if (endpointId != 0xFFFF) {
      sl_zigbee_af_fill_command_simple_metering_cluster_request_mirror_response(endpointId);
      sl_zigbee_af_send_response();
      return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
    } else {
      sl_zigbee_af_gas_proxy_function_println("GPF: Invalid endpoint. Sending Default Response");
      return SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
    }
  }
  return SL_ZIGBEE_ZCL_STATUS_FAILURE;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_remove_mirror_cb(void)
{
  sl_802154_long_addr_t otaEui;
  uint16_t endpointId;

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: RemoveMirror");

  if (sl_zigbee_lookup_eui64_by_node_id(sl_zigbee_af_response_destination, otaEui) == SL_STATUS_OK) {
    endpointId = sl_zigbee_af_meter_mirror_remove_mirror(otaEui);
    sl_zigbee_af_fill_command_simple_metering_cluster_mirror_removed(endpointId);
    sl_zigbee_af_send_response();
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }
  return SL_ZIGBEE_ZCL_STATUS_FAILURE;
}

/** @brief Publish Info
 *
 * This function is called by the calendar-server plugin after receiving any of
 * the following commands and just before it starts publishing the response:
 * GetCalendar, GetDayProfiles, GetSeasons, GetSpecialDays, and GetWeekProfiles.
 *
 * @param publishCommandId ZCL command to be published  Ver.: always
 * @param clientNodeId Destination nodeId  Ver.: always
 * @param clientEndpoint Destination endpoint  Ver.: always
 * @param totalCommands Total number of publish commands to be sent  Ver.:
 * always
 */
void sl_zigbee_af_calendar_server_publish_info_cb(uint8_t publishCommandId,
                                                  sl_802154_short_addr_t clientNodeId,
                                                  uint8_t clientEndpoint,
                                                  uint8_t totalCommands)
{
  UNUSED_VAR(publishCommandId);
  UNUSED_VAR(clientEndpoint);

  // If the publish is a result of a non-TOM use case then the clientNodeId
  // will be our nodeId and if so we need to update the number of expected
  // response to include all of the publish commands to be sent. We subtract
  // one from the total commands as we always expect at least one response
  // when we initialize the expected responses count.
  if (nonTomHandlingActive && (clientNodeId == sl_zigbee_get_node_id())) {
    uint8_t expectedZclResps = nonTomExpectedZclResps + (totalCommands - 1);
    sl_zigbee_af_gas_proxy_function_println("GPF: Updating number of Non-TOM expected responses from %d to %d",
                                            nonTomExpectedZclResps,
                                            expectedZclResps);
    nonTomExpectedZclResps = expectedZclResps;
  }
}

sl_zigbee_af_gpf_message_t * sl_zigbee_af_gas_proxy_function_get_current_message(void)
{
  if ((currentGpfMessage.gbzCommands == NULL)
      && (currentGpfMessage.gbzCommandsLength == 0)) {
    return NULL;
  }
  return &currentGpfMessage;
}

bool sl_zigbee_af_gas_proxy_function_ignore_future_command(uint32_t startTime)
{
  uint32_t now = sl_zigbee_af_get_current_time();
  if ((startTime > now)
      && (sl_zigbee_af_gas_proxy_function_get_current_message() != NULL)) {
    sl_zigbee_af_gas_proxy_function_tap_off_message_future_command_ignored_cb(sl_zigbee_af_gas_proxy_function_get_current_message(),
                                                                              sl_zigbee_af_current_command());
    return true;
  }

  return false;
}

uint32_t sli_zigbee_af_gas_proxy_function_calendar_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                                                sl_service_function_context_t *context)
{
  UNUSED_VAR(opcode);

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_PUBLISH_CALENDAR_COMMAND_ID:
      {
        status = sl_zigbee_af_calendar_cluster_publish_calendar_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_DAY_PROFILE_COMMAND_ID:
      {
        status = sl_zigbee_af_calendar_cluster_publish_day_profile_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_WEEK_PROFILE_COMMAND_ID:
      {
        status = sl_zigbee_af_calendar_cluster_publish_week_profile_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_SEASONS_COMMAND_ID:
      {
        status = sl_zigbee_af_calendar_cluster_publish_seasons_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_SPECIAL_DAYS_COMMAND_ID:
      {
        status = sl_zigbee_af_calendar_cluster_publish_special_days_cb(cmd);
        break;
      }
    }
  }

  return status;
}

uint32_t sli_zigbee_af_gas_proxy_function_device_management_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                                                         sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_PUBLISH_CHANGE_OF_TENANCY_COMMAND_ID:
      {
        status = sl_zigbee_af_device_management_cluster_publish_change_of_tenancy_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_CHANGE_OF_SUPPLIER_COMMAND_ID:
      {
        status = sl_zigbee_af_device_management_cluster_publish_change_of_supplier_cb(cmd);
        break;
      }
    }
  }

  return status;
}

uint32_t sli_zigbee_af_gas_proxy_function_price_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                                             sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_PUBLISH_BLOCK_PERIOD_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_block_period_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_CONVERSION_FACTOR_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_conversion_factor_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_CALORIFIC_VALUE_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_calorific_value_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_TARIFF_INFORMATION_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_tariff_information_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_BLOCK_THRESHOLDS_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_block_thresholds_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_BILLING_PERIOD_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_billing_period_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_PRICE_MATRIX_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_price_matrix_cb(cmd);
        break;
      }
    }
  }

  return status;
}

uint32_t sli_zigbee_af_gas_proxy_function_messaging_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                                                 sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_DISPLAY_MESSAGE_COMMAND_ID:
      {
        status = sl_zigbee_af_messaging_cluster_display_message_cb(cmd);
        break;
      }
    }
  }

  return status;
}

uint32_t sli_zigbee_af_gas_proxy_function_simple_metering_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                                                       sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_REQUEST_MIRROR_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_request_mirror_cb();
        break;
      }
      case ZCL_REMOVE_MIRROR_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_remove_mirror_cb();
        break;
      }
      case ZCL_GET_NOTIFIED_MESSAGE_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_get_notified_message_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_SNAPSHOT_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_publish_snapshot_cb(cmd);
        break;
      }
      case ZCL_GET_SAMPLED_DATA_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_get_sampled_data_response_cb(cmd);
        break;
      }
    }
  }

  return status;
}

uint32_t sli_zigbee_af_gas_proxy_function_simple_metering_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                                                       sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_GET_SAMPLED_DATA_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_get_sampled_data_cb(cmd);
        break;
      }
      case ZCL_GET_SNAPSHOT_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_get_snapshot_cb(cmd);
        break;
      }
    }
  }

  return status;
}

uint32_t sli_zigbee_af_gas_proxy_function_prepayment_cluster_client_command_parse(sl_service_opcode_t opcode,
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

uint32_t sli_zigbee_af_gas_proxy_function_prepayment_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                                                  sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_GET_PREPAY_SNAPSHOT_COMMAND_ID:
      {
        status = sl_zigbee_af_prepayment_cluster_get_prepay_snapshot_cb(cmd);
        break;
      }
      case ZCL_GET_TOP_UP_LOG_COMMAND_ID:
      {
        status = sl_zigbee_af_prepayment_cluster_get_top_up_log_cb(cmd);
        break;
      }
      case  ZCL_GET_DEBT_REPAYMENT_LOG_COMMAND_ID:
      {
        status = sl_zigbee_af_prepayment_cluster_get_debt_repayment_log_cb(cmd);
        break;
      }
    }
  }

  return status;
}
