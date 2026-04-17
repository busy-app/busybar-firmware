/*******************************************************************************
 * @file  network_commands.c
 * @brief Network AT Commands
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

#include "sl_board_configuration.h"
#include "console.h"
#include "sl_net.h"
#include "sl_net_dns.h"
#include "sl_utility.h"
#include "sl_wifi.h"
#include "sl_net_default_values.h"
#include "sl_si91x_driver.h"
#include <string.h>
#include "sl_string.h"
#include "at_utility.h"
#include "sli_wifi_utility.h"

/******************************************************
 *                    Constants
 ******************************************************/

#ifdef SLI_SI91X_LWIP_NETWORK_INTERFACE
#include "sl_net_ethernet_types.h"
extern sl_net_ethernet_api_t ethernet_interface;

sl_ethernet_context_t ethernet_context;
const sl_ethernet_config_t ethernet_config = {
  .mac_address       = { { 0xD6, 0x15, 0x2C, 0x98, 0x1E, 0x29 } },
  .phy_bus_mode      = SL_ETHERNET_PHY_BUS_MODE_RMII,
  .duplex_mode       = SL_ETHERNET_DUPLEX_MODE_FULL,
  .speed             = SL_ETHERNET_SPEED_100_MBPS,
  .phy_address       = 0xFFu,
  .pin_configuration = &ethernet_pin_configuration,
};
#endif

#ifdef SL_WIFI_COMPONENT_INCLUDED
#include "sl_net_wifi_types.h"
#include "sl_wifi_callback_framework.h"

#ifdef SLI_SI91X_LWIP_HOSTED_NETWORK_STACK
#include "sl_net_for_lwip.h"
static sl_net_wifi_lwip_context_t wifi_client_context;
static sl_net_wifi_lwip_context_t wifi_ap_context;
#elif SLI_SI91X_OFFLOAD_NETWORK_STACK
#include "sl_net_si91x.h"
#include "sl_wifi_callback_framework.h"
#include "sl_net_ping.h"

extern char *strtok_r(char *, const char *, char **);
extern void wifi_set_default_configuration(void);
static sl_status_t convert_string_to_sl_ipv6_address(char *line, uint16_t *ipv6);

sl_status_t ping_response_callback_handler(sl_net_event_t event, sl_status_t status, void *data, uint32_t user_data);
sl_status_t ota_fw_up_response_handler(sl_net_event_t event, sl_status_t status, void *data, uint32_t user_data);
static sl_status_t net_event_response_handler(sl_net_event_t event, sl_status_t status, void *data, uint32_t user_data);
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

  sl_mac_address_t *mac_address = (sl_mac_address_t *)data;
  if (mac_address == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  AT_PRINTF("at+WIFI_AP_CLIENT_CONNECTED=");
  AT_PRINTF("%2X:%2X:%2X:%2X:%2X:%2X",
            mac_address->octet[0],
            mac_address->octet[1],
            mac_address->octet[2],
            mac_address->octet[3],
            mac_address->octet[4],
            mac_address->octet[5]);
  AT_PRINTF("\r\n>\r\n");

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

  sl_mac_address_t *mac_address = (sl_mac_address_t *)data;
  if (mac_address == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  AT_PRINTF("at+WIFI_AP_CLIENT_DISCONNECTED=");
  AT_PRINTF("%2X:%2X:%2X:%2X:%2X:%2X",
            mac_address->octet[0],
            mac_address->octet[1],
            mac_address->octet[2],
            mac_address->octet[3],
            mac_address->octet[4],
            mac_address->octet[5]);
  AT_PRINTF("\r\n>\r\n");

  return SL_STATUS_OK;
}

#endif

#endif

#define MAX_WIFI_PROFILES MAX_WIFI_CLIENT_PROFILES
#define OTAF_DEFAULT_PORT 80

static sl_net_wifi_client_profile_t *wifi_client_profiles[MAX_WIFI_PROFILES] = { NULL };
static sl_net_ip_configuration_t ip_config                                   = { 0 };

extern sl_wifi_device_configuration_t si91x_init_configuration;

void initialize_wifi_profiles(sl_net_profile_id_t profile_id)
{
  if (profile_id <= MAX_WIFI_PROFILES) {
    wifi_client_profiles[profile_id] = (sl_net_wifi_client_profile_t *)malloc(sizeof(sl_net_wifi_client_profile_t));
    if (wifi_client_profiles[profile_id] != NULL) {
      memcpy(wifi_client_profiles[profile_id], &DEFAULT_WIFI_CLIENT_PROFILE, sizeof(sl_net_wifi_client_profile_t));
    }
  }
}

void free_wifi_profiles()
{
  for (int i = 0; i < MAX_WIFI_PROFILES; i++) {
    if (wifi_client_profiles[i] != NULL) {
      free(wifi_client_profiles[i]);
      wifi_client_profiles[i] = NULL;
    }
  }
}

sl_net_wifi_client_profile_t *get_wifi_profile(sl_net_profile_id_t profile_id)
{
  if (profile_id >= MAX_WIFI_PROFILES) {
    return NULL; // Invalid profile ID
  }
  if (wifi_client_profiles[profile_id] == NULL) {
    initialize_wifi_profiles(profile_id);
  }
  return wifi_client_profiles[profile_id];
}

// at+net-init?
sl_status_t net_init_check_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%d\r\n", sl_si91x_is_device_initialized());
  return SL_STATUS_OK;
}

