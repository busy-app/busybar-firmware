#pragma once

#include "wifi_common.h"

#include <stdbool.h>

typedef struct {
    WifiCredentials credentials;
    WifiIpConfig ip_config;
    bool enabled;
} WifiSettings;

bool wifi_settings_load(WifiSettings* settings);

bool wifi_settings_save(const WifiSettings* settings);
