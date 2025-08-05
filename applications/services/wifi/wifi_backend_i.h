#pragma once

#include "wifi_common_i.h"

#include <lwip/netif.h>

#include <furi.h>
#include <intercom/intercom.h>

#define TAG "Wifi"

struct Wifi {
    FuriEventLoop* event_loop;
    FuriPubSub* event_pubsub;
    Intercom* intercom;
    struct netif netif;
    WifiRequest request;
    WifiResponse response;
    WifiState state;
};

void wifi_net_lwip_init(Wifi* instance);
