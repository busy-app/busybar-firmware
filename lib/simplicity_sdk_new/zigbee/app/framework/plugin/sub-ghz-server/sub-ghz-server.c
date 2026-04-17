/***************************************************************************//**
 * @file
 * @brief Routines for the Sub-Ghz Server plugin.
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
#include "app/framework/util/af-main.h"
#include "app/framework/util/util.h"
#include "stack/include/sl_zigbee_duty_cycle.h"

#include "zap-cluster-command-parser.h"
#include "sub-ghz-server-config.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_OTA_SERVER_PRESENT
#include "ota-server.h"
#endif
#define sl_zigbee_duty_cycle_handler sli_zigbee_af_sub_ghz_server_duty_cycle_callback

#include "sub-ghz-server.h"

//TODO: Remove the temporarily included macro definition once EMHAL-2701 is resolved.
#define INT8U_TO_INT32U(byte3, byte2, byte1, byte0) ( \
    (((uint32_t) (byte3)) << 24)                      \
    + (((uint32_t) (byte2)) << 16)                    \
    + (((uint32_t) (byte1)) << 8)                     \
    + ((uint32_t) ((byte0) & 0xFFu))                  \
    )

#define PLUGIN_NAME_STR         "Sub-GHz server"

//------------------------------------------------------------------------------
// Internal declarations

// The Mgmt_NWK_Unsolicited_Enhanced_Update_notify command payload:
// Offset Length Desription
//    0     1    Sequence No
//    1     1    Status - always 0; pointless but consistent with other "notify" commands
//    2     4    Channel Mask - 32 bits, top 5 for page No, the rest a bitmask for channels
//    6     2    MAC Ucast Total
//    8     2    MAC Ucast Failures
//   10     2    MAC Ucast Retries
//   12     1    Period - in minutes
// The overall length of the command payload is 13.
#define NWK_UNSOLICITED_ENHANCED_UPDATE_NOTIFY_CMD_PAYLOAD_LENGTH       13

// An array of data to keep track of all the clients
typedef struct {
  // Client's node ID.
  // NOTE: The table is preinitialized on boot to all bytes 0, including node IDs.
  // That means all not yet assigned entries will have nodeId == 0. Fortunately,
  // the SE1.4 spec is on our side and assumes that the Sub-GHz server resides
  // on the comms hub.
  // NOTE: Once a table entry becomes used, it will never become "unused".
  sl_802154_short_addr_t nodeId;

  // Time until the current suspend period expires. 0 = not suspended.
  // Suspend period is 1-255 min, or up to 1530 sec. We need to track it in seconds,
  // because different nodes may be sent the suspend command at different times,
  // and thus the timer has to have a finer granularity than one minute.
  uint16_t suspendTime;
} sli_zigbee_client_table_t;
static sli_zigbee_client_table_t clientTable[SL_ZIGBEE_MAX_CHILDREN_FOR_PER_DEVICE_DUTY_CYCLE_MONITOR] = { 0 };

// As per SE 1.4, a Sub-GHz server, on the transition to the Limited Duty Cycle
// state, uses "an implementation defined method" to choose a client and suspends
// it.
// If that client tries to communicate whilst the server is still in the Limited
// state, the server suspends it again.
// This is the node Id of the choosen client.
static sl_802154_short_addr_t chosenNodeId = SL_ZIGBEE_NULL_NODE_ID;

// Seconds to minutes calculation
#define SECOND_TO_MINUTES(sec)  (((sec) + 59) / 60)

// The Sub-GHz cluster does not have an endpoint but our send function still
// needs one and fails if it is not provided. So we use endpoint 1 as default.
#define DEFAULT_ENDPOINT        1

//------------------------------------------------------------------------------
// Internal Functions

/** @brief Find a client with the given node ID
 *
 * Returns a pointer to the entry in the client table matching the given node ID
 * or NULL to indicate an entry was not found.
 */
static sli_zigbee_client_table_t* findClient(sl_802154_short_addr_t nodeId)
{
  uint32_t i;

  // Find an entry in the table with the same node ID
  for (i = 0; i < sizeof clientTable / sizeof clientTable[0]; ++i) {
    if (clientTable[i].nodeId == nodeId) {
      return &clientTable[i];
    }
  }

  // A matching entry not found.
  return NULL;
}

/** @brief Find a client with the given node ID with succeed guaranteed
 *
 * Return a pointer to the client table that can be, in the given order:
 * 1) an entry matching the given node ID;
 * 2) the first unused entry, if any;
 * 3) an entry vacated from the table to make space for the new entry.
 */
