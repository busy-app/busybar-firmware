#include "discovery.h"

#include <m-array.h>

#include <lwip/tcpip.h>
#include <lwip/apps/mdns.h>

#include <device_name/device_name.h>
#include <network/network.h>
#include <wifi/wifi.h>
#include <usb_network/usb_network.h>

#define TAG "Discovery"

#define DISCOVERY_API_TIMEOUT_TICKS (1000)
#define DISCOVERY_API_QUEUE_SIZE    (8)

// =====
// Types
// =====

typedef struct {
    const DiscoveryServiceInfo* info;
    void* context;
} DiscoveryService;

ARRAY_DEF(DiscoveryServiceArray, DiscoveryService*, M_PTR_OPLIST)
#define M_OPL_DiscoveryServiceArray_t() ARRAY_OPLIST(DiscoveryServiceArray, M_POD_OPLIST)

typedef enum {
    DiscoveryApiMessageTypeAddService,
    DiscoveryApiMessageTypeDeviceName,
    DiscoveryApiMessageTypeUsbNetwork,
    DiscoveryApiMessageTypeWifiNetwork,
    DiscoveryApiMessageTypeMax,
} DiscoveryApiMessageType;

typedef struct {
    DiscoveryApiMessageType type;
    union {
        DiscoveryService service_to_add;
        DeviceNameInfo device_name_info;
        UsbNetworkState usb_network_state;
        WifiState wifi_network_state;
    };
} DiscoveryApiMessage;

struct Discovery {
    FuriEventLoop* event_loop;
    FuriMessageQueue* api_queue;
    struct netif* netifs[NetworkNetifCount];
    FuriString* device_name;
    DiscoveryServiceArray_t services;
    WifiState wifi_state;
};

typedef void (
    *DiscoveryApiMessageHandler)(Discovery* discovery, const DiscoveryApiMessage* api_message);

// ==============
// Internal logic
// ==============

static bool
    discovery_send_api_message(Discovery* discovery, const DiscoveryApiMessage* api_message) {
    bool success = true;

    const FuriStatus status =
        furi_message_queue_put(discovery->api_queue, api_message, DISCOVERY_API_TIMEOUT_TICKS);

    if(status != FuriStatusOk) {
        furi_check(status == FuriStatusErrorTimeout);
        success = false;
    }

    return success;
}

static enum mdns_sd_proto discovery_transport_to_lwip(DiscoveryTransportType transport) {
    if(transport == DiscoveryTransportTypeTcp) {
        return DNSSD_PROTO_TCP;
    } else if(transport == DiscoveryTransportTypeUdp) {
        return DNSSD_PROTO_UDP;
    } else {
        furi_crash();
    }
}

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

static void discovery_mdns_txt_callback(struct mdns_service* lwip_srv, void* context) {
    furi_assert(lwip_srv);
    furi_assert(context);
    LWIP_ASSERT_CORE_LOCKED();

    DiscoveryService* service = context;
    const DiscoveryServiceInfo* info = service->info;

    if(!info->txt_callback) return;

    size_t index = 0;
    bool record_valid = true;

    while(record_valid) {
        FuriString* txt = furi_string_alloc();

        record_valid = info->txt_callback(index, txt, service->context);
        if(record_valid) {
            mdns_resp_add_service_txtitem(
                lwip_srv, furi_string_get_cstr(txt), furi_string_size(txt));
        }

        furi_string_free(txt);
        index++;
    }
}

static void discovery_bind_service_to_netif(DiscoveryService* service, struct netif* netif) {
    furi_assert(netif);
    furi_assert(service);
    LWIP_ASSERT_CORE_LOCKED();

    const DiscoveryServiceInfo* info = service->info;
    mdns_resp_add_service(
        netif,
        info->name,
        info->service,
        discovery_transport_to_lwip(info->transport_type),
        info->port,
        discovery_mdns_txt_callback,
        service);

    FURI_LOG_D(
        TAG, "Bound '%s' to netif '%c%c'", service->info->name, netif->name[0], netif->name[1]);
}

