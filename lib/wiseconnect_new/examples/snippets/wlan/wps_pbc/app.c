/***************************************************************************/ /**
 * @file
 * @brief WPS Push Button Configuration (PBC) Example Application
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "app.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_wifi_credentials.h"
#include "sl_utility.h"
#include "cmsis_os2.h"
#include "sl_wifi_callback_framework.h"
#include "sl_net_si91x.h"
#include <stdio.h>
#include <string.h>

/******************************************************
 *               Constant Definitions
 ******************************************************/
#define SLI_WIFI_CONNECT_TIMEOUT 120000 // 120 seconds in milliseconds
#define KEY_LENGTH               32

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

static const sl_wifi_device_configuration_t wps_client_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_WIFI_BAND_MODE_2_4GHZ,
  .region_code = SL_WIFI_REGION_US,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
#ifdef SLI_SI91X_MCU_INTERFACE
                     (SL_WIFI_FEAT_SECURITY_OPEN),
#else
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION),
#endif
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_SSL | SL_SI91X_TCP_IP_FEAT_MDNSD
#ifdef SLI_SI91X_ENABLE_IPV6
                                              | SL_SI91X_TCP_IP_FEAT_DHCPV6_CLIENT | SL_SI91X_TCP_IP_FEAT_IPV6
#endif
                                              | SL_SI91X_TCP_IP_FEAT_ICMP | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
#if defined(SLI_SI917)
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_WINDOW_SCALING | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(10)),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = 0 }
};

/******************************************************
 *               Function Declarations
 ******************************************************/
static void application_start(void *argument);
static sl_status_t wps_pbc(void);
static void print_wps_response(const sl_wifi_wps_response_t *response);

/******************************************************
 *               Function Definitions
 ******************************************************/

void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static void print_wps_response(const sl_wifi_wps_response_t *response)
{
  if (response == NULL) {
    printf("WPS Response: NULL\r\n");
    return;
  }
  printf("SSID: ");
  if (response->ssid_len > 0 && response->ssid_len <= 32) {
    for (size_t i = 0; i < response->ssid_len; i++) {
      printf("%c", response->ssid[i]);
    }
  } else {
    printf("(empty)");
  }
  printf("\r\n");

  printf("SSID Length: %u bytes\r\n", response->ssid_len);
  printf("Security Type: 0x%02X\r\n", response->security_type);
  printf("Status: 0x%08lX\r\n", response->status);
  printf("Key: ");
  for (int i = 0; i < KEY_LENGTH; i++) {
    printf("%02X", response->key[i]);
  }
  printf("\r\n");
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &wps_client_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to start Wi-Fi Client interface: 0x%lX\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client interface up success\r\n");
  status = wps_pbc();
  if (status != SL_STATUS_OK) {
    printf("\r\nWPS connection failed: 0x%lX\r\n", status);
    return;
  }
  printf("\r\nWPS connection successful\r\n");

  while (1) {
    osDelay(2000);
  }
}

static sl_status_t wps_pbc(void)
{
  sl_wifi_wps_config_t wps_config     = { 0 };
  sl_wifi_wps_response_t wps_response = { 0 };
  sl_status_t status;
  printf("Starting WPS PBC procedure...\r\n");
  // Configure WPS settings
  wps_config.role         = SL_WIFI_WPS_ENROLLEE_ROLE;
  wps_config.mode         = SL_WIFI_WPS_PUSH_BUTTON_MODE;
  wps_config.auto_connect = true;
  memset(wps_config.optional_pin, 0, sizeof(wps_config.optional_pin));
  status = sl_wifi_start_wps_v2(SL_WIFI_CLIENT_INTERFACE, wps_config, &wps_response);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to start WPS PBC: 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nWPS procedure completed successfully!\r\n");
  print_wps_response(&wps_response);
  if (wps_config.auto_connect == false) {
    sl_wifi_client_configuration_t client_config = { 0 };

    // SSID
    memcpy(client_config.ssid.value, wps_response.ssid, wps_response.ssid_len);
    client_config.ssid.length = wps_response.ssid_len;

    // Set security type
    client_config.security = wps_response.security_type;

    // Handle credentials based on security type
    if (wps_response.security_type != SL_WIFI_OPEN) {
      // Set up PMK credential for WPA/WPA2 networks
      sl_wifi_pmk_credential_t pmk_credential = { 0 };

      // Copy the key from WPS response
      memcpy(pmk_credential.value, wps_response.key, KEY_LENGTH);
      // Set the credential using sl_net_set_credential with PMK type
      status = sl_net_set_credential(SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
                                     SL_NET_WIFI_PMK,
                                     pmk_credential.value,
                                     KEY_LENGTH);
      if (status != SL_STATUS_OK) {
        printf("\r\nFailed to set PMK credential: 0x%lX\r\n", status);
        return status;
      }
      // Set credential ID in client configuration
      client_config.credential_id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;
    }

    printf("\r\nConnecting to SSID: %.*s with security type: 0x%X\r\n",
           (int)client_config.ssid.length,
           client_config.ssid.value,
           client_config.security);

    if (client_config.ssid.length == 0) {
      printf("\r\nClient configuration is invalid (empty SSID)\r\n");
      return SL_STATUS_INVALID_PARAMETER;
    }

    status = sl_wifi_connect(SL_WIFI_CLIENT_INTERFACE, &client_config, SLI_WIFI_CONNECT_TIMEOUT);
    if (status != SL_STATUS_OK) {
      printf("\r\nFailed to connect to Wi-Fi: 0x%lX\r\n", status);
      return status;
    }
    printf("\r\nConnected to Wi-Fi successfully!\r\n");

    // Configure IP after successful connection
    sl_net_ip_configuration_t ip_address = { 0 };
    ip_address.type                      = SL_IPV4;
    ip_address.mode                      = SL_IP_MANAGEMENT_DHCP;
    ip_address.host_name                 = NULL;

    status = sl_si91x_configure_ip_address(&ip_address, SL_SI91X_WIFI_CLIENT_VAP_ID);
    if (status != SL_STATUS_OK) {
      printf("\r\nIP Configuration failed, error: 0x%lX\r\n", status);
      return status;
    }

    // Print the assigned IP address
    sl_ip_address_t ip = { 0 };
    ip.type            = ip_address.type;
    ip.ip.v4.value     = ip_address.ip.v4.ip_address.value;
    printf("\r\nIP Configuration successful\r\n");
    print_sl_ip_address(&ip);
  }
  return status;
}