/***************************************************************************/ /**
* @file
* @brief  WebSocket Client - Synchronous Socket
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
#include "sl_websocket_client.h"
#include "sl_websocket_client_types.h"
#include "sli_websocket_client_sync.h"
#include "sl_constants.h"
#include "sl_slist.h"
#include "sl_net.h"
#include "sl_net_constants.h"
#include "sl_wifi_constants.h"
#include "sl_si91x_constants.h"
#include "sl_si91x_driver.h"
#include "sl_si91x_protocol_types.h"
#include "sl_si91x_socket_utility.h"
#include "sl_si91x_socket_constants.h"
#include "sl_si91x_socket.h"
#ifndef __ZEPHYR__
#include "socket.h"
#else
#include <sys/socket.h>
#endif
#include "sli_net_utility.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

sl_websocket_error_t sli_websocket_connect_sync(sl_websocket_client_t *client)
{
  // Validate the WebSocket client
  if (!client) {
    return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
  }

  // Ensure the WebSocket client is in the correct state
  if (client->state != SL_WEBSOCKET_STATE_DISCONNECTED) {
    SL_DEBUG_LOG("\r\nWebSocket client is not in a disconnected state\r\n");
    return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
  }

  SL_DEBUG_LOG("\r\nStarting synchronous WebSocket connection");

  int sock_fd     = -1;
  int sock_result = 0;

  // Set callbacks to NULL for synchronous operation
  client->data_cb             = NULL;
  client->remote_terminate_cb = NULL;

  client->state = SL_WEBSOCKET_STATE_CONNECTING;

#ifdef SLI_SI91X_ENABLE_IPV6
  struct sockaddr_in6 server_address = { 0 };
  struct sockaddr_in6 client_address = { 0 };
  socklen_t socket_length            = sizeof(struct sockaddr_in6);
  client_address.sin6_family         = AF_INET6;
  server_address.sin6_family         = AF_INET6;
  server_address.sin6_port           = client->server_port;
  client_address.sin6_port           = client->client_port;
#ifndef __ZEPHYR__
  memcpy(&server_address.sin6_addr.__u6_addr.__u6_addr32,
         &(client->ip_address.ip.v6.value),
         sizeof(server_address.sin6_addr.__u6_addr.__u6_addr32));
#else
  memcpy(&server_address.sin6_addr.s6_addr32,
         &(client->ip_address.ip.v6.value),
         sizeof(server_address.sin6_addr.s6_addr32));
#endif
  sock_fd = sl_si91x_socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
#else
  struct sockaddr_in server_address = { 0 };
  struct sockaddr_in client_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  client_address.sin_family         = AF_INET;
  server_address.sin_family         = AF_INET;
  server_address.sin_port           = client->server_port;
  client_address.sin_port           = client->client_port;
  memcpy(&server_address.sin_addr.s_addr, &(client->ip_address.ip.v4.value), sizeof(server_address.sin_addr.s_addr));
  sock_fd = sl_si91x_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif

  if (sock_fd < 0) {
    SL_DEBUG_LOG("\r\nFailed to create socket, error: %d\r\n", errno);
    client->state = SL_WEBSOCKET_STATE_DISCONNECTED;
    return SL_WEBSOCKET_ERR_SOCKET_CREATION;
  }

  SL_DEBUG_LOG("\r\nClient Socket created successfully, ID %d\r\n", sock_fd);
  client->socket_fd = sock_fd;

  if (client->enable_ssl) {
    sock_result = setsockopt(sock_fd, SOL_TCP, TCP_ULP, TLS, sizeof(TLS));
    if (sock_result < 0) {
      SL_DEBUG_LOG("\r\nFailed to set SSL options, error: %d\r\n", errno);
      close(sock_fd);
      return SL_WEBSOCKET_ERR_SSL_SETSOCKOPT;
    }
  }

  sock_result = sl_si91x_bind(sock_fd, (struct sockaddr *)&client_address, socket_length);
  if (sock_result < 0) {
    SL_DEBUG_LOG("\r\nSocket bind failed, error: %d\r\n", errno);
    close(sock_fd);
    client->state = SL_WEBSOCKET_STATE_DISCONNECTED;
    return SL_WEBSOCKET_ERR_SOCKET_BIND;
  }

  // Retrieve the socket using the socket index
  sli_si91x_socket_t *si91x_socket = sli_get_si91x_socket(sock_fd);
  if (!si91x_socket) {
    SL_DEBUG_LOG("\r\nUnable to retrieve socket information\r\n");
    close(sock_fd);
    client->state = SL_WEBSOCKET_STATE_DISCONNECTED;
    return SL_WEBSOCKET_ERR_SOCKET_CREATION;
  }
  si91x_socket->ssl_bitmap |= SLI_SI91X_WEBSOCKET_FEAT;

  // Copy the host name,resource name and subprotocol name from client to si91x_socket->websocket_info
  size_t host_length        = strlen(client->host);
  size_t resource_length    = strlen(client->resource);
  size_t subprotocol_length = strlen(client->subprotocol);

  // Allocate memory for websocket_info
  si91x_socket->websocket_info = (sli_si91x_websocket_info_t *)malloc(sizeof(sli_si91x_websocket_info_t) + host_length
                                                                      + resource_length + subprotocol_length);

  if (si91x_socket->websocket_info == NULL) {
    SL_DEBUG_LOG("\r\nMemory allocation for WebSocket info failed\r\n");
    close(sock_fd);
    client->state = SL_WEBSOCKET_STATE_DISCONNECTED;
    return SL_WEBSOCKET_ERR_SOCKET_CREATION;
  }

  // Set lengths and copy data
  si91x_socket->websocket_info->host_length        = host_length;
  si91x_socket->websocket_info->resource_length    = resource_length;
  si91x_socket->websocket_info->subprotocol_length = subprotocol_length;

  // Copy the host, resource name and subprotocol name to websocket_info
  memcpy(si91x_socket->websocket_info->websocket_data, client->host, host_length);
  memcpy(si91x_socket->websocket_info->websocket_data + host_length, client->resource, resource_length);
  memcpy(si91x_socket->websocket_info->websocket_data + host_length + resource_length,
         client->subprotocol,
         subprotocol_length);

  sock_result = connect(sock_fd, (struct sockaddr *)&server_address, socket_length);
  if (sock_result < 0) {
    SL_DEBUG_LOG("\r\nFailed to connect socket, error: %d\r\n", errno);
    close(sock_fd);
    client->state = SL_WEBSOCKET_STATE_DISCONNECTED;
    return SL_WEBSOCKET_ERR_SOCKET_CONNECT;
  }
  SL_DEBUG_LOG("\r\nSocket successfully connected to the server\r\n");

  client->state = SL_WEBSOCKET_STATE_CONNECTED;
  return SL_WEBSOCKET_SUCCESS;
}

// Sets the subprotocol for the WebSocket client.
sl_status_t sli_websocket_set_subprotocol(sl_websocket_client_t *client, const char *subprotocol)
{
  if (client == NULL || strlen(subprotocol) >= SLI_WEBS_MAX_SUBPROTOCOL_LENGTH) {
    return SL_STATUS_FAIL;
  }
  strncpy(client->subprotocol, subprotocol, SLI_WEBS_MAX_SUBPROTOCOL_LENGTH - 1);
  client->subprotocol[SLI_WEBS_MAX_SUBPROTOCOL_LENGTH - 1] = '\0'; // Ensure null termination
  return SL_STATUS_OK;
}
