/***************************************************************************/ /**
 * @file  sl_http_client.c
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

#include "sl_http_client.h"
#include "sl_constants.h"
#include "sl_slist.h"
#include "sl_net.h"
#include "sl_net_constants.h"
#include "sl_wifi_constants.h"
#include "sl_si91x_constants.h"
#include "sl_si91x_driver.h"
#include "sl_si91x_protocol_types.h"
#include "sli_net_utility.h"
#include "sl_si91x_http_client_callback_framework.h"
#include "sl_rsi_utility.h"
#include <sl_string.h>

/******************************************************
 *                      Macros
 ******************************************************/

#define VERIFY_MALLOC_AND_FREE_IF_FAIL(ptr, ptr1, ptr2) \
  do {                                                  \
    if (ptr == NULL) {                                  \
      if (ptr1 != NULL)                                 \
        free(ptr1);                                     \
      if (ptr2 != NULL)                                 \
        free(ptr2);                                     \
      return SL_STATUS_ALLOCATION_FAILED;               \
    }                                                   \
  } while (0)

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

/******************************************************
 *                    Constants
 ******************************************************/
//! HTTP client state
typedef enum {
  HTTP_STATE_DEINITIALIZED = 0,        ///< HTTP client deinitialized state
  HTTP_STATE_INITIALIZED,              ///< HTTP client initialized state
  HTTP_STATE_GET_REQUEST_INITIALIZED,  ///< HTTP client state after GET request initialized
  HTTP_STATE_POST_REQUEST_INITIALIZED, ///< HTTP client state after POST request initialized
  HTTP_STATE_PUT_REQUEST_INITIALIZED,  ///< HTTP client state after PUT request initialized
  HTTP_STATE_REQUEST_SENT,             ///< HTTP client state after GET/POST/PUT request sent
  HTTP_STATE_CHUNKED_REQUEST_SENT      ///< HTTP client state after sending chunked request
} sl_http_client_state_t;

//! MAX supported length for Username and Password together
#define SI91X_MAX_SUPPORTED_HTTP_CREDENTIAL_LENGTH 278
#define TEMP_STR_SIZE                              7
/******************************************************
 *                    Structures
 ******************************************************/
//! HTTP client internal context
typedef struct {
  sl_http_client_credentials_t *client_credentials; ///< HTTP client credentials
  sl_http_client_configuration_t configuration;     ///< HTTP client configurations
  sl_http_client_request_t request;                 ///< HTTP client request configurations
  sl_http_client_state_t client_state;              ///< HTTP client state
} sl_http_client_internal_t;

/******************************************************
 *                 Global Variables
 ******************************************************/
static sl_http_client_internal_t http_client_handle = { 0 };

extern bool device_initialized;

/******************************************************
 *               Function Declarations
 ******************************************************/
// Validates HTTP clients request configurations
static sl_status_t sli_si91x_copy_ip_address_and_port(const sl_http_client_request_t *request);

// Copy extended headers into HTTP request buffer
static void sli_si91x_load_extended_headers_into_request_buffer(uint8_t *buffer,
                                                                sl_http_client_header_t *extended_header,
                                                                uint16_t *http_buffer_offset);

// Sends GET/POST request
static sl_status_t sli_si91x_send_http_client_request(sl_http_client_method_type_t send_request,
                                                      const sl_http_client_internal_t *client_internal,
                                                      const sl_http_client_request_t *request);

// Abort ongoing HTTP client operation
static sl_status_t sli_si91x_http_client_abort(void);

/******************************************************
 *               Function Definitions
 ******************************************************/
sl_status_t sl_http_client_init(const sl_http_client_configuration_t *client_configuration, sl_http_client_t *client)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  SL_WIFI_ARGS_CHECK_NULL_POINTER(client_configuration);
  SL_WIFI_ARGS_CHECK_NULL_POINTER(client);

  // Only one http client supported in current release
  if (http_client_handle.client_state != HTTP_STATE_DEINITIALIZED) {
    return SL_STATUS_ALREADY_EXISTS;
  }

  // Check https configurations
  if (client_configuration->certificate_index > SL_HTTPS_CLIENT_CERTIFICATE_INDEX_2) {
    return SL_STATUS_INVALID_CONFIGURATION;
  }

  // Check HTTP version
  if ((client_configuration->http_version != SL_HTTP_V_1_0) && (client_configuration->http_version != SL_HTTP_V_1_1)) {
    return SL_STATUS_INVALID_CONFIGURATION;
  }

  // Check SSL/TLS version
  switch (client_configuration->tls_version) {
    case SL_TLS_DEFAULT_VERSION:
    case SL_TLS_V_1_0:
    case SL_TLS_V_1_1:
    case SL_TLS_V_1_2: {
      break;
    }
    case SL_TLS_V_1_3: {
#ifdef SLI_SI917
      break;
#else
      return SL_STATUS_NOT_SUPPORTED;
#endif
    }
    default: {
      return SL_STATUS_INVALID_CONFIGURATION;
    }
  }

  // Check for network interface
  if ((client_configuration->network_interface != SL_NET_WIFI_CLIENT_INTERFACE)
      && (client_configuration->network_interface != SL_NET_WIFI_AP_INTERFACE)) {
    return SL_STATUS_INVALID_MODE;
  }

  // Store client configurations into internal configurations
  memcpy(&http_client_handle.configuration, client_configuration, sizeof(sl_http_client_configuration_t));

  sl_net_credential_type_t type;
  uint32_t max_credential_size = sizeof(sl_http_client_credentials_t) + SI91X_MAX_SUPPORTED_HTTP_CREDENTIAL_LENGTH;

  http_client_handle.client_credentials = (sl_http_client_credentials_t *)malloc(max_credential_size);
  SLI_VERIFY_MALLOC_AND_RETURN(http_client_handle.client_credentials);
  memset(http_client_handle.client_credentials, 0, max_credential_size);
  sl_status_t status = sl_net_get_credential(SL_NET_HTTP_CLIENT_CREDENTIAL_ID(0),
                                             &type,
                                             http_client_handle.client_credentials,
                                             &max_credential_size);

  if (status != SL_STATUS_OK || type != SL_NET_HTTP_CLIENT_CREDENTIAL) {
    return status != SL_STATUS_OK ? status : SL_STATUS_INVALID_CREDENTIALS;
  }

  // Set HTTP client state
  http_client_handle.client_state = HTTP_STATE_INITIALIZED;

  // Copy address of HTTP client internal handle
  *client = (sl_http_client_t)&http_client_handle;

  return SL_STATUS_OK;
}

