/***************************************************************************/ /**
 * @file  sl_http_client.h
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
#include "sl_wifi_types.h"
#include "sl_ip_types.h"
#include "sl_net_constants.h"
#include "cmsis_os2.h"
#include "sl_si91x_socket_types.h"
#include "sl_si91x_socket_utility.h"
#include "sl_slist.h"
#include <stdbool.h>

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/**
 *  @addtogroup SERVICE_HTTP_CLIENT_CONSTANTS 
 *  @{
 */

/**
 * @def SL_HTTP_CLIENT_MAX_WRITE_BUFFER_LENGTH
 * @brief
 *   Maximum buffer length for write data.
 * 
 * @details
 *   This macro defines the maximum length, in bytes, of the buffer used for writing data in HTTP client operations. It ensures that the buffer allocated for outgoing data is sufficiently large to handle typical HTTP requests.
 */
#define SL_HTTP_CLIENT_MAX_WRITE_BUFFER_LENGTH 900

/**
 * @def SL_HTTPS_CLIENT_DEFAULT_CERTIFICATE_INDEX
 * @brief
 *   Default certificate index for HTTPS client.
 * 
 * @details
 *   This macro defines the default index for the client certificate used in HTTPS connections. It is used to specify which certificate to use from the certificate store when establishing a secure connection.
 */
#define SL_HTTPS_CLIENT_DEFAULT_CERTIFICATE_INDEX 0

/**
 * @def SL_HTTPS_CLIENT_CERTIFICATE_INDEX_1
 * @brief
 *   Certificate index for an HTTPS client certificate.
 * 
 * @details
 *   This macro defines the index for an HTTPS client certificate used in secure connections. It specifies which certificate to use from the certificate store when establishing a secure connection.
 */
#define SL_HTTPS_CLIENT_CERTIFICATE_INDEX_1 1

/**
 * @def SL_HTTPS_CLIENT_CERTIFICATE_INDEX_2
 * @brief
 *   Certificate index for the an HTTPS client certificate.
 * 
 * @details
 *   This macro defines the index for an HTTPS client certificate used in secure connections. It specifies which certificate to use from the certificate store when establishing a secure connection.
 */
#define SL_HTTPS_CLIENT_CERTIFICATE_INDEX_2 2

/******************************************************
 *                   Enumerations
 ******************************************************/

/**
 * @brief
 *   Enumeration of HTTP client methods.
 * 
 * @details
 *   This enumeration defines the HTTP methods that can be used by the HTTP client to perform various operations on the server.
 * 
 * @note
 *   Current SDK does not support SL_HTTP_HEAD and SL_HTTP_DELETE methods.
 */
typedef enum {
  SL_HTTP_GET    = 0, ///< HTTP GET method, used to retrieve data from the server.
  SL_HTTP_POST   = 1, ///< HTTP POST method, used to send data to the server.
  SL_HTTP_HEAD   = 2, ///< HTTP HEAD method, used to retrieve headers from the server.
  SL_HTTP_PUT    = 3, ///< HTTP PUT method, used to update or create a resource on the server.
  SL_HTTP_DELETE = 4  ///< HTTP DELETE method, used to delete a resource from the server.
} sl_http_client_method_type_t;

/**
 * @brief
 *   Enumeration of HTTPS client TLS versions.
 * 
 * @details
 *   This enumeration defines the versions of the Transport Layer Security (TLS) protocol that can be used by the HTTPS client. Each version provides different levels of security and compatibility.
 */
typedef enum {
  SL_TLS_V_1_0           = 0, ///< Use TLS Version 1.0 for HTTPS Client.
  SL_TLS_V_1_1           = 1, ///< Use TLS Version 1.1 for HTTPS Client.
  SL_TLS_V_1_2           = 2, ///< Use TLS Version 1.2 for HTTPS Client.
  SL_TLS_V_1_3           = 3, ///< Use TLS Version 1.3 for HTTPS Client.
  SL_TLS_DEFAULT_VERSION = -1 ///< Use the default TLS version for HTTPS Client.
} sl_http_client_tls_version_t;

/**
 * @brief
 *   Enumeration of HTTP protocol versions.
 * 
 * @details
 *   This enumeration defines the versions of the HTTP protocol that can be used by the HTTP client. Each version corresponds to a specific set of features and behaviors defined by the HTTP standard.
 */
typedef enum {
  SL_HTTP_V_1_0 = 0, ///< Use HTTP Protocol version 1.0
  SL_HTTP_V_1_1 = 1  ///< Use HTTP Protocol version 1.1
} sl_http_client_version_t;

