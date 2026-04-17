/***************************************************************************//**
 * @file
 * @brief Routines for the HC 11073 Tunnel plugin, which implements the
 *        tunneling of 11073 data over zigbee.
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
#include "11073-tunnel.h"

#include "zap-cluster-command-parser.h"

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_11073_protocol_tunnel_cluster_connect_request_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_11073_protocol_tunnel_cluster_connect_request_command_t cmd_data;
  bool connected = false;
  bool preemptible = false;
  // NOTE status set never checked.
  sl_zigbee_af_status_t status;
  (void) status;

  if (zcl_decode_11073_protocol_tunnel_cluster_connect_request_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  // Check to see if we are already connected by looking at connected attribute
  status = sl_zigbee_af_read_server_attribute(HC_11073_TUNNEL_ENDPOINT,
                                              CLUSTER_ID_11073_TUNNEL,
                                              ATTRIBUTE_11073_TUNNEL_CONNECTED,
                                              (uint8_t *)&connected,
                                              1);

  // if we are already connected send back connection status ALREADY_CONNECTED
  if (connected) {
    sl_zigbee_af_fill_command_11073_protocol_tunnel_cluster_connect_status_notification(
      SL_ZIGBEE_ZCL_11073_TUNNEL_CONNECTION_STATUS_ALREADY_CONNECTED);
    sl_zigbee_af_send_response();
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  // if not already connected copy attributes
  connected = true;
  status = sl_zigbee_af_write_server_attribute(HC_11073_TUNNEL_ENDPOINT,
                                               CLUSTER_ID_11073_TUNNEL,
                                               ATTRIBUTE_11073_TUNNEL_CONNECTED,
                                               (uint8_t *)&connected,
                                               ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  preemptible = cmd_data.connectControl
                & SL_ZIGBEE_ZCL_11073_CONNECT_REQUEST_CONNECT_CONTROL_PREEMPTIBLE;
  status = sl_zigbee_af_write_server_attribute(HC_11073_TUNNEL_ENDPOINT,
                                               CLUSTER_ID_11073_TUNNEL,
                                               ATTRIBUTE_11073_TUNNEL_PREEMPTIBLE,
                                               (uint8_t *)&preemptible,
                                               ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  status = sl_zigbee_af_write_server_attribute(HC_11073_TUNNEL_ENDPOINT,
                                               CLUSTER_ID_11073_TUNNEL,
                                               ATTRIBUTE_11073_TUNNEL_IDLE_TIMEOUT,
                                               (uint8_t*)&cmd_data.idleTimeout,
                                               ZCL_INT16U_ATTRIBUTE_TYPE);

  status = sl_zigbee_af_write_server_attribute(HC_11073_TUNNEL_ENDPOINT,
                                               CLUSTER_ID_11073_TUNNEL,
                                               ATTRIBUTE_11073_TUNNEL_MANAGER_TARGET,
                                               (uint8_t*)cmd_data.managerTarget,
                                               ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE);

  status = sl_zigbee_af_write_server_attribute(HC_11073_TUNNEL_ENDPOINT,
                                               CLUSTER_ID_11073_TUNNEL,
                                               ATTRIBUTE_11073_TUNNEL_MANAGER_ENDPOINT,
                                               &cmd_data.managerEndpoint,
                                               ZCL_INT8U_ATTRIBUTE_TYPE);

  // if idle timer other than 0xffff, set timer to disconnect, reset timer when
  // rx data

  // Generate conection status connected back to manager
  sl_zigbee_af_fill_command_11073_protocol_tunnel_cluster_connect_status_notification(
    SL_ZIGBEE_ZCL_11073_TUNNEL_CONNECTION_STATUS_CONNECTED);
  sl_zigbee_af_send_response();

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_11073_protocol_tunnel_cluster_disconnect_request_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_11073_protocol_tunnel_cluster_disconnect_request_command_t cmd_data;
  bool connected = false;
  sl_802154_long_addr_t currentManager;
  bool preemptible = false;
  // NOTE status set never checked.
  sl_zigbee_af_status_t status;
  (void) status;

  if (zcl_decode_11073_protocol_tunnel_cluster_disconnect_request_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  // check to see if already connected
  status = sl_zigbee_af_read_server_attribute(HC_11073_TUNNEL_ENDPOINT,
                                              CLUSTER_ID_11073_TUNNEL,
                                              ATTRIBUTE_11073_TUNNEL_CONNECTED,
                                              (uint8_t *)&connected,
                                              1);

  // if not currently connected, generate connection status DISCONNECTED
  if (!connected) {
    sl_zigbee_af_fill_command_11073_protocol_tunnel_cluster_connect_status_notification(
      SL_ZIGBEE_ZCL_11073_TUNNEL_CONNECTION_STATUS_DISCONNECTED);
    sl_zigbee_af_send_response();
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  // if is connected, is ieee address same or is pre-emptible set to true?
  status = sl_zigbee_af_read_server_attribute(HC_11073_TUNNEL_ENDPOINT,
                                              CLUSTER_ID_11073_TUNNEL,
                                              ATTRIBUTE_11073_TUNNEL_PREEMPTIBLE,
                                              (uint8_t *)&preemptible,
                                              1);

  if (!preemptible) {
    status = sl_zigbee_af_read_server_attribute(HC_11073_TUNNEL_ENDPOINT,
                                                CLUSTER_ID_11073_TUNNEL,
                                                ATTRIBUTE_11073_TUNNEL_MANAGER_TARGET,
                                                (uint8_t*)&currentManager,
                                                EUI64_SIZE);
    if (memcmp(&currentManager, cmd_data.managerIEEEAddress, EUI64_SIZE) != 0) {
      sl_zigbee_af_fill_command_11073_protocol_tunnel_cluster_connect_status_notification(
        SL_ZIGBEE_ZCL_11073_TUNNEL_CONNECTION_STATUS_NOT_AUTHORIZED);
      sl_zigbee_af_send_response();
      return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
    }
  }

  // Set attribute to disconnected
  connected = false;
  status = sl_zigbee_af_write_server_attribute(HC_11073_TUNNEL_ENDPOINT,
                                               CLUSTER_ID_11073_TUNNEL,
                                               ATTRIBUTE_11073_TUNNEL_CONNECTED,
                                               (uint8_t *)&connected,
                                               ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  // If it is authorized, then we can disconnect.Within 12 seconds device must send
  // DISCONNECTED notification to the manager device. Connected attribute set to
  // false to manager.
  sl_zigbee_af_fill_command_11073_protocol_tunnel_cluster_connect_status_notification(
    SL_ZIGBEE_ZCL_11073_TUNNEL_CONNECTION_STATUS_DISCONNECTED);
  sl_zigbee_af_send_response();
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;

  // Send another DISCONNECTED connection event to sender of message. (may be same
  // as manager, may be some other device).
}

uint32_t sl_zigbee_af_11073_protocol_tunnel_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                                         sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_CONNECT_REQUEST_COMMAND_ID:
      {
        status = sl_zigbee_af_11073_protocol_tunnel_cluster_connect_request_cb(cmd);
        break;
      }
      case ZCL_DISCONNECT_REQUEST_COMMAND_ID:
      {
        status = sl_zigbee_af_11073_protocol_tunnel_cluster_disconnect_request_cb(cmd);
        break;
      }
    }
  }

  return status;
}
