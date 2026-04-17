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
#include "sl_net_wifi_types.h"
#include "cmsis_os2.h"
#include "sl_ip_types.h"
#include "wifiuser.pem.h"
#include "sl_wifi.h"
#include <string.h>
#include "sl_utility.h"

/******************************************************
 *               Function Declarations
 ******************************************************/
static void application_start(void *argument);

/******************************************************
 *                    Constants
 ******************************************************/
#define WIFI_ACCESS_POINT_SSID     "enterprise"
#define WIFI_CLIENT_CERTIFICATE_ID 1

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

//uint8_t data_buffer[BUFFER_SIZE];
sl_ip_address_t ip_address           = { 0 };
sl_net_wifi_client_profile_t profile = { 0 };

static const sl_net_wifi_client_profile_t wifi_client_enterprise_eap_profile = {
    .config = {
        .ssid.value = WIFI_ACCESS_POINT_SSID,
        .ssid.length = sizeof(WIFI_ACCESS_POINT_SSID)-1,
        .channel.channel = SL_WIFI_AUTO_CHANNEL,
        .channel.band = SL_WIFI_AUTO_BAND,
        .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH,
        .bssid = {{0}},
        .bss_type = SL_WIFI_BSS_TYPE_INFRASTRUCTURE,
        .security = SL_WIFI_WPA2_ENTERPRISE,
        .encryption = SL_WIFI_PEAP_MSCHAPV2_ENCRYPTION,
        .client_options = 0,
        .credential_id = SL_NET_WIFI_EAP_CLIENT_CREDENTIAL_ID,
    },
    .ip = {
        .mode = SL_IP_MANAGEMENT_DHCP,
        .type = SL_IPV4,
        .host_name = NULL,
        .ip = {{{0}}},
        .dhcp_config = {
            .min_discover_retry_interval = 5,
            .max_discover_retry_interval = 10,
            .min_request_retry_interval = 5,
            .max_request_retry_interval = 10,
            .max_discover_retries = 5,
            .max_request_retries = 10,
        }
    }
};

static const sl_net_wifi_eap_credential_entry_t wifi_client_enterprise_eap_credential = {
  .type                 = SL_NET_WIFI_PSK,
  .data_length          = sizeof(sl_wifi_eap_credential_t),
  .data.username        = "user1",
  .data.password        = "12345678",
  .data.certificate_key = {},
  .data.certificate_id  = WIFI_CLIENT_CERTIFICATE_ID,
  .data.eap_flags       = 0
};

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
  sl_status_t status = SL_STATUS_OK;

  // Initialize the network interface
  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_default_enterprise_client_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client interface up Success\r\n");

  // Set the custom Wi-Fi client profile
  sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE,
                     SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID,
                     &wifi_client_enterprise_eap_profile);

  // Set the custom Wi-Fi client enterprise credential
  sl_net_set_credential(SL_NET_WIFI_EAP_CLIENT_CREDENTIAL_ID,
                        SL_NET_EAP_CLIENT_CREDENTIAL,
                        &(wifi_client_enterprise_eap_credential.data),
                        sizeof(sl_wifi_eap_credential_t));

  // Load SSL CA certificate
  status =
    sl_net_set_credential(SL_NET_WIFI_EAP_CLIENT_CREDENTIAL_ID, SL_NET_CERTIFICATE, wifiuser, sizeof(wifiuser) - 1);
  if (status != SL_STATUS_OK) {
    printf("\r\nLoading TLS EAP certificate in to FLASH Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
  printf("\r\nLoad TLS EAP certificate Success\r\n");

  // Bring up network interface
  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("Failed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
    return;
  }
  printf("Wi-Fi client connected\r\n");

  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile);
  if (status != SL_STATUS_OK) {
    printf("Failed to get client profile: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nSuccess to get client profile\r\n");

  ip_address.type = SL_IPV4;
  memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
  printf("\r\nIP address of the client: ");
  print_sl_ip_address(&ip_address);
  printf("\r\n");
}
