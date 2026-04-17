/***************************************************************************/ /**
 * @file
 * @brief Concurrent Mode Dual IP Example Application
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "cmsis_os2.h"
#include "errno.h"
#include "sl_wifi_device.h"
#include "sl_utility.h"
#include "sl_wifi.h"
#include "sl_net_wifi_types.h"
#include "socket.h"
#include "sl_net_wifi_types.h"
#include "sl_net.h"
#include "sl_si91x_socket_support.h"
#include "sl_si91x_socket_constants.h"
#include <string.h>
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_socket.h"
#include "sl_si91x_socket_utility.h"
#include "sl_net_si91x.h"
#include "sl_si91x_core_utilities.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define WIFI_CLIENT_PROFILE_SSID "YOUR_AP_SSID"
#define WIFI_CLIENT_CREDENTIAL   "YOUR_AP_PASSPHRASE"

#define WIFI_AP_PROFILE_SSID "MY_DUAL_AP_SSID"
#define WIFI_AP_CREDENTIAL   "MY_AP_PASSPHRASE"

#define WIFI_CLIENT_SECURITY_TYPE   SL_WIFI_WPA2
#define WIFI_CLIENT_ENCRYPTION_TYPE SL_WIFI_DEFAULT_ENCRYPTION

#define CHANNEL_NO 11

// Memory length for send buffer
#define TCP_BUFFER_SIZE 1380
#define UDP_BUFFER_SIZE 1390

//IPv6 address of STA connected to ThirdParty AP
#define SERVER_IP "2401:4900:1245:10ed::105"
// Server port number
#define SERVER_PORT 5000
// Module port number
#define LISTENING_PORT 5005
#define BACK_LOG       1

#define AP_VAP 1

#define BYTES_TO_SEND (1 << 29) // 512MB
#define TEST_TIMEOUT  (10000)   // 10sec

#define SL_HIGH_PERFORMANCE_SOCKET BIT(7)

/******************************************************
 *               Variable Definitions
 ******************************************************/

uint8_t address_buffer[SL_IPV6_ADDRESS_LENGTH];
uint8_t ap_vap = AP_VAP;
uint8_t data_buffer[TCP_BUFFER_SIZE];
uint8_t address_buffer[SL_IPV6_ADDRESS_LENGTH];
uint8_t has_data_received = 0;
uint32_t bytes_read       = 0;
uint32_t start            = 0;
uint32_t now              = 0;
uint8_t first_data_frame  = 1;

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

/*****************************************************
 *                      Socket configuration
*****************************************************/
#define TOTAL_SOCKETS                   1  //@ Total number of sockets. TCP TX + TCP RX + UDP TX + UDP RX
#define TOTAL_TCP_SOCKETS               1  //@ Total TCP sockets. TCP TX + TCP RX
#define TOTAL_UDP_SOCKETS               0  //@ Total UDP sockets. UDP TX + UDP RX
#define TCP_TX_ONLY_SOCKETS             0  //@ Total TCP TX only sockets. TCP TX
#define TCP_RX_ONLY_SOCKETS             1  //@ Total TCP RX only sockets. TCP RX
#define UDP_TX_ONLY_SOCKETS             0  //@ Total UDP TX only sockets. UDP TX
#define UDP_RX_ONLY_SOCKETS             0  //@ Total UDP RX only sockets. UDP RX
#define TCP_RX_HIGH_PERFORMANCE_SOCKETS 1  //@ Total TCP RX High Performance sockets
#define TCP_RX_WINDOW_SIZE_CAP          44 //@ TCP RX Window size
#define TCP_RX_WINDOW_DIV_FACTOR        44 //@ TCP RX Window division factor

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

//! IP address of the module
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#define DEFAULT_WIFI_AP_MODULE_IP4_ADDRESS "192.168.10.10"

//! IP address of netmask
//! E.g: 0x00FFFFFF == 255.255.255.0
#define DEFAULT_WIFI_AP_NS_MODULE_IP4_ADDRESS "255.255.255.0"

//! IP address of Gateway
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#define DEFAULT_WIFI_AP_GATEWAY4_ADDRESS "192.168.10.10"

//! IPv6 address of the module
#define DEFAULT_WIFI_AP_MODULE_IP6_ADDRESS "2001:db8:0:1::121"

//! IP address of Gateway
#define DEFAULT_WIFI_AP_GATEWAY6_ADDRESS "2001:db8:0:1::121"

