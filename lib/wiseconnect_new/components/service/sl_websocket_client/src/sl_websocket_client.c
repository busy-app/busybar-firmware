/***************************************************************************/ /**
 * @file  sl_websocket_client.c
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

/******************************************************
 *               API Definitions
 ******************************************************/
sl_websocket_error_t sl_websocket_init(sl_websocket_client_t *handle, const sl_websocket_config_t *config)
{
  if (!handle || !config) {
    return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
  }

  // Clear the handle structure to ensure all fields are initialized to zero
  memset(handle, 0, sizeof(sl_websocket_client_t));

  // Check if the WebSocket client is in a valid state to initialize
  if (handle->state != SL_WEBSOCKET_STATE_DISCONNECTED) {
    SL_DEBUG_LOG("\r\nInvalid state for initializing a WebSocket client\r\n");
    return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
  }

  // Copy host
  if (config->host != NULL) {
    if (strlen(config->host) >= SL_SI91X_WEBSOCKET_MAX_HOST_LENGTH) {
      SL_DEBUG_LOG("\r\nHost name length exceeds maximum allowed length\r\n");
      return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
    }
    snprintf(handle->host, sizeof(handle->host), "%s", config->host);
  }

  // Copy resource
  if (config->resource != NULL) {
    if (strlen(config->resource) >= SL_SI91X_WEBSOCKET_MAX_RESOURCE_LENGTH) {
      SL_DEBUG_LOG("\r\nResource name length exceeds maximum allowed length\r\n");
      return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
    }
    snprintf(handle->resource, sizeof(handle->resource), "%s", config->resource);
  }

#ifdef SLI_SI91X_ENABLE_IPV6
  unsigned char hex_addr[SL_IPV6_ADDRESS_LENGTH] = { 0 };
  sl_status_t status                             = sl_inet_pton6(config->ip_address,
                                     config->ip_address + strlen(config->ip_address),
                                     hex_addr,
                                     (unsigned int *)handle->ip_address.ip.v6.value);
  if (status != 0x1) {
    printf("\r\nIPv6 conversion failed.\r\n");
    return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
  }
  handle->ip_address.type = SL_IPV6;
#else
  sl_status_t status = sl_net_inet_addr(config->ip_address, &(handle->ip_address.ip.v4.value));
  if (status != SL_STATUS_OK) {
    printf("Failed to convert IP address \r\n");
    return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
  }
  handle->ip_address.type           = SL_IPV4;
#endif

  handle->server_port         = config->server_port;
  handle->client_port         = config->client_port;
  handle->data_cb             = config->data_cb;
  handle->remote_terminate_cb = config->remote_terminate_cb;
  handle->state               = SL_WEBSOCKET_STATE_DISCONNECTED;
  handle->enable_ssl          = config->enable_ssl;
  handle->user_context        = NULL;
  return SL_WEBSOCKET_SUCCESS;
}

