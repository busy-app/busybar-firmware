/*
 * Copyright (c) 2021 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stddef.h>
#include "iot_socket.h"
#include "iot_socket_mux.h"

// Registered socket API
static const iotSocketApi_t *SocketApi = NULL;

// Register socket API
int32_t iotSocketRegisterApi (const iotSocketApi_t *api) {
  SocketApi = api;
  return 0;
}

// ==================== IoT Socket Multiplexer ===================

// Create a communication socket
int32_t iotSocketCreate (int32_t af, int32_t type, int32_t protocol) {
  int32_t rc;

  if (SocketApi != NULL) {
    rc = SocketApi->SocketCreate(af, type, protocol);
  } else {
    rc = IOT_SOCKET_ERROR;
  }
  return rc;
}

// Assign a local address to a socket
int32_t iotSocketBind (int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
  int32_t rc;

  if (SocketApi != NULL) {
    rc = SocketApi->SocketBind(socket, ip, ip_len, port);
  } else {
    rc = IOT_SOCKET_ERROR;
  }
  return rc;
}

// Listen for socket connections
int32_t iotSocketListen (int32_t socket, int32_t backlog) {
  int32_t rc;

  if (SocketApi != NULL) {
    rc = SocketApi->SocketListen(socket, backlog);
  } else {
    rc = IOT_SOCKET_ERROR;
  }
  return rc;
}

// Accept a new connection on a socket
int32_t iotSocketAccept (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  int32_t rc;

  if (SocketApi != NULL) {
    rc = SocketApi->SocketAccept(socket, ip, ip_len, port);
  } else {
    rc = IOT_SOCKET_ERROR;
  }
  return rc;
}

// Connect a socket to a remote host
int32_t iotSocketConnect (int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
  int32_t rc;

  if (SocketApi != NULL) {
    rc = SocketApi->SocketConnect(socket, ip, ip_len, port);
  } else {
    rc = IOT_SOCKET_ERROR;
  }
  return rc;
}

// Receive data on a connected socket
int32_t iotSocketRecv (int32_t socket, void *buf, uint32_t len) {
  int32_t rc;

  if (SocketApi != NULL) {
    rc = SocketApi->SocketRecv(socket, buf, len);
  } else {
    rc = IOT_SOCKET_ERROR;
  }
  return rc;
}

// Receive data on a socket
int32_t iotSocketRecvFrom (int32_t socket, void *buf, uint32_t len, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  int32_t rc;

  if (SocketApi != NULL) {
    rc = SocketApi->SocketRecvFrom(socket, buf, len, ip, ip_len, port);
  } else {
    rc = IOT_SOCKET_ERROR;
  }
  return rc;
}

// Send data on a connected socket
int32_t iotSocketSend (int32_t socket, const void *buf, uint32_t len) {
  int32_t rc;

  if (SocketApi != NULL) {
    rc = SocketApi->SocketSend(socket, buf, len);
  } else {
    rc = IOT_SOCKET_ERROR;
  }
  return rc;
}

// Send data on a socket
int32_t iotSocketSendTo (int32_t socket, const void *buf, uint32_t len, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
  int32_t rc;

  if (SocketApi != NULL) {
    rc = SocketApi->SocketSendTo(socket, buf, len, ip, ip_len, port);
  } else {
    rc = IOT_SOCKET_ERROR;
  }
  return rc;
}

// Retrieve local IP address and port of a socket
int32_t iotSocketGetSockName (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  int32_t rc;

  if (SocketApi != NULL) {
    rc = SocketApi->SocketGetSockName(socket, ip, ip_len, port);
  } else {
    rc = IOT_SOCKET_ERROR;
  }
  return rc;
}

// Retrieve remote IP address and port of a socket
int32_t iotSocketGetPeerName (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  int32_t rc;

  if (SocketApi != NULL) {
    rc = SocketApi->SocketGetPeerName(socket, ip, ip_len, port);
  } else {
    rc = IOT_SOCKET_ERROR;
  }
  return rc;
}

// Get socket option
int32_t iotSocketGetOpt (int32_t socket, int32_t opt_id, void *opt_val, uint32_t *opt_len) {
  int32_t rc;

  if (SocketApi != NULL) {
    rc = SocketApi->SocketGetOpt(socket, opt_id, opt_val, opt_len);
  } else {
    rc = IOT_SOCKET_ERROR;
  }
  return rc;
}

// Set socket option
int32_t iotSocketSetOpt (int32_t socket, int32_t opt_id, const void *opt_val, uint32_t opt_len) {
  int32_t rc;

  if (SocketApi != NULL) {
    rc = SocketApi->SocketSetOpt(socket, opt_id, opt_val, opt_len);
  } else {
    rc = IOT_SOCKET_ERROR;
  }
  return rc;
}

// Close and release a socket
int32_t iotSocketClose (int32_t socket) {
  int32_t rc;

  if (SocketApi != NULL) {
    rc = SocketApi->SocketClose(socket);
  } else {
    rc = IOT_SOCKET_ERROR;
  }
  return rc;
}

// Retrieve host IP address from host name
int32_t iotSocketGetHostByName (const char *name, int32_t af, uint8_t *ip, uint32_t *ip_len) {
  int32_t rc;

  if (SocketApi != NULL) {
    rc = SocketApi->SocketGetHostByName (name, af, ip, ip_len);
  } else {
    rc = IOT_SOCKET_ERROR;
  }
  return rc;
}
