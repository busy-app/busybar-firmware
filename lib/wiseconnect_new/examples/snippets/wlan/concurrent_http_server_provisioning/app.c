/***************************************************************************/ /**
 * @file
 * @brief Concurrent HTTP Server Provisioning Example Application
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
 ******************************da************************************************/

#include "sl_net.h"
#include "app.h"
#include "errno.h"
#include "sl_utility.h"
#include "sl_wifi.h"
#include "sl_net_wifi_types.h"
#include "sl_si91x_socket_support.h"
#include "sl_si91x_socket_constants.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_socket.h"
#include "sl_net_si91x.h"
#include "login.h"
#include "jsmn.h"
#include "sl_http_server.h"
#include "sl_si91x_socket_utility.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define CLEAN_HTTP_CLIENT_IF_FAILED(status, client_handle, is_sync) \
  {                                                                 \
    if (status != SL_STATUS_OK) {                                   \
      sl_http_client_deinit(client_handle);                         \
      return ((is_sync == 0) ? status : callback_status);           \
    }                                                               \
  }

#define HTTP_SERVER_PORT 80

#define HTTP_SERVER_RESPONSE 0
#define HTTP_SERVER_ENABLE   1

#define WIFI_AP_PROFILE_SSID "MY_DUAL_AP_SSID"
#define WIFI_AP_CREDENTIAL   "MY_AP_PASSPHRASE"

/******************************************************
 *               Variable Definitions
 ******************************************************/

uint8_t has_data_received = 0;
uint32_t bytes_read       = 0;
uint32_t start            = 0;
uint32_t now              = 0;
uint8_t retry             = 5;
uint8_t is_provisioning   = 0;
uint32_t tick_count_s     = 1;
static char response[300] = { 0 };

volatile bool is_server_running = false;

uint8_t address_buffer[SL_IPV6_ADDRESS_LENGTH];

char WIFI_CLIENT_PROFILE_SSID[32]; // Assuming SSID can be up to 32 characters long
char WIFI_CLIENT_CREDENTIAL[64];   // Assuming Password can be up to 64 characters long
char WIFI_CLIENT_SECURITY_TYPE[32];

static jsmntok_t t[128]; /* We expect no more than 128 tokens */
static sl_http_server_t server_handle = { 0 };

sl_status_t callback_status = SL_STATUS_OK;

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

app_state_t app_cb; //! application control block

//! IP address of the module as AP
#define DEFAULT_WIFI_AP_MODULE_IP4_ADDRESS "192.168.10.10"

//! IP address of netmask
#define DEFAULT_WIFI_AP_NS_MODULE_IP4_ADDRESS "255.255.255.0"

//! IP address of Gateway
#define DEFAULT_WIFI_AP_GATEWAY4_ADDRESS "192.168.10.10"

//! IPv6 address of the module
#define DEFAULT_WIFI_AP_MODULE_IP6_ADDRESS "2001:db8:0:1::121"

//! IP address of Gateway
#define DEFAULT_WIFI_AP_GATEWAY6_ADDRESS "2001:db8:0:1::121"

static sl_net_wifi_client_profile_t wifi_client_profile_4 = {
    .config = {
        .channel.channel = SL_WIFI_AUTO_CHANNEL,
        .channel.band = SL_WIFI_AUTO_BAND,
        .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH,
        .bssid = {{0}},
        .bss_type = SL_WIFI_BSS_TYPE_INFRASTRUCTURE,
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

static sl_net_wifi_client_profile_t wifi_client_profile_6 = {
    .config = {
        .channel.channel = SL_WIFI_AUTO_CHANNEL,
        .channel.band = SL_WIFI_AUTO_BAND,
        .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH,
        .bssid = {{0}},
        .bss_type = SL_WIFI_BSS_TYPE_INFRASTRUCTURE,
        .client_options = 0,
        .credential_id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
    },
    .ip = {
        .mode = SL_IP_MANAGEMENT_DHCP,
        .type = SL_IPV6,
        .host_name = NULL,
        .ip = {{{0}}},
    }
};

static sl_net_wifi_ap_profile_t wifi_ap_profile_4 = {
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
        .beacon_stop = 0,
    },
    .ip = {
        .mode = SL_IP_MANAGEMENT_STATIC_IP,
        .type = SL_IPV4,
        .host_name = NULL,
        .ip = {{{0}}},
    }
};

static sl_net_wifi_ap_profile_t wifi_ap_profile_6 = {
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
        .type = SL_IPV6,
        .host_name = NULL,
        .ip = {{{0}}},
    }
};

