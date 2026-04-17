/***************************************************************************/ /**
 * @file
 * @brief DTLS Client Example Application
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
#include "cmsis_os2.h"
#include "sl_board_configuration.h"
#include "sl_net.h"
#include "sl_wifi_types.h"
#include <string.h>
#include "sl_wifi.h"
#include "errno.h"
#include "socket.h"
#include "sl_net_si91x.h"
#include "sl_utility.h"
#include "sl_si91x_socket_constants.h"
#include "sl_si91x_socket_types.h"
#include "sl_si91x_socket_utility.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_socket_support.h"
#include "sl_si91x_socket.h"

//include certificates
#include "cacert.pem.h"
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define SERVER_IP              "192.168.0.206"
#define SERVER_PORT1           5002
#define SERVER_PORT2           5003
#define DATA                   "Hello from DTLS Client\n"
#define NUMBER_OF_PACKETS      10
#define DTLS_EXTENSION_ENABLE  1
#define TLS_ALPN_EXTENSION     "http/1.1"
#define TLS_SNI_EXTENSION      "example.com"
#define RECV_BUFFER_SIZE       500
#define MAX_SOCKET_RETRY_COUNT 3

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

static const sl_wifi_device_configuration_t dtls_init_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION | SL_WIFI_FEAT_WPS_DISABLE),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID

                                              | SL_SI91X_TCP_IP_FEAT_SSL | SL_SI91X_TCP_IP_FEAT_DTLS_THREE_SOCKETS

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
                      | SL_SI91X_EXT_TCP_IP_WAIT_FOR_SOCKET_CLOSE

                      | SL_SI91X_EXT_TCP_IP_FEAT_SSL_THREE_SOCKETS | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(10)

                        ),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
};

/******************************************************
 *               Function Declarations
 ******************************************************/
static void application_start(void *argument);
sl_status_t send_data_from_dtls_socket();
sl_status_t set_dtls_extensions(int client_socket);

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
  sl_status_t status = SL_STATUS_OK;

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &dtls_init_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client interface init success\r\n");

  status =
    sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0), SL_NET_SIGNING_CERTIFICATE, cacert, sizeof(cacert) - 1);
  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error while loading certificate: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nLoading certificate Success\r\n");

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client interface up Success\r\n");

  status = send_data_from_dtls_socket();
  if (status != SL_STATUS_OK) {
    printf("\r\nError while sending data: 0x%lx \r\n", status);
    return;
  }
}

sl_status_t send_data_from_dtls_socket()
{
  int client_socket1   = -1;
  int client_socket2   = -1;
  sl_status_t status   = SL_STATUS_OK;
  int32_t return_value = 0;
  int32_t packet_count = 0;
  bool need_retry      = false;
  int32_t retry_count  = 0;

  socklen_t socket_length            = sizeof(struct sockaddr_in);
  struct sockaddr_in server_address1 = { 0 };
  server_address1.sin_family         = AF_INET;
  server_address1.sin_port           = SERVER_PORT1;
  sl_net_inet_addr(SERVER_IP, &server_address1.sin_addr.s_addr);

  struct sockaddr_in server_address2 = { 0 };
  server_address2.sin_family         = AF_INET;
  server_address2.sin_port           = SERVER_PORT2;
  sl_net_inet_addr(SERVER_IP, &server_address2.sin_addr.s_addr);

  while (retry_count < MAX_SOCKET_RETRY_COUNT) {
    need_retry     = false;
    client_socket1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client_socket1 < 0) {
      printf("\r\nSocket1 creation failed with bsd error: %d\r\n", errno);
      return SL_STATUS_FAIL;
    }
    printf("\r\nSocket1 ID: %d\r\n", client_socket1);

    uint16_t enable_dtls = SL_SI91X_ENABLE_DTLS | SL_SI91X_DTLS_V_1_2;
    return_value =
      sl_si91x_setsockopt(client_socket1, SOL_SOCKET, SL_SI91X_SO_DTLS_V_1_2_ENABLE, &enable_dtls, sizeof(enable_dtls));
    if (return_value < 0) {
      printf("\r\nSet Socket option failed with bsd error: %d\r\n", errno);
      sl_si91x_shutdown(client_socket1, 0);
      return SL_STATUS_FAIL;
    }

#if DTLS_EXTENSION_ENABLE
    status = set_dtls_extensions(client_socket1);
    if (status != SL_STATUS_OK) {
      printf("\r\nFailed to set DTLS extension: 0x%lx\r\n", status);
      close(client_socket1);
      return SL_STATUS_FAIL;
    }
    printf("\r\nDTLS extension set successfully\r\n");
#endif

    client_socket2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client_socket2 < 0) {
      printf("\r\nSocket2 creation failed with bsd error: %d\r\n", errno);
      close(client_socket1);
      return SL_STATUS_FAIL;
    }
    printf("\r\nSocket2 ID: %d\r\n", client_socket2);

    uint16_t enable_dtls2 = SL_SI91X_ENABLE_DTLS | SL_SI91X_DTLS_V_1_2;
    return_value          = sl_si91x_setsockopt(client_socket2,
                                       SOL_SOCKET,
                                       SL_SI91X_SO_DTLS_V_1_2_ENABLE,
                                       &enable_dtls2,
                                       sizeof(enable_dtls2));
    if (return_value < 0) {
      printf("\r\nSet socket2 option failed with bsd error: %d\r\n", errno);
      close(client_socket1);
      close(client_socket2);
      return SL_STATUS_FAIL;
    }

