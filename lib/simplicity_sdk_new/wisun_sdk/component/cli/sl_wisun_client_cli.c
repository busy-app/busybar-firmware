/***************************************************************************//**
 * @file sl_wisun_client_cli.c
 * @brief Wi-SUN Client CLI handler
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
#include <stdlib.h>

#include "sl_assert.h"
#include "sl_component_catalog.h"
#include "sl_cli.h"
#include "sl_wisun_cli_core.h"
#include "sl_mempool.h"
#include "socket/socket.h"

#if defined(SL_CATALOG_WISUN_TCP_CLIENT_PRESENT)
#include "sl_wisun_tcp_client.h"
#endif
#if defined(SL_CATALOG_WISUN_UDP_CLIENT_PRESENT)
#include "sl_wisun_udp_client.h"
#endif
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define SL_WISUN_CLIENT_CLI_MAX_RESOURCES   (10U)

/// Socket entry
typedef struct sl_wisun_client_cli_socket {
  /// Socket ID
  int32_t socket_id;
  /// Socket type
  uint8_t socket_type;
} sl_wisun_client_cli_socket_t;
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// CLI sockets storage mempool
static sl_mempool_t _cli_sockets = { 0U };

/// Internal CLI socket buffer
static sl_wisun_client_cli_socket_t _cli_socket_table[SL_WISUN_CLIENT_CLI_MAX_RESOURCES] = { 0U };

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/* CLI app tcp/udp client cli init */
void sl_wisun_client_cli_init(void)
{
  // create mempool for sockets
  sl_status_t stat = sl_mempool_create(&_cli_sockets,
                                       SL_WISUN_CLIENT_CLI_MAX_RESOURCES,
                                       sizeof(sl_wisun_client_cli_socket_t),
                                       _cli_socket_table,
                                       sizeof(_cli_socket_table));
  EFM_ASSERT(stat == SL_STATUS_OK);
}

/* CLI app tcp/udp client create handler */
#if defined(SL_CATALOG_WISUN_TCP_CLIENT_PRESENT)
void app_tcp_client(const sl_cli_command_arg_t *arguments)
{
  const char *remote_ip = NULL;
  uint16_t remote_port = 0;

  sl_wisun_client_cli_socket_t *tmp_socket = NULL;
  int32_t sockid = SOCKET_INVALID_ID;

  remote_ip = sl_cli_get_argument_string(arguments, 0);
  remote_port = sl_cli_get_argument_uint16(arguments, 1);
  sockid = sl_wisun_tcp_client_create(remote_ip, remote_port);
  if (sockid == SOCKET_INVALID_ID) {
    return;
  }

  tmp_socket = sl_mempool_alloc(&_cli_sockets);
  if (tmp_socket == NULL) {
    printf("[Failed to allocate socket, socket closed: %ld]\n", sockid);
    sl_wisun_tcp_client_close(sockid);
    return;
  }

  tmp_socket->socket_id = sockid;
  tmp_socket->socket_type = SOCK_STREAM;
}
#endif
#if defined(SL_CATALOG_WISUN_UDP_CLIENT_PRESENT)
void app_udp_client(const sl_cli_command_arg_t *arguments)
{
  (void) arguments;

  sl_wisun_client_cli_socket_t *tmp_socket = NULL;
  int32_t sockid = SOCKET_INVALID_ID;

  sockid = sl_wisun_udp_client_create();
  if (sockid == SOCKET_INVALID_ID) {
    return;
  }

  tmp_socket = sl_mempool_alloc(&_cli_sockets);
  if (tmp_socket == NULL) {
    printf("[Failed to allocate socket, socket closed: %ld]\n", sockid);
    sl_wisun_udp_client_close(sockid);
    return;
  }

  tmp_socket->socket_id = sockid;
  tmp_socket->socket_type = SOCK_DGRAM;
}
#endif