static const sl_net_wifi_client_profile_t wifi_client_profile = {
    .config = {
        .ssid.value = WIFI_CLIENT_PROFILE_SSID,
        .ssid.length = sizeof(WIFI_CLIENT_PROFILE_SSID)-1,
        .channel.channel = SL_WIFI_AUTO_CHANNEL,
        .channel.band = SL_WIFI_AUTO_BAND,
        .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH,
        .bssid = {{0}},
        .bss_type = SL_WIFI_BSS_TYPE_INFRASTRUCTURE,
        .security = WIFI_CLIENT_SECURITY_TYPE,
        .encryption = WIFI_CLIENT_ENCRYPTION_TYPE,
        .client_options = 0,
        .credential_id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
    },
    .ip = {
        .mode = SL_IP_MANAGEMENT_DHCP,
        .type = (SL_IPV4|SL_IPV6),
        .host_name = NULL,
        .ip = {{{0}}},

    }
};

static sl_net_wifi_ap_profile_t wifi_ap_profile = {
    .config = {
        .ssid.value = WIFI_AP_PROFILE_SSID,
        .ssid.length = sizeof(WIFI_AP_PROFILE_SSID)-1,
        .channel.channel = SL_WIFI_AUTO_CHANNEL,
        .channel.band = SL_WIFI_AUTO_BAND,
        .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH,
        .security = SL_WIFI_WPA2,
        .encryption = SL_WIFI_CCMP_ENCRYPTION,
        .rate_protocol = SL_WIFI_RATE_PROTOCOL_AUTO,
        .options = 0,
        .credential_id = SL_NET_USER_CREDENTIAL_ID,
        .keepalive_type = SL_SI91X_AP_NULL_BASED_KEEP_ALIVE,
        .beacon_interval = 100,
        .client_idle_timeout = 0xFF,
        .dtim_beacon_count = 3,
        .maximum_clients = 3,
        .is_11n_enabled = 0,
        .beacon_stop = 0,
    },
    .ip = {
        .mode = SL_IP_MANAGEMENT_STATIC_IP,
        .type = (SL_IPV4|SL_IPV6),
        .host_name = NULL,
        .ip = {{{0}}},
    }
};

sl_net_wifi_psk_credential_entry_t wifi_client_credential = { .type        = SL_NET_WIFI_PSK,
                                                              .data_length = sizeof(WIFI_CLIENT_CREDENTIAL) - 1,
                                                              .data        = WIFI_CLIENT_CREDENTIAL };

sl_net_wifi_psk_credential_entry_t wifi_ap_credential = { .type        = SL_NET_WIFI_PSK,
                                                          .data_length = sizeof(WIFI_AP_CREDENTIAL) - 1,
                                                          .data        = WIFI_AP_CREDENTIAL };
/******************************************************
 *               Function Declarations
 ******************************************************/
void receive_data_from_tcp_client(void);
void send_data_to_udp_server(void);
static void application_start(void *argument);
static sl_status_t ap_connected_event_handler(sl_wifi_event_t event,
                                              sl_status_t status_code,
                                              void *data,
                                              uint32_t data_length,
                                              void *arg);
