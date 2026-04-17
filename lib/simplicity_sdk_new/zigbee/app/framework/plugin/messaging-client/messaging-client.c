/***************************************************************************//**
 * @file
 * @brief Routines for the Messaging Client plugin, which implements the client
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
#include "../../util/common.h"
#include "messaging-client.h"

#include "messaging-client-config.h"
#include "zap-cluster-command-parser.h"
#include "esi-management.h"

static sl_zigbee_af_plugin_messaging_client_message_t messageTable[SL_ZIGBEE_ZCL_MESSAGING_CLUSTER_CLIENT_ENDPOINT_COUNT];

#define MESSAGE_CONTROL_INTER_PAN_TRANSMISSION_ONLY (0x2)
/**
 * The callback function passed to the ESI management plugin. It handles
 * ESI entry deletions.
 */
static void esiDeletionCallback(uint8_t esiIndex)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_ZCL_MESSAGING_CLUSTER_CLIENT_ENDPOINT_COUNT; i++) {
    messageTable[i].esiBitmask &= ~BIT(esiIndex);
  }
}

void sl_zigbee_af_messaging_cluster_client_init_cb(uint8_t endpoint)
{
  sli_zigbee_af_messaging_client_clear_message(endpoint);
  // Subscribing for ESI Management plugin deletion announcements.
  sl_zigbee_af_esi_management_subscribe_to_deletion_announcements(esiDeletionCallback);
}

void sli_zigbee_af_messaging_client_clear_message(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_MESSAGING_CLUSTER_ID);
  if ( ep != 0xFF ) {
    messageTable[ep].valid             = false;
    messageTable[ep].active            = false;
    messageTable[ep].messageId         = 0x00000000UL;
    messageTable[ep].messageControl    = 0x00;
    messageTable[ep].startTime         = 0x00000000UL;
    messageTable[ep].endTime           = 0x00000000UL;
    messageTable[ep].durationInMinutes = 0x0000;
    messageTable[ep].esiBitmask        = 0;
    messageTable[ep].message[0]        = 0;
  }
}

void sl_zigbee_af_messaging_cluster_client_tick_cb(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_MESSAGING_CLUSTER_ID);

  if (ep == 0xFF) {
    return;
  }

  // The tick is used to deactivate messages that are active or to activate
  // messages that are inactive.  So, first, just flip the active state and
  // notify the application of the change.
  messageTable[ep].active = !messageTable[ep].active;
  if (messageTable[ep].active) {
    sl_zigbee_af_messaging_client_display_message_cb(&messageTable[ep]);
  } else {
    sl_zigbee_af_messaging_client_cancel_message_cb(&messageTable[ep]);
  }

  // If the new state is active and the duration is not "until changed," then we
  // have to schedule a tick to deactivate the message after the duration has
  // elapsed.  Otherwise, cancel the tick because no further processing is
  // required as either the message is no longer active or it will live until
  // changed.
  if (messageTable[ep].active
      && (messageTable[ep].durationInMinutes
          != ZCL_MESSAGING_CLUSTER_DURATION_UNTIL_CHANGED)) {
    sl_zigbee_zcl_schedule_client_tick(endpoint,
                                       ZCL_MESSAGING_CLUSTER_ID,
                                       ((messageTable[ep].endTime
                                         - messageTable[ep].startTime)
                                        * MILLISECOND_TICKS_PER_SECOND));
  }
}

