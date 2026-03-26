#pragma once

#include "device_name_settings_interface_v1.h"

typedef DeviceNameSettingsV1 DeviceNameSettings;

bool device_name_settings_load(DeviceNameSettings* settings);
bool device_name_settings_save(const DeviceNameSettings* settings);
