/***************************************************************************//**
 * @file
 * @brief Routines for the Messaging Server plugin, which implements the server
 *        side of the Messaging cluster.
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

#include "../../include/af.h"
#include "messaging-server.h"

#include "zap-cluster-command-parser.h"

// The internal message is stored in the same structure type that is defined
// publicly.  The internal state of the message is stored in the
// messageStatusControl field
static sl_zigbee_af_plugin_messaging_server_message_t msgTable[SL_ZIGBEE_ZCL_MESSAGING_CLUSTER_SERVER_ENDPOINT_COUNT];

// These bits are used by the messageStatusControl to indicate whether or not
// a message is valid, active, or if it is a "send now" message
#define VALID  BIT(0)
#define ACTIVE BIT(1)
#define NOW    BIT(2)

#define messageIsValid(ep)   (msgTable[ep].messageStatusControl & VALID)
#define messageIsActive(ep)  (msgTable[ep].messageStatusControl & ACTIVE)
#define messageIsNow(ep)     (msgTable[ep].messageStatusControl & NOW)
#define messageIsForever(ep) (msgTable[ep].durationInMinutes == ZCL_MESSAGING_CLUSTER_DURATION_UNTIL_CHANGED)

static bool messageIsCurrentOrScheduled(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_MESSAGING_CLUSTER_ID);

  if (ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX) {
    return false;
  }

  return (messageIsValid(ep)
          && messageIsActive(ep)
          && (messageIsForever(ep)
              || (sl_zigbee_af_get_current_time()
                  < msgTable[ep].startTime + (uint32_t)msgTable[ep].durationInMinutes * 60)));
}

void sl_zigbee_af_messaging_cluster_server_init_cb(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_MESSAGING_CLUSTER_ID);

  if (ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX) {
    return;
  }

  msgTable[ep].messageStatusControl &= ~VALID;
}

//----------------------
// ZCL command callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_messaging_cluster_get_last_message_cb(void)
{
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  sl_zigbee_af_plugin_messaging_server_message_t message;
  sl_zigbee_af_messaging_cluster_println("RX: GetLastMessage");
  if (sl_zigbee_af_messaging_server_get_message(endpoint, &message)) {
    sl_zigbee_af_fill_command_messaging_cluster_display_message(message.messageId,
                                                                message.messageControl,
                                                                message.startTime,
                                                                message.durationInMinutes,
                                                                message.message,
                                                                message.extendedMessageControl);
    sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
    sl_zigbee_af_send_response();
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }
  return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_messaging_cluster_message_confirmation_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;

#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
  sl_zcl_messaging_cluster_message_confirmation_command_t cmd_data;

  if (zcl_decode_messaging_cluster_message_confirmation_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_messaging_cluster_println("RX: MessageConfirmation 0x%08X, 0x%08X",
                                         cmd_data.messageId,
                                         cmd_data.confirmationTime);
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

bool sl_zigbee_af_messaging_server_get_message(uint8_t endpoint,
                                               sl_zigbee_af_plugin_messaging_server_message_t *message)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_MESSAGING_CLUSTER_ID);

  if (ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX) {
    return false;
  }

  memmove(message, &msgTable[ep], sizeof(sl_zigbee_af_plugin_messaging_server_message_t));

  // Clear out our internal bits from the message control.
  message->messageStatusControl &= ~ZCL_MESSAGING_CLUSTER_RESERVED_MASK;

  // If the message is expired or it has an absolute time, set the start time
  // and duration to the original start time and duration.  For "start now"
  // messages that are current or scheduled, set the start time to the special
  // value for "now" and set the duration to the remaining time, if it is not
  // already the special value for "until changed."
  if (messageIsCurrentOrScheduled(endpoint) && messageIsNow(ep)) {
    message->startTime = ZCL_MESSAGING_CLUSTER_START_TIME_NOW;
    if (!messageIsForever(ep)) {
      message->durationInMinutes -= ((sl_zigbee_af_get_current_time() - msgTable[ep].startTime)
                                     / 60);
    }
  }
  return messageIsCurrentOrScheduled(endpoint);
}

void sl_zigbee_af_messaging_server_set_message(uint8_t endpoint,
                                               const sl_zigbee_af_plugin_messaging_server_message_t *message)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_MESSAGING_CLUSTER_ID);

  if (ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX) {
    return;
  }

  if (message == NULL) {
    msgTable[ep].messageStatusControl &= ~ACTIVE;
    return;
  }

  memmove(&msgTable[ep], message, sizeof(sl_zigbee_af_plugin_messaging_server_message_t));

  // Rember if this is a "start now" message, but store the start time as the
  // current time so the duration can be adjusted.
  if (msgTable[ep].startTime == ZCL_MESSAGING_CLUSTER_START_TIME_NOW) {
    msgTable[ep].messageStatusControl |= NOW;
    msgTable[ep].startTime = sl_zigbee_af_get_current_time();
  } else {
    msgTable[ep].messageStatusControl &= ~NOW;
  }

  msgTable[ep].messageStatusControl |= (VALID | ACTIVE);
}

void sli_zigbee_af_messaging_server_print_info(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_MESSAGING_CLUSTER_ID);

  if (ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX) {
    return;
  }

  sl_zigbee_af_messaging_cluster_println("= Server Message =");
  sl_zigbee_af_messaging_cluster_flush();

  sl_zigbee_af_messaging_cluster_println(" vld: %s", (messageIsValid(ep) ? "YES" : "NO"));
  sl_zigbee_af_messaging_cluster_println(" act: %s", (messageIsCurrentOrScheduled(endpoint)
                                                      ? "YES"
                                                      : "NO"));
  sl_zigbee_af_messaging_cluster_println("  id: 0x%08X", msgTable[ep].messageId);
  sl_zigbee_af_messaging_cluster_println("  mc: 0x%02X",
                                         (msgTable[ep].messageControl
                                          & ~ZCL_MESSAGING_CLUSTER_RESERVED_MASK));
  sl_zigbee_af_messaging_cluster_println("  st: 0x%08X", msgTable[ep].startTime);
  sl_zigbee_af_messaging_cluster_println(" now: %s", (messageIsNow(ep) ? "YES" : "NO"));
  sl_zigbee_af_messaging_cluster_println("time: 0x%08X", sl_zigbee_af_get_current_time());
  sl_zigbee_af_messaging_cluster_println(" dur: 0x%04X", msgTable[ep].durationInMinutes);
  sl_zigbee_af_messaging_cluster_flush();
  sl_zigbee_af_messaging_cluster_print(" mes: \"");
  sl_zigbee_af_messaging_cluster_print_string(msgTable[ep].message);
  sl_zigbee_af_messaging_cluster_println("\"");
  sl_zigbee_af_messaging_cluster_flush();
}

void sl_zigbee_af_messaging_server_display_message(sl_802154_short_addr_t nodeId,
                                                   uint8_t srcEndpoint,
                                                   uint8_t dstEndpoint)
{
  sl_status_t status;
  sl_zigbee_af_plugin_messaging_server_message_t message;
  if (!sl_zigbee_af_messaging_server_get_message(srcEndpoint, &message)) {
    sl_zigbee_af_messaging_cluster_println("invalid msg");
    return;
  }

  sl_zigbee_af_fill_command_messaging_cluster_display_message(message.messageId,
                                                              message.messageControl,
                                                              message.startTime,
                                                              message.durationInMinutes,
                                                              message.message,
                                                              message.extendedMessageControl);
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_messaging_cluster_println("Error in display %02X", status);
  }
}

void sl_zigbee_af_messaging_server_cancel_message(sl_802154_short_addr_t nodeId,
                                                  uint8_t srcEndpoint,
                                                  uint8_t dstEndpoint)
{
  sl_status_t status;
  sl_zigbee_af_plugin_messaging_server_message_t message;

  // Nullify the current message before sending the cancellation.
  sl_zigbee_af_messaging_server_set_message(srcEndpoint, NULL);

  // Then send the response
  sl_zigbee_af_messaging_server_get_message(srcEndpoint, &message);

  sl_zigbee_af_fill_command_messaging_cluster_cancel_message(message.messageId,
                                                             message.messageControl);
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_messaging_cluster_println("Error in cancel %02X", status);
  }
}

uint32_t sl_zigbee_af_messaging_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                             sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_GET_LAST_MESSAGE_COMMAND_ID:
      {
        status = sl_zigbee_af_messaging_cluster_get_last_message_cb();
        break;
      }
      case ZCL_MESSAGE_CONFIRMATION_COMMAND_ID:
      {
        status = sl_zigbee_af_messaging_cluster_message_confirmation_cb(cmd);
        break;
      }
      default:
        break;
    }
  }

  return status;
}
