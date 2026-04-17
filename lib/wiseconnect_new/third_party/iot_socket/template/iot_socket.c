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

// Create a communication socket
int32_t iotSocketCreate (int32_t af, int32_t type, int32_t protocol) {

  // Add implementation
  // return socket_id;
  return IOT_SOCKET_ERROR;
}

// Assign a local address to a socket
int32_t iotSocketBind (int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port) {

  // Check parameters
  if (socket < 0) {
    return IOT_SOCKET_ESOCK;
  }
  if ((ip == NULL) || (port == 0)) {
    return IOT_SOCKET_EINVAL;
  }

  // Add implementation
  // return 0;
  return IOT_SOCKET_ERROR;
}

// Listen for socket connections
int32_t iotSocketListen (int32_t socket, int32_t backlog) {

  // Check parameters
  if (socket < 0) {
    return IOT_SOCKET_ESOCK;
  }

  // Add implementation
  // return 0;
  return IOT_SOCKET_ERROR;
}

// Accept a new connection on a socket
int32_t iotSocketAccept (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {

  // Check parameters
  if (socket < 0) {
    return IOT_SOCKET_ESOCK;
  }

  // Add implementation
  // return socket_id;
  return IOT_SOCKET_ERROR;
}

// Connect a socket to a remote host
int32_t iotSocketConnect (int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port) {

  // Check parameters
  if (socket < 0) {
    return IOT_SOCKET_ESOCK;
  }
  if ((ip == NULL) || (port == 0)) {
    return IOT_SOCKET_EINVAL;
  }

  // Add implementation
  // return 0;
  return IOT_SOCKET_ERROR;
}

// Receive data on a connected socket
int32_t iotSocketRecv (int32_t socket, void *buf, uint32_t len) {

  // Check parameters
  if (socket < 0) {
    return IOT_SOCKET_ESOCK;
  }

  if (len == 0U) {
    // Check if socket readable
    // return 0;
    return IOT_SOCKET_EAGAIN;
  }

  if (buf == NULL) {
    return IOT_SOCKET_EINVAL;
  }

  // Add implementation
  // return num_of_bytes_received;
  return IOT_SOCKET_ERROR;
}

// Receive data on a socket
int32_t iotSocketRecvFrom (int32_t socket, void *buf, uint32_t len, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {

  // Check parameters
  if (socket < 0) {
    return IOT_SOCKET_ESOCK;
  }

  if (len == 0U) {
    // Check if socket readable
    // return 0;
    return IOT_SOCKET_EAGAIN;
  }

  if (buf == NULL) {
    return IOT_SOCKET_EINVAL;
  }

  // Add implementation
  // return num_of_bytes_received;
  return IOT_SOCKET_ERROR;
}

// Send data on a connected socket
int32_t iotSocketSend (int32_t socket, const void *buf, uint32_t len) {

  // Check parameters
  if (socket < 0) {
    return IOT_SOCKET_ESOCK;
  }

  if (len == 0U) {
    // Check if socket writable
    // return 0;
    return IOT_SOCKET_ERROR;
  }

  if (buf == NULL) {
    return IOT_SOCKET_EINVAL;
  }

  // Add implementation
  // return num_of_bytes_sent;
  return IOT_SOCKET_ERROR;
}

// Send data on a socket
int32_t iotSocketSendTo (int32_t socket, const void *buf, uint32_t len, const uint8_t *ip, uint32_t ip_len, uint16_t port) {

  // Check parameters
  if (socket < 0) {
    return IOT_SOCKET_ESOCK;
  }

  if (len == 0U) {
    // Check if socket writable
    // return 0;
    return IOT_SOCKET_ERROR;
  }

  if (buf == NULL) {
    return IOT_SOCKET_EINVAL;
  }

  // Add implementation
  // return num_of_bytes_sent;
  return IOT_SOCKET_ERROR;
}

// Retrieve local IP address and port of a socket
int32_t iotSocketGetSockName (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {

  // Check parameters
  if (socket < 0) {
    return IOT_SOCKET_ESOCK;
  }

  // Add implementation
  // return 0;
  return IOT_SOCKET_ERROR;
}

// Retrieve remote IP address and port of a socket
int32_t iotSocketGetPeerName (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {

  // Check parameters
  if (socket < 0) {
    return IOT_SOCKET_ESOCK;
  }

  // Add implementation
  // return 0;
  return IOT_SOCKET_ERROR;
}

// Get socket option
int32_t iotSocketGetOpt (int32_t socket, int32_t opt_id, void *opt_val, uint32_t *opt_len) {

  // Check parameters
  if (socket < 0) {
    return IOT_SOCKET_ESOCK;
  }
  if ((opt_val == NULL) || (opt_len == NULL)) {
    return IOT_SOCKET_EINVAL;
  }

  // Add implementation
  // return 0;
  return IOT_SOCKET_ERROR;
}

// Set socket option
int32_t iotSocketSetOpt (int32_t socket, int32_t opt_id, const void *opt_val, uint32_t opt_len) {

  // Check parameters
  if (socket < 0) {
    return IOT_SOCKET_ESOCK;
  }
  if (opt_val == NULL) {
    return IOT_SOCKET_EINVAL;
  }

  // Add implementation
  // return 0;
  return IOT_SOCKET_ERROR;
}

// Close and release a socket
int32_t iotSocketClose (int32_t socket) {

  // Check parameters
  if (socket < 0) {
    return IOT_SOCKET_ESOCK;
  }

  // Add implementation
  // return 0;
  return IOT_SOCKET_ERROR;
}


// Retrieve host IP address from host name
int32_t iotSocketGetHostByName (const char *name, int32_t af, uint8_t *ip, uint32_t *ip_len) {

  // Check parameters
  if ((name == NULL) || (ip == NULL) || (ip_len == NULL)) {
    return IOT_SOCKET_EINVAL;
  }

  // Add implementation
  // return 0;
  return IOT_SOCKET_ERROR;
}