sl_status_t sl_http_client_deinit(const sl_http_client_t *client)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(client);

  sl_status_t status = SL_STATUS_OK;

  if (http_client_handle.client_state == HTTP_STATE_DEINITIALIZED) {
    return SL_STATUS_INVALID_STATE;
  }

  if (*client != (sl_http_client_t)&http_client_handle) {
    return SL_STATUS_INVALID_HANDLE;
  }

  // Free HTTP client credentials
  if (http_client_handle.client_credentials != NULL) {
    free(http_client_handle.client_credentials);
  }

  // Free extended headers
  if (http_client_handle.request.extended_header != NULL) {
    status = sl_http_client_delete_all_headers(&http_client_handle.request);
    VERIFY_STATUS_AND_RETURN(status);
  }

  memset(&http_client_handle, 0, sizeof(sl_http_client_internal_t));

  status = sli_si91x_http_client_abort();

  return status;
}

static sl_status_t sli_si91x_copy_ip_address_and_port(const sl_http_client_request_t *request)
{
  // Validate host name
  SL_WIFI_ARGS_CHECK_NULL_POINTER(request->ip_address);

  // Validate resource
  SL_WIFI_ARGS_CHECK_NULL_POINTER(request->resource);

  // 917 does not support HTTP POST and PUT request with body_length as 0 in Alpha 3 release
  // Bug in NWP, when SL_HTTP_POST is requested with body_length = 0, NWP sends SL_HTTP_GET
  // Bug in NWP, when SL_HTTP_PUT is requested with body_length = 0, NWP doesn't send end of data to host so currently there is no way to handle put_delete()
  // Below check should be removed once the above NWP bugs are fixed
  if (request->http_method_type != SL_HTTP_GET && request->body_length == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Validate http request method
  switch (request->http_method_type) {
    case SL_HTTP_GET:
    case SL_HTTP_POST:
    case SL_HTTP_PUT: {
      break;
    }
    case SL_HTTP_HEAD:
    case SL_HTTP_DELETE: {
      // Not supported in current release
      return SL_STATUS_NOT_SUPPORTED;
    }
    default: {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }

  return SL_STATUS_OK;
}

sl_status_t sl_http_client_request_init(sl_http_client_request_t *request,
                                        sl_http_client_event_handler_t event_handler,
                                        void *request_context)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(request);
  SL_WIFI_ARGS_CHECK_NULL_POINTER(event_handler);

  sl_status_t status = SL_STATUS_OK;

  // Validate HTTP client request
  status = sli_si91x_copy_ip_address_and_port(request);
  VERIFY_STATUS_AND_RETURN(status);

  request->event_handler = event_handler;
  request->context       = request_context;

  sl_http_client_event_t http_event;

  // Set HTTP client event for requested method
  switch (request->http_method_type) {
    case SL_HTTP_GET: {
      http_event                      = SL_HTTP_CLIENT_GET_RESPONSE_EVENT;
      http_client_handle.client_state = HTTP_STATE_GET_REQUEST_INITIALIZED;
      break;
    }
    case SL_HTTP_POST: {
      http_event                      = SL_HTTP_CLIENT_POST_RESPONSE_EVENT;
      http_client_handle.client_state = HTTP_STATE_POST_REQUEST_INITIALIZED;
      break;
    }
    case SL_HTTP_PUT: {
      http_event                      = SL_HTTP_CLIENT_PUT_RESPONSE_EVENT;
      http_client_handle.client_state = HTTP_STATE_PUT_REQUEST_INITIALIZED;
      break;
    }
    default: {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }

  // Register callback function for the http event
  status = sli_http_client_register_callback(http_event, (sl_http_client_t)&http_client_handle, event_handler);

  return status;
}

sl_status_t sl_http_client_add_header(sl_http_client_request_t *request, const char *key, const char *value)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(request);
  SL_WIFI_ARGS_CHECK_NULL_POINTER(key);
  SL_WIFI_ARGS_CHECK_NULL_POINTER(value);

  // Allocate memory for the new header
  sl_http_client_header_t *new_header = (sl_http_client_header_t *)malloc(sizeof(sl_http_client_header_t));
  SLI_VERIFY_MALLOC_AND_RETURN(new_header);

  // Memset the heap chunk
  memset(new_header, 0, sizeof(sl_http_client_header_t));

  // Allocate memory for key
  new_header->key = (char *)malloc(strlen(key) + 1);
  VERIFY_MALLOC_AND_FREE_IF_FAIL(new_header->key, new_header, NULL);

  // Copy key in header
  snprintf(new_header->key, strlen(key) + 1, "%s", key);

  // Allocate memory for value
  new_header->value = (char *)malloc(strlen(value) + 1);
  VERIFY_MALLOC_AND_FREE_IF_FAIL(new_header->value, new_header->key, new_header);

  // Copy value in header
  snprintf(new_header->value, strlen(value) + 1, "%s", value);

  // Add new header at start of linked list
  sl_slist_push((sl_slist_node_t **)&request->extended_header, (sl_slist_node_t *)new_header);

  return SL_STATUS_OK;
}

sl_status_t sl_http_client_delete_header(sl_http_client_request_t *request, const char *key)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(request);
  SL_WIFI_ARGS_CHECK_NULL_POINTER(key);

  sl_http_client_header_t *current_header = request->extended_header;

  // Check if linked list is empty
  if (request->extended_header == NULL) {
    return SL_STATUS_EMPTY;
  }

  // Search key in linked list
  while ((current_header != NULL) && (strcmp(current_header->key, key) != 0)) {
    current_header = (sl_http_client_header_t *)current_header->node.node;
  }

  // If key not present in linked list
  if (current_header == NULL) {
    return SL_STATUS_INVALID_KEY;
  }

  // Unlink node from the linked list
  sl_slist_remove((sl_slist_node_t **)&request->extended_header, (sl_slist_node_t *)current_header);

  if (current_header != NULL) {
    free(current_header->key);
    free(current_header->value);
    free(current_header);
  }

  return SL_STATUS_OK;
}

