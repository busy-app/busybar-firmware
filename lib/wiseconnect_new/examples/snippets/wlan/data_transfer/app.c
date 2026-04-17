/***************************************************************************/ /**
 * @file
 * @brief Data transfer in concurrent mode
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
/**
 * Include files
 **/
//! SL Wi-Fi SDK includes
#include "sl_board_configuration.h"
#include "cmsis_os2.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "socket.h"
#include "sl_utility.h"
#include "errno.h"
#include <string.h>
#include "sl_si91x_driver.h"
#include "sl_net_wifi_types.h"
#include "sl_si91x_socket_utility.h"

#include "sl_si91x_socket_support.h"
#include "sl_si91x_socket_constants.h"
#include "sl_si91x_socket.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define WIFI_CLIENT_PROFILE_SSID "YOUR_AP_SSID"
#define WIFI_CLIENT_CREDENTIAL   "YOUR_AP_PASSPHRASE"

#define WIFI_AP_PROFILE_SSID "MY_AP_SSID"
#define WIFI_AP_CREDENTIAL   "MY_AP_PASSPHRASE"

#define WIFI_CLIENT_SECURITY_TYPE   SL_WIFI_WPA_WPA2_MIXED
#define WIFI_CLIENT_ENCRYPTION_TYPE SL_WIFI_CCMP_ENCRYPTION

//! IP address of the module
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#define DEFAULT_WIFI_MODULE_IP_ADDRESS 0x0A0AA8C0

//! IP address of netmask
//! E.g: 0x00FFFFFF == 255.255.255.0
#define DEFAULT_WIFI_SN_MASK_ADDRESS 0x00FFFFFF

//! IP address of Gateway
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#define DEFAULT_WIFI_GATEWAY_ADDRESS 0x0A0AA8C0

// Memory length for send buffer
#define TCP_BUFFER_SIZE 1460
#define SERVER_IP       "192.168.10.11"

// Server port number
#define SERVER_PORT 5000

// Module port number
#define LISTENING_PORT 5005
#define BACK_LOG       1

#define SL_HIGH_PERFORMANCE_SOCKET BIT(7)
/******************************************************
 *                    Constants
 ******************************************************/
/******************************************************

 *               Variable Definitions
 ******************************************************/
static sl_ip_address_t ip_address                  = { 0 };
static sl_net_wifi_client_profile_t client_profile = { 0 };
static sl_net_wifi_ap_profile_t ap_profile         = { 0 };

const osThreadAttr_t thread_attributes = {
  .name       = "application_thread",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityNormal,
  .tz_module  = 0,
  .reserved   = 0,
};

static const sl_net_wifi_client_profile_t wifi_client_profile = {
    .config = {
        .ssid.value = WIFI_CLIENT_PROFILE_SSID,
        .ssid.length = sizeof(WIFI_CLIENT_PROFILE_SSID) - 1,
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

sl_net_wifi_psk_credential_entry_t wifi_client_credential = { .type        = SL_NET_WIFI_PSK,
                                                              .data_length = sizeof(WIFI_CLIENT_CREDENTIAL) - 1,
                                                              .data        = WIFI_CLIENT_CREDENTIAL };

sl_net_wifi_psk_credential_entry_t wifi_ap_credential = { .type        = SL_NET_WIFI_PSK,
                                                          .data_length = sizeof(WIFI_AP_CREDENTIAL) - 1,
                                                          .data        = WIFI_AP_CREDENTIAL };
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

uint8_t data_buffer[TCP_BUFFER_SIZE];
bool is_remote_terminated = 0;
/******************************************************
 *               Function Declarations
 ******************************************************/
void data_transfer_through_client_and_ap_interface();
sl_status_t initialize_wifi_client_interface();
void remote_terminate_callback(int socket_id, uint16_t port_number, uint32_t bytes_sent);

/******************************************************
 *               Function Definitions
 ******************************************************/

void remote_terminate_callback(int socket_id, uint16_t port_number, uint32_t bytes_sent)
{
  is_remote_terminated = 1;
  printf("Remote client terminated on socket: %d, port: %d. Bytes sent: %ld.\r\n", socket_id, port_number, bytes_sent);
}

sl_status_t initialize_wifi_client_interface()
{
  sl_status_t status = SL_STATUS_OK;

  //! Initialize Wi-Fi client interface
  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_default_concurrent_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nWi-Fi Client initialization failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nWi-Fi Client initialization success\r\n");

  //! Set Wi-Fi client profile
  status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_client_profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to store the Wi-Fi client network profile: 0x%lx\r\n", status);
    return status;
  }
  printf("\r\nSuccessfully stored the Wi-Fi client network profile\r\n");

  //! Set network credentials
  status = sl_net_set_credential(SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
                                 wifi_client_credential.type,
                                 &wifi_client_credential.data,
                                 wifi_client_credential.data_length);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to configure Wi-Fi client credentials: 0x%lx\r\n", status);
    return status;
  }
  printf("\r\nConfiguring Wi-Fi client credentials is successful\r\n");

  //! Bring up Wi-Fi client interface
  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring up Wi-Fi client interface up: 0x%lx\r\n", status);
    return status;
  }
  printf("\r\nWi-Fi client interface up\r\n");

  //! Get profile
  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &client_profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to get client profile: 0x%lx\r\n", status);
    return status;
  }
  printf("\r\nGet client profile is successful\r\n");

  ip_address.type = SL_IPV4;
  memcpy(&ip_address.ip.v4.bytes, &client_profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));

  printf("\r\nIP Address of client:");
  print_sl_ip_address(&ip_address);
  printf("\r\nWi-Fi client connected\r\n");

  return status;
}

