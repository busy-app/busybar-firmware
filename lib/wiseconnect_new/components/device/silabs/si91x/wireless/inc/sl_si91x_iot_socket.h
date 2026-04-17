/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#pragma once

#include "sl_status.h"

/******************************************************
 *                     Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
/******************************************************
 *                   Enumerations
 ******************************************************/
/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/
sl_status_t si91x_iot_socket_init(void);
int32_t si91x_iot_socket_create(int32_t af, int32_t type, int32_t protocol);
int32_t si91x_iot_socket_bind(int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port);
int32_t si91x_iot_socket_listen(int32_t socket, int32_t backlog);
int32_t si91x_iot_socket_accept(int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port);
int32_t si91x_iot_socket_connect(int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port);
int32_t si91x_iot_socket_recv(int32_t socket, void *buf, uint32_t len);
int32_t si91x_iot_socket_recv_from(int32_t socket,
                                   void *buf,
                                   uint32_t len,
                                   uint8_t *ip,
                                   uint32_t *ip_len,
                                   uint16_t *port);
int32_t si91x_iot_socket_send(int32_t socket, const void *buf, uint32_t len);
int32_t si91x_iot_socket_send_to(int32_t socket,
                                 const void *buf,
                                 uint32_t len,
                                 const uint8_t *ip,
                                 uint32_t ip_len,
                                 uint16_t port);
int32_t si91x_iot_socket_get_sock_name(int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port);
int32_t si91x_iot_socket_get_peer_name(int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port);
int32_t si91x_iot_socket_get_opt(int32_t socket, int32_t opt_id, void *opt_val, uint32_t *opt_len);
int32_t si91x_iot_socket_set_opt(int32_t socket, int32_t opt_id, const void *opt_val, uint32_t opt_len);
int32_t si91x_iot_socket_close(int32_t socket);
int32_t si91x_iot_socket_get_host_by_name(const char *name, int32_t af, uint8_t *ip, uint32_t *ip_len);
