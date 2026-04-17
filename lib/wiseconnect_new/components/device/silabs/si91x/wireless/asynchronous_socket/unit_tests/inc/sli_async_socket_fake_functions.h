/*******************************************************************************
 * @file
 * @brief
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
#pragma once
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "sl_si91x_socket.h"
#include "sl_si91x_socket_utility.h"
#include "fff.h"

DECLARE_FAKE_VALUE_FUNC1(sli_si91x_socket_t *, sli_get_si91x_socket, int32_t);
DECLARE_FAKE_VOID_FUNC1(sli_si91x_set_remote_socket_termination_callback,
                        sl_si91x_socket_remote_termination_callback_t);
DECLARE_FAKE_VOID_FUNC1(sli_si91x_host_free_buffer, sl_wifi_buffer_t *);
DECLARE_FAKE_VALUE_FUNC3(void *, sli_wifi_host_get_buffer_data, sl_wifi_buffer_t *, uint16_t, uint16_t *);
DECLARE_FAKE_VALUE_FUNC6(int,
                         sli_si91x_select,
                         int,
                         fd_set *,
                         fd_set *,
                         fd_set *,
                         const struct timeval *,
                         sl_si91x_socket_select_callback_t);
DECLARE_FAKE_VALUE_FUNC4(int, sli_si91x_socket, int, int, int, sl_si91x_socket_receive_data_callback_t);
DECLARE_FAKE_VALUE_FUNC4(int, sli_si91x_accept, int, struct sockaddr *, socklen_t *, sl_si91x_socket_accept_callback_t);
DECLARE_FAKE_VALUE_FUNC2(int, sli_si91x_shutdown, int, int);
DECLARE_FAKE_VALUE_FUNC3(int, sli_si91x_connect, int, const struct sockaddr *, socklen_t);
DECLARE_FAKE_VALUE_FUNC3(int, sli_si91x_bind, int, const struct sockaddr *, socklen_t);
DECLARE_FAKE_VALUE_FUNC3(sl_status_t, sli_create_and_send_socket_request, int, int, const int *);
DECLARE_FAKE_VALUE_FUNC6(sl_status_t,
                         sli_si91x_send_socket_command,
                         sli_si91x_socket_t *,
                         uint32_t,
                         const void *,
                         uint32_t,
                         uint32_t,
                         sl_wifi_buffer_t **);
DECLARE_FAKE_VALUE_FUNC3(sl_status_t,
                         sli_si91x_driver_send_socket_data,
                         const sli_si91x_socket_send_request_t *,
                         const void *,
                         uint32_t);
DECLARE_FAKE_VALUE_FUNC2(sl_status_t,
                         sli_si91x_add_tls_extension,
                         sli_si91x_tls_extensions_t *,
                         const sl_si91x_socket_type_length_value_t *);
