/********************************************************************************
 * @file  app_xapi_ap.c
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "sl_status.h"
#include "app_wifi_xapi_cmd_rx_handler.h"
#include "si91x_device.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_driver.h"
#include "FreeRTOS.h"
#include "task.h"

DEFINE_XAPI_CLASS(wifi, ap, WIFI, AP, NULL, NULL);

/******************************************************
*               Function Declarations
******************************************************/

void app_wifi_cmd_ap_disconnect_client(app_wifi_cmd_ap_disconnect_client_t *msg)
{
  // Initialize status to failure
  sl_status_t status = SL_STATUS_FAIL;

  // Cast the MAC address from the message to sl_mac_address_t*
  sl_mac_address_t *mac_address = (sl_mac_address_t *)&msg->mac_address;

  if (msg->net_interface == APP_WIFI_NET_INTF_INTERFACE_AP) {
    // Attempt to disconnect the client using the provided MAC address and reason
    status = sl_wifi_disconnect_ap_client(SL_WIFI_AP_INTERFACE, mac_address, msg->reason);
  }

  // Respond with the status of the disconnect operation
  app_wifi_rsp_ap_disconnect_client(status);
  return;
}

void app_wifi_cmd_ap_get_client_info(app_wifi_cmd_ap_get_client_info_t *msg)
{
  // Initialize status to failure
  sl_status_t status = SL_STATUS_FAIL;

  // Check if the network interface is not AP, return invalid parameter status
  if (msg->net_interface != APP_WIFI_NET_INTF_INTERFACE_AP) {
    app_wifi_rsp_ap_get_client_info(SL_STATUS_INVALID_PARAMETER, 0);
    return;
  }

  // Initialize client info response structure
  sl_wifi_client_info_response_t client_info = { 0 };

  // If the network interface is AP, get the client info
  status = sl_wifi_get_ap_client_info(SL_WIFI_AP_INTERFACE, &client_info);
  if (status != SL_STATUS_OK) {
    app_wifi_rsp_ap_get_client_info(status, 0);
    return;
  }

  // Return success status with the number of connected clients
  app_wifi_rsp_ap_get_client_info(SL_STATUS_OK, client_info.client_count);
  // Iterate through the connected clients and send their info through xAPI event handler
  for (uint16_t station_info_index = 0; station_info_index < client_info.client_count; station_info_index++) {
    sl_wifi_client_info_t *station_info = &client_info.client_info[station_info_index];
    // Send client info event with IP and MAC address
    app_wifi_evt_ap_get_client_info_event(&station_info->ip_address.ip.v4.bytes, &station_info->mac_adddress);
  }

  return;
}
