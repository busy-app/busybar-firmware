/***************************************************************************/ /**
 * @file  sl_http_server_types.h
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
#ifndef SL_HTTP_SERVER_TYPES_H
#define SL_HTTP_SERVER_TYPES_H

#include "sl_status.h"
#include "sl_wifi_types.h"
#include "sl_ip_types.h"
#include "sl_net_constants.h"
#include "cmsis_os2.h"
#include "sl_si91x_socket_types.h"
#include <stdbool.h>
#include <stdint.h>

/******************************************************
 *                    Constants
 ******************************************************/

/**
 *  @addtogroup SERVICE_HTTP_SERVER_CONSTANTS 
 *  @{
 */

/**
 * @def SL_HTTP_CONTENT_TYPE_TEXT_CSS
 * @brief
 *   MIME type for CSS files.
 * 
 * @details
 *   This macro defines the MIME (Multipurpose Internet Mail Extensions) type for Cascading Style Sheets (CSS) files. It is used in HTTP headers to specify the content type of the response.
 */
#define SL_HTTP_CONTENT_TYPE_TEXT_CSS "text/css"

/**
 * @def SL_HTTP_CONTENT_TYPE_TEXT_CSV
 * @brief
 *   MIME type for CSV files.
 * 
 * @details
 *   This macro defines the MIME (Multipurpose Internet Mail Extensions) type for Comma-Separated Values (CSV) files. It is used in HTTP headers to specify the content type of the response.
 */
#define SL_HTTP_CONTENT_TYPE_TEXT_CSV "text/csv"

/**
 * @def SL_HTTP_CONTENT_TYPE_TEXT_HTML
 * @brief
 *   MIME type for HTML files.
 * 
 * @details
 *   This macro defines the MIME (Multipurpose Internet Mail Extensions) type for HyperText Markup Language (HTML) files. It is used in HTTP headers to specify the content type of the response.
 */
#define SL_HTTP_CONTENT_TYPE_TEXT_HTML "text/html"

/**
 * @def SL_HTTP_CONTENT_TYPE_TEXT_JAVASCRIPT
 * @brief
 *   MIME type for JavaScript files.
 * 
 * @details
 *   This macro defines the MIME (Multipurpose Internet Mail Extensions) type for JavaScript files. It is used in HTTP headers to specify the content type of the response.
 */
#define SL_HTTP_CONTENT_TYPE_TEXT_JAVASCRIPT "text/javascript"

/**
 * @def SL_HTTP_CONTENT_TYPE_TEXT_PLAIN
 * @brief
 *   MIME type for plain text files.
 * 
 * @details
 *   This macro defines the MIME (Multipurpose Internet Mail Extensions) type for plain text files. It is used in HTTP headers to specify the content type of the response.
 */
#define SL_HTTP_CONTENT_TYPE_TEXT_PLAIN "text/plain"

/**
 * @def SL_HTTP_CONTENT_TYPE_TEXT_XML
 * @brief
 *   MIME type for XML files.
 * 
 * @details
 *   This macro defines the MIME (Multipurpose Internet Mail Extensions) type for Extensible Markup Language (XML) files. It is used in HTTP headers to specify the content type of the response.
 */
#define SL_HTTP_CONTENT_TYPE_TEXT_XML "text/xml"

/**
 * @def SL_HTTP_CONTENT_TYPE_APPLICATION_PDF
 * @brief
 *   MIME type for PDF files.
 * 
 * @details
 *   This macro defines the MIME (Multipurpose Internet Mail Extensions) type for Portable Document Format (PDF) files. It is used in HTTP headers to specify the content type of the response.
 */
#define SL_HTTP_CONTENT_TYPE_APPLICATION_PDF "application/pdf"

/**
 * @def SL_HTTP_CONTENT_TYPE_APPLICATION_JSON
 * @brief
 *   MIME type for JSON files.
 * 
 * @details
 *   This macro defines the MIME (Multipurpose Internet Mail Extensions) type for JavaScript Object Notation (JSON) files. It is used in HTTP headers to specify the content type of the response.
 */
#define SL_HTTP_CONTENT_TYPE_APPLICATION_JSON "application/json"