/**
 * @brief
 *   Enumeration of HTTP client events.
 * 
 * @details
 *   This enumeration defines the various events that can occur during HTTP client operations. Each event corresponds to a specific type of HTTP response received by the client.
 */
typedef enum {
  SL_HTTP_CLIENT_GET_RESPONSE_EVENT = 0, ///< Event for receiving a response to an HTTP GET request.
  SL_HTTP_CLIENT_POST_RESPONSE_EVENT,    ///< Event for receiving a response to an HTTP POST request.
  SL_HTTP_CLIENT_PUT_RESPONSE_EVENT,     ///< Event for receiving a response to an HTTP PUT request.
  SL_HTTP_CLIENT_MAX_EVENT               ///< Maximum number of HTTP client events.
} sl_http_client_event_t;

/** @} */

/******************************************************
 *                   Type Definitions
 ******************************************************/

/**
 * @addtogroup SERVICE_HTTP_CLIENT_TYPES
 * @{
 */

/**
 * @brief
 *   Handle for the HTTP client.
 * 
 * @details
 *   This type defines a handle used to identify and manage an HTTP client instance. It is used in various HTTP client operations to reference a specific client.
 */
typedef uint32_t sl_http_client_t;

/**
 * @typedef sl_http_client_event_handler_t
 * @brief
 *   Callback function type for handling HTTP client events.
 * 
 * @details
 *   This callback is invoked when an HTTP response is received. It provides the client handle, the event that occurred, the response data, and a user-defined context pointer.
 * 
 * @param[out] client
 *   HTTP client handle of type @ref sl_http_client_t.
 * @param[out] event
 *   HTTP event that has occurred, of type @ref sl_http_client_event_t.
 * @param[out] data
 *   Pointer to the HTTP response data.
 * @param[out] request_context
 *   User-defined context pointer.
 * 
 * @return
 *   Returns an sl_status_t value indicating the result of the callback execution. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 */
typedef sl_status_t (*sl_http_client_event_handler_t)(const sl_http_client_t *client,
                                                      sl_http_client_event_t event,
                                                      void *data,
                                                      void *request_context);

/******************************************************
 *                    Structures
 ******************************************************/
/**
 * @brief
 *   Structure to hold HTTP client credentials.
 * 
 * @details
 *   This structure is used to store the credentials required for HTTP client authentication. It includes the lengths of the username ,and password as well as, a flexible array to store the actual username, and password data.
 */
typedef struct {
  uint16_t username_length; ///< Length of the username. Maximum supported length is 139 bytes.
  uint16_t password_length; ///< Length of the password. Maximum supported length is 139 bytes.
  uint8_t data[];           ///< Flexible array to store both the username, and password.
} sl_http_client_credentials_t;

/**
 * @brief
 *   HTTP client configurations.
 * 
 * @details
 *   This structure holds the configuration settings for the HTTP client, that includes security options, protocol versions, and network interface settings.
 */
typedef struct {
  uint8_t certificate_index;                ///< Index of the HTTPS client certificate.
  sl_http_client_tls_version_t tls_version; ///< TLS version for the HTTP client. See @ref sl_http_client_tls_version_t.
  sl_http_client_version_t http_version;    ///< HTTP protocol version. See @ref sl_http_client_version_t.
  bool https_enable;                        ///< Enable or disable HTTPS.
  bool https_use_sni; ///< Enable or disable the use of Server Name Indication (SNI) extension in HTTPS.
  sl_ip_address_type_t
    ip_version; ///< IP version for the HTTP client. See [sl_ip_address_type_t](../wiseconnect-api-reference-guide-common/sl-ip-address-t).
  sl_net_interface_t
    network_interface; ///< Network interface for the HTTP client. See [sl_net_interface_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-net-constants#sl-net-interface-t).
} sl_http_client_configuration_t;

/**
 * @brief
 *   Structure representing an HTTP client extended header node.
 * 
 * @details
 *   This structure is used to represent a single HTTP header in a linked list of headers. Each node contains a key-value pair for the header, and an sl_slist node for list management. Access the next header via node.node.
 */
typedef struct sl_http_client_header_s {
  sl_slist_node_t node; ///< Linked list node for chaining headers. Access next header via node.node.
  char *key;            ///< Key name of the HTTP header.
  char *value;          ///< Value of the HTTP header.
} sl_http_client_header_t;

/**
 * @brief
 *   HTTP client request configurations.
 * 
 * @details
 *   This structure holds the configuration settings for an HTTP client request, that includes the HTTP method, server address, resource URL, headers, and other options.
 */
