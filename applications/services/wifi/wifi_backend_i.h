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
    IntercomChHandle* intercom_main;
    IntercomChHandle* intercom_data;
    FuriSemaphore* tcpip_lock;
    FuriSemaphore* ip6_addr_valid;
    struct netif netif;
    WifiRequest request;
    WifiResponse response;
    WifiState state;
};

void wifi_net_tcpip_init(Wifi* instance);
