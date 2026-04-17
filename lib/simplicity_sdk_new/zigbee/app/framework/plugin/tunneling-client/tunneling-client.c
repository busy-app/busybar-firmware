/***************************************************************************//**
 * @file
 * @brief Routines for the Tunneling Client plugin.
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
#include "tunneling-client.h"
#include "app/framework/plugin/address-table/address-table.h"

#include "zap-cluster-command-parser.h"
#include "tunneling-client-config.h"

#define UNUSED_ENDPOINT_ID 0xFF

// If addressIndex is SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX and serverEndpoint is
// UNUSED_ENDPOINT_ID, then the entry is unused and available for use by a new
// tunnel.  Occasionally, serverEndpoint will be UNUSED_ENDPOINT_ID, but
// addressIndex will contain a valid index.  This happens after a tunnel is
// removed but before the address table entry has been cleaned up.  There is a
// delay between closure and cleanup to allow the stack to continue using the
// address table entry to send messages to the server.
typedef struct {
  /* addressIndex refers to the index of addr table within the Addr Table Plugin */
  uint8_t   addressIndex;
  uint8_t   clientEndpoint;
  uint8_t   serverEndpoint;
  uint16_t  tunnelId;
  uint8_t   protocolId;
  uint16_t  manufacturerCode;
  bool flowControlSupport;
} sli_zigbee_af_tunneling_client_tunnel;

static sli_zigbee_af_tunneling_client_tunnel tunnels[SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT];

static uint8_t pendingIndex = SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX;

static bool findIeeeAddress(sl_802154_short_addr_t server);
static void ieeeAddressCallback(const sl_zigbee_af_service_discovery_result_t *result);
static bool createAddressTableEntry(sli_zigbee_af_tunneling_client_tunnel *tunnel,
                                    sl_802154_long_addr_t eui64);
static bool haveLinkKey(const sli_zigbee_af_tunneling_client_tunnel *tunnel);
static bool partnerLinkKeyExchange(const sli_zigbee_af_tunneling_client_tunnel *tunnel);
static void partnerLinkKeyExchangeCallback(bool success);
static bool sendRequestTunnel(const sli_zigbee_af_tunneling_client_tunnel *tunnel);
static void cleanUp(sl_zigbee_af_plugin_tunneling_client_status_t status);
static sl_zigbee_af_status_t clientFindTunnel(uint16_t tunnelId,
                                              uint8_t addressIndex,
                                              uint8_t clientEndpoint,
                                              uint8_t serverEndpoint,
                                              sli_zigbee_af_tunneling_client_tunnel **tunnel,
                                              uint8_t *tunnelIndex);

void sl_zigbee_af_tunneling_cluster_client_init_cb(uint8_t endpoint)
{
  uint8_t i;
  UNUSED_VAR(endpoint);
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT; i++) {
    tunnels[i].addressIndex = SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX;
    tunnels[i].serverEndpoint = UNUSED_ENDPOINT_ID;
  }
}

void sl_zigbee_af_tunneling_cluster_client_tick_cb(uint8_t endpoint)
{
  uint8_t i;

  if (pendingIndex != SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX
      && tunnels[pendingIndex].clientEndpoint == endpoint) {
    cleanUp(SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_REQUEST_TUNNEL_TIMEOUT);
    return;
  }

  // Search the table for unused entries that still have an address table
  // index.  These tunnels have been closed, but the address table entry was
  // not immediately removed so the stack could continue using it.  By this
  // point, we've given the stack a fair shot to use it, so now remove the
  // address table entry.
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT; i++) {
    if (tunnels[i].addressIndex != SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX
        && tunnels[i].clientEndpoint == endpoint
        && tunnels[i].serverEndpoint == UNUSED_ENDPOINT_ID) {
      sl_zigbee_af_address_table_remove_entry_by_index(tunnels[i].addressIndex);
      tunnels[i].addressIndex = SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX;
    }
  }
}