sl_status_t initialize_wifi_ap_interface()
{
  sl_status_t status               = SL_STATUS_OK;
  sl_wifi_channel_t client_channel = { 0 };

  //! Initialize Wi-Fi Access Point interface
  status = sl_net_init(SL_NET_WIFI_AP_INTERFACE, &sl_wifi_default_concurrent_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nWi-Fi AP initialization failed, Error code: 0x%lx\r\n", status);
    return status;
  }
  printf("\r\nWi-Fi AP initialization success.\r\n");

  //! Get channel
  status = sl_wifi_get_channel(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &client_channel);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to get client channel: 0x%lx\r\n", status);
    return status;
  }
  printf("\r\nSuccess to get client channel.\r\n");

  wifi_ap_profile.config.channel.channel = client_channel.channel;

  //! Set profile
  status = sl_net_set_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_ap_profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to store the Wi-Fi client network profile.: 0x%lx\r\n", status);
    return status;
  }
  printf("\r\nSuccessfully stored the Wi-Fi client network profile.\r\n");

  //! Set credential
  status = sl_net_set_credential(SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID,
                                 wifi_ap_credential.type,
                                 &wifi_ap_credential.data,
                                 wifi_ap_credential.data_length);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to configure Wi-Fi AP credentials: 0x%lx\r\n", status);
    return status;
  }
  printf("\r\nConfigured Wi-Fi AP credentials Successful\r\n");

  //! Bring up AP interface
  status = sl_net_up(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring Wi-Fi AP interface up: 0x%lx\r\n", status);
    return status;
  }
  printf("\r\nAP started\r\n");

  //! Get AP profile
  status = sl_net_get_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1, &ap_profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to get AP profile: 0x%lx\r\n", status);
    return status;
  }
  printf("\r\nSuccess to AP profile\r\n");

  ip_address.type = SL_IPV4;
  memcpy(&ip_address.ip.v4.bytes, &ap_profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));

  printf("\r\n IP Address of AP:");
  print_sl_ip_address(&ip_address);

  return status;
}

void application(void *argument)
{
  UNUSED_PARAMETER(argument);

  int32_t status = SL_STATUS_OK;

  //! Initialize Wi-Fi client interface
  status = initialize_wifi_client_interface();
  if (status != SL_STATUS_OK) {
    return;
  }

  //! Initialize Wi-Fi Access Point interface
  status = initialize_wifi_ap_interface();
  if (status != SL_STATUS_OK) {
    return;
  }

  //! Data transfer from Client to AP interface
  data_transfer_through_client_and_ap_interface();
}