static sli_zigbee_client_table_t* findOrMakeClient(sl_802154_short_addr_t nodeId)
{
  sli_zigbee_client_table_t* client = findClient(nodeId);
  if (client == NULL) {
    // We need to find an entry to kick out, choosing the one with the shortest
    // remaining suspend time.
    uint16_t shortestSuspendTime = 0xFFFF;
    uint32_t i;
    for (i = 0; i < sizeof clientTable / sizeof clientTable[0]; ++i) {
      // Finding an unused entry has a priority...
      if (clientTable[i].nodeId == 0) {
        client = &clientTable[i];
        break;
      }
      // ...followed by one with the shortest remaining time to overwrite
      if (clientTable[i].suspendTime <= shortestSuspendTime) {
        client = &clientTable[i];
        shortestSuspendTime = clientTable[i].suspendTime;
      }
    }

    // Replacing a previously used client's entry, start from scratch
    client->nodeId = nodeId;
  }

  return client;
}

/** @brief Update client's remaining suspend time
 */
static void updateClientSuspendTime(sli_zigbee_client_table_t* client,
                                    uint8_t period)
{
  client->suspendTime = 60 * period;
  if (period != 0) {
    // Note: If the timer is already running, e.g. because another client is
    // already suspended, and there is let's say half a second to the next tick,
    // then this will reset that half a second to a full csecond. Fortunately,
    // a) our tick period is one second, so we will never get too far off, and
    // b) suspening new clients is (hopefully) not taking place very frequently
    sl_zigbee_zcl_schedule_server_tick(sl_zigbee_af_primary_endpoint_for_current_network_index(),
                                       ZCL_SUB_GHZ_CLUSTER_ID,
                                       MILLISECOND_TICKS_PER_SECOND);
  }
}

/** @brief Incoming ZDO message handler
 *
 * Calls the user's sl_zigbee_af_sub_ghz_unsolicited_enhanced_update_notify_cb
 * if the incoming message is Mgmt_NWK_Unsolicited_Enhanced_Update_notify.
 * That's it.
 */
void sli_zigbee_af_sub_ghz_server_zdo_message_received_callback(sl_802154_short_addr_t sender,
                                                                const uint8_t* payloadBuffer,
                                                                uint16_t payloadLength)
{
  if (payloadBuffer != NULL
      && payloadLength >= NWK_UNSOLICITED_ENHANCED_UPDATE_NOTIFY_CMD_PAYLOAD_LENGTH) {
    uint32_t channelMask;
    uint8_t channelPage;
    uint8_t channel;
    uint16_t macTxUcastTotal;
    uint16_t macTxUcastFailures;
    uint16_t macTxUcastRetries;
    uint8_t period;

    sl_zigbee_af_sub_ghz_cluster_println("%s: Mgmt_NWK_Unsolicited_Enhanced_Update_notify received",
                                         PLUGIN_NAME_STR);

    channelMask = INT8U_TO_INT32U(payloadBuffer[5], payloadBuffer[4], payloadBuffer[3], payloadBuffer[2]);
    channelPage = channelMask >> SL_ZIGBEE_MAX_CHANNELS_PER_PAGE;

    // Convert channel bitmask into a bit position (0x0000001 = 0, 0x04000000 = 26)
    for (channel = 0; channel < SL_ZIGBEE_MAX_CHANNELS_PER_PAGE && (channelMask & 0x01) == 0; ++channel) {
      channelMask >>= 1;
    }

    if (sli_zigbee_af_validate_channel_pages(channelPage, channel) != SL_STATUS_OK) {
      sl_zigbee_af_sub_ghz_cluster_println("Invalid channel mask: 0x%08X (page %d)",
                                           channelMask,
                                           channelPage);
      return;
    }

    macTxUcastTotal = HIGH_LOW_TO_INT(payloadBuffer[7], payloadBuffer[6]);
    macTxUcastFailures = HIGH_LOW_TO_INT(payloadBuffer[9], payloadBuffer[8]);
    macTxUcastRetries = HIGH_LOW_TO_INT(payloadBuffer[11], payloadBuffer[10]);
    period = payloadBuffer[12];

    sl_zigbee_af_sub_ghz_cluster_println("Current page      : %d", channelPage);
    sl_zigbee_af_sub_ghz_cluster_println("Current channel   : %d", channel);
    sl_zigbee_af_sub_ghz_cluster_println("MAC Tx Ucast Total: %d", macTxUcastTotal);
    sl_zigbee_af_sub_ghz_cluster_println("MAC Tx Ucast Fail : %d", macTxUcastFailures);
    sl_zigbee_af_sub_ghz_cluster_println("MAC Tx Ucast Retry: %d", macTxUcastRetries);
    sl_zigbee_af_sub_ghz_cluster_println("Sample period     : %d", period);

    // Leave it to the application to handle it
    sl_zigbee_af_sub_ghz_unsolicited_enhanced_update_notify_cb(channelPage,
                                                               channel,
                                                               macTxUcastTotal,
                                                               macTxUcastFailures,
                                                               macTxUcastRetries,
                                                               period);
  }
}

