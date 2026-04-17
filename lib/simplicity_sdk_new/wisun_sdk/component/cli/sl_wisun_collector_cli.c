/***************************************************************************//**
 * @file sl_wisun_collector_cli.c
 * @brief Wi-SUN Collector CLI handler
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdio.h>

#include "sl_string.h"
#include "sl_cli.h"
#include "sl_status.h"
#include "socket/socket.h"
#include "sl_wisun_cli_core.h"
#include "sl_wisun_meter_collector_config.h"
#include "sli_wisun_meter_collector.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define __build_request(__type) \
  __type SL_WISUN_METER_REQUEST_DELIMITER SL_WISUN_METER_COLLECTOR_TOKEN

#define SL_WISUN_COLLECTOR_CLI_REQUEST_STR_REG \
  __build_request(SL_WISUN_METER_REQUEST_TYPE_STR_REGISTER)

#define SL_WISUN_COLLECTOR_CLI_REQUEST_STR_RM \
  __build_request(SL_WISUN_METER_REQUEST_TYPE_STR_REMOVE)

#define SL_WISUN_COLLECTOR_CLI_REQUEST_STR_ASYNC \
  __build_request(SL_WISUN_METER_REQUEST_TYPE_STR_ASYNC)
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static void _send_request(const char *ip_str, const char *request);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/* CLI app register meter handler */
void app_register_meter(const sl_cli_command_arg_t * arguments)
{
  const char *meter_ip = NULL;

  meter_ip = sl_cli_get_argument_string(arguments, 0);
  if (meter_ip == NULL) {
    return;
  }

  _send_request(meter_ip, SL_WISUN_COLLECTOR_CLI_REQUEST_STR_REG);
}

/* CLI app remove meter handler */
void app_remove_meter(const sl_cli_command_arg_t * arguments)
{
  const char *meter_ip = NULL;

  meter_ip = sl_cli_get_argument_string(arguments, 0);
  if (meter_ip == NULL) {
    return;
  }

  _send_request(meter_ip, SL_WISUN_COLLECTOR_CLI_REQUEST_STR_RM);
}

/* CLI app async request handler */
void app_async_request(const sl_cli_command_arg_t * arguments)
{
  const char *meter_ip = NULL;

  meter_ip = sl_cli_get_argument_string(arguments, 0);
  if (meter_ip == NULL) {
    return;
  }

  _send_request(meter_ip, SL_WISUN_COLLECTOR_CLI_REQUEST_STR_ASYNC);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static void _send_request(const char *ip_str, const char *request)
{
  int32_t sockid = SOCKET_INVALID_ID;
  sockaddr_in6_t meter_addr = {
    .sin6_family = AF_INET6,
    .sin6_addr = { .address = { 0U } },
    .sin6_port = htons(SL_WISUN_METER_COLLECTOR_PORT)
  };

  if (ip_str == NULL || request == NULL) {
    printf("[Failed: invalid collector send request parameter]\n");
    return;
  }

  if (inet_pton(AF_INET6, ip_str, &meter_addr.sin6_addr) != 1) {
    printf("[Failed: invalid remote address parameter]\n");
    return;
  }

  sockid = socket(AF_INET6, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
  if (sockid == SOCKET_INVALID_ID) {
    printf("[Failed: collector socket creation failed]\n");
    return;
  }

  if (sendto(sockid, request, sl_strlen(request), 0,
             (const struct sockaddr *)&meter_addr, sizeof(meter_addr)) == SOCKET_RETVAL_ERROR) {
    printf("[Failed: collector sendto failed]\n");
    return;
  }

  printf("[Request sent: '%s']\n", request);
  close(sockid);
}
