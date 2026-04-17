/*
 * Copyright (c) 2019 Arm Limited. All rights reserved.
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

#include "iot_socket.h"
#include "Driver_WiFi.h"

// WiFi driver number
#ifndef DRIVER_WIFI_NUM
#define DRIVER_WIFI_NUM         0
#endif

extern ARM_DRIVER_WIFI ARM_Driver_WiFi_(DRIVER_WIFI_NUM);
#define ptrWiFi      (&ARM_Driver_WiFi_(DRIVER_WIFI_NUM))

// Create a communication socket
int32_t iotSocketCreate (int32_t af, int32_t type, int32_t protocol) {
  return ptrWiFi->SocketCreate(af, type, protocol);
}

// Assign a local address to a socket
int32_t iotSocketBind (int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
  return ptrWiFi->SocketBind(socket, ip, ip_len, port);
}

// Listen for socket connections
int32_t iotSocketListen (int32_t socket, int32_t backlog) {
  return ptrWiFi->SocketListen(socket, backlog);
}

// Accept a new connection on a socket
int32_t iotSocketAccept (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  return ptrWiFi->SocketAccept(socket, ip, ip_len, port);
}

// Connect a socket to a remote host
int32_t iotSocketConnect (int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
  return ptrWiFi->SocketConnect(socket, ip, ip_len, port);
}

// Receive data on a connected socket
int32_t iotSocketRecv (int32_t socket, void *buf, uint32_t len) {
  return ptrWiFi->SocketRecv(socket, buf, len);
}

// Receive data on a socket
int32_t iotSocketRecvFrom (int32_t socket, void *buf, uint32_t len, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  return ptrWiFi->SocketRecvFrom(socket, buf, len, ip, ip_len, port);
}

// Send data on a connected socket
int32_t iotSocketSend (int32_t socket, const void *buf, uint32_t len) {
  return ptrWiFi->SocketSend(socket, buf, len);
}

// Send data on a socket
int32_t iotSocketSendTo (int32_t socket, const void *buf, uint32_t len, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
  return ptrWiFi->SocketSendTo(socket, buf, len, ip, ip_len, port);
}

// Retrieve local IP address and port of a socket
int32_t iotSocketGetSockName (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  return ptrWiFi->SocketGetSockName(socket, ip, ip_len, port);
}

// Retrieve remote IP address and port of a socket
int32_t iotSocketGetPeerName (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  return ptrWiFi->SocketGetPeerName(socket, ip, ip_len, port);
}

// Get socket option
int32_t iotSocketGetOpt (int32_t socket, int32_t opt_id, void *opt_val, uint32_t *opt_len) {
  return ptrWiFi->SocketGetOpt(socket, opt_id, opt_val, opt_len);
}

// Set socket option
int32_t iotSocketSetOpt (int32_t socket, int32_t opt_id, const void *opt_val, uint32_t opt_len) {
  return ptrWiFi->SocketSetOpt(socket, opt_id, opt_val, opt_len);
}

// Close and release a socket
int32_t iotSocketClose (int32_t socket) {
  return ptrWiFi->SocketClose(socket);
}

// Retrieve host IP address from host name
int32_t iotSocketGetHostByName (const char *name, int32_t af, uint8_t *ip, uint32_t *ip_len) {
  return ptrWiFi->SocketGetHostByName (name, af, ip, ip_len);
}