/**
 * @brief
 *   Configuration settings for an HTTP client request.
 * 
 * @details
 *   This structure contains all the necessary configuration parameters for making an HTTP client request, which includes the HTTP method, server address, resource URL, headers, and other options.
 */
typedef struct {
  sl_http_client_method_type_t http_method_type; ///< HTTP request method. See @ref sl_http_client_method_type_t.
  uint8_t *
    ip_address; ///< IP address should contain the IP address of the HTTP server as a string (values in ASCII), for example, "192.168.1.1" for IPv4 or "2001:0db8:85a3:0000:0000:8a2e:0370:7334" for IPv6.
  uint8_t *
    resource; ///< Full URL string for the requested resource, including the scheme (for example, http, https), domain, port, path, query parameters, and fragment. The maximum supported HTTP URL is 2048 bytes when the SL_SI91X_FEAT_LONG_HTTP_URL bit is enabled in the feature_bit_map. If the SL_SI91X_FEAT_LONG_HTTP_URL bit is disabled, then the maximum supported length for the HTTP URL is (872 - (length of username + length of password) - length of hostname - length of IP address) bytes, excludes the delimiters.
  uint16_t port; ///< Port number of the HTTP server.
  sl_si91x_socket_type_length_value_t *
    sni_extension; ///< SNI (Server Name Indication) extension to specify the hostname for servers hosting multiple domains on the same IP address of type [si91x_socket_type_length_value_t](../wiseconnect-api-reference-guide-sockets/si91x-socket-type-length-value-t).
  uint8_t
    *body; ///< HTTP body to be sent to the server. Setting this to NULL will process the request in chunked encoding.
  uint32_t
    body_length; ///< Length of the HTTP body data to be posted. In the case of a chunked request, body length should be equal to the total content length.
  sl_http_client_header_t *
    extended_header; ///< User-defined extended header. If NULL, the default extended header will be added internally. See @ref sl_http_client_header_t.
  uint16_t timeout_ms; ///< HTTP request timeout period in milliseconds. (Si91x chipsets do not support this feature).
  uint16_t
    retry_count; ///< Maximum number of retry attempts after a timeout. (Si91x chipsets do not support this feature).
  uint16_t
    retry_period_ms; ///< Retry period in milliseconds after the maximum retry count is reached. (Si91x chipsets do not support this feature).
  bool
    tcp_connection_reuse; ///< Flag to indicate whether to reuse the same TCP socket for the connection. (Si91x chipsets do not support this feature).
  void *context;          ///< User-defined context for the request.
  sl_http_client_event_handler_t
    event_handler; ///< Callback function for handling HTTP client events. See @ref sl_http_client_event_handler_t.
  uint8_t *
    host_name; ///< Hostname of the HTTP server as specified in the request header. If NULL, the ip_address is used instead of the hostname.
} sl_http_client_request_t;

/**
 * @brief
 *   HTTP client response structure.
 * 
 * @details
 *   This structure holds the response data received from an HTTP server, which includes the status, data buffer, data length, HTTP response code, and response headers.
 */
typedef struct {
  uint32_t status;      ///< Status of the HTTP request.
  uint8_t *data_buffer; ///< Pointer to the buffer containing the response data.
  uint16_t data_length; ///< Length of the received data in bytes.
  uint32_t end_of_data; ///< Indicates the completion status of a data chunk or communication phase.
    ///<
    ///< GET/POST (indications from server):
    ///<   0: More response data is expected.
    ///<   1: This is the final chunk of the response.
    ///<
    ///< PUT:
    ///<   Acknowledgment for transmitted data (notification from NWP):
    ///<     0: Further data transmission is expected.
    ///<     1: Data transmission is complete.
    ///<   Server response (data received from server after transmission):
    ///<     8: More response data is expected.
    ///<     9: This is the final chunk of the response.
  uint16_t
    http_response_code; ///< HTTP response code from the server. (Si91x chipsets do not support this feature for SL_HTTP_PUT).
  uint8_t version; ///< HTTP version used in the response. (Si91x chipsets do not support this feature).
  sl_http_client_header_t *
    response_headers; ///< Pointer to the HTTP response headers. See @ref sl_http_client_header_t. (Si91x chipsets do not support this feature).
} sl_http_client_response_t;

/** @} */

/******************************************************
 *               Function Declarations
 ******************************************************/

/**
 * @addtogroup SERVICE_HTTP_CLIENT_FUNCTIONS
 * @{
 */