/* CLI app tcp/udp client close handler */
void app_socket_close(const sl_cli_command_arg_t *arguments)
{
  int32_t sockid = 0;
  sl_mempool_block_hnd_t *block = NULL;
  const sl_wisun_client_cli_socket_t *client_cli_socket = NULL;

  sockid = sl_cli_get_argument_int32(arguments, 0);

  block = _cli_sockets.blocks;
  while (block != NULL) {
    client_cli_socket = (sl_wisun_client_cli_socket_t *) block->start_addr;
    if (client_cli_socket->socket_id == sockid) {
      if (client_cli_socket->socket_type == SOCK_STREAM) {
        #if defined(SL_CATALOG_WISUN_TCP_CLIENT_PRESENT)
        sl_wisun_tcp_client_close(sockid);
        #endif
      } else {
        #if defined(SL_CATALOG_WISUN_UDP_CLIENT_PRESENT)
        sl_wisun_udp_client_close(sockid);
        #endif
      }
      sl_mempool_free(&_cli_sockets, client_cli_socket);
    }
    block = block->next;
  }
}

/* CLI app tcp/udp client write handler */
void app_socket_write(const sl_cli_command_arg_t *arguments)
{
  int32_t sockid = 0;
  const char *str = NULL;
#if defined(SL_CATALOG_WISUN_UDP_CLIENT_PRESENT)
  const char *remote_ip = NULL;
  uint16_t remote_port = 0;
#endif

#if defined(SL_CATALOG_WISUN_TCP_CLIENT_PRESENT)
  if (sl_cli_get_argument_count(arguments) == 2) {
    sockid = atoi(sl_cli_get_argument_string(arguments, 0));
    str = sl_cli_get_argument_string(arguments, 1);
    sl_wisun_tcp_client_write(sockid, str);
  }
#endif
#if defined(SL_CATALOG_WISUN_UDP_CLIENT_PRESENT)
  if (sl_cli_get_argument_count(arguments) == 4) {
    sockid = atoi(sl_cli_get_argument_string(arguments, 0));
    remote_ip = sl_cli_get_argument_string(arguments, 1);
    remote_port = (uint16_t)atoi(sl_cli_get_argument_string(arguments, 2));
    str = sl_cli_get_argument_string(arguments, 3);
    sl_wisun_udp_client_write(sockid, remote_ip, remote_port, str);
  }
#endif
}

/* CLI app tcp/udp client read handler */
void app_socket_read(const sl_cli_command_arg_t *arguments)
{
#if defined(SL_CATALOG_WISUN_TCP_CLIENT_PRESENT)
  uint16_t size = 0;
#endif
  int32_t sockid = 0;
  sl_mempool_block_hnd_t *block = NULL;
  const sl_wisun_client_cli_socket_t *client_cli_socket = NULL;

#if defined(SL_CATALOG_WISUN_TCP_CLIENT_PRESENT)
  if (sl_cli_get_argument_count(arguments) == 2) {
    sockid = atoi(sl_cli_get_argument_string(arguments, 0));
    size = (uint16_t)atoi(sl_cli_get_argument_string(arguments, 1));
  }
#endif
#if defined(SL_CATALOG_WISUN_UDP_CLIENT_PRESENT)
  if (sl_cli_get_argument_count(arguments) == 1) {
    sockid = atoi(sl_cli_get_argument_string(arguments, 0));
  }
#endif

  block = _cli_sockets.blocks;
  while (block != NULL) {
    client_cli_socket = (sl_wisun_client_cli_socket_t *) block->start_addr;
    if (client_cli_socket->socket_id == sockid) {
      if (client_cli_socket->socket_type == SOCK_STREAM) {
        #if defined(SL_CATALOG_WISUN_TCP_CLIENT_PRESENT)
        sl_wisun_tcp_client_read(sockid, size);
        #endif
      } else {
        #if defined(SL_CATALOG_WISUN_UDP_CLIENT_PRESENT)
        sl_wisun_udp_client_read(sockid);
        #endif
      }
    }
    block = block->next;
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
