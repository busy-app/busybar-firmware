/***************************************************************************/ /**
 * @file
 * @brief WebSocket Client Example Application
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_websocket_client_types.h"
#include "sl_websocket_client.h"
#include "cacert.pem.h"
#include <string.h>

/******************************************************
 *                      Macros
 ******************************************************/
#ifdef SLI_SI91X_ENABLE_IPV6
#define SERVER_IP_ADDR "fd7f:c929:e748::3df"
#else
#define SERVER_IP_ADDR "192.168.29.160"
#endif

#define HOST_NAME         "example.com"
#define RESOURCE_NAME     "/myresource"
#define CERTIFICATE_INDEX 0
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

static sl_websocket_client_t ws_handle = { 0 };
volatile bool is_ping_received         = false;

static const sl_wifi_device_configuration_t websocket_client_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION | SL_WIFI_FEAT_WPS_DISABLE),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_SSL
                                              | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID
#ifdef SLI_SI91X_ENABLE_IPV6
                                              | SL_SI91X_TCP_IP_FEAT_DHCPV6_CLIENT | SL_SI91X_TCP_IP_FEAT_IPV6
#endif
                                              ),
                   .custom_feature_bit_map =
                     (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID | SL_SI91X_CUSTOM_FEAT_SOC_CLK_CONFIG_160MHZ),
                   .ext_custom_feature_bit_map = (MEMORY_CONFIG
#ifdef SLI_SI917
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_WINDOW_DIV | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID
                      | SL_SI91X_EXT_TCP_IP_FEAT_SSL_THREE_SOCKETS | SL_SI91X_EXT_TCP_IP_WAIT_FOR_SOCKET_CLOSE),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
};

/******************************************************
 *               Function Declarations
 ******************************************************/
static void application_start(void *argument);
sl_websocket_error_t create_and_send_websocket_data(void);
/******************************************************
 *               Function Definitions
 ******************************************************/
void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

void data_callback(uint32_t sock_no,
                   uint8_t *buffer,
                   uint32_t length,
                   const sl_si91x_socket_metadata_t *firmware_socket_response)
{
  UNUSED_PARAMETER(sock_no);
  sl_websocket_opcode_t opcode = sl_si91x_get_opcode_from_socket_id(firmware_socket_response->socket_id);

  if (opcode == SL_WEBSOCKET_OPCODE_PING) {
    printf("Ping response received, sending Pong ");
    // Send the pong response
    sl_websocket_send_request_t send_request;
    send_request.opcode           = SL_WEBSOCKET_OPCODE_PONG | SL_WEBSOCKET_FIN_BIT;
    send_request.buffer           = buffer;
    send_request.length           = length;
    sl_websocket_error_t ws_error = sl_websocket_send_frame(&ws_handle, &send_request);
    if (ws_error != SL_WEBSOCKET_SUCCESS) {
      printf("\r\nError while sending pong frame:%d", ws_error);
      return;
    }
    printf("\r\nPong Send success");
    //After sending the pong response we are closing the socket in this application
    is_ping_received = true;
    return;
  } else if (opcode == SL_WEBSOCKET_OPCODE_CLOSE) {
    // Validate the length
    if (length < 2) {
      printf("Close frame received with insufficient length. No status code or reason available.\r\n");
      return;
    }

    // Extract status code from the buffer
    uint16_t status_code = (buffer[0] << 8) | buffer[1];

    // Check if there is a reason text
    if (length == 2) {
      printf("Close frame received. Status code: %d, No reason available.\r\n", status_code);
      return;
    }

    // Extract reason from the buffer
    char *reason = (char *)malloc(length - 2 + 1); // +1 for null-terminator
    if (reason == NULL) {
      printf("Memory allocation failed for reason text.\r\n");
      return;
    }
    memcpy(reason, buffer + 2, length - 2);
    reason[length - 2] = '\0';

    // Print the status code and reason
    printf("Close frame received. Status code: %d, Reason: %s\r\n", status_code, reason);

    // Free the allocated memory
    free(reason);
    return;
  }

  // Print data as characters
  printf("\r\nData Received: ");
  for (uint32_t i = 0; i < length; i++) {
    printf("%c", buffer[i]);
  }
  printf("\r\n");
}