static void discovery_device_name_state_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    const DeviceNameInfo* state = item;
    Discovery* discovery = context;

    const DiscoveryApiMessage api_message = {
        .type = DiscoveryApiMessageTypeDeviceName,
        .device_name_info = *state,
    };

    if(!discovery_send_api_message(discovery, &api_message)) {
        FURI_LOG_W(TAG, "Device name change dropped");
    }
}

static void discovery_netif_up(Discovery* discovery, NetworkNetif netif_id) {
    furi_check(discovery);

    LOCK_TCPIP_CORE();
    struct netif* netif = network_find_netif(netif_id);
    FURI_LOG_D(TAG, "Network up: netif '%c%c'", netif->name[0], netif->name[1]);
    UNLOCK_TCPIP_CORE();

    if(discovery->netifs[netif_id] == NULL) {
        discovery->netifs[netif_id] = netif;

        FuriString* hostname_buf = furi_string_alloc();
        const char* hostname = discovery_device_name_to_hostname(
            furi_string_get_cstr(discovery->device_name), hostname_buf);

        LOCK_TCPIP_CORE();

        mdns_resp_add_netif(netif, hostname);
        FURI_LOG_D(
            TAG, "Added netif '%c%c' with name '%s'", netif->name[0], netif->name[1], hostname);

        /* clang-format off */
        for M_EACH(service, discovery->services, DiscoveryServiceArray_t) {
            discovery_bind_service_to_netif(*service, netif);
        }
        /* clang-format on */

        UNLOCK_TCPIP_CORE();
        furi_string_free(hostname_buf);
    }

    LOCK_TCPIP_CORE();
    mdns_resp_announce(netif);
    UNLOCK_TCPIP_CORE();

    FURI_LOG_D(TAG, "Announced to netif '%c%c'", netif->name[0], netif->name[1]);
}

static void
    discovery_add_service_handler(Discovery* discovery, const DiscoveryApiMessage* api_message) {
    DiscoveryService* service = malloc(sizeof(DiscoveryService));
    *service = api_message->service_to_add;

    DiscoveryServiceArray_push_back(discovery->services, service);

    LOCK_TCPIP_CORE();

    for(size_t i = 0; i < COUNT_OF(discovery->netifs); i++) {
        struct netif* netif = discovery->netifs[i];

        if(netif != NULL) {
            discovery_bind_service_to_netif(service, netif);
        }
    }

    UNLOCK_TCPIP_CORE();

    FURI_LOG_I(TAG, "Service added: '%s'", service->info->name);
}

static void
    discovery_device_name_handler(Discovery* discovery, const DiscoveryApiMessage* api_message) {
    const char* new_device_name = api_message->device_name_info.name;

    if(furi_string_equal(discovery->device_name, new_device_name)) {
        return;
    }

    FuriString* hostname_buf = furi_string_alloc();
    const char* hostname = discovery_device_name_to_hostname(new_device_name, hostname_buf);

    LOCK_TCPIP_CORE();

    furi_string_set(discovery->device_name, new_device_name);

    for(size_t i = 0; i < COUNT_OF(discovery->netifs); i++) {
        struct netif* netif = discovery->netifs[i];
        if(netif == NULL) {
            continue;
        }

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

    furi_string_free(hostname_buf);
}

static void
    discovery_usb_network_handler(Discovery* discovery, const DiscoveryApiMessage* api_message) {
    if(api_message->usb_network_state == UsbNetworkStateUp) {
        discovery_netif_up(discovery, NetworkNetifUsb);
    }
}

static void
    discovery_wifi_network_handler(Discovery* discovery, const DiscoveryApiMessage* api_message) {
    const WifiState wifi_state = api_message->wifi_network_state;
    /* Restrict mdns events to Wifi state changes only. */
    if(discovery->wifi_state != wifi_state) {
        if(wifi_state == WifiStateConnected) {
            discovery_netif_up(discovery, NetworkNetifWifi);
        }
        discovery->wifi_state = wifi_state;
    }
}

static const DiscoveryApiMessageHandler discovery_api_handlers[DiscoveryApiMessageTypeMax] = {
    [DiscoveryApiMessageTypeAddService] = discovery_add_service_handler,
    [DiscoveryApiMessageTypeDeviceName] = discovery_device_name_handler,
    [DiscoveryApiMessageTypeUsbNetwork] = discovery_usb_network_handler,
    [DiscoveryApiMessageTypeWifiNetwork] = discovery_wifi_network_handler,
};

static void discovery_api_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    Discovery* discovery = context;

    furi_assert(object == discovery->api_queue);
    FuriMessageQueue* api_queue = object;

    DiscoveryApiMessage api_message;
    while(furi_message_queue_get(api_queue, &api_message, 0) == FuriStatusOk) {
        furi_check(api_message.type < DiscoveryApiMessageTypeMax);
        discovery_api_handlers[api_message.type](discovery, &api_message);
    }
}

