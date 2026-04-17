/***************************************************************************/ /**
 * @file
 * @brief Http Server Example Application
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "cmsis_os2.h"
#include "sl_net.h"
#include "sl_board_configuration.h"
#include "sl_net_ping.h"
#include "sl_utility.h"
#include "sl_si91x_driver.h"
#include "sl_wifi.h"
#include "sl_net_wifi_types.h"
#include "sl_net_default_values.h"
#include "sl_http_server.h"
#include <string.h>
#include <wifiuser.pem.h>

/******************************************************
 *                      Macros
 ******************************************************/
#define HTTP_SERVER_PORT 80

/******************************************************
 *               Variable Definitions
 ******************************************************/
const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};

sl_ip_address_t ip_address            = { 0 };
sl_net_wifi_client_profile_t profile  = { 0 };
volatile bool is_server_running       = false;
static char response[1025]            = { 0 };
static sl_http_server_t server_handle = { 0 };

static const sl_wifi_device_configuration_t http_server_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION | SL_WIFI_FEAT_WPS_DISABLE),
                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map =
                     (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID | SL_SI91X_CUSTOM_FEAT_SOC_CLK_CONFIG_160MHZ),
                   .ext_custom_feature_bit_map = (MEMORY_CONFIG
#ifdef SLI_SI917
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_WINDOW_DIV | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
};

sl_status_t buffered_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
sl_status_t large_data_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
sl_status_t large_response_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
sl_status_t chunked_large_response_handler(sl_http_server_t *handle, sl_http_server_request_t *req);

static sl_http_server_handler_t request_handlers[4] = { { .uri = "/test", .handler = buffered_request_handler },
                                                        { .uri = "/data", .handler = large_data_handler },
                                                        { .uri = "/cert1.pem", .handler = large_response_handler },
                                                        { .uri     = "/cert2.pem",
                                                          .handler = chunked_large_response_handler } };

static char *request_type[5] = { [SL_HTTP_REQUEST_GET]    = "GET",
                                 [SL_HTTP_REQUEST_POST]   = "POST",
                                 [SL_HTTP_REQUEST_PUT]    = "PUT",
                                 [SL_HTTP_REQUEST_DELETE] = "DELETE",
                                 [SL_HTTP_REQUEST_HEAD]   = "HEAD" };

/******************************************************
 *               Function Declarations
 ******************************************************/
static void application_start(void *argument);

sl_status_t large_response_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_recv_req_data_t recvData        = { 0 };
  sl_http_server_response_t http_response = { 0 };
  sl_http_header_t request_headers[5]     = { 0 };
  sl_http_header_t header                 = { .key = "Server", .value = "SI917-HTTPServer" };
  uint8_t *large_data                     = (uint8_t *)wifiuser;

  printf("Got request [%s] of type : %s with data length : %lu\n",
         req->uri.path,
         request_type[req->type],
         req->request_data_length);
  if (req->request_data_length > 0) {
    recvData.request       = req;
    recvData.buffer        = (uint8_t *)response;
    recvData.buffer_length = 1024;

    sl_http_server_read_request_data(handle, &recvData);
    response[recvData.received_data_length] = 0;
    printf("Got request data as : %s\n", response);
  }

  printf("Got request query parameter count : %u\n", req->uri.query_parameter_count);
  if (req->uri.query_parameter_count > 0) {
    for (int i = 0; i < req->uri.query_parameter_count; i++) {
      printf("query: %s, value: %s\n", req->uri.query_parameters[i].query, req->uri.query_parameters[i].value);
    }
  }

  printf("Got header count : %u\n", req->request_header_count);
  sl_http_server_get_request_headers(handle, req, request_headers, 5);

  int length = (req->request_header_count > 5) ? 5 : req->request_header_count;
  for (int i = 0; i < length; i++) {
    printf("Key: %s, Value: %s\n", request_headers[i].key, request_headers[i].value);
  }

  // Set the response code to 200 (OK)
  http_response.response_code = SL_HTTP_RESPONSE_OK;

  // Set the content type to plain text
  http_response.content_type = SL_HTTP_CONTENT_TYPE_TEXT_PLAIN;
  http_response.headers      = &header;
  http_response.header_count = 1;

  // Set the response data
  http_response.data                 = (uint8_t *)large_data;
  http_response.current_data_length  = strlen((const char *)large_data);
  http_response.expected_data_length = strlen((const char *)large_data);
  sl_http_server_send_response(handle, &http_response);

  is_server_running = false;
  return SL_STATUS_OK;
}

