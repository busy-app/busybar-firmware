/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "MQTTSi91x.h"
#include "sl_constants.h"
#include <string.h>
#include <stdint.h>
#include "cmsis_os2.h"
#include "socket.h"
#include "sl_net.h"
#include "errno.h"
#include "sl_si91x_socket_types.h"
//For TLS
#include "sl_si91x_socket_constants.h"
#include "sl_si91x_constants.h"

//For WebSocket functionality
#include "sl_websocket_client_types.h"
#include "sl_websocket_client.h"
#include "sli_websocket_client_sync.h"

#define MQTT_WEBSOCKET_RESOURCE    "/mqtt"
#define MQTT_WEBSOCKET_SUBPROTOCOL "mqtt"

// WebSocket handle for MQTT connection
static sl_websocket_client_t ws_handle = { 0 };

char expired(Timer *timer)
{
  long left = timer->end_time - osKernelGetTickCount();
  return (left < 0);
}

void countdown_ms_mqtt(Timer *timer, unsigned int timeout)
{
  timer->end_time = osKernelGetTickCount() + timeout;
}

void countdown(Timer *timer, unsigned int timeout)
{
  timer->end_time = osKernelGetTickCount() + (timeout * 1000);
}

int left_ms_mqtt(Timer *timer)
{
  long left = timer->end_time - osKernelGetTickCount();
  return (left < 0) ? 0 : left;
}

void InitTimer(Timer *timer)
{
  timer->end_time = 0;
}