// at+net-init=<interface>
sl_status_t net_init_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);
  sl_status_t status;
  sl_net_interface_t interface = (sl_net_interface_t)GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, sl_net_interface_t);
  sl_net_event_handler_t event_handler = NULL;
  const void *config                   = NULL;

  if (si91x_init_configuration.boot_option != 0) {
    config = &si91x_init_configuration;
  }

  switch (SL_NET_INTERFACE_TYPE(interface)) {
#ifdef SLI_SI91X_LWIP_NETWORK_INTERFACE
    case SL_NET_ETHERNET_INTERFACE:
      status = sl_net_init(SL_NET_ETHERNET_INTERFACE, &ethernet_config, &ethernet_context, 0);
      VERIFY_STATUS_AND_RETURN(status);
      break;
#endif

#ifdef SL_WIFI_COMPONENT_INCLUDED
    case SL_NET_WIFI_CLIENT_INTERFACE:
#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
      event_handler = net_event_response_handler;
      status        = sl_net_init(interface, config, NULL, event_handler);
#elif SLI_SI91X_LWIP_HOSTED_NETWORK_STACK
      status = sl_net_init(interface, config, &wifi_client_context, event_handler);
#endif
      VERIFY_STATUS_AND_RETURN(status);
      break;
    case SL_NET_WIFI_AP_INTERFACE:
#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
      status = sl_net_init(interface, (const void *)config, NULL, NULL);
#elif SLI_SI91X_LWIP_HOSTED_NETWORK_STACK
      status = sl_net_init(interface, (const void *)config, (void *)&wifi_ap_context, NULL);
#endif
      VERIFY_STATUS_AND_RETURN(status);
      sl_wifi_set_callback_v2(SL_WIFI_CLIENT_CONNECTED_EVENTS, ap_connected_event_handler, NULL);
      sl_wifi_set_callback_v2(SL_WIFI_CLIENT_DISCONNECTED_EVENTS, ap_disconnected_event_handler, NULL);
      break;
#endif
    default:
      return SL_STATUS_INVALID_PARAMETER;
  }

  PRINT_AT_CMD_SUCCESS;
  return status;
}

// at+net-deinit=<interface>
sl_status_t net_deinit_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);
  sl_net_interface_t interface = (sl_net_interface_t)GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, sl_net_interface_t);

  sl_status_t status = sl_net_deinit(interface);
  VERIFY_STATUS_AND_RETURN(status);

  wifi_set_default_configuration();

  PRINT_AT_CMD_SUCCESS;
  return status;
}

// at+net-up=<interface>,<profile-id>
sl_status_t net_up_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);

  sl_net_interface_t interface   = (sl_net_interface_t)GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, sl_net_interface_t);
  sl_net_profile_id_t profile_id = GET_OPTIONAL_COMMAND_ARG(arguments, 1, SL_NET_PROFILE_ID_0, sl_net_profile_id_t);

  sl_net_wifi_client_profile_t *profile = get_wifi_profile(profile_id);
  if (profile == NULL) {
    return SL_STATUS_INVALID_PARAMETER; // Invalid profile ID
  }

  sl_status_t status = sl_net_up(interface, profile_id);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return status;
}

// at+net-down=<interface>
sl_status_t net_down_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);
  sl_net_interface_t interface = (sl_net_interface_t)GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, sl_net_interface_t);

  sl_status_t status = sl_net_down(interface);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+net-cred-wifipsk=<credential-id>,<psk>
