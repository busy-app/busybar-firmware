/***************************************************************************/ /**
 * @file
 * @brief Three SSL Concurrent Client Sockets Example Application
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
#include "sl_wifi.h"
#include "sl_net_dns.h"
#include "socket.h"
#include "sl_si91x_socket_support.h"
#include "errno.h"
#include "sl_rsi_utility.h"

//! Certificates to be loaded
#include "aws_client_certificate.pem.crt.h"
#include "aws_client_private_key.pem.key.h"
#include "aws_starfield_ca.pem.h"
#include "cacert.pem.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define VERFIY_SOCKET_CREATE(expression)                                    \
  {                                                                         \
    if (expression) {                                                       \
      printf("\r\nSSL Socket Create failed with bsd error: %d\r\n", errno); \
    }                                                                       \
  };

/******************************************************
 *                    Constants
 ******************************************************/
//! Server IP address.
#define SERVER_ADDR "192.168.1.108"

//! AWS Domain name
#define AWS_DOMAIN_NAME "a2m21kovu9tcsh-ats.iot.us-east-2.amazonaws.com"

//! Load certificate to device flash :
//! Certificate should be loaded once and need not be loaded for every boot up
#define LOAD_CERTIFICATE 1

//! Server port numbers
#define SERVER_PORT1 5001
#define SERVER_PORT2 5002
#define SERVER_PORT3 8883

#define DNS_TIMEOUT 20000

#define SL_CERT_INDEX_0        0
#define SL_CERT_INDEX_1        1
#define SL_CERT_INDEX_2        2
#define MAX_SSL_CERTS_TO_FLASH 3

#define MAX_DNS_RETRY_COUNT 10

#define MAX_DATA_LENGTH 100

#define MAX_SOCKET 3

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
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
#ifdef SLI_SI91X_MCU_INTERFACE
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_WPS_DISABLE),
#else
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION),
#endif
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_SSL | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
#ifdef SLI_SI917
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_FEAT_SSL_THREE_SOCKETS | SL_SI91X_EXT_TCP_IP_FEAT_SSL_MEMORY_CLOUD),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = 0 }
};

uint8_t send_buffer[MAX_DATA_LENGTH];

/******************************************************
 *               Function Declarations
 ******************************************************/
static void application_start(void *argument);
sl_status_t clear_and_load_certificates_in_flash(void);
sl_status_t create_three_ssl_client_sockets(void);

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
  sl_status_t status;
  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &station_init_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to start Wi-Fi Client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client interface init success\r\n");

#if LOAD_CERTIFICATE
  status = clear_and_load_certificates_in_flash();
  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error while loading certificate: 0x%lX\r\n", status);
    return;
  }
#endif

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client connected\r\n");

  status = create_three_ssl_client_sockets();
  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error while creating SSL sockets: 0x%lX\r\n", status);
    return;
  }
  printf("\r\nExample Demonstration Completed\r\n");
}

