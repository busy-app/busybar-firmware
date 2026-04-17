/***************************************************************************/ /**
 * @file
 * @brief Mdns Disovery Example Application
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
#include "errno.h"
#include "netdb.h"
#include "sl_wifi_callback_framework.h"
#include "sl_status.h"
#include "cmsis_os2.h"
#include "sl_wifi.h"
#include "sl_utility.h"
#include "sl_net.h"
#include "sl_net_si91x.h"
#include "sl_net_wifi_types.h"
#include <string.h>
#include <stdint.h>
#include "sl_mdns.h"

/******************************************************
 *                    Constants
 ******************************************************/
#define MDNSV6_ENABLE 0

#if MDNSV6_ENABLE
#define SERVER_IP  "2401:4901:1290:10de::1004"
#define IP_VERSION SL_IPV6_VERSION
#else
#define SERVER_IP  "192.168.50.131"
#define IP_VERSION SL_IPV4_VERSION
#endif

#define CLIENT_MESSAGE "WIFISDK"

#define MDNS_HOST_NAME       "wiseconnect.local."
#define MDNS_SERVICE_TYPE    "_http._tcp.local."
#define MDNS_SERVICE_NAME    "wiseconnect._http._tcp.local"
#define MDNS_SERVICE_MESSAGE "HTTP service"
#define MDNS_SERVICE_PORT    80
#define MDNS_SERVICE_TTL     300

volatile bool mdns_discovery_done = false;

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

sl_ip_address_t ip_address           = { 0 };
sl_net_wifi_client_profile_t profile = { 0 };
/******************************************************
 *               Function Declarations
 ******************************************************/
static void application_start(void *argument);
static sl_status_t mdns_event_handler(sl_net_event_t event, sl_status_t status, void *data, uint32_t data_length);

/******************************************************
 *               Function Definitions
 ******************************************************/

static sl_status_t mdns_event_handler(sl_net_event_t event, sl_status_t status, void *data, uint32_t data_length)
{
  UNUSED_PARAMETER(data_length);

  if (event == SL_NET_MDNS_STOP_EVENT) {
    printf("\r\nDiscovery operation timed out.\r\n");
    mdns_discovery_done = true;
    return SL_STATUS_OK;
  }

  if (event != SL_NET_MDNS_EVENT) {
    return SL_STATUS_OK;
  }

  printf("\r\nReceived mDNS Discovery Event with status: 0x%lX\r\n", status);
  if (status != SL_STATUS_OK) {
    printf("mDNS Discovery failed!\r\n");
    return status;
  }

  sl_mdns_response_t *result = (sl_mdns_response_t *)data;

  switch ((sl_mdns_query_type_t)result->query_type) {
    case SL_MDNS_QUERY_TYPE_PTR:
      printf("\r\n=== PTR Record ===\r\n");
      if (result->instance_name)
        printf("Instance Name : %s\r\n", result->instance_name);
      if (result->service_type)
        printf("Service Type  : %s\r\n", result->service_type);
      if (result->proto)
        printf("Protocol      : %s\r\n", result->proto);
      printf("TTL           : %lu\r\n", result->ttl);
      break;

    case SL_MDNS_QUERY_TYPE_SRV:
      printf("\r\n=== SRV Record ===\r\n");
      if (result->instance_name)
        printf("Service Name  : %s\r\n", result->instance_name);
      if (result->hostname)
        printf("Target Host   : %s\r\n", result->hostname);
      printf("Port          : %u\r\n", result->port);
      printf("TTL           : %lu\r\n", result->ttl);
      break;

    case SL_MDNS_QUERY_TYPE_A:
      printf("\r\n=== A Record ===\r\n");
      if (result->hostname)
        printf("Hostname      : %s\r\n", result->hostname);
      for (size_t i = 0; i < result->addr.addr_count; i++) {
        sl_ip_address_t *ip = &result->addr.addr[i];
        if (ip->type == SL_IPV4) {
          printf("IPv4 Address  : ");
          print_sl_ip_address(ip);
        }
      }
      printf("TTL           : %lu\r\n", result->ttl);
      break;

    case SL_MDNS_QUERY_TYPE_AAAA:
      printf("\r\n=== AAAA Record ===\r\n");
      if (result->hostname)
        printf("Hostname      : %s\r\n", result->hostname);
      for (size_t i = 0; i < result->addr.addr_count; i++) {
        sl_ip_address_t *ip = &result->addr.addr[i];
        if (ip->type == SL_IPV6) {
          printf("IPv6 Address  : ");
          print_sl_ip_address(ip);
        }
      }
      printf("TTL           : %lu\r\n", result->ttl);
      break;

    case SL_MDNS_QUERY_TYPE_TXT:
      printf("\r\n=== TXT Record ===\r\n");
      if (result->instance_name)
        printf("Service Name  : %s\r\n", result->instance_name);
      for (size_t i = 0; i < result->txt.txt_count; i++) {
        if (result->txt.txt[i]) {
          printf("TXT Record    : %s\r\n", result->txt.txt[i]);
        }
      }
      printf("TTL           : %lu\r\n", result->ttl);
      break;

    default:
      printf("Unknown query type: %u\r\n", result->query_type);
      break;
  }

  // Clean up
  if (result->txt.txt) {
    free(result->txt.txt);
  }

  if (result->addr.addr) {
    free(result->addr.addr);
  }
  return SL_STATUS_OK;
}