sl_status_t net_cred_wifipsk_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);
  sl_net_credential_id_t cred_id = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, sl_net_credential_id_t);
  const char *password           = GET_OPTIONAL_COMMAND_ARG(arguments, 1, NULL, const char *);

  if ((password == NULL) || (cred_id == 0))
    return SL_STATUS_INVALID_PARAMETER;

  sl_status_t status = sl_net_set_credential(cred_id, SL_NET_WIFI_PSK, password, strlen(password));
  VERIFY_STATUS_AND_RETURN(status);
  PRINT_AT_CMD_SUCCESS;

  return SL_STATUS_OK;
}

// at+net-sta-cred=<profile-id>,<ssid>,<credential-id>,<security>,<encryption>
sl_status_t net_sta_credential_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x07);
  sl_net_profile_id_t profile_id = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_NET_PROFILE_ID_0, sl_net_profile_id_t);
  char *ssid                     = GET_OPTIONAL_COMMAND_ARG(arguments, 1, NULL, char *);
  sl_net_wifi_client_profile_t *profile = get_wifi_profile(profile_id);

  if (profile == NULL) {
    return SL_STATUS_INVALID_PARAMETER; // Invalid profile ID
  }

  profile->config.credential_id =
    GET_OPTIONAL_COMMAND_ARG(arguments, 2, SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID, sl_net_credential_id_t);
  profile->config.security   = GET_OPTIONAL_COMMAND_ARG(arguments, 3, SL_WIFI_WPA2, sl_wifi_security_t);
  profile->config.encryption = GET_OPTIONAL_COMMAND_ARG(arguments, 4, SL_WIFI_DEFAULT_ENCRYPTION, sl_wifi_encryption_t);

  if (ssid == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  profile->config.ssid.length = sl_strlen(ssid);
  memcpy(profile->config.ssid.value, ssid, profile->config.ssid.length);
  profile->config.ssid.value[profile->config.ssid.length] = 0;

  sl_status_t status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, profile_id, profile);
  VERIFY_STATUS_AND_RETURN(status);
  PRINT_AT_CMD_SUCCESS;

  return SL_STATUS_OK;
}

// at+net-sta-chan=<profile-id>,<channel>,<band>,<bandwidth>
sl_status_t net_sta_channel_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);
  sl_net_profile_id_t profile_id = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_NET_PROFILE_ID_0, sl_net_profile_id_t);
  sl_net_wifi_client_profile_t *profile = get_wifi_profile(profile_id);

  if (profile == NULL) {
    return SL_STATUS_INVALID_PARAMETER; // Invalid profile ID
  }

  profile->config.channel.channel   = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint16_t);
  profile->config.channel.band      = GET_OPTIONAL_COMMAND_ARG(arguments, 2, SL_WIFI_AUTO_BAND, uint8_t);
  profile->config.channel.bandwidth = GET_OPTIONAL_COMMAND_ARG(arguments, 3, SL_WIFI_AUTO_BANDWIDTH, uint8_t);

  sl_status_t status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, profile_id, profile);
  VERIFY_STATUS_AND_RETURN(status);
  PRINT_AT_CMD_SUCCESS;

  return SL_STATUS_OK;
}

// at+net-sta-bss=<profile-id>,<bssid>,<bss-type>
sl_status_t net_sta_bss_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);
  sl_net_profile_id_t profile_id = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_NET_PROFILE_ID_0, sl_net_profile_id_t);
  sl_net_wifi_client_profile_t *profile = get_wifi_profile(profile_id);

  if (profile == NULL) {
    return SL_STATUS_INVALID_PARAMETER; // Invalid profile ID
  }

  char *bssid              = GET_OPTIONAL_COMMAND_ARG(arguments, 1, NULL, char *);
  profile->config.bss_type = GET_OPTIONAL_COMMAND_ARG(arguments, 2, SL_WIFI_BSS_TYPE_INFRASTRUCTURE, uint16_t);

  memset(&profile->config.bssid, 0, sizeof(sl_mac_address_t));
  if (bssid != NULL) {
    convert_string_to_mac_address(bssid, &profile->config.bssid);
  }

  sl_status_t status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, profile_id, profile);
  VERIFY_STATUS_AND_RETURN(status);

  status = sl_wifi_set_join_configuration(SL_WIFI_CLIENT_INTERFACE,
                                          SL_WIFI_JOIN_FEAT_LISTEN_INTERVAL_VALID | SL_WIFI_JOIN_FEAT_BSSID_BASED);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  PRINT_AT_CMD_SUCCESS;

  return SL_STATUS_OK;
}