void sl_zigbee_af_tunneling_client_stack_status_cb(sl_status_t status)
{
  uint8_t i;

  if (status == SL_STATUS_NETWORK_DOWN
      && !sl_zigbee_is_performing_rejoin()) {
    sl_zigbee_af_simple_metering_cluster_println("Re-initializing tunnels due to stack down.");
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT; i++) {
      sl_zigbee_af_tunneling_client_cleanup(i);
    }
  }
}

sl_zigbee_af_plugin_tunneling_client_status_t sl_zigbee_af_tunneling_client_request_tunnel(sl_802154_short_addr_t server,
                                                                                           uint8_t clientEndpoint,
                                                                                           uint8_t serverEndpoint,
                                                                                           uint8_t protocolId,
                                                                                           uint16_t manufacturerCode,
                                                                                           bool flowControlSupport)
{
  uint8_t i;

  // Only one outgoing request is allowed at a time.
  if (pendingIndex != SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX) {
    return SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_BUSY;
  }

  // TODO: Implement support for flow control.
  if (flowControlSupport) {
    return SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_FLOW_CONTROL_NOT_SUPPORTED;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT; i++) {
    if (tunnels[i].addressIndex == SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX
        && tunnels[i].serverEndpoint == UNUSED_ENDPOINT_ID) {
      sl_802154_long_addr_t eui64;
      sli_zigbee_af_tunneling_client_tunnel tunnel;
      tunnel.clientEndpoint = clientEndpoint;
      tunnel.serverEndpoint = serverEndpoint;
      // The node id of the server is tucked away in the tunnel id field so we
      // can easily print it during setup.  Once the tunnel is opened, we'll
      // overwrite it.
      tunnel.tunnelId = server;
      tunnel.protocolId = protocolId;
      tunnel.manufacturerCode = manufacturerCode;
      tunnel.flowControlSupport = flowControlSupport;

      // If we know the long address of the server and have a link key to it,
      // we can send the request right now.  Otherwise, we need discover the
      // long address or exchange keys before proceeding.  If an operation
      // fails along the way, we give up.
      if (sl_zigbee_lookup_eui64_by_node_id(server, eui64) == SL_STATUS_OK) {
        if (!createAddressTableEntry(&tunnel, eui64)) {
          return SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_ADDRESS_TABLE_FULL;
        }
        if (haveLinkKey(&tunnel)) {
          if (!sendRequestTunnel(&tunnel)) {
            sl_zigbee_af_address_table_remove_entry_by_index(tunnel.addressIndex);
            return SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_REQUEST_TUNNEL_FAILED;
          }
        } else if (!partnerLinkKeyExchange(&tunnel)) {
          sl_zigbee_af_address_table_remove_entry_by_index(tunnel.addressIndex);
          return SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_LINK_KEY_EXCHANGE_REQUEST_FAILED;
        }
      } else if (!findIeeeAddress(server)) {
        return SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_IEEE_ADDRESS_REQUEST_FAILED;
      }

      // If we made it here, we either managed to send out the request or we're
      // waiting for a long address lookup or link key exchange to finish.
      // Either way, we can save the tunnel to the table and return.
      pendingIndex = i;
      memmove(&tunnels[i], &tunnel, sizeof(sli_zigbee_af_tunneling_client_tunnel));
      return SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_SUCCESS;
    }
  }

  return SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_NO_MORE_TUNNEL_IDS;
}

static bool findIeeeAddress(sl_802154_short_addr_t server)
{
  sl_status_t status = sl_zigbee_af_find_ieee_address(server, ieeeAddressCallback);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_tunneling_cluster_println("ERR: Could not request EUI64"
                                           " for node 0x%04X: 0x%02X",
                                           server,
                                           status);
  }
  return (status == SL_STATUS_OK);
}