//-----------------------
// ZCL commands callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_messaging_cluster_display_message_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_messaging_cluster_display_message_command_t cmd_data;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_MESSAGING_CLUSTER_ID);
  uint32_t now = sl_zigbee_af_get_current_time();
  uint8_t esiIndex = sl_zigbee_af_esi_management_update_esi_and_get_index(cmd);

  if (zcl_decode_messaging_cluster_display_message_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_messaging_cluster_print("RX: DisplayMessage"
                                       " 0x%08X, 0x%02X, 0x%08X, 0x%04X, \"",
                                       cmd_data.messageId,
                                       cmd_data.messageControl,
                                       cmd_data.startTime,
                                       cmd_data.durationInMinutes);
  sl_zigbee_af_messaging_cluster_print_string(cmd_data.message);
  sl_zigbee_af_messaging_cluster_println("\"");

  if (sl_zigbee_af_messaging_client_pre_display_message_cb(cmd_data.messageId,
                                                           cmd_data.messageControl,
                                                           cmd_data.startTime,
                                                           cmd_data.durationInMinutes,
                                                           cmd_data.message,
                                                           cmd_data.optionalExtendedMessageControl)) {
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  if (ep == 0xFF) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  // Use of Inter-PAN transmission is deprecated.
  if ((cmd_data.messageControl & 0x03) == MESSAGE_CONTROL_INTER_PAN_TRANSMISSION_ONLY) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  }

  if (messageTable[ep].active) {
    // First check whether it is a duplicate message
    if (cmd_data.messageId == messageTable[ep].messageId) {
      //Duplicate message from a different ESI, add the ESI to the bitmask
      if (esiIndex < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE
          && (messageTable[ep].esiBitmask & BIT(esiIndex)) == 0) {
        messageTable[ep].esiBitmask |= BIT(esiIndex);
      }
      // Either way, we send back a default response.
      return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    } else {
      // If we already have another message, notify the application that it
      // should no longer display it.
      messageTable[ep].active = false;
      sl_zigbee_af_messaging_client_cancel_message_cb(&messageTable[ep]);
      sl_zigbee_zcl_deactivate_client_tick(sl_zigbee_af_current_endpoint(),
                                           ZCL_MESSAGING_CLUSTER_ID);
    }
  }

  if (esiIndex < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE) {
    messageTable[ep].esiBitmask = BIT(esiIndex);
  }

  messageTable[ep].clientEndpoint =
    sl_zigbee_af_current_command()->apsFrame->destinationEndpoint;
  messageTable[ep].messageId         = cmd_data.messageId;
  messageTable[ep].messageControl    = cmd_data.messageControl;
  messageTable[ep].startTime         = (cmd_data.startTime == ZCL_MESSAGING_CLUSTER_START_TIME_NOW
                                        ? now
                                        : cmd_data.startTime);
  messageTable[ep].endTime           = (cmd_data.durationInMinutes == ZCL_MESSAGING_CLUSTER_DURATION_UNTIL_CHANGED
                                        ? ZCL_MESSAGING_CLUSTER_END_TIME_NEVER
                                        : messageTable[ep].startTime + cmd_data.durationInMinutes * 60);
  messageTable[ep].durationInMinutes = cmd_data.durationInMinutes;
  sl_zigbee_af_copy_string(messageTable[ep].message, cmd_data.message, SL_ZIGBEE_AF_PLUGIN_MESSAGING_CLIENT_MESSAGE_SIZE);

  if (messageTable[ep].startTime <= now) {
    if (messageTable[ep].durationInMinutes
        == ZCL_MESSAGING_CLUSTER_DURATION_UNTIL_CHANGED) {
      // If a message starts in the past and the duration is "until changed," it
      // gets marked as active and no tick is scheduled becaues the message will
      // live until we receive a new message or a cancel command.
      messageTable[ep].active = true;
      sl_zigbee_af_messaging_client_display_message_cb(&messageTable[ep]);
    } else if (messageTable[ep].endTime > now) {
      // If a message starts in the past and the duration has not elapsed, it
      // gets marked as active and a tick is scheduled for the time remaining in
      // the duration to wake us up and mark the message as inactive.
      messageTable[ep].active = true;
      sl_zigbee_af_messaging_client_display_message_cb(&messageTable[ep]);
      sl_zigbee_zcl_schedule_client_tick(sl_zigbee_af_current_endpoint(),
                                         ZCL_MESSAGING_CLUSTER_ID,
                                         ((messageTable[ep].endTime - now)
                                          * MILLISECOND_TICKS_PER_SECOND));
    } else {
      // Finally, if a message starts in the past and the duration has elapsed,
      // it gets marked as inactive and no tick is scheduled because no further
      // processing is needed.
      messageTable[ep].active = false;
    }
  } else {
    // If a message starts in the future, it gets marked as inactive and a tick
    // is scheduled to wake us up at startTime to mark the message as active.
    messageTable[ep].active = false;
    sl_zigbee_zcl_schedule_client_tick(sl_zigbee_af_current_endpoint(),
                                       ZCL_MESSAGING_CLUSTER_ID,
                                       ((messageTable[ep].startTime - now)
                                        * MILLISECOND_TICKS_PER_SECOND));
  }

  // Once we receive a message, the data in the structure becomes valid and
  // remains valid forever.  When the message expires or is canceled, it gets
  // marked as inactive, but stays valid.  This allows the application to
  // display or confirm old messages.
  messageTable[ep].valid = true;

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_messaging_cluster_cancel_message_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_messaging_cluster_cancel_message_command_t cmd_data;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_MESSAGING_CLUSTER_ID);
  sl_zigbee_af_status_t status;

  if (zcl_decode_messaging_cluster_cancel_message_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS
      || ep == 0xFF) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_messaging_cluster_println("RX: CancelMessage 0x%08X, 0x%02X",
                                         cmd_data.messageId,
                                         cmd_data.messageControl);

  if (messageTable[ep].active && messageTable[ep].messageId == cmd_data.messageId) {
    messageTable[ep].active = false;
    messageTable[ep].messageControl = cmd_data.messageControl;
    sl_zigbee_af_messaging_client_cancel_message_cb(&messageTable[ep]);
    sl_zigbee_zcl_deactivate_client_tick(sl_zigbee_af_current_endpoint(),
                                         ZCL_MESSAGING_CLUSTER_ID);
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  } else {
    status = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }

  return status;
}

