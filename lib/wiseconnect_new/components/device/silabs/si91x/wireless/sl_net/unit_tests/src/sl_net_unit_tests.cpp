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
extern "C" {
#include "sl_net_fake_functions.h"
}

#define SLI_MAX_PRIVATE_KEY_LENGTH          4096
#define SLI_MAX_PRIVATE_KEY_METADATA_LENGTH 8

uint32_t custom_htonl(uint32_t value)
{
  return ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) | ((value & 0x00FF0000) >> 8)
         | ((value & 0xFF000000) >> 24);
}

// Custom fake function for sl_redirect_log
void sl_redirect_log(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vprintf(format, args); // Simulate logging
  va_end(args);
}

static void SetupIPv4SocketAndDest(sli_si91x_socket_t &mock_socket,
                                   uint32_t ip_addr,
                                   uint8_t socket_index,
                                   sl_ip_address_t &dest_ip_add)
{
  memset(&mock_socket, 0, sizeof(mock_socket));
  mock_socket.vap_id = 1;
  struct sockaddr_in mock_address;
  mock_address.sin_addr.s_addr = custom_htonl(ip_addr);
  memcpy(&mock_socket.remote_address, &mock_address, sizeof(mock_address));
  sli_si91x_sockets[socket_index] = &mock_socket;

  dest_ip_add.type           = SL_IPV4;
  dest_ip_add.ip.v4.bytes[0] = (ip_addr >> 24) & 0xFF;
  dest_ip_add.ip.v4.bytes[1] = (ip_addr >> 16) & 0xFF;
  dest_ip_add.ip.v4.bytes[2] = (ip_addr >> 8) & 0xFF;
  dest_ip_add.ip.v4.bytes[3] = ip_addr & 0xFF;
}

// Mock socket array
sli_si91x_socket_t *sli_si91x_sockets[SLI_NUMBER_OF_SOCKETS];

//validate ecb wrap key with valid length
TEST(sl_net_unit_tests, handles_tls_private_key_ecb_wrap_valid_length)
{
  sl_net_credential_id_t id     = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;
  sl_net_credential_type_t type = SL_NET_TLS_PRIVATE_KEY_ECB_WRAP;
  const char credential[]       = "dummy_key";
  uint32_t credential_length    = SLI_MAX_PRIVATE_KEY_LENGTH - SLI_MAX_PRIVATE_KEY_METADATA_LENGTH - SL_SI91X_IV_SIZE;

  RESET_FAKE(sl_si91x_wifi_set_certificate_index);
  sl_si91x_wifi_set_certificate_index_fake.return_val = SL_STATUS_OK;

  sl_status_t status = sli_net_set_credential(id, type, credential, credential_length);

  EXPECT_EQ(status, SL_STATUS_OK);
  EXPECT_EQ(sl_si91x_wifi_set_certificate_index_fake.call_count, 2);
}

//validate ecb wrap key with invalid length
TEST(sl_net_unit_tests, handles_tls_private_key_ecb_wrap_invalid_length)
{
  sl_net_credential_id_t id     = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;
  sl_net_credential_type_t type = SL_NET_TLS_PRIVATE_KEY_ECB_WRAP;
  const char credential[]       = "dummy_key";
  uint32_t credential_length    = SLI_MAX_PRIVATE_KEY_LENGTH; // Exceeds the allowed length

  RESET_FAKE(sl_si91x_wifi_set_certificate_index);
  sl_status_t status = sli_net_set_credential(id, type, credential, credential_length);

  // Validate the result
  EXPECT_EQ(status, SL_STATUS_INVALID_PARAMETER);
  EXPECT_EQ(sl_si91x_wifi_set_certificate_index_fake.call_count, 0); // Should not call the underlying function
}

//validate cbc wrap key with valid length
TEST(sl_net_unit_tests, handles_tls_private_key_cbc_wrap_valid_length)
{
  sl_net_credential_id_t id     = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;
  sl_net_credential_type_t type = SL_NET_TLS_PRIVATE_KEY_CBC_WRAP;
  const char credential[]       = "dummy_key";
  uint32_t credential_length    = SLI_MAX_PRIVATE_KEY_LENGTH - SLI_MAX_PRIVATE_KEY_METADATA_LENGTH - SL_SI91X_IV_SIZE;

  RESET_FAKE(sl_si91x_wifi_set_certificate_index);
  sl_si91x_wifi_set_certificate_index_fake.return_val = SL_STATUS_OK;

  sl_status_t status = sli_net_set_credential(id, type, credential, credential_length);

  EXPECT_EQ(status, SL_STATUS_OK);
  EXPECT_EQ(sl_si91x_wifi_set_certificate_index_fake.call_count, 2); // Called twice: once to clear, once to set
}

//validate cbc wrap key with invalid length
TEST(sl_net_unit_tests, handles_tls_private_key_cbc_wrap_invalid_length)
{
  sl_net_credential_id_t id     = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;
  sl_net_credential_type_t type = SL_NET_TLS_PRIVATE_KEY_CBC_WRAP;
  const char credential[]       = "dummy_key";
  uint32_t credential_length    = SLI_MAX_PRIVATE_KEY_LENGTH; // Exceeds the allowed length

  RESET_FAKE(sl_si91x_wifi_set_certificate_index);
  sl_status_t status = sli_net_set_credential(id, type, credential, credential_length);

  EXPECT_EQ(status, SL_STATUS_INVALID_PARAMETER);
  EXPECT_EQ(sl_si91x_wifi_set_certificate_index_fake.call_count, 0); // Should not call the underlying function
}

