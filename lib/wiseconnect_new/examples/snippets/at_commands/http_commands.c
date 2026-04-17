/*******************************************************************************
 * @file  http_commands.c
 * @brief HTTP AT Commands
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

#include "sl_board_configuration.h"
#include "sl_rsi_utility.h"
#include "sl_si91x_socket_constants.h"
#include "console.h"
#include "sl_net.h"
#include "sl_http_client.h"
#include "at_utility.h"
#include "at_command_data_mode.h"
#include <string.h>
#include <inttypes.h>

/******************************************************
 *                    Constants
 ******************************************************/
#define HTTP_PORT 80

#define HTTP_CLIENT_INVALID_ID 0xFF
#define HTTP_CLIENT_DEFAULT_ID 0

#define HTTP_USERNAME_DEFAULT ""
#define HTTP_PASSWORD_DEFAULT ""

#define HTTP_INVALID_REQUEST_ID 0xFFFFFFFFU

#ifndef HTTP_OUTPUT_BUFFER_SIZE
#define HTTP_OUTPUT_BUFFER_SIZE 1024
#endif

/******************************************************
 *               Variable Definitions
 ******************************************************/
extern sl_wifi_device_configuration_t si91x_init_configuration;

static sl_http_client_t http_client = 0;

// HTTP Client Request
typedef struct {
  bool is_first_init;
  bool flush_now;
  int request_id;
  volatile uint32_t remaining;
  sl_http_client_request_t client_request;
} http_client_request_t;

static http_client_request_t http_request = {
  .is_first_init  = true,
  .flush_now      = false,
  .request_id     = HTTP_INVALID_REQUEST_ID,
  .remaining      = 0,
  .client_request = { 0 },
};

static sl_status_t check_http_client_instance_id(console_args_t *arguments, uint8_t index)
{
  if (http_client == 0) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  uint8_t client_inst_id = GET_OPTIONAL_COMMAND_ARG(arguments, index, HTTP_CLIENT_INVALID_ID, uint8_t);
  if (client_inst_id != HTTP_CLIENT_DEFAULT_ID) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return SL_STATUS_OK;
}

static void cleanup_sl_http_client_request(sl_http_client_request_t *request)
{
  if (request == NULL) {
    return;
  }
  SL_CLEANUP_MALLOC(request->ip_address);
  SL_CLEANUP_MALLOC(request->host_name);
  SL_CLEANUP_MALLOC(request->sni_extension);
  SL_CLEANUP_MALLOC(request->resource);
  SL_CLEANUP_MALLOC(request->body);
  request->body_length = 0;
  sl_http_client_delete_all_headers(request);
}

static void cleanup_http_client_request(http_client_request_t *request)
{
  if (request == NULL) {
    return;
  }
  request->is_first_init = true;
  request->flush_now     = false;
  request->request_id    = HTTP_INVALID_REQUEST_ID;
  request->remaining     = 0;
  cleanup_sl_http_client_request(&request->client_request);
}

static void is_first_init_then_clean_up_request(http_client_request_t *request)
{
  if (request == NULL) {
    return;
  }
  if (request->is_first_init) {
    cleanup_http_client_request(request);
    request->is_first_init = false;
  }
}