static void ieeeAddressCallback(const sl_zigbee_af_service_discovery_result_t *result)
{
  if (!sl_zigbee_af_have_discovery_response_status(result->status)) {
    sl_zigbee_af_tunneling_cluster_println("ERR: Could not find EUI64"
                                           " for node 0x%04X",
                                           tunnels[pendingIndex].tunnelId);
    cleanUp(SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_IEEE_ADDRESS_NOT_FOUND);
  } else if (!createAddressTableEntry(&tunnels[pendingIndex],
                                      (uint8_t *)result->responseData)) {
    cleanUp(SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_ADDRESS_TABLE_FULL);
  } else if (haveLinkKey(&tunnels[pendingIndex])) {
    if (!sendRequestTunnel(&tunnels[pendingIndex])) {
      cleanUp(SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_REQUEST_TUNNEL_FAILED);
    }
  } else if (!partnerLinkKeyExchange(&tunnels[pendingIndex])) {
    cleanUp(SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_LINK_KEY_EXCHANGE_REQUEST_FAILED);
  }
}

static bool createAddressTableEntry(sli_zigbee_af_tunneling_client_tunnel *tunnel,
                                    sl_802154_long_addr_t eui64)
{
  tunnel->addressIndex = sl_zigbee_af_address_table_add_entry(eui64);
  if (tunnel->addressIndex == SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX) {
    sl_zigbee_af_tunneling_cluster_println("ERR: Could not create address"
                                           " table entry for node 0x%04X",
                                           tunnel->tunnelId);
  }
  return (tunnel->addressIndex != SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX);
}

static void transferDataMessageSentCallback(sl_zigbee_outgoing_message_type_t type,
                                            uint16_t indexOrDestination,
                                            sl_zigbee_aps_frame_t *apsFrame,
                                            uint16_t msgLen,
                                            uint8_t *message,
                                            sl_status_t status)
{
  UNUSED_VAR(type);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_tunneling_client_transfer_data_failure_cb(indexOrDestination,
                                                           apsFrame,
                                                           msgLen,
                                                           message,
                                                           status);
  }
}

static bool haveLinkKey(const sli_zigbee_af_tunneling_client_tunnel *tunnel)
{
  // We assume we always have a link with the server if one of us is the trust
  // center.  We may already have a link key with other nodes, but we don't
  // know if the other side still has the key, so it's safer to assume we don't
  // have a link key in that case and instead always request a new one.
  return (sl_zigbee_af_get_node_id() == SL_ZIGBEE_TRUST_CENTER_NODE_ID
          || (sl_zigbee_af_address_table_lookup_node_id_by_index(tunnel->addressIndex)
              == SL_ZIGBEE_TRUST_CENTER_NODE_ID));
}

static bool partnerLinkKeyExchange(const sli_zigbee_af_tunneling_client_tunnel *tunnel)
{
  sl_status_t status = sl_zigbee_af_initiate_partner_link_key_exchange_cb(tunnel->tunnelId,
                                                                          tunnel->serverEndpoint,
                                                                          partnerLinkKeyExchangeCallback);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_tunneling_cluster_println("ERR: Could not request link key exchange"
                                           " with node 0x%04X: 0x%02X",
                                           tunnel->tunnelId,
                                           status);
  }
  return (status == SL_STATUS_OK);
}

static void partnerLinkKeyExchangeCallback(bool success)
{
  if (!success) {
    sl_zigbee_af_tunneling_cluster_println("ERR: Link key exchange"
                                           " with node 0x%04X failed",
                                           tunnels[pendingIndex].tunnelId);
    cleanUp(SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_LINK_KEY_EXCHANGE_FAILED);
  } else if (!sendRequestTunnel(&tunnels[pendingIndex])) {
    cleanUp(SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_REQUEST_TUNNEL_FAILED);
  }
}