sl_net_wifi_psk_credential_entry_t wifi_ap_credential = { .type        = SL_NET_WIFI_PSK,
                                                          .data_length = sizeof(WIFI_AP_CREDENTIAL) - 1,
                                                          .data        = WIFI_AP_CREDENTIAL };

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

extern osMutexId_t printf_mutex;

/******************************************************
 *               Function Declarations
 ******************************************************/

void parse_json_response(void);
static void application_start(void *argument);
sl_wifi_security_t string_to_security_type(const char *securityType);
sl_status_t login_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
sl_status_t default_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
sl_status_t connect_data_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
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
uint8_t start_wifi_throughput();

/******************************************************
 *               Function Definitions
 ******************************************************/

void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

sl_status_t join_callback_handler(sl_wifi_event_t event,
                                  sl_status_t status_code,
                                  char *result,
                                  uint32_t result_length,
                                  void *arg)
{
  UNUSED_PARAMETER(result);
  UNUSED_PARAMETER(arg);

  LOG_PRINT("in join CB\r\n");
  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    LOG_PRINT("F: Join Event received with %lu bytes payload\n", result_length);
    app_cb = MODULE_DISCONNECT_STATE;
    return status_code;
  }
  return SL_STATUS_OK;
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status                             = SL_STATUS_OK;
  sl_wifi_channel_t client_channel               = { 0 };
  sl_net_wifi_client_profile_t profile           = { 0 };
  sl_ip_address_t ip_address                     = { 0 };
  unsigned char hex_addr[SL_IPV6_ADDRESS_LENGTH] = { 0 };
  uint32_t ip_addr[4]                            = { 0 };
  uint32_t gateway[4]                            = { 0 };

  static sl_http_server_handler_t request_handlers[2] = { { .uri = "/login", .handler = login_request_handler },
                                                          { .uri = "/connect", .handler = connect_data_handler } };

  sl_http_server_config_t server_config = { 0 };
  printf_mutex                          = osMutexNew(NULL);
  if (printf_mutex == NULL) {
    printf("Failed to create printf_mutex\r\n");
    return;
  }

  int return_value;

  while (1) {
    switch (app_cb) {

      case INITIAL_STATE:

        status = sl_net_init(SL_NET_WIFI_AP_INTERFACE, &sl_wifi_default_concurrent_v6_configuration, NULL, NULL);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to start Wi-Fi AP interface: 0x%lx\r\n", status);
          return;
        }

        sl_wifi_set_callback_v2(SL_WIFI_CLIENT_CONNECTED_EVENTS, ap_connected_event_handler, NULL);
        sl_wifi_set_callback_v2(SL_WIFI_CLIENT_DISCONNECTED_EVENTS, ap_disconnected_event_handler, NULL);
        LOG_PRINT("\r\nWi-Fi AP interface init Success");

        status = sl_net_up(SL_NET_WIFI_AP_INTERFACE, SL_NET_DEFAULT_WIFI_AP_PROFILE_ID);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to bring Wi-Fi AP interface up: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nAP started\r\n");

        is_provisioning = 1;
        if (app_cb == INITIAL_STATE)
          app_cb = WAIT_STATE;

        break;

      case PROVISION_STATE:

        server_config.port             = HTTP_SERVER_PORT;
        server_config.default_handler  = default_handler;
        server_config.handlers_list    = request_handlers;
        server_config.handlers_count   = 2;
        server_config.client_idle_time = 1;

        status = sl_http_server_init(&server_handle, &server_config);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nHTTP server init failed:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nHttp Init done\r\n");

        status = sl_http_server_start(&server_handle);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nServer start fail:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nServer start done\r\n");

        osDelay(10000);

        is_server_running = true;

        //! Server is exited when we receive the credentials
        while (is_server_running) {
          osDelay(100);
        }

        parse_json_response();

        SL_DEBUG_LOG("SSID stored:%s\r\n", WIFI_CLIENT_PROFILE_SSID);
        SL_DEBUG_LOG("Password stored:%s\r\n", WIFI_CLIENT_CREDENTIAL);
        SL_DEBUG_LOG("Security type stored:%s\r\n", WIFI_CLIENT_SECURITY_TYPE);

        status = sl_http_server_stop(&server_handle);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nServer stop fail:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nServer stop done\r\n");

        status = sl_http_server_deinit(&server_handle);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nServer deinit fail:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nServer deinit done\r\n");

        status = sl_net_deinit(SL_NET_WIFI_AP_INTERFACE);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nAp deinit : 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nAP Deinit success\r\n");

        app_cb = STA_BRINGUP_STATE;

        break;

      case STA_BRINGUP_STATE:

        status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_default_concurrent_v6_configuration, NULL, NULL);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to start Wi-Fi client interface: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nWi-Fi Client interface init\r\n");

        sl_wifi_set_callback_v2(SL_WIFI_CLIENT_CONNECTED_EVENTS, ap_connected_event_handler, NULL);
        sl_wifi_set_callback_v2(SL_WIFI_CLIENT_DISCONNECTED_EVENTS, ap_disconnected_event_handler, NULL);

        wifi_client_profile_4.config.ssid.length = strlen((char *)WIFI_CLIENT_PROFILE_SSID);
        memcpy(wifi_client_profile_4.config.ssid.value,
               WIFI_CLIENT_PROFILE_SSID,
               wifi_client_profile_4.config.ssid.length);
        wifi_client_profile_4.config.security      = string_to_security_type(WIFI_CLIENT_SECURITY_TYPE);
        wifi_client_profile_4.config.credential_id = (wifi_client_profile_4.config.security == SL_WIFI_OPEN)
                                                       ? SL_NET_NO_CREDENTIAL_ID
                                                       : SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;

        //  Keeping the station ipv4 record in profile_id_0
        status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_0, &wifi_client_profile_4);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to set client profile: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nWi-Fi set client profile v4 success\r\n");

        wifi_client_profile_6.config.ssid.length = strlen((char *)WIFI_CLIENT_PROFILE_SSID);
        memcpy(wifi_client_profile_6.config.ssid.value,
               WIFI_CLIENT_PROFILE_SSID,
               wifi_client_profile_6.config.ssid.length);
        wifi_client_profile_6.config.security      = string_to_security_type(WIFI_CLIENT_SECURITY_TYPE);
        wifi_client_profile_6.config.credential_id = (wifi_client_profile_6.config.security == SL_WIFI_OPEN)
                                                       ? SL_NET_NO_CREDENTIAL_ID
                                                       : SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;
        //  Keeping the station ipv6 record in profile_id_1
        status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_client_profile_6);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to set client profile: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nWi-Fi set client profile v6 success\r\n");

        SL_DEBUG_LOG("\r\nSTA credentials through http server");

        sl_wifi_security_t sec_type = string_to_security_type(WIFI_CLIENT_SECURITY_TYPE);
        sl_wifi_credential_id_t id;
        sl_status_t cred_status = SL_STATUS_OK;

        if (sec_type == SL_WIFI_OPEN) {
          id = SL_NET_NO_CREDENTIAL_ID;
          SL_DEBUG_LOG("Connecting to Open network - no credentials required\n");
        } else {
          // Handle PSK-based security (WPA, WPA2, WPA3, etc.)
          id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;

          // Only set credentials if password is not empty
          if (strlen(WIFI_CLIENT_CREDENTIAL) > 0) {
            cred_status = sl_net_set_credential(id,
                                                SL_NET_WIFI_PSK,
                                                WIFI_CLIENT_CREDENTIAL,
                                                strlen((char *)WIFI_CLIENT_CREDENTIAL));
          } else {
            printf("Error: Password required for %s security\n", WIFI_CLIENT_SECURITY_TYPE);
            return;
          }
        }

        if (sec_type == SL_WIFI_OPEN || cred_status == SL_STATUS_OK) {
          SL_DEBUG_LOG("Credentials set, id : %lu\n", id);
          sl_wifi_client_configuration_t access_point = { 0 };
          access_point.ssid.length                    = strlen((char *)WIFI_CLIENT_PROFILE_SSID);
          memcpy(access_point.ssid.value, WIFI_CLIENT_PROFILE_SSID, access_point.ssid.length);
          access_point.security      = sec_type;
          access_point.encryption    = SL_WIFI_CCMP_ENCRYPTION;
          access_point.credential_id = id;

          SL_DEBUG_LOG("SSID %s\n", access_point.ssid.value);

          sl_wifi_set_join_callback_v2(join_callback_handler, NULL);
          status = sl_wifi_connect(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &access_point, 25000);
        } else {
          LOG_PRINT("\r\nFailed to set credentials; status: %lu\n", status);
        }
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
          //! If scan failed retry again after 5 seconds
          if (status == 0x10003) {
            if (retry) {
              retry--;
              osDelay(5000);
              break;
            } else {
              retry = 5;
            }
          }
          status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
          if (status != SL_STATUS_OK) {
            LOG_PRINT("\r\nSTA deinit : 0x%lx\r\n", status);
          }
          app_cb = INITIAL_STATE;
          break;
        }

        status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_0, &profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
          return;
        }

        status = sl_si91x_configure_ip_address(&profile.ip, SL_SI91X_WIFI_CLIENT_VAP_ID);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nIPv4 address configuration is failed : 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nIPv4 address configuration is done");

        if (profile.ip.type == SL_IPV4) {
          ip_address.type = SL_IPV4;
          memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
          LOG_PRINT("\r\nClient IPv4 : ");
          print_sl_ip_address(&ip_address);
        }

        //  Station ipv6 config using profile_id_1
        status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
          return;
        }

        status = sl_si91x_configure_ip_address(&profile.ip, SL_SI91X_WIFI_CLIENT_VAP_ID);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nIPv6 address configuration is failed : 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nIPv6 address configuration is done");

        status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to set client profile: 0x%lx\r\n", status);
          return;
        }

        status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to get client_v6 profile: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nClient IPv6 : ");

        if (profile.ip.type == SL_IPV6) {
          sl_ip_address_t link_local_address = { 0 };
          link_local_address.ip.v6           = profile.ip.ip.v6.link_local_address;
          link_local_address.type            = SL_IPV6;
          LOG_PRINT("\nLink Local Address: ");
          print_sl_ip_address(&link_local_address);

          sl_ip_address_t global_address = { 0 };
          global_address.ip.v6           = profile.ip.ip.v6.global_address;
          global_address.type            = SL_IPV6;
          LOG_PRINT("Global Address: ");
          print_sl_ip_address(&global_address);

          sl_ip_address_t gateway = { 0 };
          gateway.ip.v6           = profile.ip.ip.v6.gateway;
          gateway.type            = SL_IPV6;
          LOG_PRINT("Gateway Address: ");
          print_sl_ip_address(&gateway);
        }

        app_cb = AP_BRINGUP_STATE;

        break;

      case AP_BRINGUP_STATE:

        status = sl_net_init(SL_NET_WIFI_AP_INTERFACE, &sl_wifi_default_concurrent_v6_configuration, NULL, NULL);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to start Wi-Fi AP interface: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nWi-Fi AP interface init\r\n");

        sl_net_inet_addr(DEFAULT_WIFI_AP_MODULE_IP4_ADDRESS, (uint32_t *)&wifi_ap_profile_4.ip.ip.v4.ip_address.bytes);
        sl_net_inet_addr(DEFAULT_WIFI_AP_NS_MODULE_IP4_ADDRESS, (uint32_t *)&wifi_ap_profile_4.ip.ip.v4.netmask.bytes);
        sl_net_inet_addr(DEFAULT_WIFI_AP_GATEWAY4_ADDRESS, (uint32_t *)&wifi_ap_profile_4.ip.ip.v4.gateway.bytes);

        return_value = sl_inet_pton6(DEFAULT_WIFI_AP_MODULE_IP6_ADDRESS,
                                     DEFAULT_WIFI_AP_MODULE_IP6_ADDRESS + strlen(DEFAULT_WIFI_AP_MODULE_IP6_ADDRESS),
                                     hex_addr,
                                     (unsigned int *)ip_addr);
        if (return_value != 0x1) {
          LOG_PRINT("\r\nIPv6 conversion failed.\r\n");
          return;
        }
        return_value = sl_inet_pton6(DEFAULT_WIFI_AP_GATEWAY6_ADDRESS,
                                     DEFAULT_WIFI_AP_GATEWAY6_ADDRESS + strlen(DEFAULT_WIFI_AP_GATEWAY6_ADDRESS),
                                     hex_addr,
                                     (unsigned int *)gateway);
        if (return_value != 0x1) {
          LOG_PRINT("\r\nIPv6 conversion failed.\r\n");
          return;
        }

        memcpy(wifi_ap_profile_6.ip.ip.v6.global_address.value, ip_addr, sizeof(sl_ipv6_address_t));
        memcpy(wifi_ap_profile_6.ip.ip.v6.gateway.value, gateway, sizeof(sl_ipv6_address_t));

        status = sl_wifi_get_channel(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &client_channel);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to get client channel: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nSuccess to get client channel\r\n");

        wifi_ap_profile_4.config.channel.channel = client_channel.channel;
        wifi_ap_profile_6.config.channel.channel = client_channel.channel;

        status = sl_net_set_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_0, &wifi_ap_profile_4);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to set AP profile: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nSuccess to set AP v4 profile \r\n");

        status = sl_net_set_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_ap_profile_6);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to set AP profile: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nSuccess to set AP v6 profile \r\n");

        status = sl_net_set_credential(SL_NET_USER_CREDENTIAL_ID,
                                       wifi_ap_credential.type,
                                       &wifi_ap_credential.data,
                                       wifi_ap_credential.data_length);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to set credentials: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nWi-Fi set credential success\n");

        status = sl_si91x_configure_ip_address(&wifi_ap_profile_4.ip, SL_SI91X_WIFI_AP_VAP_ID);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nIPv4 address configuration is failed : 0x%lx\r\n", status);
          return;
        }
        status = sl_net_get_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_0, &wifi_ap_profile_4);

        if (wifi_ap_profile_4.ip.type == SL_IPV4) {
          ip_address.type = SL_IPV4;
          LOG_PRINT("\r\nAP IPv4 : ");
          memcpy(&ip_address.ip.v4.bytes, &wifi_ap_profile_4.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
          print_sl_ip_address(&ip_address);
        }

        status = sl_net_get_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_ap_profile_6);

        status = sl_si91x_configure_ip_address(&wifi_ap_profile_6.ip, SL_SI91X_WIFI_AP_VAP_ID);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nIPv4 address configuration is failed : 0x%lx\r\n", status);
          return;
        }

        LOG_PRINT("\r\nAP IPv6 : ");
        if (wifi_ap_profile_6.ip.type == SL_IPV6) {
          sl_ip_address_t link_local_address = { 0 };
          link_local_address.ip.v6           = wifi_ap_profile_6.ip.ip.v6.link_local_address;
          link_local_address.type            = SL_IPV6;
          LOG_PRINT("\nLink Local Address: ");
          print_sl_ip_address(&link_local_address);
          sl_ip_address_t global_address = { 0 };
          global_address.ip.v6           = wifi_ap_profile_6.ip.ip.v6.global_address;
          global_address.type            = SL_IPV6;
          LOG_PRINT("Global Address: ");
          print_sl_ip_address(&global_address);

          sl_ip_address_t gateway = { 0 };
          gateway.ip.v6           = wifi_ap_profile_6.ip.ip.v6.gateway;
          gateway.type            = SL_IPV6;
          LOG_PRINT("Gateway Address: ");
          print_sl_ip_address(&gateway);
        }

        status = sl_wifi_start_ap(SL_WIFI_AP_2_4GHZ_INTERFACE, &wifi_ap_profile_4.config);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nAp start fail: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nAP started\r\n");

        status = sl_si91x_config_socket(socket_config);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("Socket config failed: %ld \r\n", status);
        }
        LOG_PRINT("Socket config Done : \r\n");