sl_status_t chunked_large_response_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_recv_req_data_t recvData        = { 0 };
  sl_http_server_response_t http_response = { 0 };
  sl_http_header_t request_headers[5]     = { 0 };
  sl_http_header_t header                 = { .key = "Server", .value = "SI917-HTTPServer" };
  uint8_t *large_data                     = (uint8_t *)wifiuser;

  printf("Got request [%s] of type : %s with data length : %lu\n",
         req->uri.path,
         request_type[req->type],
         req->request_data_length);
  if (req->request_data_length > 0) {
    recvData.request       = req;
    recvData.buffer        = (uint8_t *)response;
    recvData.buffer_length = 1024;

    sl_http_server_read_request_data(handle, &recvData);
    response[recvData.received_data_length] = 0;
    printf("Got request data as : %s\n", response);
  }

  printf("Got request query parameter count : %u\n", req->uri.query_parameter_count);
  if (req->uri.query_parameter_count > 0) {
    for (int i = 0; i < req->uri.query_parameter_count; i++) {
      printf("query: %s, value: %s\n", req->uri.query_parameters[i].query, req->uri.query_parameters[i].value);
    }
  }

  printf("Got header count : %u\n", req->request_header_count);
  sl_http_server_get_request_headers(handle, req, request_headers, 5);

  int length = (req->request_header_count > 5) ? 5 : req->request_header_count;
  for (int i = 0; i < length; i++) {
    printf("Key: %s, Value: %s\n", request_headers[i].key, request_headers[i].value);
  }

  // Set the response code to 200 (OK)
  http_response.response_code = SL_HTTP_RESPONSE_OK;

  // Set the content type to plain text
  http_response.content_type = SL_HTTP_CONTENT_TYPE_TEXT_PLAIN;
  http_response.headers      = &header;
  http_response.header_count = 1;

  // Set the response data to "Hello, World!"
  uint32_t data_length               = strlen((const char *)large_data);
  uint32_t tx_length                 = 1024;
  http_response.data                 = (uint8_t *)large_data;
  http_response.current_data_length  = tx_length;
  http_response.expected_data_length = strlen((const char *)large_data);
  sl_http_server_send_response(handle, &http_response);

  data_length = http_response.expected_data_length - tx_length;
  while (data_length > 0) {
    tx_length = (data_length > 1024 ? 1024 : data_length);
    http_response.data += tx_length;
    data_length -= tx_length;
    sl_http_server_write_data(handle, http_response.data, tx_length);
  }

  is_server_running = false;
  return SL_STATUS_OK;
}

sl_status_t buffered_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_recv_req_data_t recvData        = { 0 };
  sl_http_server_response_t http_response = { 0 };
  sl_http_header_t request_headers[5]     = { 0 };
  sl_http_header_t header                 = { .key = "Server", .value = "SI917-HTTPServer" };

  printf("Got request [%s] of type : %s with data length : %lu\n",
         req->uri.path,
         request_type[req->type],
         req->request_data_length);
  if (req->request_data_length > 0) {
    recvData.request       = req;
    recvData.buffer        = (uint8_t *)response;
    recvData.buffer_length = 1024;

    sl_http_server_read_request_data(handle, &recvData);
    response[recvData.received_data_length] = 0;
    printf("Got request data as : %s\n", response);
  }

  printf("Got request query parameter count : %u\n", req->uri.query_parameter_count);
  if (req->uri.query_parameter_count > 0) {
    for (int i = 0; i < req->uri.query_parameter_count; i++) {
      printf("query: %s, value: %s\n", req->uri.query_parameters[i].query, req->uri.query_parameters[i].value);
    }
  }

  printf("Got header count : %u\n", req->request_header_count);
  sl_http_server_get_request_headers(handle, req, request_headers, 5);

  int length = (req->request_header_count > 5) ? 5 : req->request_header_count;
  for (int i = 0; i < length; i++) {
    printf("Key: %s, Value: %s\n", request_headers[i].key, request_headers[i].value);
  }

  // Set the response code to 200 (OK)
  http_response.response_code = SL_HTTP_RESPONSE_OK;

  // Set the content type to plain text
  http_response.content_type = SL_HTTP_CONTENT_TYPE_TEXT_PLAIN;
  http_response.headers      = &header;
  http_response.header_count = 1;

  // Set the response data to "Hello, World!"
  char *response_data                = "Hello, World!";
  http_response.data                 = (uint8_t *)response_data;
  http_response.current_data_length  = strlen(response_data);
  http_response.expected_data_length = http_response.current_data_length;
  sl_http_server_send_response(handle, &http_response);
  is_server_running = false;
  return SL_STATUS_OK;
}

