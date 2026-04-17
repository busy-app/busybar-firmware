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
#include "sl_si91x_protocol_types.h"
#include "sl_si91x_constants.h"
#include "sl_net_constants.h"
#include "sl_wifi_constants.h"
#include "sl_status.h"
#include "sl_constants.h"
#include "sl_wifi_types.h"
#include "sli_net_utility.h"
#include <string.h>
#include <sl_string.h>
#include "sl_utility.h"
#ifdef SLI_SI91X_INTERNAL_MDNS
#include "sl_mdns.h"
#define SLI_MDNS_MAX_PARAM_LENGTH 255
#endif

sl_status_t sli_convert_rsi_ipv4_address_to_sl_ip_address(sl_ip_address_t *ip_address_buffer,
                                                          const sli_si91x_rsp_ipv4_params_t *ip_params)
{
  // Verify input pointers
  SL_VERIFY_POINTER_OR_RETURN(ip_address_buffer, SL_STATUS_WIFI_NULL_PTR_ARG);
  SL_VERIFY_POINTER_OR_RETURN(ip_params, SL_STATUS_WIFI_NULL_PTR_ARG);

  uint8_t *ip_address;

  ip_address_buffer->type = SL_IPV4;
  ip_address              = ip_address_buffer->ip.v4.bytes;

  memcpy(ip_address, ip_params->ipaddr, sizeof(sl_ipv4_address_t));
  return SL_STATUS_OK;
}

sl_status_t sli_convert_si91x_dns_response(sl_ip_address_t *ip_address,
                                           const sli_si91x_dns_response_t *si91x_dns_response)
{
  SL_VERIFY_POINTER_OR_RETURN(ip_address, SL_STATUS_WIFI_NULL_PTR_ARG);
  SL_VERIFY_POINTER_OR_RETURN(si91x_dns_response, SL_STATUS_WIFI_NULL_PTR_ARG);

  // Check if DNS response has IP addresses
  if ((si91x_dns_response->ip_count[0] | (si91x_dns_response->ip_count[1] << 8)) <= 0) {
    return SL_STATUS_OK;
  }

  // Determine IP address size (IPv4 or IPv6) and copy the address bytes
  uint8_t ip_address_size = (si91x_dns_response->ip_version[0] | si91x_dns_response->ip_version[1] << 8)
                                == SL_IPV4_ADDRESS_LENGTH
                              ? SL_IPV4_ADDRESS_LENGTH
                              : SL_IPV6_ADDRESS_LENGTH;
  uint8_t *sl_ip_address;
  const uint8_t *si91x_ip_address;

  ip_address->type = ip_address_size == SL_IPV4_ADDRESS_LENGTH ? SL_IPV4 : SL_IPV6;

  si91x_ip_address = ip_address_size == SL_IPV4_ADDRESS_LENGTH ? si91x_dns_response->ip_address[0].ipv4_address
                                                               : si91x_dns_response->ip_address[0].ipv6_address;
  sl_ip_address    = ip_address_size == SL_IPV4_ADDRESS_LENGTH ? ip_address->ip.v4.bytes : ip_address->ip.v6.bytes;

  memcpy(sl_ip_address, si91x_ip_address, ip_address_size);

  return SL_STATUS_OK;
}

sl_status_t sli_convert_si91x_event_to_sl_net_event(const uint16_t *event,
                                                    sl_net_event_t *sl_net_event,
                                                    const sl_wifi_system_packet_t *packet)
{
  // Verify input pointers
  SL_WIFI_ARGS_CHECK_NULL_POINTER(event);
  SL_WIFI_ARGS_CHECK_NULL_POINTER(sl_net_event);

  // Map SI91X events to SimpleLink network events
  switch (*event) {
    case SLI_WLAN_RSP_DNS_QUERY: {
      *sl_net_event = SL_NET_DNS_RESOLVE_EVENT;
      return SL_STATUS_OK;
    }
    case SLI_WLAN_RSP_PING_PACKET: {
      *sl_net_event = SL_NET_PING_RESPONSE_EVENT;
      return SL_STATUS_OK;
    }
    case SLI_WLAN_RSP_OTA_FWUP: {
      *sl_net_event = SL_NET_OTA_FW_UPDATE_EVENT;
      return SL_STATUS_OK;
    }
    case SLI_WLAN_RSP_IPCONFV4: {
      *sl_net_event = SL_NET_DHCP_NOTIFICATION_EVENT;
      return SL_STATUS_OK;
    }
    case SLI_WLAN_RSP_IPV4_CHANGE:
    case SLI_WLAN_RSP_IPCONFV6: {
      *sl_net_event = SL_NET_IP_ADDRESS_CHANGE_EVENT;
      return SL_STATUS_OK;
    }
    case SLI_WLAN_RSP_DISCOVER_SERVICE: {
      // Check packet length to differentiate between MDNS STOP and MDNS QUERY
      if (packet->length == 1) {
        *sl_net_event = SL_NET_MDNS_STOP_EVENT;
      } else {
        *sl_net_event = SL_NET_MDNS_EVENT;
      }
      return SL_STATUS_OK;
    }
    default:
      break;
  }

  return SL_STATUS_FAIL;
}

bool sli_wifi_is_ip_address_zero(const sl_ip_address_t *ip_addr)
{
  if (ip_addr->type == SL_IPV4) {
    for (int i = 0; i < 4; i++) {
      if (ip_addr->ip.v4.bytes[i] != 0) {
        return false; // Non-zero byte found
      }
    }
    return true; // All bytes are zero
  } else if (ip_addr->type == SL_IPV6) {
    for (int i = 0; i < 16; i++) {
      if (ip_addr->ip.v6.bytes[i] != 0) {
        return false; // Non-zero byte found
      }
    }
    return true; // All bytes are zero
  }

  return false; // Invalid or unsupported type
}

