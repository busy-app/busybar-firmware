/***************************************************************************/ /**
 * @file
 * @brief sl_si91x_http_client_callback_framework.h
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

#include "sl_http_client.h"
#include "sl_si91x_core_utilities.h"
#include "sl_si91x_constants.h"
#include "sl_si91x_driver.h"
#include "sl_si91x_protocol_types.h"

/******************************************************
 *               Function Declarations
 ******************************************************/
sl_status_t sli_http_client_register_callback(sl_http_client_event_t event,
                                              sl_http_client_t client_handle,
                                              sl_http_client_event_handler_t function);

sl_status_t sli_http_client_default_event_handler(sl_http_client_event_t event,
                                                  sl_wifi_buffer_t *buffer,
                                                  void *sdk_context);