/**
 * @def SL_HTTP_CONTENT_TYPE_APPLICATION_XML
 * @brief
 *   MIME type for XML files.
 * 
 * @details
 *   This macro defines the MIME (Multipurpose Internet Mail Extensions) type for Extensible Markup Language (XML) files. It is used in HTTP headers to specify the content type of the response.
 */
#define SL_HTTP_CONTENT_TYPE_APPLICATION_XML "application/xml"

/**
 * @def SL_HTTP_SERVER_MAX_QUERY_PARAMETERS
 * @brief
 *   Maximum number of query parameters in a URI.
 * 
 * @details
 *   This macro defines the maximum number of query parameters that can be included in a Uniform Resource Identifier (URI) for an HTTP request. It is used to limit the number of parameters that the server will process.
 */
#define SL_HTTP_SERVER_MAX_QUERY_PARAMETERS 4

/**
 * @def MAX_QUERY_PARAMETERS
 * @brief
 *   Maximum number of query parameters in a URI.
 * 
 * @details
 *   This macro defines the maximum number of query parameters that can be included in a Uniform Resource Identifier (URI) for an HTTP request. It is used to limit the number of parameters that the server will process.
 * 
 * @note
 *   This macro will be depreciated in future releases. The renamed macro is @ref SL_HTTP_SERVER_MAX_QUERY_PARAMETERS.
 */
#define MAX_QUERY_PARAMETERS SL_HTTP_SERVER_MAX_QUERY_PARAMETERS

/**
 * @def SL_HTTP_SERVER_MAX_HEADER_BUFFER_LENGTH
 * @brief
 *   Maximum length of the header buffer.
 * 
 * @details
 *   This macro defines the maximum length, in bytes, of the buffer used to store HTTP headers. It is used to ensure that the buffer allocated for headers is sufficiently large to handle typical HTTP requests and responses.
 */
#define SL_HTTP_SERVER_MAX_HEADER_BUFFER_LENGTH 1024

/**
 * @def MAX_HEADER_BUFFER_LENGTH
 * @brief
 *   Maximum length of the header buffer.
 * 
 * @details
 *   This macro defines the maximum length, in bytes, of the buffer used to store HTTP headers. It is used to ensure that the buffer allocated for headers is sufficiently large to handle typical HTTP requests and responses.
 * 
 * @note
 *   This macro depreciates in future releases. The renamed macro is @ref SL_HTTP_SERVER_MAX_HEADER_BUFFER_LENGTH.
 */
#define MAX_HEADER_BUFFER_LENGTH SL_HTTP_SERVER_MAX_HEADER_BUFFER_LENGTH

/******************************************************
 *                   Enumerations
 ******************************************************/

/**
 * @brief
 *   Enumeration of HTTP server response types.
 * 
 * @details
 *   This enumeration defines the standard HTTP/1.1 response status codes used by the HTTP server to indicate the result of a client's request.
 * 
 * @note
 *   These are the standard HTTP/1.1 response status codes.
 */
typedef enum {
  SL_HTTP_RESPONSE_OK                    = 200, ///< Success response code (200 OK)
  SL_HTTP_RESPONSE_BAD_REQUEST           = 400, ///< Bad request response code (400 Bad Request)
  SL_HTTP_RESPONSE_UNAUTHORIZED          = 401, ///< Unauthorized response code (401 Unauthorized)
  SL_HTTP_RESPONSE_FORBIDDEN             = 403, ///< Forbidden response code (403 Forbidden)
  SL_HTTP_RESPONSE_NOT_FOUND             = 404, ///< Not found response code (404 Not Found)
  SL_HTTP_RESPONSE_METHOD_NOT_ALLOWED    = 405, ///< Method not allowed response code (405 Method Not Allowed)
  SL_HTTP_RESPONSE_INTERNAL_SERVER_ERROR = 500, ///< Internal server error response code (500 Internal Server Error)
  SL_HTTP_RESPONSE_NOT_IMPLEMENTED       = 501, ///< Not implemented response code (501 Not Implemented)
  SL_HTTP_RESPONSE_SERVICE_UNAVAILABLE   = 503, ///< Service unavailable response code (503 Service Unavailable)
  SL_HTTP_RESPONSE_HTTP_VERSION_NOT_SUPPORTED =
    505, ///< HTTP version not supported response code (505 HTTP Version Not Supported)
} sl_http_response_code_t;

