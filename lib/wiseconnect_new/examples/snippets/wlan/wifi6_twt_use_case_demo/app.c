/***************************************************************************/ /**
 * @file
 * @brief  Twt Use Case Remote App
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
#include <string.h>
#include <stdint.h>
#include "errno.h"
#include "sl_wifi_callback_framework.h"
#include "sl_status.h"
#include "sl_board_configuration.h"
#include "cmsis_os2.h"
#include "string.h"
#include "sl_wifi.h"
#include "socket.h"
#include "sl_utility.h"
#include "sl_net.h"
#include "sl_ip_types.h"
#include "sl_string.h"
#include "sl_net_si91x.h"
#include "sl_net_wifi_types.h"
#include "sl_si91x_socket_utility.h"
#include "sl_si91x_socket_constants.h"
#include "sl_si91x_driver.h"
#include "sl_si91x_socket.h"
#include "sl_wifi_types.h"
#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_power_manager.h"
#include "sl_si91x_m4_ps.h"
#include "rsi_rom_clks.h"
#endif
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define DOOR_LOCK_SIMULATION 1 // 1- for Door Lock Simulation and 0- for Camera Simulation
#define BACK_LOG             1
#define TCP_SERVER_PORT      5001
#define UDP_SERVER_PORT      5002
#define SERVER_IP_UDP        "192.168.1.10"
#define SERVER_IP_TCP        "192.168.1.112"
#if DOOR_LOCK_SIMULATION
#define BUF_SIZE 1460
#else
#define BUF_SIZE 1470
#endif
#define EXCPECTED_TX_THROUGHPUT_KBPS 1000
#define NUMBER_OF_PACKETS            (((EXCPECTED_TX_THROUGHPUT_KBPS * 1000) + (BUF_SIZE * 8)) / (BUF_SIZE * 8))
#define SL_HIGH_PERFORMANCE_SOCKET   BIT(7)
#define TWT_AUTO_CONFIG              1
#define TWT_SCAN_TIMEOUT             10000
#define ENABLE_NWP_POWER_SAVE        1

// Use case based TWT selection params
#define TWT_RX_LATENCY                       5000
#define DEVICE_AVG_THROUGHPUT                20000
#define ESTIMATE_EXTRA_WAKE_DURATION_PERCENT 0
#define TWT_TOLERABLE_DEVIATION              10
#define TWT_DEFAULT_WAKE_INTERVAL_MS         1024     // in milli seconds
#define TWT_DEFAULT_WAKE_DURATION_MS         8        // in milli seconds
#define MAX_TX_AND_RX_LATENCY_LIMIT          22118400 // 6hrs in milli seconds
#define MAX_BEACON_WAKE_UP_AFTER_SP \
  2 // The number of beacons after the service period completion for which the module wakes up and listens for any pending RX.

#define KEEP_ALIVE_TIMEOUT        SL_WIFI_DEFAULT_KEEP_ALIVE_TIMEOUT
#define AUTH_ASSOCIATION_TIMEOUT  SL_WIFI_DEFAULT_AUTH_ASSOCIATION_TIMEOUT
#define ACTIVE_CHANNEL_SCAN_TIME  SL_WIFI_DEFAULT_ACTIVE_CHANNEL_SCAN_TIME
#define PASSIVE_CHANNEL_SCAN_TIME SL_WIFI_DEFAULT_PASSIVE_CHANNEL_SCAN_TIME

static const sl_wifi_device_configuration_t twt_client_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
#ifdef SLI_SI91X_MCU_INTERFACE
                      | SL_WIFI_FEAT_WPS_DISABLE
#endif
                      ),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID
                                              | SL_SI91X_TCP_IP_FEAT_SSL),
                   .custom_feature_bit_map = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map = (SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK
                                                  | SL_SI91X_EXT_FEAT_DISABLE_DEBUG_PRINTS | MEMORY_CONFIG
#ifdef SLI_SI917
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map = SL_SI91X_BT_RF_TYPE,
                   .ext_tcp_ip_feature_bit_map =
                     SL_SI91X_CONFIG_FEAT_EXTENTION_VALID
                     | (SL_SI91X_EXT_FEAT_HTTP_OTAF_SUPPORT | SL_SI91X_EXT_TCP_IP_SSL_16K_RECORD),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | SL_WIFI_ENABLE_ENHANCED_MAX_PSP) }
};

/******************************************************
 *               Variable Definitions
 ******************************************************/