static bool sendRequestTunnel(const sli_zigbee_af_tunneling_client_tunnel *tunnel)
{
  sl_status_t status;
  sl_zigbee_af_fill_command_tunneling_cluster_request_tunnel(tunnel->protocolId,
                                                             tunnel->manufacturerCode,
                                                             tunnel->flowControlSupport,
                                                             SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_MAXIMUM_INCOMING_TRANSFER_SIZE);
  sl_zigbee_af_set_command_endpoints(tunnel->clientEndpoint, tunnel->serverEndpoint);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_VIA_ADDRESS_TABLE,
                                             tunnel->addressIndex);
  if (status == SL_STATUS_OK) {
    sl_zigbee_zcl_schedule_client_tick_extended(tunnel->clientEndpoint,
                                                ZCL_TUNNELING_CLUSTER_ID,
                                                (SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_TIMEOUT_SECONDS
                                                 * MILLISECOND_TICKS_PER_SECOND),
                                                SL_ZIGBEE_AF_SHORT_POLL,
                                                SL_ZIGBEE_AF_OK_TO_SLEEP);
  } else {
    sl_zigbee_af_tunneling_cluster_println("ERR: Could not send RequestTunnel: 0x%02X",
                                           status);
  }
  return (status == SL_STATUS_OK);
}

static void cleanUp(sl_zigbee_af_plugin_tunneling_client_status_t status)
{
  if (tunnels[pendingIndex].addressIndex != SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX) {
    sl_zigbee_af_address_table_remove_entry_by_index(tunnels[pendingIndex].addressIndex);
    tunnels[pendingIndex].addressIndex = SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX;
  }
  tunnels[pendingIndex].serverEndpoint = UNUSED_ENDPOINT_ID;
  pendingIndex = SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX;
  sl_zigbee_af_tunneling_client_tunnel_opened_cb(SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX,
                                                 status,
                                                 0xFFFF);      // transfer size
}