/** @brief Send the 'SuspendZclMessages command.
 */
static sl_status_t sendSuspendZclMessagesCommand(sl_802154_short_addr_t nodeId,
                                                 uint8_t period)
{
  sl_zigbee_af_fill_command_sub_ghz_cluster_suspend_zcl_messages(period);
  sl_zigbee_af_set_command_endpoints(sl_zigbee_af_primary_endpoint_for_current_network_index(),
                                     DEFAULT_ENDPOINT);
  return sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
}

//------------------------------------------------------------------------------
// Global APIs provided by the plugin

/** @brief Send the 'Suspend ZCL Messages' command.
 *
 * See prototype in sub-ghz-server.h for a detailed description.
 */
sl_status_t sl_zigbee_af_sub_ghz_server_send_suspend_zcl_messages_command(sl_802154_short_addr_t nodeId,
                                                                          uint8_t endpoint,
                                                                          uint8_t period)
{
  if (!sl_zigbee_is_zigbee_broadcast_address(nodeId)) {
    // Update the client's suspend time
    sli_zigbee_client_table_t* client = findOrMakeClient(nodeId);
    updateClientSuspendTime(client, period);
    return sendSuspendZclMessagesCommand(nodeId, period);
  } else {
    // Broadcast is not allowed
    return SL_STATUS_INVALID_PARAMETER;
  }
}

/** @brief Get the current suspend status for a given client.
 *
 * @return  the number of seconds remaining; 0 = not suspended
 */
uint16_t sl_zigbee_af_sub_ghz_server_suspend_zcl_messages_status(sl_802154_short_addr_t nodeId)
{
  const sli_zigbee_client_table_t* client = findClient(nodeId);
  return client ? client->suspendTime : 0;
}

//------------------------------------------------------------------------------
// Non-public global functions

/** @brief An incoming message handler, called from sli_zigbee_af_incoming_message_handler
 *
 * Based on the Duty Cycle state, client's ID (has the client been suspended?)
 * and the message type, determines the right action, which could be one of:
 * - let the message through to allow the framework to process it further;
 * - respond with 'Suspend ZCL Messages' and suppress this message
 * - suppress the message silently
 *
 * @return true if the message has been processed by the Sub-GHz plugin and
 *         should be suppressed, false to allow the message through
 *
 * Note:
 * We have to juggle some contradictory requirements:
 * - if the message is an OTA block request and we are in the "Critical" duty
 *   cycle mode, respond with "Wait for data"
 * - otherwise, if the message came from a suspended client, respond with
 *   "Suspend ZCL Messages"...
 * - ...UNLESS the message is a Default Response to a previous suspend command,
 *   to prevent a message loop.
 * This means we have to at least partially parse the incoming message BEFORE
 * we decide whether to let it through or not for parsing.
 */