const osThreadAttr_t thread_attributes = {
  .name       = "application_thread",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = 0,
  .tz_module  = 0,
  .reserved   = 0,
};

sl_ip_address_t ip_address           = { 0 };
sl_net_wifi_client_profile_t profile = { 0 };
int tcp_client_socket = -1, udp_client_socket = -1;

uint32_t start_rx = 0, start_rtt = 0, end_rtt = 0;
volatile uint32_t num_pkts  = 0;
int32_t packet_count        = 0;
volatile uint64_t num_bytes = 0;
int8_t send_buf[BUF_SIZE];

osSemaphoreId_t data_semaphore;
volatile bool data_received_flag = false;

sl_wifi_twt_request_t default_twt_setup_configuration = {
  .twt_enable              = 1,
  .twt_flow_id             = 1,
  .wake_duration           = 0x60,
  .wake_duration_unit      = 0,
  .wake_duration_tol       = 0x60,
  .wake_int_exp            = 13,
  .wake_int_exp_tol        = 13,
  .wake_int_mantissa       = 0x1D4C,
  .wake_int_mantissa_tol   = 0x1D4C,
  .implicit_twt            = 1,
  .un_announced_twt        = 1,
  .triggered_twt           = 0,
  .twt_channel             = 0,
  .twt_protection          = 0,
  .restrict_tx_outside_tsp = 1,
  .twt_retry_limit         = 6,
  .twt_retry_interval      = 10,
  .req_type                = 1,
  .negotiation_type        = 0,
};

sl_wifi_twt_selection_t default_twt_selection_configuration = {
  .twt_enable                            = 1,
  .average_tx_throughput                 = EXCPECTED_TX_THROUGHPUT_KBPS,
  .tx_latency                            = 0,
  .rx_latency                            = TWT_RX_LATENCY,
  .device_average_throughput             = DEVICE_AVG_THROUGHPUT,
  .estimated_extra_wake_duration_percent = ESTIMATE_EXTRA_WAKE_DURATION_PERCENT,
  .twt_tolerable_deviation               = TWT_TOLERABLE_DEVIATION,
  .default_wake_interval_ms              = TWT_DEFAULT_WAKE_INTERVAL_MS,
  .default_minimum_wake_duration_ms      = TWT_DEFAULT_WAKE_DURATION_MS,
  .beacon_wake_up_count_after_sp         = MAX_BEACON_WAKE_UP_AFTER_SP
};

sl_wifi_timeout_t timeout_configuration = { .keep_alive_timeout_value       = KEEP_ALIVE_TIMEOUT,
                                            .auth_assoc_timeout_value       = AUTH_ASSOCIATION_TIMEOUT,
                                            .active_chan_scan_timeout_value = ACTIVE_CHANNEL_SCAN_TIME,
                                            .passive_scan_timeout_value     = PASSIVE_CHANNEL_SCAN_TIME };

volatile sl_status_t callback_status = SL_STATUS_OK;

/******************************************************
  *               Function Declarations
  ******************************************************/
void application_start();
sl_status_t create_tcp_socket(void);
sl_status_t send_udp_data(void);
sl_status_t receive_and_send_data(void);
static sl_status_t twt_callback_handler(sl_wifi_event_t event,
                                        sl_status_t status_code,
                                        sl_wifi_twt_response_t *result,
                                        uint32_t result_length,
                                        void *arg);

/******************************************************
  *               Function Definitions
  ******************************************************/