#if DTLS_EXTENSION_ENABLE
    status = set_dtls_extensions(client_socket2);
    if (status != SL_STATUS_OK) {
      printf("\r\nFailed to set DTLS extension: 0x%lx\r\n", status);
      close(client_socket1);
      close(client_socket2);
      return SL_STATUS_FAIL;
    }
    printf("\r\nDTLS extension set successfully\r\n");
#endif

    while (packet_count <= NUMBER_OF_PACKETS) {
      return_value = sendto(client_socket1, DATA, strlen(DATA), 0, (struct sockaddr *)&server_address1, socket_length);
      if (return_value < 0) {
        if (errno == ENOBUFS)
          continue;
        sl_status_t fw_status = sl_wifi_get_saved_firmware_status();

        if (fw_status == SL_STATUS_SI91X_SSL_TLS_HANDSHAKE_FAIL) {
          printf("\r\nSocket1 send failed error = %d and status = 0x%lx \r\n", errno, fw_status);
          close(client_socket1);
          close(client_socket2);
          need_retry = true;
          break;
        }
        printf("\r\nSocket1 send failed with bsd error: %d\r\n", errno);
        close(client_socket1);
        close(client_socket2);
        return SL_STATUS_FAIL;
      }

      while (1) {
        return_value =
          sendto(client_socket2, DATA, strlen(DATA), 0, (struct sockaddr *)&server_address2, socket_length);
        if (return_value < 0) {
          if (errno == ENOBUFS)
            continue;
          sl_status_t fw_status = sl_wifi_get_saved_firmware_status();

          if (fw_status == SL_STATUS_SI91X_SSL_TLS_HANDSHAKE_FAIL) {
            printf("\r\nSocket2 send failed error = %d and status = 0x%lx \r\n", errno, fw_status);
            close(client_socket1);
            close(client_socket2);
            need_retry = true;
            break;
          }
          printf("\r\nSocket2 send failed with bsd error: %d\r\n", errno);
          close(client_socket1);
          close(client_socket2);
          return SL_STATUS_FAIL;
        }
        break;
      }
      if (!need_retry)
        packet_count++;
    }

    if (need_retry) {
      printf("\r\n Retrying due to TLS Handshake failure\r\n");
      packet_count = 0;
      retry_count++;
      continue;
    }

    break; // Transfer complete, break out of retry loop
  }

  if (retry_count >= MAX_SOCKET_RETRY_COUNT) {
    printf("\r\nFailed after max retries\r\n");
    return SL_STATUS_FAIL;
  }

  printf("\r\nData sent successfully\r\n");

  fd_set read_fds;
  int select_result;
  FD_ZERO(&read_fds);
  FD_SET(client_socket1, &read_fds);
  FD_SET(client_socket2, &read_fds);

  uint8_t recv_buffer[RECV_BUFFER_SIZE];
  printf("\r\nWaiting on select to receive data on any of the two sockets\r\n");
  select_result = select(client_socket2 + 1, &read_fds, NULL, NULL, NULL);
  if (select_result < 0) {
    printf("\r\nSelect call failed with bsd error: %d\r\n", errno);
    close(client_socket1);
    close(client_socket2);
    return SL_STATUS_FAIL;
  } else if (select_result == 0) {
    printf("\r\nSelect call timed out\r\n");
  } else {
    if (FD_ISSET(client_socket1, &read_fds)) {
      int bytes_received = recv(client_socket1, recv_buffer, sizeof(recv_buffer), 0);
      if (bytes_received > 0) {
        recv_buffer[bytes_received] = '\0'; // Null-terminate the received data
        printf("Received data on client_socket1: %s\n", recv_buffer);
      }
    }
    if (FD_ISSET(client_socket2, &read_fds)) {
      int bytes_received = recv(client_socket2, recv_buffer, sizeof(recv_buffer), 0);
      if (bytes_received > 0) {
        recv_buffer[bytes_received] = '\0'; // Null-terminate the received data
        printf("Received data on client_socket2: %s\n", recv_buffer);
      }
    }
  }

  close(client_socket1);
  close(client_socket2);

  printf("\r\nSockets closed successfully\r\n");

  return status;
}