void app_init(const void *unused)
{
  UNUSED_PARAMETER(unused);
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status             = SL_STATUS_FAIL;
  sl_mdns_t mdns                 = { 0 };
  sl_mdns_configuration_t config = { .protocol = SL_MDNS_PROTO_UDP, .type = IP_VERSION, .host_name = MDNS_HOST_NAME };
  sl_mdns_service_t service      = { .instance_name   = MDNS_SERVICE_NAME,
                                     .service_type    = MDNS_SERVICE_TYPE,
                                     .service_message = MDNS_SERVICE_MESSAGE,
                                     .port            = MDNS_SERVICE_PORT,
                                     .ttl             = MDNS_SERVICE_TTL };

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_default_client_configuration, NULL, mdns_event_handler);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to start Wi-Fi Client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi Init Success \r\n");

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client connected\r\n");

  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to get client profile: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nClient profile is fetched successfully.\r\n");

  if (profile.ip.type == SL_IPV4) {
    ip_address.type = SL_IPV4;
    memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
    print_sl_ip_address(&ip_address);
  } else if (profile.ip.type == SL_IPV6) {
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
  }

  // Initialize MDNS service
  status = sl_mdns_init(&mdns, (const sl_mdns_configuration_t *)&config, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to initialize MDNS : 0x%lx\r\n", status);
    return;
  }
  printf("\r\nMDNS initialized\r\n");

  // Add an interface to MDNS Instance
  status = sl_mdns_add_interface(&mdns, SL_NET_WIFI_CLIENT_INTERFACE);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to add interface to MDNS : 0x%lx\r\n", status);
    return;
  }
  printf("\r\nInterface Added to MDNS\r\n");

  // Add a service to MDNS Instance
  status = sl_mdns_register_service(&mdns, SL_NET_WIFI_CLIENT_INTERFACE, &service);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to register service to MDNS: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nService Added to MDNS\r\n");

  sl_mdns_service_query_t service_query = { .service_type = "_http._tcp.local.", // Query for HTTP services
                                            .query_type   = SL_MDNS_QUERY_TYPE_PTR,
                                            .timeout      = 5000 };

  status = sl_mdns_service_discovery_start(&mdns, SL_NET_WIFI_CLIENT_INTERFACE, &service_query);
  if (status != SL_STATUS_IN_PROGRESS) {
    printf("\r\nFailed to send mDNS service discovery request: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nmDNS Service Discovery request sent successfully\r\n");

  while (!mdns_discovery_done) {
    osDelay(100); // Wait in 100ms intervals
  }
  // Deinitialize MDNS service
  status = sl_mdns_deinit(&mdns);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to deinitialize MDNS : 0x%lx\r\n", status);
    return;
  }
  printf("\r\nDeinitialize MDNS successfully\r\n");
}
