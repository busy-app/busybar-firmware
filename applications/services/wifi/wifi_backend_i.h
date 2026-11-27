#pragma once

#include "wifi_common_i.h"

#include <lwip/netif.h>

#include <sl_ieee802_types.h>

#include <furi.h>
#include <intercom/intercom.h>
#include <network/network.h>
#include <toolbox/mdns_bridge.h>

#define TAG "Wifi"

typedef struct {
    MdnsBridgeIpv4Address source_addr; // tcpip thread only
    bool has_source_addr; // tcpip thread only
    MdnsBridgeIpv4Address seen_addr; // data intercom channel thread only
    uint32_t seen_tick; // data intercom channel thread only
} WifiMdnsBridge;

struct Wifi {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriMessageQueue* request_queue;
    FuriEventLoopTimer* info_timer;
    FuriPubSub* event_pubsub;
    IntercomChannel* intercom_ch_control;
    IntercomChannel* intercom_ch_data;
    FuriSemaphore* tcpip_lock;
    FuriSemaphore* ipv6_ready_semaphore;
    struct netif netif;
    uint32_t tx_drop_count;
    uint32_t rx_drop_count;
    WifiMdnsBridge mdns_bridge;
    WifiBackendState state;
    bool scan_in_progress;
};

void wifi_net_tcpip_init(Wifi* instance, sl_mac_address_t* mac_addr);

void wifi_net_tcpip_netif_up(Wifi* instance);

void wifi_net_tcpip_netif_down(Wifi* instance);

// mDNS bridge

void wifi_mdns_bridge_track_ipv4_source(Wifi* instance, const uint8_t* frame, size_t frame_size);

void wifi_mdns_bridge_forward_query(Wifi* instance, const uint8_t* frame, size_t frame_size);

void wifi_mdns_bridge_mirror_response(Wifi* instance, const uint8_t* frame, size_t frame_size);

void wifi_mdns_bridge_reset(Wifi* instance);
