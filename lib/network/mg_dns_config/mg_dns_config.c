#include "mg_dns_config.h"

#include <wifi/wifi.h>

#define TAG "MgDnsConfig"

// Exactly as defined in `lib/mongoose/src/net.c`
#define DEFAULT_DNS_SERVER "udp://8.8.8.8:53"

// ==================
// Internal functions
// ==================

static void
    mg_dns_config_determine_url(const WifiInfo* wifi_info, char* out_buf, size_t buf_size) {
    furi_assert(out_buf);

    bool success = false;

    do {
        const WifiIpv4* dns_server = &wifi_info->ip_config.ip4.dns;
        if(dns_server->value == 0) break;

        snprintf(
            out_buf,
            buf_size,
            "udp://" WIFI_IP4_ADDR_FORMAT ":53",
            WIFI_IP4_ADDR_SPREAD(dns_server));
        success = true;
    } while(0);

    if(!success) strncpy(out_buf, DEFAULT_DNS_SERVER, buf_size - 1);
}

static void mg_dns_config_apply(struct mg_mgr* mgr, const WifiInfo* wifi_info) {
    furi_assert(mgr);
    furi_assert(wifi_info);

    char new_url[64];
    mg_dns_config_determine_url(wifi_info, new_url, sizeof(new_url));
    char* old_url = (char*)mgr->dns4.url;

    if(strcmp(old_url, new_url) == 0) return;

    bool new_url_will_be_on_heap = strcmp(new_url, DEFAULT_DNS_SERVER) != 0;
    mgr->dns4.url = new_url_will_be_on_heap ? strdup(new_url) : DEFAULT_DNS_SERVER;

    // on any next query, the DNS structure will be re-initialized
    // we have no way of doing that ourselves (`mg_dnsc_init` is private)

    if(mgr->dns4.c) mg_close_conn(mgr->dns4.c);
    mgr->dns4.c = NULL;

    FURI_LOG_D(TAG, "Applied DNS %s to mg_mgr %p", new_url, mgr);
}

// ==========
// Public API
// ==========

void mg_dns_config_apply_auto(struct mg_mgr* mgr) {
    furi_check(mgr);

    Wifi* wifi = furi_record_open(RECORD_WIFI);

    WifiInfo wifi_info;
    if(wifi_get_info(wifi, &wifi_info) == WifiStatusOk) {
        mg_dns_config_apply(mgr, &wifi_info);
    }

    furi_record_close(RECORD_WIFI);
}

void mg_dns_config_apply_from_info(struct mg_mgr* mgr, const WifiInfo* info) {
    furi_check(mgr);
    furi_check(info);

    mg_dns_config_apply(mgr, info);
}

void mg_dns_config_cleanup(struct mg_mgr* mgr) {
    furi_check(mgr);

    char* old_url = (char*)mgr->dns4.url;

    // `old_url` might be a pointer to a string in flash, assigned in `mg_mgr_init`
    // good news though: it is only ever assigned as `DEFAULT_DNS_SERVER`

    bool old_url_is_on_heap = strcmp(old_url, DEFAULT_DNS_SERVER) != 0;
    if(old_url_is_on_heap) free(old_url);
}

// TODO: subscription to Wi-Fi service for long-running services that have an event loop?