bool sli_zigbee_af_sub_ghz_server_incoming_message(sl_zigbee_incoming_message_type_t type,
                                                   sl_zigbee_aps_frame_t *apsFrame,
                                                   sl_802154_short_addr_t sender,
                                                   uint16_t messageLength,
                                                   uint8_t *messageContents)
{
  sl_zigbee_duty_cycle_state_t dcState;
  sli_zigbee_client_table_t* client = findOrMakeClient(sender);

  if (sl_zigbee_get_duty_cycle_state(&dcState) != SL_STATUS_OK) {
    dcState = SL_ZIGBEE_DUTY_CYCLE_TRACKING_OFF;    // fallback to a sensible default
  } else if (client->suspendTime == 0
             && (dcState >= SL_ZIGBEE_DUTY_CYCLE_LBT_CRITICAL_THRESHOLD_REACHED
                 || (dcState >= SL_ZIGBEE_DUTY_CYCLE_LBT_LIMITED_THRESHOLD_REACHED
                     && sender == chosenNodeId))) {
    // Client's expired suspension needs to be renewed
    updateClientSuspendTime(client, SL_ZIGBEE_AF_PLUGIN_SUB_GHZ_SERVER_SUSPEND_PERIOD);
  }

  if (client->suspendTime > 0) {
    // First, sort out special cases like the OTA block requests and default
    // responses to previous suspend commands
    if (apsFrame->profileId == SE_PROFILE_ID
        && (apsFrame->clusterId == ZCL_OTA_BOOTLOAD_CLUSTER_ID
            || apsFrame->clusterId == ZCL_SUB_GHZ_CLUSTER_ID)) {
      sl_zigbee_af_cluster_command_t cmd;
      if (sl_zigbee_af_process_message_into_zcl_cmd(apsFrame,
                                                    type,
                                                    messageContents,
                                                    messageLength,
                                                    sender,
                                                    NULL, // interPanHeader, not needed here
                                                    &cmd)) {
# ifdef SL_CATALOG_ZIGBEE_OTA_SERVER_PRESENT
        if (dcState <= SL_ZIGBEE_DUTY_CYCLE_LBT_LIMITED_THRESHOLD_REACHED
            && apsFrame->clusterId == ZCL_OTA_BOOTLOAD_CLUSTER_ID
            && cmd.direction == ZCL_DIRECTION_CLIENT_TO_SERVER
            && cmd.clusterSpecific) {
          // If Server is in Limited DC then it should still continue to process any message related to OTA cluster
          return false;
        }
        if (dcState >= SL_ZIGBEE_DUTY_CYCLE_LBT_CRITICAL_THRESHOLD_REACHED
            && apsFrame->clusterId == ZCL_OTA_BOOTLOAD_CLUSTER_ID
            && cmd.direction == ZCL_DIRECTION_CLIENT_TO_SERVER
            && cmd.clusterSpecific
            && cmd.commandId == ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID) {
          sl_zigbee_af_core_println("%s: client %04X %s within its suspend period.",
                                    PLUGIN_NAME_STR,
                                    sender,
                                    "requesting an OTA block");
          // Leave the responding with "wait for data" to the OTA plugin itself
          return false;
        } else
# endif // SL_CATALOG_ZIGBEE_OTA_SERVER_PRESENT
        if (apsFrame->clusterId == ZCL_SUB_GHZ_CLUSTER_ID
            && cmd.direction == ZCL_DIRECTION_CLIENT_TO_SERVER
            && !cmd.clusterSpecific
            && cmd.commandId == ZCL_DEFAULT_RESPONSE_COMMAND_ID) {
          // Silently drop a default response to a Sub-GHz server command
          // to prevent an infinite "suspend -> default response" message loop
          return true;
        }
      }
    }

    // Special cases sorted, back to the general case: send "Suspend ZCL Messages"
    if (sender != sl_zigbee_af_get_node_id()) { // do not accidentally suspend ourselves
      sl_zigbee_af_core_println("%s: client %04X %s within its suspend period.",
                                PLUGIN_NAME_STR,
                                sender,
                                "transmitting");
      sendSuspendZclMessagesCommand(sender, SECOND_TO_MINUTES(client->suspendTime));
    }
    return true;
  }

  // Message not handled by this plugin
  return false;
}

//------------------------------------------------------------------------------
// Sub-GHz Library Callbacks

/** @brief Duty cycle state change callback, code shared between SoC and NCP.
 */
