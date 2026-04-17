/***************************************************************************//**
 * @file
 * @brief  The Tunnel Manager is responsible for establishing and maintaining tunnels
 * to all devices.  There are four APIs exposed by the tunnel manager. The
 * init function is called at startup and initializes internal data structures.
 * The create function is called after the CBKE with the device, the sendData
 * function is called whenever data is to be sent to the device, and the
 * destroy function is called whenever the tunnel to the device should
 * be torn down. There are also 1 callback that the tunnel manager will call.
 * It is sli_zigbee_af_comms_hub_function_tunnel_data_received_callback which is
 * called when data is received from a tunnel.
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
#include "app/framework/plugin/tunneling-client/tunneling-client.h"
#include "app/framework/plugin/tunneling-server/tunneling-server.h"
#include "comms-hub-function.h"
#include "tunnel-manager.h"

typedef enum {
  CLIENT_TUNNEL,
  SERVER_TUNNEL,
} sli_zigbee_af_comms_hub_function_tunnel_type;

typedef enum {
  UNUSED_TUNNEL,
  REQUEST_PENDING_TUNNEL,
  RESPONSE_PENDING_TUNNEL,
  ACTIVE_TUNNEL,
  CLOSED_TUNNEL
} sli_zigbee_af_comms_hub_function_tunnel_state;

typedef struct {
  sl_802154_long_addr_t remoteDeviceId;
  uint8_t remoteEndpoint;
  sl_802154_short_addr_t remoteNodeId;
  sli_zigbee_af_comms_hub_function_tunnel_type type;
  sli_zigbee_af_comms_hub_function_tunnel_state state;
  uint8_t tunnelId;
  uint32_t timeoutMSec;
} sli_zigbee_af_comms_hub_function_tunnel;

static sli_zigbee_af_comms_hub_function_tunnel tunnels[SL_ZIGBEE_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT];

/*
 * The Tunneling-client plugin only allows one tunnel to be created at a time.
 * If you attempt to create a second tunnel before the first finishes it returns
 * SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_BUSY so we only need to keep track of one
 * pending tunnel. Any other tunnel creation attempts which get the BUSY error
 * and will be attempted again when the emberAfPluginCommsHubFunctionTunnelEventControl
 * event fires (see tunnelEventHandler).
 */
static uint8_t responsePendingIndex = EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;

/*
 * Temporary storage for messages to sleepy devices where the data includes a
 * header
 */
static uint8_t message[1500];

/*
 * Per section 10.2.2 of the GBCS version 0.8
 *
 * "Devices shall set the value of the ManufacturerCode field in any
 * RequestTunnel command to 0xFFFF ('not used').
 *
 * The ProtocolID of all Remote Party Messages shall be 6 ('GB-HGRP'). Devices
 * shall set the value of the ProtocolID field in any RequestTunnel command to 6.
 *
 * Devices shall set the value of the FlowControlSupport field in any
 * RequestTunnel command to 'False'.
 */
#define GBCS_TUNNELING_MANUFACTURER_CODE      0xFFFF
#define GBCS_TUNNELING_PROTOCOL_ID            0x06
#define GBCS_TUNNELING_FLOW_CONTROL_SUPPORT   false

sl_zigbee_af_event_t sl_zigbee_af_comms_hub_function_tunnel_event;
#define tunnelEventControl (&sl_zigbee_af_comms_hub_function_tunnel_event)
static void tunnelEventHandler(sl_zigbee_af_event_t * event);

//------------------------------------------------------------------------------
// Forward Declarations
static uint8_t findUnusedTunnel(void);
static bool requestTunnel(uint8_t tunnelIndex);
static bool handleRequestTunnelFailure(uint8_t tunnelIndex,
                                       sl_zigbee_af_plugin_tunneling_client_status_t status);
static uint8_t findTunnelByDeviceId(sl_802154_long_addr_t remoteDeviceId);
static uint8_t findTunnelByTunnelId(uint8_t tunnelId, sli_zigbee_af_comms_hub_function_tunnel_type type);

//------------------------------------------------------------------------------
// API Functions

void sli_zigbee_af_comms_hub_function_tunnel_init(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
      sl_zigbee_af_event_init(&sl_zigbee_af_comms_hub_function_tunnel_event,
                              tunnelEventHandler);
      sl_zigbee_af_debug_println("CHF: TunnelInit");
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA:
    {
      uint8_t i;
      for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT; i++) {
        tunnels[i].state = UNUSED_TUNNEL;
      }

      // Per GBCS v0.8 section 10.2.2, Devices supporting the Tunneling Cluster
      // as a Server shall have a MaximumIncomingTransferSize set to 1500 octets,
      // in line with the ZSE default.  All Devices supporting the Tunneling
      // Cluster shall use this value in any RequestTunnelResponse command and
      // any RequestTunnel command.
      //
      // If the tunneling client's configured maximumIncomingTransferSize
      // is less than 1500 we'll log a warning.
      //
      // See equivalent check in sl_zigbee_af_tunneling_client_tunnel_opened_cb()
      if (SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_MAXIMUM_INCOMING_TRANSFER_SIZE < 1500) {
        sl_zigbee_af_comms_hub_function_println("WARN: Tunneling Client MaximumIncomingTransferSize is %d but should be 1500",
                                                SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_MAXIMUM_INCOMING_TRANSFER_SIZE);
      }
      break;
    }
  }
}