sl_websocket_error_t sl_websocket_connect(sl_websocket_client_t *handle)
{
  // Check for null handle
  if (!handle) {
    return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
  }

  // Check if the WebSocket client is in a valid state to connect
  if (handle->state != SL_WEBSOCKET_STATE_DISCONNECTED) {
    SL_DEBUG_LOG("\r\nInvalid state for connecting a WebSocket client\r\n");
    return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
  }

  SL_DEBUG_LOG("\r\nIn websocket connect");

  int client_socket       = -1;
  int socket_return_value = 0;

  handle->state = SL_WEBSOCKET_STATE_CONNECTING;

  sl_si91x_set_remote_termination_callback(handle->remote_terminate_cb);

#ifdef SLI_SI91X_ENABLE_IPV6
  struct sockaddr_in6 server_address = { 0 };
  struct sockaddr_in6 client_address = { 0 };
  socklen_t socket_length            = sizeof(struct sockaddr_in6);
  client_address.sin6_family         = AF_INET6;
  server_address.sin6_family         = AF_INET6;
  server_address.sin6_port           = handle->server_port;
  client_address.sin6_port           = handle->client_port;
#ifndef __ZEPHYR__
  memcpy(&server_address.sin6_addr.__u6_addr.__u6_addr32,
         &(handle->ip_address.ip.v6.value),
         sizeof(server_address.sin6_addr.__u6_addr.__u6_addr32));
#else
  memcpy(&server_address.sin6_addr.s6_addr32,
         &(handle->ip_address.ip.v6.value),
         sizeof(server_address.sin6_addr.s6_addr32));
#endif
  client_socket = sl_si91x_socket_async(AF_INET6, SOCK_STREAM, IPPROTO_TCP, handle->data_cb);
#else
  struct sockaddr_in server_address = { 0 };
  struct sockaddr_in client_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  client_address.sin_family         = AF_INET;
  server_address.sin_family         = AF_INET;
  server_address.sin_port           = handle->server_port;
  client_address.sin_port           = handle->client_port;
  memcpy(&server_address.sin_addr.s_addr, &(handle->ip_address.ip.v4.value), sizeof(server_address.sin_addr.s_addr));
  client_socket = sl_si91x_socket_async(AF_INET, SOCK_STREAM, IPPROTO_TCP, handle->data_cb);
#endif

  if (client_socket < 0) {
    SL_DEBUG_LOG("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    handle->state = SL_WEBSOCKET_STATE_DISCONNECTED;
    return SL_WEBSOCKET_ERR_SOCKET_CREATION;
  }

  SL_DEBUG_LOG("\r\nClient Socket ID : %d\r\n", client_socket);
  handle->socket_fd = client_socket;

  if (handle->enable_ssl) {
    socket_return_value = setsockopt(client_socket, SOL_TCP, TCP_ULP, TLS, sizeof(TLS));
    if (socket_return_value < 0) {
      SL_DEBUG_LOG("\r\nSet socket failed with bsd error: %d\r\n", errno);
      close(client_socket);
      return SL_WEBSOCKET_ERR_SSL_SETSOCKOPT;
    }
  }

  socket_return_value = sl_si91x_bind(client_socket, (struct sockaddr *)&client_address, socket_length);
  if (socket_return_value < 0) {
    SL_DEBUG_LOG("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    close(client_socket);
    handle->state = SL_WEBSOCKET_STATE_DISCONNECTED;
    return SL_WEBSOCKET_ERR_SOCKET_BIND;
  }

  // Retrieve the socket using the socket index
  sli_si91x_socket_t *si91x_socket = sli_get_si91x_socket(client_socket);
  if (!si91x_socket) {
    SL_DEBUG_LOG("\r\nFailed to retrieve si91x socket\r\n");
    close(client_socket);
    handle->state = SL_WEBSOCKET_STATE_DISCONNECTED;
    return SL_WEBSOCKET_ERR_SOCKET_CREATION;
  }
  si91x_socket->ssl_bitmap |= SLI_SI91X_WEBSOCKET_FEAT;

  // Copy the host name and resource name from handle to si91x_socket->websocket_info
  size_t host_length     = strlen(handle->host);
  size_t resource_length = strlen(handle->resource);

  // Allocate memory for websocket_info
  si91x_socket->websocket_info =
    (sli_si91x_websocket_info_t *)malloc(sizeof(sli_si91x_websocket_info_t) + host_length + resource_length);

  // Check if memory allocation was successful
  if (si91x_socket->websocket_info == NULL) {
    SL_DEBUG_LOG("\r\nMemory allocation for websocket_info failed\r\n");
    close(client_socket);
    handle->state = SL_WEBSOCKET_STATE_DISCONNECTED;
    return SL_WEBSOCKET_ERR_SOCKET_CREATION;
  }

  // Clear the websocket_info structure to ensure all fields are initialized to zero
  memset(si91x_socket->websocket_info, 0, sizeof(sli_si91x_websocket_info_t) + host_length + resource_length);

  // Set the lengths
  si91x_socket->websocket_info->host_length     = host_length;
  si91x_socket->websocket_info->resource_length = resource_length;

  // Copy the host and resource names to websocket_info
  memcpy(si91x_socket->websocket_info->websocket_data, handle->host, host_length);
  memcpy(si91x_socket->websocket_info->websocket_data + host_length, handle->resource, resource_length);

  socket_return_value = connect(client_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    SL_DEBUG_LOG("\r\nSocket Connect failed with bsd error: %d\r\n", errno);
    close(client_socket);
    handle->state = SL_WEBSOCKET_STATE_DISCONNECTED;
    return SL_WEBSOCKET_ERR_SOCKET_CONNECT;
  }
  SL_DEBUG_LOG("\r\nSocket connected to TCP server\r\n");

  handle->state = SL_WEBSOCKET_STATE_CONNECTED;
  return SL_WEBSOCKET_SUCCESS;
}

sl_websocket_error_t sl_websocket_send_frame(sl_websocket_client_t *handle,
                                             const sl_websocket_send_request_t *send_request)
{
  // Check for null handle
  if (!handle) {
    return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
  }

  // Check for null send_request
  if (!send_request) {
    return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
  }

  // Check if the WebSocket client is in a valid state to send a frame
  if (handle->state != SL_WEBSOCKET_STATE_CONNECTED) {
    SL_DEBUG_LOG("\r\nInvalid state for sending a WebSocket frame\r\n");
    return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
  }

  // Retrieve the socket using the socket index
  sli_si91x_socket_t *si91x_socket = sli_get_si91x_socket(handle->socket_fd);
  if (!si91x_socket) {
    SL_DEBUG_LOG("\r\nFailed to retrieve socket\r\n");
    return SL_WEBSOCKET_ERR_SOCKET_CREATION;
  }

  // Set the opcode for the WebSocket frame
  si91x_socket->opcode = send_request->opcode;

  // Send the WebSocket frame
  int sent_bytes = send(handle->socket_fd, send_request->buffer, send_request->length, 0);
  if (sent_bytes < 0) {
    if (errno == ENOBUFS) {
      return SL_WEBSOCKET_SUCCESS;
    }
    SL_DEBUG_LOG("\r\nFailed to send WebSocket frame with error: %d\r\n", errno);
    return SL_WEBSOCKET_ERR_SEND_FRAME;
  }

  SL_DEBUG_LOG("\r\nSent bytes: %d\r\n", sent_bytes);
  return SL_WEBSOCKET_SUCCESS;
}

sl_websocket_error_t sl_websocket_close(sl_websocket_client_t *handle)
{
  if (!handle) {
    return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
  }

  // Check if the WebSocket client is in a valid state to be closed
  if (handle->state != SL_WEBSOCKET_STATE_CONNECTED && handle->state != SL_WEBSOCKET_STATE_CLOSING) {
    SL_DEBUG_LOG("\r\nInvalid state for closing the WebSocket connection\r\n");
    return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
  }

  // Update state to closing
  handle->state = SL_WEBSOCKET_STATE_CLOSING;
  SL_DEBUG_LOG("\r\nAttempting to close socket with fd: %d\r\n", handle->socket_fd);
  int status = close(handle->socket_fd);
  if (status == 0) {
    SL_DEBUG_LOG("\r\nSocket closed in FW");
    handle->socket_fd = -1;                        // Invalidate the socket file descriptor
    handle->state     = SL_WEBSOCKET_STATE_CLOSED; // Update state to closed
    return SL_WEBSOCKET_SUCCESS;
  } else {
    SL_DEBUG_LOG("\r\nSocket closed in FW failed with error: %d\r\n", errno);
    return SL_WEBSOCKET_ERR_CLOSE_FRAME;
  }
}

sl_websocket_error_t sl_websocket_deinit(sl_websocket_client_t *handle)
{
  if (!handle) {
    return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
  }

  // Check if the socket is closed
  if (handle->state != SL_WEBSOCKET_STATE_CLOSED) {
    SL_DEBUG_LOG("\r\nSocket is not closed. Deinit can only be called if the socket is closed.\r\n");
    return SL_WEBSOCKET_ERR_INVALID_PARAMETER;
  }

  // Close the WebSocket connection if it's still open
  if (handle->socket_fd >= 0) {
    SL_DEBUG_LOG("\r\nDeinit: Closing socket with fd: %d\r\n", handle->socket_fd);
    close(handle->socket_fd);
    handle->socket_fd = -1;
  }

  // Free the allocated memory for websocket_info
  sli_si91x_socket_t *si91x_socket = sli_get_si91x_socket(handle->socket_fd);
  if (si91x_socket && si91x_socket->websocket_info) {
    free(si91x_socket->websocket_info);
    si91x_socket->websocket_info = NULL;
  }

  // Set the handle's state to disconnected
  handle->state = SL_WEBSOCKET_STATE_DISCONNECTED;

  SL_DEBUG_LOG("\r\nWebSocket deinit success\r\n");
  return SL_WEBSOCKET_SUCCESS;
}

sl_websocket_opcode_t sl_si91x_get_opcode_from_socket_id(uint16_t socket_id)
{
  // Extract the second byte of the socket_id
  uint8_t websocket_info = (socket_id >> 8) & 0xFF;

  // Extract the first 4 bits to get the opcode
  sl_websocket_opcode_t opcode = (sl_websocket_opcode_t)(websocket_info & 0x0F);

  return opcode;
}
