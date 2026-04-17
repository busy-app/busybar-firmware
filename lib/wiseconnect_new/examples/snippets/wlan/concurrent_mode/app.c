/***************************************************************************/ /**
 * @file
 * @brief Concurrent Mode Example Application
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
#include "cmsis_os2.h"
#include "sl_wifi_device.h"
#include "sl_utility.h"
#include "sl_wifi.h"
#include "sl_net_wifi_types.h"
#include "sl_wifi_callback_framework.h"
#include "socket.h"
#include "sl_si91x_socket_support.h"
#include "sl_si91x_socket_constants.h"
#include "sl_si91x_socket.h"
#include "sl_si91x_socket_utility.h"

/******************************************************
 *                      Macros
 ******************************************************/
// Type of throughput
#define UDP_TX 0
#define UDP_RX 1
#define TCP_TX 2
#define TCP_RX 3

#define THROUGHPUT_TYPE UDP_TX

#define WIFI_CLIENT_PROFILE_SSID "YOUR_AP_SSID"
#define WIFI_CLIENT_CREDENTIAL   "YOUR_AP_PASSPHRASE"

#define WIFI_AP_PROFILE_SSID "MY_AP_SSID"
#define WIFI_AP_CREDENTIAL   "MY_AP_PASSPHRASE"

#define WIFI_CLIENT_SECURITY_TYPE   SL_WIFI_WPA_WPA2_MIXED
#define WIFI_CLIENT_ENCRYPTION_TYPE SL_WIFI_DEFAULT_ENCRYPTION

//! IP address of the module
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#define DEFAULT_WIFI_MODULE_IP_ADDRESS 0x0A0AA8C0

//! IP address of netmask
//! E.g: 0x00FFFFFF == 255.255.255.0
#define DEFAULT_WIFI_SN_MASK_ADDRESS 0x00FFFFFF

//! IP address of Gateway
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#define DEFAULT_WIFI_GATEWAY_ADDRESS 0x0A0AA8C0

#define SOCKET_ASYNC_FEATURE 1

// Memory length for send buffer
#define TCP_BUFFER_SIZE 1460
#define UDP_BUFFER_SIZE 1470
#if ((THROUGHPUT_TYPE == UDP_RX) || (THROUGHPUT_TYPE == UDP_TX))
#define BUFFER_SIZE UDP_BUFFER_SIZE
#elif ((THROUGHPUT_TYPE == TCP_RX) || (THROUGHPUT_TYPE == TCP_TX))
#define BUFFER_SIZE TCP_BUFFER_SIZE
#endif
#define SERVER_IP "192.168.0.175"

// Server port number
#define SERVER_PORT 5000

// Module port number
#define LISTENING_PORT 5005
#define BACK_LOG       1

#define AP_VAP 1

#define BYTES_TO_SEND    (1 << 29) // 512MB
#define BYTES_TO_RECEIVE (1 << 28) //256MB
#define TEST_TIMEOUT     (30000)   // 10sec

#define SL_HIGH_PERFORMANCE_SOCKET BIT(7)
#define LISTEN_ON_AP_INTERFACE     0

// Enable vendor-specific Information Elements (IEs) feature.
#define SL_WIFI_ENABLE_VENDOR_IE 0

/******************************************************
 *               Variable Definitions
 ******************************************************/
uint8_t ap_vap = AP_VAP;
uint8_t data_buffer[BUFFER_SIZE];
sl_ip_address_t ip_address           = { 0 };
sl_net_wifi_client_profile_t profile = { 0 };
uint8_t has_data_received            = 0;
uint32_t bytes_read                  = 0;
uint32_t start                       = 0;
uint32_t now                         = 0;
uint8_t first_data_frame             = 1;

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
        .type = SL_IPV4,
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
        .credential_id = SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID,
        .keepalive_type = SL_SI91X_AP_NULL_BASED_KEEP_ALIVE,
        .beacon_interval = 100,
        .client_idle_timeout = 0xFF,
        .dtim_beacon_count = 3,
        .maximum_clients = 3,
        .beacon_stop = 0,
        .is_11n_enabled = 0,
    },
    .ip = {
        .mode = SL_IP_MANAGEMENT_STATIC_IP,
        .type = SL_IPV4,
        .host_name = NULL,
        .ip = {
        		.v4.ip_address.value = DEFAULT_WIFI_MODULE_IP_ADDRESS,
				.v4.gateway.value = DEFAULT_WIFI_GATEWAY_ADDRESS,
				.v4.netmask.value = DEFAULT_WIFI_SN_MASK_ADDRESS
        },
    }
};

