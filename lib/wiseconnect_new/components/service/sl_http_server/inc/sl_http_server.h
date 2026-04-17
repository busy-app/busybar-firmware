/***************************************************************************/ /**
 * @file  sl_http_server.h
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
#ifndef SL_HTTP_SERVER_H
#define SL_HTTP_SERVER_H

#include <stdbool.h>
#include <stdint.h>
#include "cmsis_os2.h"
#include "sl_status.h"
#include "sl_wifi_types.h"
#include "sl_ip_types.h"
#include "sl_net_constants.h"
#include "sl_si91x_socket_types.h"
#include "sl_http_server_types.h"

/******************************************************
 *               Function Declarations
 ******************************************************/

/**
 * @addtogroup SERVICE_HTTP_SERVER_FUNCTIONS
 * @{
 */

/***************************************************************************/ /**
 * @brief HTTP server initialization function.
 *
 * @details
 *   This function initializes an HTTP server using the provided configuration structure and returns a handle to the server. 
 *   It sets up the server's configuration, request buffer, and event flags. All HTTP server APIs must use the server handle returned in the `handle` parameter.
 *
 * @pre Pre-conditions:
 * - [sl_net_up](../wiseconnect-api-reference-guide-nwk-mgmt/net-interface-functions#sl-net-up) API needs to be called before this API.
 *
 * @param[out] handle
 *   Pointer to an @ref sl_http_server_t object that will be initialized with the server handle. Must not be NULL.
 *
 * @param[in] config
 *   Pointer to the HTTP server configuration structure of type @ref sl_http_server_config_t. Must not be NULL.
 *
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: Invalid input parameter.
 */
sl_status_t sl_http_server_init(sl_http_server_t *handle, const sl_http_server_config_t *config);

/***************************************************************************/ /**
 * @brief
 *   Deinitializes the HTTP server and frees all associated resources.
 * 
 * @details
 *   This function deinitializes the HTTP server, releasing all resources associated with the provided HTTP server handle. 
 *   It closes the server socket, deletes event flags, and resets the request handlers. 
 *   After calling this function, the `handle` parameter should not be used in any other API without reinitializing it using @ref sl_http_server_init.
 * 
 * @pre
 *   The HTTP server handle should be initialized using @ref sl_http_server_init before calling this function.
 * 
 * @param[in] handle
 *   Pointer to an @ref sl_http_server_t object representing the HTTP server handle. Must not be NULL.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: Invalid input parameter.
 * 
 * @note
 *   After calling this function, the `handle` parameter should not be used in any other API without calling @ref sl_http_server_init again.
 */
sl_status_t sl_http_server_deinit(sl_http_server_t *handle);

/***************************************************************************/ /**
 * @brief
 *   Starts the HTTP server to accept incoming requests.
 * 
 * @details
 *   This function spawns a new thread dedicated to accept the incoming HTTP requests on the configured HTTP port. 
 *   The HTTP request handlers are invoked from this thread to process the requests. It ensures that the server is ready to handle client connections.
 * 
 * @pre
 *   The HTTP server handle should be initialized using @ref sl_http_server_init before calling this function.
 * 
 * @param[in] handle
 *   Pointer to an @ref sl_http_server_t object representing the HTTP server handle. Must not be NULL.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: The provided handle is NULL.
 *   - SL_STATUS_FAIL: Failed to create the server thread.
 */
sl_status_t sl_http_server_start(sl_http_server_t *handle);

/***************************************************************************/ /**
 * @brief
 *   Stops the HTTP server and waits for all ongoing requests to complete.
 * 
 * @details
 *   This function sends a stop command to the HTTP server thread, and waits for all ongoing requests to complete. 
 *   It ensures that the server thread is properly terminated, and all resources associated with the server thread are released.
 * 
 * @pre
 *   The HTTP server handle should be initialized using @ref sl_http_server_init before calling this function.
 * 
 * @param[in] handle
 *   Pointer to an @ref sl_http_server_t object representing the HTTP server handle. Must not be NULL.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_FAIL: Failed to stop the server thread.
 */
sl_status_t sl_http_server_stop(sl_http_server_t *handle);

/***************************************************************************/ /**
 * @brief
 *   Retrieves the headers from an HTTP request.
 * 
 * @details
 *   This function extracts all headers from a given HTTP request, and stores them in the provided headers array.
 *   It parses the HTTP request to identify and extract the headers, which are then stored in the array pointed to by the `headers` parameter.
 * 
 * @param[in] handle
 *   Pointer to an @ref sl_http_server_t object representing the HTTP server handle. Must not be NULL.
 * 
 * @param[in] request
 *   Pointer to an @ref sl_http_server_request_t object representing the HTTP request from which headers are to be retrieved. Must not be NULL.
 * 
 * @param[out] headers
 *   Pointer to an array of @ref sl_http_header_t where the retrieved headers would be stored. Must not be NULL.
 * 
 * @param[in] header_count
 *   The number of headers that the `headers` array can hold.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: One or more input parameters are NULL or invalid.
 *   - SL_STATUS_FAIL: Failed to retrieve headers from the HTTP request.
 */