void sli_zigbee_af_messaging_client_print_info(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_MESSAGING_CLUSTER_ID);

  if (ep == 0xFF) {
    return;
  }

  sl_zigbee_af_messaging_cluster_println("= Client Message =");
  sl_zigbee_af_messaging_cluster_flush();

  sl_zigbee_af_messaging_cluster_println(" vld: %s", (messageTable[ep].valid ? "YES" : "NO"));
  sl_zigbee_af_messaging_cluster_println(" act: %s", (messageTable[ep].active ? "YES" : "NO"));
  sl_zigbee_af_messaging_cluster_println("  id: 0x%08X", messageTable[ep].messageId);
  sl_zigbee_af_messaging_cluster_println("  mc: 0x%02X", messageTable[ep].messageControl);
  sl_zigbee_af_messaging_cluster_println("  st: 0x%08X", messageTable[ep].startTime);
  sl_zigbee_af_messaging_cluster_println("  et: 0x%08X", messageTable[ep].endTime);
  sl_zigbee_af_messaging_cluster_println("time: 0x%08X", sl_zigbee_af_get_current_time());
  sl_zigbee_af_messaging_cluster_println(" dur: 0x%04X", messageTable[ep].durationInMinutes);
  sl_zigbee_af_messaging_cluster_flush();
  sl_zigbee_af_messaging_cluster_print(" mes: \"");
  sl_zigbee_af_messaging_cluster_print_string(messageTable[ep].message);
  sl_zigbee_af_messaging_cluster_println("\"");
  sl_zigbee_af_messaging_cluster_flush();
}

sl_zigbee_af_status_t sl_zigbee_af_messaging_client_confirm_message(uint8_t endpoint)
{
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_MESSAGING_CLUSTER_ID);
  uint8_t i;

  if (ep == 0xFF) {
    return SL_ZIGBEE_ZCL_STATUS_FAILURE;
  }

  if (messageTable[ep].valid && messageTable[ep].active ) {
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE; i++) {
      sl_zigbee_af_plugin_esi_management_esi_entry_t *esiEntry =
        sl_zigbee_af_esi_management_esi_look_up_by_index(i);
      sl_802154_short_addr_t nodeId;
      if ((messageTable[ep].esiBitmask & BIT(i)) == 0 || esiEntry == NULL) {
        continue;
      }

      (void)sl_zigbee_lookup_node_id_by_eui64(esiEntry->eui64, &nodeId);
      sl_zigbee_af_fill_command_messaging_cluster_message_confirmation(messageTable[ep].messageId,
                                                                       sl_zigbee_af_get_current_time(),
                                                                       0x00,
                                                                       "");
      // The source and destination are reversed for the confirmation.
      sl_zigbee_af_set_command_endpoints(messageTable[ep].clientEndpoint, esiEntry->endpoint);
      status = ((sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId)
                 == SL_STATUS_OK)
                ? SL_ZIGBEE_ZCL_STATUS_SUCCESS
                : SL_ZIGBEE_ZCL_STATUS_FAILURE);
    }
  } else {
    sl_zigbee_af_messaging_cluster_println("Err: No Valid Msg");
  }
  return status;
}

uint32_t sl_zigbee_af_messaging_cluster_client_command_parse(sl_service_opcode_t opcode,
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
      case ZCL_CANCEL_MESSAGE_COMMAND_ID:
      {
        status = sl_zigbee_af_messaging_cluster_cancel_message_cb(cmd);
        break;
      }
    }
  }

  return status;
}