//! Receive data on client interface and store it in data buffer
//! Send the data buffer on AP interface
void data_transfer_through_client_and_ap_interface()
{
  int server_socket                  = -1; //TCP_RX server socket
  int client_socket1                 = -1; //Client socket after server creation
  int client_socket2                 = -1; //TCP_TX socket
  int socket_return_value            = 0;
  int read_bytes                     = 1;
  int sent_bytes                     = 1;
  uint8_t high_performance_socket    = SL_HIGH_PERFORMANCE_SOCKET;
  struct sockaddr_in server_address1 = { 0 };
  struct sockaddr_in server_address2 = { 0 };
  socklen_t socket_length            = sizeof(struct sockaddr_in);

  sl_si91x_set_remote_termination_callback(remote_terminate_callback);

  //! Create server socket to listen for client connection from remote device
  server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_socket < 0) {
    printf("\r\nSocket create failed with BSD error: %d\r\n", errno);
    return;
  }
  printf("\r\nServer socket creation is successful, socket id: %d\r\n", server_socket);

  //! Set socket
  socket_return_value = setsockopt(server_socket,
                                   SOL_SOCKET,
                                   SL_SO_HIGH_PERFORMANCE_SOCKET,
                                   &high_performance_socket,
                                   sizeof(high_performance_socket));
  if (socket_return_value < 0) {
    printf("\r\nSet Socket option failed with BSD error: %d\r\n", errno);
    close(server_socket);
    return;
  }
  server_address1.sin_family = AF_INET;
  server_address1.sin_port   = LISTENING_PORT;

  //! Bind Socket
  socket_return_value = bind(server_socket, (struct sockaddr *)&server_address1, socket_length);
  if (socket_return_value < 0) {
    printf("\r\nSocket bind failed with BSD error: %d\r\n", errno);
    close(server_socket);
    return;
  }

  //! Listen on socket for client connection
  socket_return_value = listen(server_socket, BACK_LOG);
  if (socket_return_value < 0) {
    printf("\r\nSocket listen failed with BSD error: %d\r\n", errno);
    close(server_socket);
    return;
  }
  printf("\r\nListening on Local Port : %d\r\n", LISTENING_PORT);

  //! Accept socket to receive data from remote device
  client_socket1 = accept(server_socket, NULL, NULL);
  if (client_socket1 < 0) {
    printf("\r\nSocket accept failed with BSD error: %d\r\n", errno);
    close(server_socket);
    return;
  }
  printf("\r\nClient Socket ID : %d\r\n", client_socket1);

  //! Create client socket
  client_socket2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_socket2 < 0) {
    printf("\r\nSocket create failed with BSD error: %d\r\n", errno);
    close(server_socket);
    close(client_socket1);
    return;
  }
  printf("\r\nClient socket creation is succesful, socket id: %d\r\n", client_socket2);

  server_address2.sin_family = AF_INET;
  server_address2.sin_port   = SERVER_PORT;
  sl_net_inet_addr(SERVER_IP, &server_address2.sin_addr.s_addr);

  //! Connect client socket
  socket_return_value = connect(client_socket2, (struct sockaddr *)&server_address2, socket_length);
  if (socket_return_value < 0) {
    printf("\r\nSocket Connect failed with BSD error: %d\r\n", errno);
    close(client_socket2);
    return;
  }
  printf("\r\nSocket connected to TCP server\r\n");

  printf("\r\nStart data transfer to remote device\r\n");
  while (1) {
    memset(data_buffer, 0, sizeof(data_buffer));

    //! Receive data from remote device
    read_bytes = recv(client_socket1, data_buffer, sizeof(data_buffer), 0);
    if (read_bytes < 0) {
      if (!(is_remote_terminated)) {
        printf("\r\nReceive failed with BSD error:%d\r\n", errno);
      }
      break;
    }

    printf("\r\nData received from remote device on WiFi client interface: %s\r\n", data_buffer);
    printf("\r\nBytes received : %d\r\n", read_bytes);

    printf("\r\nSend data to the connected third party station from AP interface\r\n");

    //! Send received data to remote device
    while (1) {
      sent_bytes = send(client_socket2, data_buffer, read_bytes, 0);
      if (sent_bytes < 0) {
        if (errno == ENOBUFS)
          continue;
        printf("\r\nSend failed with BSD error:%d\r\n", errno);
        break;
      }
      printf("\r\nData sent: %s\r\n", data_buffer);
      break;
    }
  }

  printf("\r\nExample Demonstration Completed\r\n");

  is_remote_terminated = 0;

  close(server_socket);
  close(client_socket1);
  close(client_socket2);
  return;
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)application, NULL, &thread_attributes);
}
