/*******************************************************************************
* @file  wifi_app.c
* @brief
*******************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
******************************************************************************/
/*************************************************************************
 *
 */

/*================================================================================
 * @brief : This file contains example application for WLAN Station BLE Throughput
 * @section Description :
 * This application demonstrates how to configure SiLabs module in UDP client/
 * server or TCP client/server to measure throughput.
 =================================================================================*/

/*=======================================================================*/
//  ! INCLUDES
/*=======================================================================*/

//! SL Wi-Fi SDK includes
#include "sl_constants.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "sl_net.h"
#include "sl_net_si91x.h"
#include "sl_utility.h"
#include "sl_si91x_driver.h"
#include "sl_si91x_socket_support.h"

#include "errno.h"
#include "socket.h"
#include "sl_si91x_socket_support.h"

#include "cmsis_os2.h"
#include <rsi_common_apis.h>
#include <string.h>
#include "stdlib.h"
#include "wifi_config.h"

#include "sl_si91x_socket_utility.h"
#include "sl_si91x_socket_constants.h"
#include "sl_si91x_socket.h"
#include "sl_si91x_socket_types.h"
#include "sl_si91x_core_utilities.h"
#ifdef SLI_SI91X_MCU_INTERFACE
#include "rsi_rom_clks.h"
#endif

/*=======================================================================*/
//   ! MACROS
/*=======================================================================*/
#define DHCP_HOST_NAME    NULL
#define TIMEOUT_MS        25000
#define WIFI_SCAN_TIMEOUT 10000

#define BYTES_TO_RECEIVE           (1 << 28) //256MB
#define TEST_TIMEOUT               (30000)   //30sec
#define BACK_LOG                   1
#define SL_HIGH_PERFORMANCE_SOCKET BIT(7)

#if ENABLE_TLS
#include "cacert.pem.h" //! Include TLS CA certificate
// Load certificate to device flash :
// Certificate could be loaded once and need not be loaded for every boot up
#endif

/*=======================================================================*/
//   ! GLOBAL VARIABLES
/*=======================================================================*/
//! Throughput parameters
uint8_t data_buffer[BUFFER_SIZE];
volatile uint8_t wlan_data_rcv;
volatile uint8_t wlan_completed;

/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/
extern osSemaphoreId_t ble_conn_sem;

/*=======================================================================*/
//   ! PROCEDURES
/*=======================================================================*/
void wlan_throughput_task(void);
static void measure_and_print_throughput(uint32_t total_num_of_bytes, uint32_t test_timeout)
{
  float duration = ((test_timeout) / 1000);                    // ms to sec
  float result   = ((float)total_num_of_bytes * 8) / duration; // bytes to bps
  result         = (result / 1000000);                         // bps to Mbps
  LOG_PRINT("\r\nThroughput achieved @ %0.02f Mbps in %0.03f sec successfully\r\n", result, duration);
}
static sl_si91x_socket_config_t socket_config = {
  TOTAL_SOCKETS,                   // Total sockets
  TOTAL_TCP_SOCKETS,               // Total TCP sockets
  TOTAL_UDP_SOCKETS,               // Total UDP sockets
  TCP_TX_ONLY_SOCKETS,             // TCP TX only sockets
  TCP_RX_ONLY_SOCKETS,             // TCP RX only sockets
  UDP_TX_ONLY_SOCKETS,             // UDP TX only sockets
  UDP_RX_ONLY_SOCKETS,             // UDP RX only sockets
  TCP_RX_HIGH_PERFORMANCE_SOCKETS, // TCP RX high performance sockets
  TCP_RX_WINDOW_SIZE_CAP,          // TCP RX window size
  TCP_RX_WINDOW_DIV_FACTOR         // TCP RX window division factor
};

uint8_t has_data_received = 0;
uint32_t bytes_read       = 0;
uint32_t start            = 0;
uint32_t now              = 0;

