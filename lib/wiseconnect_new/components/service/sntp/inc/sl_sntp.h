/***************************************************************************/ /**
 * @file  sl_sntp.h
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

#include "sl_status.h"
#include "sl_common.h"
#include <stdint.h>
#include <stdbool.h>

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/**
 *  @addtogroup SERVICE_SNTP_CONSTANTS 
 *  @{
 */

/**
 * @addtogroup SERVICE_SNTP_METHODS SNTP Methods
 * @{
 */
/**
 * @def SL_SNTP_BROADCAST_MODE
 * @brief
 *   Enable SNTP for broadcast mode.
 * 
 * @details
 *   In the broadcast mode, the server periodically sends broadcast messages to the client, and the client receives the messages from the server.
 * 
 * @note
 *   To use broadcast mode, the client, and the SNTP server must be in the same subnet.
 */
#define SL_SNTP_BROADCAST_MODE 1

/**
 * @def SL_SNTP_UNICAST_MODE
 * @brief
 *   Enable SNTP for unicast mode.
 * 
 * @details
 *   In the unicast mode, the client initiates a request to the server. After receiving the request, the server constructs a response message based on the local time and sends the response message back to the client.
 */
#define SL_SNTP_UNICAST_MODE 2
/** @} */

/**
 * @addtogroup SERVICE_SNTP_FLAGS SNTP Flags
 * @{
 */
/**
 * @def SL_SNTP_ENABLE_IPV6
 * @brief
 *   Enables IPv6 for SNTP client.
 * 
 * @details
 *   This macro enables IPv6 support for the SNTP client. By default, IPv4 is enabled.
 * 
 * @note
 *   - 1: Enable IPv6 support.
 *   - 0: Disable IPv6 support.
 */
#define SL_SNTP_ENABLE_IPV6 1
/** @} */

/******************************************************
 *                   Enumerations
 ******************************************************/

/**
 * @enum sl_sntp_client_events_t
 * @brief
 *   Enumeration of SNTP client events.
 * 
 * @details
 *   The enumeration defines the various events that the SNTP client can trigger. These events are used to notify the application about the status, and results of SNTP operations.
 */
typedef enum {
  SL_SNTP_CLIENT_START = 1,       ///< Event indicates SNTP client start.
  SL_SNTP_CLIENT_GET_TIME,        ///< Event indicates the SNTP client retrieves the current time.
  SL_SNTP_CLIENT_GET_TIME_DATE,   ///< Event indicates the SNTP client retrieves the current date and time.
  SL_SNTP_CLIENT_GET_SERVER_INFO, ///< Event indicates the SNTP client retrieves server information.
  SL_SNTP_CLIENT_STOP             ///< Event indicates SNTP client stop.
} sl_sntp_client_events_t;

/** @} */

/******************************************************
 *                   Type Definitions
 ******************************************************/

/**
 * @addtogroup SERVICE_SNTP_TYPES
 * @{
 */

/**
 * @brief
 *   Structure to hold SNTP Server Information response.
 * 
 * @details
 *   This structure contains the information about the SNTP server, including the command type, IP version, server IP address, and SNTP method.
 */
typedef struct {
  uint8_t command_type; ///< Command type indicating the type of SNTP command.
  uint8_t ip_version;   ///< IP version of the SNTP server. 4 for IPv4 and 6 for IPv6.
  union {
    uint8_t ipv4_address[4];               ///< IPv4 address of the SNTP server.
    unsigned long ipv6_address[4];         ///< IPv6 address of the SNTP server.
  } SL_ATTRIBUTE_PACKED server_ip_address; ///< Server IP address, either IPv4 or IPv6.
  uint8_t sntp_method;                     ///< SNTP server method used for synchronization. Supported methods:
                                           ///< - 1: Broadcast mode
                                           ///< - 2: Unicast mode
} SL_ATTRIBUTE_PACKED sl_sntp_server_info_t;

/**
 * @brief
 *   Structure to hold SNTP client response.
 * 
 * @details
 *   The structure contains the response information from the SNTP client, including the event type, status, response data, and the length of the response data.
 */
typedef struct {
  uint8_t event_type; ///< Type of event being received, indicated by @ref sl_sntp_client_events_t.
  sl_status_t
    status; ///< Status of the call which triggered this response. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
  uint8_t *
    data; ///< Pointer to response data. For SL_SNTP_CLIENT_GET_TIME and SL_SNTP_CLIENT_GET_TIME_DATE events, it is a (uint8_t *) buffer. For SL_SNTP_CLIENT_GET_SERVER_INFO event, it is a pointer to an sl_sntp_server_info_t structure.
  uint16_t data_length; ///< Length of the response data.
} sl_sntp_client_response_t;