#if HTTP_SERVER_ENABLE
        server_config.port            = HTTP_SERVER_PORT;
        server_config.default_handler = default_handler;
        server_config.handlers_list   = request_handlers;
        server_config.handlers_count  = 2;

        status = sl_http_server_init(&server_handle, &server_config);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nHTTP server init failed:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nHttp Init done\r\n");

        status = sl_http_server_start(&server_handle);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nServer start fail:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nServer start done\r\n");
#if HTTP_SERVER_RESPONSE
        is_server_running = true;
        while (is_server_running) { //Server is exited when we receive the credentials
          osDelay(100);
        }

        parse_json_response();

        SL_DEBUG_LOG("SSID stored:%s\r\n", WIFI_CLIENT_PROFILE_SSID);
        SL_DEBUG_LOG("Password stored:%s\r\n", WIFI_CLIENT_CREDENTIAL);
        SL_DEBUG_LOG("Security type stored:%s\r\n", WIFI_CLIENT_SECURITY_TYPE);

#endif
#endif

        is_provisioning = 0;
        app_cb          = DATA_TRANSFER_STATE;

        break;

      case DATA_TRANSFER_STATE:

        start_wifi_throughput();

        if (app_cb == DATA_TRANSFER_STATE)
          app_cb = WAIT_STATE;

        break;

      case WAIT_STATE:
        osDelay(1000);
        break;

      case MODULE_DISCONNECT_STATE:

        //! Upon rejoin failure MODULE_DISCONNECT_STATE is called. Hence, pulling down HTTP server & AP
        status = sl_http_server_stop(&server_handle);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n Server stop fail:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\n Server stop done\r\n");

        status = sl_http_server_deinit(&server_handle);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n Server deinit fail:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nServer deinit done\r\n");

        osDelay(2000);

        status = sl_net_down(SL_NET_WIFI_AP_INTERFACE);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nAp stop fail: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nAP Disconnected\r\n");

        app_cb = STA_BRINGUP_STATE;

        break;
    }
  }
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

  LOG_PRINT("Remote Client connected: ");
  print_mac_address((sl_mac_address_t *)data);
  LOG_PRINT("\n");
  if (is_provisioning)
    app_cb = PROVISION_STATE;

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

  LOG_PRINT("Remote Client disconnected: ");
  print_mac_address((sl_mac_address_t *)data);
  LOG_PRINT("\n");

  return SL_STATUS_OK;
}

