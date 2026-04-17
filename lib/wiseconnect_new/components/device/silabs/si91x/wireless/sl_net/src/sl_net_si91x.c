/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_status.h"
#include "sl_wifi_types.h"
#include "sl_net.h"
#include "stddef.h"
#include "sl_status.h"
#include "sl_utility.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_net_wifi_types.h"
#include "sl_net_si91x.h"
#include "sl_si91x_host_interface.h"
#include "sl_si91x_driver.h"
#include "sl_rsi_utility.h"
#include "sli_net_utility.h"
#include "sl_si91x_core_utilities.h"
#include "sli_net_common_utility.h"
#include "sli_net_constants.h"
#include <stdbool.h>
#include <string.h>
#include "sl_wifi_callback_framework.h"
#include "sl_net_dns.h"
#include "sli_wifi_constants.h"
#include "sli_wifi_utility.h"

typedef enum { SLI_SI91X_CLIENT = 0, SLI_SI91X_AP = 1, SLI_SI91X_MAX_INTERFACES = 2 } sli_si91x_interfaces_t;
sl_status_t sl_net_dns_resolve_hostname(const char *host_name,
                                        const uint32_t timeout,
                                        const sl_net_dns_resolution_ip_type_t dns_resolution_ip,
                                        sl_ip_address_t *sl_ip_address);
static bool sli_si91x_get_dns_mode(const sl_net_dns_address_t *address);
extern bool device_initialized;
extern osMessageQueueId_t sli_network_manager_request_queue;
extern osEventFlagsId_t sli_network_manager_response_flags;
static sl_status_t sli_si91x_send_multicast_request(sl_wifi_interface_t interface,
                                                    const sl_ip_address_t *ip_address,
                                                    uint8_t command_type);
static sl_wifi_interface_t nmap_ap_interface_with_band[MAX_NET_AP_INTERFACES]         = { 0 };
static sl_wifi_interface_t nmap_client_interface_with_band[MAX_NET_CLIENT_INTERFACES] = { 0 };
static sl_ip_management_t dhcp_type[SLI_SI91X_MAX_INTERFACES]                         = { 0 };

