/***************************************************************************//**
 * @file
 * @brief Routines for the Tunneling Server plugin.
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
#include "tunneling-server.h"
#include "app/framework/plugin/address-table/address-table.h"

#include "zap-cluster-command-parser.h"
#include "tunneling-server-config.h"
#if (SL_ZIGBEE_AF_PLUGIN_TUNNELING_SERVER_TEST_PROTOCOL_SUPPORT == 1)
#define TEST_PROTOCOL_SUPPORT
#endif
#if (SL_ZIGBEE_AF_PLUGIN_TUNNELING_SERVER_CLOSURE_NOTIFICATION_SUPPORT == 1)
#define CLOSURE_NOTIFICATION_SUPPORT
#endif

#define UNUSED_ENDPOINT_ID 0xFF

// If addressIndex is SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX and clientEndpoint is
// UNUSED_ENDPOINT_ID, then the entry is unused and available for use by a new
// tunnel.  Occasionally, clientEndpoint will be UNUSED_ENDPOINT_ID, but
// addressIndex will contain a valid index.  This happens after a tunnel is
// removed but before the address table entry has been cleaned up.  There is a
// delay between closure and cleanup to allow the stack to continue using the
// address table entry to send messages to the client.
typedef struct {
/* addressIndex refers to the table entries of both Address Table plugin/stack Addr Table */
  uint8_t   addressIndex;
  uint8_t   clientEndpoint;
  uint8_t   serverEndpoint;
  uint8_t   protocolId;
  uint16_t  manufacturerCode;
  bool flowControlSupport;
  uint32_t  lastActive;
} sli_zigbee_af_tunneling_server_tunnel;

// this tells you both if the test protocol IS SUPPORTED and if
// the current protocol requested IS the test protocol
#ifdef TEST_PROTOCOL_SUPPORT
  #define sli_zigbee_af_is_test_protocol(protocolId, manufacturerCode) \
  ((protocolId) == SL_ZIGBEE_ZCL_TUNNELING_PROTOCOL_ID_TEST            \
   && (manufacturerCode) == ZCL_TUNNELING_CLUSTER_UNUSED_MANUFACTURER_CODE)
  #define sli_zigbee_af_tunnel_is_test_protocol(tunnel) \
  (sli_zigbee_af_is_test_protocol((tunnel)->protocolId, (tunnel)->manufacturerCode))
#else
  #define sli_zigbee_af_is_test_protocol(protocolId, manufacturerCode) (false)
  #define sli_zigbee_af_tunnel_is_test_protocol(tunnel) (false)
#endif

// global for keeping track of test-harness behavior "busy status"
static bool sl_zigbee_af_tunneling_server_busy_status = false;

static sli_zigbee_af_tunneling_server_tunnel tunnels[SL_ZIGBEE_AF_PLUGIN_TUNNELING_SERVER_TUNNEL_LIMIT];

static sl_zigbee_af_status_t serverFindTunnel(uint16_t tunnelIndex,
                                              uint8_t addressIndex,
                                              uint8_t clientEndpoint,
                                              uint8_t serverEndpoint,
                                              sli_zigbee_af_tunneling_server_tunnel **tunnel);
static void closeInactiveTunnels(uint8_t endpoint);

void sl_zigbee_af_tunneling_cluster_server_init_cb(uint8_t endpoint)
{
  sl_zigbee_af_status_t status;
  uint16_t closeTunnelTimeout = SL_ZIGBEE_AF_PLUGIN_TUNNELING_SERVER_CLOSE_TUNNEL_TIMEOUT;
  uint8_t i;

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_TUNNELING_SERVER_TUNNEL_LIMIT; i++) {
    tunnels[i].addressIndex = SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX;
    tunnels[i].clientEndpoint = UNUSED_ENDPOINT_ID;
  }

  status = sl_zigbee_af_write_server_attribute(endpoint,
                                               ZCL_TUNNELING_CLUSTER_ID,
                                               ZCL_CLOSE_TUNNEL_TIMEOUT_ATTRIBUTE_ID,
                                               (uint8_t *)&closeTunnelTimeout,
                                               ZCL_INT16U_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_tunneling_cluster_println("ERR: writing close tunnel timeout 0x%02X",
                                           status);
  }
}