sl_status_t large_data_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = { 0 };
  sl_http_recv_req_data_t recvData        = { 0 };
  uint32_t data_length                    = 0;

  printf("Got request [%s] of type : %s with data length : %lu\n",
         req->uri.path,
         request_type[req->type],
         req->request_data_length);
  if (req->request_data_length > 0) {
    recvData.request       = req;
    recvData.buffer        = (uint8_t *)response;
    recvData.buffer_length = 1024;
    data_length            = req->request_data_length;

    while (0 != data_length) {
      sl_http_server_read_request_data(handle, &recvData);
      data_length -= recvData.received_data_length;
      printf("Read %lu bytes, remaining %lu bytes\n", recvData.received_data_length, data_length);
    }
  }

  // Set the response code to 200 (OK)
  http_response.response_code = SL_HTTP_RESPONSE_OK;

  // Set the content type to plain text
  http_response.content_type = SL_HTTP_CONTENT_TYPE_TEXT_PLAIN;

  // Set the response data to "Hello, World!"
  char *response_data                = "Done";
  http_response.data                 = (uint8_t *)response_data;
  http_response.current_data_length  = strlen(response_data);
  http_response.expected_data_length = http_response.current_data_length;
  sl_http_server_send_response(handle, &http_response);
  is_server_running = false;

  return SL_STATUS_OK;
}

/******************************************************
 *               Function Definitions
 ******************************************************/
void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status                    = 0;
  sl_http_server_config_t server_config = { 0 };

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &http_server_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to start Wi-Fi Client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client interface init success\r\n");

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring Wi-Fi client interface up: 0x%lX\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client connected\r\n");

  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile);
  if (status != SL_STATUS_OK) {
    printf("Failed to get client profile: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nSuccess to get client profile\r\n");

  ip_address.type = SL_IPV4;
  memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
  print_sl_ip_address(&ip_address);

  server_config.port             = HTTP_SERVER_PORT;
  server_config.default_handler  = NULL;
  server_config.handlers_list    = request_handlers;
  server_config.handlers_count   = 4;
  server_config.client_idle_time = 1;

  status = sl_http_server_init(&server_handle, &server_config);
  if (status != SL_STATUS_OK) {
    printf("\r\nHTTP server init failed:%lx\r\n", status);
    return;
  }
  printf("\r\n Http Init done\r\n");

  status = sl_http_server_start(&server_handle);
  if (status != SL_STATUS_OK) {
    printf("\r\n Server start fail:%lx\r\n", status);
    return;
  }
  printf("\r\n Server start done\r\n");

  is_server_running = true;
  while (is_server_running) {
    osDelay(100);
  }

  status = sl_http_server_stop(&server_handle);
  if (status != SL_STATUS_OK) {
    printf("\r\n Server stop fail:%lx\r\n", status);
    return;
  }
  printf("\r\n Server stop done\r\n");

  status = sl_http_server_deinit(&server_handle);
  if (status != SL_STATUS_OK) {
    printf("\r\n Server deinit fail:%lx\r\n", status);
    return;
  }
  printf("\r\n Server deinit done\r\n");
}
