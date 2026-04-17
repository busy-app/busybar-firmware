/***************************************************************************//**
 * @file
 * @brief Routines for the Sub-Ghz Client plugin.
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
#include "app/framework/util/util.h"
#include "app/framework/util/af-main.h"                 // sli_zigbee_af_send()
#include "app/util/zigbee-framework/zigbee-device-common.h"      // sl_zigbee_next_zig_dev_request_sequence()

#include "sub-ghz-client.h"
#ifdef SL_CATALOG_ZIGBEE_TRUST_CENTER_KEEPALIVE_PRESENT
#include "trust-center-keepalive.h"
#endif // SL_CATALOG_ZIGBEE_TRUST_CENTER_KEEPALIVE_PRESENT

#include "zap-cluster-command-parser.h"

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

// A bitmask representing the current suspend status
#define SUSPEND_STATUS_SUSPENDED        0x01
#define SUSPEND_STATUS_SUSPEND_IGNORED  0x02
static uint8_t suspendStatus = 0;

//------------------------------------------------------------------------------
// Global APIs provided by the plugin

/** Send the 'Get Suspend ZCL Messages Status' command.
 *
 * See prototype in sub-ghz-client.h for a detailed description.
 */
sl_status_t sl_zigbee_af_sub_ghz_client_send_get_suspend_zcl_messages_status_command(sl_802154_short_addr_t nodeId,
                                                                                     uint8_t endpoint)
{
  const uint8_t sourceEndpoint = sl_zigbee_af_primary_endpoint_for_current_network_index();
  sl_zigbee_af_fill_command_sub_ghz_cluster_get_suspend_zcl_messages_status();
  sl_zigbee_af_set_command_endpoints(sourceEndpoint,
                                     endpoint);
  return sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
}

/** @brief Sends a request to change the channel to the server by means of the
 * Mgmt_NWK_Unsolicited_Enhanced_Update_notify command.
 *
 * See prototype in sub-ghz-client.h for a detailed description.
 */
