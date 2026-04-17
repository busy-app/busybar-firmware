/***************************************************************************/ /**
 * @file  sl_websocket_client.h
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
#ifndef SL_WEBSOCKET_CLIENT_H
#define SL_WEBSOCKET_CLIENT_H

#include <stdbool.h>
#include <stdint.h>
#include "cmsis_os2.h"
#include "sl_status.h"
#include "sl_wifi_types.h"
#include "sl_ip_types.h"
#include "sl_net_constants.h"
#include "sl_si91x_socket_types.h"
#include "sl_websocket_client_types.h"

/******************************************************
 *               Function Declarations
 ******************************************************/

/**
 * @addtogroup SERVICE_WEBSOCKET_CLIENT_FUNCTIONS
 * @{
 */

/***************************************************************************/ /**
 * @brief Initialize the WebSocket client.
 *
 * @details
 *   This function initializes the WebSocket client with the provided configuration.
 *   It allocates necessary resources and sets up the client for connection.
 *
 * @param[out] handle
 *   Pointer to the WebSocket client structure. Must not be NULL.
 *
 * @param[in] config
 *   Pointer to the WebSocket client configuration constant structure. Must not be NULL.
 *
 * @return
 *   sl_websocket_error_t - Error code indicating the result of the operation.
 */
sl_websocket_error_t sl_websocket_init(sl_websocket_client_t *handle, const sl_websocket_config_t *config);

/***************************************************************************/ /**
 * @brief Connect to a WebSocket server.
 *
 * @details
 *   This function creates a socket, binds it, and connects to the specified WebSocket server.
 * 
 * @pre
 *   The WebSocket handle should be initialized using @ref sl_websocket_init before calling this function.
 * 
 * @param[in] handle
 *   Pointer to the WebSocket client structure. Must not be NULL.
 *
 * @return
 *   sl_websocket_error_t - Error code indicating the result of the operation.
 */
sl_websocket_error_t sl_websocket_connect(sl_websocket_client_t *handle);

/***************************************************************************/ /**
 * @brief Send a WebSocket frame.
 *
 * @details
 *   This function sends a WebSocket frame to the server. Masking is taken care of by the firmware.
 *
 * @pre
 *   The WebSocket handle should be initialized using @ref sl_websocket_init and connected using @ref sl_websocket_connect before calling this function.
 * 
 * @param[in] handle
 *   Pointer to the WebSocket client structure. Must not be NULL.
 *
 * @param[in] send_request
 *   Pointer to the send request constant structure containing the frame details. Must not be NULL.
 *
 * @return
 *   sl_websocket_error_t - Error code indicating the result of the operation.
 *
 * @note Masking refers to applying a random 32-bit mask to the data sent to the server to ensure data integrity and security.
 *
 * @note The following table lists the maximum individual chunk of data that can be sent over each supported protocol.
 *       The length of the payload is specified in the `length` field of the `sl_websocket_send_request_t` structure.
 *  
 *  Protocol            | Maximum data chunk (bytes)
 *  --------------------|---------------------------
 *  WebSocket           | 1450 bytes
 *  WebSocket over SSL  | 1362 bytes
 */
sl_websocket_error_t sl_websocket_send_frame(sl_websocket_client_t *handle,
                                             const sl_websocket_send_request_t *send_request);

/***************************************************************************/ /**
 * @brief Close the WebSocket connection.
 *
 * @details
 *   This function closes the WebSocket connection and cleans up resources.
 *
 * @pre
 *   The WebSocket handle should be initialized using @ref sl_websocket_init and connected using @ref sl_websocket_connect before calling this function.
 * 
 * @param[in] handle
 *   Pointer to the WebSocket client structure. Must not be NULL.
 *
 * @return
 *   sl_websocket_error_t - Error code indicating the result of the operation.
 */
sl_websocket_error_t sl_websocket_close(sl_websocket_client_t *handle);

/***************************************************************************/ /**
 * @brief Deinitialize the WebSocket client.
 *
 * @details
 *   This function deinitializes the WebSocket client by freeing allocated resources and resetting the state.
 *   It should be called only after the WebSocket connection has been closed because it also attempts to close the socket as part of its cleanup process if the socket remains open after the @ref sl_websocket_close is invoked.
 *   Therefore, calling this function is mandatory whenever a WebSocket connection is terminated, ensuring proper cleanup.
 *
 * @pre
 *   The WebSocket handle should be initialized using @ref sl_websocket_init before calling this function.
 * 
 * @param[in] handle
 *   Pointer to the WebSocket client structure. Must not be NULL.
 *
 * @return
 *   sl_websocket_error_t - Error code indicating the result of the operation.
 */
sl_websocket_error_t sl_websocket_deinit(sl_websocket_client_t *handle);

/***************************************************************************/ /**
 * @brief Extracts the WebSocket opcode from a given socket ID.
 *
 * @details
 *   This function determines the opcode of the WebSocket from the socket ID.
 *   The socket_id is obtained from the recv_data_callback from the sl_si91x_socket_metadata_t parameter.
 *
 * @param[in] socket_id
 *   The 16-bit socket ID from which to extract the opcode.
 *
 * @return
 *   sl_websocket_opcode_t - The extracted WebSocket opcode.
 */
sl_websocket_opcode_t sl_si91x_get_opcode_from_socket_id(uint16_t socket_id);
/** @} */

#endif //SL_WEBSOCKET_CLIENT_H