sl_status_t sl_net_wifi_client_init(sl_net_interface_t interface,
                                    const void *configuration,
                                    void *context,
                                    sl_net_event_handler_t event_handler)
{
  UNUSED_PARAMETER(interface);
  UNUSED_PARAMETER(context);
  sl_status_t status = SL_STATUS_FAIL;

  // Set the user-defined event handler for client mode
  sli_net_register_event_handler(event_handler);

  status = sl_wifi_init(configuration, NULL, sl_wifi_default_event_handler);
  // Verify the initialization status and return it
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_net_wifi_client_deinit(sl_net_interface_t interface)
{
  UNUSED_PARAMETER(interface);
  return sl_wifi_deinit();
}

sl_status_t sl_net_wifi_client_up(sl_net_interface_t interface, sl_net_profile_id_t profile_id)
{
  //Check whether the Interfaces are valid
  if (interface != SL_NET_WIFI_CLIENT_1_INTERFACE && interface != SL_NET_WIFI_CLIENT_2_INTERFACE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status;
  sl_net_wifi_client_profile_t profile;
  sl_wifi_interface_t client_interface = 0;

  // Connect to the Wi-Fi network
  if (profile_id == SL_NET_AUTO_JOIN) {
    return sli_network_manager_auto_join_request(interface, profile_id);
  }

  // Get the client profile using the provided profile_id
  status = sl_net_get_profile(interface, profile_id, &profile);
  VERIFY_STATUS_AND_RETURN(status);

  // Configure the client interface based on the band
  if (profile.config.channel.band == SL_WIFI_BAND_2_4GHZ || profile.config.channel.band == SL_WIFI_AUTO_BAND) {
    client_interface = SL_WIFI_CLIENT_2_4GHZ_INTERFACE;
  } else if (profile.config.channel.band == SL_WIFI_BAND_5GHZ) {
    client_interface = SL_WIFI_CLIENT_5GHZ_INTERFACE;
  } else if (profile.config.channel.band == SL_WIFI_BAND_DUAL) {
    client_interface = SL_WIFI_CLIENT_DUAL_INTERFACE;
  }

  // Connect to the Wi-Fi network
  status = sl_wifi_connect(client_interface, &profile.config, SLI_WIFI_CONNECT_TIMEOUT);
  VERIFY_STATUS_AND_RETURN(status);

  // Configure the IP address settings
  if (interface == SL_NET_WIFI_CLIENT_1_INTERFACE) {
    nmap_client_interface_with_band[0] = client_interface; //Map the Interface with the band
    status                             = sl_si91x_configure_ip_address(&profile.ip, SL_WIFI_CLIENT_VAP_ID);
  } else if (interface == SL_NET_WIFI_CLIENT_2_INTERFACE) {
    nmap_client_interface_with_band[1] = client_interface; //Map the Interface with the band
    status                             = sl_si91x_configure_ip_address(&profile.ip, SL_WIFI_CLIENT_VAP_ID_1);
  }

  // Disconnect WiFi on IP configuration failure
  if (status != SL_STATUS_OK) {
    sl_wifi_disconnect(client_interface);
    return status;
  }

  dhcp_type[SLI_SI91X_CLIENT] = profile.ip.mode;

  // Set the client profile
  status = sl_net_set_profile(interface, profile_id, &profile);
  return status;
}

sl_status_t sl_net_wifi_client_down(sl_net_interface_t interface)
{
  //Check whether the Interfaces are valid
  if (interface != SL_NET_WIFI_CLIENT_1_INTERFACE && interface != SL_NET_WIFI_CLIENT_2_INTERFACE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_wifi_interface_t client_interface = (interface == SL_NET_WIFI_CLIENT_1_INTERFACE)
                                           ? nmap_client_interface_with_band[0]
                                           : nmap_client_interface_with_band[1];

  // Disconnect from the Wi-Fi network
  return sl_wifi_disconnect(client_interface);
}

sl_status_t sl_net_wifi_ap_init(sl_net_interface_t interface,
                                const void *configuration,
                                const void *workspace,
                                sl_net_event_handler_t event_handler)
{
  UNUSED_PARAMETER(interface);
  UNUSED_PARAMETER(workspace);
  sl_status_t status = SL_STATUS_FAIL;

  // Set the user-defined event handler for AP mode
  sli_net_register_event_handler(event_handler);

  status = sl_wifi_init(configuration, NULL, sl_wifi_default_event_handler);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_net_wifi_ap_deinit(sl_net_interface_t interface)
{
  UNUSED_PARAMETER(interface);
  return sl_wifi_deinit();
}

sl_status_t sl_net_wifi_ap_up(sl_net_interface_t interface, sl_net_profile_id_t profile_id)
{
  //Check whether the Interfaces are valid
  if (interface != SL_NET_WIFI_AP_1_INTERFACE && interface != SL_NET_WIFI_AP_2_INTERFACE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sl_status_t status;
  sl_net_wifi_ap_profile_t profile;
  sl_wifi_interface_t ap_interface = 0;

  status = sl_net_get_profile(interface, profile_id, &profile);
  VERIFY_STATUS_AND_RETURN(status);

  // Validate if profile configuration is valid
  // AP + DHCP client not supported
  // AP + link local not supported
  if (profile.ip.mode != SL_IP_MANAGEMENT_STATIC_IP) {
    return SL_STATUS_INVALID_CONFIGURATION;
  }
  if (interface == SL_NET_WIFI_AP_1_INTERFACE) {
    status = sl_si91x_configure_ip_address(&profile.ip, SL_WIFI_AP_VAP_ID);
    VERIFY_STATUS_AND_RETURN(status);
  } else if (interface == SL_NET_WIFI_AP_2_INTERFACE) {
    status = sl_si91x_configure_ip_address(&profile.ip, SL_WIFI_AP_VAP_ID_1);
    VERIFY_STATUS_AND_RETURN(status);
  }
  dhcp_type[SLI_SI91X_AP] = profile.ip.mode;

  // Set the AP profile
  status = sl_net_set_profile(interface, profile_id, &profile);
  VERIFY_STATUS_AND_RETURN(status);

  // Configure the ap_interface based on the band
  if (profile.config.channel.band == SL_WIFI_BAND_2_4GHZ || profile.config.channel.band == SL_WIFI_AUTO_BAND) {
    ap_interface = SL_WIFI_AP_2_4GHZ_INTERFACE;
  } else if (profile.config.channel.band == SL_WIFI_BAND_5GHZ) {
    ap_interface = SL_WIFI_AP_5GHZ_INTERFACE;
  } else if (profile.config.channel.band == SL_WIFI_BAND_DUAL) {
    ap_interface = SL_WIFI_AP_DUAL_INTERFACE;
  }
  // Map the AP Interface with band
  if (interface == SL_NET_WIFI_AP_1_INTERFACE) {
    nmap_ap_interface_with_band[0] = ap_interface;
  } else if (interface == SL_NET_WIFI_AP_2_INTERFACE) {
    nmap_ap_interface_with_band[1] = ap_interface;
  }

  status = sl_wifi_start_ap(ap_interface, &profile.config);
  VERIFY_STATUS_AND_RETURN(status);

  return status;
}

sl_status_t sl_net_wifi_ap_down(sl_net_interface_t interface)
{
  //Check whether the Interfaces are valid
  if (interface != SL_NET_WIFI_AP_1_INTERFACE && interface != SL_NET_WIFI_AP_2_INTERFACE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_wifi_interface_t ap_interface = (interface == SL_NET_WIFI_AP_1_INTERFACE) ? nmap_ap_interface_with_band[0]
                                                                               : nmap_ap_interface_with_band[1];

  return sl_wifi_stop_ap(ap_interface);
}

sl_status_t sl_net_join_multicast_address(sl_net_interface_t interface, const sl_ip_address_t *ip_address)
{
  return sli_si91x_send_multicast_request((sl_wifi_interface_t)interface, ip_address, SL_WIFI_MULTICAST_JOIN);
}

sl_status_t sl_net_leave_multicast_address(sl_net_interface_t interface, const sl_ip_address_t *ip_address)
{
  return sli_si91x_send_multicast_request((sl_wifi_interface_t)interface, ip_address, SL_WIFI_MULTICAST_LEAVE);
}

static sl_status_t sli_si91x_send_multicast_request(sl_wifi_interface_t interface,
                                                    const sl_ip_address_t *ip_address,
                                                    uint8_t command_type)
{
  UNUSED_PARAMETER(interface);
  sli_si91x_req_multicast_t multicast = { 0 };
  sl_status_t status                  = SL_STATUS_OK;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  //Fill IP version and IP address
  if (ip_address->type == SL_IPV6) {
    multicast.ip_version[0] = 6;
    memcpy(multicast.multicast_address.ipv6_address, ip_address->ip.v6.bytes, SLI_IP_ADDRESS_LEN * 4);
  } else {
    multicast.ip_version[0] = 4;
    memcpy(multicast.multicast_address.ipv4_address, ip_address->ip.v4.bytes, SLI_IP_ADDRESS_LEN);
  }
  multicast.type[0] = command_type;

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_MULTICAST,
                                         SLI_SI91X_NETWORK_CMD,
                                         &multicast,
                                         sizeof(multicast),
                                         SLI_WLAN_RSP_MULTICAST_WAIT_TIME,
                                         NULL,
                                         NULL);

  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

// Resolve a host name to an IP address using DNS
sl_status_t sl_net_dns_resolve_hostname(const char *host_name,
                                        const uint32_t timeout,
                                        const sl_net_dns_resolution_ip_type_t dns_resolution_ip,
                                        sl_ip_address_t *sl_ip_address)
{
  // Check for a NULL pointer for sl_ip_address
  SL_WIFI_ARGS_CHECK_NULL_POINTER(sl_ip_address);

  sl_status_t status;
  sl_wifi_system_packet_t *packet;
  sl_wifi_buffer_t *buffer                        = NULL;
  const sli_si91x_dns_response_t *dns_response    = { 0 };
  sli_si91x_dns_query_request_t dns_query_request = { 0 };

  // Determine the wait period based on the timeout value
  sli_wifi_wait_period_t wait_period = timeout == 0 ? SLI_WIFI_RETURN_IMMEDIATELY : SLI_WIFI_WAIT_FOR_RESPONSE(timeout);
  // Determine the IP version to be used (IPv4 or IPv6)
  dns_query_request.ip_version[0] = (dns_resolution_ip == SL_NET_DNS_TYPE_IPV4) ? 4 : 6;
  memcpy(dns_query_request.url_name, host_name, sizeof(dns_query_request.url_name));

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_DNS_QUERY,
                                         SLI_SI91X_NETWORK_CMD,
                                         &dns_query_request,
                                         sizeof(dns_query_request),
                                         wait_period,
                                         NULL,
                                         &buffer);

  // Check if the command failed and free the buffer if it was allocated
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);

  // Extract the DNS response from the SI91X packet buffer
  packet       = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  dns_response = (sli_si91x_dns_response_t *)packet->data;

  // Convert the SI91X DNS response to the sl_ip_address format
  sli_convert_si91x_dns_response(sl_ip_address, dns_response);
  sli_si91x_host_free_buffer(buffer);
  return SL_STATUS_OK;
}

sl_status_t sl_net_set_dns_server(sl_net_interface_t interface, const sl_net_dns_address_t *address)
{
  UNUSED_PARAMETER(interface);
  sl_status_t status                                  = 0;
  sli_dns_server_add_request_t dns_server_add_request = { 0 };

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  //! Check for invalid parameters
  if ((address->primary_server_address && address->primary_server_address->type != SL_IPV4
       && address->primary_server_address->type != SL_IPV6)
      || (address->secondary_server_address && address->secondary_server_address->type != SL_IPV4
          && address->secondary_server_address->type != SL_IPV6)) {
    //! Throw error in case of invalid parameters
    return SL_STATUS_INVALID_PARAMETER;
  }

  dns_server_add_request.dns_mode[0] = sli_si91x_get_dns_mode(address);

  if (address->primary_server_address && address->primary_server_address->type == SL_IPV4) {
    dns_server_add_request.ip_version[0] = SL_IPV4_VERSION;
    //! Fill Primary IP address
    memcpy(dns_server_add_request.sli_ip_address1.primary_dns_ipv4,
           address->primary_server_address->ip.v4.bytes,
           SL_IPV4_ADDRESS_LENGTH);
  } else if (address->primary_server_address && address->primary_server_address->type == SL_IPV6) {
    dns_server_add_request.ip_version[0] = SL_IPV6_VERSION;
    //! Fill Primary IP address
    memcpy(dns_server_add_request.sli_ip_address1.primary_dns_ipv6,
           address->primary_server_address->ip.v6.bytes,
           SL_IPV6_ADDRESS_LENGTH);
  }

  if (address->secondary_server_address && address->secondary_server_address->type == SL_IPV4) {
    dns_server_add_request.ip_version[0] = SL_IPV4_VERSION;
    //! Fill Secondary IP address
    memcpy(dns_server_add_request.sli_ip_address2.secondary_dns_ipv4,
           address->secondary_server_address->ip.v4.bytes,
           SL_IPV4_ADDRESS_LENGTH);
  } else if (address->secondary_server_address && address->secondary_server_address->type == SL_IPV6) {
    dns_server_add_request.ip_version[0] = SL_IPV6_VERSION;
    //! Fill Secondary IP address
    memcpy(dns_server_add_request.sli_ip_address2.secondary_dns_ipv6,
           address->secondary_server_address->ip.v6.bytes,
           SL_IPV6_ADDRESS_LENGTH);
  }

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_DNS_SERVER_ADD,
                                         SLI_SI91X_NETWORK_CMD,
                                         &dns_server_add_request,
                                         sizeof(dns_server_add_request),
                                         SLI_WLAN_RSP_DNS_SERVER_ADD_WAIT_TIME,
                                         NULL,
                                         NULL);

  return status;
}

sl_status_t sl_net_configure_ip(sl_net_interface_t interface,
                                const sl_net_ip_configuration_t *ip_config,
                                uint32_t timeout)
{
  uint8_t vap_id                   = 0;
  sl_net_ip_configuration_t config = { 0 };

  if (timeout == 0 || timeout & (1 << 30)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (SL_NET_WIFI_CLIENT_INTERFACE == SL_NET_INTERFACE_TYPE(interface)) {
    vap_id                      = SL_WIFI_CLIENT_VAP_ID;
    dhcp_type[SLI_SI91X_CLIENT] = ip_config->mode;
  } else if (SL_NET_WIFI_AP_INTERFACE == SL_NET_INTERFACE_TYPE(interface)) {
    vap_id                  = SL_WIFI_AP_VAP_ID;
    dhcp_type[SLI_SI91X_AP] = ip_config->mode;
  } else {
    return SL_STATUS_WIFI_UNSUPPORTED;
  }

  memcpy(&config, ip_config, sizeof(sl_net_ip_configuration_t));
  return sli_net_configure_ip_address(&config, vap_id, timeout);
}

sl_status_t sl_net_get_ip_address(sl_net_interface_t interface, sl_net_ip_address_t *ip_address, uint32_t timeout)
{
  uint8_t vap_id                      = 0;
  sl_status_t status                  = 0;
  sl_net_ip_configuration_t ip_config = { 0 };

  if (SL_NET_WIFI_CLIENT_INTERFACE == SL_NET_INTERFACE_TYPE(interface)) {
    vap_id           = SL_WIFI_CLIENT_VAP_ID;
    ip_address->mode = dhcp_type[SLI_SI91X_CLIENT];
  } else if (SL_NET_WIFI_AP_INTERFACE == SL_NET_INTERFACE_TYPE(interface)) {
    ip_address->mode = dhcp_type[SLI_SI91X_AP];
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_WIFI_UNSUPPORTED;
  }

  ip_config.mode = SL_IP_MANAGEMENT_DHCP;
#ifdef SLI_SI91X_ENABLE_IPV6
  ip_config.type = SL_IPV6;
#else
  ip_config.type = SL_IPV4;
#endif
  status = sli_net_configure_ip_address(&ip_config, vap_id, timeout);
  if (status != SL_STATUS_OK) {
    return status;
  }

  ip_address->type = ip_config.type;
  // Copy the IPv4 addresses to the address structure
  memcpy(ip_address->v4.ip_address.bytes, (const uint8_t *)ip_config.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
  memcpy(ip_address->v4.netmask.bytes, (const uint8_t *)ip_config.ip.v4.netmask.bytes, sizeof(sl_ipv4_address_t));
  memcpy(ip_address->v4.gateway.bytes, (const uint8_t *)ip_config.ip.v4.gateway.bytes, sizeof(sl_ipv4_address_t));

  // Copy the IPv6 addresses to the address structure
  memcpy(&ip_address->v6.link_local_address.bytes,
         (const uint8_t *)ip_config.ip.v6.link_local_address.bytes,
         sizeof(sl_ipv6_address_t));
  memcpy(&ip_address->v6.global_address.bytes,
         (const uint8_t *)ip_config.ip.v6.global_address.bytes,
         sizeof(sl_ipv6_address_t));
  memcpy(&ip_address->v6.gateway.bytes, (const uint8_t *)ip_config.ip.v6.gateway.bytes, sizeof(sl_ipv6_address_t));

  return SL_STATUS_OK;
}

//! Set DNS mode based on the configuration of primary and secondary server addresses
static bool sli_si91x_get_dns_mode(const sl_net_dns_address_t *address)
{
  bool primary_is_zero = address->primary_server_address == NULL
                         || sli_wifi_is_ip_address_zero(address->primary_server_address);
  bool secondary_is_zero = address->secondary_server_address == NULL
                           || sli_wifi_is_ip_address_zero(address->secondary_server_address);
  if (primary_is_zero && secondary_is_zero) {
    return SLI_NET_DHCP; /* Set to DHCP mode if both addresses are zero or NULL */
  } else {
    return SLI_NET_STATIC_IP; /* Set to static mode if at least one address is non-zero */
  }
}