bool sli_zigbee_af_comms_hub_function_tunnel_exists(sl_802154_long_addr_t deviceEui64)
{
  return (EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX != findTunnelByDeviceId(deviceEui64));
}

// This should be called after CBKE.
bool sli_zigbee_af_comms_hub_function_tunnel_create(sl_802154_long_addr_t remoteDeviceId,
                                                    uint8_t remoteEndpoint)
{
  uint8_t tunnelIndex;
  sl_zigbee_af_debug_print("CHF: TunnelCreate ");
  sl_zigbee_af_debug_debug_exec(sl_zigbee_af_print_big_endian_eui64(remoteDeviceId));
  sl_zigbee_af_debug_println(" 0x%02X", remoteEndpoint);

  // We only support one tunnel to a given remote device/endpoint so if we
  // already have a tunnel lets work with it.
  tunnelIndex = findTunnelByDeviceId(remoteDeviceId);
  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    if (tunnels[tunnelIndex].state == CLOSED_TUNNEL) {
      return requestTunnel(tunnelIndex);
    }
    return true;
  }

  // Find a slot in the tunnels table for the new tunnel
  tunnelIndex = findUnusedTunnel();
  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    memcpy(tunnels[tunnelIndex].remoteDeviceId, remoteDeviceId, EUI64_SIZE);
    (void) sl_zigbee_lookup_node_id_by_eui64(remoteDeviceId, &tunnels[tunnelIndex].remoteNodeId);
    tunnels[tunnelIndex].remoteEndpoint = remoteEndpoint;
    tunnels[tunnelIndex].type = CLIENT_TUNNEL;
    tunnels[tunnelIndex].state = CLOSED_TUNNEL;
    tunnels[tunnelIndex].tunnelId = 0xFF;
    tunnels[tunnelIndex].timeoutMSec = 0;
    return requestTunnel(tunnelIndex);
  }

  // This is a misconfiguration or a bug in the code calling this API. Either
  // the tunnel client plugin limit is set too low for the number of tunnels
  // required or the code that is calling this function is in error.  Either way,
  // we'll print the error and return false indicating that the tunnel was
  // not created.
  sl_zigbee_af_comms_hub_function_println("%s%s%s",
                                          "Error: ",
                                          "Tunnel Create failed: ",
                                          "Too many tunnels");
  return false;
}

bool sli_zigbee_af_comms_hub_function_tunnel_send_data(sl_802154_long_addr_t remoteDeviceId,
                                                       uint16_t headerLen,
                                                       uint8_t *header,
                                                       uint16_t dataLen,
                                                       uint8_t *data)
{
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
  bool success;
  uint8_t tunnelIndex;

  sl_zigbee_af_debug_print("CHF: TunnelSendData ");
  sl_zigbee_af_debug_debug_exec(sl_zigbee_af_print_big_endian_eui64(remoteDeviceId));
  sl_zigbee_af_debug_print(" [");
  sl_zigbee_af_debug_print_buffer(header, headerLen, false);
  sl_zigbee_af_debug_print_buffer(data, dataLen, false);
  sl_zigbee_af_debug_println("]");

  tunnelIndex = findTunnelByDeviceId(remoteDeviceId);
  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    if (tunnels[tunnelIndex].state == ACTIVE_TUNNEL) {
      // Add the header to the output buffers to that we don't copy data twice
      if (headerLen > 0) {
        memcpy(message, header, headerLen);
        memcpy(message + headerLen, data, dataLen);
        data = message;
        dataLen += headerLen;
      }
      status = (tunnels[tunnelIndex].type == CLIENT_TUNNEL)
               ? sl_zigbee_af_tunneling_client_transfer_data(tunnels[tunnelIndex].tunnelId, data, dataLen)
               : sl_zigbee_af_tunneling_server_transfer_data(tunnels[tunnelIndex].tunnelId, data, dataLen);
    } else if (tunnels[tunnelIndex].state == CLOSED_TUNNEL) {
      // we'll return failure to this message but we'll start the process
      // of bring up the tunnel so that if the message is resent the tunnel
      // should be up.
      sl_zigbee_af_comms_hub_function_println("Tunnel Closed: New tunnel is requested for the device.");
      requestTunnel(tunnelIndex);
    }
  }

  success = (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS);
  if (!success) {
    sl_zigbee_af_comms_hub_function_println("%s%s%s0x%02X",
                                            "Error: ",
                                            "Tunnel SendData failed: ",
                                            "Tunneling Status: ",
                                            status);
  }
  return success;
}

