/***************************************************************************/ /**
 * @file
 * @brief TLS Client Example Application
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
#include "sl_si91x_socket_support.h"

//include certificates
#include "cacert.pem.h"
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

//! Load certificate to device flash :
//! Certificate should be loaded once and need not need to load for every boot up
#define LOAD_CERTIFICATE     1
#define SERVER_IP            "192.168.0.247"
#define SERVER_PORT1         5002
#define SERVER_PORT2         5003
#define DATA                 "Hello from SSL TCP Client\n"
#define NUMBER_OF_PACKETS    1000
#define TLS_EXTENSION_ENABLE 0
#define TLS_ALPN_EXTENSION   "http/1.1"
#define TLS_SNI_EXTENSION    "example.com"

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

static const sl_wifi_device_configuration_t station_init_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode       = SL_SI91X_CLIENT_MODE,
                   .coex_mode       = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map = (SL_WIFI_FEAT_SECURITY_PSK
#ifdef SLI_SI91X_MCU_INTERFACE
                                       | SL_WIFI_FEAT_WPS_DISABLE
#endif
                                       ),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_SSL | SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_SSL_VERSIONS_SUPPORT | MEMORY_CONFIG
#ifdef SLI_SI917
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
};

/******************************************************
 *               Function Declarations
 ******************************************************/
static void application_start(void *argument);
sl_status_t send_data_from_tls_socket();
sl_status_t set_tls_extensions(int client_socket);

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

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &station_init_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client interface init success\r\n");

#if LOAD_CERTIFICATE
  status =
    sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0), SL_NET_SIGNING_CERTIFICATE, cacert, sizeof(cacert) - 1);
  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error while loading certificate: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nLoading certificate Success\r\n");
#endif

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client interface up Success\r\n");

  status = send_data_from_tls_socket();
  if (status != SL_STATUS_OK) {
    printf("\r\nError while sending data: 0x%lx \r\n", status);
    return;
  }
}

sl_status_t send_data_from_tls_socket()
{
  int client_socket1                = -1;
  int client_socket2                = -1;
  sl_ipv4_address_t ip              = { 0 };
  sl_status_t status                = SL_STATUS_OK;
  int32_t return_value              = 0;
  int32_t packet_count              = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  char *tls_socket1                 = "tls_1_0";
  char *tls_socket2                 = "tls_1_2";

  sl_net_inet_addr(SERVER_IP, (uint32_t *)&ip);

  server_address.sin_family      = AF_INET;
  server_address.sin_port        = SERVER_PORT1;
  server_address.sin_addr.s_addr = ip.value;

  client_socket1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_socket1 < 0) {
    printf("\r\nSocket1 creation failed with bsd error: %d\r\n", errno);
    return SL_STATUS_FAIL;
  }
  printf("\r\nSocket1 ID: %d\r\n", client_socket1);

  return_value = setsockopt(client_socket1, SOL_TCP, TCP_ULP, TLS_1_0, sizeof(TLS_1_0));
  if (return_value < 0) {
    printf("\r\nSet socket1 option failed with bsd error: %d\r\n", errno);
    close(client_socket1);
    return SL_STATUS_FAIL;
  }

#if TLS_EXTENSION_ENABLE
  status = set_tls_extensions(client_socket1);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to set TLS extension: 0x%lx\r\n", status);
    close(client_socket1);
    return SL_STATUS_FAIL;
  }
  printf("\r\nTLS extension set successfully\r\n");
#endif

  return_value = connect(client_socket1, (struct sockaddr *)&server_address, socket_length);
  if (return_value < 0) {
    printf("\r\nSocket1 connect failed with %s with bsd error: %d\r\n", tls_socket1, errno);
    close(client_socket1);
    return SL_STATUS_FAIL;
  }
  printf("\r\nSocket1 connect success with %s\r\n", tls_socket1);

  client_socket2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_socket2 < 0) {
    printf("\r\nSocket2 creation failed with bsd error: %d\r\n", errno);
    close(client_socket1);
    return SL_STATUS_FAIL;
  }
  printf("\r\nSocket2 ID: %d\r\n", client_socket2);

  return_value = setsockopt(client_socket2, SOL_TCP, TCP_ULP, TLS_1_2, sizeof(TLS_1_2));
  if (return_value < 0) {
    printf("\r\nSet socket2 option failed with bsd error: %d\r\n", errno);
    close(client_socket1);
    close(client_socket2);
    return SL_STATUS_FAIL;
  }

#if TLS_EXTENSION_ENABLE
  status = set_tls_extensions(client_socket2);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to set TLS extension: 0x%lx\r\n", status);
    close(client_socket1);
    close(client_socket2);
    return SL_STATUS_FAIL;
  }
  printf("\r\nTLS extension set successfully\r\n");
#endif

  server_address.sin_port = SERVER_PORT2;
  return_value            = connect(client_socket2, (struct sockaddr *)&server_address, socket_length);
  if (return_value < 0) {
    printf("\r\nSocket2 connect failed with %s with bsd error: %d\r\n", tls_socket2, errno);
    close(client_socket1);
    close(client_socket2);
    return SL_STATUS_FAIL;
  }
  printf("\r\nSocket2 connect success with %s \r\n", tls_socket2);

  while (packet_count < NUMBER_OF_PACKETS) {
    return_value = send(client_socket1, DATA, strlen(DATA), 0);
    if (return_value < 0) {
      if (errno == ENOBUFS)
        continue;
      printf("\r\nSocket1 send failed with bsd error: %d\r\n", errno);
      close(client_socket1);
      close(client_socket2);
      return SL_STATUS_FAIL;
    }

    while (1) {
      return_value = send(client_socket2, DATA, strlen(DATA), 0);
      if (return_value < 0) {
        if (errno == ENOBUFS)
          continue;
        printf("\r\nSocket2 send failed with bsd error: %d\r\n", errno);
        close(client_socket1);
        close(client_socket2);
        return SL_STATUS_FAIL;
      }
      break;
    }
    packet_count++;
  }

  printf("\r\nData sent successfully\r\n");

  close(client_socket1);
  close(client_socket2);

  printf("\r\nSockets closed successfully\r\n");

  return status;
}

sl_status_t set_tls_extensions(int client_socket)
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
