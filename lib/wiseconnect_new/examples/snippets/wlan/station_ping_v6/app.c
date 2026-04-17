/***************************************************************************/ /**
 * @file
 * @brief Station Ping Example Application
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
#include "cmsis_os2.h"
#include "sl_net.h"
#include "sl_board_configuration.h"
#include "sl_net_ping.h"
#include "sl_utility.h"
#include "sl_si91x_driver.h"
#include "sl_wifi.h"
#include "sl_net_wifi_types.h"
#include "sl_net_default_values.h"
#include <string.h>

/******************************************************
 *                    Constants
 ******************************************************/
#define REMOTE_IP_ADDRESS "2001:4860:4860::8888"

#define CONNECT_WITH_PMK 0

#define PING_PACKET_SIZE 64

/******************************************************
 *               Variable Definitions
 ******************************************************/
uint8_t address_buffer[SL_IPV6_ADDRESS_LENGTH];
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

/******************************************************
 *               Function Declarations
 ******************************************************/
static void application_start(void *argument);
static sl_status_t network_event_handler(sl_net_event_t event, sl_status_t status, void *data, uint32_t data_length);

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
  sl_net_wifi_client_profile_t profile = { 0 };

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, NULL, NULL, network_event_handler);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to start Wi-Fi Client interface: 0x%lX\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client interface up success\r\n");

#if CONNECT_WITH_PMK
  uint8_t pairwise_master_key[32] = { 0 };
  sl_wifi_ssid_t ssid;
  uint8_t type = 3;
  ssid.length  = (uint8_t)(sizeof(DEFAULT_WIFI_CLIENT_PROFILE_SSID) - 1);
  memcpy(ssid.value, DEFAULT_WIFI_CLIENT_PROFILE_SSID, ssid.length);

  status = sl_wifi_get_pairwise_master_key(SL_WIFI_CLIENT_INTERFACE,
                                           type,
                                           &ssid,
                                           DEFAULT_WIFI_CLIENT_CREDENTIAL,
                                           pairwise_master_key);
  if (status != SL_STATUS_OK) {
    printf("\r\nGet Pairwise Master Key Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
  printf("\r\nGet Pairwise Master Key Success\r\n");

  status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE,
                              SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID,
                              &DEFAULT_WIFI_CLIENT_PROFILE);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to set client profile: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi set client profile success\r\n");

  status = sl_net_set_credential(SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
                                 SL_NET_WIFI_PMK,
                                 pairwise_master_key,
                                 sizeof(pairwise_master_key));
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed sl_net_set_credential: 0x%lX\r\n", status);
    return;
  }
  printf("\r\nPMK Credentials are set successfully\r\n");
#endif
  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring Wi-Fi client interface up: 0x%lX\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client connected\r\n");

  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile);
  if (status != SL_STATUS_OK) {
    printf("Failed to get client profile: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nClient profile is fetched successfully\r\n");

  sl_ip_address_t link_local_address = { 0 };
  memcpy(&link_local_address.ip.v6, &profile.ip.ip.v6.link_local_address, SL_IPV6_ADDRESS_LENGTH);
  link_local_address.type = SL_IPV6;
  printf("Link Local Address: ");
  print_sl_ip_address(&link_local_address);

  sl_ip_address_t global_address = { 0 };
  memcpy(&global_address.ip.v6, &profile.ip.ip.v6.global_address, SL_IPV6_ADDRESS_LENGTH);
  global_address.type = SL_IPV6;
  printf("Global Address: ");
  print_sl_ip_address(&global_address);

  sl_ip_address_t gateway = { 0 };
  memcpy(&gateway.ip.v6, &profile.ip.ip.v6.gateway, SL_IPV6_ADDRESS_LENGTH);
  gateway.type = SL_IPV6;
  printf("Gateway Address: ");
  print_sl_ip_address(&gateway);

  sl_ip_address_t remote_ip_address = { 0 };

  status = sl_inet_pton6(REMOTE_IP_ADDRESS,
                         REMOTE_IP_ADDRESS + strlen(REMOTE_IP_ADDRESS),
                         address_buffer,
                         (unsigned int *)remote_ip_address.ip.v6.value);
  if (status != 0x1) {
    printf("\r\nIPv6 conversion failed.\r\n");
    return;
  }
  remote_ip_address.type = SL_IPV6;

  while (1) {
    // Send ping
    status = sl_si91x_send_ping(remote_ip_address, PING_PACKET_SIZE);
    if (status != SL_STATUS_IN_PROGRESS) {
      printf("\r\nPing request failed with status 0x%lX\r\n", status);
      return;
    }

    osDelay(2000);
  }
}