//-----------------------
// ZCL commands callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_tunneling_cluster_request_tunnel_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_tunneling_cluster_request_tunnel_response_command_t cmd_data;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_FAILURE;

  if (zcl_decode_tunneling_cluster_request_tunnel_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_tunneling_cluster_println("RX: RequestTunnelResponse 0x%04X, 0x%02X, 0x%04X",
                                         cmd_data.tunnelId,
                                         cmd_data.tunnelStatus,
                                         cmd_data.maximumIncomingTransferSize);

  if (pendingIndex != SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX
      && (tunnels[pendingIndex].addressIndex
          == sli_zigbee_af_get_address_index())
      && (tunnels[pendingIndex].clientEndpoint
          == cmd->apsFrame->destinationEndpoint)
      && (tunnels[pendingIndex].serverEndpoint
          == cmd->apsFrame->sourceEndpoint)) {
    // If the server accepted the request and created the tunnel, we need to
    // save the assigned tunnel id for future communication.  However, we do
    // not inform the application of the real tunnel id and instead give them
    // an index into our table.  This makes it possible for the application to
    // uniquely identify a tunnel even if multiple servers give us duplicate
    // tunnel ids.
    uint8_t tunnelIndex;
    if (cmd_data.tunnelStatus == SL_ZIGBEE_ZCL_TUNNELING_TUNNEL_STATUS_SUCCESS) {
      tunnels[pendingIndex].tunnelId = cmd_data.tunnelId;
      tunnelIndex = pendingIndex;
    } else {
      tunnels[pendingIndex].serverEndpoint = UNUSED_ENDPOINT_ID;
      tunnelIndex = SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX;
    }

    // Reschedule the tick to clean up any unused entries that may have been
    // closed while we were waiting for this request to finish.
    sl_zigbee_zcl_schedule_client_tick(cmd->apsFrame->destinationEndpoint,
                                       ZCL_TUNNELING_CLUSTER_ID,
                                       MILLISECOND_TICKS_PER_SECOND);

    pendingIndex = SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX;
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    sl_zigbee_af_tunneling_client_tunnel_opened_cb(tunnelIndex,
                                                   (sl_zigbee_af_plugin_tunneling_client_status_t)cmd_data.tunnelStatus,
                                                   cmd_data.maximumIncomingTransferSize);
  }

  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_tunneling_cluster_transfer_data_server_to_client_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_tunneling_cluster_transfer_data_server_to_client_command_t cmd_data;
  sli_zigbee_af_tunneling_client_tunnel *tunnel;
  sl_zigbee_af_status_t status;
  sl_zigbee_af_tunneling_transfer_data_status_t tunnelError = SL_ZIGBEE_ZCL_TUNNELING_TRANSFER_DATA_STATUS_DATA_OVERFLOW;
  uint16_t dataLen;
  uint8_t tunnelIndex;

  if (zcl_decode_tunneling_cluster_transfer_data_server_to_client_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  dataLen = (cmd->bufLen - (cmd->payloadStartIndex + sizeof(cmd_data.tunnelId)));

  sl_zigbee_af_tunneling_cluster_print("RX: TransferData 0x%04X, [", cmd_data.tunnelId);
  sl_zigbee_af_tunneling_cluster_print_buffer(cmd_data.data, dataLen, false);
  sl_zigbee_af_tunneling_cluster_println("]");

  status = clientFindTunnel(cmd_data.tunnelId,
                            sli_zigbee_af_get_address_index(),
                            cmd->apsFrame->destinationEndpoint,
                            cmd->apsFrame->sourceEndpoint,
                            &tunnel,
                            &tunnelIndex);
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    if (dataLen <= SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_MAXIMUM_INCOMING_TRANSFER_SIZE) {
      sl_zigbee_af_tunneling_client_data_received_cb(tunnelIndex,
                                                     cmd_data.data,
                                                     dataLen);
      return status;
    }
    // else
    //   tunnelError code already set (overflow)
  } else {
    tunnelError = (status == SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED
                   ? SL_ZIGBEE_ZCL_TUNNELING_TRANSFER_DATA_STATUS_WRONG_DEVICE
                   : SL_ZIGBEE_ZCL_TUNNELING_TRANSFER_DATA_STATUS_NO_SUCH_TUNNEL);
  }

  // Error
  sl_zigbee_af_fill_command_tunneling_cluster_transfer_data_error_client_to_server(cmd_data.tunnelId,
                                                                                   tunnelError);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  sl_zigbee_af_send_response();

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_tunneling_cluster_transfer_data_error_server_to_client_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_tunneling_cluster_transfer_data_error_server_to_client_command_t cmd_data;
  sli_zigbee_af_tunneling_client_tunnel *tunnel;
  sl_zigbee_af_status_t status;
  uint8_t tunnelIndex;

  if (zcl_decode_tunneling_cluster_transfer_data_error_server_to_client_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_tunneling_cluster_println("RX: TransferDataError 0x%04X, 0x%02X",
                                         cmd_data.tunnelId,
                                         cmd_data.transferDataStatus);

  status = clientFindTunnel(cmd_data.tunnelId,
                            sli_zigbee_af_get_address_index(),
                            cmd->apsFrame->destinationEndpoint,
                            cmd->apsFrame->sourceEndpoint,
                            &tunnel,
                            &tunnelIndex);
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_tunneling_client_data_error_cb(tunnelIndex,
                                                (sl_zigbee_af_tunneling_transfer_data_status_t)cmd_data.transferDataStatus);
  }

  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_tunneling_cluster_tunnel_closure_notification_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_tunneling_cluster_tunnel_closure_notification_command_t cmd_data;
  sli_zigbee_af_tunneling_client_tunnel *tunnel;
  sl_zigbee_af_status_t status;
  uint8_t tunnelIndex;

  if (zcl_decode_tunneling_cluster_tunnel_closure_notification_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_tunneling_cluster_println("RX: TunnelClosureNotification 0x%04X",
                                         cmd_data.tunnelId);

  status = clientFindTunnel(cmd_data.tunnelId,
                            sli_zigbee_af_get_address_index(),
                            cmd->apsFrame->destinationEndpoint,
                            cmd->apsFrame->sourceEndpoint,
                            &tunnel,
                            &tunnelIndex);
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    // Mark the entry as unused and, unless we have a pending tunnel request on
    // the same endpoint, schedule a tick to clean up the address table entry.
    // The delay before cleaning up the address table is to give the stack some
    // time to continue using it for sending the response to the server.
    // However, if we have a pending request, the tick is already being used as
    // a timeout for the request and we don't want to interfere with that.
    tunnel->serverEndpoint = UNUSED_ENDPOINT_ID;
    if (pendingIndex == SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX
        || tunnels[pendingIndex].clientEndpoint != tunnel->clientEndpoint) {
      sl_zigbee_zcl_schedule_client_tick(tunnel->clientEndpoint,
                                         ZCL_TUNNELING_CLUSTER_ID,
                                         MILLISECOND_TICKS_PER_SECOND);
    }
    sl_zigbee_af_tunneling_client_tunnel_closed_cb(tunnelIndex);
  }

  return status;
}