bool sli_zigbee_af_comms_hub_function_tunnel_destroy(sl_802154_long_addr_t remoteDeviceId)
{
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  uint8_t tunnelIndex;

  sl_zigbee_af_debug_print("CHF: TunnelDestroy ");
  sl_zigbee_af_debug_debug_exec(sl_zigbee_af_print_big_endian_eui64(remoteDeviceId));
  sl_zigbee_af_debug_println("");

  tunnelIndex = findTunnelByDeviceId(remoteDeviceId);
  if (tunnelIndex < SL_ZIGBEE_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT) {
    status = (tunnels[tunnelIndex].type == CLIENT_TUNNEL)
             ? sl_zigbee_af_tunneling_client_close_tunnel(tunnels[tunnelIndex].tunnelId)
             : SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      tunnels[tunnelIndex].state = UNUSED_TUNNEL;
      if (tunnelIndex == responsePendingIndex) {
        responsePendingIndex = EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;
        // let the tunnel event handler retry any pending create requests
        sl_zigbee_af_event_set_active(tunnelEventControl);
      }
    }
  }
  return (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS);
}

void sli_zigbee_af_comms_hub_function_tunnel_cleanup(sl_802154_long_addr_t remoteDeviceId)
{
  uint8_t tunnelIndex;

  tunnelIndex = findTunnelByDeviceId(remoteDeviceId);
  if (tunnelIndex < SL_ZIGBEE_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT) {
    sl_zigbee_af_debug_print("CHF: TunnelCleanup ");
    sl_zigbee_af_debug_debug_exec(sl_zigbee_af_print_big_endian_eui64(remoteDeviceId));
    sl_zigbee_af_debug_println("");

    if (tunnels[tunnelIndex].type == CLIENT_TUNNEL) {
      sl_zigbee_af_tunneling_client_cleanup(tunnels[tunnelIndex].tunnelId);
    } else {
      sl_zigbee_af_tunneling_server_cleanup(tunnels[tunnelIndex].tunnelId);
    }
    tunnels[tunnelIndex].state = UNUSED_TUNNEL;
  }
}

void sli_zigbee_af_comms_hub_function_tunnel_close(sl_802154_long_addr_t remoteDeviceId)
{
  uint8_t tunnelId;
  tunnelId = findTunnelByDeviceId(remoteDeviceId);
  sl_zigbee_af_debug_println("CHF: TunnelClosed:0x%02X", tunnelId);

  if (tunnelId != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    tunnels[tunnelId].state = CLOSED_TUNNEL;
  }
}

void sli_zigbee_af_comms_hub_function_print(void)
{
  uint8_t i;
  sl_zigbee_af_comms_hub_function_println("");
  sl_zigbee_af_comms_hub_function_println("#   remNodeId remEp  remDevId             type     tId   timeout      state");
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT; i++) {
    sl_zigbee_af_comms_hub_function_print("%02X: ", i);
    if (tunnels[i].state != UNUSED_TUNNEL) {
      sl_zigbee_af_comms_hub_function_print("0x%04X    0x%02X   ",
                                            tunnels[i].remoteNodeId,
                                            tunnels[i].remoteEndpoint);
      sl_zigbee_af_comms_hub_function_debug_exec(sl_zigbee_af_print_big_endian_eui64(tunnels[i].remoteDeviceId));
      sl_zigbee_af_comms_hub_function_print("  %s  ", (tunnels[i].type == CLIENT_TUNNEL ? "client" : "server"));
      sl_zigbee_af_comms_hub_function_print(" 0x%02X  0x%08X   ",
                                            tunnels[i].tunnelId,
                                            tunnels[i].timeoutMSec);
      switch (tunnels[i].state) {
        case REQUEST_PENDING_TUNNEL:
          sl_zigbee_af_comms_hub_function_print("request-pending");
          break;
        case RESPONSE_PENDING_TUNNEL:
          sl_zigbee_af_comms_hub_function_print("response-pending");
          break;
        case ACTIVE_TUNNEL:
          sl_zigbee_af_comms_hub_function_print("active");
          break;
        case CLOSED_TUNNEL:
          sl_zigbee_af_comms_hub_function_print("closed");
          break;
        default:
          sl_zigbee_af_comms_hub_function_print("unknown");
      }
    }
    sl_zigbee_af_comms_hub_function_println("");
  }
}

//------------------------------------------------------------------------------
// Callbacks