void sl_zigbee_af_tunneling_cluster_server_tick_cb(uint8_t endpoint)
{
  closeInactiveTunnels(endpoint);
}

void sl_zigbee_af_tunneling_server_stack_status_cb(sl_status_t status)
{
  uint8_t i;

  if (status == SL_STATUS_NETWORK_DOWN
      && !sl_zigbee_is_performing_rejoin()) {
    sl_zigbee_af_simple_metering_cluster_println("Re-initializing tunnels due to stack down.");
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_TUNNELING_SERVER_TUNNEL_LIMIT; i++) {
      sl_zigbee_af_tunneling_server_cleanup(i);
    }
  }
}

void sl_zigbee_af_tunneling_cluster_server_attribute_changed_cb(uint8_t endpoint,
                                                                sl_zigbee_af_attribute_id_t attributeId)
{
  if (attributeId == ZCL_CLOSE_TUNNEL_TIMEOUT_ATTRIBUTE_ID) {
    closeInactiveTunnels(endpoint);
  }
}

//-----------------------
// ZCL commands callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_tunneling_cluster_request_tunnel_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_tunneling_cluster_request_tunnel_command_t cmd_data;
  uint16_t tunnelIndex = ZCL_TUNNELING_CLUSTER_INVALID_TUNNEL_ID;
  sl_zigbee_af_tunneling_tunnel_status_t status = SL_ZIGBEE_ZCL_TUNNELING_TUNNEL_STATUS_NO_MORE_TUNNEL_IDS;

  if (zcl_decode_tunneling_cluster_request_tunnel_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_tunneling_cluster_println("RX: RequestTunnel 0x%02X, 0x%04X, 0x%02X, 0x%04X",
                                         cmd_data.protocolId,
                                         cmd_data.manufacturerCode,
                                         cmd_data.flowControlSupport,
                                         cmd_data.maximumIncomingTransferSize);

  if (!sli_zigbee_af_is_test_protocol(cmd_data.protocolId, cmd_data.manufacturerCode)
      && !sl_zigbee_af_tunneling_server_is_protocol_supported_cb(cmd_data.protocolId,
                                                                 cmd_data.manufacturerCode)) {
    status = SL_ZIGBEE_ZCL_TUNNELING_TUNNEL_STATUS_PROTOCOL_NOT_SUPPORTED;
  } else if (cmd_data.flowControlSupport) {
    // TODO: Implement support for flow control.
    status = SL_ZIGBEE_ZCL_TUNNELING_TUNNEL_STATUS_FLOW_CONTROL_NOT_SUPPORTED;
  } else if (sl_zigbee_af_tunneling_server_busy_status) {
    status = SL_ZIGBEE_ZCL_TUNNELING_TUNNEL_STATUS_BUSY;
  } else {
    uint8_t i;
    bool tunnelExists = false;
    // ------------------------------------------------------------------------------------------
    // EMZIGBEE-1395, (old EMAPPFWKV2-1350) :
    // GBCS v2.0,Page 98, Section 10.2.2 Requirements for the Tunneling Cluster
    // "when receiving a RequestTunnel command from a client Device which has
    // an active TunnelID registered with the server Device, reset the
    // corresponding inactivity timer for that active TunnelID and send a
    // RequestTunnelResponse containing that activeTunnelID."
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_TUNNELING_SERVER_TUNNEL_LIMIT; i++) {
      if (tunnels[i].addressIndex == sli_zigbee_af_get_address_index()
          && tunnels[i].clientEndpoint == cmd->apsFrame->sourceEndpoint
          && tunnels[i].serverEndpoint == cmd->apsFrame->destinationEndpoint
          && tunnels[i].protocolId == cmd_data.protocolId
          && tunnels[i].manufacturerCode == cmd_data.manufacturerCode) {
        // Update Tunnel Index for response
        tunnelIndex = i;
        // If we are here the tunnel is exsisiting already - hence update the
        // tunnel param as per request.
        tunnels[i].flowControlSupport = cmd_data.flowControlSupport;
        // Reset the timer so that it ticks from now.
        tunnels[i].lastActive = sl_zigbee_af_get_current_time();
        status = SL_ZIGBEE_ZCL_TUNNELING_TUNNEL_STATUS_SUCCESS;
        // This will reschedule the tick that will timeout tunnels.
        closeInactiveTunnels(cmd->apsFrame->destinationEndpoint);
        tunnelExists = true;
        break;
      }
    }
    // ------------------------------------------------------------------------------------------
    // The tunnel already exists - so skip following loop of creating one
    if (!tunnelExists) {
      for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_TUNNELING_SERVER_TUNNEL_LIMIT; i++) {
        if (tunnels[i].addressIndex == SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX
            && tunnels[i].clientEndpoint == UNUSED_ENDPOINT_ID) {
          sl_802154_long_addr_t eui64;
          sl_802154_short_addr_t client = cmd->source;
          status = SL_ZIGBEE_ZCL_TUNNELING_TUNNEL_STATUS_BUSY;
          if (sl_zigbee_lookup_eui64_by_node_id(client, eui64) == SL_STATUS_OK) {
            tunnels[i].addressIndex = sl_zigbee_af_address_table_add_entry(eui64);
            if (tunnels[i].addressIndex != SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX) {
              tunnelIndex = i;
              tunnels[i].clientEndpoint = cmd->apsFrame->sourceEndpoint;
              tunnels[i].serverEndpoint = cmd->apsFrame->destinationEndpoint;
              tunnels[i].protocolId = cmd_data.protocolId;
              tunnels[i].manufacturerCode = cmd_data.manufacturerCode;
              tunnels[i].flowControlSupport = cmd_data.flowControlSupport;
              tunnels[i].lastActive = sl_zigbee_af_get_current_time();
              status = SL_ZIGBEE_ZCL_TUNNELING_TUNNEL_STATUS_SUCCESS;
              // This will reschedule the tick that will timeout tunnels.
              closeInactiveTunnels(cmd->apsFrame->destinationEndpoint);
            } else {
              sl_zigbee_af_tunneling_cluster_println("ERR: Could not create address"
                                                     " table entry for node 0x%04X",
                                                     client);
            }
          } else {
            sl_zigbee_af_tunneling_cluster_println("ERR: EUI64 for node 0x%04X"
                                                   " is unknown",
                                                   client);
          }
          break;
        }
      }
    }
    if (status == SL_ZIGBEE_ZCL_TUNNELING_TUNNEL_STATUS_SUCCESS) {
      sl_zigbee_af_tunneling_server_tunnel_opened_cb(tunnelIndex,
                                                     cmd_data.protocolId,
                                                     cmd_data.manufacturerCode,
                                                     cmd_data.flowControlSupport,
                                                     cmd_data.maximumIncomingTransferSize);
    }
  }
  sl_zigbee_af_fill_command_tunneling_cluster_request_tunnel_response(tunnelIndex,
                                                                      status,
                                                                      SL_ZIGBEE_AF_PLUGIN_TUNNELING_SERVER_MAXIMUM_INCOMING_TRANSFER_SIZE);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  sl_zigbee_af_send_response();
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_tunneling_cluster_close_tunnel_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_tunneling_cluster_close_tunnel_command_t cmd_data;
  sli_zigbee_af_tunneling_server_tunnel *tunnel;
  sl_zigbee_af_status_t status;

  if (zcl_decode_tunneling_cluster_close_tunnel_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_tunneling_cluster_println("RX: CloseTunnel 0x%04X", cmd_data.tunnelId);

  status = serverFindTunnel(cmd_data.tunnelId,
                            sli_zigbee_af_get_address_index(),
                            cmd->apsFrame->sourceEndpoint,
                            cmd->apsFrame->destinationEndpoint,
                            &tunnel);
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    // Mark the entry as unused and schedule a tick to clean up the address
    // table entry.  The delay before cleaning up the address table is to give
    // the stack some time to continue using it for sending the response to the
    // server.
    tunnel->clientEndpoint = UNUSED_ENDPOINT_ID;
    sl_zigbee_zcl_schedule_server_tick(cmd->apsFrame->destinationEndpoint,
                                       ZCL_TUNNELING_CLUSTER_ID,
                                       MILLISECOND_TICKS_PER_SECOND);
    sl_zigbee_af_tunneling_server_tunnel_closed_cb(cmd_data.tunnelId,
                                                   CLOSE_INITIATED_BY_CLIENT);
  }

  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_tunneling_cluster_transfer_data_client_to_server_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_tunneling_cluster_transfer_data_client_to_server_command_t cmd_data;
  sli_zigbee_af_tunneling_server_tunnel *tunnel;
  sl_zigbee_af_status_t status;

  if (zcl_decode_tunneling_cluster_transfer_data_client_to_server_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint16_t dataLen = (cmd->bufLen
                      - (cmd->payloadStartIndex
                         + sizeof(cmd_data.tunnelId)));
  sl_zigbee_af_tunneling_transfer_data_status_t tunnelError = SL_ZIGBEE_ZCL_TUNNELING_TRANSFER_DATA_STATUS_DATA_OVERFLOW;

  sl_zigbee_af_tunneling_cluster_print("RX: TransferData 0x%04X, [", cmd_data.tunnelId);
  sl_zigbee_af_tunneling_cluster_print_buffer(cmd_data.data, dataLen, false);
  sl_zigbee_af_tunneling_cluster_println("]");

  status = serverFindTunnel(cmd_data.tunnelId,
                            sli_zigbee_af_get_address_index(),
                            cmd->apsFrame->sourceEndpoint,
                            cmd->apsFrame->destinationEndpoint,
                            &tunnel);
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    if (dataLen <= SL_ZIGBEE_AF_PLUGIN_TUNNELING_SERVER_MAXIMUM_INCOMING_TRANSFER_SIZE) {
      tunnel->lastActive = sl_zigbee_af_get_current_time();

      // If this is the test protocol (and the option for test protocol support
      // is enabled), just turn the data around without notifying the
      // application.  Otherwise, everything goes to the application via a
      // callback.
      if (sli_zigbee_af_tunnel_is_test_protocol(tunnel)) {
        sl_zigbee_af_tunneling_server_transfer_data(cmd_data.tunnelId, cmd_data.data, dataLen);
      } else {
        sl_zigbee_af_tunneling_server_data_received_cb(cmd_data.tunnelId, cmd_data.data, dataLen);
      }
      return status;
    }
    // else
    //  tunnelError code already set (overflow)
  } else {
    tunnelError = (status == SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED
                   ? SL_ZIGBEE_ZCL_TUNNELING_TRANSFER_DATA_STATUS_WRONG_DEVICE
                   : SL_ZIGBEE_ZCL_TUNNELING_TRANSFER_DATA_STATUS_NO_SUCH_TUNNEL);
  }

  // Error
  sl_zigbee_af_fill_command_tunneling_cluster_transfer_data_error_server_to_client(cmd_data.tunnelId,
                                                                                   tunnelError);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  sl_zigbee_af_send_response();
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_tunneling_cluster_transfer_data_error_client_to_server_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_tunneling_cluster_transfer_data_error_client_to_server_command_t cmd_data;
  sli_zigbee_af_tunneling_server_tunnel *tunnel;
  sl_zigbee_af_status_t status;

  if (zcl_decode_tunneling_cluster_transfer_data_error_client_to_server_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_tunneling_cluster_println("RX: TransferDataError 0x%04X, 0x%02X",
                                         cmd_data.tunnelId,
                                         cmd_data.transferDataStatus);

  status = serverFindTunnel(cmd_data.tunnelId,
                            sli_zigbee_af_get_address_index(),
                            cmd->apsFrame->sourceEndpoint,
                            cmd->apsFrame->destinationEndpoint,
                            &tunnel);
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_tunneling_server_data_error_cb(cmd_data.tunnelId,
                                                (sl_zigbee_af_tunneling_transfer_data_status_t)cmd_data.transferDataStatus);
  }

  return status;
}

