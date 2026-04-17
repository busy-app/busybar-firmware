/***************************************************************************/ /**
 * @file
 * @brief Dual Network Stack Example Application
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

/*================================================================================
 * @brief : This file contains example application for Dual Network Stack

 * @section Description:
 * This application demonstrates how to use the Dual Network Stack feature i.e. to use
 * offload network stack and hosted LWIP network stack on M4 interfaces simultaneously.
 * 
 * The application creates a Wi-Fi client connection and enables the offload network stack and
 * hosted LWIP network stack interfaces. Sends a ping request to a remote server using the offload network stack 
 * and receives TCP packets from a remote server to the hosted LWIP network stack interface.
 =================================================================================*/

#include "cmsis_os2.h"
#include "sl_net.h"
#include "sl_board_configuration.h"
#include "sl_net_ping.h"
#include "sl_utility.h"
#include "sl_si91x_driver.h"
#include "sl_wifi.h"
#include "sl_net_wifi_types.h"
#include "sl_net_default_values.h"
#include <string.h>

#include "lwip/errno.h"
#include "lwip/sockets.h"
#include "sl_net_for_lwip.h"
#include "sl_si91x_socket.h"

/******************************************************
 *                    Constants
******************************************************/

#define HOST_DATA_TRANSFER 1 // Set to 1 to enable host data transfer, or 0 to disable it.

#define LISTENING_PORT 5005
#define SERVER_PORT    5004

#define HOST_LISTENING_PORT 5002
#define HOST_SERVER_PORT    5001

#define BUFFER_SIZE     512
#define TCP_BUFFER_SIZE 1460

#define BACKLOG          1
#define TEST_TIMEOUT     20000
#define BYTES_TO_SEND    (1 << 29) // 512MB
#define BYTES_TO_RECEIVE (1 << 28) // 256MB
#define SERVER_IP        "192.168.0.206"

/******************************************************
        *               Variable Definitions
        ******************************************************/
uint8_t data_buffer[BUFFER_SIZE];

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

const osThreadAttr_t traffic_thread_attributes = {
  .name       = "traffic_thread",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 2048,
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};

static const sl_wifi_device_configuration_t client_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
#ifdef SLI_SI91X_MCU_INTERFACE
                      | SL_WIFI_FEAT_WPS_DISABLE
#endif
                      ),
                   .tcp_ip_feature_bit_map     = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_ICMP | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map     = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map = (SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK
                                                  | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
#ifdef SLI_SI917
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID | SL_SI91X_EXT_TCP_IP_DUAL_MODE_ENABLE),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
};

static sl_net_wifi_lwip_context_t wifi_client_context;

/******************************************************
        *               Function Declarations
        ******************************************************/
static void application_start(void *argument);

/******************************************************
        *               Function Definitions
        ******************************************************/

volatile uint8_t has_data_received = 0;
volatile uint32_t bytes_read       = 0;
uint8_t first_data_frame           = 1;
uint32_t start                     = 0;
uint32_t now                       = 0;

// Define the mode variables
int TCP_MODE = 0; // 1 for TCP, 0 for UDP
int TX_MODE  = 0; // 0 for Receive, 1 for Transmit

void data_callback(uint32_t sock_no,
                   uint8_t *buffer,
                   uint32_t length,
                   const sl_si91x_socket_metadata_t *firmware_socket_response)
{
  UNUSED_PARAMETER(sock_no);
  UNUSED_PARAMETER(buffer);
  UNUSED_PARAMETER(firmware_socket_response);

  if (first_data_frame) {
    start = osKernelGetTickCount();
    if (!TCP_MODE && !TX_MODE) {
      printf("\r\nOffload UDP_RX test start\r\n");
    } else {
      printf("\r\nOffload TCP_RX test start\r\n");
    }
    first_data_frame = 0;
  }

  bytes_read += length;
  now = osKernelGetTickCount();
  if ((bytes_read > BYTES_TO_RECEIVE) || ((now - start) > TEST_TIMEOUT)) {
    has_data_received = 1;
  }
}

