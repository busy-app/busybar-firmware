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
#include "sl_net_types.h"
#include "sli_net_utility.h"
#include "sli_net_common_utility.h"
#include "sl_si91x_core_utilities.h"
#include "sl_si91x_driver.h"
#include "sl_si91x_constants.h"
#include "sli_wifi_utility.h"
#include "sl_si91x_protocol_types.h"
#ifdef SLI_SI91X_INTERNAL_MDNS
#include "sl_mdns.h"
#endif
extern sl_net_event_handler_t net_event_handler;

sl_status_t sl_si91x_default_handler(sl_net_event_t event, sl_wifi_buffer_t *buffer)
{
  sl_wifi_system_packet_t *packet     = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  sl_status_t status                  = sli_convert_and_save_firmware_status(sli_get_si91x_frame_status(packet));
  sl_ip_address_t ip                  = { 0 };
  sl_net_ip_configuration_t ip_config = { 0 };
  const sli_si91x_rsp_ipv4_params_t *ipv4_parameters = NULL;
  const sli_si91x_rsp_ipv6_params_t *ipv6_parameters = NULL;
  void *data;

  // Check if there's a valid event handler registered for this event
  if (net_event_handler == NULL) {
    return SL_STATUS_FAIL; // If no event handler is registered, return failure
  }

  // Depending on the event type, prepare data for the event handler
  switch (event) {
    case SL_NET_DNS_RESOLVE_EVENT: {
      data = &ip;

      // Convert the SI91x DNS response to an IP address structure
      sli_convert_si91x_dns_response(&ip, (sli_si91x_dns_response_t *)packet->data);
      break;
    }
    case SL_NET_OTA_FW_UPDATE_EVENT:
    case SL_NET_PING_RESPONSE_EVENT:
    case SL_NET_DHCP_NOTIFICATION_EVENT: {
      data = &packet->data; // Use packet data directly for certain events
      break;
    }
#ifdef SLI_SI91X_INTERNAL_MDNS
    case SL_NET_MDNS_EVENT: {
      sl_mdns_response_t *mdns_result = malloc(sizeof(sl_mdns_response_t));
      if (mdns_result == NULL) {
        return SL_STATUS_ALLOCATION_FAILED;
      }

      sl_status_t conv_status = sli_convert_si91x_mdns_response(mdns_result, (sli_net_mdns_response_t *)packet->data);
      if (conv_status != SL_STATUS_OK) {
        free(mdns_result);
        return conv_status;
      }

      data = mdns_result;
      break;
    }
    case SL_NET_MDNS_STOP_EVENT: {
      data = NULL;
      break;
    }
#endif
    case SL_NET_IP_ADDRESS_CHANGE_EVENT: {

      data                = &ip_config;
      ip_config.host_name = NULL;
      ip_config.mode      = SL_IP_MANAGEMENT_DHCP;

      if (packet->command == SLI_WLAN_RSP_IPCONFV6) {
        ipv6_parameters = (sli_si91x_rsp_ipv6_params_t *)packet->data;
        ip_config.type  = SL_IPV6;

        if (NULL != ipv6_parameters) {
          memcpy(ip_config.ip.v6.link_local_address.bytes,
                 (const uint8_t *)ipv6_parameters->link_local_address,
                 sizeof(ipv6_parameters->link_local_address));
          memcpy(ip_config.ip.v6.global_address.bytes,
                 (const uint8_t *)ipv6_parameters->global_address,
                 sizeof(ipv6_parameters->global_address));
          memcpy(ip_config.ip.v6.gateway.bytes,
                 (const uint8_t *)ipv6_parameters->gateway_address,
                 sizeof(ipv6_parameters->gateway_address));
        }

      } else {
        ipv4_parameters = (sli_si91x_rsp_ipv4_params_t *)packet->data;
        ip_config.type  = SL_IPV4;

        if (NULL != ipv4_parameters) {
          memcpy(ip_config.ip.v4.ip_address.bytes, ipv4_parameters->ipaddr, sizeof(ipv4_parameters->ipaddr));
          memcpy(ip_config.ip.v4.netmask.bytes, ipv4_parameters->netmask, sizeof(ipv4_parameters->netmask));
          memcpy(ip_config.ip.v4.gateway.bytes, ipv4_parameters->gateway, sizeof(ipv4_parameters->gateway));
        }
      }
      break;
    }
    default: {
      SL_DEBUG_LOG("\r\nUnsupported event\r\n");
      return SL_STATUS_FAIL; // Return failure for unsupported events
    }
  }
  // Call the registered event handler function with event details
  net_event_handler(event, status, data, packet->length);
#ifdef SLI_SI91X_INTERNAL_MDNS
  if (event == SL_NET_MDNS_EVENT) {
    sl_mdns_response_t *mdns_result = (sl_mdns_response_t *)data;
    if (mdns_result->txt.txt != NULL) {
      free(mdns_result->txt.txt);
    }
    if (mdns_result->addr.addr != NULL) {
      free(mdns_result->addr.addr);
      mdns_result->addr.addr = NULL;
    }
    free(mdns_result);
  }
#endif
  return SL_STATUS_OK;
}