void data_callback(uint32_t sock_no,
                   uint8_t *buffer,
                   uint32_t length,
                   const sl_si91x_socket_metadata_t *firmware_socket_response)
{
  UNUSED_PARAMETER(sock_no);
  UNUSED_PARAMETER(buffer);
  UNUSED_PARAMETER(firmware_socket_response);

  bytes_read += length;
  now = osKernelGetTickCount();
  if ((bytes_read > BYTES_TO_RECEIVE) || ((now - start) > TEST_TIMEOUT)) {
    has_data_received = 1;
  }
}
#if ENABLE_TLS && LOAD_CERTIFICATE
sl_status_t clear_and_load_certificates_in_flash(void)
{
  sl_status_t status = SL_STATUS_OK;

  // Load TLS CA certificate
  status =
    sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0), SL_NET_SIGNING_CERTIFICATE, cacert, sizeof(cacert) - 1);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nLoading TLS CA certificate in to FLASH Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
  LOG_PRINT("\r\nLoad TLS CA certificate at index %d Success\r\n", 0);

  return status;
}
#endif

sl_status_t join_callback_handler(sl_wifi_event_t event,
                                  sl_status_t status_code,
                                  char *result,
                                  uint32_t result_length,
                                  void *arg)
{
  UNUSED_PARAMETER(result);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    LOG_PRINT("F: Join Event received with %lu bytes payload\n", result_length);
    return status_code;
  }

  return SL_STATUS_OK;
}

void wifi_app_callbacks_init(void)
{
  sl_wifi_set_join_callback_v2(join_callback_handler, NULL);
}

/*====================================================*/
/**
 * @fn         int32_t application(void)
 * @brief      This function explains how to scan, connect and tx/rx wlan packets from remote device
 * @param[in]  void
 * @return     int32_t
 * @section description
 *====================================================*/
int32_t rsi_wlan_app_task()
{
  int32_t status                              = RSI_SUCCESS;
  sl_net_ip_configuration_t ip_address        = { 0 };
  sl_wifi_client_configuration_t access_point = { 0 };

  wifi_app_callbacks_init();

#if TX_RX_RATIO_ENABLE // FixMe
  status = rsi_wlan_buffer_config();
  if (status != RSI_SUCCESS) {
    return status;
  }
#endif

  osSemaphoreAcquire(ble_conn_sem, osWaitForever);

  // Connect to an Access point
  sl_wifi_credential_t cred  = { 0 };
  sl_wifi_credential_id_t id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;

  cred.type = SL_WIFI_PSK_CREDENTIAL;
  memcpy(cred.psk.value, PSK, strlen((char *)PSK));

  status = sl_net_set_credential(id, SL_NET_WIFI_PSK, PSK, strlen((char *)PSK));
  if (SL_STATUS_OK == status) {
    LOG_PRINT("Credentials set, id : %lu\n", id);

    access_point.ssid.length = strlen((char *)SSID);
    memcpy(access_point.ssid.value, SSID, access_point.ssid.length);
    access_point.security      = SECURITY_TYPE;
    access_point.encryption    = SL_WIFI_DEFAULT_ENCRYPTION;
    access_point.credential_id = id;

    LOG_PRINT("SSID %s\n", access_point.ssid.value);
    status = sl_wifi_connect(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &access_point, TIMEOUT_MS);
  }
  if (status != RSI_SUCCESS) {
    LOG_PRINT("WLAN connection failed %lx\r\n", status);
    return status;
  } else {
    LOG_PRINT("WLAN connected\r\n");
  }

  //! Configure IP
  ip_address.type      = SL_IPV4;
  ip_address.mode      = SL_IP_MANAGEMENT_DHCP;
  ip_address.host_name = DHCP_HOST_NAME;

  status = sl_si91x_configure_ip_address(&ip_address, SL_SI91X_WIFI_CLIENT_VAP_ID);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("IP Config failed %lx\r\n", status);
    return status;
  }
  sl_ip_address_t ip = { 0 };
  ip.type            = ip_address.type;
  ip.ip.v4.value     = ip_address.ip.v4.ip_address.value;
  print_sl_ip_address(&ip);

  wlan_throughput_task();

  return 0;
}