void data_callback(uint32_t sock_no,
                   uint8_t *buffer,
                   uint32_t length,
                   const sl_si91x_socket_metadata_t *firmware_socket_response)
{
  UNUSED_PARAMETER(firmware_socket_response);
  UNUSED_PARAMETER(sock_no);
  uint32_t i;

  printf("Command length : %ld\r\n", length);
  printf("Command Received from remote app is:\r\n");
  printf("\"");
  for (i = 0; i < length; i++) {
    printf("%c", buffer[i]);
  }
  printf("\"\r\n");
  data_received_flag = true;
  osSemaphoreRelease(data_semaphore);
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

void application_start()
{
  sl_status_t status;
  sl_wifi_performance_profile_v2_t performance_profile = { 0 };
  sl_wifi_firmware_version_t version                   = { 0 };
  sl_mac_address_t mac_addr                            = { 0 };
  data_semaphore                                       = osSemaphoreNew(1, 0, NULL);

  sl_si91x_set_timeout(&timeout_configuration);
  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &twt_client_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("Wi-Fi Init Done\r\n");

#ifdef SLI_SI91X_MCU_INTERFACE
  uint8_t xtal_enable = 1;
  status              = sl_si91x_m4_ta_secure_handshake(SL_SI91X_ENABLE_XTAL, 1, &xtal_enable, 0, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to bring m4_ta_secure_handshake: 0x%lx\r\n", status);
    return;
  }
  printf("m4_ta_secure_handshake Success\r\n");
#endif

  status = sl_wifi_get_firmware_version(&version);
  if (status != SL_STATUS_OK) {
    printf("Failed to bring m4_ta_secure_handshake: 0x%lx\r\n", status);
    return;
  } else {
    print_firmware_version(&version);
  }

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, 0);
  if (status != SL_STATUS_OK) {
    printf("Failed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
    return;
  }
  printf("Wi-Fi Client Connected\r\n");

  status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_addr);
  if (status != SL_STATUS_OK) {
    printf("Failed to get MAC address: 0x%lx\r\n", status);
    return;
  }
  printf("MAC Address: %x:%x:%x:%x:%x:%x\r\n",
         mac_addr.octet[0],
         mac_addr.octet[1],
         mac_addr.octet[2],
         mac_addr.octet[3],
         mac_addr.octet[4],
         mac_addr.octet[5]);

  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile);
  if (status != SL_STATUS_OK) {
    printf("Failed to get firmware version: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nSuccess to get client profile\r\n");

  ip_address.type = SL_IPV4;
  memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
  print_sl_ip_address(&ip_address);

#if !DOOR_LOCK_SIMULATION
  status = send_udp_data();
  if (status != SL_STATUS_OK) {
    printf("\r\nError while doing UDP TX: 0x%lx \r\n", status);
    return;
  }
  printf("\r\nUDP TX complete\r\n");
#endif

  status = create_tcp_socket();
  if (status != SL_STATUS_OK) {
    printf("\r\nError while creating TCP Socket: 0x%lx \r\n", status);
    return;
  }
  printf("TCP Socket Creation done\r\n");

  sl_wifi_set_twt_config_callback_v2(twt_callback_handler, NULL);
  if (TWT_AUTO_CONFIG == 1) {
    performance_profile.twt_selection = default_twt_selection_configuration;
    status                            = sl_wifi_target_wake_time_auto_selection(&performance_profile.twt_selection);
  } else {
    performance_profile.twt_request = default_twt_setup_configuration;
    status                          = sl_wifi_enable_target_wake_time(&performance_profile.twt_request);
  }
  if (status != SL_STATUS_OK) {
    printf("\r\nError while configuring TWT: 0x%lx \r\n", status);
    return;
  }
  // A small delay is added so that the asynchronous response from TWT is printed in correct format.
  osDelay(100);

  status = sl_wifi_filter_broadcast(5000, 1, 1);
  if (status != SL_STATUS_OK) {
    printf("\r\nBroadcast Data Filter Failed: 0x%lx \r\n", status);
    return;
  }
  printf("Enabled Broadcast Data Filter\r\n");

#if ENABLE_NWP_POWER_SAVE
  performance_profile.profile = ASSOCIATED_POWER_SAVE_LOW_LATENCY;
  status                      = sl_wifi_set_performance_profile_v2(&performance_profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nPowersave Configuration Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
  printf("Associated Power Save Enabled\r\n");
#endif

  status = receive_and_send_data();
  if (status != SL_STATUS_OK) {
    printf("Send and Receive Data fail: 0x%lx \r\n", status);
    return;
  }
}

sl_status_t send_udp_data(void)
{
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  int32_t status                    = SL_STATUS_OK;
  int32_t packet_count              = 0;

  server_address.sin_family = AF_INET;
  server_address.sin_port   = UDP_SERVER_PORT;
  sl_net_inet_addr(SERVER_IP_UDP, &server_address.sin_addr.s_addr);

  udp_client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (udp_client_socket < 0) {
    printf("\r\nSocket creation failed with BSD error: %d\r\n", errno);
    return SL_STATUS_FAIL;
  }
  printf("Socket ID : %d\r\n", udp_client_socket);

  printf("Sending a UDP packet\r\n");
  while (packet_count < NUMBER_OF_PACKETS) {
    status = sendto(udp_client_socket,
                    (int8_t *)"Hello from UDP client!!!",
                    (sizeof("Hello from UDP client!!!") - 1),
                    0,
                    (struct sockaddr *)&server_address,
                    socket_length);
    if (status < 0) {
      if (errno == ENOBUFS)
        continue;
      printf("\r\nFailed to send data to UDP Server, Error Code : 0x%lX\r\n", status);
      close(udp_client_socket);
    }
    packet_count++;
  }
  packet_count = 0;
  return SL_STATUS_OK;
}

sl_status_t create_tcp_socket(void)
{
  struct sockaddr_in server_address = { 0 };
  uint32_t high_performance_socket  = SL_HIGH_PERFORMANCE_SOCKET;

  int socket_return_value = 0;

  tcp_client_socket = sl_si91x_socket_async(AF_INET, SOCK_STREAM, IPPROTO_TCP, &data_callback);
  if (tcp_client_socket < 0) {
    printf("\r\nTCP Socket creation failed with BSD error: %d\r\n", errno);
    return SL_STATUS_FAIL;
  }
  printf("\r\nTCP Client Socket ID : %d\r\n", tcp_client_socket);

  socket_return_value = sl_si91x_setsockopt(tcp_client_socket,
                                            SOL_SOCKET,
                                            SL_SI91X_SO_HIGH_PERFORMANCE_SOCKET,
                                            &high_performance_socket,
                                            sizeof(high_performance_socket));
  if (socket_return_value < 0) {
    printf("\r\nTCP Set Socket option failed with BSD error: %d\r\n", errno);
    close(tcp_client_socket);
    return SL_STATUS_FAIL;
  }
  printf("Set Socket Option Success\r\n");

  server_address.sin_family = AF_INET;
  server_address.sin_port   = TCP_SERVER_PORT;
  sl_net_inet_addr(SERVER_IP_TCP, &server_address.sin_addr.s_addr);

  socket_return_value = connect(tcp_client_socket, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in));
  if (socket_return_value < 0) {
    printf("\r\nSocket Connect failed with BSD error: %d\r\n", errno);
    close(tcp_client_socket);
    return SL_STATUS_FAIL;
  }
  printf("Socket connected to TCP server\r\n");

  return SL_STATUS_OK;
}