static sl_status_t ap_disconnected_event_handler(sl_wifi_event_t event,
                                                 sl_status_t status_code,
                                                 void *data,
                                                 uint32_t data_length,
                                                 void *arg);

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
  sl_wifi_channel_t client_channel               = { 0 };
  sl_net_wifi_client_profile_t profile           = { 0 };
  sl_ip_address_t ip_address                     = { 0 };
  unsigned char hex_addr[SL_IPV6_ADDRESS_LENGTH] = { 0 };
  uint32_t ip_addr[4]                            = { 0 };
  uint32_t gateway[4]                            = { 0 };

  int return_value;
  enum state_check { STA_ALONE, STA_FIRST, STA_NEXT, AP_ALONE, AP_FIRST, AP_NEXT, DATA_TRANSFER, QUIT } state;

  state = STA_FIRST;
  while (1) {
    switch (state) {

      case STA_ALONE:
      case STA_FIRST:
      case STA_NEXT:

        status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_default_concurrent_v6_configuration, NULL, NULL);
        if (status != SL_STATUS_OK) {
          printf("\r\nFailed to start Wi-Fi client interface: 0x%lx\r\n", status);
          return;
        }
        printf("\r\nWi-Fi Client interface init\r\n");

        sl_wifi_set_callback_v2(SL_WIFI_CLIENT_CONNECTED_EVENTS, ap_connected_event_handler, NULL);
        sl_wifi_set_callback_v2(SL_WIFI_CLIENT_DISCONNECTED_EVENTS, ap_disconnected_event_handler, NULL);

        //  Client profile set
        status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_0, &wifi_client_profile);
        if (status != SL_STATUS_OK) {
          printf("\r\nFailed to set client profile: 0x%lx\r\n", status);
          return;
        }
        printf("\r\nWi-Fi set client profile success\r\n");

        status = sl_net_set_credential(SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
                                       wifi_client_credential.type,
                                       &wifi_client_credential.data,
                                       wifi_client_credential.data_length);
        if (status != SL_STATUS_OK) {
          printf("Failed to set credentials: 0x%lx\r\n", status);
          return;
        }
        printf("\r\nWi-Fi set credential success\r\n");

        //  STA Scan, Connect and IPv4 + IPv6 config
        status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_0);
        if (status != SL_STATUS_OK) {
          printf("Failed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
          return;
        }
        printf("\r\nWi-Fi client interface successful\r\n");

        status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_0, &profile);
        if (status != SL_STATUS_OK) {
          printf("\r\nFailed to get client profile: 0x%lx", status);
          return;
        }

        if (profile.ip.type & SL_IPV4) {
          ip_address.type = SL_IPV4;
          memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
          printf("\r\nClient IPv4 : ");
          print_sl_ip_address(&ip_address);
          printf("\r\n");
        }

        if (profile.ip.type & SL_IPV6) {
          printf("\r\nClient IPv6 : ");
          sl_ip_address_t link_local_address = { 0 };
          link_local_address.ip.v6           = profile.ip.ip.v6.link_local_address;
          link_local_address.type            = SL_IPV6;
          printf("\nLink Local Address: ");
          print_sl_ip_address(&link_local_address);

          sl_ip_address_t global_address = { 0 };
          global_address.ip.v6           = profile.ip.ip.v6.global_address;
          global_address.type            = SL_IPV6;
          printf("Global Address: ");
          print_sl_ip_address(&global_address);

          sl_ip_address_t gateway = { 0 };
          gateway.ip.v6           = profile.ip.ip.v6.gateway;
          gateway.type            = SL_IPV6;
          printf("Gateway Address: ");
          print_sl_ip_address(&gateway);
        }

        if (state == STA_ALONE) {
          goto EXIT;
        } else if (state == STA_NEXT) {
          state = DATA_TRANSFER;
        } else if (state == STA_FIRST) {
          state = AP_NEXT;
        }
        break;
      case AP_ALONE:
      case AP_FIRST:
      case AP_NEXT:

        status = sl_net_init(SL_NET_WIFI_AP_INTERFACE, &sl_wifi_default_concurrent_v6_configuration, NULL, NULL);
        if (status != SL_STATUS_OK) {
          printf("\r\nFailed to start Wi-Fi AP interface: 0x%lx\r\n", status);
          return;
        }
        printf("\r\nWi-Fi AP interface init\r\n");

        sl_net_inet_addr(DEFAULT_WIFI_AP_MODULE_IP4_ADDRESS, (uint32_t *)&wifi_ap_profile.ip.ip.v4.ip_address.bytes);
        sl_net_inet_addr(DEFAULT_WIFI_AP_NS_MODULE_IP4_ADDRESS, (uint32_t *)&wifi_ap_profile.ip.ip.v4.netmask.bytes);
        sl_net_inet_addr(DEFAULT_WIFI_AP_GATEWAY4_ADDRESS, (uint32_t *)&wifi_ap_profile.ip.ip.v4.gateway.bytes);

        return_value = sl_inet_pton6(DEFAULT_WIFI_AP_MODULE_IP6_ADDRESS,
                                     DEFAULT_WIFI_AP_MODULE_IP6_ADDRESS + strlen(DEFAULT_WIFI_AP_MODULE_IP6_ADDRESS),
                                     hex_addr,
                                     (unsigned int *)ip_addr);
        if (return_value != 0x1) {
          printf("\r\nIPv6 conversion failed.\r\n");
          return;
        }
        return_value = sl_inet_pton6(DEFAULT_WIFI_AP_GATEWAY6_ADDRESS,
                                     DEFAULT_WIFI_AP_GATEWAY6_ADDRESS + strlen(DEFAULT_WIFI_AP_GATEWAY6_ADDRESS),
                                     hex_addr,
                                     (unsigned int *)gateway);
        if (return_value != 0x1) {
          printf("\r\nIPv6 conversion failed.\r\n");
          return;
        }

        memcpy(wifi_ap_profile.ip.ip.v6.global_address.value, ip_addr, sizeof(sl_ipv6_address_t));
        memcpy(wifi_ap_profile.ip.ip.v6.gateway.value, gateway, sizeof(sl_ipv6_address_t));

        if (state == AP_NEXT) {
          status = sl_wifi_get_channel(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &client_channel);
          if (status != SL_STATUS_OK) {
            printf("\r\nFailed to get client channel: 0x%lx\r\n", status);
            return;
          }
          printf("\r\nSuccess to get client channel\r\n");

          wifi_ap_profile.config.channel.channel = client_channel.channel;
        } else if (state == AP_FIRST) {
          wifi_ap_profile.config.channel.channel = CHANNEL_NO;
        }

        //  AP profile set
        status = sl_net_set_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_0, &wifi_ap_profile);
        if (status != SL_STATUS_OK) {
          printf("\r\nFailed to set AP profile: 0x%lx\r\n", status);
          return;
        }
        printf("\r\nWi-Fi set AP profile success\r\n");

        status = sl_net_set_credential(SL_NET_USER_CREDENTIAL_ID,
                                       wifi_ap_credential.type,
                                       &wifi_ap_credential.data,
                                       wifi_ap_credential.data_length);
        if (status != SL_STATUS_OK) {
          printf("Failed to set credentials: 0x%lx\r\n", status);
          return;
        }
        printf("\r\nWi-Fi set credential success\r\n");

        //  AP IPv4 + IPv6 config and AP Start
        status = sl_net_up(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_0);
        if (status != SL_STATUS_OK) {
          printf("Failed to bring Wi-Fi AP interface up: 0x%lx\r\n", status);
          return;
        }

        status = sl_net_get_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_0, &wifi_ap_profile);
        if (status != SL_STATUS_OK) {
          printf("Failed to get AP profile: 0x%lx\r\n", status);
          return;
        }

        if (wifi_ap_profile.ip.type & SL_IPV4) {
          ip_address.type = SL_IPV4;
          printf("\r\nAP IPv4 : ");
          memcpy(&ip_address.ip.v4.bytes, &wifi_ap_profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
          print_sl_ip_address(&ip_address);
          printf("\r\n");
        }

        if (wifi_ap_profile.ip.type & SL_IPV6) {
          printf("\r\nAP IPv6 : ");
          sl_ip_address_t link_local_address = { 0 };
          link_local_address.ip.v6           = wifi_ap_profile.ip.ip.v6.link_local_address;
          link_local_address.type            = SL_IPV6;
          printf("\nLink Local Address: ");
          print_sl_ip_address(&link_local_address);
          sl_ip_address_t global_address = { 0 };
          global_address.ip.v6           = wifi_ap_profile.ip.ip.v6.global_address;
          global_address.type            = SL_IPV6;
          printf("Global Address: ");
          print_sl_ip_address(&global_address);

          sl_ip_address_t gateway = { 0 };
          gateway.ip.v6           = wifi_ap_profile.ip.ip.v6.gateway;
          gateway.type            = SL_IPV6;
          printf("Gateway Address: ");
          print_sl_ip_address(&gateway);
        }

        printf("\r\nAP started\r\n");

        if (state == AP_ALONE) {
          state = DATA_TRANSFER;
        } else if (state == AP_NEXT) {
          state = DATA_TRANSFER;
        } else if (state == AP_FIRST) {
          state = STA_NEXT;
        }
        break;
      case DATA_TRANSFER:

        for (size_t i = 0; i < sizeof(data_buffer); i++)
          data_buffer[i] = 'A' + (i % 26);

        printf("\r\nSending UDP Data over IPv6 on STA VAP\r\n");
        send_data_to_udp_server();

        printf("\r\nReceving TCP Data over IPv4 on AP VAP\r\n");
        receive_data_from_tcp_client();
        state = QUIT;
        break;

      case QUIT:
        printf("\nExitting");
        goto EXIT;
    }
  }