static sl_status_t http_response_callback_handler(const sl_http_client_t *client,
                                                  sl_http_client_event_t event,
                                                  void *data,
                                                  void *request_context)
{
  http_client_request_t *request = (http_client_request_t *)request_context;

  if ((request == NULL) || (client == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }

  if ((http_client == 0) || (http_client != *client)) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_status_t status                  = SL_STATUS_OK;
  sl_http_client_response_t *response = (sl_http_client_response_t *)data;

  if (response->status != SL_STATUS_OK) {
    status = response->status;
  }

  AT_PRINTF("at+HTTP_CLIENT_RESPONSE=%d,%d,%d,%" PRIu32 ",%d,%d,",
            HTTP_CLIENT_DEFAULT_ID,
            request->request_id,
            event,
            response->status,
            response->http_response_code,
            response->version);

  uint32_t header_count                = 0;
  char output[HTTP_OUTPUT_BUFFER_SIZE] = { 0 }; // Buffer to store headers
  char *buffer                         = output;
  int remaining_size                   = HTTP_OUTPUT_BUFFER_SIZE;

  sl_http_client_header_t *header = response->response_headers;
  while (header != NULL) {
    // Count headers
    header_count++;

    // Add header key and value to output
    int written = snprintf(buffer,
                           remaining_size,
                           "\"%s\",\"%s\",",
                           header->key ? header->key : "",
                           header->value ? header->value : "");
    if (written > 0 && written < remaining_size) {
      buffer += written;
      remaining_size -= written;
    } else {
      return SL_STATUS_WOULD_OVERFLOW;
    }

    header = (sl_http_client_header_t *)header->node.node;
  }

  AT_PRINTF("%" PRIu32 ",%s,", header_count, output);
  AT_PRINTF("%" PRIu32 ",%d,", response->end_of_data, response->data_length);
  at_print_char_buffer((char *)response->data_buffer, response->data_length);
  AT_PRINTF("\r\n>\r\n");
  return status;
}

static sl_status_t http_client_req_body_send_buffer_handler(uint8_t *buffer, uint32_t length, void *user_data)
{
  http_client_request_t *request = (http_client_request_t *)user_data;
  if (request == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // Set the buffer and length in the request
  SL_CLEANUP_MALLOC(request->client_request.body);
  request->client_request.body = calloc(1, length);
  SL_VERIFY_POINTER_OR_RETURN(request->client_request.body, SL_STATUS_ALLOCATION_FAILED);
  memcpy(request->client_request.body, buffer, length);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

static sl_status_t http_client_send_chunk_send_buffer_handler(uint8_t *buffer, uint32_t length, void *user_data)
{
  http_client_request_t *request = (http_client_request_t *)user_data;
  if (request == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  uint32_t offset       = 0;
  uint32_t chunk_length = 0;

  sl_status_t status;

  while (request->remaining > 0) {
    chunk_length = (length - offset > SL_HTTP_CLIENT_MAX_WRITE_BUFFER_LENGTH) ? SL_HTTP_CLIENT_MAX_WRITE_BUFFER_LENGTH
                                                                              : (length - offset);
    chunk_length = chunk_length > request->remaining ? request->remaining : chunk_length;
    if (chunk_length > 0) {
      status =
        sl_http_client_write_chunked_data(&http_client, (uint8_t *)(buffer + offset), chunk_length, request->flush_now);
      if (status != SL_STATUS_IN_PROGRESS && status != SL_STATUS_OK) {
        return status;
      }
      offset += chunk_length;
      request->remaining -= chunk_length;
    } else {
      break;
    }
    osDelay(20);
  }

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+http-client-mode=<mode>
sl_status_t http_client_mode_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (sl_si91x_is_device_initialized()) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  uint8_t mode = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);

  if (mode == 1) {
    si91x_init_configuration.boot_config.tcp_ip_feature_bit_map |= SL_SI91X_TCP_IP_FEAT_HTTP_CLIENT;
  } else if (mode == 0) {
    si91x_init_configuration.boot_config.tcp_ip_feature_bit_map &= ~SL_SI91X_TCP_IP_FEAT_HTTP_CLIENT;
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+net-cred-httpclient=<credential-id>,<username>,<password>
sl_status_t net_cred_httpclient_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  sl_net_credential_id_t http_cred_id =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_NET_INVALID_CREDENTIAL_ID, sl_net_credential_id_t);
  const char *username = GET_OPTIONAL_COMMAND_ARG(arguments, 1, HTTP_USERNAME_DEFAULT, const char *);
  const char *password = GET_OPTIONAL_COMMAND_ARG(arguments, 2, HTTP_PASSWORD_DEFAULT, const char *);

  if (http_cred_id == SL_NET_INVALID_CREDENTIAL_ID) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint32_t malloc_size = sizeof(sl_http_client_credentials_t) + strlen(username) + strlen(password);
  sl_http_client_credentials_t *http_client_credentials = calloc(1, malloc_size);
  SL_VERIFY_POINTER_OR_RETURN(http_client_credentials, SL_STATUS_ALLOCATION_FAILED);

  http_client_credentials->username_length = strlen(username);
  http_client_credentials->password_length = strlen(password);

  memcpy(&http_client_credentials->data[0], username, strlen(username));
  memcpy(&http_client_credentials->data[strlen(username)], password, strlen(password));

  sl_status_t status =
    sl_net_set_credential(http_cred_id, SL_NET_HTTP_CLIENT_CREDENTIAL, http_client_credentials, malloc_size);
  if (status != SL_STATUS_OK) {
    SL_CLEANUP_MALLOC(http_client_credentials);
    return status;
  }

  SL_CLEANUP_MALLOC(http_client_credentials);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+http-client-init=<certificate-index>,<tls-version>,<http-version>,
//                     <https-enable>,<https-use-sni>,<ip-version>,
//                     <network-interface>
sl_status_t http_client_init_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x3F);

  sl_http_client_configuration_t client_configuration = { 0 };

  client_configuration.certificate_index = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);
  client_configuration.tls_version = GET_OPTIONAL_COMMAND_ARG(arguments, 1, SL_TLS_V_1_2, sl_http_client_tls_version_t);
  client_configuration.http_version  = GET_OPTIONAL_COMMAND_ARG(arguments, 2, SL_HTTP_V_1_1, sl_http_client_version_t);
  client_configuration.https_enable  = GET_OPTIONAL_COMMAND_ARG(arguments, 3, false, bool);
  client_configuration.https_use_sni = GET_OPTIONAL_COMMAND_ARG(arguments, 4, false, bool);
  sl_ip_address_type_t ip_version    = GET_OPTIONAL_COMMAND_ARG(arguments, 5, SL_INVALID_IP, sl_ip_address_type_t);
  client_configuration.network_interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 6, SL_NET_WIFI_CLIENT_INTERFACE, sl_net_interface_t);

  if (ip_version == SL_INVALID_IP) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  client_configuration.ip_version = ip_version;

  sl_status_t status = sl_http_client_init(&client_configuration, &http_client);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%d\r\n", HTTP_CLIENT_DEFAULT_ID);

  return SL_STATUS_OK;
}

