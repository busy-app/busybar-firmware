/***************************************************************************//**
 * @file sl_wisun_tcp_server.c
 * @brief Wi-SUN TCP Server
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
#include "sl_wisun_tcp_server.h"
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "sl_wisun_app_core_util.h"
#include "sl_wisun_trace_util.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define SL_WISUN_TCP_SERVER_STACK_SIZE_WORD         (192U)
#define SL_WISUN_TCP_SERVER_DELAY_MS                (1000UL)
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief TCP Server task function
 * @details Thread function of TCP Server
 * @param[in] args Arguments (not used)
 *****************************************************************************/
static void _tcp_server_task_fnc(void *args);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// TCP Server thread id
static osThreadId_t _tcp_server_thr_id;

/// TCP Server task attributes
static const osThreadAttr_t _tcp_server_task_attr = {
  .name        = "TCP Server",
  .attr_bits   = osThreadDetached,
  .cb_mem      = NULL,
  .cb_size     = 0,
  .stack_mem   = NULL,
  .stack_size  = app_stack_size_word_to_byte(SL_WISUN_TCP_SERVER_STACK_SIZE_WORD),
  .priority    = osPriorityNormal,
  .tz_module   = 0
};

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/* TCP Server init */
void sl_wisun_tcp_server_init(void)
{
  _tcp_server_thr_id = osThreadNew(_tcp_server_task_fnc, NULL, &_tcp_server_task_attr);
  EFM_ASSERT(_tcp_server_thr_id != NULL);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

/* TCP Server task function */
static void _tcp_server_task_fnc(void *args)
{
  static char buff[SL_WISUN_TCP_SERVER_BUFF_SIZE] = { 0U };
  static sockaddr_in6_t srv_addr_tcp              = { 0U };
  static sockaddr_in6_t clnt_addr_tcp             = { 0U };
  socklen_t len                                   = sizeof(clnt_addr_tcp);
  int32_t sockd_tcp_srv                           = SOCKET_INVALID_ID;
  int32_t sockd_tcp_clnt                          = SOCKET_INVALID_ID;
  int32_t r                                       = SOCKET_RETVAL_ERROR;
  bool client_connected                           = false;
  const char *ip_str                              = NULL;
  bool print_prompt                               = true;

  (void) args;

  // waiting for connection
  sl_wisun_app_core_util_wait_for_connection();
  osDelay(SL_WISUN_TCP_SERVER_DELAY_MS);
  printf("[TCP Server port: %u]\n", SL_WISUN_TCP_SERVER_PORT);

  // creating socket
  sockd_tcp_srv = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
  assert_res(sockd_tcp_srv, "TCP server socket()");

  // fill the server address structure
  srv_addr_tcp.sin6_family = AF_INET6;
  srv_addr_tcp.sin6_addr = in6addr_any;
  srv_addr_tcp.sin6_port = htons(SL_WISUN_TCP_SERVER_PORT);

  // bind address to the socket
  r = bind(sockd_tcp_srv, (const struct sockaddr *) &srv_addr_tcp, sizeof(struct sockaddr_in6));
  assert_res(r, "TCP server bind()");

  // listen on socket
  r = listen(sockd_tcp_srv, 0);
  assert_res(r, "TCP server listen()");

  // waiting for connection request
  SL_WISUN_THREAD_LOOP {
    if (print_prompt) {
      printf("[Waiting for connection request]\n");
      print_prompt = false;
    }

    sockd_tcp_clnt = accept(sockd_tcp_srv, (struct sockaddr *)&clnt_addr_tcp, &len);
    if (sockd_tcp_clnt != SOCKET_INVALID_ID) {
      client_connected = true;
      printf("TCP server accept() [%ld] done.\n", sockd_tcp_clnt);
    }

    SL_WISUN_THREAD_LOOP {
      if (!client_connected) {
        break;
      }

      // receiver loop
      r = recv(sockd_tcp_clnt, buff, SL_WISUN_TCP_SERVER_BUFF_SIZE - 1, 0);
      switch (r) {
        case -1:
          sl_wisun_app_core_util_dispatch_thread();
          continue;

        case 0: // socket closed, EOF
          printf("[Socket closing: %ld]\n", sockd_tcp_clnt);
          close(sockd_tcp_clnt);
          client_connected = false;
          print_prompt = true;
          break;

        default: // default behaviour
          buff[r] = 0;
          // print the received message
          ip_str = app_wisun_trace_util_get_ip_str((void *) &clnt_addr_tcp.sin6_addr);
          printf("[%s] %s\n", ip_str, buff);
          app_wisun_trace_util_destroy_ip_str(ip_str);
          // send back to the client
          if (send(sockd_tcp_clnt, buff, r, 0) == -1) {
            printf("Error send()\n");
          }
          break;
      }
      // dispatch thread
      sl_wisun_app_core_util_dispatch_thread();
    }
  }
}
