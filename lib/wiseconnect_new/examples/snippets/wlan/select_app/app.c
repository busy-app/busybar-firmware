/***************************************************************************/ /**
 * @file
 * @brief Select App Example Application
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
#include "sl_wifi_types.h"
#include "errno.h"
#include "sl_net.h"
#include "socket.h"
#include "sl_utility.h"
#include "sl_net_si91x.h"
#include "sl_wifi_callback_framework.h"
#include "select.h"
#include "string.h"
#include "sl_net_wifi_types.h"
#include "sl_si91x_core_utilities.h"

/******************************************************
 *                    Constants
 ******************************************************/
#define RECV_BUFFER_SIZE 500
#define DEVICE_PORT      5001
#define BACKLOG          1
#define SELECT_TIME_OUT  30 // In seconds

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

/******************************************************
 *               Function Declarations
 ******************************************************/
static void application_start(void *argument);
void socket_select();

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
  // Initialize the network interface
  sl_status_t status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, NULL, NULL, NULL);

  sl_ip_address_t ip_address           = { 0 };
  sl_net_wifi_client_profile_t profile = { 0 };

  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client interface up success\r\n");

  // Bring up network interface
  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("Failed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
    return;
  }

  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile);
  if (status != SL_STATUS_OK) {
    printf("Failed to get client profile: 0x%lx\r\n", status);
    return;
  }

  printf("\r\nSuccess to get client profile\r\n");

  ip_address.type = SL_IPV4;
  memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
  print_sl_ip_address(&ip_address);

  printf("Wi-Fi client connected\r\n");

  socket_select();
}

void close_sockets(int client_socket, int server_socket)
{
  if (client_socket > 0)
    close(client_socket);

  if (server_socket > 0)
    close(server_socket);

  return;
}

void socket_select()
{
  int server_socket = -1;
  int client_socket = -1;

  int highest_socket_number = 1;
  int total_set_fds_count   = 0;
  int socket_return_value   = 0;

  uint8_t recv_buffer[RECV_BUFFER_SIZE];

  fd_set read_fds;

  struct sockaddr_in server_address        = { 0 };
  socklen_t socket_length                  = sizeof(struct sockaddr_in);
  struct sockaddr_in remote_socket_address = { 0 };
  struct timeval timeout                   = { 0 };
  timeout.tv_sec                           = SELECT_TIME_OUT;

  server_address.sin_family = AF_INET;
  server_address.sin_port   = DEVICE_PORT;

  // Create socket
  server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  printf("\n server socket: %d", server_socket);
  if (server_socket < 0) {
    printf("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  printf("\r\nSocket creation Success\r\n");

  // Bind socket
  socket_return_value = bind(server_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    printf("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }
  printf("\r\n Socket bind success \r\n");

  // Socket listen
  socket_return_value = listen(server_socket, BACKLOG);
  if (socket_return_value < 0) {
    printf("\r\nSocket listen failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }
  printf("\r\n Socket listening on port = %d\r\n", DEVICE_PORT);

  //socket accept
  client_socket = accept(server_socket, (struct sockaddr *)&remote_socket_address, &socket_length);
  printf("\r\n Client socket = %d\r\n", client_socket);

  if (client_socket < 0) {
    printf("\r\nSocket accept failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }

  FD_ZERO(&read_fds);

  for (uint8_t socket = 0; socket <= highest_socket_number; socket++) {
    FD_SET(socket, &read_fds);
  }

  total_set_fds_count = select(highest_socket_number + 1, &read_fds, NULL, NULL, &timeout);

  if (total_set_fds_count == -1) {
    if (errno == 0) {
      // get the error code returned by the firmware
      sl_status_t status = sl_wifi_get_saved_firmware_status();
      printf("\r\nSocket select failed with bsd error: %d and status = 0x%lx\r\n", errno, status);
    } else {
      printf("\r\nSocket select failed with bsd error: %d\r\n", errno);
    }

    close_sockets(client_socket, server_socket);
    return;
  }

  if (total_set_fds_count == 0) {
    printf("\r\n No data available on any file descriptor \r\n");
    close_sockets(client_socket, server_socket);
    return;
  }

  for (uint8_t socket = 1; socket <= highest_socket_number; socket++) {

    if (FD_ISSET(socket, &read_fds)) {
      recv(client_socket, recv_buffer, sizeof(recv_buffer), 0);
    }
  }

  printf("\r\n Data received successfully \r\n");
  close_sockets(client_socket, server_socket);
  printf("\r\n Socket close success\r\n");
}