/**
 * @typedef sl_sntp_client_event_handler_t
 * @brief
 *   Callback type for handling SNTP client responses.
 * 
 * @details
 *   This callback is invoked to handle responses from the SNTP client. It provides the response data, user data, and the length of the user data.
 * 
 * @param[in] response
 *   Pointer to the SNTP client response of type @ref sl_sntp_client_response_t.
 * 
 * @param[in] user_data
 *   Pointer to user data passed in the APIs @ref sl_sntp_client_get_time(), @ref sl_sntp_client_get_time_date(), and @ref sl_sntp_client_get_server_info().
 * 
 * @param[in] user_data_length
 *   Length of the user data passed in the APIs @ref sl_sntp_client_get_time(), @ref sl_sntp_client_get_time_date(), and @ref sl_sntp_client_get_server_info().
 */
typedef void (*sl_sntp_client_event_handler_t)(sl_sntp_client_response_t *response,
                                               uint8_t *user_data,
                                               uint16_t user_data_length);

/**
 * @typedef sl_sntp_set_time_sync_notification_handler_t
 * @brief
 *   Callback type for handling SNTP time synchronization notifications.
 * 
 * @details
 *   This callback is invoked to handle notifications related to SNTP time synchronization. It provides the command type, status of the operation, and a data buffer.
 * 
 * @param[in] cmd_type
 *   Command type indicating the type of SNTP operation.
 * 
 * @param[in] status
 *   Status of the SNTP operation. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 * 
 * @param[in] buffer
 *   Pointer to the data buffer containing additional information related to the SNTP operation.
 * 
 * @note
 *   This feature is currently not supported in SiWx91x chipsets.
 */
typedef void (*sl_sntp_set_time_sync_notification_handler_t)(const uint8_t cmd_type,
                                                             sl_status_t status,
                                                             const uint8_t *buffer);

/**
 * @brief
 *   SNTP client configuration structure.
 * 
 * @details
 *   This structure holds the configuration parameters for the SNTP client, including server details, synchronization methods, timeouts, and various flags.
 */
typedef struct {
  uint8_t *server_host_name;                    ///< SNTP server host name or address.
  uint8_t sntp_method;                          ///< SNTP method. One of the defines from @ref SERVICE_SNTP_METHODS.
  uint16_t sntp_timeout;                        ///< SNTP operation timeout in milliseconds (ms).
  uint8_t flags;                                ///< SNTP flags. Combination of values from @ref SERVICE_SNTP_FLAGS.
                                                ///< @note Bits 1-7 are reserved. User must set them to 0.
  sl_sntp_client_event_handler_t event_handler; ///< SNTP response handler of type @ref sl_sntp_client_event_handler_t.
  sl_sntp_set_time_sync_notification_handler_t
    time_sync_notifiication_handler; ///< Time synchronization notification handler of type @ref sl_sntp_set_time_sync_notification_handler_t.
                                     ///< @note This feature is currently not supported in SiWx91x chipsets.
  bool smooth_sync;                ///< Set to true if smooth synchronization is required.
                                   ///< @note This feature is currently not supported in SiWx91x chipsets.
  bool server_from_dhcp;           ///< Set to true to request NTP server configuration from DHCP.
                                   ///< @note This feature is currently not supported in SiWx91x chipsets.
  bool renew_servers_after_new_ip; ///< Set to true to refresh the server list if NTP is provided by DHCP.
                                   ///< @note This feature is currently not supported in SiWx91x chipsets.
} sl_sntp_client_config_t;
/** @} */

/**
 * @addtogroup SERVICE_SNTP_FUNCTIONS
 * @{
 */

/**
 * @brief
 * Start the SNTP client.
 * 
 * @details
 *    This function initializes and starts the SNTP client with the provided configuration. It verifies the input parameters, sets up the client request, and sends the command to start the SNTP client. The function can operate in both blocking and non-blocking modes based on the timeout value. 
 * 
 * @param[in] config		
 *    A valid pointer to the client configuration structure of type @ref sl_sntp_client_config_t. This parameter must not be null.
 * 
 * @param[in] timeout
 *    The timeout for starting the SNTP client in milliseconds (ms). If the timeout is greater than 0, this function operates in blocking mode. Otherwise, results are returned via the @ref sl_sntp_client_event_handler_t callback.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_FAIL: Operation failed.
 *   - SL_STATUS_INVALID_PARAMETER: Invalid input parameter.
 * @note
 *   This API needs to be called before calling any other SNTP API.
 */
sl_status_t sl_sntp_client_start(sl_sntp_client_config_t *config, uint32_t timeout);