static void jointDutyCycleHandler(uint8_t channelPage,
                                  uint8_t channel,
                                  sl_zigbee_duty_cycle_state_t newState,
                                  uint8_t totalDevices,
                                  sl_zigbee_per_device_duty_cycle_t *arrayOfDutyCycles)
{
  static uint8_t /*sl_zigbee_duty_cycle_state_t*/ oldState = SL_ZIGBEE_DUTY_CYCLE_TRACKING_OFF;

  sl_zigbee_af_sub_ghz_cluster_println("%s: Duty Cycle handler called: page=%d, chan=%d, state=%d (previous=%d)",
                                       PLUGIN_NAME_STR,
                                       channelPage,
                                       channel,
                                       newState,
                                       oldState);

  // Consult the user if they want to override the default decision
  if (sl_zigbee_af_sub_ghz_duty_cycle_cb(channelPage,
                                         channel,
                                         oldState,
                                         newState)) {
    // The callback returning true means the user app has handled the callback
    // which means there is nothing for us to do.
  } else if (oldState < SL_ZIGBEE_DUTY_CYCLE_LBT_CRITICAL_THRESHOLD_REACHED
             && newState >= SL_ZIGBEE_DUTY_CYCLE_LBT_CRITICAL_THRESHOLD_REACHED) {
    // If the transition is from "Limited" to "Critical", suspend everyone.
    uint8_t i;
    for (i = 0; i < sl_zigbee_af_get_child_table_size(); ++i) {
      sl_802154_long_addr_t senderEui64;
      sl_802154_short_addr_t shortId;
      sl_zigbee_get_address_table_info(i, &shortId, senderEui64);
      sl_802154_short_addr_t const nodeId = shortId;
      if (nodeId != SL_ZIGBEE_UNKNOWN_NODE_ID
          && nodeId != SL_ZIGBEE_DISCOVERY_ACTIVE_NODE_ID
          && nodeId != SL_ZIGBEE_TABLE_ENTRY_UNUSED_NODE_ID) {
        sl_zigbee_af_sub_ghz_server_send_suspend_zcl_messages_command(nodeId,
                                                                      0, // use the saved EP (default to 1 if unknown)
                                                                      SL_ZIGBEE_AF_PLUGIN_SUB_GHZ_SERVER_SUSPEND_PERIOD);
      }
    }
  } else if (oldState < SL_ZIGBEE_DUTY_CYCLE_LBT_LIMITED_THRESHOLD_REACHED
             && newState >= SL_ZIGBEE_DUTY_CYCLE_LBT_LIMITED_THRESHOLD_REACHED) {
    // If the transition is from "Normal" to "Limited", find a single client to suspend.
    // We choose the most talkative client that has not been suspended yet.
    // Failing that, we choose the one with the shortest remaining suspend time.
    uint8_t i;
    sl_zigbee_duty_cycle_hecto_pct_t highestsDutyCycle = 0;

    chosenNodeId = SL_ZIGBEE_NULL_NODE_ID;

    // Iterate through the duty cycle results from the stack to find the candidate.
    // It must be one that has not been suspended yet.
    // Start from 1 to skip the cumulative duty cycle for the local device.
    for (i = 1; i < totalDevices; ++i) {
      const sl_802154_short_addr_t currentNode = arrayOfDutyCycles[i].nodeId;
      const sl_zigbee_duty_cycle_hecto_pct_t currentDutyCycle = arrayOfDutyCycles[i].dutyCycleConsumed;

      sl_zigbee_af_sub_ghz_cluster_println("%s: Checking client %04X, %s = %d",
                                           PLUGIN_NAME_STR,
                                           currentNode,
                                           "duty cycle",
                                           currentDutyCycle);

      if (currentNode != 0xFFFF                 // skip unused entries
          && currentNode != sl_zigbee_af_get_node_id()  // skip ourselves
          && currentDutyCycle > highestsDutyCycle) {
        // Cross-check the node with our list. It can be used if unknown to us yet
        // (i.e. it is not suspended) or if the remaining suspend period is 0.
        const sli_zigbee_client_table_t* client = findClient(currentNode);
        if (!client
            || client->suspendTime == 0) {
          highestsDutyCycle = currentDutyCycle;
          chosenNodeId = currentNode;
          sl_zigbee_af_sub_ghz_cluster_println("%s: Found new candidate to suspend",
                                               PLUGIN_NAME_STR);
        }
      }
    }

    // If we have not found the candidate in the pool of duty cycle results
    // from the stack, we need to pick one with the least suspend time remaining.
    if (chosenNodeId == SL_ZIGBEE_NULL_NODE_ID) {
      uint16_t shortestSuspendTime = 0xFFFF;
      for (i = 0; i < sizeof clientTable / sizeof clientTable[0]; ++i) {
        sl_zigbee_af_sub_ghz_cluster_println("%s: Checking client %04X, %s = %d",
                                             PLUGIN_NAME_STR,
                                             clientTable[i].nodeId,
                                             "time left",
                                             clientTable[i].suspendTime);
        if (clientTable[i].nodeId != 0
            && clientTable[i].suspendTime < shortestSuspendTime) {
          shortestSuspendTime = clientTable[i].suspendTime;
          chosenNodeId = clientTable[i].nodeId;
          sl_zigbee_af_sub_ghz_cluster_println("%s: Found new candidate to suspend",
                                               PLUGIN_NAME_STR);
        }
      }
    }

    // There is still a chance that we have not found a suitable client, however
    // the only possible reason at this point is that our clients table is still
    // empty (unlikely given we are inside the "duty cycle limit reached" callback).
    if (chosenNodeId != SL_ZIGBEE_NULL_NODE_ID) {
      sl_zigbee_af_sub_ghz_cluster_println("%s: Suspending client %04X",
                                           PLUGIN_NAME_STR,
                                           chosenNodeId);
      sl_zigbee_af_sub_ghz_server_send_suspend_zcl_messages_command(chosenNodeId,
                                                                    0, // use the saved EP (default to 1 if unknown)
                                                                    SL_ZIGBEE_AF_PLUGIN_SUB_GHZ_SERVER_SUSPEND_PERIOD);
    }
  }

  oldState = newState;
}

