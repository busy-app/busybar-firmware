/* -----------------------------------------------------------------------------
 * Copyright (c) 2021 Arm Limited (or its affiliates). All rights reserved.
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
 * -------------------------------------------------------------------------- */

#ifndef IOT_SOCKET_MUX_H_
#define IOT_SOCKET_MUX_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/**
\brief Access structure of the IoT Socket API.
*/
typedef struct {
  int32_t (*SocketCreate)        (int32_t af, int32_t type, int32_t protocol);
  int32_t (*SocketBind)          (int32_t socket, const uint8_t *ip, uint32_t  ip_len, uint16_t  port);
  int32_t (*SocketListen)        (int32_t socket, int32_t backlog);
  int32_t (*SocketAccept)        (int32_t socket,       uint8_t *ip, uint32_t *ip_len, uint16_t *port);
  int32_t (*SocketConnect)       (int32_t socket, const uint8_t *ip, uint32_t  ip_len, uint16_t  port);
  int32_t (*SocketRecv)          (int32_t socket, void *buf, uint32_t len);
  int32_t (*SocketRecvFrom)      (int32_t socket, void *buf, uint32_t len, uint8_t *ip, uint32_t *ip_len, uint16_t *port);
  int32_t (*SocketSend)          (int32_t socket, const void *buf, uint32_t len);
  int32_t (*SocketSendTo)        (int32_t socket, const void *buf, uint32_t len, const uint8_t *ip, uint32_t ip_len, uint16_t port);
  int32_t (*SocketGetSockName)   (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port);
  int32_t (*SocketGetPeerName)   (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port);
  int32_t (*SocketGetOpt)        (int32_t socket, int32_t opt_id,       void *opt_val, uint32_t *opt_len);
  int32_t (*SocketSetOpt)        (int32_t socket, int32_t opt_id, const void *opt_val, uint32_t  opt_len);
  int32_t (*SocketClose)         (int32_t socket);
  int32_t (*SocketGetHostByName) (const char *name, int32_t af, uint8_t *ip, uint32_t *ip_len);
} iotSocketApi_t;

/**
  \brief         Register socket API.
  \param[in]     api      pointer to API access structure (NULL disables socket API)
  \return        status information:
                 - 0              = Operation successful.
                 - negative value = error.
 */
extern int32_t iotSocketRegisterApi (const iotSocketApi_t *api);

#ifdef  __cplusplus
}
#endif

#endif /* IOT_SOCKET_MUX_H_ */
