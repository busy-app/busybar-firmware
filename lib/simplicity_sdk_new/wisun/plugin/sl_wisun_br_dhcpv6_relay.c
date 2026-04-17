/***************************************************************************//**
 * @file sl_wisun_br_dhcpv6_relay.c
 * @brief DHCPv6 Relay to Wi-SUN Border Router
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_assert.h"
#include <cmsis_os2.h>
#include <string.h>
#include "sl_memory_manager.h"
#include "sl_wisun_ip6string.h"
#include "sl_wisun_br_lwip.h"
#include "sl_wisun_trace_api.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "common/endian.h"
#include "socket/socket.h"
#include "sl_wisun_br_dhcpv6_relay.h"

#define DHCPV6_RELAY_LENGTH 34
#define DHCPV6_OPTION_RELAY 0x0009
#define DHCPV6_OPTION_INTERFACE_ID 0x0012

#define DHCPV6_SOLICITATION_TYPE        1
#define DHCPV6_REPLY_TYPE               7
#define DHCPV6_RELAY_FORWARD            12
#define DHCPV6_RELAY_REPLY              13

typedef struct {
    uint16_t type;
    uint16_t len;
    uint8_t *msg_ptr;
} dhcp_options_msg_t;

typedef struct dhcpv6_relay_msg {
    uint8_t    type;
    uint8_t    hop_limit;
    uint8_t    *link_address;
    uint8_t    *peer_address;
    dhcp_options_msg_t relay_interface_id;
    dhcp_options_msg_t relay_options;
} dhcpv6_relay_msg_t;

static struct udp_pcb *dhcpv6_relay_wan_pcb;
static int dhcpv6_relay_pan_socket = -1;
static ip_addr_t dhcpv6_server_addr;

static const char *reply_str = "Reply";
static const char *relay_reply_str = "Relay-reply";
static const char *solicit_str = "Solicit";
static const char *relay_forward_str = "Relay-forward";

int dhcpv6_message_malformed_check(uint8_t *ptr, uint16_t data_len)
{
    uint8_t *dptr;
    uint16_t length;
    if (data_len > 4) {
        dptr = ptr + 4; //Skip Type & TXID
        data_len -= 4;
        while (data_len) {
            if (data_len >= 4) {

                length = read_be16(dptr + 2); //Skip Type
                dptr += 4;
                data_len -= 4;
                if (data_len >= length) {
                    data_len -= length;
                    dptr += length;
                }
            } else {
                return -1;
            }
        }
    }
    return 0;
}

static int dhcpv6_message_option_discover(uint8_t *ptr, uint16_t data_len, uint16_t discovered_type, dhcp_options_msg_t *option_info)
{
    uint8_t *dptr;
    uint16_t type, length;
    dptr = ptr;
    if (data_len < 4) {
        sl_wisun_trace_warn("dhcp: data_len < 4");
        return -1;
    }
    while (data_len >= 4) {
        type = read_be16(dptr);
        dptr += 2;
        length = read_be16(dptr);
        dptr += 2;
        data_len -= 4;
        if (data_len >= length) {
            if (type == discovered_type) {
                option_info->len = length;
                option_info->type = type;
                option_info->msg_ptr = dptr;
                return 0;
            }
            data_len -= length;
            dptr += length;
        } else {
            sl_wisun_trace_warn("dhcp: data_len < length=%"PRIu16, length);
            break;
        }
    }
    return -1;
}

static bool dhcpv6_relay_msg_read(uint8_t *ptr, uint16_t length, dhcpv6_relay_msg_t *relay_msg)
{
    if (length < DHCPV6_RELAY_LENGTH + 4) {
        return false;
    }
    // Relay message base first
    relay_msg->type = *ptr++;
    relay_msg->hop_limit = *ptr++;
    relay_msg->link_address = ptr;
    relay_msg->peer_address = ptr + 16;
    ptr += 32;
    //Discover
    if (dhcpv6_message_option_discover(ptr, length - 34, DHCPV6_OPTION_INTERFACE_ID, &relay_msg->relay_interface_id) != 0) {
        relay_msg->relay_interface_id.len = 0;
        relay_msg->relay_interface_id.msg_ptr = NULL;
    }

    if (dhcpv6_message_option_discover(ptr, length - 34, DHCPV6_OPTION_RELAY, &relay_msg->relay_options) != 0) {
        return false;
    }

    return true;
}

static uint8_t *dhcpv6_dhcp_option_header_write(uint8_t *ptr, uint16_t option_type, uint16_t length)
{
    ptr = write_be16(ptr, option_type);
    ptr = write_be16(ptr, length);
    return ptr;
}

static uint8_t *dhcpv6_dhcp_relay_msg_write(uint8_t *ptr, uint8_t type, uint8_t hop_limit,  uint8_t *peer_addres, uint8_t *link_address)
{
    *ptr++ = type;
    *ptr++ = hop_limit;
    memcpy(ptr, link_address, 16);
    ptr += 16;
    memcpy(ptr, peer_addres, 16);
    ptr += 16;
    return ptr;
}

/**
 * Handle DHCPv6 relay-reply and reply messages from WAN interface.
 */
