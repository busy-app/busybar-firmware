#include "discovery.h"

#include <device_name/device_name.h>
#include <lwip/tcpip.h>
#include <lwip/apps/mdns.h>
#include <m-array.h>

#include <network/network.h>
#include <wifi/wifi.h>
#include <usb_network/usb_network.h>

#define TAG "Discovery"

// =====
// Types
// =====

typedef struct {
    struct netif* netif;
} DiscoveryInterface;

typedef struct {
    DiscoveryInfo info;
    void* context;
} DiscoveryService;

ARRAY_DEF(DiscoveryServices, DiscoveryService, M_POD_OPLIST);
#define M_OPL_DiscoveryServices_t() ARRAY_OPLIST(DiscoveryServices, M_POD_OPLIST)

struct Discovery {
    FuriMutex* mutex;
    DiscoveryInterface interfaces[NetworkNetifCount];
    DiscoveryServices_t services;

    DeviceName* device_name;

    Wifi* wifi;
    WifiState wifi_state;
    UsbNetwork* usb_network;
};

struct DiscoveryRequest {
    Discovery* discovery;
    struct mdns_service* service;
};

// ==============
// Internal logic
// ==============

/**
 * Context:
 * - Discovery: unlocked
 */
static void discovery_lock(Discovery* discovery) {
    furi_assert(discovery);
    furi_check(furi_mutex_acquire(discovery->mutex, FuriWaitForever) == FuriStatusOk);
}

/**
 * Context:
 * - Discovery: locked
 */
static void discovery_unlock(Discovery* discovery) {
    furi_assert(discovery);
    furi_check(furi_mutex_release(discovery->mutex) == FuriStatusOk);
}

/**
 * No context requirements
 */
static enum mdns_sd_proto discovery_transport_to_lwip(DiscoveryTransport transport) {
    if(transport == DiscoveryTransportTcp) {
        return DNSSD_PROTO_TCP;
    } else if(transport == DiscoveryTransportUdp) {
        return DNSSD_PROTO_UDP;
    } else {
        furi_crash();
    }
}

/**
 * No context requirements
 */
static const char* discovery_device_name_to_hostname(const char* dev_name, FuriString* buffer) {
    furi_assert(buffer);

    furi_string_reset(buffer);

    char c;
    while((c = *(dev_name++))) {
        if(isalnum(c)) {
            furi_string_push_back(buffer, tolower(c));
        }
    }

    if(furi_string_empty(buffer)) {
        if(strcmp(dev_name, DEVICE_NAME_DEFAULT) == 0) {
            furi_crash("Default device name has no alphanumeric characters");
        }
        return discovery_device_name_to_hostname(DEVICE_NAME_DEFAULT, buffer);
    }

    return furi_string_get_cstr(buffer);
}

/**
 * Context:
 * - Discovery: locked
 * - lwIP: locked
 */
static void discovery_txt_adapter(struct mdns_service* lwip_srv, void* context) {
    furi_assert(lwip_srv);
    furi_assert(context);

    DiscoveryService* service = context;
    DiscoveryRequest request = {
        .service = lwip_srv,
    };

    if(service->info.txt) {
        service->info.txt(&request, service->context);
    }
}

/**
 * Context:
 * - Discovery: locked
 * - lwIP: locked
 */
static void
    discovery_bind_service(const DiscoveryInterface* interface, DiscoveryService* service) {
    furi_assert(interface);
    furi_assert(service);

    const DiscoveryInfo* info = &service->info;
    mdns_resp_add_service(
        interface->netif,
        info->name,
        info->service,
        discovery_transport_to_lwip(info->transport),
        info->port,
        discovery_txt_adapter,
        service);

    FURI_LOG_D(
        TAG,
        "Bound '%s' to netif '%c%c'",
        service->info.name,
        interface->netif->name[0],
        interface->netif->name[1]);
}

/**
 * Context:
 * - Discovery: unlocked
 * - lwIP: unlocked
 */
static void discovery_device_name_event(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const DeviceNameEvent* event = message;
    Discovery* discovery = context;

    if(event->type != DeviceNameEventTypeNameChanged) return;

    discovery_lock(discovery);

    FuriString* hostname_furi = furi_string_alloc();
    const char* dev_name = event->name_changed.name;
    const char* hostname = discovery_device_name_to_hostname(dev_name, hostname_furi);

    LOCK_TCPIP_CORE();

    for(size_t i = 0; i < COUNT_OF(discovery->interfaces); i++) {
        DiscoveryInterface* interface = &discovery->interfaces[i];
        struct netif* netif = interface->netif;
        if(!netif) continue;

        mdns_resp_rename_netif(netif, hostname);
        mdns_resp_announce(netif);

        FURI_LOG_D(
            TAG,
            "Renamed netif '%c%c' to '%s' & re-announced",
            netif->name[0],
            netif->name[1],
            hostname);
    }

    UNLOCK_TCPIP_CORE();

    furi_string_free(hostname_furi);
    discovery_unlock(discovery);
}