/***************************************************************************/ /**
 * @brief
 *   Initializes an HTTP client.
 * 
 * @details
 *   It prepares the client to send HTTP requests. You should call this function before making any HTTP requests to ensure the client is properly configured.
 * 
 * @note
 *   - You can call this function multiple times to initialize multiple HTTP client resources.
 *   - You can use this function after calling `deinit()` to reinitialize the resource.
 * 
 * @param[in] configuration
 *   Pointer to an @ref sl_http_client_configuration_t structure containing the configuration settings for the HTTP client. Must not be NULL.
 * 
 * @param[out] client
 *   Pointer to an @ref sl_http_client_t object that would be initialized with the client handle. Must not be NULL.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: One or more input parameters are NULL or invalid.
 *   - SL_STATUS_FAIL: Failed to initialize the HTTP client.
 ******************************************************************************/
sl_status_t sl_http_client_init(const sl_http_client_configuration_t *configuration, sl_http_client_t *client);

/***************************************************************************/
/**
 * @brief
 *   Deinitializes an HTTP client and releases resources used by the HTTP client.
 * 
 * @details
 *   This function deinitializes the HTTP client, freeing any resources that were allocated during its initialization and usage.
 *   It also deletes any extended headers that may exist.
 * 
 * @pre
 *   The HTTP client must be initialized using @ref sl_http_client_init before calling this function.
 * 
 * @param[in] client
 *   Pointer to an @ref sl_http_client_t object representing the HTTP client handle. Must not be NULL.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: The provided client handle is NULL or invalid.
 *   - SL_STATUS_FAIL: Failed to deinitialize the HTTP client.
 * 
 * @note
 *   The user must call this function to release resources once the HTTP client is no longer needed.
 ******************************************************************************/
sl_status_t sl_http_client_deinit(const sl_http_client_t *client);

/***************************************************************************/
/**
 * @brief
 *   Initializes a callback function for the specified HTTP request.
 * 
 * @details
 *   This function sets up a callback function to handle events for a specific HTTP request. It must be called after initializing the HTTP client.
 * 
 * @pre
 *   - @ref sl_http_client_init should be called before this function.
 * 
 * @param[in] request
 *   Pointer to an @ref sl_http_client_request_t structure containing the HTTP client request configuration. Must not be NULL.
 * 
 * @param[in] event_handler
 *   Callback function of type @ref sl_http_client_event_handler_t to handle HTTP client events. Must not be NULL.
 * 
 * @param[in] request_context
 *   User-defined context pointer. The memory space for this context must remain valid until the response is received.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: One or more input parameters are NULL or invalid.
 *   - SL_STATUS_FAIL: Failed to initialize the HTTP client request.
 * 
 * @note
 *   SNI Limitation:
 *   - Only one SNI command can be active at a time, irrespective of protocol (MQTT or HTTP).
 *   - You must wait for the connection to be fully established before initiating another connection with SNI.
 *   - If multiple SNI commands are sent simultaneously, SL_STATUS_SI91X_FEATURE_NOT_AVAILABLE returns.
 *   - For reconnection scenarios, set SNI before calling reconnect.
 ******************************************************************************/
sl_status_t sl_http_client_request_init(sl_http_client_request_t *request,
                                        sl_http_client_event_handler_t event_handler,
                                        void *request_context);

/***************************************************************************/
/**
 * @brief
 *   Adds an extended header with a key and value to the client request.
 * 
 * @details
 *   This function adds an extended header to the specified HTTP client request. If the request does not already contain any extended headers, 
 *   this function allocates memory for them.
 * 
 * @pre
 *   - @ref sl_http_client_init should be called before this function.
 * 
 * @param[in] request
 *   Pointer to an @ref sl_http_client_request_t structure representing the HTTP client request configuration. Must not be NULL.
 * 
 * @param[in] key
 *   Pointer to a string containing the header key. Must not be NULL.
 * 
 * @param[in] value
 *   Pointer to a string containing the header value. Must not be NULL.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: One or more input parameters are NULL or invalid.
 *   - SL_STATUS_FAIL: Failed to add the header to the request.
 * 
 * @note
 *   If the request does not contain any extended headers, this function allocates memory for them.
 ******************************************************************************/
sl_status_t sl_http_client_add_header(sl_http_client_request_t *request, const char *key, const char *value);

/***************************************************************************/
/**
 * @brief
 *   Deletes a specified header field from the extended headers of an HTTP client request.
 * 
 * @details
 *   This function removes a header field from the extended headers of the specified HTTP client request based on the provided key.
 * 
 * @pre
 *   - @ref sl_http_client_add_header should be called before this function.
 * 
 * @param[in] request
 *   Pointer to an @ref sl_http_client_request_t structure represents the HTTP client request configuration. Must not be NULL.
 * 
 * @param[in] key
 *   Pointer to a string contains the header key to be deleted. Must not be NULL.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: One or more input parameters are NULL or invalid.
 *   - SL_STATUS_FAIL: Failed to delete the header from the request.
 ******************************************************************************/
