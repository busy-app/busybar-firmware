/***************************************************************************//**
 * @file
 * @brief CLI for the Tunneling Client plugin.
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
#include "tunneling-client.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"

void sli_zigbee_af_tunneling_client_cli_print_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sli_zigbee_af_tunneling_client_cli_print();
}
// plugin tunneling-client request <server:2> <client endpoint:1> <server endpoint:1> <protocol id:1> <manufacturer code:2> <flow control support:1>
void sli_zigbee_af_tunneling_client_cli_request(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t server = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(arguments, 0);
  uint8_t clientEndpoint = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t serverEndpoint = sl_cli_get_argument_uint8(arguments, 2);
  uint8_t protocolId = sl_cli_get_argument_uint8(arguments, 3);
  uint16_t manufacturerCode = sl_cli_get_argument_uint16(arguments, 4);
  bool flowControlSupport = (bool)sl_cli_get_argument_uint8(arguments, 5);
  sl_zigbee_af_plugin_tunneling_client_status_t status = sl_zigbee_af_tunneling_client_request_tunnel(server,
                                                                                                      clientEndpoint,
                                                                                                      serverEndpoint,
                                                                                                      protocolId,
                                                                                                      manufacturerCode,
                                                                                                      flowControlSupport);
  sl_zigbee_af_tunneling_cluster_println("%s 0x%02X", "request", status);
}

// plugin tunneling-client transfer <tunnel index:1> <data>
void sli_zigbee_af_tunneling_client_cli_transfer(sl_cli_command_arg_t *arguments)
{
  uint8_t tunnelIndex = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t data[255];
  uint16_t dataLen = sl_zigbee_copy_hex_arg(arguments, 1, data, sizeof(data), false);
  sl_zigbee_af_status_t status = sl_zigbee_af_tunneling_client_transfer_data(tunnelIndex,
                                                                             data,
                                                                             dataLen);
  sl_zigbee_af_tunneling_cluster_println("%s 0x%02X", "transfer", status);
}

// plugin tunneling-client close <tunnel index:1>
void sli_zigbee_af_tunneling_client_cli_close(sl_cli_command_arg_t *arguments)
{
  uint8_t tunnelIndex = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_status_t status = sl_zigbee_af_tunneling_client_close_tunnel(tunnelIndex);
  sl_zigbee_af_tunneling_cluster_println("%s 0x%02X", "close", status);
}