sl_status_t set_dtls_extensions(int client_socket)
{

  int socket_return_value = 0;

  // Calculate the length of the SNI data
  uint16_t sni_length = (uint16_t)strlen(TLS_SNI_EXTENSION);

  // Allocate memory for the sl_si91x_socket_type_length_value_t structure
  sl_si91x_socket_type_length_value_t *sni_value =
    (sl_si91x_socket_type_length_value_t *)malloc(sizeof(sl_si91x_socket_type_length_value_t) + sni_length);

  if (sni_value == NULL) {
    printf("\r\nMemory allocation failed for SNI value\r\n");
    return SL_STATUS_ALLOCATION_FAILED;
  }

  // Set the type to 1 for SNI extension
  sni_value->type = SL_SI91X_TLS_EXTENSION_SNI_TYPE;

  // Set the length of the SNI data
  sni_value->length = sni_length;

  // Copy the SNI data into the value field
  memcpy(sni_value->value, TLS_SNI_EXTENSION, sni_length);

  socket_return_value = setsockopt(client_socket,
                                   SOL_SOCKET,
                                   SL_SO_TLS_ALPN,
                                   sni_value,
                                   sizeof(sl_si91x_socket_type_length_value_t) + sni_length);

  if (socket_return_value < 0) {
    printf("\r\nSet Socket option SNI extension failed with bsd error: %d\r\n", errno);
    free(sni_value);
    return SL_STATUS_FAIL;
  }

  free(sni_value);

  // Calculate the length of the ALPN data
  uint16_t alpn_length = (uint16_t)strlen(TLS_ALPN_EXTENSION);

  // Allocate memory for the sl_si91x_socket_type_length_value_t  structure
  sl_si91x_socket_type_length_value_t *alpn_value =
    (sl_si91x_socket_type_length_value_t *)malloc(sizeof(sl_si91x_socket_type_length_value_t) + alpn_length);

  if (alpn_value == NULL) {
    printf("\r\nMemory allocation failed for ALPN value\r\n");
    return SL_STATUS_ALLOCATION_FAILED;
  }

  // Set the type to 2 for ALPN extension
  alpn_value->type = SL_SI91X_TLS_EXTENSION_ALPN_TYPE;

  // Set the length of the ALPN data
  alpn_value->length = alpn_length;

  // Copy the ALPN data into the value field
  memcpy(alpn_value->value, TLS_ALPN_EXTENSION, alpn_length);

  socket_return_value = setsockopt(client_socket,
                                   SOL_SOCKET,
                                   SL_SO_TLS_ALPN,
                                   alpn_value,
                                   sizeof(sl_si91x_socket_type_length_value_t) + alpn_length);

  if (socket_return_value < 0) {
    SL_DEBUG_LOG("\r\nSet Socket option ALPN extension failed with bsd error: %d\r\n", errno);
    free(alpn_value);
    return SL_STATUS_FAIL;
  }

  // Free the allocated memory after usage
  free(alpn_value);
  return SL_STATUS_OK;
}
