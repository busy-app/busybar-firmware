/***************************************************************************/ /**
* @file
* @brief  WebSocket Client - Synchronous Socket
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
#ifndef SLI_WEBSOCKET_CLIENT_SYNC_H
#define SLI_WEBSOCKET_CLIENT_SYNC_H

#include <stdbool.h>
#include <stdint.h>
#include "cmsis_os2.h"
#include "sl_status.h"
#include "sl_wifi_types.h"
#include "sl_ip_types.h"
#include "sl_net_constants.h"
#include "sl_si91x_socket_types.h"
#include "sl_websocket_client_types.h"
#include "sl_websocket_client.h"
/******************************************************
 *               Function Declarations
 ******************************************************/

/***************************************************************************/ /**
 * @brief Connect to a WebSocket server synchronously.
 *
 * @details
 *   This function creates a socket, binds it, and connects to the specified WebSocket server.
 *   Ensures that the data_cb and remote_terminate_cb fields are explicitly set to NULL during initialization.
 * 
 * @pre
 *   The WebSocket handle should be initialized using @ref sl_websocket_init before calling this function.
 * 
 * @param[in] client
 *   Pointer to the WebSocket client structure. Must not be NULL.
 *
 * @return
 *   sl_websocket_error_t - Error code indicating the result of the operation.
 */
sl_websocket_error_t sli_websocket_connect_sync(sl_websocket_client_t *client);

/***************************************************************************/ /**
 * @brief Sets the subprotocol for the WebSocket client.
 *
 * This function allows the user to specify a subprotocol for the WebSocket
 * client. Subprotocols are used to define a specific protocol to be used
 * over the WebSocket connection, enabling the server and client to agree
 * on a common protocol for communication.
 *
 * @param[in] client Pointer to the WebSocket client instance.
 * @param[in] subprotocol Pointer to a null-terminated string specifying
 *                        the desired subprotocol. The string must remain
 *                        valid for the duration of the WebSocket connection.
 *                        For example, use "mqtt" for MQTT over WebSocket.
 *
 * @return SL_STATUS_OK if the subprotocol was successfully set.
 *         An appropriate error code otherwise.
 */
sl_status_t sli_websocket_set_subprotocol(sl_websocket_client_t *client, const char *subprotocol);

/***************************************************************************/

#endif // SLI_WEBSOCKET_CLIENT_SYNC_H