static void dhcpv6_relay_wan_on_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
  dhcpv6_relay_msg_t relay_msg;
  uint8_t msg_type;
  ssize_t count;
  const char *msg_type_str;
  char src_addr_str[MAX_IPV6_STRING_LEN_WITH_TRAILING_NULL];
  char dst_addr_str[MAX_IPV6_STRING_LEN_WITH_TRAILING_NULL];
  uint16_t dst_port;

  sockaddr_in6_t peer_address = {
    .sin6_family = AF_INET6,
    .sin6_port = htons(DHCPV6_SERVER_PORT),
    .sin6_flowinfo = 0,
    .sin6_addr = IN6ADDR_ANY_INIT,
    .sin6_scope_id = 0,
  };

  (void)arg;
  (void)pcb;
  (void)addr;
  (void)port;

  msg_type = *(uint8_t *)p->payload;

  if (msg_type != DHCPV6_RELAY_REPLY) {
    sl_wisun_trace_error("dhcp-wan: option type %"PRIu16" not handled", msg_type);
    goto cleanup;
  }

  if (!dhcpv6_relay_msg_read(p->payload, p->len, &relay_msg)) {
    sl_wisun_trace_error("dhcp-wan: invalid relay-reply message");
    goto cleanup;
  }

  msg_type = *(uint8_t *)relay_msg.relay_options.msg_ptr;

  memcpy(peer_address.sin6_addr.address, relay_msg.peer_address, 16);
  if (relay_msg.hop_limit > 0 && msg_type == DHCPV6_RELAY_REPLY) {
    peer_address.sin6_port = htons(DHCPV6_SERVER_PORT);
    msg_type_str = relay_reply_str;
    dst_port = DHCPV6_SERVER_PORT;
  } else if (relay_msg.hop_limit == 0 && msg_type == DHCPV6_REPLY_TYPE) {
    peer_address.sin6_port = htons(DHCPV6_CLIENT_PORT);
    msg_type_str = reply_str;
    dst_port = DHCPV6_CLIENT_PORT;
  } else {
    sl_wisun_trace_error("dhcp-wan: malformed packet");
    goto cleanup;
  }

  ip6tos(ip_2_ip6(addr), src_addr_str);
  ip6tos(peer_address.sin6_addr.address, dst_addr_str);
  sl_wisun_trace_info("%s from %s relayed to %s port %"PRIu16, msg_type_str, src_addr_str, dst_addr_str, dst_port);

  count = sendto(dhcpv6_relay_pan_socket, relay_msg.relay_options.msg_ptr, relay_msg.relay_options.len, 0, (struct sockaddr *)&peer_address, sizeof(sockaddr_in6_t));
  if (count < 0) {
    sl_wisun_trace_error("dhcp-wan: sendto error %zd", count);
    goto cleanup;
  }

cleanup:
  pbuf_free(p);
}