sl_status_t sl_http_client_delete_all_headers(sl_http_client_request_t *request)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(request);

  sl_http_client_header_t *current_header = request->extended_header;
  sl_http_client_header_t *next_header    = NULL;

  // Check if linked list is empty
  if (request->extended_header == NULL) {
    return SL_STATUS_EMPTY;
  }

  while (current_header != NULL) {
    // Store next header link
    next_header = (sl_http_client_header_t *)current_header->node.node;

    // Remove node from list
    sl_slist_remove((sl_slist_node_t **)&request->extended_header, (sl_slist_node_t *)current_header);

    // Free current node
    free(current_header->key);
    free(current_header->value);
    free(current_header);

    // Traverse to next node
    current_header = next_header;
  }

  // Set head node of a linked list to NULL
  request->extended_header                   = NULL;
  http_client_handle.request.extended_header = NULL;

  return SL_STATUS_OK;
}
static void sli_si91x_load_extended_headers_into_request_buffer(uint8_t *buffer,
                                                                sl_http_client_header_t *extended_header,
                                                                uint16_t *http_buffer_offset)
{
  sl_http_client_header_t *current_header = extended_header;

  if (current_header == NULL) {
    buffer[(*http_buffer_offset)] = '\0';
    (*http_buffer_offset)++;
  }

  while (current_header != NULL) {
    // Copy header key
    *http_buffer_offset += snprintf((char *)(buffer + (*http_buffer_offset)),
                                    SLI_SI91X_HTTP_BUFFER_LEN - (*http_buffer_offset),
                                    "%s",
                                    current_header->key);

    buffer[(*http_buffer_offset)] = ':';
    (*http_buffer_offset)++;

    // Copy header value
    *http_buffer_offset += snprintf((char *)(buffer + (*http_buffer_offset)),
                                    SLI_SI91X_HTTP_BUFFER_LEN - (*http_buffer_offset),
                                    "%s",
                                    current_header->value);

    // Add carriage return to buffer
    buffer[(*http_buffer_offset)] = '\r';
    (*http_buffer_offset)++;

    // Add newline to buffer
    buffer[(*http_buffer_offset)] = '\n';
    (*http_buffer_offset)++;

    // Check if current header is the last one
    if (current_header->node.node == NULL) {
      // Add null terminator to buffer
      buffer[(*http_buffer_offset)] = '\0';
      (*http_buffer_offset)++;
      break;
    } else {
      // Point to next header
      current_header = (sl_http_client_header_t *)current_header->node.node;
    }
  }
}

static sli_si91x_http_client_request_t *sli_allocate_and_initialize_request()
{
  sli_si91x_http_client_request_t *http_client_request =
    (sli_si91x_http_client_request_t *)malloc(sizeof(sli_si91x_http_client_request_t));
  if (http_client_request != NULL) {
    memset(http_client_request, 0, sizeof(sli_si91x_http_client_request_t));
  }
  return http_client_request;
}