// Tunnel event handler used to retry previously attempted tunnel creations
static void tunnelEventHandler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);

  uint8_t tunnelIndex;
  uint32_t timeNowMs;
  uint32_t nearestEventTimeoutDelayMs = UINT32_MAX;
  uint32_t currentTunnelTimeoutMs = 0;

  sl_zigbee_af_event_set_inactive(tunnelEventControl);
  timeNowMs = halCommonGetInt32uMillisecondTick();

  sl_zigbee_af_comms_hub_function_println("CHF: tunnelEventHandler");

  // If we're no longer waiting for a tunnel to come up then find which tunnel
  // (or tunnels) are ready for another attempt at tunnel creation
  if (responsePendingIndex == EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    for (tunnelIndex = 0; tunnelIndex < SL_ZIGBEE_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT; tunnelIndex++) {
      if (tunnels[tunnelIndex].type == CLIENT_TUNNEL
          && tunnels[tunnelIndex].state == REQUEST_PENDING_TUNNEL) {
        // JIRA EMAPPFWKV2-1392: Event handler was not registered and was not working properly
        // if timeout time has passed, then request a tunnel, else retry after least time remaining
        if (timeGTorEqualInt32u(timeNowMs, tunnels[tunnelIndex].timeoutMSec)) {
          sl_zigbee_af_comms_hub_function_println("Retrying tunnel creation to node ID 0x%04X",
                                                  tunnels[tunnelIndex].remoteNodeId);
          if (requestTunnel(tunnelIndex)) {
            sl_zigbee_af_event_set_delay_ms(tunnelEventControl,
                                            MILLISECOND_TICKS_PER_SECOND);
            return;
          }
        } else {
          currentTunnelTimeoutMs = elapsedTimeInt32u(timeNowMs, tunnels[tunnelIndex].timeoutMSec);
          if (currentTunnelTimeoutMs < nearestEventTimeoutDelayMs) {
            nearestEventTimeoutDelayMs = currentTunnelTimeoutMs;
          }
        }
      }
    }
  }

  if (nearestEventTimeoutDelayMs != MAX_INT32U_VALUE) {
    sl_zigbee_af_event_set_delay_ms(tunnelEventControl, nearestEventTimeoutDelayMs);
  }
}

/** @brief Tunnel Opened
 *
 * This function is called by the Tunneling client plugin whenever a tunnel is
 * opened.
 *
 * @param tunnelId The ID of the tunnel that has been opened.  Ver.:
 * always
 * @param tunnelStatus The status of the request.  Ver.: always
 * @param maximumIncomingTransferSize The maximum incoming transfer size of
 * the server.  Ver.: always
 */
void sl_zigbee_af_tunneling_client_tunnel_opened_cb(uint8_t tunnelId,
                                                    sl_zigbee_af_plugin_tunneling_client_status_t tunnelStatus,
                                                    uint16_t maximumIncomingTransferSize)
{
  sl_zigbee_af_debug_println("CHF: ClientTunnelOpened:0x%02X,0x%02X,0x%04X", tunnelId, tunnelStatus, maximumIncomingTransferSize);

  if (responsePendingIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    uint8_t tunnelIndex = responsePendingIndex;
    responsePendingIndex = EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;
    sl_zigbee_af_event_set_active(tunnelEventControl);
    if (tunnelStatus == SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_SUCCESS) {
      tunnels[tunnelIndex].tunnelId = tunnelId;
      tunnels[tunnelIndex].state = ACTIVE_TUNNEL;

      // Per GBCS v0.8 section 10.2.2, Devices supporting the Tunneling Cluster
      // as a Server shall have a MaximumIncomingTransferSize set to 1500 octets,
      // in line with the ZSE default.  All Devices supporting the Tunneling
      // Cluster shall use this value in any RequestTunnelResponse command and
      // any RequestTunnel command.
      //
      // So rather than bring down the tunnel in the case when the maximumIncomingTransferSize
      // is less than 1500 we'll just log a warning message.
      if (maximumIncomingTransferSize < 1500) {
        sl_zigbee_af_comms_hub_function_println("Warning: tunnel opened but MaximumIncomingTransferSize of server is %d but should be 1500",
                                                maximumIncomingTransferSize);
      }

      // providing as a stub for the user to re-send data if the tunnel was re-established while sending data
      // and other cases
      sl_zigbee_af_comms_hub_function_tunnel_opened_cb(tunnels[tunnelIndex].remoteDeviceId);
      return;
    }

    // see if we can recover from the open failure
    if (handleRequestTunnelFailure(tunnelIndex, tunnelStatus)) {
      //Jira EMAPPFWKV2-1421: tunnel event handler will take care of retries and request again to setup tunnel, don't need to save
      //tunnelIndex in responsePendingIndex here, as response to tunnel request is already received.
      responsePendingIndex = EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;
      return;
    }
  } else {
    // The tunnel has opened but we no longer require it so close it
    sl_zigbee_af_tunneling_client_close_tunnel(tunnelId);
  }
}

