/***************************************************************************/ /**
 * @file
 * @brief SNTP client Example Application
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

#include "sl_net.h"
#include "sl_utility.h"
#include "cmsis_os2.h"
#include "sl_constants.h"
#include "sl_sntp.h"
#include "sl_wifi.h"
#include "sl_net_dns.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_types.h"
#include "string.h"

/******************************************************
 *                    Constants
 ******************************************************/

#define SNTP_METHOD         SL_SNTP_UNICAST_MODE
#define FLAGS               0
#define NTP_SERVER_IP       "0.pool.ntp.org" // Mostly "162.159.200.123"
#define DATA_BUFFER_LENGTH  50
#define SNTP_TIMEOUT        50
#define SNTP_API_TIMEOUT    0
#define ASYNC_WAIT_TIMEOUT  60000
#define DNS_TIMEOUT         20000
#define MAX_DNS_RETRY_COUNT 5
#define DNS_SERVER1_IP      "8.8.8.8"
#define DNS_SERVER2_IP      "8.8.4.4"

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

static const sl_wifi_device_configuration_t sntp_client_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .boot_config = { .oper_mode       = SL_SI91X_CLIENT_MODE,
                   .coex_mode       = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map = (SL_WIFI_FEAT_SECURITY_PSK | SL_WIFI_FEAT_AGGREGATION),
                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT | SL_SI91X_TCP_IP_FEAT_SSL
                      | SL_SI91X_TCP_IP_FEAT_SNTP_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map =
                     (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID | SL_WIFI_CUSTOM_FEAT_ASYNC_CONNECTION_STATUS),
                   .ext_custom_feature_bit_map = (SL_SI91X_EXT_FEAT_SSL_VERSIONS_SUPPORT | SL_SI91X_EXT_FEAT_XTAL_CLK
                                                  | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
#ifdef SLI_SI917
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_WINDOW_SCALING | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(10)
                      | SL_SI91X_EXT_TCP_IP_FEAT_SSL_THREE_SOCKETS | SL_SI91X_EXT_TCP_IP_FEAT_SSL_MEMORY_CLOUD),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = 0 }
};

static uint8_t callback_event = 0;
static sl_status_t cb_status  = SL_STATUS_FAIL;
static uint8_t exec_status    = 0;
static char *event_type[]     = { [SL_SNTP_CLIENT_START]           = "SNTP Client Start",
                                  [SL_SNTP_CLIENT_GET_TIME]        = "SNTP Client Get Time",
                                  [SL_SNTP_CLIENT_GET_TIME_DATE]   = "SNTP Client Get Time and Date",
                                  [SL_SNTP_CLIENT_GET_SERVER_INFO] = "SNTP Client Get Server Info",
                                  [SL_SNTP_CLIENT_STOP]            = "SNTP Client Stop" };

/******************************************************
 *               Function Declarations
 ******************************************************/
sl_status_t embedded_sntp_client(void);
static void application_start(void *argument);

/******************************************************
 *               Function Definitions
 ******************************************************/
static sl_status_t module_status_handler(sl_wifi_event_t event,
                                         sl_status_t status_code,
                                         void *data,
                                         uint32_t data_length,
                                         void *arg)
{
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }

  sl_wifi_module_state_stats_response_t *notif = (sl_wifi_module_state_stats_response_t *)data;

  SL_DEBUG_LOG("---> Module status handler event with length : %lu\r\n", data_length);
  SL_DEBUG_LOG("  <> Timestamp : %lu, state_code : 0x%02X, reason_code : 0x%02X, channel : %u, rssi : %u.\n",
               notif->timestamp,
               notif->state_code,
               notif->reason_code,
               notif->channel,
               notif->rssi);
  SL_DEBUG_LOG("  <> BSSID : %x:%x:%x:%x:%x:%x.\n",
               notif->bssid[0],
               notif->bssid[1],
               notif->bssid[2],
               notif->bssid[3],
               notif->bssid[4],
               notif->bssid[5]);
  return SL_STATUS_OK;
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;

  printf("SNTP client execution Started \r\n");

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sntp_client_configuration, NULL, NULL);
  if (status != SL_STATUS_OK && status != SL_STATUS_ALREADY_INITIALIZED) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  sl_wifi_set_callback_v2(SL_WIFI_STATS_RESPONSE_EVENTS, module_status_handler, NULL);

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("Failed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
    return;
  }

  printf("Wi-Fi client connected\r\n");

  embedded_sntp_client();

  while (1) {
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // Let the CPU go to sleep if the system allows it.
    sl_power_manager_sleep();
#else
    osDelay(osWaitForever);
#endif
  }
}