static int mqtt_tcp_read(Network *n, unsigned char *buffer, int len, int timeout_ms)
{
  struct timeval timeout;
  timeout.tv_sec  = timeout_ms / 1000;
  timeout.tv_usec = (timeout_ms % 1000) * 1000;

  // Set socket receive timeout
  setsockopt(n->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  int total_bytes_read = 0;
  while (total_bytes_read < len) {

    int rc = recv(n->socket, &buffer[total_bytes_read], (len - total_bytes_read), 0);
    if (rc == -1) {
      if (errno != ENOTCONN && errno != ECONNRESET)
        total_bytes_read = -1;
      break;
    } else if (rc == 0) {
      total_bytes_read = 0;
      break;
    } else
      total_bytes_read += rc;
  }
  return total_bytes_read;
}

static int mqtt_tcp_write(Network *n, unsigned char *buffer, int len, int timeout_ms)
{
  UNUSED_PARAMETER(timeout_ms);
  return send(n->socket, buffer, len, 0);
}

static void mqtt_tcp_disconnect(Network *n)
{
  if (n->socket >= 0) {
    close(n->socket);
  }
}

static int mqtt_tcpconnection_handler(Network *n, uint8_t flags, char *addr, int dst_port, int src_port, bool ssl)
{
  UNUSED_PARAMETER(flags);
  int type = SOCK_STREAM;
  int rc   = -1;
  int status;

#ifdef SLI_SI91X_ENABLE_IPV6
  struct sockaddr_in6 server_address_v6 = { 0 };
  struct sockaddr_in6 clientAddr_v6     = { 0 };
  socklen_t socket_length_v6            = sizeof(struct sockaddr_in6);
#else
  struct sockaddr_in server_address = { 0 };
  struct sockaddr_in client_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
#endif

#ifdef SLI_SI91X_ENABLE_IPV6
  server_address_v6.sin6_family = AF_INET6;
  server_address_v6.sin6_port   = dst_port;

  memcpy(&server_address_v6.sin6_addr.__u6_addr.__u6_addr8, addr, SL_IPV6_ADDRESS_LENGTH);

  if (ssl)
    n->socket = socket(AF_INET, type, IPPROTO_TCP);
  else
    n->socket = socket(AF_INET, type, IPPROTO_TCP);

  if (n->socket < 0) {
    printf("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return -1;
  }

  clientAddr_v6.sin6_family = AF_INET6;
  clientAddr_v6.sin6_port   = src_port;

#else
  //! Set family type
  server_address.sin_family = AF_INET;
  //! Set local port number
  server_address.sin_port = dst_port;

  memcpy(&server_address.sin_addr.s_addr, addr, sizeof(server_address.sin_addr.s_addr));

  if (ssl)
    n->socket = socket(AF_INET, type, IPPROTO_TCP);
  else
    n->socket = socket(AF_INET, type, IPPROTO_TCP);

  if (n->socket < 0) {
    printf("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return -1;
  }
  client_address.sin_family = AF_INET;
  client_address.sin_port   = src_port;
#endif

  //To enable SSL/TLS
  if (ssl) {
    int socket_return_value = 0;
    socket_return_value     = setsockopt(n->socket, SOL_SOCKET, TCP_ULP, TLS, sizeof(TLS));
    if (socket_return_value < 0) {
      printf("\r\nSet Socket option failed with bsd error: %d\r\n", errno);
      close(n->socket);
    }
  }

#ifdef SLI_SI91X_ENABLE_IPV6
  status = bind(n->socket, (struct sockaddr *)&clientAddr_v6, socket_length_v6);
#else
  //! Bind socket
  status = bind(n->socket, (struct sockaddr *)&client_address, socket_length);
#endif

  if (status != SL_STATUS_OK) {
    printf("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    mqtt_tcp_disconnect(n);
    return status;
  }

#ifdef SLI_SI91X_ENABLE_IPV6
  rc = connect(n->socket, (struct sockaddr *)&server_address_v6, socket_length_v6);
#else
  rc     = connect(n->socket, (struct sockaddr *)&server_address, socket_length);
#endif
  if (rc == -1) {
    printf("\r\nSocket Connect failed with bsd error: %d\r\n", errno);
    close(n->socket);
    return rc;
  }
  printf("\nSocket connection success \n");
  return status;
}

static int mqtt_ws_read(Network *n, unsigned char *buffer, int len, int timeout_ms)
{
  struct timeval timeout;

  timeout.tv_sec  = timeout_ms / 1000;
  timeout.tv_usec = (timeout_ms % 1000) * 1000;

  // Set socket receive timeout
  setsockopt(n->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  int bytes = recv(n->socket, buffer, len, 0);
  return (bytes < 0) ? -1 : bytes;
}

static int mqtt_ws_write(Network *n, unsigned char *buffer, int len, int timeout_ms)
{
  UNUSED_PARAMETER(n);
  UNUSED_PARAMETER(timeout_ms);

  sl_websocket_send_request_t send_request;
  send_request.opcode = SL_WEBSOCKET_OPCODE_BINARY | SL_WEBSOCKET_FIN_BIT;
  send_request.buffer = buffer;
  send_request.length = len;

  sl_websocket_error_t ws_error = sl_websocket_send_frame(&ws_handle, &send_request);
  if (ws_error != SL_WEBSOCKET_SUCCESS) {
    return ws_error;
  }

  return len;
}

static void mqtt_ws_disconnect(Network *n)
{
  UNUSED_PARAMETER(n);

  sl_websocket_close(&ws_handle);
  sl_websocket_deinit(&ws_handle);
}

static int mqtt_websocketconnection_handler(Network *n, uint8_t flags, char *addr, int dst_port, int src_port, bool ssl)
{
  UNUSED_PARAMETER(flags);
  sl_websocket_config_t ws_config = {
    .host                = addr,
    .resource            = MQTT_WEBSOCKET_RESOURCE,
    .server_port         = dst_port,
    .client_port         = src_port,
    .ip_address          = addr,
    .data_cb             = NULL,
    .remote_terminate_cb = NULL,
    .enable_ssl          = ssl,
  };

  // Initialize WebSocket
  sl_websocket_error_t ws_error = sl_websocket_init(&ws_handle, &ws_config);
  if (ws_error != SL_WEBSOCKET_SUCCESS) {
    return ws_error;
  }

  // Set MQTT subprotocol
  ws_error = sli_websocket_set_subprotocol(&ws_handle, MQTT_WEBSOCKET_SUBPROTOCOL);
  if (ws_error != SL_WEBSOCKET_SUCCESS) {
    sl_websocket_deinit(&ws_handle);
    return ws_error;
  }

  // Connect WebSocket
  ws_error = sli_websocket_connect_sync(&ws_handle);
  if (ws_error != SL_WEBSOCKET_SUCCESS) {
    sl_websocket_deinit(&ws_handle);
    return ws_error;
  }

  n->socket = ws_handle.socket_fd;
  return 0;
}

// Initialize the network structure
void NetworkInit(Network *n)
{
  if (n == NULL)
    return;

  n->socket = -1;

  if (n->transport_type == MQTT_TRANSPORT_TCP) {
    n->mqttread   = mqtt_tcp_read;
    n->mqttwrite  = mqtt_tcp_write;
    n->disconnect = mqtt_tcp_disconnect;
  } else if (n->transport_type == MQTT_TRANSPORT_WEBSOCKET) {
    n->mqttread   = mqtt_ws_read;
    n->mqttwrite  = mqtt_ws_write;
    n->disconnect = mqtt_ws_disconnect;
  }
}

// Connect to the MQTT broker using the specified transport type (TCP or WebSocket)
int NetworkConnect(Network *n, uint8_t flags, char *addr, int dst_port, int src_port, bool ssl)
{
  if (n == NULL) {
    return NETWORK_ERROR_NULL_STRUCTURE; // Error: NULL network structure
  }

  if (addr == NULL) {
    return NETWORK_ERROR_NULL_ADDRESS; // Error: NULL address
  }

  if (n->transport_type == MQTT_TRANSPORT_TCP) {
    return mqtt_tcpconnection_handler(n, flags, addr, dst_port, src_port, ssl);
  } else if (n->transport_type == MQTT_TRANSPORT_WEBSOCKET) {
    return mqtt_websocketconnection_handler(n, flags, addr, dst_port, src_port, ssl);
  } else {
    return NETWORK_ERROR_INVALID_TYPE; // Error: invalid transport type
  }
}

// Cleans up and closes the network connection.
void NetworkDisconnect(Network *n)
{
  if (n && n->disconnect) {
    n->disconnect(n);
  }
  return;
}