/** @brief Data Received
 *
 * This function is called by the Tunneling client plugin whenever data is
 * received from a server through a tunnel.
 *
 * @param tunnelId The id of the tunnel through which the data was
 * received.  Ver.: always
 * @param data Buffer containing the raw octets of the data.  Ver.: always
 * @param dataLen The length in octets of the data.  Ver.: always
 */
void sl_zigbee_af_tunneling_client_data_received_cb(uint8_t tunnelId,
                                                    uint8_t *data,
                                                    uint16_t dataLen)
{
  uint8_t tunnelIndex;

  sl_zigbee_af_debug_print("CHF: ClientDataReceived:%02X,[", tunnelId);
  sl_zigbee_af_debug_print_buffer(data, dataLen, false);
  sl_zigbee_af_debug_println("]");

  tunnelIndex = findTunnelByTunnelId(tunnelId, CLIENT_TUNNEL);
  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    sli_zigbee_af_comms_hub_function_tunnel_data_received_callback(tunnels[tunnelIndex].remoteDeviceId,
                                                                   dataLen,
                                                                   data);
  }
}

/** @brief Tunnel Closed
 *
 * This function is called by the Tunneling client plugin whenever a server
 * sends a notification that it preemptively closed an inactive tunnel.
 * Servers are not required to notify clients of tunnel closures, so
 * applications cannot rely on this callback being called for all tunnels.
 *
 * @param tunnelId The ID of the tunnel that has been closed.  Ver.:
 * always
 */
void sl_zigbee_af_tunneling_client_tunnel_closed_cb(uint8_t tunnelId)
{
  uint8_t tunnelIndex;
  sl_zigbee_af_debug_println("CHF: ClientTunnelClosed:0x%02X", tunnelId);

  tunnelIndex = findTunnelByTunnelId(tunnelId, CLIENT_TUNNEL);
  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    tunnels[tunnelIndex].state = CLOSED_TUNNEL;
    if (tunnelIndex == responsePendingIndex) {
      responsePendingIndex = EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;
      sl_zigbee_af_event_set_active(tunnelEventControl);
    }
  }
}

/** @brief Is Protocol Supported
 *
 * This function is called by the Tunneling server plugin whenever a Request
 * Tunnel command is received.  The application should return true if the
 * protocol is supported and false otherwise.
 *
 * @param protocolId The identifier of the metering communication protocol for
 * which the tunnel is requested.  Ver.: always
 * @param manufacturerCode The manufacturer code for manufacturer-defined
 * protocols or 0xFFFF in unused.  Ver.: always
 */
bool sl_zigbee_af_tunneling_server_is_protocol_supported_cb(uint8_t protocolId,
                                                            uint16_t manufacturerCode)
{
  sl_802154_long_addr_t remoteDeviceId;

  sl_zigbee_af_debug_println("CHF: ServerIsProtocolSupported:0x%02X 0x%04X", protocolId, manufacturerCode);

  // Since the tunneling cluster server code does not pass the EUI64 or the
  // node ID of the remote end of the tunnel so we need to look them up.
  // Luckily this callback is called in the context of the RequestTunnel
  // command processing and we look into the command for this info.
  sl_zigbee_lookup_eui64_by_node_id(sl_zigbee_af_current_command()->source, remoteDeviceId);

  return (GBCS_TUNNELING_PROTOCOL_ID == protocolId
          && GBCS_TUNNELING_MANUFACTURER_CODE == manufacturerCode
          && sli_zigbee_af_comms_hub_function_tunnel_accept_callback(remoteDeviceId));
}

/** @brief Tunnel Opened
 *
 * This function is called by the Tunneling server plugin whenever a tunnel is
 * opened.  Clients may open tunnels by sending a Request Tunnel command.
 *
 * @param tunnelId The identifier of the tunnel that has been opened.  Ver.:
 * always
 * @param protocolId The identifier of the metering communication protocol for
 * the tunnel.  Ver.: always
 * @param manufacturerCode The manufacturer code for manufacturer-defined
 * protocols or 0xFFFF in unused.  Ver.: always
 * @param flowControlSupport true is flow control support is requested or false
 * if it is not.  Ver.: always
 * @param maximumIncomingTransferSize The maximum incoming transfer size of the
 * client.  Ver.: always
 */