void send_data_to_tcp_server()
{
  int client_socket                 = -1;
  uint32_t total_bytes_sent         = 0;
  int socket_return_value           = 0;
  int sent_bytes                    = 1;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  server_address.sin_family         = AF_INET;
  server_address.sin_port           = SERVER_PORT;
  sl_net_inet_addr(SERVER_IP, &server_address.sin_addr.s_addr);

  client_socket = sl_si91x_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_socket < 0) {
    printf("\r\nSocket Create failed with bsd error: %d\r\n", errno);
    return;
  }
  printf("\r\nSocket ID : %d\r\n", client_socket);

  socket_return_value = sl_si91x_connect(client_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    printf("\r\nSocket Connect failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }
  printf("\r\nSocket connected to TCP server\r\n");

  printf("\r\nOffload TCP_TX data transfer test start\r\n");
  start = osKernelGetTickCount();
  while (total_bytes_sent < BYTES_TO_SEND) {
    sent_bytes = sl_si91x_send(client_socket, data_buffer, TCP_BUFFER_SIZE, 0);
    now        = osKernelGetTickCount();
    if (sent_bytes < 0) {
      if (errno == ENOBUFS)
        continue;
      printf("\r\nSocket send failed with bsd error: %d\r\n", errno);
      close(client_socket);
      break;
    }
    total_bytes_sent = total_bytes_sent + sent_bytes;

    if ((now - start) > TEST_TIMEOUT) {
      printf("\r\nTime Out: %ld\r\n", (now - start));
      break;
    }
  }
  printf("\r\nOffload TCP_TX data transfer test finished\r\n");
  printf("\r\nTotal bytes sent : %ld\r\n", total_bytes_sent);

  close(client_socket);
}

void receive_data_from_tcp_client()
{
  int server_socket = -1;
  int client_socket = -1;

  int socket_return_value = 0;

  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);

  server_socket = sl_si91x_socket_async(AF_INET, SOCK_STREAM, IPPROTO_TCP, &data_callback);
  if (server_socket < 0) {
    printf("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }

  printf("\r\nServer Socket ID : %d\r\n", server_socket);

  if (socket_return_value < 0) {
    printf("\r\nSet Socket option failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }

  server_address.sin_family      = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port        = LISTENING_PORT;

  socket_return_value = sl_si91x_bind(server_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    printf("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }

  socket_return_value = sl_si91x_listen(server_socket, BACKLOG);
  if (socket_return_value < 0) {
    printf("\r\nSocket listen failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }

  printf("\r\nOffload Stack: listening on local port : %d\r\n", LISTENING_PORT);

  socklen_t addr_len = 0;
  client_socket      = sl_si91x_accept(server_socket, NULL, addr_len);
  if (client_socket < 0) {
    printf("\r\nSocket accept failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }

  printf("\r\nClient Socket ID : %d\r\n", client_socket);

  while (!has_data_received) {
    osThreadYield();
  }

  now = osKernelGetTickCount();

  printf("\r\nOffload TCP_RX test finished\r\n");
  printf("\r\nTotal bytes received : %ld\r\n", bytes_read);

  sl_si91x_shutdown(server_socket, 0);
  sl_si91x_shutdown(client_socket, 0);
}

void send_data_to_udp_server()
{
  int client_socket                 = -1;
  uint32_t total_bytes_sent         = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  int sent_bytes                    = 1;
  server_address.sin_family         = AF_INET;
  server_address.sin_port           = SERVER_PORT;
  sl_net_inet_addr(SERVER_IP, &server_address.sin_addr.s_addr);

  client_socket = sl_si91x_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (client_socket < 0) {
    printf("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  printf("\r\nSocket ID : %d\r\n", client_socket);

  printf("\r\nOffload UDP_TX data transfer test start\r\n");
  start = osKernelGetTickCount();
  while (total_bytes_sent < BYTES_TO_SEND) {
    sent_bytes =
      sl_si91x_sendto(client_socket, data_buffer, 1470, 0, (struct sockaddr *)&server_address, socket_length);
    now = osKernelGetTickCount();
    if ((now - start) > TEST_TIMEOUT) {
      printf("\r\nTime Out: %ld\r\n", (now - start));
      break;
    }
    if (sent_bytes < 0) {
      if (errno == ENOBUFS)
        continue;
      printf("\r\nSocket send failed with bsd error: %d\r\n", errno);
      close(client_socket);
      break;
    }
    total_bytes_sent = total_bytes_sent + sent_bytes;
  }
  printf("\r\nOffload UDP_TX data transfer test finished\r\n");
  printf("\r\nTotal bytes sent : %ld\r\n", total_bytes_sent);

  close(client_socket);
}

void receive_data_from_udp_client()
{
  int client_socket = -1;

  int socket_return_value           = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);

  client_socket = sl_si91x_socket_async(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &data_callback);
  if (client_socket < 0) {
    printf("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  printf("\r\nSocket ID : %d\r\n", client_socket);

  server_address.sin_family = AF_INET;
  server_address.sin_port   = LISTENING_PORT;

  socket_return_value = sl_si91x_bind(client_socket, (struct sockaddr *)&server_address, socket_length);

  if (socket_return_value < 0) {
    printf("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }
  printf("\r\nOffload Stack: Listening on Local Port %d\r\n", LISTENING_PORT);

  while (!has_data_received) {
    osThreadYield();
  }
  now = osKernelGetTickCount();
  printf("\r\nOffload UDP_RX Async test finished\r\n");
  printf("\r\nTotal bytes received : %ld\r\n", bytes_read);

  sl_si91x_shutdown(client_socket, 0);
}

// Function for offload data transfer
void send_and_receive_offload_data_traffic()
{
  if (TCP_MODE) {
    if (TX_MODE) {
      send_data_to_tcp_server();
    } else {
      receive_data_from_tcp_client();
    }
  } else {
    if (TX_MODE) {
      send_data_to_udp_server();
    } else {
      receive_data_from_udp_client();
    }
  }
}

#if HOST_DATA_TRANSFER
void send_lwip_data_to_tcp_server()
{
  int client_socket = -1;

  uint32_t total_bytes_sent = 0;
  int socket_return_value   = 0;
  int sent_bytes            = 1;

  uint32_t fail = 0;
  uint32_t pass = 0;

  uint32_t start = 0;
  uint32_t now   = 0;

  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);

  server_address.sin_family = AF_INET;
  server_address.sin_port   = htons(HOST_SERVER_PORT);

  sl_net_inet_addr(SERVER_IP, &server_address.sin_addr.s_addr);

  client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_socket < 0) {
    printf("\r\nSocket Create failed with bsd error: %d\r\n", errno);
    return;
  }

  printf("\r\nSocket ID : %d\r\n", client_socket);

  socket_return_value = connect(client_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    printf("\r\nSocket Connect failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }

  printf("\r\nSocket connected to TCP server\r\n");
  printf("\r\nHost TCP_TX data transfer test start\r\n");

  start = osKernelGetTickCount();

  do {
    sent_bytes = send(client_socket, data_buffer, BUFFER_SIZE, 0);
    now        = osKernelGetTickCount();

    if (sent_bytes < 0) {
      if (errno == ENOBUFS)
        continue;
      fail++;
    } else {
      pass++;
      total_bytes_sent = total_bytes_sent + sent_bytes;
    }
  } while ((total_bytes_sent < BYTES_TO_SEND) && ((now - start) < TEST_TIMEOUT));

  printf("\r\nHost TCP_TX data transfer test finished\r\n");
  printf("\r\nTotal bytes sent : %ld\r\n", total_bytes_sent);
  printf("\r\nSend fail count : %ld, Send pass count : %ld\r\n", fail, pass);

  close(client_socket);
}

void receive_lwip_data_from_tcp_client()
{
  int server_socket = -1;
  int client_socket = -1;

  int read_bytes                = 0;
  int socket_return_value       = 0;
  uint32_t total_bytes_received = 0;

  uint32_t start = 0;
  uint32_t now   = 0;

  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);

  server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_socket < 0) {
    printf("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }

  printf("\r\nServer Socket ID : %d\r\n", server_socket);

  if (socket_return_value < 0) {
    printf("\r\nSet Socket option failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }

  server_address.sin_family      = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port        = htons(HOST_LISTENING_PORT);

  socket_return_value = bind(server_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    printf("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }

  socket_return_value = listen(server_socket, BACKLOG);
  if (socket_return_value < 0) {
    printf("\r\nSocket listen failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }

  printf("\r\nHost Stack: Listening on Local Port : %d\r\n", HOST_LISTENING_PORT);

  client_socket = accept(server_socket, NULL, NULL);
  if (client_socket < 0) {
    printf("\r\nSocket accept failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }

  printf("\r\nClient Socket ID : %d\r\n", client_socket);

  printf("\r\nHost TCP_RX test start\r\n");
  start = osKernelGetTickCount();

  do {
    read_bytes = recv(client_socket, data_buffer, BUFFER_SIZE, 0);

    if (read_bytes < 0) {
      printf("\r\nReceive failed with bsd error:%d\r\n", errno);
      close(client_socket);
      close(server_socket);
      return;
    }

    total_bytes_received = total_bytes_received + read_bytes;
    now                  = osKernelGetTickCount();
  } while ((read_bytes > 0) && ((now - start) < TEST_TIMEOUT));

  printf("\r\nHost TCP_RX test finished\r\n");
  printf("\r\nTotal bytes received : %ld\r\n", total_bytes_received);

  close(client_socket);
  close(server_socket);
}

void send_lwip_data_to_udp_server()
{
  int client_socket = -1;
  int sent_bytes    = 1;

  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);

  uint32_t total_bytes_sent = 0;
  uint32_t fail             = 0;
  uint32_t pass             = 0;

  uint32_t start = 0;
  uint32_t now   = 0;

  server_address.sin_family = AF_INET;
  server_address.sin_port   = htons(HOST_SERVER_PORT);

  sl_net_inet_addr(SERVER_IP, &server_address.sin_addr.s_addr);

  client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (client_socket < 0) {
    printf("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }

  printf("\r\nSocket ID : %d\r\n", client_socket);
  printf("\r\nHost UDP_TX data transfer test start\r\n");

  start = osKernelGetTickCount();

  do {
    sent_bytes = sendto(client_socket, data_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_address, socket_length);
    now        = osKernelGetTickCount();

    if (sent_bytes < 0) {
      if (errno == ENOBUFS)
        continue;
      fail++;
    } else {
      pass++;
      total_bytes_sent = total_bytes_sent + sent_bytes;
    }
  } while (total_bytes_sent < BYTES_TO_SEND && (now - start) < TEST_TIMEOUT);

  printf("\r\nHost UDP_TX data transfer test finished\r\n");
  printf("\r\nTotal bytes sent : %ld\r\n", total_bytes_sent);
  printf("\r\nSend fail count : %ld, Send pass count : %ld\r\n", fail, pass);

  close(client_socket);
}

void receive_lwip_data_from_udp_client()
{
  int client_socket = -1;

  int socket_return_value           = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);

  uint32_t start = 0;
  uint32_t now   = 0;

  int read_bytes                = 0;
  uint32_t total_bytes_received = 0;

  client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (client_socket < 0) {
    printf("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }

  printf("\r\nSocket ID : %d\r\n", client_socket);

  server_address.sin_family      = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port        = htons(HOST_LISTENING_PORT);

  socket_return_value = bind(client_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    printf("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }

  printf("\r\nOffload Stack: Listening on Local Port %d\r\n", HOST_LISTENING_PORT);
  printf("\r\nHost UDP_RX test start\r\n");

  start = osKernelGetTickCount();

  do {
    read_bytes = recvfrom(client_socket, data_buffer, BUFFER_SIZE, 0, NULL, NULL);
    if (read_bytes < 0) {
      printf("\r\nReceive failed with bsd error: %d\r\n", errno);
      close(client_socket);
      return;
    }

    total_bytes_received = total_bytes_received + read_bytes;
    now                  = osKernelGetTickCount();
  } while (total_bytes_received < BYTES_TO_RECEIVE && (now - start) < TEST_TIMEOUT);

  printf("\r\nHost UDP_RX test finished\r\n");
  printf("\r\nTotal bytes received : %ld\r\n", total_bytes_received);

  close(client_socket);
}

// Thread function for lwip data transfer
void lwip_data_traffic_thread(void *argument)
{
  UNUSED_PARAMETER(argument);
  if (TCP_MODE) {
    if (TX_MODE) {
      send_lwip_data_to_tcp_server();
    } else {
      receive_lwip_data_from_tcp_client();
    }
  } else {
    if (TX_MODE) {
      send_lwip_data_to_udp_server();
    } else {
      receive_lwip_data_from_udp_client();
    }
  }

  osThreadTerminate(osThreadGetId());
}
#endif

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;
  sl_net_wifi_client_profile_t profile = { 0 };
  sl_ip_address_t ip_address           = { 0 };

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &client_configuration, &wifi_client_context, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to start Wi-Fi Client interface: 0x%lX\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client interface initialization is successful\r\n");

  printf("\r\nWi-Fi client interface bring up is in progress\r\n");
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

#if HOST_DATA_TRANSFER
  osThreadId_t lwip_thread_id = NULL;

  // Run lwip data traffic in a thread
  lwip_thread_id = osThreadNew((osThreadFunc_t)lwip_data_traffic_thread, NULL, &traffic_thread_attributes);
  if (lwip_thread_id == NULL) {
    printf("Failed to create lwip thread.\n");
    return;
  }
#endif

  // Run offload data traffic
  send_and_receive_offload_data_traffic();
  osThreadSuspend(osThreadGetId());
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}