sl_status_t clear_and_load_certificates_in_flash(void)
{
  sl_status_t status = SL_STATUS_OK;

  // Load SSL CA certificate
  status =
    sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0), SL_NET_SIGNING_CERTIFICATE, cacert, sizeof(cacert) - 1);
  if (status != SL_STATUS_OK) {
    printf("\r\nLoading TLS CA certificate in to FLASH Failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nLoad TLS CA certificate at index %d Success\r\n", 0);

  // Load SSL CA certificate
  status =
    sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(1), SL_NET_SIGNING_CERTIFICATE, cacert, sizeof(cacert) - 1);
  if (status != SL_STATUS_OK) {
    printf("\r\nLoading TLS CA certificate in to FLASH Failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nLoad TLS CA certificate at index %d Success\r\n", 1);

  // Load SSL CA certificate
  status = sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(2),
                                 SL_NET_SIGNING_CERTIFICATE,
                                 aws_starfield_ca,
                                 sizeof(aws_starfield_ca) - 1);
  if (status != SL_STATUS_OK) {
    printf("\r\nLoading TLS CA certificate in to FLASH Failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nLoading TLS CA certificate at index %d Successfull\r\n", 2);

  // Load SSL Client certificate
  status = sl_net_set_credential(SL_NET_TLS_CLIENT_CREDENTIAL_ID(2),
                                 SL_NET_CERTIFICATE,
                                 aws_client_certificate,
                                 sizeof(aws_client_certificate) - 1);
  if (status != SL_STATUS_OK) {
    printf("\r\nLoading TLS Client certificate in to FLASH Failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nLoading TLS Client certificate at index %d Successfull\r\n", 2);

  // Load SSL Client private key
  status = sl_net_set_credential(SL_NET_TLS_CLIENT_CREDENTIAL_ID(2),
                                 SL_NET_PRIVATE_KEY,
                                 aws_client_private_key,
                                 sizeof(aws_client_private_key) - 1);
  if (status != SL_STATUS_OK) {
    printf("\r\nLoading TLS Client private key in to FLASH Failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nLoading TLS Client private key at index %d Successfull\r\n", 2);

  return SL_STATUS_OK;
}

sl_status_t create_three_ssl_client_sockets(void)
{
  sl_status_t status            = SL_STATUS_OK;
  uint8_t ssl_certificate_index = SL_CERT_INDEX_0;
  int32_t dns_retry_count       = 0;
  int32_t socket_status         = -1;
  int32_t client_socket[MAX_SOCKET];
  int32_t bytes_sent = 0;
  int sock_id;
  sl_ip_address_t dns_query_rsp    = { 0 };
  sl_ipv4_address_t server_address = { 0 };
  struct sockaddr_in server_addr[MAX_SOCKET];

  //! Converting and storing given IP address into SL IPV4
  sl_net_inet_addr(SERVER_ADDR, (uint32_t *)&server_address);

  do {
    //! Getting IP address of the AWS server using DNS request
    status =
      sl_net_dns_resolve_hostname((const char *)AWS_DOMAIN_NAME, DNS_TIMEOUT, SL_NET_DNS_TYPE_IPV4, &dns_query_rsp);
    dns_retry_count++;
  } while ((dns_retry_count < MAX_DNS_RETRY_COUNT) && (status != SL_STATUS_OK));

  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error while while resolving dns, Error 0x%lX\r\n", status);
    return status;
  }

  //! Create SSL Socket
  for (sock_id = 0; sock_id < MAX_SOCKET; sock_id++) {
    client_socket[sock_id] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    VERFIY_SOCKET_CREATE(client_socket[sock_id] < 0);
    printf("\r\n%d SSL Socket Create Success\r\n", sock_id + 1);
  }

  //! Setting SSL socket option
  socket_status = setsockopt(client_socket[0], SOL_TCP, TCP_ULP, TLS, sizeof(TLS));
  if (socket_status < 0) {
    printf("\r\n1st SSL Set socket failed with bsd error: %d\r\n", errno);
    close(client_socket[0]);
    return SL_STATUS_FAIL;
  }

  //! Set certificate index for ssl socket
  ssl_certificate_index = SL_CERT_INDEX_0;
  socket_status =
    setsockopt(client_socket[0], SOL_SOCKET, SL_SO_CERT_INDEX, &ssl_certificate_index, sizeof(ssl_certificate_index));
  if (socket_status < 0) {
    printf("\r\n 1st SSL set certificate index failed with bsd error: %d\r\n", errno);
    close(client_socket[0]);
    return SL_STATUS_FAIL;
  }

  //! Setting SSL socket option
  socket_status = setsockopt(client_socket[1], SOL_TCP, TCP_ULP, TLS, sizeof(TLS));
  if (socket_status < 0) {
    printf("\r\n 2nd SSL set socket failed with bsd error: %d\r\n", errno);
    close(client_socket[1]);
    return SL_STATUS_FAIL;
  }

  //! Set certificate index for ssl socket
  ssl_certificate_index = SL_CERT_INDEX_1;
  socket_status =
    setsockopt(client_socket[1], SOL_SOCKET, SL_SO_CERT_INDEX, &ssl_certificate_index, sizeof(ssl_certificate_index));
  if (socket_status < 0) {
    printf("\r\n 2nd SSL Set certificate index failed with bsd error: %d\r\n", errno);
    close(client_socket[1]);
    return SL_STATUS_FAIL;
  }

  //! Setting SSL socket option
  socket_status = setsockopt(client_socket[2], SOL_TCP, TCP_ULP, TLS, sizeof(TLS));
  if (socket_status < 0) {
    printf("\r\n3rd SSL set socket failed with bsd error: %d\r\n", errno);
    close(client_socket[2]);
    return SL_STATUS_FAIL;
  }

  //! Set certificate index for ssl socket
  ssl_certificate_index = SL_CERT_INDEX_2;
  socket_status =
    setsockopt(client_socket[2], SOL_SOCKET, SL_SO_CERT_INDEX, &ssl_certificate_index, sizeof(ssl_certificate_index));
  if (socket_status < 0) {
    printf("\r\n3rd SSL set certificate index failed with bsd error: %d\r\n", errno);
    close(client_socket[2]);
    return SL_STATUS_FAIL;
  }

  printf("\r\nAws Ip : %u.%u.%u.%u\r\n",
         dns_query_rsp.ip.v4.bytes[0],
         dns_query_rsp.ip.v4.bytes[1],
         dns_query_rsp.ip.v4.bytes[2],
         dns_query_rsp.ip.v4.bytes[3]);

  for (sock_id = 0; sock_id < MAX_SOCKET; sock_id++) {
    server_addr[sock_id].sin_family = AF_INET; //! Set server address family
    if (sock_id == 0) {
      server_addr[sock_id].sin_port        = SERVER_PORT1;         //! Set server port number
      server_addr[sock_id].sin_addr.s_addr = server_address.value; //! Set IP address to localhost
    }
    if (sock_id == 1) {
      server_addr[sock_id].sin_port        = SERVER_PORT2;         //! Set server port number
      server_addr[sock_id].sin_addr.s_addr = server_address.value; //! Set IP address to localhost
    }
    if (sock_id == 2) {
      server_addr[sock_id].sin_port        = SERVER_PORT3;              //! Set server port number
      server_addr[sock_id].sin_addr.s_addr = dns_query_rsp.ip.v4.value; //! Set IP address to cloud server
    }
  }

  //! Connect to server socket
  for (sock_id = 0; sock_id < MAX_SOCKET; sock_id++) {
    socket_status =
      connect(client_socket[sock_id], (struct sockaddr *)&server_addr[sock_id], sizeof(server_addr[sock_id]));
    if (socket_status < 0) {
      printf("\r\n%d SSL Socket Connect failed with bsd error: %d\r\n", sock_id, errno);
      close(client_socket[sock_id]);
      return SL_STATUS_FAIL;
    }
    printf("\r\n%d Socket Connect Success\r\n", sock_id + 1);
  }

  // Fill send buffer
  for (uint16_t i = 0; i < MAX_DATA_LENGTH; i++) {
    send_buffer[i] = i % 10;
  }

  for (sock_id = 0; sock_id < MAX_SOCKET; sock_id++) {
    bytes_sent = send(client_socket[sock_id], &send_buffer, sizeof(send_buffer), 0);
    if (bytes_sent < 0) {
      if (errno == ENOBUFS) {
        sock_id--;
        continue;
      }
      printf("\r\nSend failed on client_socket %d with bsd error : %d\r\n", sock_id, errno);
      return SL_STATUS_FAIL;
    }
  }

  printf("\r\nData transfer on 3 sockets is completed\r\n");

  for (sock_id = 0; sock_id < MAX_SOCKET; sock_id++) {
    close(client_socket[sock_id]);
  }

  printf("\r\nSocket Close Success\r\n");

  return SL_STATUS_OK;
}