void sl_wisun_br_dhcpv6_relay_pan_on_recv(uint8_t *buffer, ssize_t length, in6_addr_t peer_address, in_port_t remote_port)
{
  dhcpv6_relay_msg_t relay_msg;
  uint8_t link_address[16];
  sl_status_t status;
  uint8_t msg_type, *ptr, hop_limit;
  struct pbuf *p = NULL;
  err_t err;
  const char *msg_type_str;
  char src_addr_str[MAX_IPV6_STRING_LEN_WITH_TRAILING_NULL];
  char dst_addr_str[MAX_IPV6_STRING_LEN_WITH_TRAILING_NULL];

  (void)remote_port;

  msg_type = *buffer;

  if (msg_type == DHCPV6_RELAY_FORWARD) {
    if (!dhcpv6_relay_msg_read(buffer, length, &relay_msg)) {
      sl_wisun_trace_error("dhcp-pan: not valid relay-reply message");
      goto cleanup;
    }
    if (dhcpv6_message_malformed_check(relay_msg.relay_options.msg_ptr, relay_msg.relay_options.len) < 0) {
      sl_wisun_trace_error("dhcp-pan: malformed packet");
      goto cleanup;
    }
    hop_limit = relay_msg.hop_limit + 1;
    msg_type_str = relay_forward_str;
  } else if (msg_type == DHCPV6_SOLICITATION_TYPE) {
    if (dhcpv6_message_malformed_check(buffer, length) < 0) {
      sl_wisun_trace_error("dhcp-pan: malformed packet");
      goto cleanup;
    }
    hop_limit = 0;
    msg_type_str = solicit_str;
  } else {
    sl_wisun_trace_error("dhcp-pan: unhandled message type %"PRIu8, msg_type);
    goto cleanup;
  }

  p = pbuf_alloc(PBUF_TRANSPORT, DHCPV6_RELAY_LENGTH + 4 + length, PBUF_RAM);
  if (!p) {
    sl_wisun_trace_error("dhcp-pan: pbuf_alloc failed");
    goto cleanup;
  }

  status = sl_wisun_br_lwip_pan_get_address(link_address);
  if (status != SL_STATUS_OK) {
    sl_wisun_trace_error("dhcp-pan: sl_wisun_br_lwip_pan_get_address failed");
    goto cleanup;
  }

  ptr = p->payload;
  ptr = dhcpv6_dhcp_relay_msg_write(ptr, DHCPV6_RELAY_FORWARD, hop_limit, peer_address.address, link_address);
  ptr = dhcpv6_dhcp_option_header_write(ptr, DHCPV6_OPTION_RELAY, length);
  memcpy(ptr, buffer, length);

  ip6tos(peer_address.address, src_addr_str);
  ip6tos(dhcpv6_server_addr.addr, dst_addr_str);
  sl_wisun_trace_info("%s from  %s relayed to %s port %"PRIu16, msg_type_str, src_addr_str, dst_addr_str, DHCPV6_SERVER_PORT);

  err = udp_sendto(dhcpv6_relay_wan_pcb, p, &dhcpv6_server_addr, DHCPV6_SERVER_PORT);
  if (err != ERR_OK) {
    sl_wisun_trace_error("dhcp-pan: udp_sendto error %d", (int)err);
    goto cleanup;
  }

cleanup:
  if (p) {
    pbuf_free(p);
  }
}

sl_status_t sl_wisun_br_dhcpv6_relay_init(void)
{
  err_t err;

  dhcpv6_relay_wan_pcb = udp_new();
  if (!dhcpv6_relay_wan_pcb) {
    sl_wisun_trace_error("dhcp-relay: udp_new failed");
    goto error;
  }

  err = udp_bind(dhcpv6_relay_wan_pcb, IP_ANY_TYPE, DHCPV6_SERVER_PORT);
  if (err != ERR_OK) {
    sl_wisun_trace_error("dhcp-relay: udp_bind failed");
    goto error;
  }

  udp_recv(dhcpv6_relay_wan_pcb, dhcpv6_relay_wan_on_recv, NULL);

  return SL_STATUS_OK;

error:

  return SL_STATUS_FAIL;
}

sl_status_t sl_wisun_br_dhcpv6_relay_start(int socket, const uint8_t *server_addr)
{
  dhcpv6_relay_pan_socket = socket;
  IP6_ADDR_PART(&dhcpv6_server_addr, 0, server_addr[0], server_addr[1], server_addr[2], server_addr[3]);
  IP6_ADDR_PART(&dhcpv6_server_addr, 1, server_addr[4], server_addr[5], server_addr[6], server_addr[7]);
  IP6_ADDR_PART(&dhcpv6_server_addr, 2, server_addr[8], server_addr[9], server_addr[10], server_addr[11]);
  IP6_ADDR_PART(&dhcpv6_server_addr, 3, server_addr[12], server_addr[13], server_addr[14], server_addr[15]);

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_br_dhcpv6_relay_stop(void) {
  dhcpv6_relay_pan_socket = -1;
  return SL_STATUS_OK;
}
