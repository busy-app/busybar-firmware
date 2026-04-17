/********************************************************************************
 * @file  app_xapi_ping.c
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
#include "sl_net_wifi_types.h"
#include "sl_net_ping.h"

DEFINE_XAPI_CLASS(wifi, ping, WIFI, PING, NULL, NULL);

/******************************************************
*               Function Definitions
******************************************************/

void app_wifi_cmd_ping_ipv4(app_wifi_cmd_ping_ipv4_t *msg)
{
  // Initialize status to failure
  sl_status_t status = SL_STATUS_FAIL;

  // Create an IP address structure and populate it with the provided IPv4 address
  sl_ip_address_t ip_address;
  ip_address.type = SL_IPV4;

  // The message is received with the byte order reversed, so it is being aligned to the correct byte order.
  ip_address.ip.v4.bytes[0] = msg->ipv4_address.addr[0];
  ip_address.ip.v4.bytes[1] = msg->ipv4_address.addr[1];
  ip_address.ip.v4.bytes[2] = msg->ipv4_address.addr[2];
  ip_address.ip.v4.bytes[3] = msg->ipv4_address.addr[3];

  // Send a ping request to the specified IP address with the given ping size
  status = sl_si91x_send_ping(ip_address, msg->ping_size);

  if (status == SL_STATUS_IN_PROGRESS) {
    status = SL_STATUS_OK;
  }

  // Respond with the status of the ping request
  app_wifi_rsp_ping_ipv4(status);
}