sl_zigbee_af_status_t sl_zigbee_af_tunneling_client_transfer_data(uint8_t tunnelIndex,
                                                                  uint8_t *data,
                                                                  uint16_t dataLen)
{
  if (tunnelIndex < SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT
      && tunnels[tunnelIndex].serverEndpoint != UNUSED_ENDPOINT_ID) {
    sl_status_t status;
    sl_zigbee_af_fill_command_tunneling_cluster_transfer_data_client_to_server(tunnels[tunnelIndex].tunnelId,
                                                                               data,
                                                                               dataLen);
    sl_zigbee_af_set_command_endpoints(tunnels[tunnelIndex].clientEndpoint,
                                       tunnels[tunnelIndex].serverEndpoint);
    sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
    status = sl_zigbee_af_send_command_unicast_with_cb(SL_ZIGBEE_OUTGOING_VIA_ADDRESS_TABLE,
                                                       tunnels[tunnelIndex].addressIndex,
                                                       transferDataMessageSentCallback);
    return (status == SL_STATUS_OK
            ? SL_ZIGBEE_ZCL_STATUS_SUCCESS
            : SL_ZIGBEE_ZCL_STATUS_FAILURE);
  }
  return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
}

sl_zigbee_af_status_t sl_zigbee_af_tunneling_client_close_tunnel(uint8_t tunnelIndex)
{
  if (tunnelIndex < SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT
      && tunnels[tunnelIndex].serverEndpoint != UNUSED_ENDPOINT_ID) {
    sl_status_t status;
    sl_zigbee_af_fill_command_tunneling_cluster_close_tunnel(tunnels[tunnelIndex].tunnelId);
    sl_zigbee_af_set_command_endpoints(tunnels[tunnelIndex].clientEndpoint,
                                       tunnels[tunnelIndex].serverEndpoint);
    sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
    status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_VIA_ADDRESS_TABLE,
                                               tunnels[tunnelIndex].addressIndex);
    if (status == SL_STATUS_OK) {
      // Mark the entry as unused and, unless we have a pending tunnel request
      // on the same endpoint, schedule a tick to clean up the address table
      // entry.  The delay before cleaning up the address table is to give the
      // stack some time to continue using it for sending the response to the
      // server.  However, if we have a pending request, the tick is already
      // being used as a timeout for the request and we don't want to interfere
      // with that.
      tunnels[tunnelIndex].serverEndpoint = UNUSED_ENDPOINT_ID;
      if (pendingIndex == SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX
          || (tunnels[pendingIndex].clientEndpoint
              != tunnels[tunnelIndex].clientEndpoint)) {
        sl_zigbee_zcl_schedule_client_tick(tunnels[tunnelIndex].clientEndpoint,
                                           ZCL_TUNNELING_CLUSTER_ID,
                                           MILLISECOND_TICKS_PER_SECOND);
      }
      return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    } else {
      return SL_ZIGBEE_ZCL_STATUS_FAILURE;
    }
  }
  return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
}

void sl_zigbee_af_tunneling_client_cleanup(uint8_t tunnelIndex)
{
  if (tunnels[tunnelIndex].serverEndpoint != UNUSED_ENDPOINT_ID) {
    tunnels[tunnelIndex].serverEndpoint = UNUSED_ENDPOINT_ID;
    sl_zigbee_af_tunneling_client_tunnel_closed_cb(tunnelIndex);
  }
  if (tunnels[tunnelIndex].addressIndex != SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX) {
    sl_zigbee_af_address_table_remove_entry_by_index(tunnels[tunnelIndex].addressIndex);
    tunnels[tunnelIndex].addressIndex = SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX;
  }
}