sl_status_t sl_http_server_get_request_headers(sl_http_server_t *handle,
                                               sl_http_server_request_t *request,
                                               sl_http_header_t *headers,
                                               uint16_t header_count);

/***************************************************************************/ /**
 * @brief
 *   Reads the HTTP request data being received after the headers.
 * 
 * @details
 *   This function reads the data portion of an HTTP request that follows the headers. It is used to process the body of the request.
 * 
 * @pre
 *   - The `req_data_length` parameter in @ref sl_http_server_request_t must be greater than 0.
 *   - This function must be called from within the request handler of the corresponding request.
 * 
 * @param[in] handle
 *   Pointer to an @ref sl_http_server_t object representing the HTTP server handle. Must not be NULL.
 * 
 * @param[in] data
 *   Pointer to an @ref sl_http_recv_req_data_t structure where the received request data would be stored. Must not be NULL.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: One or more input parameters are NULL or invalid.
 *   - SL_STATUS_FAIL: Failed to read the request data.
 */
sl_status_t sl_http_server_read_request_data(sl_http_server_t *handle, sl_http_recv_req_data_t *data);

/***************************************************************************/ /**
 * @brief
 *   Sends the HTTP response for the current request.
 * 
 * @details
 *   This function makes the HTTP server send the response to the current request. It can only be called once per request.
 * 
 * @pre
 *   The HTTP server handle should be initialized using @ref sl_http_server_init before calling this function.
 * 
 * @param[in] handle
 *   Pointer to an @ref sl_http_server_t object representing the HTTP server handle. Must not be NULL.
 * 
 * @param[in] response
 *   Pointer to an @ref sl_http_server_response_t object representing the response to be sent. Must not be NULL.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: One or more input parameters are NULL or invalid.
 *   - SL_STATUS_FAIL: Failed to send the response.
 */
sl_status_t sl_http_server_send_response(sl_http_server_t *handle, sl_http_server_response_t *response);

/***************************************************************************/ /**
 * @brief
 *   Sends a chunk of data as part of the HTTP response.
 * 
 * @details
 *   This function sends a chunk of data as part of the HTTP response. It should be used after a response has been started with 
 *   @ref sl_http_server_send_response, and can be called multiple times to send the response data in chunks.
 * 
 * @pre
 *   The HTTP response must be initiated using @ref sl_http_server_send_response before calling this function.
 * 
 * @param[in] handle
 *   Pointer to an @ref sl_http_server_t object representing the HTTP server handle. Must not be NULL.
 * 
 * @param[in] data
 *   Pointer to the data to be sent. Must not be NULL.
 * 
 * @param[in] data_length
 *   Length of the data to be sent.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: One or more input parameters are NULL or invalid.
 *   - SL_STATUS_FAIL: Failed to send the data chunk.
 */
sl_status_t sl_http_server_write_data(sl_http_server_t *handle, uint8_t *data, uint32_t data_length);

/***************************************************************************/ /**
 * @brief 
 *     Binds the HTTP server to a specific network interface.
 *
 * @details
 *    This function sets the network interface for the HTTP server based on the 
 *    provided interface identifier. It defaults to the client interface and 
 *    adjusts the interface if the operation mode is concurrent.
 *
 * @note
 *    If this function not called, the HTTP server defaults to using the Access Point (AP) interface in concurrent mode. In standalone mode, it uses the respective mode as the default interface.
 *    It should be called before @ref sl_http_server_start to configure the desired interface.
 *
 * @note 
 *    If the operation mode is concurrent, the function checks the provided 
 *    interface and sets the appropriate network interface identifier.
 *
 * @param[in] interface 
 *    The network interface identifier. Possible values:
 *    - SL_NET_WIFI_CLIENT_INTERFACE: Client interface.
 *    - SL_NET_WIFI_AP_INTERFACE: Access Point interface.
 *
 * @return 
 *    sl_status_t Status of the operation:
 *    - SL_STATUS_OK: Operation successful.
 *    - SL_STATUS_INVALID_PARAMETER: Invalid interface provided.
 */
sl_status_t sl_http_server_bind_interface(sl_net_interface_t interface);
/** @} */

#endif //SL_HTTP_SERVER_H
