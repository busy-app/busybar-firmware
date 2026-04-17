/***************************************************************************//**
 * @file sl_wisun_br_lwip.c
 * @brief Integrates lwIP to Wi-SUN Border Router
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

#include <string.h>
#include "lwip/init.h"
#include "lwip/ip6.h"
#include "lwip/etharp.h"
#include "lwip/ethip6.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "border_router/sl_wisun_br_api.h"
#include "sl_wisun_ip6string.h"
#include "sl_wisun_trace_api.h"
#include "sl_wisun_br_lwip.h"

#define IPV6_MIN_LINK_MTU   1280

NETIF_DECLARE_EXT_CALLBACK(netif_ext_callback);
static struct netif netif_wan;
static struct netif netif_pan;
static ip6_addr_t pan_ipv6_address;
static sl_wisun_br_lwip_wan_output_handler_t wan_output_handler;
static sl_wisun_br_lwip_wan_link_state_handler_t wan_link_state_handler;

static void netif_ext_callback_handler(struct netif *netif, netif_nsc_reason_t reason, const netif_ext_callback_args_t *args)
{
  s8_t addr_index;
  char addr_str[MAX_IPV6_STRING_LEN_WITH_TRAILING_NULL];

  if (reason == LWIP_NSC_IPV6_ADDR_STATE_CHANGED) {
    addr_index = args->ipv6_addr_state_changed.addr_index;
    if (addr_index < LWIP_IPV6_NUM_ADDRESSES
        && ip6_addr_isvalid(netif_ip6_addr_state(netif, addr_index))) {
      ip6tos(netif_ip_addr6(netif, addr_index), addr_str);
      if (netif == &netif_pan) {
        sl_wisun_trace_debug("lwip-pan: %s", addr_str);
      } else if (netif == &netif_wan) {
        sl_wisun_trace_debug("lwip-wan: %s", addr_str);
        if (wan_link_state_handler
            && (ip6_addr_isglobal(netif_ip_addr6(netif, addr_index))
                || ip6_addr_isuniquelocal(netif_ip_addr6(netif, addr_index)))) {
          wan_link_state_handler(true);
        }
      }
    }
  }
}

static sl_status_t netif_get_ipv6_address(struct netif *netif, uint8_t *addr)
{
  uint8_t i;
  for (i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++) {
    if (ip6_addr_isvalid(netif_ip6_addr_state(netif, i))
        && (ip6_addr_isglobal(netif_ip_addr6(netif, i))
            || ip6_addr_isuniquelocal(netif_ip_addr6(netif, i)))) {
      memcpy(addr, netif_ip_addr6(netif, i)->addr, IPV6_ADDR_SIZE);
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_FAIL;
}

struct netif *sl_wisun_br_lwip_ip6_route_hook(const ip6_addr_t *src, const ip6_addr_t *dest)
{
  uint8_t ipv6_addr[IPV6_ADDR_SIZE];
  sl_status_t status;
  bool exists;

  (void)src;

  // Is the destination node the Border Router itself?
  if (ip_addr_cmp(&pan_ipv6_address, dest)) {
    return &netif_pan;
  }

  // Is the destination node in Wi-SUN Network?
  memcpy(ipv6_addr, dest->addr, IPV6_ADDR_SIZE);
  status = sl_wisun_br_ipv6_route_exists(ipv6_addr, &exists);
  if (status == SL_STATUS_OK && exists) {
    return &netif_pan;
  }

  return NULL;
}

sl_status_t sl_wisun_br_lwip_wan_input(const uint8_t *data, size_t data_length)
{
  struct pbuf *p;
  struct eth_hdr *eth_hdr;
  err_t ret;

  EFM_ASSERT(data_length >= ETH_HWADDR_LEN);
  eth_hdr = (struct eth_hdr *)data;
  if (!memcmp(eth_hdr->src.addr, netif_wan.hwaddr, ETH_HWADDR_LEN)) {
    return SL_STATUS_OK;
  }

  // No extra headroom needed since the IPv6 packet is injected to Nanostack
  p = pbuf_alloc(PBUF_RAW, data_length, PBUF_RAM);
  if (!p) {
    sl_wisun_trace_error("lwip-wan: pbuf_alloc failed");
    return SL_STATUS_FAIL;
  }

  ret = pbuf_take(p, data, data_length);
  if (ret != ERR_OK) {
    // In case of an error, the caller is responsible for freeing the buffer
    pbuf_free(p);
    sl_wisun_trace_error("lwip-wan: pbuf_take error %d", (int)ret);
    return SL_STATUS_FAIL;
  }

  ret = netif_wan.input(p, &netif_wan);
  if (ret != ERR_OK) {
    sl_wisun_trace_error("lwip-wan: lwIP input error %d", (int)ret);
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_br_lwip_set_wan_output_handler(sl_wisun_br_lwip_wan_output_handler_t handler)
{
  wan_output_handler = handler;
  return SL_STATUS_OK;
}

static err_t netif_wan_output(struct netif *netif, struct pbuf *p)
{
  sl_status_t status;

  (void)netif;

  if (!wan_output_handler) {
    sl_wisun_trace_error("lwip-wan: no handler registered");
    return ERR_IF;
  }

  status = wan_output_handler(p->payload, p->len);
  if (status != SL_STATUS_OK) {
    sl_wisun_trace_error("lwip-wan: wan_output_handler error %d", (int)status);
    return ERR_IF;
  }

  sl_wisun_trace_debug("lwip-wan: %"PRIu16" bytes written", p->len);

  return ERR_OK;
}

static err_t netif_wan_on_init(struct netif *netif)
{
  netif->name[0] = 'W';
  netif->name[1] = 'A';
  netif->output_ip6 = ethip6_output;
  netif->linkoutput = netif_wan_output;
  netif->mtu = IPV6_MIN_LINK_MTU;
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHERNET;

  netif_set_ip6_autoconfig_enabled(netif, 1);

  return ERR_OK;
}

sl_status_t sl_wisun_br_lwip_get_wan_address(uint8_t *ipv6_address)
{
  return netif_get_ipv6_address(&netif_wan, ipv6_address);
}

sl_status_t sl_wisun_br_lwip_set_wan_link_state_handler(sl_wisun_br_lwip_wan_link_state_handler_t handler)
{
  wan_link_state_handler = handler;
  return SL_STATUS_OK;
}

void sl_wisun_br_lwip_wan_up(const uint8_t *eui48)
{
  netif_wan.hwaddr[0] = eui48[0];
  netif_wan.hwaddr[1] = eui48[1];
  netif_wan.hwaddr[2] = eui48[2];
  netif_wan.hwaddr[3] = eui48[3];
  netif_wan.hwaddr[4] = eui48[4];
  netif_wan.hwaddr[5] = eui48[5];
  netif_wan.hwaddr_len = ETH_HWADDR_LEN;
  netif_create_ip6_linklocal_address(&netif_wan, 1);

  netif_set_link_up(&netif_wan);
  netif_set_up(&netif_wan);
  netif_set_default(&netif_wan);
}

void sl_wisun_br_lwip_wan_down(void)
{
  netif_set_link_down(&netif_wan);
  netif_set_down(&netif_wan);
}

sl_status_t sl_wisun_br_lwip_pan_input(const uint8_t *data, size_t data_length)
{
  struct pbuf *p;
  err_t ret;

  // Allocate extra headroom for link layer
  p = pbuf_alloc(PBUF_LINK, data_length, PBUF_RAM);
  if (!p) {
    sl_wisun_trace_error("lwip-pan: pbuf_alloc failed");
    return SL_STATUS_FAIL;
  }

  ret = pbuf_take(p, data, data_length);
  if (ret != ERR_OK) {
    // In case of an error, the caller is responsible for freeing the buffer
    pbuf_free(p);
    sl_wisun_trace_error("lwip-pan: pbuf_take error %d", (int)ret);
    return SL_STATUS_FAIL;
  }

  ret = netif_pan.input(p, &netif_pan);
  if (ret != ERR_OK) {
    sl_wisun_trace_error("lwip-pan: lwIP input error %d", (int)ret);
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

static err_t netif_pan_output(struct netif *netif, struct pbuf *p, const ip6_addr_t *ipaddr)
{
  uint8_t buf[IPV6_MIN_LINK_MTU];
  uint8_t *data;
  sl_status_t status;

  // pbuf ownership is not transferred

  (void)netif;
  (void)ipaddr;
  data = pbuf_get_contiguous(p, buf, sizeof(buf), p->tot_len, 0);
  if (!data) {
    sl_wisun_trace_error("lwip-pan: pbuf_get_contiguous failed");
    return ERR_ARG;
  }

  status = sl_wisun_br_ipv6_down(data, p->tot_len);
  if (status != SL_STATUS_OK) {
    sl_wisun_trace_error("lwip-pan: sl_wisun_br_ipv6_down error %d", (int)status);
    return ERR_ARG;
  }

  sl_wisun_trace_debug("lwip-pan: %"PRIu16" bytes written", p->tot_len);

  return ERR_OK;
}

static err_t netif_pan_on_init(struct netif *netif)
{
  netif->name[0] = 'F';
  netif->name[1] = 'A';
  netif->output_ip6 = netif_pan_output;
  netif->mtu = IPV6_MIN_LINK_MTU;
  netif->flags = NETIF_FLAG_BROADCAST;

  return ERR_OK;
}

sl_status_t sl_wisun_br_lwip_pan_get_address(uint8_t *ipv6_address)
{
  memcpy(ipv6_address, pan_ipv6_address.addr, IPV6_ADDR_SIZE);
  return SL_STATUS_OK;
}

void sl_wisun_br_lwip_pan_up(const uint8_t *ll_addr, const uint8_t *gua_addr)
{
  ip6_addr_t lwip_ll;

  IP6_ADDR_PART(&lwip_ll, 0, ll_addr[0], ll_addr[1], ll_addr[2], ll_addr[3]);
  IP6_ADDR_PART(&lwip_ll, 1, ll_addr[4], ll_addr[5], ll_addr[6], ll_addr[7]);
  IP6_ADDR_PART(&lwip_ll, 2, ll_addr[8], ll_addr[9], ll_addr[10], ll_addr[11]);
  IP6_ADDR_PART(&lwip_ll, 3, ll_addr[12], ll_addr[13], ll_addr[14], ll_addr[15]);
  netif_ip6_addr_set(&netif_pan, 0, &lwip_ll);
  netif_ip6_addr_set_state(&netif_pan, 0, IP6_ADDR_PREFERRED);

  IP6_ADDR_PART(&pan_ipv6_address, 0, gua_addr[0], gua_addr[1], gua_addr[2], gua_addr[3]);
  IP6_ADDR_PART(&pan_ipv6_address, 1, gua_addr[4], gua_addr[5], gua_addr[6], gua_addr[7]);
  IP6_ADDR_PART(&pan_ipv6_address, 2, gua_addr[8], gua_addr[9], gua_addr[10], gua_addr[11]);
  IP6_ADDR_PART(&pan_ipv6_address, 3, gua_addr[12], gua_addr[13], gua_addr[14], gua_addr[15]);

  netif_set_link_up(&netif_pan);
  netif_set_up(&netif_pan);
  // Prevent Router Solicitations on PAN interface
  netif_pan.rs_count = 0;
}

void sl_wisun_br_lwip_pan_down(void)
{
  netif_set_link_down(&netif_pan);
  netif_set_down(&netif_pan);
}

void sl_wisun_br_lwip_init(void)
{
  tcpip_init(NULL, NULL);
  netif_add_ext_callback(&netif_ext_callback, netif_ext_callback_handler);
  netif_add_noaddr(&netif_pan, NULL, netif_pan_on_init, tcpip_input);
  netif_add_noaddr(&netif_wan, NULL, netif_wan_on_init, tcpip_input);
}