void remote_terminate_callback(int socket_id, uint16_t port_number, uint32_t bytes_sent)
{
  printf("Remote client terminated on socket %d, port %d , bytes_sent %ld\r\n", socket_id, port_number, bytes_sent);
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status                   = 0;
  sl_ip_address_t ip_address           = { 0 };
  sl_net_wifi_client_profile_t profile = { 0 };

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &websocket_client_configuration, NULL, NULL);
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

  if (profile.ip.type == SL_IPV4) {
    ip_address.type = SL_IPV4;
    memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
    print_sl_ip_address(&ip_address);
  } else if (profile.ip.type == SL_IPV6) {
    sl_ip_address_t link_local_address = { 0 };
    memcpy(&link_local_address.ip.v6, &profile.ip.ip.v6.link_local_address, SL_IPV6_ADDRESS_LENGTH);
    link_local_address.type = SL_IPV6;
    printf("Link Local Address: ");
    print_sl_ip_address(&link_local_address);

    sl_ip_address_t global_address = { 0 };
    memcpy(&global_address.ip.v6, &profile.ip.ip.v6.global_address, SL_IPV6_ADDRESS_LENGTH);
    global_address.type = SL_IPV6;
    printf("Global Address: ");
    print_sl_ip_address(&global_address);

    sl_ip_address_t gateway = { 0 };
    memcpy(&gateway.ip.v6, &profile.ip.ip.v6.gateway, SL_IPV6_ADDRESS_LENGTH);
    gateway.type = SL_IPV6;
    printf("Gateway Address: ");
    print_sl_ip_address(&gateway);
  }

  sl_websocket_error_t ws_error = create_and_send_websocket_data();
  if (ws_error != SL_WEBSOCKET_SUCCESS) {
    printf("\r\ncreate_and_send_websocket_data failed with error:%d\r\n", ws_error);
    return;
  }
  printf("\r\nExample Demonstration Completed\r\n");
}

sl_websocket_error_t create_and_send_websocket_data(void)
{
  sl_websocket_config_t ws_config = {
    .host                = HOST_NAME,
    .resource            = RESOURCE_NAME,
    .server_port         = 8080,
    .client_port         = 5001,
    .ip_address          = SERVER_IP_ADDR,
    .data_cb             = data_callback,
    .remote_terminate_cb = remote_terminate_callback,
    .enable_ssl          = false,
  };

  // Load SSL CA certificate only when SSL is enabled
  if (ws_config.enable_ssl) {
    sl_status_t status = sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(CERTIFICATE_INDEX),
                                               SL_NET_SIGNING_CERTIFICATE,
                                               cacert,
                                               sizeof(cacert) - 1);
    if (status != SL_STATUS_OK) {
      printf("\r\nLoading TLS CA certificate into FLASH Failed, Error Code : 0x%lX\r\n", status);
      return SL_WEBSOCKET_ERR_SSL_SETSOCKOPT;
    }
    printf("\r\nLoad SSL CA certificate at index %d Success\r\n", CERTIFICATE_INDEX);
  }

  sl_websocket_error_t ws_error = sl_websocket_init(&ws_handle, &ws_config);
  if (ws_error != SL_WEBSOCKET_SUCCESS) {
    printf("\r\nError in sl_websocket_init:%d", ws_error);
    return ws_error;
  }
  printf("\r\nWebSocket Init done");

  ws_error = sl_websocket_connect(&ws_handle);
  if (ws_error != SL_WEBSOCKET_SUCCESS) {
    printf("\r\nError while websocket connect:%d", ws_error);
    return ws_error;
  }
  printf("\r\nWebSocket connect Success");

  const char *data = "hello server";

  // Create and populate the send_request structure
  sl_websocket_send_request_t send_request;
  send_request.opcode = SL_WEBSOCKET_OPCODE_TEXT | SL_WEBSOCKET_FIN_BIT;
  send_request.buffer = (const uint8_t *)data;
  send_request.length = strlen(data);
  int count           = 5;
  while (count > 0) {
    ws_error = sl_websocket_send_frame(&ws_handle, &send_request);
    if (ws_error != SL_WEBSOCKET_SUCCESS) {
      printf("\r\nError while websocket send frame:%d", ws_error);
      break;
    }
    count--;
  }

  is_ping_received = false;
  while (!is_ping_received) {
    osDelay(100);
  }

  ws_error = sl_websocket_close(&ws_handle);
  if (ws_error != SL_WEBSOCKET_SUCCESS) {
    printf("\r\nError while websocket close:%d", ws_error);
    return ws_error;
  }
  printf("\r\nWebSocket close success");

  ws_error = sl_websocket_deinit(&ws_handle);
  if (ws_error != SL_WEBSOCKET_SUCCESS) {
    printf("\r\nError while websocket deinit:%d", ws_error);
    return ws_error;
  }
  printf("\r\nWebSocket deinit success");

  return SL_WEBSOCKET_SUCCESS;
}