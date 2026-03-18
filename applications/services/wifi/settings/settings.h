#pragma once

#include "interface_v1.h"

typedef WifiSettingsV1 WifiSettings;

bool wifi_settings_reset(WifiSettings* settings);
bool wifi_settings_load(WifiSettings* settings);
bool wifi_settings_save(const WifiSettings* settings);
