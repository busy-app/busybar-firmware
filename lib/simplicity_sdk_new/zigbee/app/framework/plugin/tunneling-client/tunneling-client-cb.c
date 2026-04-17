/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

/** @brief Tunnel Opened
 *
 * This function is called by the Tunneling client plugin whenever a tunnel is
 * opened. Clients may open tunnels by sending a Request Tunnel command.
 *
 * @param tunnelIndex The index of the tunnel that has been opened.
 * Ver.: always
 * @param tunnelStatus The status of the request. Ver.: always
 * @param maximumIncomingTransferSize The maximum incoming transfer size of the
 * server. Ver.: always
 */
WEAK(void sl_zigbee_af_tunneling_client_tunnel_opened_cb(uint8_t tunnelIndex,
                                                         sl_zigbee_af_plugin_tunneling_client_status_t tunnelStatus,
                                                         uint16_t maximumIncomingTransferSize))
{
  UNUSED_VAR(tunnelIndex);
  UNUSED_VAR(tunnelStatus);
  UNUSED_VAR(maximumIncomingTransferSize);
}

/** @brief Data Received
 *
 * This function is called by the Tunneling client plugin whenever data is
 * received from a server through a tunnel.
 *
 * @param tunnelIndex The index of the tunnel through which the data was
 * received. Ver.: always
 * @param data Buffer containing the raw octets of the data. Ver.: always
 * @param dataLen The length in octets of the data. Ver.: always
 */
WEAK(void sl_zigbee_af_tunneling_client_data_received_cb(uint8_t tunnelIndex,
                                                         uint8_t *data,
                                                         uint16_t dataLen))
{
  UNUSED_VAR(tunnelIndex);
  UNUSED_VAR(data);
  UNUSED_VAR(dataLen);
}

/** @brief Data Error
 *
 * This function is called by the Tunneling client plugin whenever a data error
 * occurs on a tunnel. Errors occur if a device attempts to send data on tunnel
 * that is no longer active or if the tunneling does not belong to the device.
 *
 * @param tunnelIndex The index of the tunnel on which this data error
 * occurred. Ver.: always
 * @param transferDataStatus The error that occurred. Ver.: always
 */
WEAK(void sl_zigbee_af_tunneling_client_data_error_cb(uint8_t tunnelIndex,
                                                      sl_zigbee_af_tunneling_transfer_data_status_t transferDataStatus))
{
  UNUSED_VAR(tunnelIndex);
  UNUSED_VAR(transferDataStatus);
}

/** @brief Tunnel Closed
 *
 * This function is called by the Tunneling client plugin whenever a server
 * sends a notification that it preemptively closed an inactive tunnel. Servers
 * are not required to notify clients of tunnel closures, so applications
 * cannot rely on this callback being called for all tunnels.
 *
 * @param tunnelIndex The index of the tunnel that has been closed.
 * Ver.: always
 */
WEAK(void sl_zigbee_af_tunneling_client_tunnel_closed_cb(uint8_t tunnelIndex))
{
  UNUSED_VAR(tunnelIndex);
}

/** @brief Transfer Data Failure
 *
 * This function is called when a TransferData command fails to be sent to the
 * intended destination.
 *
 * @param indexOfDestination The index or destination address of the
 * TransferData command. Ver.: always
 * @param apsFrame The ::sl_zigbee_aps_frame_t of the command. Ver.: always
 * @param msgLen The length of the payload sent in the command. Ver.: always
 * @param message The payload that was sent in the command. Ver.: always
 * @param status The non-success status code from the transmission of the
 * command. Ver.: always
 */
WEAK(void sl_zigbee_af_tunneling_client_transfer_data_failure_cb(uint16_t indexOfDestination,
                                                                 sl_zigbee_aps_frame_t*apsFrame,
                                                                 uint16_t msgLen,
                                                                 uint8_t*message,
                                                                 sl_status_t status))
{
  UNUSED_VAR(indexOfDestination);
  UNUSED_VAR(msgLen);
  UNUSED_VAR(message);
  sl_zigbee_af_core_println("ERR: ClientTransferData failure (0x%04X) for APS SEQ %d",
                            status, apsFrame->sequence);
}