/**
 * @brief
 *   Enumeration of HTTP request versions.
 * 
 * @details
 *   This enumeration defines the supported versions of the HTTP protocol that can be used in HTTP requests.
 */
typedef enum {
  SL_HTTP_VERSION_1_0, ///< HTTP Version 1.0
  SL_HTTP_VERSION_1_1  ///< HTTP Version 1.1
} sl_http_request_version_t;

/**
 * @brief
 *   Enumeration of HTTP server request types.
 * 
 * @details
 *   This enumeration defines the types of HTTP requests that the server can handle. Each request type corresponds to a standard HTTP method.
 */
typedef enum {
  SL_HTTP_REQUEST_GET,    ///< GET request type, used to retrieve data from the server.
  SL_HTTP_REQUEST_POST,   ///< POST request type, used to submit data to the server.
  SL_HTTP_REQUEST_PUT,    ///< PUT request type, used to update or create a resource on the server.
  SL_HTTP_REQUEST_DELETE, ///< DELETE request type, used to delete a resource from the server.
  SL_HTTP_REQUEST_HEAD    ///< HEAD request type, used to retrieve the headers for a resource, without the body.
} sl_http_server_request_type_t;

/** @} */

/******************************************************
 *                   Type Definitions
 ******************************************************/

// Forward declaration of the type
typedef struct sl_http_server_s sl_http_server_t;
typedef struct sl_http_server_request_s sl_http_server_request_t;

/**
 * @addtogroup SERVICE_HTTP_SERVER_TYPES
 * @{
 */

/**
 * @typedef sl_http_request_handler_t
 * @brief
 *   Callback function type for handling HTTP requests.
 * 
 * @details
 *   This callback is invoked when an HTTP request is received by the server. The function should process the request and return an appropriate status.
 * 
 * @param[out] handle
 *   Pointer to the HTTP server handle of type @ref sl_http_server_t. This handle represents the current state and configuration of the server.
 * 
 * @param[out] request
 *   Pointer to the HTTP request of type @ref sl_http_server_request_t. This structure contains the details of the received request, including URI, headers, and data.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status for details..
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_PARAMETER: One or more input parameters are NULL or invalid.
 *   - SL_STATUS_FAIL: Failed to process the HTTP request.
 */
typedef sl_status_t (*sl_http_request_handler_t)(sl_http_server_t *handle, sl_http_server_request_t *request);

/******************************************************
 *                    Structures
 ******************************************************/

/**
 * @brief
 *   Structure representing an HTTP header.
 * 
 * @details
 *   This structure holds the key-value pair for an HTTP header. It is used to store and manage HTTP header parameters.
 */
typedef struct {
  char *key;   ///< Key of the header. Must be a null-terminated string.
  char *value; ///< Value of the header. Must be a null-terminated string.
} sl_http_header_t;

/**
 * @brief
 *   Structure representing HTTP server response parameters.
 * 
 * @details
 *   This structure holds the parameters for an HTTP server response, including the response code, content type, headers, and data buffer.
 */
typedef struct {
  sl_http_response_code_t response_code; ///< HTTP response code (for example, 200 for OK, 404 for Not Found)
  char *
    content_type; ///< Content type of the response (for example, "text/html", "application/json"). Must be a null-terminated string.
  sl_http_header_t *headers;     ///< Pointer to an array of headers. Each header is a key-value pair.
  uint16_t header_count;         ///< Number of headers in the headers array.
  uint8_t *data;                 ///< Pointer to the data buffer containing the response body.
  uint32_t current_data_length;  ///< Length of the data currently in the data buffer.
  uint32_t expected_data_length; ///< Total length of the data to be sent in the response.
} sl_http_server_response_t;

/**
 * @brief
 *   Structure representing HTTP server URI query parameters.
 * 
 * @details
 *   This structure holds the key-value pairs for URI query parameters in an HTTP server request. It is used to parse and manage query parameters.
 */
typedef struct {
  char *query; ///< Query parameter key. Must be a null-terminated string.
  char *value; ///< Query parameter value. Must be a null-terminated string.
} sl_http_server_uri_query_parameter_t;

/**
 * @brief
 *   Structure representing HTTP server request URI parameters.
 * 
 * @details
 *   This structure holds the URI path and query parameters for an HTTP server request. It is used to parse and manage the request URI and its associated parameters.
 */