/**
 * Context:
 * - Discovery: unlocked
 * - lwIP: unlocked
 */
static void discovery_netif_up(Discovery* discovery, NetworkNetif netif_id) {
    furi_check(discovery);

    LOCK_TCPIP_CORE();
    struct netif* netif = network_find_netif(netif_id);
    FURI_LOG_D(TAG, "Network up: netif '%c%c'", netif->name[0], netif->name[1]);
    UNLOCK_TCPIP_CORE();

    discovery_lock(discovery);

    DiscoveryInterface* interface = &discovery->interfaces[netif_id];

    if(!interface->netif) {
        interface->netif = netif;

        FuriString* hostname_furi = furi_string_alloc();
        FuriString* dev_name = furi_string_alloc();
        device_name_get(discovery->device_name, dev_name);
        const char* hostname =
            discovery_device_name_to_hostname(furi_string_get_cstr(dev_name), hostname_furi);
        furi_string_free(dev_name);

        LOCK_TCPIP_CORE();

        mdns_resp_add_netif(netif, hostname);
        FURI_LOG_D(
            TAG, "Added netif '%c%c' with name '%s'", netif->name[0], netif->name[1], hostname);

        for
            M_EACH(service, discovery->services, DiscoveryServices_t) {
                discovery_bind_service(interface, service);
            }

        UNLOCK_TCPIP_CORE();
        furi_string_free(hostname_furi);
    }

    LOCK_TCPIP_CORE();
    mdns_resp_announce(netif);
    UNLOCK_TCPIP_CORE();

    FURI_LOG_D(TAG, "Announced to netif '%c%c'", netif->name[0], netif->name[1]);

    discovery_unlock(discovery);
}

// =======================
// Network driver adapters
// =======================

static void discovery_wifi_event(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);
    const WifiInfo* info = item;
    Discovery* discovery = context;

    // `Wifi` spams `WifiStateConnected` every 30s. we don't want to flood the network.
    if((info->state == WifiStateConnected) && (discovery->wifi_state != WifiStateConnected)) {
        discovery_netif_up(discovery, NetworkNetifWifi);
    }

    discovery->wifi_state = info->state;
}

static void discovery_usb_network_event(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);
    const UsbNetworkInfo* info = item;
    Discovery* discovery = context;

    if(info->state == UsbNetworkStateUp) {
        discovery_netif_up(discovery, NetworkNetifUsb);
    }
}

static void discovery_subscribe_to_network_drivers(Discovery* discovery) {
    furi_assert(discovery);

    discovery->wifi = furi_record_open(RECORD_WIFI);
    discovery->usb_network = furi_record_open(RECORD_USB_NETWORK);

    furi_state_subscribe(wifi_get_state(discovery->wifi), discovery_wifi_event, discovery);
    furi_state_subscribe(
        usb_network_get_state(discovery->usb_network), discovery_usb_network_event, discovery);
}

// ===============
// Service startup
// ===============

static Discovery* discovery_alloc(void) {
    Discovery* discovery = malloc(sizeof(Discovery));

    discovery->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    DiscoveryServices_init(discovery->services);

    discovery->device_name = furi_record_open(RECORD_DEVICE_NAME);
    furi_pubsub_subscribe(
        device_name_get_pubsub(discovery->device_name), discovery_device_name_event, discovery);

    discovery_subscribe_to_network_drivers(discovery);

    return discovery;
}

void discovery_on_system_start(void) {
    LOCK_TCPIP_CORE();
    mdns_resp_init();
    UNLOCK_TCPIP_CORE();

    Discovery* discovery = discovery_alloc();
    furi_record_create(RECORD_DISCOVERY, discovery);

    FURI_LOG_D(TAG, "Started");
}

// =======================
// Public API for services
// =======================

void discovery_service_add(Discovery* discovery, const DiscoveryInfo* info, void* context) {
    furi_check(discovery);
    furi_check(info);
    furi_check(info->name);
    furi_check(info->service);

    FURI_LOG_I(TAG, "Service added: '%s'", info->name);

    discovery_lock(discovery);

    DiscoveryService* service = DiscoveryServices_push_new(discovery->services);
    service->info = *info;
    service->context = context;

    LOCK_TCPIP_CORE();

    for(size_t i = 0; i < COUNT_OF(discovery->interfaces); i++) {
        DiscoveryInterface* interface = &discovery->interfaces[i];
        if(!interface->netif) continue;

        discovery_bind_service(interface, service);
    }

    UNLOCK_TCPIP_CORE();

    discovery_unlock(discovery);
}

void discovery_request_feed_txt(DiscoveryRequest* request, const char* txt) {
    furi_check(request);
    furi_check(txt);

    mdns_resp_add_service_txtitem(request->service, txt, strlen(txt));
}