void sl_zigbee_af_tunneling_server_tunnel_opened_cb(uint16_t tunnelId,
                                                    uint8_t protocolId,
                                                    uint16_t manufacturerCode,
                                                    bool flowControlSupport,
                                                    uint16_t maximumIncomingTransferSize)
{
  UNUSED_VAR(protocolId);
  UNUSED_VAR(manufacturerCode);
  UNUSED_VAR(flowControlSupport);

  sl_802154_long_addr_t remoteDeviceId;
  sl_802154_short_addr_t remoteNodeId;
  uint8_t tunnelIndex;

  sl_zigbee_af_debug_println("CHF: ServerTunnelOpened:0x%02X,0x%04X", tunnelId, maximumIncomingTransferSize);

  // Since the tunneling cluster server code does not pass the EUI64 or the
  // node ID of the remote end of the tunnel so we need to look them up.
  // Luckily this callback is called in the context of the RequestTunnel
  // command processing and we look into the command for this info.
  remoteNodeId = sl_zigbee_af_current_command()->source;
  sl_zigbee_lookup_eui64_by_node_id(sl_zigbee_af_current_command()->source, remoteDeviceId);

  // We only support one tunnel to a given remote device/endpoint so if we
  // already have a tunnel lets work with it.
  tunnelIndex = findTunnelByDeviceId(remoteDeviceId);
  if (tunnelIndex == EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    // Find a slot in the tunnels table for the new tunnel
    tunnelIndex = findUnusedTunnel();
  }

  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    memcpy(tunnels[tunnelIndex].remoteDeviceId, remoteDeviceId, EUI64_SIZE);
    tunnels[tunnelIndex].remoteNodeId = remoteNodeId;
    tunnels[tunnelIndex].remoteEndpoint = sl_zigbee_af_current_command()->apsFrame->sourceEndpoint;
    tunnels[tunnelIndex].type = SERVER_TUNNEL;
    tunnels[tunnelIndex].state = ACTIVE_TUNNEL;
    tunnels[tunnelIndex].tunnelId = tunnelId;
    tunnels[tunnelIndex].timeoutMSec = 0;

    // Per GBCS v0.8 section 10.2.2, Devices supporting the Tunneling Cluster
    // as a Server shall have a MaximumIncomingTransferSize set to 1500 octets,
    // in line with the ZSE default.  All Devices supporting the Tunneling
    // Cluster shall use this value in any RequestTunnelResponse command and
    // any RequestTunnel command.
    //
    // So rather than bring down the tunnel in the case when the maximumIncomingTransferSize
    // is less than 1500 we'll just log a warning message.
    if (maximumIncomingTransferSize < 1500) {
      sl_zigbee_af_comms_hub_function_println("Warning: tunnel opened but MaximumIncomingTransferSize of client is %d but should be 1500",
                                              maximumIncomingTransferSize);
    }
    return;
  }

  // This is a misconfiguration or a bug in the code calling this API. Either
  // the tunnel client plugin limit is set too low for the number of tunnels
  // required or the code that is calling this function is in error.  Either way,
  // we'll print the error and return false indicating that the tunnel was
  // not created.
  sl_zigbee_af_comms_hub_function_println("%s%s%s",
                                          "Error: ",
                                          "Tunnel Opened failed: ",
                                          "Too many tunnels");
}

/** @brief Data Received
 *
 * This function is called by the Tunneling server plugin whenever data is
 * received from a client through a tunnel.
 *
 * @param tunnelId The identifier of the tunnel through which the data was
 * received.  Ver.: always
 * @param data Buffer containing the raw octets of the data.  Ver.: always
 * @param dataLen The length in octets of the data.  Ver.: always
 */
void sl_zigbee_af_tunneling_server_data_received_cb(uint16_t tunnelId,
                                                    uint8_t * data,
                                                    uint16_t dataLen)
{
  uint8_t tunnelIndex;
  sl_zigbee_af_debug_print("CHF: ServerDataReceived:%02X,[", tunnelId);
  sl_zigbee_af_debug_print_buffer(data, dataLen, false);
  sl_zigbee_af_debug_println("]");

  tunnelIndex = findTunnelByTunnelId(tunnelId, SERVER_TUNNEL);
  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    sli_zigbee_af_comms_hub_function_tunnel_data_received_callback(tunnels[tunnelIndex].remoteDeviceId,
                                                                   dataLen,
                                                                   data);
  }
}

/** @brief Tunnel Closed
 *
 * This function is called by the Tunneling server plugin whenever a tunnel is
 * closed.  Clients may close tunnels by sending a Close Tunnel command.  The
 * server can preemptively close inactive tunnels after a timeout.
 *
 * @param tunnelId The identifier of the tunnel that has been closed.  Ver.:
 * always
 * @param clientInitiated true if the client initiated the closing of the tunnel
 * or false if the server closed the tunnel due to inactivity.  Ver.: always
 */
