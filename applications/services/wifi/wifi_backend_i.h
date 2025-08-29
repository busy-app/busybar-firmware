#pragma once

#include "wifi_common_i.h"

#include <lwip/netif.h>

#include <furi.h>
#include <intercom/intercom.h>
#include <network/network.h>

#define TAG "Wifi"

struct Wifi {
    FuriEventLoop* event_loop;
    FuriPubSub* event_pubsub;
    Intercom* intercom;
    FuriSemaphore* tcpip_lock;
    struct netif netif;
    WifiRequest request;
    WifiResponse response;
    WifiState state;
};

void wifi_net_tcpip_init(Wifi* instance);
