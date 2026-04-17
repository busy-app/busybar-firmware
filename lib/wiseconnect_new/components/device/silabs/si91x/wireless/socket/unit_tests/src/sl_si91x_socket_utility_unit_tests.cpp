/*******************************************************************************
 * @file
 * @brief
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
#include <gtest/gtest.h>
#include <cstring> // For memset
#include "fff.h"

// Include the header file for the function under test
extern "C" {
#include "sl_si91x_socket_utility_fake_function.h"
#include "sl_string.h"
void sli_si91x_create_socket_request(sli_si91x_socket_t *si91x_bsd_socket,
                                     sli_si91x_socket_create_request_t *socket_create_request,
                                     int type,
                                     const int *backlog,
                                     sli_wifi_wait_period_t *wait_period);
}

uint16_t custom_htons(uint16_t value)
{
  return ((value & 0x00FF) << 8) | ((value & 0xFF00) >> 8);
}

uint32_t custom_htonl(uint32_t value)
{
  return ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) | ((value & 0x00FF0000) >> 8)
         | ((value & 0xFF000000) >> 24);
}

// Fake variables
volatile uint32_t tx_socket_data_queues_status    = 0;
volatile uint32_t tx_socket_command_queues_status = 0;

// Fake types
sli_task_register_id_t sli_fw_status_storage_index;
typedef struct {
  void *data;
} fake_wifi_system_packet_t;

// Test case: Handle IPv4 accept response
TEST(sl_si91x_socket_utility_unit_tests, HandleIPv4AcceptResponse)
{
  // Reset the fake function

  // Set up mock socket
  sli_si91x_socket_t mock_socket;
  memset(&mock_socket, 0, sizeof(mock_socket));

  // Set up mock accept response
  sli_si91x_rsp_ltcp_est_t mock_accept_response;
  memset(&mock_accept_response, 0, sizeof(mock_accept_response));
  mock_accept_response.socket_id                    = 1;
  mock_accept_response.src_port_num                 = custom_htons(12345);
  mock_accept_response.dest_port                    = custom_htons(54321);
  mock_accept_response.mss                          = 1500;
  mock_accept_response.ip_version                   = SL_IPV4_ADDRESS_LENGTH;
  mock_accept_response.dest_ip_addr.ipv4_address[0] = 192;
  mock_accept_response.dest_ip_addr.ipv4_address[1] = 168;
  mock_accept_response.dest_ip_addr.ipv4_address[2] = 1;
  mock_accept_response.dest_ip_addr.ipv4_address[3] = 1;

  // Call the function under test
  sli_handle_accept_response(&mock_socket, &mock_accept_response);

  // Verify the results
  EXPECT_EQ(mock_socket.id, 1);
  EXPECT_EQ(mock_socket.local_address.sin6_port, custom_htons(12345));
  EXPECT_EQ(mock_socket.remote_address.sin6_port, custom_htons(54321));
  EXPECT_EQ(mock_socket.mss, 1500);
  EXPECT_EQ(mock_socket.state, CONNECTED);
  EXPECT_EQ(mock_socket.remote_address.sin6_family, AF_INET);

  // Verify that the IPv4 address was copied correctly
  struct sockaddr_in *remote_address = (struct sockaddr_in *)&mock_socket.remote_address;
  EXPECT_EQ(remote_address->sin_addr.s_addr, *(uint32_t *)mock_accept_response.dest_ip_addr.ipv4_address);
}

// Test case: Handle IPv6 accept response
TEST(sl_si91x_socket_utility_unit_tests, HandleIPv6AcceptResponse)
{

  // Set up mock socket
  sli_si91x_socket_t mock_socket;
  memset(&mock_socket, 0, sizeof(mock_socket));

  // Set up mock accept response
  sli_si91x_rsp_ltcp_est_t mock_accept_response;
  memset(&mock_accept_response, 0, sizeof(mock_accept_response));
  mock_accept_response.socket_id    = 2;
  mock_accept_response.src_port_num = custom_htons(12345);
  mock_accept_response.dest_port    = custom_htons(54321);
  mock_accept_response.mss          = 1500;
  mock_accept_response.ip_version   = SL_IPV6_ADDRESS_LENGTH;
  memset(mock_accept_response.dest_ip_addr.ipv6_address, 0xAB, SL_IPV6_ADDRESS_LENGTH);

  // Call the function under test
  sli_handle_accept_response(&mock_socket, &mock_accept_response);

  // Verify the results
  EXPECT_EQ(mock_socket.id, 2);
  EXPECT_EQ(mock_socket.local_address.sin6_port, custom_htons(12345));
  EXPECT_EQ(mock_socket.remote_address.sin6_port, custom_htons(54321));
  EXPECT_EQ(mock_socket.mss, 1500);
  EXPECT_EQ(mock_socket.state, CONNECTED);
  EXPECT_EQ(mock_socket.remote_address.sin6_family, AF_INET6);

// Verify that the IPv6 address was copied correctly
#ifdef SLI_SI91X_NETWORK_DUAL_STACK
  EXPECT_EQ(memcmp(mock_socket.remote_address.sin6_addr.un.u8_addr,
                   mock_accept_response.dest_ip_addr.ipv6_address,
                   SL_IPV6_ADDRESS_LENGTH),
            0);
#else
#ifndef __ZEPHYR__
  EXPECT_EQ(memcmp(mock_socket.remote_address.sin6_addr.__u6_addr.__u6_addr8,
                   mock_accept_response.dest_ip_addr.ipv6_address,
                   SL_IPV6_ADDRESS_LENGTH),
            0);
#else
  EXPECT_EQ(memcmp(mock_socket.remote_address.sin6_addr.s6_addr,
                   mock_accept_response.dest_ip_addr.ipv6_address,
                   SL_IPV6_ADDRESS_LENGTH),
            0);
#endif
#endif
}

// Test case: Null socket
TEST(sl_si91x_socket_utility_unit_tests, NullSocket)
{

  // Set up mock accept response
  sli_si91x_rsp_ltcp_est_t mock_accept_response;
  memset(&mock_accept_response, 0, sizeof(mock_accept_response));

  // Call the function under test with a null socket
  sli_handle_accept_response(NULL, &mock_accept_response);

  // Verify that no crash or unexpected behavior occurred
  SUCCEED();
}

// Test case: Create socket request for IPv4
TEST(sl_si91x_socket_utility_unit_tests, CreateIPv4SocketRequest)
{

  // Set up mock socket
  sli_si91x_socket_t mock_socket;
  memset(&mock_socket, 0, sizeof(mock_socket));
  mock_socket.local_address.sin6_family                                = AF_INET;
  mock_socket.remote_address.sin6_port                                 = custom_htons(54321);
  mock_socket.local_address.sin6_port                                  = custom_htons(12345);
  mock_socket.vap_id                                                   = 1;
  mock_socket.mss                                                      = 1500;
  mock_socket.ssl_bitmap                                               = 0;
  mock_socket.socket_bitmap                                            = 0;
  mock_socket.max_tcp_retries                                          = 3;
  mock_socket.tcp_keepalive_initial_time                               = 100;
  mock_socket.recv_data_callback                                       = NULL;
  mock_socket.remote_address.sin6_family                               = AF_INET;
  ((struct sockaddr_in *)&mock_socket.remote_address)->sin_addr.s_addr = custom_htonl(0xC0A80001); // 192.168.0.1

  // Set up mock socket create request
  sli_si91x_socket_create_request_t mock_request;
  memset(&mock_request, 0, sizeof(mock_request));

  // Set up wait period
  sli_wifi_wait_period_t wait_period;

  // Call the function under test
  sli_si91x_create_socket_request(&mock_socket, &mock_request, SOCK_STREAM, NULL, &wait_period);

  // Verify the results
  EXPECT_EQ(mock_request.ip_version, SL_IPV4_ADDRESS_LENGTH);
  EXPECT_EQ(mock_request.local_port, custom_htons(12345));
  EXPECT_EQ(mock_request.remote_port, custom_htons(54321));
  EXPECT_EQ(mock_request.socket_type, SOCK_STREAM);
  EXPECT_EQ(mock_request.vap_id, 1);
  EXPECT_EQ(mock_request.rx_window_size, SLI_TCP_RX_WINDOW_SIZE);
  EXPECT_EQ(mock_request.tcp_mss, 1500);
  EXPECT_EQ(mock_request.max_tcp_retries_count, 3);
  EXPECT_EQ(mock_request.tcp_keepalive_initial_time, 100);
  EXPECT_EQ(mock_request.socket_bitmap & SLI_SI91X_SOCKET_FEAT_SYNCHRONOUS, SLI_SI91X_SOCKET_FEAT_SYNCHRONOUS);

  // Verify that the IPv4 address was copied correctly
  EXPECT_EQ(memcmp(mock_request.dest_ip_addr.ipv4_address,
                   &((struct sockaddr_in *)&mock_socket.remote_address)->sin_addr,
                   SL_IPV4_ADDRESS_LENGTH),
            0);
}

// Test case: Create socket request for IPv6
TEST(sl_si91x_socket_utility_unit_tests, CreateIPv6SocketRequest)
{

  // Set up mock socket
  sli_si91x_socket_t mock_socket;
  memset(&mock_socket, 0, sizeof(mock_socket));
  mock_socket.local_address.sin6_family  = AF_INET6;
  mock_socket.remote_address.sin6_port   = custom_htons(54321);
  mock_socket.local_address.sin6_port    = custom_htons(12345);
  mock_socket.vap_id                     = 1;
  mock_socket.mss                        = 1500;
  mock_socket.ssl_bitmap                 = 0;
  mock_socket.socket_bitmap              = 0;
  mock_socket.max_tcp_retries            = 3;
  mock_socket.tcp_keepalive_initial_time = 100;
  mock_socket.recv_data_callback         = NULL;
  memset(mock_socket.remote_address.sin6_addr.s6_addr, 0xAB, SL_IPV6_ADDRESS_LENGTH);

  // Set up mock socket create request
  sli_si91x_socket_create_request_t mock_request;
  memset(&mock_request, 0, sizeof(mock_request));

  // Set up wait period
  sli_wifi_wait_period_t wait_period;

  // Call the function under test
  sli_si91x_create_socket_request(&mock_socket, &mock_request, SOCK_STREAM, NULL, &wait_period);

  // Verify the results
  EXPECT_EQ(mock_request.ip_version, SL_IPV6_VERSION);
  EXPECT_EQ(mock_request.local_port, custom_htons(12345));
  EXPECT_EQ(mock_request.remote_port, custom_htons(54321));
  EXPECT_EQ(mock_request.socket_type, SOCK_STREAM);
  EXPECT_EQ(mock_request.vap_id, 1);
  EXPECT_EQ(mock_request.rx_window_size, SLI_TCP_RX_WINDOW_SIZE);
  EXPECT_EQ(mock_request.tcp_mss, 1500);
  EXPECT_EQ(mock_request.max_tcp_retries_count, 3);
  EXPECT_EQ(mock_request.tcp_keepalive_initial_time, 100);
  EXPECT_EQ(mock_request.socket_bitmap & SLI_SI91X_SOCKET_FEAT_SYNCHRONOUS, SLI_SI91X_SOCKET_FEAT_SYNCHRONOUS);

  // Verify that the IPv6 address was copied correctly
  EXPECT_EQ(memcmp(mock_request.dest_ip_addr.ipv6_address,
                   mock_socket.remote_address.sin6_addr.s6_addr,
                   SL_IPV6_ADDRESS_LENGTH),
            0);
}

// Test case: Create socket request with SSL
TEST(sl_si91x_socket_utility_unit_tests, CreateSocketRequestWithSSL)
{

  // Set up mock socket
  sli_si91x_socket_t mock_socket;
  memset(&mock_socket, 0, sizeof(mock_socket));
  mock_socket.local_address.sin6_family  = AF_INET;
  mock_socket.remote_address.sin6_port   = custom_htons(54321);
  mock_socket.local_address.sin6_port    = custom_htons(12345);
  mock_socket.vap_id                     = 1;
  mock_socket.mss                        = 1500;
  mock_socket.ssl_bitmap                 = SL_SI91X_ENABLE_TLS;
  mock_socket.socket_bitmap              = 0;
  mock_socket.max_tcp_retries            = 3;
  mock_socket.tcp_keepalive_initial_time = 100;
  mock_socket.recv_data_callback         = NULL;

  // Set up mock socket create request
  sli_si91x_socket_create_request_t mock_request;
  memset(&mock_request, 0, sizeof(mock_request));

  // Set up wait period
  sli_wifi_wait_period_t wait_period;

  // Call the function under test
  sli_si91x_create_socket_request(&mock_socket, &mock_request, SOCK_STREAM, NULL, &wait_period);

  // Verify the results
  EXPECT_EQ(mock_request.ssl_bitmap, SL_SI91X_ENABLE_TLS);
  EXPECT_EQ(wait_period, SLI_WIFI_WAIT_FOR_RESPONSE(130000));
}

// Test case: Successful socket request creation and sending
TEST(sl_si91x_socket_utility_unit_tests, SuccessfulRequest)
{
  // Reset all fake functions
  RESET_FAKE(sli_si91x_driver_send_command);
  RESET_FAKE(sli_si91x_host_free_buffer);
  RESET_FAKE(sli_wifi_host_get_buffer_data);

  // Set up mock socket
  sli_si91x_socket_t mock_socket;
  memset(&mock_socket, 0, sizeof(mock_socket));
  mock_socket.state = INITIALIZED;

  // Set up mock socket creation response
  sli_si91x_socket_create_response_t mock_response;
  memset(&mock_response, 0, sizeof(mock_response));
  mock_response.socket_id[0]   = 1;
  mock_response.socket_id[1]   = 0;
  mock_response.module_port[0] = 0x12;
  mock_response.module_port[1] = 0x34;

  sl_wifi_system_packet_t *fake_packet;
  fake_packet =
    (sl_wifi_system_packet_t *)malloc(sizeof(sl_wifi_system_packet_t) + sizeof(sli_si91x_socket_create_response_t));
  memcpy(fake_packet->data, &mock_response, sizeof(sli_si91x_socket_create_response_t));

  // Set up fake function behavior
  sli_wifi_host_get_buffer_data_fake.return_val = fake_packet;
  sli_si91x_driver_send_command_fake.return_val = SL_STATUS_OK;

  sli_si91x_sockets[0] = &mock_socket;

  // Call the function under test
  sl_status_t status = sli_create_and_send_socket_request(0, SLI_SI91X_SOCKET_TCP_CLIENT, NULL);

  sli_si91x_sockets[0] = NULL;
  free(fake_packet);
  // Verify the results
  EXPECT_EQ(status, SL_STATUS_OK);
  EXPECT_EQ(mock_socket.id, 1);
  EXPECT_EQ(mock_socket.local_address.sin6_port, 0x3412); // Port is in little-endian format
  EXPECT_EQ(sli_si91x_driver_send_command_fake.call_count, 1);
  EXPECT_EQ(sli_wifi_host_get_buffer_data_fake.call_count, 1);
}

// Test case: Socket does not exist
TEST(sl_si91x_socket_utility_unit_tests, SocketDoesNotExist)
{
  // Reset all fake functions
  RESET_FAKE(sli_si91x_driver_send_command);
  RESET_FAKE(sli_si91x_host_free_buffer);
  RESET_FAKE(sli_wifi_host_get_buffer_data);

  // Call the function under test
  sl_status_t status = sli_create_and_send_socket_request(-1, SLI_SI91X_SOCKET_TCP_CLIENT, NULL);

  // Verify the results
  EXPECT_EQ(status, -1);
}

// Test case: Driver send command fails
TEST(sl_si91x_socket_utility_unit_tests, DriverSendCommandFails)
{
  // Reset all fake functions
  RESET_FAKE(sli_si91x_driver_send_command);
  RESET_FAKE(sli_si91x_host_free_buffer);
  RESET_FAKE(sli_wifi_host_get_buffer_data);

  // Set up mock socket
  sli_si91x_socket_t mock_socket;
  memset(&mock_socket, 0, sizeof(mock_socket));
  mock_socket.state = INITIALIZED;

  // Set up fake function behavior
  sli_si91x_driver_send_command_fake.return_val = SL_STATUS_FAIL;

  sli_si91x_sockets[0] = &mock_socket;

  // Call the function under test
  sl_status_t status = sli_create_and_send_socket_request(0, SLI_SI91X_SOCKET_TCP_CLIENT, NULL);

  sli_si91x_sockets[0] = NULL;

  // Verify the results
  EXPECT_EQ(status, SL_STATUS_FAIL);
  EXPECT_EQ(sli_si91x_driver_send_command_fake.call_count, 1);
}