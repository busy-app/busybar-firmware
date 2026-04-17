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

#include "MQTTSi91x_fake_functions.h"

DEFINE_FFF_GLOBALS;

DEFINE_FAKE_VALUE_FUNC(uint32_t, osKernelGetTickCount);
DEFINE_FAKE_VOID_FUNC1(mqtt_fake_disconnect, Network *);

// Socket and BSD-like functions
DEFINE_FAKE_VALUE_FUNC5(int, setsockopt, int, int, int, const void *, socklen_t);
DEFINE_FAKE_VALUE_FUNC4(ssize_t, recv, int, void *, size_t, int);
DEFINE_FAKE_VALUE_FUNC4(int, send, int, const void *, size_t, int);
DEFINE_FAKE_VALUE_FUNC3(int, socket, int, int, int);
DEFINE_FAKE_VALUE_FUNC3(int, bind, int, const struct sockaddr *, socklen_t);
DEFINE_FAKE_VALUE_FUNC3(int, connect, int, const struct sockaddr *, socklen_t);

// WebSocket functions
DEFINE_FAKE_VALUE_FUNC4(int, sl_websocket_send_frame, int, const void *, size_t, int);
DEFINE_FAKE_VALUE_FUNC1(int, sl_websocket_close, int);
DEFINE_FAKE_VALUE_FUNC1(int, sl_websocket_deinit, int);
DEFINE_FAKE_VALUE_FUNC2(int, sl_websocket_init, int, void *);
DEFINE_FAKE_VALUE_FUNC2(int, sli_websocket_set_subprotocol, int, const char *);
DEFINE_FAKE_VALUE_FUNC2(int, sli_websocket_connect_sync, int, int);
