/***************************************************************************/ /**
 * @file  sl_si91x_socket_callback_framework.h
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

#pragma once

#include "sl_status.h"
#include "sl_si91x_types.h"

typedef struct {
  void *user_context;
  void *socket_context;
  int32_t socket_id; //socket_id to update in command trace of bus thread.
} sli_si91x_socket_context_t;

/**
 * This is internal event handler to handle all events of sockets[Including BSD]
 * @param status 	  sl_status of rx_packet frame status
 * @param sdk_context sli_si91x_socket_context_t that was sent at the time sending the request.
 * @param rx_packet	  Firmware response.
 * @return
 */
sl_status_t sli_si91x_socket_event_handler(sl_status_t status,
                                           sli_si91x_socket_context_t *sdk_context,
                                           sl_wifi_system_packet_t *rx_packet);
