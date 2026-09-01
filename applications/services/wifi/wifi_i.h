#pragma once

#include "wifi.h"

#include "wifi_common_i.h"
#include "settings/settings.h"

#include <api_lock.h>

#include <lwip/netif.h>

#include <intercom/intercom.h>
#include <device_name/device_name.h>

#define TAG "WifiSrv"

typedef enum {
    WifiEventTypeDeviceNameInfo,
} WifiEventType;

typedef struct {
    WifiEventType type;
    union {
        DeviceNameInfo device_name_info;
    };
} WifiEvent;

typedef struct {
    WifiCredentials credentials;
    WifiIpConfig ip_config;
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
    bool is_priority;
    WifiStatus* status;
    union {
        WifiConnectMessage connect_message;
        WifiScanMessage scan_message;
        WifiGetInfoMessage get_info_message;
    };
    FuriApiLock lock;
} WifiMessage;

struct Wifi {
    FuriEventLoop* event_loop;
    FuriMessageQueue* api_queue;
    FuriMessageQueue* event_queue;
    FuriMessageQueue* priority_queue;
    FuriMessageQueue* response_queue;
    FuriSemaphore* dhcp_semaphore;
    FuriState* state;
    Intercom* intercom;
    IntercomChannel* intercom_ch_control;
    IntercomChannel* intercom_ch_data;
    FuriString* hostname;
    struct netif netif;
    WifiMessage api_message;
    WifiRequest request;
    bool is_processing;
};

// Deferred actions
void wifi_pending_request_callback(void* context);

// API management
void wifi_api_unlock(Wifi* instance, WifiStatus status);

// Internal nonblocking API calls
void wifi_schedule_init_request(Wifi* instance);

void wifi_schedule_deinit_request(Wifi* instance);

void wifi_schedule_connect_request(Wifi* instance, const WifiSettings* settings);

void wifi_schedule_disconnect_request(Wifi* instance);

void wifi_send_device_name_info_event(Wifi* instance, const DeviceNameInfo* device_name_state);

// Network management
void wifi_net_init(Wifi* instance, const uint8_t* hw_addr);

bool wifi_net_up(Wifi* instance, const WifiIpConfig* ip_config);

void wifi_net_down(Wifi* instance);

void wifi_net_get_ip_config(Wifi* instance, WifiIpConfig* ip_config);

void wifi_net_set_hostname(Wifi* instance, const char* hostname);

// Power management
void wifi_power_init(Wifi* instance);
