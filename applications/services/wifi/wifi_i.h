#pragma once

#include "wifi.h"

#include "wifi_common_i.h"
#include "wifi_settings.h"

#include <api_lock.h>

#include <lwip/netif.h>

#include <intercom/intercom.h>

#define TAG "WifiSrv"

typedef enum {
    WifiEventRequest = 1UL << 0,
    WifiEventResponse = 1UL << 1,
} WifiEvent;

typedef struct {
    const WifiCredentials* credentials;
    const WifiIpConfig* ip_config;
} WifiConnectMessage;

typedef struct {
    WifiScanResult* data;
    uint8_t* count;
    uint8_t max_count;
} WifiScanMessage;

typedef struct {
    WifiInfo* info;
} WifiGetInfoMessage;

typedef struct {
    WifiHardwareAddress* hw_address;
} WifiGetHwAddressMessage;

typedef struct {
    WifiRequestType request_type;
    WifiStatus status;
    union {
        WifiConnectMessage connect_message;
        WifiScanMessage scan_message;
        WifiGetInfoMessage get_info_message;
        WifiGetHwAddressMessage get_hw_address_message;
    };
    FuriApiLock lock;
} WifiMessage;

struct Wifi {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* poll_timer;
    FuriSemaphore* api_semaphore;
    FuriState* state;
    Intercom* intercom;
    struct netif netif;
    WifiMessage api_message;
    WifiRequest request;
    WifiResponse response;
    WifiInfo info;
};

// Private API calls (to be removed in the future)
WifiStatus wifi_init(Wifi* instance);

// Internal API management
bool wifi_api_nonblocking_request(Wifi* instance, const WifiMessage* message);

bool wifi_api_is_locked(Wifi* instance);

void wifi_api_unlock(Wifi* instance, WifiStatus status);

// Network management
void wifi_net_init(Wifi* instance, const WifiHardwareAddress* addr);

bool wifi_net_up(Wifi* instance, const WifiIpConfig* ip_config);

void wifi_net_down(Wifi* instance);

void wifi_net_get_ip_config(Wifi* instance, WifiIpConfig* ip_config);