sl_status_t login_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = { 0 };

  LOG_PRINT("Got request %s with data length : %lu\n", req->uri.path, req->request_data_length);

  // Set the response code to 200 (OK)
  http_response.response_code = SL_HTTP_RESPONSE_OK;

  // Set the content type to plain text
  http_response.content_type = SL_HTTP_CONTENT_TYPE_TEXT_HTML;

  // Set the response data which is the connect page
  http_response.data                 = (uint8_t *)login_content;
  http_response.current_data_length  = strlen((const char *)login_content);
  http_response.expected_data_length = http_response.current_data_length;
  sl_http_server_send_response(handle, &http_response);
  return SL_STATUS_OK;
}

sl_status_t connect_data_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_recv_req_data_t recvData        = { 0 };
  sl_http_server_response_t http_response = { 0 };

  LOG_PRINT("Got request %s with data length : %lu\n", req->uri.path, req->request_data_length);
  if (req->request_data_length > 0) {
    recvData.request       = req;
    recvData.buffer        = (uint8_t *)response;
    recvData.buffer_length = 1024;

    sl_http_server_read_request_data(handle, &recvData);
    response[recvData.received_data_length] = 0;
    LOG_PRINT("Got request data as : %s\n", response);
  }

  // Set the response code to 200 (OK)
  http_response.response_code = SL_HTTP_RESPONSE_OK;

  // Set the content type to plain text
  http_response.content_type = SL_HTTP_CONTENT_TYPE_TEXT_HTML;

  // Set the response data as "Done"
  char *response_data                = "Done";
  http_response.data                 = (uint8_t *)response_data;
  http_response.current_data_length  = strlen(response_data);
  http_response.expected_data_length = http_response.current_data_length;
  sl_http_server_send_response(handle, &http_response);
  is_server_running = false;
  return SL_STATUS_OK;
}