// at+http-client-deinit=<client-instance-id>
sl_status_t http_client_deinit_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  sl_status_t status = check_http_client_instance_id(arguments, 0);
  VERIFY_STATUS_AND_RETURN(status);

  status = sl_http_client_deinit(&http_client);
  VERIFY_STATUS_AND_RETURN(status);

  http_client = 0;
  cleanup_http_client_request(&http_request);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+http-client-req-conncfg=<client-instance-id>,<request-id>,<method-type>,
//                            <ip-address>,<host-name>,<sni-extension>,<port>,
//                            <resource>
sl_status_t http_client_req_conncfg_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0xCF);

  sl_status_t status = check_http_client_instance_id(arguments, 0);
  VERIFY_STATUS_AND_RETURN(status);

  uint32_t request_id = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint32_t);
  UNUSED_VARIABLE(request_id); // not currently supported

  sl_http_client_method_type_t http_method_type =
    GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, sl_http_client_method_type_t);
  const char *ip_address    = GET_OPTIONAL_COMMAND_ARG(arguments, 3, NULL, const char *);
  const char *host_name     = GET_OPTIONAL_COMMAND_ARG(arguments, 4, NULL, const char *);
  const char *sni_extension = GET_OPTIONAL_COMMAND_ARG(arguments, 5, NULL, const char *);
  uint16_t port             = GET_OPTIONAL_COMMAND_ARG(arguments, 6, HTTP_PORT, uint16_t);
  const char *resource      = GET_OPTIONAL_COMMAND_ARG(arguments, 7, NULL, const char *);

  if ((ip_address == NULL) || (resource == NULL)) {
    status = SL_STATUS_INVALID_PARAMETER;
    goto error;
  }

  http_client_request_t *request = &http_request;

  is_first_init_then_clean_up_request(request);

  request->client_request.http_method_type = http_method_type;

  // IP Address
  SL_CLEANUP_MALLOC(request->client_request.ip_address);
  size_t ip_len                      = strlen(ip_address) + 1;
  request->client_request.ip_address = calloc(1, ip_len);
  if (request->client_request.ip_address == NULL) {
    status = SL_STATUS_ALLOCATION_FAILED;
    goto error;
  }
  memcpy(request->client_request.ip_address, ip_address, ip_len);

  // Hostname
  SL_CLEANUP_MALLOC(request->client_request.host_name);
  if (host_name != NULL) { // Optional
    size_t len                        = strlen(host_name) + 1;
    request->client_request.host_name = calloc(1, len);
    if (request->client_request.host_name == NULL) {
      status = SL_STATUS_ALLOCATION_FAILED;
      goto error;
    }
    memcpy(request->client_request.host_name, host_name, len);
  }

  // SNI Extension
  SL_CLEANUP_MALLOC(request->client_request.sni_extension);
  if (sni_extension != NULL) { // Optional
    size_t sni_len                        = strlen(sni_extension);
    size_t sni_extension_size             = sizeof(sl_si91x_socket_type_length_value_t) + sni_len;
    request->client_request.sni_extension = calloc(1, sni_extension_size);
    if (request->client_request.sni_extension == NULL) {
      status = SL_STATUS_ALLOCATION_FAILED;
      goto error;
    }
    request->client_request.sni_extension->type   = SL_SI91X_TLS_EXTENSION_SNI_TYPE;
    request->client_request.sni_extension->length = sni_len;
    memcpy(request->client_request.sni_extension->value, sni_extension, sni_len);
  }

  request->client_request.port = port;

  // Resource
  SL_CLEANUP_MALLOC(request->client_request.resource);
  size_t resource_len              = strlen(resource) + 1;
  request->client_request.resource = calloc(1, resource_len);
  if (request->client_request.resource == NULL) {
    status = SL_STATUS_ALLOCATION_FAILED;
    goto error;
  }
  memcpy(request->client_request.resource, resource, resource_len);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;