#ifdef SLI_SI91X_INTERNAL_MDNS
sl_status_t sli_convert_si91x_mdns_response(sl_mdns_response_t *mdns_result, const sli_net_mdns_response_t *raw_result)
{
  SL_VERIFY_POINTER_OR_RETURN(mdns_result, SL_STATUS_WIFI_NULL_PTR_ARG);
  SL_VERIFY_POINTER_OR_RETURN(raw_result, SL_STATUS_WIFI_NULL_PTR_ARG);

  const uint8_t *ptr = raw_result->data;

  mdns_result->query_type      = raw_result->query_type;
  mdns_result->port            = raw_result->port;
  mdns_result->ttl             = raw_result->ttl;
  mdns_result->txt.txt_count   = raw_result->txt_count;
  mdns_result->addr.addr_count = raw_result->addr_count;

  // Assign all string fields in fixed order
  mdns_result->instance_name = (const char *)ptr;
  ptr += sl_strnlen((const char *)ptr, SLI_MDNS_MAX_PARAM_LENGTH) + 1;

  mdns_result->service_type = (const char *)ptr;
  ptr += sl_strnlen((const char *)ptr, SLI_MDNS_MAX_PARAM_LENGTH) + 1;

  mdns_result->proto = (const char *)ptr;
  ptr += sl_strnlen((const char *)ptr, SLI_MDNS_MAX_PARAM_LENGTH) + 1;

  mdns_result->hostname = (const char *)ptr;
  ptr += sl_strnlen((const char *)ptr, SLI_MDNS_MAX_PARAM_LENGTH) + 1;

  // TXT records
  mdns_result->txt.txt = NULL;
  if (mdns_result->txt.txt_count > 0) {
    mdns_result->txt.txt = malloc(mdns_result->txt.txt_count * sizeof(const char *));
    if (mdns_result->txt.txt == NULL) {
      return SL_STATUS_ALLOCATION_FAILED;
    }

    for (size_t i = 0; i < mdns_result->txt.txt_count; i++) {
      mdns_result->txt.txt[i] = (const char *)ptr;
      ptr += sl_strnlen((const char *)ptr, SLI_MDNS_MAX_PARAM_LENGTH) + 1;
    }
  }

  if (mdns_result->addr.addr_count > 0) {
    mdns_result->addr.addr = malloc(mdns_result->addr.addr_count * sizeof(sl_ip_address_t));
    if (mdns_result->addr.addr == NULL) {
      // Free previously allocated txt.txt
      if (mdns_result->txt.txt != NULL) {
        free(mdns_result->txt.txt);
        mdns_result->txt.txt = NULL;
      }
      return SL_STATUS_ALLOCATION_FAILED;
    }

    for (size_t i = 0; i < mdns_result->addr.addr_count; i++) {
      uint8_t addr_type = *ptr++; // Read type byte

      if (addr_type == SL_IPV4_VERSION) {
        mdns_result->addr.addr[i].type = SL_IPV4;
        memcpy(mdns_result->addr.addr[i].ip.v4.bytes, ptr, SL_IPV4_ADDRESS_LENGTH);
        ptr += SL_IPV4_ADDRESS_LENGTH;
      } else if (addr_type == SL_IPV6_VERSION) {
        mdns_result->addr.addr[i].type = SL_IPV6;
        sli_big_to_little_endian((const unsigned int *)ptr,
                                 mdns_result->addr.addr[i].ip.v6.bytes,
                                 SL_IPV6_ADDRESS_LENGTH);
        ptr += SL_IPV6_ADDRESS_LENGTH;
      } else {
        return SL_STATUS_INVALID_PARAMETER;
      }
    }
  }
  return SL_STATUS_OK;
}
#endif

#ifdef SLI_SI91X_INTERNAL_HTTP_CLIENT
// Convert integer to string
void convert_itoa(uint32_t val, uint8_t *str)
{
  int16_t ii = 0;
  int16_t jj = 0;
  uint8_t tmp[10];

  if (val == 0) {
    // if value is zero then handling
    str[jj] = '0';
    jj++;
    str[jj] = '\0';
    return;
  }

  // Convert the integer to a string
  while (val) {
    tmp[ii] = '0' + (val % 10);
    val /= 10;
    ii++;
  }

  // Reverse the string
  for (jj = 0, ii--; ii >= 0; ii--, jj++) {
    str[jj] = tmp[ii];
  }
  str[jj] = '\0';
}

sl_status_t sli_convert_si91x_event_to_sl_http_client_event(const uint16_t *event,
                                                            sl_http_client_event_t *sl_http_client_event)
{
  // Verify input pointer
  SL_WIFI_ARGS_CHECK_NULL_POINTER(event);

  // Map SI91X HTTP client events to SimpleLink HTTP client events
  switch (*event) {
    case SLI_WLAN_RSP_HTTP_CLIENT_GET: {
      *sl_http_client_event = SL_HTTP_CLIENT_GET_RESPONSE_EVENT;
      return SL_STATUS_OK;
    }

    case SLI_WLAN_RSP_HTTP_CLIENT_POST:
    case SLI_WLAN_RSP_HTTP_CLIENT_POST_DATA: {
      *sl_http_client_event = SL_HTTP_CLIENT_POST_RESPONSE_EVENT;
      return SL_STATUS_OK;
    }

    case SLI_WLAN_RSP_HTTP_CLIENT_PUT: {
      *sl_http_client_event = SL_HTTP_CLIENT_PUT_RESPONSE_EVENT;
      return SL_STATUS_OK;
    }
    default:
      break;
  }

  return SL_STATUS_FAIL;
}
#endif
