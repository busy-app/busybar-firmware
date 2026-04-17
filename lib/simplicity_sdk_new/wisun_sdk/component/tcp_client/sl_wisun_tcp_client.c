/***************************************************************************//**
 * @file sl_wisun_tcp_client.c
 * @brief Wi-SUN TCP client
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
#include "socket/socket.h"
#include "sl_wisun_tcp_client.h"
#include "errno.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/* create tcp client */
int32_t sl_wisun_tcp_client_create(const char *ip_address, uint16_t port)
{
  int32_t sockid = SOCKET_INVALID_ID; // client socket id
  sockaddr_in6_t server_addr;

  if (ip_address == NULL) {
    printf("[Failed: IP address is NULL ptr]\n");
    return SOCKET_INVALID_ID;
  }

  // create client socket
  sockid = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);

  if (sockid == SOCKET_INVALID_ID) {
    printf("[Failed to create socket: %ld]\n", sockid);
    return SOCKET_INVALID_ID;
  } else {
    printf("[Socket created: %ld]\n", sockid);
  }

  // setting the server address
  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_port = htons(port);

  if (inet_pton(AF_INET6, ip_address,
                &server_addr.sin6_addr) != 1) {
    printf("[Invalid IP address: %s]\n", ip_address);
    sl_wisun_tcp_client_close(sockid);
    return SOCKET_INVALID_ID;
  }

  // connect to the server
  if (connect(sockid, (const struct sockaddr *)&server_addr,
              sizeof(server_addr)) == SOCKET_RETVAL_ERROR && errno != EINPROGRESS) {
    printf("[Failed to connect to the server: %s]\n", ip_address);
    sl_wisun_tcp_client_close(sockid);
    return SOCKET_INVALID_ID;
  }

  return sockid;
}

/* close tcp client socket */
void sl_wisun_tcp_client_close(const int32_t sockid)
{
  if (close(sockid) == SOCKET_RETVAL_ERROR) {
    printf("[Failed to close socket: %ld]\n", sockid);
  } else {
    printf("[Socket closed: %ld]\n", sockid);
  }
}

/* write to tcp client socket */
void sl_wisun_tcp_client_write(const int32_t sockid, const char *str)

{
  int32_t res;
  if (str == NULL) {
    printf("[Failed: Data to write is NULL ptr]\n");
    return;
  }
  res = send(sockid, str, sl_strlen(str), 0);
  if (res == SOCKET_RETVAL_ERROR) {
    printf("[Failed to send on socket: %ld]\n", sockid);
  }
}

/* read on tcp client socket */
void sl_wisun_tcp_client_read(const int32_t sockid, const uint16_t size)
{
  char c;
  int32_t res;
  for (uint16_t i = 0; i < size; ++i) {
    res = recv(sockid, &c, 1, 0);
    if (res == SOCKET_RETVAL_ERROR || !res) {
      break;
    }
    printf("%c", c);
  }
  printf("\n");
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