static void print_char_buffer(char *buffer, uint32_t buffer_length)
{
  uint32_t i = 0;

  for (i = 0; i < buffer_length; i++) {
    printf("%c", buffer[i]);
  }

  printf("\r\n");
  return;
}

static void sntp_client_event_handler(sl_sntp_client_response_t *response,
                                      uint8_t *user_data,
                                      uint16_t user_data_length)
{
  uint16_t length = 0;

  printf("\nReceived %s SNTP event with status %s\n",
         event_type[response->event_type],
         (0 == response->status) ? "Success" : "Failed");

  if (0 == response->status) {
    if (response->data_length > user_data_length) {
      length = user_data_length;
    } else {
      length = response->data_length;
    }

    memcpy(user_data, response->data, length);
  }

  callback_event = response->event_type;
  cb_status      = response->status;

  if (SL_SNTP_CLIENT_STOP == response->event_type) {
    exec_status = 1;
  }
  return;
}

sl_status_t embedded_sntp_client(void)
{
  uint32_t start = 0;
  sl_status_t status;
  sl_ip_address_t address          = { 0 };
  sl_sntp_client_config_t config   = { 0 };
  uint8_t data[DATA_BUFFER_LENGTH] = { 0 };
  sl_sntp_server_info_t serverInfo = { 0 };
  int32_t dns_retry_count          = MAX_DNS_RETRY_COUNT;

  // Convert DNS server IP addresses to sl_ip_address_t structures
  sl_ip_address_t primary_dns_server   = { 0 };
  sl_ip_address_t secondary_dns_server = { 0 };

  primary_dns_server.type = SL_IPV4;
  // Convert the DNS server IP address (DNS_SERVER1_IP) from string to IPv4 address format
  sl_net_inet_addr(DNS_SERVER1_IP, (uint32_t *)&primary_dns_server.ip.v4);

  secondary_dns_server.type = SL_IPV4;
  // Convert the secondary DNS server IP address (DNS_SERVER2_IP) from string to IPv4 address format
  sl_net_inet_addr(DNS_SERVER2_IP, (uint32_t *)&secondary_dns_server.ip.v4);

  // Create sl_net_set_dns_address_t structure
  sl_net_dns_address_t dns_address;
  dns_address.primary_server_address   = &primary_dns_server;
  dns_address.secondary_server_address = &secondary_dns_server;

  // Set DNS server addresses
  status = sl_net_set_dns_server(SL_NET_WIFI_CLIENT_INTERFACE, &dns_address);

  do {
    status = sl_net_dns_resolve_hostname(NTP_SERVER_IP, DNS_TIMEOUT, SL_NET_DNS_TYPE_IPV4, &address);
    dns_retry_count--;
  } while ((dns_retry_count != 0) && (status != SL_STATUS_OK));

  printf("Ip Address : %u.%u.%u.%u\n",
         address.ip.v4.bytes[0],
         address.ip.v4.bytes[1],
         address.ip.v4.bytes[2],
         address.ip.v4.bytes[3]);

  config.server_host_name = address.ip.v4.bytes;
  config.sntp_method      = SNTP_METHOD;
  config.sntp_timeout     = SNTP_TIMEOUT;
  config.event_handler    = sntp_client_event_handler;
  config.flags            = FLAGS;

  cb_status = SL_STATUS_FAIL;
  status    = sl_sntp_client_start(&config, SNTP_API_TIMEOUT);
  if ((SNTP_API_TIMEOUT == 0) && (SL_STATUS_IN_PROGRESS == status)) {
    start = osKernelGetTickCount();

    while ((SL_SNTP_CLIENT_START != callback_event) && (osKernelGetTickCount() - start) <= ASYNC_WAIT_TIMEOUT) {
      osThreadYield();
    }

    if (cb_status != SL_STATUS_OK) {
      printf("Failed to start Async SNTP client: 0x%lx\r\n", status);
      return cb_status;
    }
  } else {
    if (status == SL_STATUS_OK) {
      printf("SNTP Client started successfully\n");
    } else {
      printf("Failed to start SNTP client: 0x%lx\r\n", status);
      return status;
    }
  }

  cb_status = SL_STATUS_FAIL;
  status    = sl_sntp_client_get_time(data, DATA_BUFFER_LENGTH, SNTP_API_TIMEOUT);
  if ((SNTP_API_TIMEOUT == 0) && (SL_STATUS_IN_PROGRESS == status)) {
    start = osKernelGetTickCount();

    while ((SL_SNTP_CLIENT_GET_TIME != callback_event) && (osKernelGetTickCount() - start) <= ASYNC_WAIT_TIMEOUT) {
      osThreadYield();
    }

    if (cb_status != SL_STATUS_OK) {
      printf("Failed to get async time from ntp server : 0x%lx\r\n", status);
      return cb_status;
    }
  } else {
    if (status == SL_STATUS_OK) {
      printf("SNTP Client got TIME successfully\n");
    } else {
      printf("Failed to get time from ntp server : 0x%lx\r\n", status);
      return status;
    }
  }
  print_char_buffer((char *)data, strlen((const char *)data));

  cb_status = SL_STATUS_FAIL;
  status    = sl_sntp_client_get_time_date(data, DATA_BUFFER_LENGTH, SNTP_API_TIMEOUT);
  if ((SNTP_API_TIMEOUT == 0) && (SL_STATUS_IN_PROGRESS == status)) {
    start = osKernelGetTickCount();

    while ((SL_SNTP_CLIENT_GET_TIME_DATE != callback_event) && (osKernelGetTickCount() - start) <= ASYNC_WAIT_TIMEOUT) {
      osThreadYield();
    }

    if (cb_status != SL_STATUS_OK) {
      printf("Failed to get Async date and time from ntp server : 0x%lx\r\n", status);
      return cb_status;
    }
  } else {
    if (status == SL_STATUS_OK) {
      printf("SNTP Client got TIME and DATE successfully\n");
    } else {
      printf("Failed to get date and time from ntp server : 0x%lx\r\n", status);
      return status;
    }
  }
  print_char_buffer((char *)data, strlen((const char *)data));

  cb_status = SL_STATUS_FAIL;
  status    = sl_sntp_client_get_server_info(&serverInfo, SNTP_API_TIMEOUT);
  if ((SNTP_API_TIMEOUT == 0) && (SL_STATUS_IN_PROGRESS == status)) {
    start = osKernelGetTickCount();

    while ((SL_SNTP_CLIENT_GET_SERVER_INFO != callback_event)
           && (osKernelGetTickCount() - start) <= ASYNC_WAIT_TIMEOUT) {
      osThreadYield();
    }

    if (cb_status != SL_STATUS_OK) {
      printf("Failed to get async ntp server info : 0x%lx\r\n", status);
      return cb_status;
    }
  } else {
    if (status == SL_STATUS_OK) {
      printf("SNTP Client got server info successfully\n");
    } else {
      printf("Failed to get ntp server info : 0x%lx\r\n", status);
      return status;
    }
  }
  printf("Got Server IP version as : %u\n", serverInfo.ip_version);
  if (4 == serverInfo.ip_version) {
    printf("IPv4 Address is : %u.%u.%u.%u\n",
           serverInfo.server_ip_address.ipv4_address[0],
           serverInfo.server_ip_address.ipv4_address[1],
           serverInfo.server_ip_address.ipv4_address[2],
           serverInfo.server_ip_address.ipv4_address[3]);
  } else {
    printf("IPv6 Address is : %lx:%lx:%lx:%lx\n",
           serverInfo.server_ip_address.ipv6_address[0],
           serverInfo.server_ip_address.ipv6_address[1],
           serverInfo.server_ip_address.ipv6_address[2],
           serverInfo.server_ip_address.ipv6_address[3]);
  }
  printf("SNTP Server Method : %u\n", serverInfo.sntp_method);

  cb_status = SL_STATUS_FAIL;
  status    = sl_sntp_client_stop(SNTP_API_TIMEOUT);
  if ((SNTP_API_TIMEOUT == 0) && (SL_STATUS_IN_PROGRESS == status)) {
    start = osKernelGetTickCount();

    while ((SL_SNTP_CLIENT_STOP != callback_event) && (osKernelGetTickCount() - start) <= ASYNC_WAIT_TIMEOUT) {
      osThreadYield();
    }

    if (cb_status != SL_STATUS_OK) {
      printf("Failed to stop Async SNTP client: 0x%lx\r\n", status);
      return cb_status;
    }
  } else {
    if (status == SL_STATUS_OK) {
      printf("SNTP Client stopped successfully\n");
    } else {
      printf("Failed to stop SNTP client: 0x%lx\r\n", status);
      return status;
    }

    exec_status = 1;
  }

  printf("Done\r\n");

  while (0 == exec_status) {
    osThreadYield();
  }

  printf("SNTP client execution completed \r\n");

  return SL_STATUS_OK;
}
