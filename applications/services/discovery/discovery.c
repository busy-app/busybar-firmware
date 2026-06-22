#include "discovery.h"

#include <device_name/device_name.h>
#include <lwip/tcpip.h>
#include <lwip/apps/mdns.h>
#include <m-array.h>

#define TAG "Discovery"

// =====
// Types
// =====

typedef struct {
    Discovery* discovery;
    struct netif* netif;
} DiscoveryInterface;

ARRAY_DEF(DiscoveryInterfaces, DiscoveryInterface, M_POD_OPLIST);
#define M_OPL_DiscoveryInterfaces_t() ARRAY_OPLIST(DiscoveryInterfaces, M_POD_OPLIST)

typedef struct {
    Discovery* discovery;
    DiscoveryInfo info;
    void* context;
} DiscoveryService;

ARRAY_DEF(DiscoveryServices, DiscoveryService, M_POD_OPLIST);
#define M_OPL_DiscoveryServices_t() ARRAY_OPLIST(DiscoveryServices, M_POD_OPLIST)

struct Discovery {
    FuriMutex* mutex;
    DiscoveryInterfaces_t interfaces;
    DiscoveryServices_t services;

    DeviceName* device_name;
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

    return furi_string_get_cstr(buffer);
}

/**
 * Context:
 * - Discovery: locked
 */
static DiscoveryInterface* discovery_get_interface(Discovery* discovery, struct netif* netif) {
    furi_assert(discovery);
    furi_assert(netif);

    DiscoveryInterface* result = NULL;

    for
        M_EACH(interface, discovery->interfaces, DiscoveryInterfaces_t) {
            if(interface->netif == netif) {
                result = interface;
                break;
            }
        }

    return result;
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
        .discovery = service->discovery,
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

    FURI_LOG_D(TAG, "Bound '%s' to netif %p", service->info.name, interface->netif);
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

    for
        M_EACH(interface, discovery->interfaces, DiscoveryInterfaces_t) {
            struct netif* netif = interface->netif;

            mdns_resp_rename_netif(netif, hostname);
            mdns_resp_announce(netif);

            FURI_LOG_D(TAG, "Renamed netif %p to '%s'", netif, hostname);
            FURI_LOG_D(TAG, "Announced to netif %p", netif);
        }

    UNLOCK_TCPIP_CORE();

    furi_string_free(hostname_furi);
    discovery_unlock(discovery);
}

// ===============
// Service startup
// ===============

static Discovery* discovery_alloc(void) {
    Discovery* discovery = malloc(sizeof(Discovery));

    discovery->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    DiscoveryInterfaces_init(discovery->interfaces);
    DiscoveryServices_init(discovery->services);

    discovery->device_name = furi_record_open(RECORD_DEVICE_NAME);
    furi_pubsub_subscribe(
        device_name_get_pubsub(discovery->device_name), discovery_device_name_event, discovery);

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
    service->discovery = discovery;
    service->info = *info;
    service->context = context;

    LOCK_TCPIP_CORE();
    for
        M_EACH(interface, discovery->interfaces, DiscoveryInterfaces_t) {
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

// ==============================
// Public API for network drivers
// ==============================

void discovery_netif_up(Discovery* discovery, struct netif* netif) {
    furi_check(discovery);
    furi_check(netif);

    FURI_LOG_D(TAG, "Network up: netif %p", netif);

    discovery_lock(discovery);

    DiscoveryInterface* interface = discovery_get_interface(discovery, netif);

    if(!interface) {
        interface = DiscoveryInterfaces_push_new(discovery->interfaces);
        interface->discovery = discovery;
        interface->netif = netif;

        FuriString* hostname_furi = furi_string_alloc();
        FuriString* dev_name = furi_string_alloc();
        device_name_get(discovery->device_name, dev_name);
        const char* hostname =
            discovery_device_name_to_hostname(furi_string_get_cstr(dev_name), hostname_furi);
        furi_string_free(dev_name);

        LOCK_TCPIP_CORE();

        mdns_resp_add_netif(netif, hostname);
        FURI_LOG_D(TAG, "Added netif %p with name '%s'", netif, hostname);

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

    FURI_LOG_D(TAG, "Announced to netif %p", netif);

    discovery_unlock(discovery);
}