/**
 * @brief
 *   Get the current NTP epoch time in seconds.
 * 
 * @details
 *   The function retrieves the current NTP epoch time. It operates in synchronous and asynchronous mode based on the timeout value.
 *   - **Synchronous Mode:** If the timeout value is non-zero then, function waits for the response until the timeout occurs. It returns the value in the provided data buffer.
 *   - **Asynchronous Mode:** If the timeout value is zero then, function returns immediately, It returns the value via the @ref sl_sntp_client_event_handler_t callback.
 * 
 * @param[in] data
 *   A valid pointer to a data buffer which should be greater than or equal to 50 bytes. In synchronous mode, the time is returned in this buffer in string format. In asynchronous mode, this parameter is given to the user in the user_data parameter of the event handler.
 * 
 * @param[in] data_length
 *   The length of the data buffer. In asynchronous mode, this parameter is given to the user in the user_data_length parameter of the event handler.
 * 
 * @param[in] timeout
 *   The timeout for getting the time in milliseconds (ms). If the timeout is greater than 0, this function operates in blocking mode. Otherwise, results are returned via the @ref sl_sntp_client_event_handler_t callback.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_FAIL: Operation failed.
 *   - SL_STATUS_INVALID_PARAMETER: Invalid input parameter.
 * 
 * @note
 *   This API must be called after the SNTP client has started using @ref sl_sntp_client_start.
 */
sl_status_t sl_sntp_client_get_time(uint8_t *data, uint16_t data_length, uint32_t timeout);

/**
 * @brief
 *   Get time and date information from NTP.
 * 
 * @details
 *   The function retrieves the current NTP epoch time and date. It operates in synchronous, and asynchronous mode based on the timeout value.
 *   - **Synchronous Mode:** If the timeout value is non-zero then, function waits for the response until the timeout occurs. It returns the value in the provided data buffer.
 *   - **Asynchronous Mode:** If the timeout value is zero then, function returns immediately. It returns the value via the @ref sl_sntp_client_event_handler_t callback.
 * 
 * @param[in] data		      
 *    A valid pointer to a data buffer must have value greater than or equal to 50 bytes. In synchronous mode, the time and date parameters are return in string format in the buffer. In asynchronous mode, the parameter is specified by the user via a user_data parameter of the event handler.
 * 
 * @param[in] data_length 
 *    The parameter captures the length of the data buffer in synchronous mode. In asynchronous mode, the parameter is stated to the user via the user_data_length parameter of the event handler. 
 * 
 * @param[in] timeout
 *    The timeout for getting time and date in milliseconds (ms). If the timeout value is greater than 0, the function operates in blocking mode else, it returns the result via the @ref sl_sntp_client_event_handler_t callback.
 *
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_FAIL: Operation failed.
 *   - SL_STATUS_INVALID_PARAMETER: Invalid input parameter.
 * 
 * @note
 *   This API must be called after the SNTP client has started using @ref sl_sntp_client_start.
 */
sl_status_t sl_sntp_client_get_time_date(uint8_t *data, uint16_t data_length, uint32_t timeout);

/**
 * @brief
 *   Retrieve NTP server information.
 * 
 * @details
 *   The function retrieves information about the NTP server. It operates in both synchronous and asynchronous modes based on the timeout value.
 *       In synchronous mode, the server returns the information in the specific data buffer.
 *       In asynchronous mode, the result is return via the @ref sl_sntp_client_event_handler_t callback.
 * 
 * @param[out] data
 *   A pointer is a sl_sntp_server_info_t type buffer where the server information is stored. In asynchronous mode, the parameter is passed to the user in the user_data parameter of the event handler.
 *   In synchronous mode the server information is returned in this data buffer.  
 * @param[in] timeout
 *   The timeout for receiving server information in milliseconds (ms). If the timeout is greater than 0, the function operates in blocking mode else, it would return the result via the @ref sl_sntp_client_event_handler_t callback.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_FAIL: Operation failed.
 *   - SL_STATUS_INVALID_PARAMETER: Invalid input parameter.
 * 
 * @note
 *   This API must be called after the SNTP client has been started using @ref sl_sntp_client_start.
 *
 * @note
 *   This function uses a user-configurable timeout parameter that is not affected
 *   by the global timeout scaling factors (SL_WIFI_INTERNAL_COMMANDS_TIMEOUT_SF,
 *   SL_WIFI_MANAGEMENT_COMMANDS_TIMEOUT_SF, SL_WIFI_NETWORK_COMMANDS_TIMEOUT_SF)
 *   or the additional wait time configuration (SL_TX_ADDITIONAL_WAIT_TIME).
 */
sl_status_t sl_sntp_client_get_server_info(sl_sntp_server_info_t *data, uint32_t timeout);

/**
 * @brief
 *   Stop the SNTP client.
 * 
 * @details
 *   The function stops the SNTP client. It operates in both synchronous and asynchronous modes based on the timeout value. In synchronous mode, the function blocks until the client stops or the timeout occurs. In asynchronous mode, the results are returned via the @ref sl_sntp_client_event_handler_t callback.
 * 
 * @param[in] timeout
 *   The timeout to stop the SNTP client in milliseconds (ms). The function operates in blocking mode if the timeout value is greater than 0 else, results are returned via the @ref sl_sntp_client_event_handler_t callback.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_FAIL: Operation failed.
 *   - SL_STATUS_INVALID_PARAMETER: Invalid input parameter.
 * 
 * @note
 *   This API must be called after the SNTP client has started using @ref sl_sntp_client_start.
 */
sl_status_t sl_sntp_client_stop(uint32_t timeout);
/** @} */