error:
  SL_CLEANUP_MALLOC(request->client_request.ip_address);
  SL_CLEANUP_MALLOC(request->client_request.host_name);
  SL_CLEANUP_MALLOC(request->client_request.sni_extension);
  SL_CLEANUP_MALLOC(request->client_request.resource);

  return status;
}

// at+http-client-req-geturl=<client-instance-id>,<request-id>
sl_status_t http_client_req_geturl_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);

  sl_status_t status = check_http_client_instance_id(arguments, 0);
  VERIFY_STATUS_AND_RETURN(status);

  uint32_t request_id = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint32_t);
  UNUSED_VARIABLE(request_id); // not currently supported

  http_client_request_t *request = &http_request;

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("\"%s\"\r\n", request->client_request.resource ? (char *)request->client_request.resource : "");

  return SL_STATUS_OK;
}

// at+http-client-req-opt=<client-instance-id>,<request-id>,<timeout>,
//                        <retry-count>,<retry-period>,<tcp-connection-reuse>
sl_status_t http_client_req_opt_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x2F);

  sl_status_t status = check_http_client_instance_id(arguments, 0);
  VERIFY_STATUS_AND_RETURN(status);

  uint32_t request_id = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint32_t);
  UNUSED_VARIABLE(request_id); // not currently supported

  http_client_request_t *request = &http_request;

  request->client_request.timeout_ms           = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint16_t);
  request->client_request.retry_count          = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 0, uint16_t);
  request->client_request.retry_period_ms      = GET_OPTIONAL_COMMAND_ARG(arguments, 4, 0, uint16_t);
  request->client_request.tcp_connection_reuse = GET_OPTIONAL_COMMAND_ARG(arguments, 5, false, bool);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+http-client-req-body=<client-instance-id>,<request-id>,<body-length>
sl_status_t http_client_req_body_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x07);

  sl_status_t status = check_http_client_instance_id(arguments, 0);
  VERIFY_STATUS_AND_RETURN(status);

  uint32_t request_id = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint32_t);
  UNUSED_VARIABLE(request_id); // not currently supported

  uint32_t body_length = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint32_t);
  if (body_length == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  http_client_request_t *request = &http_request;

  is_first_init_then_clean_up_request(request);

  request->client_request.body_length = body_length;

  status = at_command_goto_data_mode(http_client_req_body_send_buffer_handler, body_length, (void *)request);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+http-client-req-body-len=<client-instance-id>,<request-id>,<body-length>
sl_status_t http_client_req_body_len_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x07);

  sl_status_t status = check_http_client_instance_id(arguments, 0);
  VERIFY_STATUS_AND_RETURN(status);

  uint32_t request_id = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint32_t);
  UNUSED_VARIABLE(request_id); // not currently supported

  uint32_t body_length = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint32_t);

  http_client_request_t *request = &http_request;

  is_first_init_then_clean_up_request(request);

  if (request->client_request.body != NULL) {
    if (body_length > request->client_request.body_length) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }

  request->remaining                  = body_length;
  request->client_request.body_length = body_length;

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+http-client-req-addhdr=<client-instance-id>,<request-id>,<key>,<value>
sl_status_t http_client_req_addhdr_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x0F);

  sl_status_t status = check_http_client_instance_id(arguments, 0);
  VERIFY_STATUS_AND_RETURN(status);

  uint32_t request_id = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint32_t);
  UNUSED_VARIABLE(request_id); // not currently supported

  http_client_request_t *request = &http_request;

  const char *key   = GET_OPTIONAL_COMMAND_ARG(arguments, 2, NULL, const char *);
  const char *value = GET_OPTIONAL_COMMAND_ARG(arguments, 3, NULL, const char *);
  if ((key == NULL) || (value == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = sl_http_client_add_header(&request->client_request, key, value);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+http-client-req-delhdr=<client-instance-id>,<request-id>,<key>
sl_status_t http_client_req_delhdr_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x07);

  sl_status_t status = check_http_client_instance_id(arguments, 0);
  VERIFY_STATUS_AND_RETURN(status);

  uint32_t request_id = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint32_t);
  UNUSED_VARIABLE(request_id); // not currently supported

  http_client_request_t *request = &http_request;

  const char *key = GET_OPTIONAL_COMMAND_ARG(arguments, 2, NULL, const char *);
  if (key == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = sl_http_client_delete_header(&request->client_request, key);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+http-client-req-delallhd=<client-instance-id>,<request-id>
sl_status_t http_client_req_delallhd_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);

  sl_status_t status = check_http_client_instance_id(arguments, 0);
  VERIFY_STATUS_AND_RETURN(status);

  uint32_t request_id = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint32_t);
  UNUSED_VARIABLE(request_id); // not currently supported

  http_client_request_t *request = &http_request;

  status = sl_http_client_delete_all_headers(&request->client_request);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+http-client-req-getallhd=<client-instance-id>,<request-id>