sl_net_wifi_psk_credential_entry_t wifi_client_credential = { .type        = SL_NET_WIFI_PSK,
                                                              .data_length = sizeof(WIFI_CLIENT_CREDENTIAL) - 1,
                                                              .data        = WIFI_CLIENT_CREDENTIAL };

sl_net_wifi_psk_credential_entry_t wifi_ap_credential = { .type        = SL_NET_WIFI_PSK,
                                                          .data_length = sizeof(WIFI_AP_CREDENTIAL) - 1,
                                                          .data        = WIFI_AP_CREDENTIAL };
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
/******************************************************
 *               Function Declarations
 ******************************************************/
void receive_data_from_tcp_client(void);
void send_data_to_tcp_server(void);
void receive_data_from_udp_client(void);
void send_data_to_udp_server(void);
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
static void application_start(void *argument);
static void measure_and_print_throughput(uint32_t total_num_of_bytes, uint32_t test_timeout);

/******************************************************
 *               Function Definitions
 ******************************************************/
void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static void measure_and_print_throughput(uint32_t total_num_of_bytes, uint32_t test_timeout)
{
  float duration = ((test_timeout) / 1000);                    // ms to sec
  float result   = ((float)total_num_of_bytes * 8) / duration; // bytes to bps
  result         = (result / 1000000);                         // bps to Mbps
  printf("\r\nThroughput achieved @ %0.02f Mbps in %0.03f sec successfully\r\n", result, duration);
}

