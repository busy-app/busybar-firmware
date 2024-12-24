/**
 * @file wifi.h
 * @brief API for controlling WiFi networks
 */
#pragma once

#include "wifi_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_WIFI "wifi"

WifiStatus wifi_init(Wifi* instance);

WifiStatus wifi_deinit(Wifi* instance);

WifiStatus wifi_scan(
    Wifi* instance,
    WifiScanResult* results,
    uint8_t* result_count,
    uint8_t max_result_count);

WifiStatus wifi_connect(Wifi* instance, const WifiCredentials* credentials);

WifiStatus wifi_disconnect(Wifi* instance);

WifiStatus wifi_get_info(Wifi* instance, WifiInfo* info);

#ifdef __cplusplus
}
#endif