// at+net-sta-opt=<profile-id>,<client-options>,<channel-bitmap-2-4>,
//                <channel-bitmap-5>
sl_status_t net_sta_options_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);
  sl_net_profile_id_t profile_id = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_NET_PROFILE_ID_0, sl_net_profile_id_t);
  sl_net_wifi_client_profile_t *profile = get_wifi_profile(profile_id);

  if (profile == NULL) {
    return SL_STATUS_INVALID_PARAMETER; // Invalid profile ID
  }

  profile->config.client_options                    = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint8_t);
  profile->config.channel_bitmap.channel_bitmap_2_4 = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint16_t);
  profile->config.channel_bitmap.channel_bitmap_5   = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 0, uint32_t);
  profile->priority                                 = GET_OPTIONAL_COMMAND_ARG(arguments, 4, 0, uint32_t);

  sl_status_t status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, profile_id, profile);
  VERIFY_STATUS_AND_RETURN(status);
  PRINT_AT_CMD_SUCCESS;

  return SL_STATUS_OK;
}

// at+net-sta-ip=<profile-id>,<ip-mode>,<ip-type>,<host-name>,<station-ip>,
//               <ipv6-global-ip>,<gateway-ip>,<ipv4-netmask>
sl_status_t net_sta_ip_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x07);
  sl_net_profile_id_t profile_id = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_NET_PROFILE_ID_0, sl_net_profile_id_t);
  sl_net_wifi_client_profile_t *profile = get_wifi_profile(profile_id);

  if (profile == NULL) {
    return SL_STATUS_INVALID_PARAMETER; // Invalid profile ID
  }

  profile->ip.mode      = GET_OPTIONAL_COMMAND_ARG(arguments, 1, SL_IP_MANAGEMENT_DHCP, uint8_t);
  profile->ip.type      = GET_OPTIONAL_COMMAND_ARG(arguments, 2, SL_IPV4, uint8_t);
  profile->ip.host_name = GET_OPTIONAL_COMMAND_ARG(arguments, 3, NULL, char *);

  if (profile->ip.mode == SL_IP_MANAGEMENT_STATIC_IP) {
    if (profile->ip.type == SL_IPV4) {
      char *sta_ip  = GET_OPTIONAL_COMMAND_ARG(arguments, 4, NULL, char *);
      char *gateway = GET_OPTIONAL_COMMAND_ARG(arguments, 6, NULL, char *);
      char *netmask = GET_OPTIONAL_COMMAND_ARG(arguments, 7, NULL, char *);

      if ((NULL == sta_ip) && (NULL == gateway) && (NULL == netmask))
        return SL_STATUS_INVALID_PARAMETER;

      convert_string_to_sl_ipv4_address(sta_ip, &profile->ip.ip.v4.ip_address);
      convert_string_to_sl_ipv4_address(gateway, &profile->ip.ip.v4.gateway);
      convert_string_to_sl_ipv4_address(netmask, &profile->ip.ip.v4.netmask);
    } else if (profile->ip.type == SL_IPV6) {
      char *v6_global_ip = GET_OPTIONAL_COMMAND_ARG(arguments, 5, NULL, char *);

      if (v6_global_ip == NULL) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      convert_string_to_sl_ipv6_address(v6_global_ip, (uint16_t *)profile->ip.ip.v6.global_address.bytes);
    }
  }

  sl_status_t status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, profile_id, profile);
  VERIFY_STATUS_AND_RETURN(status);
  PRINT_AT_CMD_SUCCESS;

  return SL_STATUS_OK;
}

// at+net-dhcp-conf=<min-discover-retry-interval>,<max-discover-retry-interval>,
//                  <min-request-retry-interval>,<max-request-retry-interval>,
//                  <max-discover-retries>,<max-request-retries>
sl_status_t net_dhcp_config_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x3f);
  ip_config.dhcp_config.min_discover_retry_interval = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 5, uint16_t);
  ip_config.dhcp_config.max_discover_retry_interval = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 100, uint16_t);
  ip_config.dhcp_config.min_request_retry_interval  = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 5, uint16_t);
  ip_config.dhcp_config.max_request_retry_interval  = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 10, uint16_t);
  ip_config.dhcp_config.min_discover_retries        = GET_OPTIONAL_COMMAND_ARG(arguments, 4, 5, uint8_t);
  ip_config.dhcp_config.max_request_retries         = GET_OPTIONAL_COMMAND_ARG(arguments, 5, 10, uint8_t);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+net-conf-ip=<interface>,<mode>,<type>,<host-name>,<ipv4-address>,