static sl_status_t sli_configure_https(sli_si91x_http_client_request_t *http_client_request,
                                       const sl_http_client_internal_t *client_internal,
                                       const sl_http_client_request_t *request)
{
  http_client_request->https_enable |= SL_SI91X_ENABLE_TLS;

  // Fill SSL/TLS version
  switch (client_internal->configuration.tls_version) {
    case SL_TLS_DEFAULT_VERSION:
      break;
    case SL_TLS_V_1_0:
      http_client_request->https_enable |= SL_SI91X_TLS_V_1_0;
      break;
    case SL_TLS_V_1_1:
      http_client_request->https_enable |= SL_SI91X_TLS_V_1_1;
      break;
    case SL_TLS_V_1_2:
      http_client_request->https_enable |= SL_SI91X_TLS_V_1_2;
      break;
#ifdef SLI_SI917
    case SL_TLS_V_1_3:
      http_client_request->https_enable |= SL_SI91X_TLS_V_1_3;
      break;
#endif
    default:
      return SL_STATUS_INVALID_CONFIGURATION;
  }

  // Fill HTTPS certificate index bitmap
  switch (client_internal->configuration.certificate_index) {
    case SL_HTTPS_CLIENT_CERTIFICATE_INDEX_1: {
      http_client_request->https_enable |= SL_SI91X_HTTPS_CERTIFICATE_INDEX_1;
      break;
    }
    case SL_HTTPS_CLIENT_CERTIFICATE_INDEX_2: {
      http_client_request->https_enable |= SL_SI91X_HTTPS_CERTIFICATE_INDEX_2;
      break;
    }
    case SL_HTTPS_CLIENT_DEFAULT_CERTIFICATE_INDEX: {
      break;
    }
    default:
      break;
  }

  if (client_internal->configuration.https_use_sni) {
    // Convert public socket type to internal TLS extension type
    sli_si91x_tls_extension_info_t *internal_sni = NULL;

    if (request->sni_extension != NULL) {
      // Check for potential buffer overflow
      if (request->sni_extension->length > SLI_SI91X_MAX_SIZE_OF_EXTENSION_DATA) {
        return SL_STATUS_WOULD_OVERFLOW;
      }

      internal_sni = (sli_si91x_tls_extension_info_t *)malloc(sizeof(sli_si91x_tls_extension_info_t)
                                                              + request->sni_extension->length);
      if (internal_sni == NULL) {
        return SL_STATUS_ALLOCATION_FAILED;
      }

      internal_sni->type   = request->sni_extension->type;
      internal_sni->length = request->sni_extension->length;
      memcpy(internal_sni->value, request->sni_extension->value, request->sni_extension->length);
    }

    sl_status_t status = sli_configure_sni(internal_sni, request->host_name, SI91X_SNI_FOR_HTTPS);

    if (internal_sni != NULL) {
      free(internal_sni);
    }

    return status;
  }

  return SL_STATUS_OK;
}

static sl_status_t sli_fill_http_request_common_fields(sli_si91x_http_client_request_t *http_client_request,
                                                       const sl_http_client_internal_t *client_internal,
                                                       const sl_http_client_request_t *request)
{
  if (client_internal->configuration.ip_version == SL_IPV6) {
    http_client_request->ip_version = 6;
  } else {
    http_client_request->ip_version = 4;
  }

  // Set default by NULL delimiter
  http_client_request->https_enable |= SL_SI91X_ENABLE_NULL_DELIMETER;

  // Fill HTTPS feature
  if (client_internal->configuration.https_enable) {
    if (client_internal->configuration.https_use_sni) {
      http_client_request->https_enable |= SL_SI91X_HTTPS_USE_SNI;
    }
    sl_status_t status = sli_configure_https(http_client_request, client_internal, request);
    if (status != SL_STATUS_OK) {
      return status;
    }
  }

  // Fill HTTP version
  if (client_internal->configuration.http_version == SL_HTTP_V_1_1) {
    http_client_request->https_enable |= SL_SI91X_HTTP_V_1_1;
  }

  // Fill port number
  http_client_request->port_number = request->port;

  return SL_STATUS_OK;
}

static sl_status_t sli_fill_http_request_buffer(sli_si91x_http_client_request_t *http_client_request,
                                                const sl_http_client_internal_t *client_internal,
                                                const sl_http_client_request_t *request,
                                                uint16_t *http_buffer_offset,
                                                sl_http_client_method_type_t send_request)
{
  // Fill username
  memcpy(http_client_request->buffer,
         &client_internal->client_credentials->data[0],
         client_internal->client_credentials->username_length);
  *http_buffer_offset += client_internal->client_credentials->username_length;
  http_client_request->buffer[*http_buffer_offset] = '\0';
  (*http_buffer_offset)++;

  // Fill password
  memcpy(http_client_request->buffer + *http_buffer_offset,
         &client_internal->client_credentials->data[client_internal->client_credentials->username_length],
         client_internal->client_credentials->password_length);
  *http_buffer_offset += client_internal->client_credentials->password_length;
  http_client_request->buffer[*http_buffer_offset] = '\0';
  (*http_buffer_offset)++;

  // Check for HTTP_V_1.1 and Empty host name and fill IP address
  if (client_internal->configuration.http_version == SL_HTTP_V_1_1
      && (request->host_name == NULL || sl_strlen((char *)request->host_name) == 0)) {
    *http_buffer_offset += snprintf((char *)(http_client_request->buffer + *http_buffer_offset),
                                    SLI_SI91X_HTTP_BUFFER_LEN - *http_buffer_offset,
                                    "%s",
                                    request->ip_address);
  } else if (request->host_name != NULL) {
    // Fill hostname
    *http_buffer_offset += snprintf((char *)(http_client_request->buffer + *http_buffer_offset),
                                    SLI_SI91X_HTTP_BUFFER_LEN - *http_buffer_offset,
                                    "%s",
                                    request->host_name);
  }
  (*http_buffer_offset)++;

  // Fill IP address
  *http_buffer_offset += snprintf((char *)(http_client_request->buffer + *http_buffer_offset),
                                  SLI_SI91X_HTTP_BUFFER_LEN - *http_buffer_offset,
                                  "%s",
                                  request->ip_address);
  (*http_buffer_offset)++;

  // Fill URL resource
  *http_buffer_offset += snprintf((char *)(http_client_request->buffer + *http_buffer_offset),
                                  SLI_SI91X_HTTP_BUFFER_LEN - *http_buffer_offset,
                                  "%s",
                                  request->resource);
  (*http_buffer_offset)++;

  // Fill extended header
  if (request->extended_header != NULL) {
    // Enable user given content type in extended header
    http_client_request->https_enable |= SL_SI91X_HTTP_USER_DEFINED_CONTENT_TYPE;
    sli_si91x_load_extended_headers_into_request_buffer(http_client_request->buffer,
                                                        request->extended_header,
                                                        http_buffer_offset);
  } else {
    http_client_request->buffer[*http_buffer_offset] = '\0';
    (*http_buffer_offset)++;
  }

  if (send_request == SL_HTTP_POST && request->body == NULL) {
    // Check for HTTP post data feature
    // Enable HTTP Post big data feature
    http_client_request->https_enable |= SL_SI91X_SUPPORT_HTTP_POST_DATA;

    // Copy total data length into buffer
    uint8_t temp_str[TEMP_STR_SIZE] = { 0 };
    convert_itoa(request->body_length, temp_str);
    size_t temp_str_len = sl_strnlen((char *)temp_str, TEMP_STR_SIZE + 1);
    memcpy(http_client_request->buffer + *http_buffer_offset, temp_str, temp_str_len);
    *http_buffer_offset += temp_str_len;
  } else if (send_request == SL_HTTP_POST) {
    // Fill HTTP post data
    memcpy(http_client_request->buffer + *http_buffer_offset, request->body, request->body_length);
    *http_buffer_offset += request->body_length;
  }

  return SL_STATUS_OK;
}