EXIT:
  while (1) {
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // Let the CPU go to sleep if the system allows it.
    sl_power_manager_sleep();
#else
    osDelay(osWaitForever);
#endif
  }
}

static sl_status_t ap_connected_event_handler(sl_wifi_event_t event,
                                              sl_status_t status_code,
                                              void *data,
                                              uint32_t data_length,
                                              void *arg)
{
  UNUSED_PARAMETER(data_length);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }

  printf("Remote Client connected: ");
  print_mac_address((sl_mac_address_t *)data);
  printf("\n");

  return SL_STATUS_OK;
}

static sl_status_t ap_disconnected_event_handler(sl_wifi_event_t event,
                                                 sl_status_t status_code,
                                                 void *data,
                                                 uint32_t data_length,
                                                 void *arg)
{
  UNUSED_PARAMETER(data_length);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }

  printf("Remote Client disconnected: ");
  print_mac_address((sl_mac_address_t *)data);
  printf("\n");

  return SL_STATUS_OK;
}

void receive_data_from_tcp_client(void)
{
  int server_socket                 = -1;
  int client_socket                 = -1;
  int socket_return_value           = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  uint8_t high_performance_socket   = SL_HIGH_PERFORMANCE_SOCKET;

  sl_status_t status = sl_si91x_config_socket(socket_config);
  if (status != SL_STATUS_OK) {
    printf("Socket config failed: %ld\r\n", status);
  }
  printf("\r\nSocket config Done\r\n");

  int read_bytes                = 1;
  uint32_t total_bytes_received = 0;
  server_socket                 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_socket < 0) {
    printf("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  printf("\r\nServer Socket ID : %d\r\n", server_socket);

  socket_return_value = setsockopt(server_socket,
                                   SOL_SOCKET,
                                   SL_SO_HIGH_PERFORMANCE_SOCKET,
                                   &high_performance_socket,
                                   sizeof(high_performance_socket));
  if (socket_return_value < 0) {
    printf("\r\nSet Socket option failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }

  socket_return_value =
    sl_si91x_setsockopt(server_socket, SOL_SOCKET, SL_SI91X_SO_SOCK_VAP_ID, &ap_vap, sizeof(ap_vap));

  server_address.sin_family = AF_INET;
  server_address.sin_port   = LISTENING_PORT;

  socket_return_value = bind(server_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    printf("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }

  socket_return_value = listen(server_socket, BACK_LOG);
  if (socket_return_value < 0) {
    printf("\r\nSocket listen failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }
  printf("\r\nListening on Local Port : %d\r\n", LISTENING_PORT);

  client_socket = accept(server_socket, NULL, NULL);
  if (client_socket < 0) {
    printf("\r\nSocket accept failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }
  printf("\r\nClient Socket ID : %d\r\n", client_socket);

  printf("\r\nTCP_RX Throughput test start\r\n");
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
          printf("\r\nrecv failed with BSD error = %d and status = 0x%lx\r\n", errno, status);
        }
      } else {
        printf("\r\nrecv failed with BSD error = %d\r\n", errno);
      }
      break;
    }
    total_bytes_received = total_bytes_received + read_bytes;
    now                  = osKernelGetTickCount();

    if ((now - start) > TEST_TIMEOUT) {
      printf("\r\nTest Time Out: %ld ms\r\n", (now - start));
      break;
    }
  }
  printf("\r\nTCP_RX Throughput test finished\r\n");
  printf("\r\nTotal bytes received : %ld\r\n", total_bytes_received);

  close(client_socket);
  close(server_socket);
}

void send_data_to_udp_server(void)
{
  int client_socket                   = -1;
  uint32_t total_bytes_sent           = 0;
  struct sockaddr_in6 server_address6 = { 0 };
  socklen_t socket_length             = sizeof(struct sockaddr_in6);
  int sent_bytes                      = 1;
  uint32_t start                      = 0;
  uint32_t now                        = 0;

  client_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  if (client_socket < 0) {
    printf("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  printf("\r\nSocket ID : %d\r\n", client_socket);

  server_address6.sin6_family = AF_INET6;
  server_address6.sin6_port   = SERVER_PORT;

  int status = sl_inet_pton6(SERVER_IP,
                             SERVER_IP + strlen(SERVER_IP),
                             address_buffer,
                             (unsigned int *)server_address6.sin6_addr.__u6_addr.__u6_addr32);
  if (status != 0x1) {
    printf("\r\nIPv6 conversion failed.\r\n");
    return;
  }

  printf("\r\nUDP_TX Throughput test start\r\n");
  start = osKernelGetTickCount();
  while (total_bytes_sent < BYTES_TO_SEND) {
    sent_bytes =
      sendto(client_socket, data_buffer, UDP_BUFFER_SIZE, 0, (struct sockaddr *)&server_address6, socket_length);
    now = osKernelGetTickCount();
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
  printf("\r\nUDP_TX Throughput test finished\r\n");
  printf("\r\nTotal bytes sent : %ld\r\n", total_bytes_sent);

  close(client_socket);
}