// =======================
// Network driver adapters
// =======================

static void discovery_wifi_state_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);
    const WifiInfo* info = item;
    Discovery* discovery = context;

    const DiscoveryApiMessage api_message = {
        .type = DiscoveryApiMessageTypeWifiNetwork,
        .wifi_network_state = info->state,
    };

    if(!discovery_send_api_message(discovery, &api_message)) {
        FURI_LOG_W(TAG, "Wireless network state dropped");
    }
}

static void discovery_usb_network_state_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);
    const UsbNetworkInfo* info = item;
    Discovery* discovery = context;

    const DiscoveryApiMessage api_message = {
        .type = DiscoveryApiMessageTypeUsbNetwork,
        .usb_network_state = info->state,
    };

    if(!discovery_send_api_message(discovery, &api_message)) {
        FURI_LOG_W(TAG, "USB network state dropped");
    }
}

static void discovery_init_mdns(Discovery* discovery) {
    UNUSED(discovery);

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);

    LOCK_TCPIP_CORE();
    mdns_resp_init();
    UNLOCK_TCPIP_CORE();
}

static void discovery_subscribe_to_device_name(Discovery* discovery) {
    DeviceName* device_name = furi_record_open(RECORD_DEVICE_NAME);
    furi_state_subscribe(
        device_name_get_state(device_name), discovery_device_name_state_callback, discovery);
}

static void discovery_subscribe_to_network_state(Discovery* discovery) {
    furi_assert(discovery);

    Wifi* wifi = furi_record_open(RECORD_WIFI);
    furi_state_subscribe(wifi_get_state(wifi), discovery_wifi_state_callback, discovery);

    UsbNetwork* usb_network = furi_record_open(RECORD_USB_NETWORK);
    furi_state_subscribe(
        usb_network_get_state(usb_network), discovery_usb_network_state_callback, discovery);
}

// ===============
// Service startup
// ===============

static Discovery* discovery_alloc(void) {
    Discovery* discovery = malloc(sizeof(Discovery));

    discovery->event_loop = furi_event_loop_alloc();
    discovery->api_queue =
        furi_message_queue_alloc(DISCOVERY_API_QUEUE_SIZE, sizeof(DiscoveryApiMessage));
    discovery->device_name = furi_string_alloc();

    DiscoveryServiceArray_init(discovery->services);

    furi_event_loop_subscribe_message_queue(
        discovery->event_loop,
        discovery->api_queue,
        FuriEventLoopEventIn,
        discovery_api_message_queue_callback,
        discovery);

    discovery_init_mdns(discovery);

    discovery_subscribe_to_device_name(discovery);
    discovery_subscribe_to_network_state(discovery);

    furi_record_create(RECORD_DISCOVERY, discovery);

    return discovery;
}

int32_t discovery_srv(void* arg) {
    UNUSED(arg);

    Discovery* discovery = discovery_alloc();
    furi_event_loop_run(discovery->event_loop);

    return 0;
}

// =======================
// Public API for services
// =======================

bool discovery_add_service(Discovery* discovery, const DiscoveryServiceInfo* info, void* context) {
    furi_check(discovery);
    furi_check(info);
    furi_check(info->name);
    furi_check(info->service);

    /* clang-format off */
    const DiscoveryApiMessage api_message = {
        .type = DiscoveryApiMessageTypeAddService,
        .service_to_add = {
            .info = info,
            .context = context,
        },
    };
    /* clang-format on */

    return discovery_send_api_message(discovery, &api_message);
}