static sl_status_t twt_callback_handler(sl_wifi_event_t event,
                                        sl_status_t status_code,
                                        sl_wifi_twt_response_t *result,
                                        uint32_t result_length,
                                        void *arg)
{
  UNUSED_PARAMETER(result_length);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }

  switch (event) {
    case SL_WIFI_TWT_RESPONSE_EVENT:
      printf("\r\nTWT Setup success");
      break;
    case SL_WIFI_TWT_UNSOLICITED_SESSION_SUCCESS_EVENT:
      printf("\r\nUnsolicited TWT Setup success");
      break;
    case SL_WIFI_TWT_AP_REJECTED_EVENT:
      printf("\r\nTWT Setup Failed. TWT Setup rejected by AP");
      break;
    case SL_WIFI_TWT_OUT_OF_TOLERANCE_EVENT:
      printf("\r\nTWT Setup Failed. TWT response out of tolerance limits");
      break;
    case SL_WIFI_TWT_RESPONSE_NOT_MATCHED_EVENT:
      printf("\r\nTWT Setup Failed. TWT Response not matched with the request parameters");
      break;
    case SL_WIFI_TWT_UNSUPPORTED_RESPONSE_EVENT:
      printf("\r\nTWT Setup Failed. TWT Response Unsupported");
      break;
    case SL_WIFI_TWT_FAIL_MAX_RETRIES_REACHED_EVENT:
      printf("\r\nTWT Setup Failed. Max retries reached");
      break;
    case SL_WIFI_TWT_INACTIVE_DUE_TO_ROAMING_EVENT:
      printf("\r\nTWT session inactive due to roaming");
      break;
    case SL_WIFI_TWT_INACTIVE_DUE_TO_DISCONNECT_EVENT:
      printf("\r\nTWT session inactive due to wlan disconnection");
      break;
    case SL_WIFI_TWT_TEARDOWN_SUCCESS_EVENT:
      printf("\r\nTWT session teardown success");
      break;
    case SL_WIFI_TWT_AP_TEARDOWN_SUCCESS_EVENT:
      printf("\r\nTWT session teardown from AP");
      break;
    case SL_WIFI_TWT_INACTIVE_NO_AP_SUPPORT_EVENT:
      printf("\r\nConnected AP Does not support TWT");
      break;
    case SL_WIFI_RESCHEDULE_TWT_SUCCESS_EVENT:
      printf("\r\nTWT rescheduled");
      break;
    case SL_WIFI_TWT_INFO_FRAME_EXCHANGE_FAILED_EVENT:
      printf("\r\nTWT rescheduling failed due to a failure in the exchange of TWT information frames.");
      break;
    default:
      printf("TWT Setup Failed.");
  }
  if (event < SL_WIFI_TWT_TEARDOWN_SUCCESS_EVENT) {
    printf("\r\n wake duration : 0x%X", result->wake_duration);
    printf("\r\n wake_duration_unit: 0x%X", result->wake_duration_unit);
    printf("\r\n wake_int_exp : 0x%X", result->wake_int_exp);
    printf("\r\n negotiation_type : 0x%X", result->negotiation_type);
    printf("\r\n wake_int_mantissa : 0x%X", result->wake_int_mantissa);
    printf("\r\n implicit_twt : 0x%X", result->implicit_twt);
    printf("\r\n un_announced_twt : 0x%X", result->un_announced_twt);
    printf("\r\n triggered_twt : 0x%X", result->triggered_twt);
    printf("\r\n twt_channel : 0x%X", result->twt_channel);
    printf("\r\n twt_protection : 0x%X", result->twt_protection);
    printf("\r\n twt_flow_id : 0x%X\r\n", result->twt_flow_id);
  } else if (event < SL_WIFI_TWT_EVENTS_END) {
    printf("\r\n twt_flow_id : 0x%X", result->twt_flow_id);
    printf("\r\n negotiation_type : 0x%X\r\n", result->negotiation_type);
  }
  return SL_STATUS_OK;
}

