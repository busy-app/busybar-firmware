#pragma once

#include "wifi.h"

#include "wifi_common_i.h"
#include "wifi_settings.h"

#include <api_lock.h>

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
    WifiRequestType request_type;
    WifiStatus status;
    union {
        WifiConnectMessage connect_message;
        WifiScanMessage scan_message;
        WifiGetInfoMessage get_info_message;
    };
    FuriApiLock lock;
} WifiMessage;

struct Wifi {
    FuriEventLoop* event_loop;
    FuriSemaphore* access_semaphore;
    FuriPubSub* pubsub;
    Intercom* intercom;
    WifiMessage* current_message;
    WifiRequest request;
    WifiResponse response;
    WifiSettings settings;
    bool settings_applied;
};
