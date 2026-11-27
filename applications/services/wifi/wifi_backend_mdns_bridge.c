// Matter advertises over IPv6 only, and some access points do not forward ff02::fb between
// wireless stations. IPv4 mDNS queries are therefore also fed into the IPv6 stack, and its
// multicast responses are mirrored to IPv4. The U5 owns IPv4, so its address is learned from
// the frames it sends.

#include "wifi_backend_i.h"

#include <lwip/pbuf.h>
#include <lwip/tcpip.h>
#include <lwip/prot/ethernet.h>

#include <sl_wifi.h>

#define WIFI_MDNS_BRIDGE_SOURCE_REFRESH_MS (10000UL)

typedef struct {
    Wifi* instance;
    MdnsBridgeIpv4Address addr;
} WifiMdnsBridgeSourceMessage;

static void wifi_mdns_bridge_set_source_callback(void* context) {
    WifiMdnsBridgeSourceMessage* message = context;
    furi_assert(message);

    WifiMdnsBridge* bridge = &message->instance->mdns_bridge;
    const bool is_changed =
        !bridge->has_source_addr ||
        (memcmp(&bridge->source_addr, &message->addr, sizeof(MdnsBridgeIpv4Address)) != 0);

    if(is_changed) {
        const uint8_t* octets = message->addr.octets;
        FURI_LOG_I(
            TAG,
            "mDNS bridge IPv4 source: %u.%u.%u.%u",
            octets[0],
            octets[1],
            octets[2],
            octets[3]);
    }

    bridge->source_addr = message->addr;
    bridge->has_source_addr = true;

    free(message);
}

void wifi_mdns_bridge_track_ipv4_source(Wifi* instance, const uint8_t* frame, size_t frame_size) {
    furi_assert(instance);
    furi_assert(frame);

    WifiMdnsBridge* bridge = &instance->mdns_bridge;
    MdnsBridgeIpv4Address addr;

    do {
        if(!mdns_bridge_get_ipv4_source(frame, frame_size, &addr)) {
            break;
        }

        const uint32_t now = furi_get_tick();
        const bool is_changed = (memcmp(&addr, &bridge->seen_addr, sizeof(addr)) != 0);
        const bool is_refresh_due = (now - bridge->seen_tick) >=
                                    furi_ms_to_ticks(WIFI_MDNS_BRIDGE_SOURCE_REFRESH_MS);

        if(!is_changed && !is_refresh_due) {
            break;
        }

        WifiMdnsBridgeSourceMessage* message = malloc(sizeof(WifiMdnsBridgeSourceMessage));
        message->instance = instance;
        message->addr = addr;

        // Not updating the seen state on failure makes the next frame retry the update
        if(tcpip_try_callback(wifi_mdns_bridge_set_source_callback, message) != ERR_OK) {
            free(message);
            break;
        }

        bridge->seen_addr = addr;
        bridge->seen_tick = now;
    } while(false);
}

void wifi_mdns_bridge_forward_query(Wifi* instance, const uint8_t* frame, size_t frame_size) {
    furi_assert(instance);
    furi_assert(frame);

    struct netif* netif = &instance->netif;

    do {
        const size_t query_size = mdns_bridge_ipv4_query_to_ipv6(frame, frame_size, NULL, 0);
        if(!query_size) {
            break;
        }

        const uint8_t* src_mac = &frame[ETH_HWADDR_LEN];
        if(memcmp(src_mac, netif->hwaddr, ETH_HWADDR_LEN) == 0) {
            break;
        }

        struct pbuf* p = pbuf_alloc(PBUF_RAW, (u16_t)query_size, PBUF_RAM);
        if(!p) {
            FURI_LOG_W(TAG, "mDNS bridge: failed to allocate %u bytes for query", query_size);
            break;
        }

        furi_check(
            mdns_bridge_ipv4_query_to_ipv6(frame, frame_size, p->payload, p->len) == query_size);

        if(netif->input(p, netif) != ERR_OK) {
            FURI_LOG_W(TAG, "mDNS bridge: IPv6 stack rejected query");
            pbuf_free(p);
        }
    } while(false);
}

void wifi_mdns_bridge_mirror_response(Wifi* instance, const uint8_t* frame, size_t frame_size) {
    furi_assert(instance);
    furi_assert(frame);

    const WifiMdnsBridge* bridge = &instance->mdns_bridge;

    do {
        if(!bridge->has_source_addr) {
            break;
        }

        const size_t response_size =
            mdns_bridge_ipv6_response_to_ipv4(frame, frame_size, &bridge->source_addr, NULL, 0);
        if(!response_size) {
            break;
        }

        uint8_t* response = malloc(response_size);
        furi_check(
            mdns_bridge_ipv6_response_to_ipv4(
                frame, frame_size, &bridge->source_addr, response, response_size) ==
            response_size);

        const sl_status_t status =
            sl_wifi_send_raw_data_frame(SL_WIFI_CLIENT_INTERFACE, response, response_size);
        if(status != SL_STATUS_OK) {
            FURI_LOG_W(TAG, "mDNS bridge: failed to send IPv4 response: 0x%lX", status);
        }

        free(response);
    } while(false);
}

void wifi_mdns_bridge_reset(Wifi* instance) {
    furi_assert(instance);
    instance->mdns_bridge.has_source_addr = false;
}