sl_status_t default_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_recv_req_data_t recvData        = { 0 };
  sl_http_server_response_t http_response = { 0 };
  sl_http_header_t header                 = { .key = "Server", .value = "SI917-HTTPServer" };

  LOG_PRINT("Got request %s with data length : %lu\n", req->uri.path, req->request_data_length);
  if (req->request_data_length > 0) {
    recvData.request       = req;
    recvData.buffer        = (uint8_t *)response;
    recvData.buffer_length = 1024;

    sl_http_server_read_request_data(handle, &recvData);
    response[recvData.received_data_length] = 0;
    LOG_PRINT("Got request data as : %s\n", response);
  }

  http_response.response_code = SL_HTTP_RESPONSE_NOT_FOUND;
  http_response.content_type  = SL_HTTP_CONTENT_TYPE_TEXT_PLAIN;
  http_response.headers       = &header;
  http_response.header_count  = 1;

  // Set the response data to "Hello, World!"
  char *response_data                = "Invalid request!";
  http_response.data                 = (uint8_t *)response_data;
  http_response.current_data_length  = strlen(response_data);
  http_response.expected_data_length = http_response.current_data_length;
  sl_http_server_send_response(handle, &http_response);

  return SL_STATUS_OK;
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start
      && strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

void parse_json_response()
{
  int i;
  int r;
  jsmn_parser p;

  jsmn_init(&p);
  r = jsmn_parse(&p, response, strlen(response), t, sizeof(t) / sizeof(t[0]));
  if (r < 0) {
    LOG_PRINT("Failed to parse JSON: %d\n", r);
    return;
  }

  /* Assume the top-level element is an object */
  if (r < 1 || t[0].type != JSMN_OBJECT) {
    LOG_PRINT("Object expected\n");
    return;
  }

  /* Loop over all keys of the root object */
  for (i = 1; i < r; i++) {
    if (jsoneq(response, &t[i], "id") == 0) {
      /* We may use strndup() to fetch string value */
      SL_DEBUG_LOG("- SSID: %.*s\n", t[i + 1].end - t[i + 1].start, response + t[i + 1].start);
      snprintf(WIFI_CLIENT_PROFILE_SSID,
               sizeof(WIFI_CLIENT_PROFILE_SSID),
               "%.*s",
               t[i + 1].end - t[i + 1].start,
               response + t[i + 1].start);
      i++;
    } else if (jsoneq(response, &t[i], "password") == 0) {
      /* Only store password if it's not empty (not Open security) */
      if (t[i + 1].end - t[i + 1].start > 0) {
        SL_DEBUG_LOG("- Password: %.*s\n", t[i + 1].end - t[i + 1].start, response + t[i + 1].start);
        snprintf(WIFI_CLIENT_CREDENTIAL,
                 sizeof(WIFI_CLIENT_CREDENTIAL),
                 "%.*s",
                 t[i + 1].end - t[i + 1].start,
                 response + t[i + 1].start);
      } else {
        /* For Open networks, set empty password */
        WIFI_CLIENT_CREDENTIAL[0] = '\0';
        SL_DEBUG_LOG("- Password: (empty for Open network)\n");
      }
      i++;
    } else if (jsoneq(response, &t[i], "securityType") == 0) {
      /* We may want to do strtol() here to get numeric value */
      SL_DEBUG_LOG("- Security Type: %.*s\n", t[i + 1].end - t[i + 1].start, response + t[i + 1].start);
      snprintf(WIFI_CLIENT_SECURITY_TYPE,
               sizeof(WIFI_CLIENT_SECURITY_TYPE),
               "%.*s",
               t[i + 1].end - t[i + 1].start,
               response + t[i + 1].start);
      i++;
    } else {
      LOG_PRINT("Unexpected key: %.*s\n", t[i].end - t[i].start, response + t[i].start);
    }
  }
}

sl_wifi_security_t string_to_security_type(const char *securityType)
{
  if (strcmp(securityType, "Open") == 0) {
    return SL_WIFI_OPEN;
  } else if (strcmp(securityType, "WPA") == 0) {
    return SL_WIFI_WPA;
  } else if (strcmp(securityType, "WPA2") == 0) {
    return SL_WIFI_WPA2;
  } else if (strcmp(securityType, "Mixed Mode") == 0) {
    return SL_WIFI_WPA_WPA2_MIXED;
  } else if (strcmp(securityType, "WPA3") == 0) {
    return SL_WIFI_WPA3;
  } else if (strcmp(securityType, "WPA3_Transition") == 0) {
    return SL_WIFI_WPA3_TRANSITION;
  } else {
    return SL_WIFI_SECURITY_UNKNOWN;
  }
}
