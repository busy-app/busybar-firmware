#include "mongoose_dns.h"

#include <furi_hal_memory.h>

#include <wifi/wifi.h>

#define TAG "MongooseDns"

#if defined(MONGOOSE_DNS_DEBUG)
#define MONGOOSE_DNS_TRACE(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define MONGOOSE_DNS_TRACE(...)
#endif

#define DEFAULT_DNS_SERVER "udp://8.8.8.8:53"

// ====================
// `MongooseDns` object
// ====================

#define RECORD_MONGOOSE_DNS "mongoose_dns"

typedef struct {
    _Atomic uint32_t address;
} MongooseDns;

static void mongoose_dns_wifi_event(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    const WifiInfo* info = item;
    MongooseDns* dns = context;

    dns->address = info->ip_config.ip4.dns.value;
}

void mongoose_dns_startup(void) {
    MongooseDns* dns = malloc(sizeof(MongooseDns));

    Wifi* wifi = furi_record_open(RECORD_WIFI);
    furi_state_subscribe(wifi_get_state(wifi), mongoose_dns_wifi_event, dns);

    furi_record_create(RECORD_MONGOOSE_DNS, dns);
}

// =====================
// `mg_mgr` manipulation
// =====================

static void mongoose_dns_free_url(struct mg_mgr* mgr) {
    furi_assert(mgr);

    if(furi_hal_memory_is_address_in_region(mgr->dns4.url, FuriHalMemoryRegionIdHeap)) {
        free((void*)mgr->dns4.url);
        mgr->dns4.url = DEFAULT_DNS_SERVER;
    }
}

static void mongoose_dns_apply(struct mg_mgr* mgr, uint32_t address) {
    furi_assert(mgr);

    mongoose_dns_free_url(mgr);

    if(address) {
        char new_url[64];
        const uint8_t* addr_array = (const uint8_t*)&address;

        snprintf(
            new_url,
            sizeof(new_url),
            "udp://" WIFI_IP4_ADDR_FORMAT ":53",
            addr_array[0],
            addr_array[1],
            addr_array[2],
            addr_array[3]);

        mgr->dns4.url = strdup(new_url);

    } else {
        mgr->dns4.url = strdup(DEFAULT_DNS_SERVER);
    }

    // on any next query, the DNS structure will be re-initialized
    // we have no way of doing that ourselves (`mg_dnsc_init` is private)

    if(mgr->dns4.c) mg_close_conn(mgr->dns4.c);
    mgr->dns4.c = NULL;

    MONGOOSE_DNS_TRACE("Applied DNS %s to mg_mgr 0x%p", mgr->dns4.url, mgr);
}

// ==========
// Public API
// ==========

void mongoose_dns_init(struct mg_mgr* mgr) {
    furi_check(mgr);

    MongooseDns* dns = furi_record_open(RECORD_MONGOOSE_DNS);
    mongoose_dns_apply(mgr, dns->address);
    furi_record_close(RECORD_MONGOOSE_DNS);
}

void mongoose_dns_deinit(struct mg_mgr* mgr) {
    furi_check(mgr);

    mongoose_dns_free_url(mgr);
}