void send_data_to_udp_server(void)
{
  int client_socket                 = -1;
  uint32_t total_bytes_sent         = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  int sent_bytes                    = 1;
  uint32_t start                    = 0;
  uint32_t now                      = 0;
  uint32_t packets                  = 0;

  server_address.sin_family = AF_INET;
  server_address.sin_port   = SERVER_PORT;
  sl_net_inet_addr(SERVER_IP_ADDRESS, &server_address.sin_addr.s_addr);

  client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (client_socket < 0) {
    LOG_PRINT("\r\nSocket creation failed with BSD error: %d\r\n", errno);
    return;
  }
  LOG_PRINT("\r\nSocket ID : %d\r\n", client_socket);

  LOG_PRINT("\r\nUDP_TX Throughput test start\r\n");
  start = osKernelGetTickCount();
  while (1) {
    sent_bytes =
      sendto(client_socket, data_buffer, UDP_BUFFER_SIZE, 0, (struct sockaddr *)&server_address, socket_length);
    if (sent_bytes < 0) {
      if (errno == ENOBUFS)
        continue;
      LOG_PRINT("\r\nsendto failed with bsd error:%d\r\n", errno);
      close(client_socket);
      return;
    } else if (sent_bytes > 0) {
      total_bytes_sent = total_bytes_sent + sent_bytes;
      packets++;
    }

    now = osKernelGetTickCount();
    if ((now - start) >= TEST_TIMEOUT) {
      LOG_PRINT("\r\nNumber of packets sent : %ld\r\n", packets);
      LOG_PRINT("\r\nTotal bytes sent : %ld\r\n", total_bytes_sent);
      measure_and_print_throughput(total_bytes_sent, (now - start));
#if CONTINUOUS_THROUGHPUT
      total_bytes_sent = 0;
      packets          = 0;
      start            = osKernelGetTickCount();
#else
      break;
#endif
    }
  }

  LOG_PRINT("\r\nUDP_TX Throughput test finished\r\n");
  close(client_socket);
}
#if SOCKET_ASYNC_FEATURE
void receive_data_from_udp_client(void)
{
  int client_socket = -1;

  int socket_return_value           = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  client_socket                     = sl_si91x_socket_async(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &data_callback);
  if (client_socket < 0) {
    LOG_PRINT("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  LOG_PRINT("\r\nSocket ID : %d\r\n", client_socket);

  server_address.sin_family = AF_INET;
  server_address.sin_port   = DEVICE_PORT;

  socket_return_value = sl_si91x_bind(client_socket, (struct sockaddr *)&server_address, socket_length);

  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }
  LOG_PRINT("\r\nListening on Local Port %d\r\n", DEVICE_PORT);

  LOG_PRINT("\r\nUDP_RX Async Throughput test start\r\n");

#if CONTINUOUS_THROUGHPUT
  // Continuous throughput mode - loop forever with interval reporting
  while (1) {
    // Reset for each interval
    has_data_received = 0;
    bytes_read        = 0;
    start             = osKernelGetTickCount();

    // Wait for TEST_TIMEOUT interval or max bytes
    while (!has_data_received) {
      osThreadYield();
    }

    now = osKernelGetTickCount();
    LOG_PRINT("\r\nTotal bytes received : %ld\r\n", bytes_read);
    measure_and_print_throughput(bytes_read, (now - start));
  }
#else
  // Single measurement mode
  start = osKernelGetTickCount();
  while (!has_data_received) {
    osThreadYield();
  }
  now = osKernelGetTickCount();
  LOG_PRINT("\r\nUDP_RX Async Throughput test finished\r\n");
  LOG_PRINT("\r\nTotal bytes received : %ld\r\n", bytes_read);

  measure_and_print_throughput(bytes_read, (now - start));

  close(client_socket);
#endif
}
#else
void receive_data_from_udp_client(void)
{
  sl_status_t status = SL_STATUS_OK;
  int client_socket = -1;
  uint32_t total_bytes_received = 0;
  int socket_return_value = 0;
  int read_bytes = 1;
  uint32_t start = 0;
  uint32_t now = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length = sizeof(struct sockaddr_in);

  client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (client_socket < 0) {
    LOG_PRINT("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  LOG_PRINT("\r\nSocket ID : %d\r\n", client_socket);

  server_address.sin_family = AF_INET;
  server_address.sin_port = DEVICE_PORT;

  socket_return_value = bind(client_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSocket bind failed with BSD error: %d\r\n", errno);
    close(client_socket);
    return;
  }
  LOG_PRINT("\r\nListening on Local Port %d\r\n", DEVICE_PORT);

  LOG_PRINT("\r\nUDP_RX Throughput test start\r\n");
  start = osKernelGetTickCount();
  while (1) {
    read_bytes = recvfrom(client_socket, data_buffer, sizeof(data_buffer), 0, NULL, NULL);
    if (read_bytes < 0) {
      if (errno == 0) {
        // get the error code returned by the firmware
        status = sl_wifi_get_saved_firmware_status();
        if (status == SL_STATUS_SI91X_MEMORY_FAILED_FROM_MODULE) {
          continue;
        } else {
          printf("\r\nrecv failed with BSD error = %d and status = 0x%lx\r\n", errno, status);
        }
      } else {
        printf("\r\nrecv failed with BSD error = %d\r\n", errno);
      }
      break;
    }
    total_bytes_received = total_bytes_received + read_bytes;

    now = osKernelGetTickCount();
    if ((now - start) > TEST_TIMEOUT) {
      LOG_PRINT("\r\nTotal bytes received : %ld\r\n", total_bytes_received);
      measure_and_print_throughput(total_bytes_received, (now - start));
#if CONTINUOUS_THROUGHPUT
      total_bytes_received = 0;
      start = osKernelGetTickCount();
#else
      break;
#endif
    }
  }

  LOG_PRINT("\r\nUDP_RX Throughput test finished\r\n");
  close(client_socket);
  return;
}
#endif
void send_data_to_tcp_server(void)
{
  int client_socket                 = -1;
  uint32_t total_bytes_sent         = 0;
  int socket_return_value           = 0;
  int sent_bytes                    = 1;
  uint32_t start                    = 0;
  uint32_t now                      = 0;
  uint32_t packets                  = 0;
  struct sockaddr_in server_address = { 0 };

  server_address.sin_family = AF_INET;
  server_address.sin_port   = SERVER_PORT;
  sl_net_inet_addr(SERVER_IP_ADDRESS, &server_address.sin_addr.s_addr);

  client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_socket < 0) {
    LOG_PRINT("\r\nSocket Create failed with BSD error: %d\r\n", errno);
    return;
  }
  LOG_PRINT("\r\nSocket ID : %d\r\n", client_socket);

  socket_return_value = connect(client_socket, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in));
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSocket Connect failed with BSD error: %d\r\n", errno);
    close(client_socket);
    return;
  }
  LOG_PRINT("\r\nSocket connected to TCP server\r\n");

  LOG_PRINT("\r\nTCP_TX Throughput test start\r\n");
  start = osKernelGetTickCount();
  while (1) {
    sent_bytes = send(client_socket, data_buffer, TCP_BUFFER_SIZE, 0);
    if (sent_bytes < 0) {
      if (errno == ENOBUFS)
        continue;
      LOG_PRINT("\nTCP send failed\n");
      break;
    } else if (sent_bytes > 0) {
      total_bytes_sent = total_bytes_sent + sent_bytes;
      packets++;
    }
    now = osKernelGetTickCount();
    if ((now - start) >= TEST_TIMEOUT) {
      LOG_PRINT("\r\nNumber of packets sent : %ld\r\n", packets);
      LOG_PRINT("\r\nTotal bytes sent : %ld\r\n", total_bytes_sent);
      measure_and_print_throughput(total_bytes_sent, (now - start));
#if CONTINUOUS_THROUGHPUT
      total_bytes_sent = 0;
      packets          = 0;
      start            = osKernelGetTickCount();
#else
      break;
#endif
    }
  }

  LOG_PRINT("\r\nTCP_TX Throughput test finished\r\n");
  close(client_socket);

  return;
}
#if SOCKET_ASYNC_FEATURE
void receive_data_from_tcp_client(void)
{
  int server_socket                 = -1;
  int client_socket                 = -1;
  int socket_return_value           = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  uint32_t high_performance_socket  = SL_HIGH_PERFORMANCE_SOCKET;

  sl_status_t status = sl_si91x_config_socket(socket_config);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("Socket config failed: %ld\r\n", status);
  }
  LOG_PRINT("\r\nSocket config Done\r\n");

  server_socket = sl_si91x_socket_async(AF_INET, SOCK_STREAM, IPPROTO_TCP, &data_callback);
  if (server_socket < 0) {
    LOG_PRINT("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  LOG_PRINT("\r\nServer Socket ID : %d\r\n", server_socket);

  socket_return_value = sl_si91x_setsockopt(server_socket,
                                            SOL_SOCKET,
                                            SL_SI91X_SO_HIGH_PERFORMANCE_SOCKET,
                                            &high_performance_socket,
                                            sizeof(high_performance_socket));
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSet Socket option failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }
  server_address.sin_family = AF_INET;
  server_address.sin_port   = DEVICE_PORT;

  socket_return_value = sl_si91x_bind(server_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }

  socket_return_value = sl_si91x_listen(server_socket, BACK_LOG);
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSocket listen failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }
  LOG_PRINT("\r\nListening on Local Port : %d\r\n", DEVICE_PORT);

  client_socket = sl_si91x_accept(server_socket, NULL, 0);
  if (client_socket < 0) {
    LOG_PRINT("\r\nSocket accept failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }
  LOG_PRINT("\r\nClient Socket ID : %d\r\n", client_socket);

  LOG_PRINT("\r\nTCP_RX Async Throughput test start\r\n");

#if CONTINUOUS_THROUGHPUT
  // Continuous throughput mode - loop forever with interval reporting
  while (1) {
    // Reset for each interval
    has_data_received = 0;
    bytes_read        = 0;
    start             = osKernelGetTickCount();

    // Wait for TEST_TIMEOUT interval or max bytes
    while (!has_data_received) {
      osThreadYield();
    }

    now = osKernelGetTickCount();
    LOG_PRINT("\r\nTotal bytes received : %ld\r\n", bytes_read);
    measure_and_print_throughput(bytes_read, (now - start));
  }
#else
  // Single measurement mode
  start = osKernelGetTickCount();

  while (!has_data_received) {
    osThreadYield();
  }

  now = osKernelGetTickCount();

  LOG_PRINT("\r\nTCP_RX  Async Throughput test finished\r\n");
  LOG_PRINT("\r\nTotal bytes received : %ld\r\n", bytes_read);

  close(client_socket);
  close(server_socket);
  measure_and_print_throughput(bytes_read, (now - start));
#endif
}
#else
void receive_data_from_tcp_client(void)
{
  int server_socket = -1;
  int client_socket = -1;
  int socket_return_value = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length = sizeof(struct sockaddr_in);
  uint8_t high_performance_socket = SL_HIGH_PERFORMANCE_SOCKET;
  uint32_t total_bytes_received = 0;
  uint32_t start = 0;
  uint32_t now = 0;
  int read_bytes = 1;
  sl_status_t status = SL_STATUS_FAIL;

  server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_socket < 0) {
    LOG_PRINT("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  LOG_PRINT("\r\nServer Socket ID : %d\r\n", server_socket);

  socket_return_value = setsockopt(server_socket,
                                   SOL_SOCKET,
                                   SL_SO_HIGH_PERFORMANCE_SOCKET,
                                   &high_performance_socket,
                                   sizeof(high_performance_socket));
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSet Socket option failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }
  server_address.sin_family = AF_INET;
  server_address.sin_port = DEVICE_PORT;

  socket_return_value = bind(server_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }

  socket_return_value = listen(server_socket, BACK_LOG);
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSocket listen failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }
  LOG_PRINT("\r\nListening on Local Port : %d\r\n", DEVICE_PORT);

  client_socket = accept(server_socket, NULL, NULL);
  if (client_socket < 0) {
    LOG_PRINT("\r\nSocket accept failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }
  LOG_PRINT("\r\nClient Socket ID : %d\r\n", client_socket);

  LOG_PRINT("\r\nTCP_RX Throughput test start\r\n");
  start = osKernelGetTickCount();
  while (1) {
    read_bytes = recv(client_socket, data_buffer, sizeof(data_buffer), 0);
    if (read_bytes < 0) {
      if (errno == 0) {
        // get the error code returned by the firmware
        status = sl_wifi_get_saved_firmware_status();
        if (status == SL_STATUS_SI91X_MEMORY_FAILED_FROM_MODULE) {
          continue;
        } else {
          LOG_PRINT("\r\nTCP recv failed with BSD error = %d and status = 0x%lx\r\n", errno, status);
        }
      } else if (errno == ENOTCONN) {
        LOG_PRINT("\r\nRemote server terminated\r\n");
      } else {
        LOG_PRINT("\r\nTLS recv failed with BSD error = %d\r\n", errno);
      }
      close(client_socket);
      close(server_socket);
      return;
    }

    total_bytes_received = total_bytes_received + read_bytes;
    now = osKernelGetTickCount();
    if ((now - start) > TEST_TIMEOUT) {
      LOG_PRINT("\r\nTotal bytes received : %ld\r\n", total_bytes_received);
      measure_and_print_throughput(total_bytes_received, (now - start));
#if CONTINUOUS_THROUGHPUT
      total_bytes_received = 0;
      start = osKernelGetTickCount();
#else
      break;
#endif
    }
  }

  LOG_PRINT("\r\nTCP_RX Throughput test finished\r\n");
  close(client_socket);
  close(server_socket);
  return;
}
#endif

void send_data_to_tls_server(void)
{
  int client_socket                 = -1;
  int socket_return_value           = 0;
  uint32_t total_bytes_sent         = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  uint32_t now                      = 0;
  uint32_t start                    = 0;
  uint32_t packets                  = 0;
  int sent_bytes                    = 0;

  client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_socket < 0) {
    LOG_PRINT("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  LOG_PRINT("\r\nSocket ID : %d\r\n", client_socket);

  socket_return_value = setsockopt(client_socket, SOL_TCP, TCP_ULP, TLS, sizeof(TLS));
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSet Socket option failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }

  server_address.sin_family = AF_INET;
  server_address.sin_port   = TLS_SERVER_PORT;
  sl_net_inet_addr(SERVER_IP_ADDRESS, &server_address.sin_addr.s_addr);

  socket_return_value = connect(client_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSocket Connect failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }
  LOG_PRINT("\r\nSocket connected to TLS server\r\n");

  LOG_PRINT("\r\nTLS_TX Throughput test start\r\n");
  start = osKernelGetTickCount();
  while (1) {
    sent_bytes = send(client_socket, data_buffer, TLS_BUFFER_SIZE, 0);
    if (sent_bytes < 0) {
      if (errno == ENOBUFS)
        continue;
      break;
    } else if (sent_bytes > 0) {
      total_bytes_sent = total_bytes_sent + sent_bytes;
      packets++;
    }
    now = osKernelGetTickCount();
    if ((now - start) > TEST_TIMEOUT) {
      LOG_PRINT("\r\nTotal bytes sent : %ld\r\n", total_bytes_sent);
      LOG_PRINT("\r\nNumber of packets sent : %ld\r\n", packets);
      measure_and_print_throughput(total_bytes_sent, (now - start));
#if CONTINUOUS_THROUGHPUT
      total_bytes_sent = 0;
      packets          = 0;
      start            = osKernelGetTickCount();
#else
      break;
#endif
    }
  }

  LOG_PRINT("\r\nTLS_TX Throughput test finished\r\n");
  close(client_socket);

  return;
}

void receive_data_from_tls_client(void)
{
  int client_socket                 = -1;
  int socket_return_value           = 0;
  uint32_t total_bytes_received     = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  uint8_t high_performance_socket   = SL_HIGH_PERFORMANCE_SOCKET;
  sl_status_t status                = SL_STATUS_FAIL;

  client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_socket < 0) {
    LOG_PRINT("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  LOG_PRINT("\r\nSocket ID : %d\r\n", client_socket);

  socket_return_value = setsockopt(client_socket, SOL_TCP, TCP_ULP, TLS, sizeof(TLS));
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSet Socket option failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }

  socket_return_value = setsockopt(client_socket,
                                   SOL_SOCKET,
                                   SL_SO_HIGH_PERFORMANCE_SOCKET,
                                   &high_performance_socket,
                                   sizeof(high_performance_socket));
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSet Socket option failed with BSD error: %d\r\n", errno);
    close(client_socket);
    return;
  }

  server_address.sin_family = AF_INET;
  server_address.sin_port   = TLS_SERVER_PORT;
  sl_net_inet_addr(SERVER_IP_ADDRESS, &server_address.sin_addr.s_addr);

  socket_return_value = connect(client_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSocket Connect failed with BSD error: %d\r\n", errno);
    close(client_socket);
    return;
  }
  LOG_PRINT("\r\nSocket connected to TLS server\r\n");

  LOG_PRINT("\r\nTLS_RX Throughput test start\r\n");
  uint32_t start = osKernelGetTickCount();
  uint32_t now   = start;
  int read_bytes = 1;
  while (1) {
    read_bytes = recv(client_socket, data_buffer, sizeof(data_buffer), 0);
    if (read_bytes < 0) {
      if (errno == 0) {
        // get the error code returned by the firmware
        status = sl_wifi_get_saved_firmware_status();
        if (status == SL_STATUS_SI91X_MEMORY_FAILED_FROM_MODULE) {
          continue;
        } else {
          LOG_PRINT("\r\nTLS recv failed with BSD error = %d and status = 0x%lx\r\n", errno, status);
        }
      } else if (errno == ENOTCONN) {
        LOG_PRINT("\r\nRemote server terminated\r\n");
      } else {
        LOG_PRINT("\r\nTLS recv failed with BSD error = %d\r\n", errno);
      }
      close(client_socket);
      return;
    }

    total_bytes_received = total_bytes_received + read_bytes;
    now                  = osKernelGetTickCount();
    if ((now - start) > TEST_TIMEOUT) {
      LOG_PRINT("\r\nTotal bytes received : %ld\r\n", total_bytes_received);
      measure_and_print_throughput(total_bytes_received, (now - start));
#if CONTINUOUS_THROUGHPUT
      total_bytes_received = 0;
      start                = osKernelGetTickCount();
#else
      break;
#endif
    }
  }

  LOG_PRINT("\r\nTLS_RX Throughput test finished\r\n");
  close(client_socket);
  return;
}

void wlan_throughput_task(void)
{

  for (size_t i = 0; i < sizeof(data_buffer); i++)
    data_buffer[i] = 'A' + (i % 26);
  LOG_PRINT("\r\nin wlan_throughput_task\n");
  switch (THROUGHPUT_TYPE) {
    case UDP_TX: {
      send_data_to_udp_server();
    } break;
    case UDP_RX: {
      receive_data_from_udp_client();
    } break;
    case TCP_TX: {
      send_data_to_tcp_server();
    } break;
    case TCP_RX: {
      receive_data_from_tcp_client();
    } break;
    case TLS_TX: {
      send_data_to_tls_server();
    } break;
    case TLS_RX: {
      receive_data_from_tls_client();
    } break;

      return;

      LOG_PRINT("In wlan task\n");
      osDelay(2000);
  }
}