static sl_zigbee_af_status_t clientFindTunnel(uint16_t tunnelId,
                                              uint8_t addressIndex,
                                              uint8_t clientEndpoint,
                                              uint8_t serverEndpoint,
                                              sli_zigbee_af_tunneling_client_tunnel **tunnel,
                                              uint8_t *tunnelIndex)
{
  uint8_t i;
  bool foundNonMatchingTunnel = false;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT; i++) {
    if (tunnels[i].clientEndpoint == clientEndpoint && tunnels[i].tunnelId == tunnelId) {
      if (tunnels[i].addressIndex == addressIndex
          && tunnels[i].serverEndpoint == serverEndpoint) {
        *tunnel = &tunnels[i];
        *tunnelIndex = i;
        return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
      } else if (tunnels[i].addressIndex != SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX
                 && tunnels[i].serverEndpoint != UNUSED_ENDPOINT_ID) {
        foundNonMatchingTunnel = true;
      }
    }
  }
  // In order to pass the SE test if only a nonmatching tunnel is found
  // we must return error of wrong device. EMAPPFWKV2-1300
  if (foundNonMatchingTunnel) {
    return SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
  }
  return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
}

void sli_zigbee_af_tunneling_client_cli_print(void)
{
  uint8_t i;
  uint8_t count = 0;
  sl_zigbee_af_tunneling_cluster_println("");
  sl_zigbee_af_tunneling_cluster_println("#   server              cep  sep  tid    pid  mfg");
  sl_zigbee_af_tunneling_cluster_flush();
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT; i++) {
    sl_zigbee_af_tunneling_cluster_print("%02X: ", i);
    if (tunnels[i].serverEndpoint != UNUSED_ENDPOINT_ID) {
      sl_802154_long_addr_t eui64 = { 0 };
      sl_status_t validEntry;

      validEntry = sl_zigbee_af_address_table_lookup_by_index(tunnels[i].addressIndex,
                                                              eui64);

      if (validEntry == SL_STATUS_INVALID_STATE) {
        continue;
      }
      count++;
      sl_zigbee_af_tunneling_cluster_debug_exec(sl_zigbee_af_print_big_endian_eui64(eui64));
      sl_zigbee_af_tunneling_cluster_print(" 0x%02X 0x%02X 0x%04X",
                                           tunnels[i].clientEndpoint,
                                           tunnels[i].serverEndpoint,
                                           tunnels[i].tunnelId);
      sl_zigbee_af_tunneling_cluster_flush();
      sl_zigbee_af_tunneling_cluster_print(" 0x%02X 0x%04X",
                                           tunnels[i].protocolId,
                                           tunnels[i].manufacturerCode);
      sl_zigbee_af_tunneling_cluster_flush();
    }
    sl_zigbee_af_tunneling_cluster_println("");
  }
  sl_zigbee_af_tunneling_cluster_println("%d of %d tunnels in use.",
                                         count,
                                         SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT);
}

uint32_t sl_zigbee_af_tunneling_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                             sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_REQUEST_TUNNEL_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_tunneling_cluster_request_tunnel_response_cb(cmd);
        break;
      }
      case ZCL_TRANSFER_DATA_SERVER_TO_CLIENT_COMMAND_ID:
      {
        status = sl_zigbee_af_tunneling_cluster_transfer_data_server_to_client_cb(cmd);
        break;
      }
      case ZCL_TRANSFER_DATA_ERROR_SERVER_TO_CLIENT_COMMAND_ID:
      {
        status = sl_zigbee_af_tunneling_cluster_transfer_data_error_server_to_client_cb(cmd);
        break;
      }
      case ZCL_TUNNEL_CLOSURE_NOTIFICATION_COMMAND_ID:
      {
        status = sl_zigbee_af_tunneling_cluster_tunnel_closure_notification_cb(cmd);
        break;
      }
    }
  }

  return status;
}