sl_status_t http_client_req_getallhd_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);

  sl_status_t status = check_http_client_instance_id(arguments, 0);
  VERIFY_STATUS_AND_RETURN(status);

  uint32_t request_id = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint32_t);
  UNUSED_VARIABLE(request_id); // not currently supported

  http_client_request_t *request = &http_request;

  // <count> <key1> <value1> <key2> <value2> ... <keyN> <valueN>
  // Count headers and build output string
  uint32_t header_count                = 0;
  char output[HTTP_OUTPUT_BUFFER_SIZE] = { 0 }; // Buffer to store headers
  char *buffer                         = output;
  int remaining_size                   = HTTP_OUTPUT_BUFFER_SIZE;

  sl_http_client_header_t *header = request->client_request.extended_header;
  while (header != NULL) {
    // Count headers
    header_count++;

    // Add header key and value to output
    int written = snprintf(buffer,
                           remaining_size,
                           " \"%s\" \"%s\"",
                           header->key ? header->key : "",
                           header->value ? header->value : "");
    if (written > 0 && written < remaining_size) {
      buffer += written;
      remaining_size -= written;
    } else {
      return SL_STATUS_WOULD_OVERFLOW;
    }

    header = (sl_http_client_header_t *)header->node.node;
  }

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%" PRIu32 "%s\r\n", header_count, output);

  return SL_STATUS_OK;
}

// at+http-client-req-send=<client-instance-id>,<request-id>
sl_status_t http_client_req_send_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);

  sl_status_t status = check_http_client_instance_id(arguments, 0);
  VERIFY_STATUS_AND_RETURN(status);

  uint32_t request_id            = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint32_t);
  http_client_request_t *request = &http_request;
  request->request_id            = request_id;

  status = sl_http_client_request_init(&request->client_request, http_response_callback_handler, (void *)request);
  VERIFY_STATUS_AND_RETURN(status);

  status = sl_http_client_send_request(&http_client, &request->client_request);

  // Mark as first init to clean up previous request data on next request configuration
  request->is_first_init = true;
  cleanup_sl_http_client_request(&request->client_request);

  if ((status == SL_STATUS_IN_PROGRESS) || (status == SL_STATUS_OK)) {
    PRINT_AT_CMD_SUCCESS;
    return SL_STATUS_OK;
  }
  return status;
}

// at+http-client-send-chunk=<client-instance-id>,<flush>,<chunk-size>
sl_status_t http_client_send_chunk_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x07);

  sl_status_t status = check_http_client_instance_id(arguments, 0);
  VERIFY_STATUS_AND_RETURN(status);

  bool flush_now      = GET_OPTIONAL_COMMAND_ARG(arguments, 1, false, bool);
  uint32_t chunk_size = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint32_t);

  if (chunk_size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  http_request.flush_now = flush_now;

  status = at_command_goto_data_mode(http_client_send_chunk_send_buffer_handler, chunk_size, (void *)&http_request);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}