sl_zigbee_af_status_t sl_zigbee_af_tunneling_server_transfer_data(uint16_t tunnelIndex,
                                                                  uint8_t *data,
                                                                  uint16_t dataLen)
{
  if (tunnelIndex < SL_ZIGBEE_AF_PLUGIN_TUNNELING_SERVER_TUNNEL_LIMIT
      && tunnels[tunnelIndex].clientEndpoint != UNUSED_ENDPOINT_ID) {
    sl_status_t status;
    sl_zigbee_af_fill_command_tunneling_cluster_transfer_data_server_to_client(tunnelIndex,
                                                                               data,
                                                                               dataLen);
    sl_zigbee_af_set_command_endpoints(tunnels[tunnelIndex].serverEndpoint,
                                       tunnels[tunnelIndex].clientEndpoint);
    sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
    status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_VIA_ADDRESS_TABLE,
                                               tunnels[tunnelIndex].addressIndex);

    tunnels[tunnelIndex].lastActive = sl_zigbee_af_get_current_time();
    return (status == SL_STATUS_OK
            ? SL_ZIGBEE_ZCL_STATUS_SUCCESS
            : SL_ZIGBEE_ZCL_STATUS_FAILURE);
  }
  return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
}

void sl_zigbee_af_tunneling_server_cleanup(uint8_t tunnelId)
{
  if (tunnels[tunnelId].clientEndpoint != UNUSED_ENDPOINT_ID) {
    tunnels[tunnelId].clientEndpoint = UNUSED_ENDPOINT_ID;
    sl_zigbee_af_tunneling_server_tunnel_closed_cb(tunnelId, CLOSE_INITIATED_BY_SERVER);
  }
  if (tunnels[tunnelId].addressIndex != SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX) {
    sl_zigbee_af_address_table_remove_entry_by_index(tunnels[tunnelId].addressIndex);
    tunnels[tunnelId].addressIndex = SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX;
  }
}

