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
    IntercomChannel* intercom_ch_control;
    IntercomChannel* intercom_ch_data;
    FuriSemaphore* tcpip_lock;
    FuriSemaphore* ip6_addr_valid;
    struct netif netif;
    WifiRequest request;
    WifiResponse response;
    WifiBackendState state;
};

void wifi_net_tcpip_init(Wifi* instance);