typedef struct {
  char *path; ///< URI path. Must be a null-terminated string.
  sl_http_server_uri_query_parameter_t
    query_parameters[SL_HTTP_SERVER_MAX_QUERY_PARAMETERS]; ///< Array of query parameters.
  uint16_t query_parameter_count;                          ///< Number of query parameters received in the URI.
} sl_http_server_request_uri_t;

/**
 * @brief
 *   Structure representing HTTP server request parameters.
 * 
 * @details
 *   This structure holds the parameters for an HTTP server request, including the request URI, HTTP version, headers, request type, and data length.
 */
typedef struct sl_http_server_request_s {
  sl_http_server_request_uri_t uri;   ///< URI of the request, including path and query parameters.
  sl_http_request_version_t version;  ///< HTTP protocol version (for example, HTTP/1.1, HTTP/2).
  uint16_t request_header_count;      ///< Number of request headers received.
  sl_http_server_request_type_t type; ///< Type of the request (for example, GET, POST, PUT, DELETE).
  uint32_t request_data_length;       ///< Length of the request data in bytes.
} sl_http_server_request_t;

/**
 * @brief
 *   Structure representing HTTP server request handling parameters.
 * 
 * @details
 *   This structure holds the parameters for handling HTTP server requests, including the URI pattern to match and the corresponding request handler function.
 */
typedef struct {
  char *uri; ///< URI pattern to match the incoming request. Must be a null-terminated string.
  sl_http_request_handler_t
    handler; ///< Function pointer to the request handler. This function will be called when the URI pattern matches.
} sl_http_server_handler_t;

/**
 * @brief
 *   HTTP server configuration structure.
 * 
 * @details
 *   This structure holds the configuration parameters for the HTTP server, including the port number, list of request handlers, and the default handler.
 */
typedef struct {
  uint16_t port; ///< Port number on which the HTTP server will listen for incoming requests.
  sl_http_server_handler_t *
    handlers_list; ///< Pointer to an array of request handlers. Each handler is associated with a specific URI pattern.
  uint16_t handlers_count; ///< Number of request handlers in the handlers_list array.
  sl_http_request_handler_t
    default_handler; ///< Default request handler function to be called when no specific handler matches the request URI.
  uint16_t client_idle_time; ///< Idle duration in seconds before the client is considered inactive.
} sl_http_server_config_t;

/**
 * @brief
 *   HTTP server handle used to manage all HTTP server functions.
 * 
 * @details
 *   This structure holds the state and configuration of the HTTP server, including sockets, synchronization events, request and response data, and buffers.
 */
typedef struct sl_http_server_s {
  sl_http_server_config_t config;   ///< Configuration settings for the HTTP server.
  int server_socket;                ///< Socket descriptor for the server.
  int client_socket;                ///< Socket descriptor for the client currently being served.
  osEventFlagsId_t http_server_id;  ///< Event ID for the HTTP server, used for synchronization.
  sl_http_server_request_t request; ///< Current HTTP request being processed.
  char request_buffer[SL_HTTP_SERVER_MAX_HEADER_BUFFER_LENGTH]; ///< Buffer for storing the HTTP request.
  char *header;                                                 ///< Pointer to a string containing headers.
  uint8_t *req_data;                                            ///< Pointer to the data of the HTTP request.
  uint32_t data_length;                                         ///< Length of the request data.
  uint32_t rem_len;         ///< Remaining length of data to be processed in the request.
  bool response_sent;       ///< Flag indicating whether the response has been sent for the current request.
  uint32_t rem_resp_length; ///< Remaining length of data to be sent in the response.
} sl_http_server_t;

/**
 * @brief
 *   HTTP read data parameters.
 * 
 * @details
 *   This structure holds the parameters for reading data from an HTTP request, including the request details, buffer for storing the data, and the length of the data received.
 */
typedef struct {
  sl_http_server_request_t
    *request;             ///< Pointer to the request structure received in the request handler. Must not be NULL.
  uint8_t *buffer;        ///< Pointer to the user-defined memory buffer for storing received data. Must not be NULL.
  uint32_t buffer_length; ///< Length of the user-defined memory buffer.
  uint32_t received_data_length; ///< Amount of data received in the read operation.
} sl_http_recv_req_data_t;

/** @} */

#endif //SL_HTTP_SERVER_TYPES_H
