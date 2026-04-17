/***************************************************************************//**
 * @file app.c
 * @brief Application code
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cmsis_os2.h>

#include "sl_status.h"
#include "sl_assert.h"
#include "sl_string.h"
#include "sl_component_catalog.h"
#include "sl_memory_manager.h"
#include "border_router/sl_wisun_br_api.h"
#include "sl_wisun_api.h"
#include "sl_wisun_event_mgr.h"
#include "sl_wisun_keychain.h"
#include "sl_select_util.h"
#include "select.h"
#include "sl_wisun_config.h"
#include "sl_wisun_br_config.h"
#include "app.h"
#include "sl_wisun_app_setting_br.h"
#include "sl_wisun_app_core_util.h"
#include "sl_wisun_br_dhcpv6_server.h"
#include "sl_wisun_types.h"
#include "border_router/sl_wisun_br_connection_params_api.h"
#include "sl_wisun_trace_util.h"

#include "sl_cli.h"
#include "sl_wisun_br_lwip.h"
#include "sl_wisun_br_wifi.h"
#include "sl_wisun_br_agent_service.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @brief Initialize the DHCPv6 socket
 *
 * @return sl_status_t
 *****************************************************************************/
static sl_status_t app_init_dhcpv6_socket(void);

/**************************************************************************//**
 * @brief Wi-Fi connection status handler
 *
 * @param connected bool
 *****************************************************************************/
static void app_wifi_on_join(bool connected);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
int app_dhcpv6_socket = SOCKET_INVALID_ID;
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/* Wi-SUN socket data available event handler */
void sl_wisun_socket_data_available_event_hnd(sl_wisun_evt_t *evt)
{
  (void) evt;
  sl_wisun_check_read_sockfd_set();
}

/* Wi-SUN socket data sent event handler */
void sl_wisun_socket_data_sent_event_hnd(sl_wisun_evt_t *evt)
{
  (void) evt;
  sl_wisun_check_write_sockfd_set();
}

/* App task function */
void app_task(void *args)
{
  (void) args;

  sl_wisun_app_core_util_project_info_init("Wi-SUN Border Router Agent Service Application");

  EFM_ASSERT(sl_wisun_br_set_ipv6_up_handler(sl_wisun_br_lwip_pan_input) == SL_STATUS_OK);
  sl_wisun_br_lwip_init();
  EFM_ASSERT(sl_wisun_br_wifi_init() == SL_STATUS_OK);
  EFM_ASSERT(sl_wisun_br_wifi_set_join_handler(app_wifi_on_join) == SL_STATUS_OK);
  EFM_ASSERT(sl_wisun_br_dhcpv6_server_init() == SL_STATUS_OK);
  EFM_ASSERT(app_init_dhcpv6_socket() == SL_STATUS_OK);

  app_service_task_init();

  while (1) {
    ///////////////////////////////////////////////////////////////////////////
    // Put your application code here!                                       //
    ///////////////////////////////////////////////////////////////////////////
    sl_wisun_app_core_util_dispatch_thread();
  }
}

/* App DHCPv6 service task function */
void app_service_task(void *args)
{
  static uint8_t buffer[350] = { 0 };
  ssize_t count = 0;
  sockaddr_in6_t src_addr = { 0 };
  socklen_t addrlen = sizeof(sockaddr_in6_t);
  fd_set readfds = { 0 };
  int max_sd = -1;

  (void) args;

  while (1) {
    FD_ZERO(&readfds);
    if (app_dhcpv6_socket >= 0) {
      FD_SET(app_dhcpv6_socket, &readfds);
      max_sd = MAX(app_dhcpv6_socket, max_sd);
    }

    if (max_sd < 0
        || select(max_sd + 1, &readfds, NULL, NULL, NULL) < 0) {
      break;
    }

    if ((app_dhcpv6_socket >= 0) && (FD_ISSET(app_dhcpv6_socket, &readfds))) {
      count = recvfrom(app_dhcpv6_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&src_addr, &addrlen);
      if (count >= 0) {
        sl_wisun_br_dhcpv6_server_on_recv(buffer, count, src_addr.sin6_addr, src_addr.sin6_port);
      }
    }
  }
  osThreadExit();
}
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static sl_status_t app_init_dhcpv6_socket(void)
{
  int retval = 0;

  const sockaddr_in6_t bind_addr = {
    .sin6_family = AF_INET6,
    .sin6_port = htons(DHCPV6_SERVER_PORT),
    .sin6_flowinfo = 0,
    .sin6_addr = IN6ADDR_ANY_INIT,
    .sin6_scope_id = 0,
  };

  app_dhcpv6_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  if (app_dhcpv6_socket == SOCKET_INVALID_ID) {
    printf("[Failed: failed to open DHCPv6 socket]\n");
    close(app_dhcpv6_socket);
    return SL_STATUS_FAIL;
  }
  retval = bind(app_dhcpv6_socket, (const struct sockaddr *)&bind_addr, sizeof(sockaddr_in6_t));
  if (retval < 0) {
    printf("[Failed: failed to bind DHCPv6 socket (%d)]\n", retval);
    close(app_dhcpv6_socket);
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

static void app_wifi_on_join(bool connected)
{
  printf("[wifi: connection %s]\n", connected ? "successful" : "failure");
  if (connected) {
    sl_wisun_agent_start_service();
  }
}
