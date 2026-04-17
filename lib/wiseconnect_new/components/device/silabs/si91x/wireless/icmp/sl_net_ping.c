/***************************************************************************/ /**
 * @file  sl_net_ping.c
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
#include "sl_constants.h"
#include "sl_net_ping.h"
#include "sl_si91x_driver.h"
#include <string.h>

/******************************************************
 *                      Macros
 ******************************************************/

// This macro must be used while sending ping timeout in sli_si91x_ping_request_t
#define SLI_CONVERT_TO_SI91X_PING_TIMEOUT(timeout) (timeout / 100)

/******************************************************
 *                    Constants
 ******************************************************/
#define SLI_PING_RESPONSE_TIMEOUT_MS 1000 // timeout in ms

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Extern Variable
 ******************************************************/
extern bool device_initialized;

/******************************************************
 *               Global Variable
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

sl_status_t sl_si91x_send_ping(sl_ip_address_t ip_address, uint16_t ping_size)
{
  sl_status_t status               = SL_STATUS_OK;
  sli_si91x_ping_request_t request = { 0 };

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if ((ip_address.type != SL_IPV4) && (ip_address.type != SL_IPV6)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (ip_address.type == SL_IPV6) {
    request.ip_version = SL_IPV6_VERSION;
    memcpy(&request.ping_address.ipv6_address, &ip_address.ip.v6, SL_IPV6_ADDRESS_LENGTH); // Copy IPv6 address
  } else {
    request.ip_version = SL_IPV4_VERSION;
    memcpy(&request.ping_address.ipv4_address, &ip_address.ip.v4, SL_IPV4_ADDRESS_LENGTH); // Copy IPv4 address
  }

  request.ping_size = ping_size;                                                       // Copy Ping size
  request.timeout   = SLI_CONVERT_TO_SI91X_PING_TIMEOUT(SLI_PING_RESPONSE_TIMEOUT_MS); // Copy Ping timeout

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_PING_PACKET,
                                         SLI_SI91X_NETWORK_CMD,
                                         &request,
                                         sizeof(sli_si91x_ping_request_t),
                                         SLI_WIFI_RETURN_IMMEDIATELY,
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);

  return status;
}