static sl_status_t network_event_handler(sl_net_event_t event, sl_status_t status, void *data, uint32_t data_length)
{
  UNUSED_PARAMETER(data_length);
  switch (event) {
    case SL_NET_PING_RESPONSE_EVENT: {
      sl_ip_address_t remote_ip_address = { 0 };
      sl_net_ping_response_t *response  = (sl_net_ping_response_t *)data;
      if (status != SL_STATUS_OK) {
        printf("\r\nPing request failed with status 0x%lX\r\n", status);
        return status;
      }
      if (response->ip_version == SL_IPV4_VERSION) {
        printf("\r\n%u bytes received from %u.%u.%u.%u\r\n",
               response->ping_size,
               response->ping_address.ipv4_address[0],
               response->ping_address.ipv4_address[1],
               response->ping_address.ipv4_address[2],
               response->ping_address.ipv4_address[3]);
      } else if (response->ip_version == SL_IPV6_VERSION) {
        memcpy(&remote_ip_address.ip.v6.bytes, &(response->ping_address.ipv6_address), SL_IPV6_ADDRESS_LENGTH);
        remote_ip_address.type = SL_IPV6;
        printf("\r\n%u bytes received from: ", response->ping_size);
        print_sl_ip_address(&remote_ip_address);
      }
      break;
    }
    case SL_NET_DHCP_NOTIFICATION_EVENT: {
      printf("\r\nReceived DHCP Notification event with status : 0x%lX\r\n", status);
      break;
    }
    case SL_NET_IP_ADDRESS_CHANGE_EVENT: {
      sl_net_ip_configuration_t *ip_config = (sl_net_ip_configuration_t *)data;

      if (ip_config->type == SL_IPV4) {
        printf("\r\nReceived Ip Address Change Notification event with status : 0x%lX\r\n", status);
        printf("\t Ip Address : %u.%u.%u.%u\r\n",
               ip_config->ip.v4.ip_address.bytes[0],
               ip_config->ip.v4.ip_address.bytes[1],
               ip_config->ip.v4.ip_address.bytes[2],
               ip_config->ip.v4.ip_address.bytes[3]);
        printf("\t Netmask : %u.%u.%u.%u\r\n",
               ip_config->ip.v4.netmask.bytes[0],
               ip_config->ip.v4.netmask.bytes[1],
               ip_config->ip.v4.netmask.bytes[2],
               ip_config->ip.v4.netmask.bytes[3]);
        printf("\t Gateway : %u.%u.%u.%u\r\n",
               ip_config->ip.v4.gateway.bytes[0],
               ip_config->ip.v4.gateway.bytes[1],
               ip_config->ip.v4.gateway.bytes[2],
               ip_config->ip.v4.gateway.bytes[3]);
      } else if (ip_config->type == SL_IPV6) {
        printf("Link Local Address: ");
        print_sl_ipv6_address(&ip_config->ip.v6.link_local_address);
        printf("Global Address: ");
        print_sl_ipv6_address(&ip_config->ip.v6.global_address);
        printf("Gateway Address: ");
        print_sl_ipv6_address(&ip_config->ip.v6.link_local_address);
      }

      break;
    }
    default:
      break;
  }

  return SL_STATUS_OK;
}