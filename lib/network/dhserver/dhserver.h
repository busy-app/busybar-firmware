/*
 * Copyright (C) 2015 by Sergey Fetisov <fsenok@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * version: 1.0 demo (7.02.2015)
 * brief:   tiny dhcp ipv4 server using lwip (pcb)
 * ref:     https://lists.gnu.org/archive/html/lwip-users/2012-12/msg00016.html
 */
#pragma once

#include <lwip/netif.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    struct netif* netif;
    ip4_addr_t router;
    uint16_t port;
    ip4_addr_t dns;
    const char* domain;
    uint8_t max_lease_count;
} DhcpServerConfig;

bool dhserv_init(const DhcpServerConfig* config);

void dhserv_deinit(void);

bool dhserv_has_lease(ip4_addr_t addr);

#ifdef __cplusplus
}
#endif