static sl_status_t sli_send_single_http_request(sl_http_client_method_type_t send_request,
                                                const sli_si91x_http_client_request_t *http_client_request,
                                                uint32_t packet_length,
                                                const sl_http_client_request_t *request)
{
  if (send_request == SL_HTTP_POST) {
    return sli_si91x_driver_send_command(SLI_WLAN_REQ_HTTP_CLIENT_POST,
                                         SLI_SI91X_NETWORK_CMD,
                                         http_client_request,
                                         packet_length,
                                         SLI_WIFI_RETURN_IMMEDIATELY,
                                         request->context,
                                         NULL);
  } else {
    return sli_si91x_driver_send_command(SLI_WLAN_REQ_HTTP_CLIENT_GET,
                                         SLI_SI91X_NETWORK_CMD,
                                         http_client_request,
                                         packet_length,
                                         SLI_WIFI_RETURN_IMMEDIATELY,
                                         request->context,
                                         NULL);
  }
}

static sl_status_t sli_send_chunked_http_request(sl_http_client_method_type_t send_request,
                                                 const sli_si91x_http_client_request_t *http_client_request,
                                                 uint16_t rem_length,
                                                 uint16_t *offset,
                                                 uint16_t *chunk_size,
                                                 const sl_http_client_request_t *request)
{
  // Allocate memory for a new packet buffer
  sli_si91x_http_client_request_t *packet_buffer = malloc(sizeof(sli_si91x_http_client_request_t));
  if (packet_buffer == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  sl_status_t status = SL_STATUS_OK;
  uint8_t packet_identifier;

  // Iterate through the length of the packet
  while (rem_length) {
    memset(packet_buffer, 0, sizeof(sli_si91x_http_client_request_t));

    // Fill the packet identifier
    if (rem_length > SLI_SI91X_MAX_HTTP_CHUNK_SIZE) {
      packet_identifier = (*offset == 0) ? SLI_HTTP_GET_FIRST_PKT : SLI_HTTP_GET_MIDDLE_PKT;
    } else {
      packet_identifier = SLI_HTTP_GET_LAST_PKT;
      *chunk_size       = rem_length;
    }

    // Fill the HTTP params
    packet_buffer->ip_version   = http_client_request->ip_version;
    packet_buffer->https_enable = http_client_request->https_enable;
    packet_buffer->port_number  = http_client_request->port_number;

    // Copy the original buffer into the new packet buffer with an offset and for a chunk size
    memcpy(packet_buffer->buffer, (http_client_request->buffer + *offset), *chunk_size);

    if (send_request == SL_HTTP_GET) {
      // HTTP Get request with custom driver command
      status = sl_si91x_custom_driver_send_command(
        SLI_WLAN_REQ_HTTP_CLIENT_GET,
        SLI_SI91X_NETWORK_CMD,
        packet_buffer,
        (sizeof(sli_si91x_http_client_request_t) - SLI_SI91X_HTTP_BUFFER_LEN + *chunk_size),
        SLI_WIFI_RETURN_IMMEDIATELY,
        request->context,
        NULL,
        packet_identifier);
    } else if (send_request == SL_HTTP_POST) {
      // HTTP POST request with custom driver command
      status = sl_si91x_custom_driver_send_command(
        SLI_WLAN_REQ_HTTP_CLIENT_POST,
        SLI_SI91X_NETWORK_CMD,
        packet_buffer,
        (sizeof(sli_si91x_http_client_request_t) - SLI_SI91X_HTTP_BUFFER_LEN + *chunk_size),
        SLI_WIFI_RETURN_IMMEDIATELY,
        request->context,
        NULL,
        packet_identifier);
    }

    // Increase the offset by chunk_size
    *offset += *chunk_size;

    // Decrease the rem_length by chunk_size
    rem_length -= *chunk_size;
  }

  // Free request structure memory
  free(packet_buffer);
  return status;
}

static sl_status_t sli_si91x_send_http_client_request(sl_http_client_method_type_t send_request,
                                                      const sl_http_client_internal_t *client_internal,
                                                      const sl_http_client_request_t *request)
{
  sl_status_t status          = SL_STATUS_OK;
  uint32_t packet_length      = 0;
  uint16_t http_buffer_offset = 0;
  uint16_t offset             = 0;
  uint16_t rem_length         = 0;
  uint16_t chunk_size         = SLI_SI91X_MAX_HTTP_CHUNK_SIZE;

  sli_si91x_http_client_request_t *http_client_request = sli_allocate_and_initialize_request();
  if (http_client_request == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  status = sli_fill_http_request_common_fields(http_client_request, client_internal, request);
  if (status != SL_STATUS_OK) {
    free(http_client_request);
    return status;
  }

  status =
    sli_fill_http_request_buffer(http_client_request, client_internal, request, &http_buffer_offset, send_request);
  if (status != SL_STATUS_OK) {
    free(http_client_request);
    return status;
  }

  // Check if request buffer is overflowed or resource length is overflowed
  if (http_buffer_offset > SLI_SI91X_HTTP_BUFFER_LEN
      || sl_strnlen((char *)request->resource, SLI_SI91X_MAX_HTTP_URL_SIZE + 1) > SLI_SI91X_MAX_HTTP_URL_SIZE) {
    free(http_client_request);
    return SL_STATUS_HAS_OVERFLOWED;
  }

  // Fill total packet length
  packet_length = sizeof(sli_si91x_http_client_request_t) - SLI_SI91X_HTTP_BUFFER_LEN + http_buffer_offset;

  // Copy the total http buffer size to the rem_length
  rem_length = http_buffer_offset;

  // Check if the HTTP buffer size exceeds the limit
  if (http_buffer_offset <= SLI_SI91X_MAX_HTTP_CHUNK_SIZE) {
    status = sli_send_single_http_request(send_request, http_client_request, packet_length, request);
  } else {
    status =
      sli_send_chunked_http_request(send_request, http_client_request, rem_length, &offset, &chunk_size, request);
  }

  // Free request structure memory
  free(http_client_request);
  return status;
}

static sl_status_t sli_http_client_send_get_request(const sl_http_client_internal_t *client_internal,
                                                    const sl_http_client_request_t *request)
{
  // Validate HTTP client state
  if (http_client_handle.client_state != HTTP_STATE_GET_REQUEST_INITIALIZED) {
    return SL_STATUS_INVALID_STATE;
  }

  //! Send HTTP client GET request
  return sli_si91x_send_http_client_request(SL_HTTP_GET, client_internal, request);
}

static sl_status_t sli_http_client_send_post_request(const sl_http_client_internal_t *client_internal,
                                                     const sl_http_client_request_t *request)
{
  // Validate HTTP client state
  if (http_client_handle.client_state != HTTP_STATE_POST_REQUEST_INITIALIZED) {
    return SL_STATUS_INVALID_STATE;
  }

  //! Send HTTP client POST request
  return sli_si91x_send_http_client_request(SL_HTTP_POST, client_internal, request);
}

static sl_status_t sli_http_client_send_put_request(const sl_http_client_internal_t *client_internal,
                                                    const sl_http_client_request_t *request)
{
  sl_status_t status          = SL_STATUS_OK;
  uint32_t packet_length      = 0;
  uint16_t http_buffer_offset = 0;

  // Validate HTTP client state
  if (http_client_handle.client_state != HTTP_STATE_PUT_REQUEST_INITIALIZED) {
    return SL_STATUS_INVALID_STATE;
  }

  // 917 does not support this feature for PUT request in Alpha 3 release
  if (request->body != NULL) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Allocate memory for request structure
  sl_si91x_http_client_put_request_t *http_put_request =
    (sl_si91x_http_client_put_request_t *)malloc(sizeof(sl_si91x_http_client_put_request_t));
  SLI_VERIFY_MALLOC_AND_RETURN(http_put_request);

  memset(http_put_request, 0, sizeof(sl_si91x_http_client_put_request_t));

  sli_si91x_http_client_put_start_t *http_put_start =
    &http_put_request->sli_http_client_put_struct.http_client_put_start;

  //! Create HTTP PUT client
  // Fill command type
  http_put_request->command_type = SLI_SI91X_HTTP_CLIENT_PUT_CREATE;

  // Fill payload length
  // Since body is not sent in this request, we are subtracting max SLI_SI91X_HTTP_CLIENT_PUT_MAX_BUFFER_LENGTH
  packet_length = sizeof(sl_si91x_http_client_put_request_t) - SLI_SI91X_HTTP_CLIENT_PUT_MAX_BUFFER_LENGTH;

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_HTTP_CLIENT_PUT,
                                         SLI_SI91X_NETWORK_CMD,
                                         http_put_request,
                                         packet_length,
                                         SLI_WLAN_RSP_HTTP_CLIENT_PUT_WAIT_TIME,
                                         NULL,
                                         NULL);
  if (status != SL_STATUS_OK) {
    free(http_put_request);
    return status;
  }

  //! Start HTTP client PUT process
  // Fill command type
  http_put_request->command_type = SLI_SI91X_HTTP_CLIENT_PUT_START;

  // Fill IP version
  if (client_internal->configuration.ip_version == SL_IPV6) {
    http_put_start->ip_version = 6;
  } else {
    http_put_start->ip_version = 4;
  }

  // Set default by NULL delimiter
  http_put_start->https_enable |= SL_SI91X_ENABLE_NULL_DELIMETER;

  // Fill HTTPS feature
  if (client_internal->configuration.https_enable) {
    http_put_start->https_enable = SL_SI91X_ENABLE_TLS;

    // Fill SSL/TLS version
    switch (client_internal->configuration.tls_version) {
      case SL_TLS_DEFAULT_VERSION: {
        break;
      }
      case SL_TLS_V_1_0: {
        http_put_start->https_enable |= SL_SI91X_TLS_V_1_0;
        break;
      }
      case SL_TLS_V_1_1: {
        http_put_start->https_enable |= SL_SI91X_TLS_V_1_1;
        break;
      }
      case SL_TLS_V_1_2: {
        http_put_start->https_enable |= SL_SI91X_TLS_V_1_2;
        break;
      }
#ifdef SLI_SI917
      case SL_TLS_V_1_3: {
        http_put_start->https_enable |= SL_SI91X_TLS_V_1_3;
        break;
      }
#endif
      default:
        free(http_put_request);
        return SL_STATUS_INVALID_CONFIGURATION;
    }

    // Fill HTTPS certificate index bitmap
    switch (client_internal->configuration.certificate_index) {
      case SL_HTTPS_CLIENT_CERTIFICATE_INDEX_1: {
        http_put_start->https_enable |= SL_SI91X_HTTPS_CERTIFICATE_INDEX_1;
        break;
      }
      case SL_HTTPS_CLIENT_CERTIFICATE_INDEX_2: {
        http_put_start->https_enable |= SL_SI91X_HTTPS_CERTIFICATE_INDEX_2;
        break;
      }
      case SL_HTTPS_CLIENT_DEFAULT_CERTIFICATE_INDEX: {
        break;
      }
      default:
        break;
    }
  }

  // Fill HTTP version
  if (client_internal->configuration.http_version == SL_HTTP_V_1_1) {
    http_put_start->https_enable |= SL_SI91X_HTTP_V_1_1;
  }

  // Fill port number
  http_put_start->port_number = (uint32_t)request->port;

  // Fill Total resource content length
  http_put_start->content_length = request->body_length;

  // Fill username
  memcpy(http_put_request->http_put_buffer,
         &client_internal->client_credentials->data[0],
         client_internal->client_credentials->username_length);
  http_buffer_offset += client_internal->client_credentials->username_length;
  http_put_request->http_put_buffer[http_buffer_offset] = '\0';
  http_buffer_offset++;

  // Fill password
  memcpy(http_put_request->http_put_buffer + http_buffer_offset,
         &client_internal->client_credentials->data[client_internal->client_credentials->username_length],
         client_internal->client_credentials->password_length);
  http_buffer_offset += client_internal->client_credentials->password_length;
  http_put_request->http_put_buffer[http_buffer_offset] = '\0';
  http_buffer_offset++;

  // Check for HTTP_V_1.1 and Empty host name
  if (client_internal->configuration.http_version == SL_HTTP_V_1_1
      && (strlen((char *)request->host_name) == 0 || request->host_name == NULL)) {
    memcpy((char *)request->host_name,
           (char *)request->ip_address,
           MIN(strlen((char *)request->host_name), strlen((char *)request->ip_address)) + 1);
  }

  // Fill hostname
  http_buffer_offset += snprintf((char *)(http_put_request->http_put_buffer + http_buffer_offset),
                                 SLI_SI91X_HTTP_CLIENT_PUT_MAX_BUFFER_LENGTH - http_buffer_offset,
                                 "%s",
                                 request->host_name);
  http_buffer_offset++;

  // Fill IP address
  http_buffer_offset += snprintf((char *)(http_put_request->http_put_buffer + http_buffer_offset),
                                 SLI_SI91X_HTTP_CLIENT_PUT_MAX_BUFFER_LENGTH - http_buffer_offset,
                                 "%s",
                                 request->ip_address);
  http_buffer_offset++;

  // Fill URL resource
  http_buffer_offset += snprintf((char *)(http_put_request->http_put_buffer + http_buffer_offset),
                                 SLI_SI91X_HTTP_CLIENT_PUT_MAX_BUFFER_LENGTH - http_buffer_offset,
                                 "%s",
                                 request->resource);
  http_buffer_offset++;

  // Fill extended header
  if (request->extended_header != NULL) {
    // Enable user given content type in extended header
    http_put_start->https_enable |= SL_SI91X_HTTP_USER_DEFINED_CONTENT_TYPE;

    sli_si91x_load_extended_headers_into_request_buffer(http_put_request->http_put_buffer,
                                                        request->extended_header,
                                                        &http_buffer_offset);
  } else {
    http_put_request->http_put_buffer[http_buffer_offset] = '\0';
    http_buffer_offset++;
  }

  // Check if request buffer is overflowed
  if (http_buffer_offset > SLI_SI91X_HTTP_BUFFER_LEN) {
    free(http_put_request);
    return SL_STATUS_HAS_OVERFLOWED;
  }

  // Fill data length
  packet_length =
    sizeof(sl_si91x_http_client_put_request_t) - SLI_SI91X_HTTP_CLIENT_PUT_MAX_BUFFER_LENGTH + http_buffer_offset;

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_HTTP_CLIENT_PUT,
                                         SLI_SI91X_NETWORK_CMD,
                                         http_put_request,
                                         packet_length,
                                         SLI_WIFI_RETURN_IMMEDIATELY,
                                         request->context,
                                         NULL);

  // Free memory of request struture
  free(http_put_request);

  return status;
}

