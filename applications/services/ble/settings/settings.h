#pragma once

#include "interface_v1.h"

typedef BleSettingsV1 BleSettings;

bool ble_settings_reset(BleSettings* settings);
bool ble_settings_load(BleSettings* settings);
bool ble_settings_save(const BleSettings* settings);
