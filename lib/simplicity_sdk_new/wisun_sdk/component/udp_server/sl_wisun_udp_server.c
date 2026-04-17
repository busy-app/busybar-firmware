/***************************************************************************//**
 * @file sl_wisun_udp_server.c
 * @brief Wi-SUN UDP server
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_assert.h"
#include "sl_wisun_udp_server.h"
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "sl_wisun_app_core_util.h"
#include "sl_wisun_trace_util.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define SL_WISUN_UDP_SERVER_STACK_SIZE_WORD         (192U)
#define SL_WISUN_UDP_SERVER_DELAY_MS                (500UL)
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief UDP Server task function
 * @details Thread function of UDP Server
 * @param[in] args Arguments (not used)
 *****************************************************************************/
static void _udp_server_task_fnc(void *args);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// UDP Server thread id
static osThreadId_t _udp_server_thr_id;

/// UDP Server task attributes
static const osThreadAttr_t _udp_server_task_attr = {
  .name       = "UDP Server",
  .attr_bits  = osThreadDetached,
  .cb_mem     = NULL,
  .cb_size    = 0,
  .stack_mem  = NULL,
  .stack_size = app_stack_size_word_to_byte(SL_WISUN_UDP_SERVER_STACK_SIZE_WORD),
  .priority   = osPriorityNormal,
  .tz_module  = 0
};

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/* UDP Server init */
void sl_wisun_udp_server_init(void)
{
  _udp_server_thr_id = osThreadNew(_udp_server_task_fnc, NULL, &_udp_server_task_attr);
  EFM_ASSERT(_udp_server_thr_id != NULL);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

/* UDP Server task function */
static void _udp_server_task_fnc(void *args)
{
  static char buff[SL_WISUN_UDP_SERVER_BUFF_SIZE] = { 0U };
  static sockaddr_in6_t srv_addr_udp              = { 0U };
  static sockaddr_in6_t clnt_addr_udp             = { 0U };
  socklen_t len                                   = sizeof(clnt_addr_udp);
  int32_t r                                       = SOCKET_RETVAL_ERROR;
  int32_t sockd_udp_srv                           = SOCKET_INVALID_ID;
  const char *ip_str                              = NULL;

  (void) args;

  // waiting for connection
  sl_wisun_app_core_util_wait_for_connection();
  osDelay(SL_WISUN_UDP_SERVER_DELAY_MS);
  printf("[UDP Server port: %u]\n", SL_WISUN_UDP_SERVER_PORT);

  // creating socket
  sockd_udp_srv = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  assert_res(sockd_udp_srv, "UDP server socket()");

  // fill the server address structure
  srv_addr_udp.sin6_family = AF_INET6;
  srv_addr_udp.sin6_addr = in6addr_any;
  srv_addr_udp.sin6_port = htons(SL_WISUN_UDP_SERVER_PORT);

  // bind address to the socket
  r = bind(sockd_udp_srv, (const struct sockaddr *) &srv_addr_udp, sizeof(struct sockaddr_in6));
  assert_res(r, "UDP server bind()");

  // receiver loop
  SL_WISUN_THREAD_LOOP {
    r = recvfrom(sockd_udp_srv, buff, SL_WISUN_UDP_SERVER_BUFF_SIZE - 1, 0, (struct sockaddr *) &clnt_addr_udp, &len);

    if (r > 0) {
      buff[r] = 0;
      // print the received message
      ip_str = app_wisun_trace_util_get_ip_str((void *) &clnt_addr_udp.sin6_addr);
      printf("[%s] %s\n", ip_str, buff);
      app_wisun_trace_util_destroy_ip_str(ip_str);

      // send back to the client
      if (sendto(sockd_udp_srv, buff, r, 0, (const struct sockaddr *) &clnt_addr_udp, len) == -1) {
        printf("Error sendto()\n");
      }
    }
    // dispatch thread
    sl_wisun_app_core_util_dispatch_thread();
  }
}