sl_status_t sl_http_client_send_request(const sl_http_client_t *client, const sl_http_client_request_t *request)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(client);
  SL_WIFI_ARGS_CHECK_NULL_POINTER(request);

  if (*client != (sl_http_client_t)&http_client_handle) {
    return SL_STATUS_INVALID_HANDLE;
  }

  // Validate HTTP client request
  sl_status_t status = sli_si91x_copy_ip_address_and_port(request);
  VERIFY_STATUS_AND_RETURN(status);

  switch (request->http_method_type) {
    case SL_HTTP_GET: {
      status = sli_http_client_send_get_request(&http_client_handle, request);
      break;
    }

    case SL_HTTP_POST: {
      status = sli_http_client_send_post_request(&http_client_handle, request);
      break;
    }

    case SL_HTTP_PUT: {
      status = sli_http_client_send_put_request(&http_client_handle, request);
      break;
    }

    default: {
      return SL_STATUS_FAIL;
    }
  }

  if (status == SL_STATUS_OK || status == SL_STATUS_IN_PROGRESS) {
    // Store request configurations into client_internal structure
    memcpy(&http_client_handle.request, request, sizeof(sl_http_client_request_t));

    // Set HTTP client state to requested sent state
    if (request->http_method_type != SL_HTTP_GET && request->body == NULL) {
      http_client_handle.client_state = HTTP_STATE_CHUNKED_REQUEST_SENT;
    } else {
      http_client_handle.client_state = HTTP_STATE_REQUEST_SENT;
    }
  }

  return status;
}

