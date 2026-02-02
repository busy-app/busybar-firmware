#pragma once

#include "interface_v1.h"

typedef WifiSettingsV1 WifiSettings;

void wifi_settings_reset(WifiSettings* settings);
void wifi_settings_load(WifiSettings* settings);
bool wifi_settings_save(const WifiSettings* settings);