//validate set certificate index failure
TEST(sl_net_unit_tests, handles_certificate_index_failure)
{
  sl_net_credential_id_t id     = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;
  sl_net_credential_type_t type = SL_NET_TLS_PRIVATE_KEY_ECB_WRAP;
  const char credential[]       = "dummy_key";
  uint32_t credential_length    = SLI_MAX_PRIVATE_KEY_LENGTH - SLI_MAX_PRIVATE_KEY_METADATA_LENGTH - SL_SI91X_IV_SIZE;

  RESET_FAKE(sl_si91x_wifi_set_certificate_index);
  sl_si91x_wifi_set_certificate_index_fake.return_val = SL_STATUS_FAIL;

  sl_status_t status = sli_net_set_credential(id, type, credential, credential_length);

  EXPECT_EQ(status, SL_STATUS_FAIL);
  EXPECT_EQ(sl_si91x_wifi_set_certificate_index_fake.call_count, 1);
}

// Test case: Flush IPv4 socket queues
TEST(sl_net_unit_tests, FlushIPv4SocketQueues)
{
  RESET_FAKE(sli_si91x_flush_socket_command_queues_based_on_queue_type);
  RESET_FAKE(sli_si91x_flush_socket_data_queues_based_on_queue_type);

  sli_si91x_socket_t mock_socket;
  sl_ip_address_t dest_ip_add;

  // Set up mock_socket and destination IP to 192.168.0.1
  SetupIPv4SocketAndDest(mock_socket, 0xC0A80001, 0, dest_ip_add);

  // Call the function under test
  sl_status_t status = sli_si91x_flush_all_socket_tx_queues_based_on_dest_ip_address(SL_STATUS_OK, &dest_ip_add, 1);

  // Verify the results
  EXPECT_EQ(status, SL_STATUS_OK);
  EXPECT_EQ(sli_si91x_flush_socket_command_queues_based_on_queue_type_fake.call_count, 1);
  EXPECT_EQ(sli_si91x_flush_socket_data_queues_based_on_queue_type_fake.call_count, 1);
}

// Test case: Flush IPv6 socket queues
TEST(sl_net_unit_tests, FlushIPv6SocketQueues)
{
  RESET_FAKE(sli_si91x_flush_socket_command_queues_based_on_queue_type);
  RESET_FAKE(sli_si91x_flush_socket_data_queues_based_on_queue_type);

  sli_si91x_socket_t mock_socket;
  memset(&mock_socket, 0, sizeof(mock_socket));
  mock_socket.vap_id = 1;
  struct sockaddr_in6 mock_address;
  memset(&mock_address.sin6_addr, 0xAB, SL_IPV6_ADDRESS_LENGTH);
  memcpy(&mock_socket.remote_address, &mock_address, sizeof(mock_address));
  sli_si91x_sockets[0] = &mock_socket;

  sl_ip_address_t dest_ip_add;
  dest_ip_add.type = SL_IPV6;
  memset(dest_ip_add.ip.v6.bytes, 0xAB, SL_IPV6_ADDRESS_LENGTH);

  sl_status_t status = sli_si91x_flush_all_socket_tx_queues_based_on_dest_ip_address(SL_STATUS_OK, &dest_ip_add, 1);

  EXPECT_EQ(status, SL_STATUS_OK);
  EXPECT_EQ(sli_si91x_flush_socket_command_queues_based_on_queue_type_fake.call_count, 1);
  EXPECT_EQ(sli_si91x_flush_socket_data_queues_based_on_queue_type_fake.call_count, 1);
}

// Test case: No matching socket
TEST(sl_net_unit_tests, NoMatchingSocket)
{
  RESET_FAKE(sli_si91x_flush_socket_command_queues_based_on_queue_type);
  RESET_FAKE(sli_si91x_flush_socket_data_queues_based_on_queue_type);

  sli_si91x_socket_t mock_socket;
  sl_ip_address_t dest_ip_add;

  // Set up mock_socket with IP 192.168.0.2
  SetupIPv4SocketAndDest(mock_socket, 0xC0A80002, 0, dest_ip_add);

  // Set destination to 192.168.0.1 (does not match mock_socket)
  dest_ip_add.type           = SL_IPV4;
  dest_ip_add.ip.v4.bytes[0] = 192;
  dest_ip_add.ip.v4.bytes[1] = 168;
  dest_ip_add.ip.v4.bytes[2] = 0;
  dest_ip_add.ip.v4.bytes[3] = 1;

  // Call the function under test
  sl_status_t status = sli_si91x_flush_all_socket_tx_queues_based_on_dest_ip_address(SL_STATUS_OK, &dest_ip_add, 1);

  // Verify the results
  EXPECT_EQ(status, SL_STATUS_OK);
  EXPECT_EQ(sli_si91x_flush_socket_command_queues_based_on_queue_type_fake.call_count, 0);
  EXPECT_EQ(sli_si91x_flush_socket_data_queues_based_on_queue_type_fake.call_count, 0);
}