void sl_zigbee_af_tunneling_server_tunnel_closed_cb(uint16_t tunnelId,
                                                    bool clientInitiated)
{
  UNUSED_VAR(clientInitiated);
  uint8_t tunnelIndex;
  sl_zigbee_af_debug_println("CHF: ServerTunnelClosed:0x%02X", tunnelId);

  tunnelIndex = findTunnelByTunnelId(tunnelId, SERVER_TUNNEL);
  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    tunnels[tunnelIndex].state = CLOSED_TUNNEL;
  }
}

/** @brief Data Error
 *
 * This function is called by the Tunneling server plugin whenever a data error
 * occurs on a tunnel.  Errors occur if a device attempts to send data on tunnel
 * that is no longer active or if the tunneling does not belong to the device.
 *
 * @param tunnelIndex The identifier of the tunnel on which this data error
 * occurred.  Ver.: always
 * @param transferDataStatus The error that occurred.  Ver.: always
 */
void sl_zigbee_af_tunneling_server_data_error_cb(uint16_t tunnelIndex,
                                                 sl_zigbee_af_tunneling_transfer_data_status_t transferDataStatus)
{
  sl_zigbee_af_comms_hub_function_println("CHF: ServerDataError: 0x%02X, 0x%02X", tunnelIndex, transferDataStatus);
}

/** @brief Data Error
 *
 * This function is called by the Tunneling client plugin whenever a data error
 * occurs on a tunnel.  Errors occur if a device attempts to send data on tunnel
 * that is no longer active or if the tunneling does not belong to the device.
 *
 * @param tunnelIndex The index of the tunnel on which this data error occurred.
 *  Ver.: always
 * @param transferDataStatus The error that occurred.  Ver.: always
 */
void sl_zigbee_af_tunneling_client_data_error_cb(uint8_t tunnelIndex,
                                                 sl_zigbee_af_tunneling_transfer_data_status_t transferDataStatus)
{
  uint8_t tunnelId;
  tunnelId = findTunnelByTunnelId(tunnelIndex, CLIENT_TUNNEL);
  sl_zigbee_af_comms_hub_function_println("CHF: ClientDataError: 0x%02X, 0x%02X", tunnelIndex, transferDataStatus);
  if (transferDataStatus == SL_ZIGBEE_ZCL_TUNNELING_TRANSFER_DATA_STATUS_NO_SUCH_TUNNEL) {
    sl_zigbee_af_comms_hub_function_println("No Tunnel Found: New tunnel is requested for the device.");
    sli_zigbee_af_comms_hub_function_tunnel_cleanup(tunnels[tunnelId].remoteDeviceId);
    requestTunnel(tunnelId);
  }
}

//------------------------------------------------------------------------------
// Internal Functions

static uint8_t findUnusedTunnel(void)
{
  uint8_t tunnelIndex;
  for (tunnelIndex = 0; tunnelIndex < SL_ZIGBEE_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT; tunnelIndex++) {
    if (tunnels[tunnelIndex].state == UNUSED_TUNNEL) {
      return tunnelIndex;
    }
  }
  return EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;
}

static bool requestTunnel(uint8_t tunnelIndex)
{
  sl_zigbee_af_plugin_tunneling_client_status_t status;

  // The tunneling cluster client code can only process one tunneling reuqest
  // at a time so if there's already on outstanding then mark this one pending
  // and let the tunnel event handler take care of the retry.
  if (responsePendingIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    tunnels[tunnelIndex].state = REQUEST_PENDING_TUNNEL;
    tunnels[tunnelIndex].timeoutMSec = halCommonGetInt32uMillisecondTick();
    sl_zigbee_af_event_set_active(tunnelEventControl);
    return true;
  }

  status = sl_zigbee_af_tunneling_client_request_tunnel(tunnels[tunnelIndex].remoteNodeId,
                                                        SL_ZIGBEE_AF_PLUGIN_COMMS_HUB_FUNCTION_COMMSHUB_TUNNEL_CLIENT_ENDPOINT,
                                                        tunnels[tunnelIndex].remoteEndpoint,
                                                        GBCS_TUNNELING_PROTOCOL_ID,
                                                        GBCS_TUNNELING_MANUFACTURER_CODE,
                                                        GBCS_TUNNELING_FLOW_CONTROL_SUPPORT);
  if (status != SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_SUCCESS
      && !handleRequestTunnelFailure(tunnelIndex, status)) {
    return false;
  }

  responsePendingIndex = tunnelIndex;
  tunnels[tunnelIndex].state = RESPONSE_PENDING_TUNNEL;
  return true;
}