/** @brief A duty cycle state change callback
 *
 */
void sli_zigbee_af_sub_ghz_server_duty_cycle_callback(uint8_t channelPage,
                                                      uint8_t channel,
                                                      sl_zigbee_duty_cycle_state_t newState,
                                                      uint8_t bogo_totalDevices,
                                                      sl_zigbee_per_device_duty_cycle_t *bogo_arrayOfDutyCycles)
{
// Uncomment once EMZIGBEE-1173 is fixed
// #ifndef EZSP_HOST
  sl_zigbee_per_device_duty_cycle_t arrayOfDutyCycles[SL_ZIGBEE_MAX_CHILDREN_FOR_PER_DEVICE_DUTY_CYCLE_MONITOR + 1];
  uint8_t totalDevices;

  // emAfPluginSubGhzServerDutyCycleHandler above uses arrayOfDutyCycles only
  // on the transition from the "normal" to "limited" state. If the callback is
  // about a different transition, we do not need to waste time querying the
  // per-node cycles and can use any value for totalDevices.
  if (newState >= SL_ZIGBEE_DUTY_CYCLE_LBT_LIMITED_THRESHOLD_REACHED
      && newState < SL_ZIGBEE_DUTY_CYCLE_LBT_CRITICAL_THRESHOLD_REACHED
      && sl_zigbee_get_current_duty_cycle(sizeof arrayOfDutyCycles / sizeof arrayOfDutyCycles[0],
                                          arrayOfDutyCycles) == SL_STATUS_OK) {
    totalDevices = sizeof arrayOfDutyCycles / sizeof arrayOfDutyCycles[0];
  } else {
    totalDevices = 0;
  }
// Uncomment once EMZIGBEE-1173 is fixed
// #endif

  jointDutyCycleHandler(channelPage,
                        channel,
                        newState,
                        totalDevices,
                        arrayOfDutyCycles);
}

/** @brief Sub-GHz Cluster Server Tick
 *
 * Server Tick.
 * Used for keeping the Suspend ZCL Messages counters up to date.
 */
void sl_zigbee_af_sub_ghz_cluster_server_tick_cb(uint8_t endpoint)
{
  bool timerStillNeeded = false;
  uint8_t i;

  // Go through the table of suspended nodes and decrement the timers
  for (i = 0; i < sizeof clientTable / sizeof clientTable[0]; ++i) {
    if (clientTable[i].suspendTime > 0) {
      if (--clientTable[i].suspendTime > 0) {
        timerStillNeeded = true;
      }
    }
  }

  // Schedule the next tick if required
  if (timerStillNeeded) {
    sl_zigbee_zcl_schedule_server_tick(endpoint, ZCL_SUB_GHZ_CLUSTER_ID, MILLISECOND_TICKS_PER_SECOND);
  }
}

//-----------------------
// ZCL commands callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_sub_ghz_cluster_get_suspend_zcl_messages_status_cb(void)
{
  sl_zigbee_af_cluster_command_t *cmd = sl_zigbee_af_current_command();
  if (cmd) {    // sanity, since we dereference cmd
    const uint16_t suspendTime = sl_zigbee_af_sub_ghz_server_suspend_zcl_messages_status(cmd->source);
    sendSuspendZclMessagesCommand(cmd->source, SECOND_TO_MINUTES(suspendTime));
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

uint32_t sl_zigbee_af_sub_ghz_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                           sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific && cmd->commandId == ZCL_GET_SUSPEND_ZCL_MESSAGES_STATUS_COMMAND_ID) {
    status = sl_zigbee_af_sub_ghz_cluster_get_suspend_zcl_messages_status_cb();
  }

  return status;
}