sl_status_t sl_http_client_write_chunked_data(const sl_http_client_t *client,
                                              const uint8_t *data,
                                              uint32_t data_length,
                                              bool flush_now)
{
  UNUSED_PARAMETER(flush_now);
  SL_WIFI_ARGS_CHECK_NULL_POINTER(client);
  SL_WIFI_ARGS_CHECK_NULL_POINTER(data);

  if (*client != (sl_http_client_t)&http_client_handle) {
    return SL_STATUS_INVALID_HANDLE;
  }

  // Check for HTTP client requested state
  if (http_client_handle.client_state != HTTP_STATE_CHUNKED_REQUEST_SENT) {
    return SL_STATUS_INVALID_STATE;
  }

  // Check for data length more than max buffer size
  if (data_length > SLI_SI91X_HTTP_CLIENT_MAX_WRITE_BUFFER_LENGTH) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Check for invalid data length
  if ((data_length == 0) && (sl_strlen((char *)data) == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status     = SL_STATUS_OK;
  uint16_t packet_length = 0;

  switch (http_client_handle.request.http_method_type) {
    case SL_HTTP_POST: {
      // Allocate memory for request structure
      sli_si91x_http_client_post_data_request_t *http_post_data =
        (sli_si91x_http_client_post_data_request_t *)malloc(sizeof(sli_si91x_http_client_post_data_request_t));
      SLI_VERIFY_MALLOC_AND_RETURN(http_post_data);
      memset(http_post_data, 0, sizeof(sli_si91x_http_client_post_data_request_t));
      // Fill HTTP Post data current chunk length
      http_post_data->current_length = (uint16_t)data_length;

      // Fill HTTP Post data
      memcpy(http_post_data->http_post_data_buffer, data, data_length);

      // Fill total packet length
      packet_length = (uint16_t)(sizeof(sli_si91x_http_client_post_data_request_t)
                                 - SLI_SI91X_HTTP_CLIENT_POST_MAX_BUFFER_LENGTH + data_length);

      // Send HTTP Post Data request
      status = sli_si91x_driver_send_command(SLI_WLAN_REQ_HTTP_CLIENT_POST_DATA,
                                             SLI_SI91X_NETWORK_CMD,
                                             http_post_data,
                                             packet_length,
                                             SLI_WIFI_RETURN_IMMEDIATELY,
                                             http_client_handle.request.context,
                                             NULL);

      // Free memory of request struture
      free(http_post_data);

      break;
    }

    case SL_HTTP_PUT: {
      // Allocate memory for request structure
      sl_si91x_http_client_put_request_t *http_put_pkt_request =
        (sl_si91x_http_client_put_request_t *)malloc(sizeof(sl_si91x_http_client_put_request_t));
      SLI_VERIFY_MALLOC_AND_RETURN(http_put_pkt_request);
      memset(http_put_pkt_request, 0, sizeof(sl_si91x_http_client_put_request_t));
      sli_si91x_http_client_put_data_request_t *http_put_data =
        &http_put_pkt_request->sli_http_client_put_struct.http_client_put_data_req;

      // Fill command type
      http_put_pkt_request->command_type = SLI_SI91X_HTTP_CLIENT_PUT_PKT;

      // Fill HTTP Put packet current chunk length
      http_put_data->current_length = (uint16_t)data_length;

      // Fill HTTP Put data
      memcpy(http_put_pkt_request->http_put_buffer, data, data_length);

      // Fill total packet length
      packet_length = (uint16_t)(sizeof(sl_si91x_http_client_put_request_t)
                                 - SLI_SI91X_HTTP_CLIENT_PUT_MAX_BUFFER_LENGTH + data_length);

      // Send HTTP Put Data request
      status = sli_si91x_driver_send_command(SLI_WLAN_REQ_HTTP_CLIENT_PUT,
                                             SLI_SI91X_NETWORK_CMD,
                                             http_put_pkt_request,
                                             packet_length,
                                             SLI_WIFI_RETURN_IMMEDIATELY,
                                             http_client_handle.request.context,
                                             NULL);

      // Free memory of request struture
      free(http_put_pkt_request);

      break;
    }

    default:
      return SL_STATUS_FAIL;
  }

  return status;
}

static sl_status_t sli_si91x_http_client_abort(void)
{
  sl_status_t status = sli_si91x_driver_send_command(SLI_WLAN_REQ_HTTP_ABORT,
                                                     SLI_SI91X_NETWORK_CMD,
                                                     NULL,
                                                     0,
                                                     SLI_WLAN_RSP_HTTP_ABORT_WAIT_TIME,
                                                     NULL,
                                                     NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}