static sl_zigbee_af_status_t serverFindTunnel(uint16_t tunnelIndex,
                                              uint8_t addressIndex,
                                              uint8_t clientEndpoint,
                                              uint8_t serverEndpoint,
                                              sli_zigbee_af_tunneling_server_tunnel **tunnel)
{
  if (tunnelIndex < SL_ZIGBEE_AF_PLUGIN_TUNNELING_SERVER_TUNNEL_LIMIT
      && tunnels[tunnelIndex].clientEndpoint != UNUSED_ENDPOINT_ID) {
    if (tunnels[tunnelIndex].addressIndex == addressIndex
        && tunnels[tunnelIndex].clientEndpoint == clientEndpoint
        && tunnels[tunnelIndex].serverEndpoint == serverEndpoint) {
      *tunnel = &tunnels[tunnelIndex];
      return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    } else {
      return SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
    }
  }
  return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
}

static void closeInactiveTunnels(uint8_t endpoint)
{
  sl_zigbee_af_status_t status;
  uint32_t currentTime = sl_zigbee_af_get_current_time();
  uint32_t delay = MAX_INT32U_VALUE;
  uint16_t closeTunnelTimeout;
  uint8_t i;

  status = sl_zigbee_af_read_server_attribute(endpoint,
                                              ZCL_TUNNELING_CLUSTER_ID,
                                              ZCL_CLOSE_TUNNEL_TIMEOUT_ATTRIBUTE_ID,
                                              (uint8_t *)&closeTunnelTimeout,
                                              sizeof(closeTunnelTimeout));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_tunneling_cluster_println("ERR: reading close tunnel timeout 0x%02X",
                                           status);
    return;
  }

  // Every time the tick fires, we search the table for inactive tunnels that
  // should be closed and unused entries that still have an address table
  // index.  The unused tunnels have been closed, but the address table entry
  // was not immediately removed so the stack could continue using it.  By this
  // point, we've given the stack a fair shot to use it, so now remove the
  // address table entry.  While looking through the tunnels, the time to next
  // tick is calculated based on how recently the tunnels were used or by the
  // need to clean up newly unused tunnels.
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_TUNNELING_SERVER_TUNNEL_LIMIT; i++) {
    if (tunnels[i].serverEndpoint == endpoint) {
      if (tunnels[i].clientEndpoint != UNUSED_ENDPOINT_ID) {
        uint32_t elapsed = currentTime - tunnels[i].lastActive;
        if (closeTunnelTimeout <= elapsed) {
          // If we are closing an inactive tunnel and will not send a closure
          // notification, we can immediately remove the address table entry
          // for the client because it will no longer be used.  Otherwise, we
          // need to schedule a tick to clean up the address table entry so we
          // give the stack a chance to continue using it for sending the
          // notification.
#ifdef CLOSURE_NOTIFICATION_SUPPORT
          sl_zigbee_af_fill_command_tunneling_cluster_tunnel_closure_notification(i);
          sl_zigbee_af_set_command_endpoints(tunnels[i].serverEndpoint,
                                             tunnels[i].clientEndpoint);
          sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
          sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_VIA_ADDRESS_TABLE,
                                            tunnels[i].addressIndex);

          delay = 1;
#else
          sl_zigbee_af_address_table_remove_entry_by_index(tunnels[i].addressIndex);
          tunnels[i].addressIndex = SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX;
#endif
          tunnels[i].clientEndpoint = UNUSED_ENDPOINT_ID;
          sl_zigbee_af_tunneling_server_tunnel_closed_cb(i,
                                                         CLOSE_INITIATED_BY_SERVER);
        } else {
          uint32_t remaining = closeTunnelTimeout - elapsed;
          if (remaining < delay) {
            delay = remaining;
          }
        }
      } else if (tunnels[i].addressIndex != SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX) {
        sl_zigbee_af_address_table_remove_entry_by_index(tunnels[i].addressIndex);
        tunnels[i].addressIndex = SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX;
      }
    }
  }

  if (delay != MAX_INT32U_VALUE) {
    sl_zigbee_zcl_schedule_server_tick(endpoint,
                                       ZCL_TUNNELING_CLUSTER_ID,
                                       delay * MILLISECOND_TICKS_PER_SECOND);
  }
}