//                <ipv4-gateway>,<ipv4-netmask>,<ipv6-link-local-address>,
//                <ipv6-global-address>,<ipv6-gateway>,<timeout>
sl_status_t net_configure_ip_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x07);
  sl_net_interface_t interface =
    (sl_net_interface_t)GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_NET_WIFI_CLIENT_INTERFACE, sl_net_interface_t);
  uint32_t timeout = GET_OPTIONAL_COMMAND_ARG(arguments, 10, SL_NET_WAIT_FOREVER, uint32_t);

  ip_config.mode      = GET_OPTIONAL_COMMAND_ARG(arguments, 1, SL_IP_MANAGEMENT_DHCP, uint8_t);
  ip_config.type      = GET_OPTIONAL_COMMAND_ARG(arguments, 2, SL_IPV4, uint8_t);
  ip_config.host_name = GET_OPTIONAL_COMMAND_ARG(arguments, 3, NULL, char *);

  if (ip_config.mode == SL_IP_MANAGEMENT_STATIC_IP) {
    if (ip_config.type == SL_IPV4) {
      char *sta_ip  = GET_OPTIONAL_COMMAND_ARG(arguments, 4, NULL, char *);
      char *gateway = GET_OPTIONAL_COMMAND_ARG(arguments, 5, NULL, char *);
      char *netmask = GET_OPTIONAL_COMMAND_ARG(arguments, 6, NULL, char *);

      if ((NULL == sta_ip) && (NULL == gateway) && (NULL == netmask))
        return SL_STATUS_INVALID_PARAMETER;

      convert_string_to_sl_ipv4_address(sta_ip, &ip_config.ip.v4.ip_address);
      convert_string_to_sl_ipv4_address(gateway, &ip_config.ip.v4.gateway);
      convert_string_to_sl_ipv4_address(netmask, &ip_config.ip.v4.netmask);
    } else if (ip_config.type == SL_IPV6) {
      char *v6_link_local = GET_OPTIONAL_COMMAND_ARG(arguments, 7, NULL, char *);
      char *v6_global_ip  = GET_OPTIONAL_COMMAND_ARG(arguments, 8, NULL, char *);
      char *v6_gateway    = GET_OPTIONAL_COMMAND_ARG(arguments, 9, NULL, char *);

      if ((v6_link_local == NULL) && (v6_global_ip == NULL) && (v6_gateway == NULL)) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      convert_string_to_sl_ipv6_address(v6_link_local, (uint16_t *)ip_config.ip.v6.link_local_address.bytes);
      convert_string_to_sl_ipv6_address(v6_global_ip, (uint16_t *)ip_config.ip.v6.global_address.bytes);
      convert_string_to_sl_ipv6_address(v6_gateway, (uint16_t *)ip_config.ip.v6.gateway.bytes);
    }
  }

  sl_status_t status = sl_net_configure_ip(interface, &ip_config, timeout);
  VERIFY_STATUS_AND_RETURN(status);
  PRINT_AT_CMD_SUCCESS;

  return SL_STATUS_OK;
}

static sl_status_t convert_string_to_sl_ipv6_address(char *line, uint16_t *ipv6)
{
  uint8_t double_colon = 0xFF;
  uint8_t i;
  char *lasts = NULL;
  char *token = strtok_r(line, ":", &lasts);

  for (i = 0; token != NULL; i++, token = strtok_r(NULL, ":", &lasts)) {
    ipv6[i] = (uint16_t)strtoul(token, 0, 16);
    if (lasts[0] == ':') {
      //      ipv6[i] = 0;
      // Skip the ':'
      ++lasts;
      double_colon = i;
    }
  }
  // Check if we need to move the data after the double-colon to the end
  if (i != 8 && double_colon != 0xFF) {
    --i;
    uint8_t end = 7;
    for (; i != double_colon; --i, --end) {
      ipv6[end] = ipv6[i];
    }
    for (; end > double_colon; --end) {
      ipv6[end] = 0;
    }
  }
  return SL_STATUS_OK;
}

// ?
sl_status_t dns_hostgetbyname_command_handler(console_args_t *arguments)
{
#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
  sl_ip_address_t ip_address = { 0 };
  sl_status_t status;

  const char *host_name  = (const char *)arguments->arg[0];
  const uint32_t timeout = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 20000, const uint32_t);
  sl_net_dns_resolution_ip_type_t ip_type =
    GET_OPTIONAL_COMMAND_ARG(arguments, 2, SL_NET_DNS_TYPE_IPV4, sl_net_dns_resolution_ip_type_t);

  status = sl_net_dns_resolve_hostname(host_name, timeout, ip_type, &ip_address);
  if (status == SL_STATUS_IN_PROGRESS) {
    return SL_STATUS_OK;
  }
  print_sl_ip_address(&ip_address);

  VERIFY_STATUS_AND_RETURN(status);
  return status;
