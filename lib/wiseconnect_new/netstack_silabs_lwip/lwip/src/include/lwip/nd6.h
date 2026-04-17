/**
 * @file
 *
 * Neighbor discovery and stateless address autoconfiguration for IPv6.
 * Aims to be compliant with RFC 4861 (Neighbor discovery) and RFC 4862
 * (Address autoconfiguration).
 */

/*
 * Copyright (c) 2010 Inico Technologies Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Ivan Delamer <delamer@inicotech.com>
 *
 *
 * Please coordinate changes and requests with Ivan Delamer
 * <delamer@inicotech.com>
 */

#ifndef LWIP_HDR_ND6_H
#define LWIP_HDR_ND6_H

#include "lwip/opt.h"

#if LWIP_IPV6  /* don't build if not configured for use in lwipopts.h */

#include "lwip/ip6_addr.h"
#include "lwip/err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 1 second period */
#define ND6_TMR_INTERVAL 1000

/** Router solicitations are sent in 4 second intervals (see RFC 4861, ch. 6.3.7) */
#ifndef ND6_RTR_SOLICITATION_INTERVAL
#define ND6_RTR_SOLICITATION_INTERVAL  4000
#endif
#if SL_LWIP_ND6_DYNAMIC_TIMER
/** Timer interval for active states like INCOMPLETE, DELAY, PROBE (1 second) */
#ifndef ND6_TMR_ACTIVE_INTERVAL
#define ND6_TMR_ACTIVE_INTERVAL  1000
#endif

/** Timer interval for eco states like STALE and REACHABLE (30 seconds) */
#ifndef ND6_TMR_ECO_INTERVAL
#define ND6_TMR_ECO_INTERVAL  30000
#endif

#if ND6_TMR_ECO_INTERVAL > 30000
#error "ND6_TMR_ECO_INTERVAL must be <= 30000ms (30 seconds - validated maximum)"
#endif

/** Minimum timer interval in milliseconds to ensure optimal performance when multiple timers exist */
#ifndef ND6_MIN_TMR_INTERVAL
#define ND6_MIN_TMR_INTERVAL  1000
#endif

/** Maximum timer value that won't overflow when multiplied by 1000 */
#ifndef ND6_MAX_TIMER_VALUE_FOR_MULTIPLICATION
#define ND6_MAX_TIMER_VALUE_FOR_MULTIPLICATION  (UINT32_MAX / 1000)
#endif

/** Maximum 32-bit unsigned integer value */
#ifndef ND6_MAX_U32_VALUE
#define ND6_MAX_U32_VALUE  UINT32_MAX
#endif

/** Eco mode router solicitation interval adjustment in seconds */
#ifndef ND6_ECO_RS_INTERVAL_ADJUSTMENT
#define ND6_ECO_RS_INTERVAL_ADJUSTMENT  2
#endif
#endif

struct pbuf;
struct netif;
#if SL_LWIP_ND6_DYNAMIC_TIMER
void nd6_tmr_init(void);
void nd6_tmr(void *arg);
#else
void nd6_tmr(void);
#endif 
void nd6_input(struct pbuf *p, struct netif *inp);
void nd6_clear_destination_cache(void);
struct netif *nd6_find_route(const ip6_addr_t *ip6addr);
err_t nd6_get_next_hop_addr_or_queue(struct netif *netif, struct pbuf *q, const ip6_addr_t *ip6addr, const u8_t **hwaddrp);
u16_t nd6_get_destination_mtu(const ip6_addr_t *ip6addr, struct netif *netif);
#if LWIP_ND6_TCP_REACHABILITY_HINTS
void nd6_reachability_hint(const ip6_addr_t *ip6addr);
#endif /* LWIP_ND6_TCP_REACHABILITY_HINTS */
void nd6_cleanup_netif(struct netif *netif);
#if LWIP_IPV6_MLD
void nd6_adjust_mld_membership(struct netif *netif, s8_t addr_idx, u8_t new_state);
#endif /* LWIP_IPV6_MLD */
void nd6_restart_netif(struct netif *netif);

#ifdef __cplusplus
}
#endif

#endif /* LWIP_IPV6 */

#endif /* LWIP_HDR_ND6_H */