sl_status_t sl_zigbee_af_sub_ghz_client_send_unsolicited_enhanced_update_notify(sl_802154_short_addr_t destination,
                                                                                uint8_t channelPage,
                                                                                uint8_t channel,
                                                                                uint16_t macTxUcastTotal,
                                                                                uint16_t macTxUcastFailures,
                                                                                uint16_t macTxUcastRetries,
                                                                                uint8_t period)
{
  sl_zigbee_aps_frame_t apsFrame = {
    SL_ZIGBEE_ZDO_PROFILE_ID,
    NWK_UNSOLICITED_ENHANCED_UPDATE_NOTIFY,
    SL_ZIGBEE_ZDO_ENDPOINT,
    SL_ZIGBEE_ZDO_ENDPOINT,
    0,  // no APS ACK, otherwise use SL_ZIGBEE_APS_OPTION_RETRY
    0,  // group id
    0,  // sequence
    0   // radius
  };
  uint16_t messageTag = 0xFF;    // INVALID_MESSAGE_TAG
  uint32_t channelMask;
  uint8_t payload[NWK_UNSOLICITED_ENHANCED_UPDATE_NOTIFY_CMD_PAYLOAD_LENGTH];
  uint8_t i = 0;

  // Sanity check. Allow only page 0, 28, 29, 30 or 31 and only channel 0-26.
  if (sli_zigbee_af_validate_channel_pages(channelPage, channel) != SL_STATUS_OK) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Broadcast not allowed. The spec says we unicast this msg to the network manager.
  // We still leave the destination open for testing purposes.
  if (sl_zigbee_is_zigbee_broadcast_address(destination)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  channelMask = (channelPage << SL_ZIGBEE_MAX_CHANNELS_PER_PAGE) | BIT32(channel);

  payload[i++] = sl_zigbee_next_zig_dev_request_sequence();
  payload[i++] = 0;     // status code, always success
  payload[i++] = BYTE_0(channelMask);
  payload[i++] = BYTE_1(channelMask);
  payload[i++] = BYTE_2(channelMask);
  payload[i++] = BYTE_3(channelMask);
  payload[i++] = LOW_BYTE(macTxUcastTotal);
  payload[i++] = HIGH_BYTE(macTxUcastTotal);
  payload[i++] = LOW_BYTE(macTxUcastFailures);
  payload[i++] = HIGH_BYTE(macTxUcastFailures);
  payload[i++] = LOW_BYTE(macTxUcastRetries);
  payload[i++] = HIGH_BYTE(macTxUcastRetries);
  payload[i++] = period;

  assert(i == NWK_UNSOLICITED_ENHANCED_UPDATE_NOTIFY_CMD_PAYLOAD_LENGTH);

  return sli_zigbee_af_send(SL_ZIGBEE_OUTGOING_DIRECT,
                            destination,
                            &apsFrame,
                            sizeof payload,
                            payload,
                            &messageTag,
                            0, // alias
                            0); // sequence
}

/** Return the current suspend status.
 *
 * @return  true if called within a suspend period, false otherwise
 */
bool sl_zigbee_af_sub_ghz_client_is_sending_zcl_messages_suspended()
{
  return ((suspendStatus & (SUSPEND_STATUS_SUSPENDED | SUSPEND_STATUS_SUSPEND_IGNORED)) == SUSPEND_STATUS_SUSPENDED);
}

/** @brief Ignore the SuspendZclMessages command.
 *
 * When the client receives the SuspendZclMessages command, it suspends sending
 * any ZCL messages for a given time. This function allows the client to ignore
 * the command and continue sending messages even when suspended.
 *
 * @param ignore  true to ignore, false to switch back to normal behaviour
 */
void sl_zigbee_af_sub_ghz_client_ignore_suspend_zcl_messages_command(bool ignore)
{
  if (ignore) {
    suspendStatus |= SUSPEND_STATUS_SUSPEND_IGNORED;
  } else {
    suspendStatus &= ~SUSPEND_STATUS_SUSPEND_IGNORED;
  }
}

//------------------------------------------------------------------------------
// Sub-GHz Library Callbacks

/** @brief Sub-GHz Cluster Client Tick
 *
 * Client Tick.
 * Used for timing out the client's suspend state.
 * The timer is started in sl_zigbee_af_sub_ghz_cluster_suspend_zcl_messages_cb above,
 * set to trigger after a given number of minutes. So triggering the timer
 * simply means our suspend period is over and we can start sending messages again.
 */
void sl_zigbee_af_sub_ghz_cluster_client_tick_cb(uint8_t endpoint)
{
  UNUSED_VAR(endpoint);

  // Cancel the suspend status. That's it.
  suspendStatus &= ~SUSPEND_STATUS_SUSPENDED;
#ifdef SL_CATALOG_ZIGBEE_TRUST_CENTER_KEEPALIVE_PRESENT
  sl_zigbee_af_trust_center_keepalive_enable();
#endif

  sl_zigbee_af_debug_println("%s resumed",
                             "Sub-GHz client: sending of ZCL messages");
}

//-----------------------
// ZCL commands callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_sub_ghz_cluster_suspend_zcl_messages_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_sub_g_hz_cluster_suspend_zcl_messages_command_t cmd_data;
  // Check we are on the sub-GHz interface
  sl_zigbee_node_type_t nodeType;
  sl_zigbee_network_parameters_t parameters;
  if (sl_zigbee_af_get_network_parameters(&nodeType, &parameters) != SL_STATUS_OK) {
    sl_zigbee_af_debug_println("Error: Could not determine node type");
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  } else if (parameters.radioChannel < 128) {
    sl_zigbee_af_debug_println("Ignoring SuspendZclMessages received on a 2.4GHz channel");
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (zcl_decode_sub_g_hz_cluster_suspend_zcl_messages_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (!sl_zigbee_af_sub_ghz_suspend_zcl_messages_cb(cmd_data.period)) {
    // Suspend ZCL messages for 'period' minutes.
    suspendStatus |= SUSPEND_STATUS_SUSPENDED;
#ifdef SL_CATALOG_ZIGBEE_TRUST_CENTER_KEEPALIVE_PRESENT
    sl_zigbee_af_trust_center_keepalive_disable();
#endif

    sl_zigbee_af_debug_println("%s suspended for %d minutes",
                               "Sub-GHz client: sending of ZCL messages",
                               cmd_data.period);

    // Set a timer to clear the suspend status.
    const uint8_t sourceEndpoint = sl_zigbee_af_primary_endpoint_for_current_network_index();
    sl_zigbee_zcl_schedule_client_tick(sourceEndpoint, ZCL_SUB_GHZ_CLUSTER_ID,
                                       cmd_data.period * MILLISECOND_TICKS_PER_MINUTE);
  }

  if (cmd_data.period == 0) {
    // Send Default Response
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

uint32_t sl_zigbee_af_sub_ghz_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                           sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific && cmd->commandId == ZCL_SUSPEND_ZCL_MESSAGES_COMMAND_ID) {
    status = sl_zigbee_af_sub_ghz_cluster_suspend_zcl_messages_cb(cmd);
  }

  return status;
}