// See if we can recover from tunnel creation issues.
static bool handleRequestTunnelFailure(uint8_t tunnelIndex, sl_zigbee_af_plugin_tunneling_client_status_t status)
{
  uint8_t i;

  sl_zigbee_af_debug_println("CHF: handleRequestTunnelFailure 0x%02X, 0x%02X",
                             tunnelIndex, status);

  if (status == SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_BUSY) {
    // Per GBCS send another request 3 minutes from now
    tunnels[tunnelIndex].state = REQUEST_PENDING_TUNNEL;
    tunnels[tunnelIndex].timeoutMSec = halCommonGetInt32uMillisecondTick()
                                       + (MILLISECOND_TICKS_PER_SECOND * 180);
    sl_zigbee_af_event_set_active(tunnelEventControl);
    sl_zigbee_af_comms_hub_function_println("CHF: Busy status received from node ID 0x%04X", tunnels[tunnelIndex].remoteNodeId);
    return true;
  } else if (status == SL_ZIGBEE_AF_PLUGIN_TUNNELING_CLIENT_NO_MORE_TUNNEL_IDS) {
    // Per GBCS close any other tunnels we may have with the device
    // and once all responses are received try the RequestTunnel again
    bool retryRequest = false;
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT; i++) {
      if (i != tunnelIndex && tunnels[i].remoteNodeId == tunnels[tunnelIndex].remoteNodeId) {
        retryRequest = true;
        sli_zigbee_af_comms_hub_function_tunnel_destroy(tunnels[i].remoteDeviceId);
      }
    }
    if (retryRequest) {
      // We'll retry the request in the tunnel event handler so as to give the
      // tunnel(s) a chance to clean up.
      tunnels[tunnelIndex].state = REQUEST_PENDING_TUNNEL;
      tunnels[tunnelIndex].timeoutMSec = halCommonGetInt32uMillisecondTick()
                                         + (MILLISECOND_TICKS_PER_SECOND * 5);
      sl_zigbee_af_event_set_active(tunnelEventControl);
      return true;
    }
    // no tunnels were closed so nothing more we can do
    sl_zigbee_af_comms_hub_function_println("%s%s%s",
                                            "Error: ",
                                            "Tunnel Create failed: ",
                                            "No more tunnel ids");
    tunnels[tunnelIndex].state = CLOSED_TUNNEL;
    return false;
  }

  // All other errors are either due to mis-configuration or errors that we
  // cannot recover from so print the error and return false.
  sl_zigbee_af_comms_hub_function_println("%s%s%s0x%02X",
                                          "Error: ",
                                          "Tunnel Create failed: ",
                                          "Tunneling Client Status: ",
                                          status);
  tunnels[tunnelIndex].state = CLOSED_TUNNEL;
  return false;
}

// Find an active tunnel to the given device.
static uint8_t findTunnelByDeviceId(sl_802154_long_addr_t remoteDeviceId)
{
  uint8_t tunnelIndex;

  // sl_zigbee_af_debug_print("CHF: findTunnelByDeviceId ");
  // sl_zigbee_af_debug_debug_exec(sl_zigbee_af_print_big_endian_eui64(remoteDeviceId));
  // sl_zigbee_af_debug_println("");

  for (tunnelIndex = 0; tunnelIndex < SL_ZIGBEE_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT; tunnelIndex++) {
    // sl_zigbee_af_debug_print("CHF: findTunnelByDeviceId compare to 0x%02X ",
    //                   tunnels[tunnelIndex].state);
    // sl_zigbee_af_debug_debug_exec(sl_zigbee_af_print_big_endian_eui64(tunnels[tunnelIndex].remoteDeviceId));
    // sl_zigbee_af_debug_println("");
    if (tunnels[tunnelIndex].state != UNUSED_TUNNEL
        && (memcmp(tunnels[tunnelIndex].remoteDeviceId, remoteDeviceId, EUI64_SIZE) == 0)) {
      return tunnelIndex;
    }
  }
  return EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;
}

// Find an active tunnel from the given client tunneling plugin identifer,
static uint8_t findTunnelByTunnelId(uint8_t tunnelId, sli_zigbee_af_comms_hub_function_tunnel_type type)
{
  uint8_t tunnelIndex;

  // sl_zigbee_af_debug_println("CHF: findTunnelByTunnelId 0x%02X 0x%02X",
  //                     tunnelId, type);

  for (tunnelIndex = 0; tunnelIndex < SL_ZIGBEE_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT; tunnelIndex++) {
    // sl_zigbee_af_debug_println("CHF: findTunnelByTunnelId compare to 0x%02X 0x%02X 0x%02X",
    //                     tunnels[tunnelIndex].state, tunnels[tunnelIndex].tunnelId, tunnels[tunnelIndex].type);
    if (tunnels[tunnelIndex].state != UNUSED_TUNNEL
        && tunnels[tunnelIndex].tunnelId == tunnelId
        && tunnels[tunnelIndex].type == type) {
      return tunnelIndex;
    }
  }
  return EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;
}