sl_status_t sl_http_client_delete_header(sl_http_client_request_t *request, const char *key);

/***************************************************************************/
/**
 * @brief
 *   Deletes all headers from the extended headers of an HTTP client request.
 * 
 * @details
 *   This function removes all headers from the extended headers of the specified HTTP client request. It frees up the memory allocated for these headers.
 * 
 * @pre
 *   - @ref sl_http_client_add_header should be called before this function.
 * 
 * @param[in] request
 *   Pointer to an @ref sl_http_client_request_t structure representing the HTTP client request configuration. Must not be NULL.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: The provided request handle is NULL or invalid.
 *   - SL_STATUS_FAIL: Failed to delete the headers from the request.
 * 
 * @note
 *   The user must call this function to free the memory allocated for the headers.
 ******************************************************************************/
sl_status_t sl_http_client_delete_all_headers(sl_http_client_request_t *request);

/***************************************************************************/
/**
 * @brief
 *   Sends an HTTP request.
 * 
 * @details
 *   This function sends an HTTP request using the specified client and request configuration. It must be called after initializing the request with 
 *   @ref sl_http_client_request_init.
 * 
 * @pre
 *   - @ref sl_http_client_request_init should be called before this function.
 * 
 * @param[in] client
 *   Pointer to an @ref sl_http_client_t object representing the HTTP client handle. Must not be NULL.
 * 
 * @param[in] request
 *   Pointer to an @ref sl_http_client_request_t object representing the HTTP client request configuration. Must not be NULL.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: One or more input parameters are NULL or invalid.
 *   - SL_STATUS_FAIL: Failed to send the HTTP request.
 * 
 * @note
 *   - HTTP HEAD and DELETE methods are not supported on Si91x specific chipsets.
 *   - The `body_length` header in the request is set internally by default on Si91x specific chipsets.
 *   - HTTP PUT does not support sending the body through this API; it is mandatory to call @ref sl_http_client_write_chunked_data on Si91x specific chipsets.
 *   - HTTP response status and response codes (e.g., 200, 201, 404) would be returned in the corresponding event handler registered during @ref sl_http_client_request_init.
 *   - If the `sni_extension` field in the `sl_http_client_request_t` structure is NULL, the `host_name` field will be used as the SNI, provided that `host_name` is not equal to `ip_address`.
 * 
 * @note
 *   SNI Limitation:
 *   - Only one SNI command can be active at a time, irrespective of protocol (MQTT or HTTP).
 *   - You must wait for the connection to be fully established before initiating another connection with SNI.
 *   - If multiple SNI commands are sent simultaneously, the device returns SL_STATUS_SI91X_FEATURE_NOT_AVAILABLE.
 *   - For reconnection scenarios, set SNI before calling reconnect.
 ******************************************************************************/
sl_status_t sl_http_client_send_request(const sl_http_client_t *client, const sl_http_client_request_t *request);

/***************************************************************************/
/**
 * @brief
 *   Sends HTTP POST and PUT chunked data.
 * 
 * @details
 *   This function sends a chunk of data as part of an HTTP POST or PUT request. It should be used after initiating the request with 
 *   @ref sl_http_client_send_request. The data can be sent in multiple chunks by calling this function multiple times.
 * 
 * @pre
 *   - @ref sl_http_client_send_request should be called before this function.
 * 
 * @param[in] client
 *   Pointer to an @ref sl_http_client_t object representing the HTTP client handle. Must not be NULL.
 * 
 * @param[in] data
 *   Pointer to the buffer containing the data to be written. Must not be NULL.
 * 
 * @param[in] data_length
 *   Length of the data chunk to be sent.
 * 
 * @param[in] flush_now
 *   Boolean flag indicating whether to flush the data immediately. Note that this feature is not supported on Si91x specific chipsets.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: One or more input parameters are NULL or invalid.
 *   - SL_STATUS_FAIL: Failed to send the data chunk.
 * 
 * @note
 *   The `flush_now` feature is not supported on Si91x specific chipsets.
 ******************************************************************************/
sl_status_t sl_http_client_write_chunked_data(const sl_http_client_t *client,
                                              const uint8_t *data,
                                              uint32_t data_length,
                                              bool flush_now);
/** @} */
