/**
 * @file wifi.h
 * @brief API for controlling WiFi networks
 */
#pragma once

#include <core/string.h>

#include "wifi_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_WIFI "wifi"

WifiStatus wifi_init(Wifi* instance);

WifiStatus wifi_deinit(Wifi* instance);

WifiStatus wifi_scan(Wifi* instance, FuriString** ssids, uint8_t max_count);

WifiStatus wifi_connect(Wifi* instance, const FuriString* ssid, const FuriString* passphrase);

WifiStatus wifi_disconnect(Wifi* instance);

#ifdef __cplusplus
}
#endif