sl_status_t receive_and_send_data(void)
{
  int32_t status = SL_STATUS_OK;
  /* Filling data buffer to be sent.*/
  for (uint16_t i = 0; i < BUF_SIZE; i++) {
    send_buf[i] = i;
  }
  printf("\r\nListening for command\r\n");
  while (1) {
    if (osSemaphoreAcquire(data_semaphore, osWaitForever) == osOK) {
      if (data_received_flag) {
#if !DOOR_LOCK_SIMULATION
        struct sockaddr_in server_address = { 0 };
        server_address.sin_family         = AF_INET;
        server_address.sin_port           = UDP_SERVER_PORT;
        sl_net_inet_addr(SERVER_IP_UDP, &server_address.sin_addr.s_addr);
        packet_count = 0;
        printf("Sending UDP Data\r\n");

        while (packet_count < NUMBER_OF_PACKETS) {
          status = sendto(udp_client_socket,
                          send_buf,
                          BUF_SIZE,
                          0,
                          (struct sockaddr *)&server_address,
                          sizeof(struct sockaddr_in));
          if (status < 0) {
            if (errno == ENOBUFS)
              continue;
            printf("\r\nFailed to send data to UDP Server, Error Code : 0x%lX\r\n", status);
            close(udp_client_socket);
          }
          packet_count++;
        }
#else
        packet_count = 0;
        while (packet_count < NUMBER_OF_PACKETS) {
          status = send(tcp_client_socket, (int8_t *)"Door lock opened", (sizeof("Door lock opened") - 1), 0);
          if (status < 0) {
            if (errno == ENOBUFS)
              continue;
            printf("\r\nFailed to send data to TCP Server, Error Code : 0x%lX\r\n", status);
            close(tcp_client_socket);
          }
          packet_count++;
        }
        printf("\r\nTCP TX completed \r\n");
#endif
        data_received_flag = false;
      }
    }
#ifdef SLI_SI91X_MCU_INTERFACE
#if (SL_SI91X_TICKLESS_MODE == 0)
    printf("M4 in Sleep\r\n");
    sl_si91x_power_manager_sleep();
    printf("M4 wake up\r\n");
#endif
#endif
  }
  return SL_STATUS_OK;
}
