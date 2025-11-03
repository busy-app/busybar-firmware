#pragma once

#include "wifi_common_i.h"

#include <lwip/netif.h>

#include <sl_ieee802_types.h>

#include <furi.h>
#include <intercom/intercom.h>
#include <network/network.h>

#define TAG "Wifi"

struct Wifi {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriEventLoopTimer* info_timer;
    FuriPubSub* event_pubsub;
    Intercom* intercom;
    FuriSemaphore* tcpip_lock;
    FuriSemaphore* ip6_addr_valid;
    struct netif netif;
    WifiBackendState state;
};

void wifi_net_tcpip_init(Wifi* instance, sl_mac_address_t* mac_addr);

bool wifi_net_tcpip_netif_up(Wifi* instance);

void wifi_net_tcpip_netif_down(Wifi* instance);