void sl_zigbee_af_tunneling_server_toggle_busy_command(void)
{
  sl_zigbee_af_tunneling_cluster_println("");
  if (sl_zigbee_af_tunneling_server_busy_status) {
    sl_zigbee_af_tunneling_server_busy_status = false;
    sl_zigbee_af_tunneling_cluster_println("  NOTE: current status is NOT BUSY (tunneling works)");
  } else {
    sl_zigbee_af_tunneling_server_busy_status = true;
    sl_zigbee_af_tunneling_cluster_println("  NOTE: current status is BUSY (tunneling won't work)");
  }
  sl_zigbee_af_tunneling_cluster_println("");
  sl_zigbee_af_tunneling_cluster_flush();
}

void sli_zigbee_af_tunneling_server_cli_print(void)
{
  uint32_t currentTime = sl_zigbee_af_get_current_time();
  uint8_t i;
  sl_zigbee_af_tunneling_cluster_println("");
  sl_zigbee_af_tunneling_cluster_println("#   client              cep  sep  tid    pid  mfg    age");
  sl_zigbee_af_tunneling_cluster_flush();
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_TUNNELING_SERVER_TUNNEL_LIMIT; i++) {
    sl_zigbee_af_tunneling_cluster_print("%02X: ", i);
    if (tunnels[i].clientEndpoint != UNUSED_ENDPOINT_ID) {
      sl_802154_long_addr_t eui64;
      sl_zigbee_af_address_table_lookup_by_index(tunnels[i].addressIndex, eui64);
      sl_zigbee_af_tunneling_cluster_debug_exec(sl_zigbee_af_print_big_endian_eui64(eui64));
      sl_zigbee_af_tunneling_cluster_print(" 0x%02X 0x%02X 0x%04X",
                                           tunnels[i].clientEndpoint,
                                           tunnels[i].serverEndpoint,
                                           i);
      sl_zigbee_af_tunneling_cluster_flush();
      sl_zigbee_af_tunneling_cluster_print(" 0x%02X 0x%04X 0x%08X",
                                           tunnels[i].protocolId,
                                           tunnels[i].manufacturerCode,
                                           currentTime - tunnels[i].lastActive);
      sl_zigbee_af_tunneling_cluster_flush();
    }
    sl_zigbee_af_tunneling_cluster_println("");
  }
}

uint32_t sl_zigbee_af_tunneling_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                             sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_REQUEST_TUNNEL_COMMAND_ID:
      {
        status = sl_zigbee_af_tunneling_cluster_request_tunnel_cb(cmd);
        break;
      }
      case ZCL_CLOSE_TUNNEL_COMMAND_ID:
      {
        status = sl_zigbee_af_tunneling_cluster_close_tunnel_cb(cmd);
        break;
      }
      case ZCL_TRANSFER_DATA_CLIENT_TO_SERVER_COMMAND_ID:
      {
        status = sl_zigbee_af_tunneling_cluster_transfer_data_client_to_server_cb(cmd);
        break;
      }
      case ZCL_TRANSFER_DATA_ERROR_CLIENT_TO_SERVER_COMMAND_ID:
      {
        status = sl_zigbee_af_tunneling_cluster_transfer_data_error_client_to_server_cb(cmd);
        break;
      }
    }
  }

  return status;
}