void data_callback(uint32_t sock_no,
                   uint8_t *buffer,
                   uint32_t length,
                   const sl_si91x_socket_metadata_t *firmware_socket_response)
{
  UNUSED_PARAMETER(buffer);
  UNUSED_PARAMETER(firmware_socket_response);

  if (first_data_frame) {
    start = osKernelGetTickCount();
    printf("\r\nClient Socket ID : %ld\r\n", sock_no);
    switch (THROUGHPUT_TYPE) {
      case UDP_RX:
        printf("\r\nUDP_RX Throughput test start\r\n");
        break;
      case TCP_RX:
        printf("\r\nTCP_RX Throughput test start\r\n");
        break;
    }
    first_data_frame = 0;
  }

  bytes_read += length;
  now = osKernelGetTickCount();
  if ((bytes_read > BYTES_TO_RECEIVE) || ((now - start) > TEST_TIMEOUT)) {
    has_data_received = 1;
  }
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;
  sl_wifi_channel_t client_channel = { 0 };

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_default_concurrent_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi Client interface init\r\n");

  status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_client_profile);
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
  printf("\nWi-Fi set credential success\n");

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1);
  if (status != SL_STATUS_OK) {
    printf("Failed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi Client interface up\r\n");

  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &profile);
  if (status != SL_STATUS_OK) {
    printf("Failed to get client profile: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nSuccess to get client profile\r\n");

  ip_address.type = SL_IPV4;
  memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
  print_sl_ip_address(&ip_address);

  status = sl_net_init(SL_NET_WIFI_AP_INTERFACE, &sl_wifi_default_concurrent_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to start Wi-Fi AP interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi AP interface init\r\n");

  sl_wifi_set_callback_v2(SL_WIFI_CLIENT_CONNECTED_EVENTS, ap_connected_event_handler, NULL);
  sl_wifi_set_callback_v2(SL_WIFI_CLIENT_DISCONNECTED_EVENTS, ap_disconnected_event_handler, NULL);

  status = sl_wifi_get_channel(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &client_channel);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to get client channel: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nSuccess to get client channel\r\n");

  wifi_ap_profile.config.channel.channel = client_channel.channel;
  status = sl_net_set_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_ap_profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to set AP profile: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nSuccess to set AP profile \r\n");

  ip_address.type = SL_IPV4;
  memcpy(&ip_address.ip.v4.bytes, &wifi_ap_profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));

  printf("\r\n IP Address of AP:");
  print_sl_ip_address(&ip_address);

  status = sl_net_set_credential(SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID,
                                 wifi_ap_credential.type,
                                 &wifi_ap_credential.data,
                                 wifi_ap_credential.data_length);
  if (status != SL_STATUS_OK) {
    printf("Failed to set credentials: 0x%lx\r\n", status);
    return;
  }
  printf("\nWi-Fi set credential success\n");

  status = sl_net_up(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring Wi-Fi AP interface up: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nAP started\r\n");

#if SL_WIFI_ENABLE_VENDOR_IE
  // -------------------------------
  // Adding 2 new vendor-specific IE
  // -------------------------------
  uint8_t fw_unique_id1       = 0;
  uint8_t vendor_ie_buffer1[] = {
    0xDD,             // Element ID for vendor-specific IE, always same
    0x05,             // Length of the following bytes (OUI + Data)
    0xAA, 0xBB, 0xCC, // OUI (Organizationally Unique Identifier)
    0x01, 0xEE        // vendor-specific data
  };
  sl_wifi_vendor_ie_t vendor_ie1 = { 0 };
  vendor_ie1.unique_id = SL_WIFI_VENDOR_IE_AUTO_ASSIGN; // While adding new vendor-specific IE use the auto-assign ID.
  vendor_ie1.mgmt_frame_bitmap = SL_WIFI_VENDOR_IE_IN_BEACON | SL_WIFI_VENDOR_IE_IN_PROBE_RESP
                                 | SL_WIFI_VENDOR_IE_IN_ASSOC_RESP;
  vendor_ie1.ie_buffer_length = sizeof(vendor_ie_buffer1);
  vendor_ie1.ie_buffer        = vendor_ie_buffer1;

  status = sl_wifi_add_vendor_ie(&vendor_ie1, &fw_unique_id1);
  if (status == SL_STATUS_OK) {
    printf("vendor-specific IE added with unique ID: %d\n", fw_unique_id1);
  } else {
    printf("Failed to add vendor-specific IE: 0x%lx\n", status);
  }

  uint8_t fw_unique_id2       = 0;
  uint8_t vendor_ie_buffer2[] = {
    0xDD,             // Element ID for vendor-specific IE, always same
    0x06,             // Length of the following bytes (OUI + Data)
    0x12, 0x34, 0x56, // OUI (Organizationally Unique Identifier)
    0x02, 0xAB, 0xCD  // vendor-specific data
  };
  sl_wifi_vendor_ie_t vendor_ie2 = { 0 };
  vendor_ie2.unique_id = SL_WIFI_VENDOR_IE_AUTO_ASSIGN; // While adding new vendor-specific IE use the auto-assign ID.
  vendor_ie2.mgmt_frame_bitmap = SL_WIFI_VENDOR_IE_IN_PROBE_REQ | SL_WIFI_VENDOR_IE_IN_ASSOC_REQ;
  vendor_ie2.ie_buffer_length  = sizeof(vendor_ie_buffer2);
  vendor_ie2.ie_buffer         = vendor_ie_buffer2;
  status                       = sl_wifi_add_vendor_ie(&vendor_ie2, &fw_unique_id2);
  if (status == SL_STATUS_OK) {
    printf("vendor-specific IE added with unique ID: %d\n", fw_unique_id2);
  } else {
    printf("Failed to add vendor-specific IE: 0x%lx\n", status);
  }

  // -------------------------------
  // Updating/replacing existing vendor-specific IE
  // -------------------------------
  uint8_t vendor_ie_buffer3[] = {
    0xDD,                  // Element ID for vendor-specific IE, always same
    0x07,                  // Length of the following bytes (OUI + Data)
    0x50, 0x6F, 0x9A,      // OUI (Organizationally Unique Identifier)
    0x09, 0x02, 0x02, 0x00 // vendor-specific data
  };

  // Updating the existing vendor IE using vendor_ie2.
  // While updating existing vendor-specific IE use the fw given unique id.
  vendor_ie2.unique_id         = fw_unique_id2;
  vendor_ie2.mgmt_frame_bitmap = SL_WIFI_VENDOR_IE_IN_PROBE_REQ | SL_WIFI_VENDOR_IE_IN_ASSOC_REQ;
  vendor_ie2.ie_buffer_length  = sizeof(vendor_ie_buffer3);
  vendor_ie2.ie_buffer         = vendor_ie_buffer3;

  // calling sl_wifi_add_vendor_ie with existing unique ID will replace the existing vendor IE in FW.
  status = sl_wifi_add_vendor_ie(&vendor_ie2, &fw_unique_id2);

  if (status == SL_STATUS_OK) {
    printf("vendor-specific IE with unique ID: %d updated.\n", fw_unique_id2);
  } else {
    printf("Failed to add vendor-specific IE: 0x%lx\n", status);
  }

#endif

  for (size_t i = 0; i < sizeof(data_buffer); i++)
    data_buffer[i] = 'A' + (i % 26);

  switch (THROUGHPUT_TYPE) {
    case TCP_RX:
      receive_data_from_tcp_client();
      break;
    case TCP_TX:
      send_data_to_tcp_server();
      break;
    case UDP_RX:
      receive_data_from_udp_client();
      break;
    case UDP_TX:
      send_data_to_udp_server();
      break;
    default:
      printf("Invalid Throughput test");
  }

#if SL_WIFI_ENABLE_VENDOR_IE
  // -------------------------------
  // Removing vendor-specific IE
  // -------------------------------
  // Calling sl_wifi_remove_vendor_ie with existing unique ID will remove the existing vendor IE from FW.
  status = sl_wifi_remove_vendor_ie(fw_unique_id1);
  if (status == SL_STATUS_OK) {
    printf("vendor-specific IE removed successfully.\n");
  } else {
    printf("Failed to remove vendor-specific IE: 0x%lx\n", status);
  }
#endif

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

#if SL_WIFI_ENABLE_VENDOR_IE
  // -------------------------------
  // Remove all vendor-specific IEs
  // -------------------------------
  sl_status_t remove_status = sl_wifi_remove_all_vendor_ie();
  if (remove_status == SL_STATUS_OK) {
    printf("All vendor-specific IEs removed successfully.\n");
  } else {
    printf("Failed to remove all vendor-specific IEs: 0x%lx\n", remove_status);
  }
#endif

  return SL_STATUS_OK;
}

void send_data_to_tcp_server(void)
{
  int client_socket                 = -1;
  uint32_t total_bytes_sent         = 0;
  int socket_return_value           = 0;
  int sent_bytes                    = 1;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  server_address.sin_family         = AF_INET;
  server_address.sin_port           = SERVER_PORT;
  sl_net_inet_addr(SERVER_IP, &server_address.sin_addr.s_addr);

  client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_socket < 0) {
    printf("\r\nSocket Create failed with bsd error: %d\r\n", errno);
    return;
  }
  printf("\r\nSocket ID : %d\r\n", client_socket);

  socket_return_value = connect(client_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    printf("\r\nSocket Connect failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }
  printf("\r\nSocket connected to TCP server\r\n");

  printf("\r\nTCP_TX Throughput test start\r\n");
  start = osKernelGetTickCount();
  while (total_bytes_sent < BYTES_TO_SEND) {
    sent_bytes = send(client_socket, data_buffer, TCP_BUFFER_SIZE, 0);
    now        = osKernelGetTickCount();

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
  printf("\r\nTCP_TX Throughput test finished\r\n");
  printf("\r\nTotal bytes sent : %ld\r\n", total_bytes_sent);

  measure_and_print_throughput(total_bytes_sent, (now - start));

  close(client_socket);
}
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
    printf("Socket config failed: %ld\r\n", status);
  }
  printf("\r\nSocket config Done\r\n");

#if SOCKET_ASYNC_FEATURE
  server_socket = sl_si91x_socket_async(AF_INET, SOCK_STREAM, IPPROTO_TCP, &data_callback);
  if (server_socket < 0) {
    printf("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  printf("\r\nServer Socket ID : %d\r\n", server_socket);

  socket_return_value = sl_si91x_setsockopt(server_socket,
                                            SOL_SOCKET,
                                            SL_SI91X_SO_HIGH_PERFORMANCE_SOCKET,
                                            &high_performance_socket,
                                            sizeof(high_performance_socket));
  if (socket_return_value < 0) {
    printf("\r\nSet Socket option failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }

#if LISTEN_ON_AP_INTERFACE
  socket_return_value =
    sl_si91x_setsockopt(server_socket, SOL_SOCKET, SL_SI91X_SO_SOCK_VAP_ID, &ap_vap, sizeof(ap_vap));
#endif
  server_address.sin_family = AF_INET;
  server_address.sin_port   = LISTENING_PORT;

  socket_return_value = sl_si91x_bind(server_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    printf("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }

  socket_return_value = sl_si91x_listen(server_socket, BACK_LOG);
  if (socket_return_value < 0) {
    printf("\r\nSocket listen failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }
  printf("\r\nListening on Local Port : %d\r\n", LISTENING_PORT);

  client_socket = sl_si91x_accept(server_socket, NULL, 0);
  if (client_socket < 0) {
    printf("\r\nSocket accept failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }

  while (!has_data_received) {
    osThreadYield();
  }

  now = osKernelGetTickCount();

  printf("\r\nTCP_RX Throughput test finished\r\n");
  printf("\r\nTotal bytes received : %ld\r\n", bytes_read);

  close(server_socket);
  close(client_socket);

  measure_and_print_throughput(bytes_read, (now - start));
#endif
}

void send_data_to_udp_server(void)
{
  int client_socket                 = -1;
  uint32_t total_bytes_sent         = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  int sent_bytes                    = 1;
  server_address.sin_family         = AF_INET;
  server_address.sin_port           = SERVER_PORT;
  sl_net_inet_addr(SERVER_IP, &server_address.sin_addr.s_addr);

  client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (client_socket < 0) {
    printf("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  printf("\r\nSocket ID : %d\r\n", client_socket);

  printf("\r\nUDP_TX Throughput test start\r\n");
  start = osKernelGetTickCount();
  while (total_bytes_sent < BYTES_TO_SEND) {
    sent_bytes =
      sendto(client_socket, data_buffer, UDP_BUFFER_SIZE, 0, (struct sockaddr *)&server_address, socket_length);
    now = osKernelGetTickCount();
    if ((now - start) > TEST_TIMEOUT) {
      printf("\r\nTime Out: %ld\r\n", (now - start));
      break;
    }
    if (sent_bytes < 0) {
      if (errno == ENOBUFS)
        continue;
      printf("\r\nSocket send failed with bsd error: %d\r\n", errno);
      close(client_socket);
      break;
    }
    total_bytes_sent = total_bytes_sent + sent_bytes;
  }
  printf("\r\nUDP_TX Throughput test finished\r\n");
  printf("\r\nTotal bytes sent : %ld\r\n", total_bytes_sent);

  measure_and_print_throughput(total_bytes_sent, (now - start));

  close(client_socket);
}
void receive_data_from_udp_client(void)
{
  int client_socket = -1;

  int socket_return_value           = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);

#if SOCKET_ASYNC_FEATURE
  client_socket = sl_si91x_socket_async(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &data_callback);
  if (client_socket < 0) {
    printf("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  printf("\r\nSocket ID : %d\r\n", client_socket);

#if LISTEN_ON_AP_INTERFACE
  socket_return_value =
    sl_si91x_setsockopt(client_socket, SOL_SOCKET, SL_SI91X_SO_SOCK_VAP_ID, &ap_vap, sizeof(ap_vap));
#endif

  server_address.sin_family = AF_INET;
  server_address.sin_port   = LISTENING_PORT;

  socket_return_value = sl_si91x_bind(client_socket, (struct sockaddr *)&server_address, socket_length);

  if (socket_return_value < 0) {
    printf("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }
  printf("\r\nListening on Local Port %d\r\n", LISTENING_PORT);

  while (!has_data_received) {
    osThreadYield();
  }
  now = osKernelGetTickCount();
  printf("\r\nUDP_RX Async Throughput test finished\r\n");
  printf("\r\nTotal bytes received : %ld\r\n", bytes_read);

  measure_and_print_throughput(bytes_read, (now - start));

  close(client_socket);
#endif
}
