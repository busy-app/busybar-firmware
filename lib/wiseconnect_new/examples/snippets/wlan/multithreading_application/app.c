/***************************************************************************/ /**
 * @file
 * @brief Multithreading Application
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
#include "app.h"
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
/*=======================================================================*/
// NWP buffer allocation parameters
/*=======================================================================*/

#ifndef TX_POOL_RATIO
#define TX_POOL_RATIO 1
#endif

#ifndef RX_POOL_RATIO
#define RX_POOL_RATIO 1
#endif

#ifndef GLOBAL_POOL_RATIO
#define GLOBAL_POOL_RATIO 1
#endif
/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************

 *               Variable Definitions
 ******************************************************/
osMutexId_t printf_mutex                           = 0;
static sl_ip_address_t ip_address                  = { 0 };
static sl_net_wifi_client_profile_t client_profile = { 0 };
sl_net_wifi_ap_profile_t ap_profile;

const osThreadAttr_t station_thread_attributes = {
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

const osThreadAttr_t ap_thread_attributes = {
  .name       = "ap_thread",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 2048,
  .priority   = osPriorityNormal,
  .tz_module  = 0,
  .reserved   = 0,
};

static const sl_wifi_device_configuration_t mqtt_concurrent_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode              = SL_SI91X_CONCURRENT_MODE,
                   .coex_mode              = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map        = SL_WIFI_FEAT_AGGREGATION,
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DHCPV4_SERVER
                                              | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT | SL_SI91X_TCP_IP_FEAT_ICMP
                                              | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID | SL_SI91X_TCP_IP_FEAT_SSL),
                   .custom_feature_bit_map = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS
                      | SL_SI91X_EXT_FEAT_SSL_VERSIONS_SUPPORT
#ifdef SLI_SI917
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_EMB_MQTT_ENABLE | SL_SI91X_EXT_TCP_IP_WINDOW_SCALING
                      | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(10) | SL_SI91X_EXT_TCP_IP_FEAT_SSL_MEMORY_CLOUD
                      | SL_SI91X_EXT_TCP_IP_FEAT_SSL_THREE_SOCKETS),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = 0 }
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

/******************************************************
 *               Function Declarations
 ******************************************************/
extern void mqtt_example();
extern void throughput();
/******************************************************
 *               Function Definitions
 ******************************************************/

void application(void *argument)
{
  UNUSED_PARAMETER(argument);

  int32_t status                   = SL_STATUS_OK;
  sl_wifi_channel_t client_channel = { 0 };

  //! Wi-Fi initialization
  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &mqtt_concurrent_configuration, NULL, NULL);
  if (status != SL_STATUS_OK && status != SL_STATUS_ALREADY_INITIALIZED) {
    LOG_PRINT("\r\nWi-Fi Initialization Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
  LOG_PRINT("\r\nWi-Fi initialization is successful\n");

  status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_client_profile);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFailed to set client profile: 0x%lx\r\n", status);
    return;
  }
  LOG_PRINT("\r\nWi-Fi set client profile success\r\n");

  status = sl_net_set_credential(SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
                                 wifi_client_credential.type,
                                 &wifi_client_credential.data,
                                 wifi_client_credential.data_length);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("Failed to set credentials: 0x%lx\r\n", status);
    return;
  }
  LOG_PRINT("\nWi-Fi set credential success\n");

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFailed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
    return;
  }
  LOG_PRINT("\r\nWi-Fi Client interface up\r\n");

  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &client_profile);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFailed to get client profile: 0x%lx\r\n", status);
    return;
  }
  LOG_PRINT("\r\nSuccess to get client profile\r\n");

  ip_address.type = SL_IPV4;
  memcpy(&ip_address.ip.v4.bytes, &client_profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));

  LOG_PRINT("\r\n IP Address of client:");
  print_sl_ip_address(&ip_address);
  LOG_PRINT("\r\nWi-Fi client connected\r\n");

  //! AP initialization
  status = sl_net_init(SL_NET_WIFI_AP_INTERFACE, &mqtt_concurrent_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFailed to start Wi-Fi AP interface: 0x%lx\r\n", status);
    return;
  }
  LOG_PRINT("\r\nWi-Fi AP interface init\r\n");

  status = sl_wifi_get_channel(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &client_channel);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFailed to get client channel: 0x%lx\r\n", status);
    return;
  }
  LOG_PRINT("\r\nSuccess to get client channel\r\n");

  wifi_ap_profile.config.channel.channel = client_channel.channel;
  status = sl_net_set_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_ap_profile);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFailed to set AP profile: 0x%lx\r\n", status);
    return;
  }
  LOG_PRINT("\r\nSuccess to set AP profile \r\n");

  status = sl_net_set_credential(SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID,
                                 wifi_ap_credential.type,
                                 &wifi_ap_credential.data,
                                 wifi_ap_credential.data_length);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("Failed to set credentials: 0x%lx\r\n", status);
    return;
  }
  LOG_PRINT("\nAP set credential success\n");

  status = sl_net_get_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1, &ap_profile);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFailed to get AP profile: 0x%lx\r\n", status);
    return;
  }
  LOG_PRINT("\r\nSuccess to AP profile\r\n");

  ip_address.type = SL_IPV4;
  memcpy(&ip_address.ip.v4.bytes, &ap_profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));

  LOG_PRINT("\r\n IP Address of AP:");
  print_sl_ip_address(&ip_address);

  status = sl_net_up(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFailed to bring Wi-Fi AP interface up: 0x%lx\r\n", status);
    return;
  }
  LOG_PRINT("\r\nAP started\r\n");

  printf_mutex = osMutexNew(NULL);
  if (printf_mutex == NULL) {
    LOG_PRINT("\r\nFailed to create printf_mutex\r\n");
    return;
  }

  if (osThreadNew((osThreadFunc_t)throughput, NULL, &ap_thread_attributes) == NULL) {
    LOG_PRINT("Failed to create ap thread\n");
  }

  mqtt_example();

  while (1) {
    osDelay(osWaitForever);
  }
  return;
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)application, NULL, &station_thread_attributes);
}