#else
  return SL_STATUS_NOT_SUPPORTED;
#endif
}

#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
sl_status_t ping_response_callback_handler(sl_net_event_t event, sl_status_t status, void *data, uint32_t user_data)
{
  UNUSED_PARAMETER(user_data);
  sl_net_ping_response_t *response = (sl_net_ping_response_t *)data;

  if (SL_NET_PING_RESPONSE_EVENT == event) {
    if (status != SL_STATUS_OK) {
      printf("\n Ping request failed! \n");
      return status;
    } else {
      printf(" Ping reply received from %u.%u.%u.%u \n\n",
             response->ping_address.ipv4_address[0],
             response->ping_address.ipv4_address[1],
             response->ping_address.ipv4_address[2],
             response->ping_address.ipv4_address[3]);
    }
  }

  return status;
}

SL_WEAK sl_status_t ota_fw_up_response_handler(sl_net_event_t event, sl_status_t status, void *data, uint32_t user_data)
{
  UNUSED_PARAMETER(event);
  UNUSED_PARAMETER(data);
  UNUSED_PARAMETER(user_data);

  return status;
}

sl_status_t net_event_response_handler(sl_net_event_t event, sl_status_t status, void *data, uint32_t user_data)
{
  if (SL_NET_PING_RESPONSE_EVENT == event) {
    return ping_response_callback_handler(event, status, data, user_data);
  } else if (SL_NET_OTA_FW_UPDATE_EVENT == event) {
    return ota_fw_up_response_handler(event, status, data, user_data);
  } else {
    // MISRA
  }

  return status;
}

// ?
sl_status_t sl_net_ping_command_handler(console_args_t *arguments)
{
  sl_status_t status         = SL_STATUS_OK;
  sl_ip_address_t ip_address = { .type = SL_IPV4 };

  sl_net_inet_addr((char *)arguments->arg[0], (uint32_t *)&ip_address.ip.v4);
  ip_address.type    = (sl_ip_address_type_t)GET_OPTIONAL_COMMAND_ARG(arguments, 1, SL_IPV4, sl_ip_address_type_t);
  uint16_t ping_size = (uint16_t)arguments->arg[2];

  status = sl_si91x_send_ping(ip_address, ping_size);

  if (SL_STATUS_IN_PROGRESS == status) {
    status = SL_STATUS_OK;
  }

  return status;
}

// at+http-otaf=<flags>,<server_ip_addr>,<port>,<resource_url>,<host_name>,
//              <extended_header>,<user_name>,<password>
sl_status_t http_otaf_upgrade_command_handler(const console_args_t *arguments)
{

  sl_status_t status                      = SL_STATUS_OK;
  sl_si91x_http_otaf_params_t http_params = {
    .flags           = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint16_t),
    .ip_address      = (uint8_t *)GET_OPTIONAL_COMMAND_ARG(arguments, 1, NULL, char *),
    .port            = GET_OPTIONAL_COMMAND_ARG(arguments, 2, OTAF_DEFAULT_PORT, uint16_t),
    .resource        = (uint8_t *)GET_OPTIONAL_COMMAND_ARG(arguments, 3, NULL, char *),
    .host_name       = (uint8_t *)GET_OPTIONAL_COMMAND_ARG(arguments, 4, NULL, char *),
    .extended_header = (uint8_t *)GET_OPTIONAL_COMMAND_ARG(arguments, 5, NULL, char *),
    .user_name       = (uint8_t *)GET_OPTIONAL_COMMAND_ARG(arguments, 6, NULL, char *),
    .password        = (uint8_t *)GET_OPTIONAL_COMMAND_ARG(arguments, 7, NULL, char *),
  };

  status = sl_si91x_http_otaf_v2(&http_params);
  VERIFY_STATUS_AND_RETURN(status);
  PRINT_AT_CMD_SUCCESS;

  return status;
}

#elif SLI_SI91X_LWIP_HOSTED_NETWORK_STACK
sl_status_t sl_net_ping_command_handler(console_args_t *arguments)
{
  return SL_STATUS_NOT_SUPPORTED;
}
#endif
