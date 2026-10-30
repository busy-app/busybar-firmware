#pragma once

#include "interface_v1.h"

typedef LowPowerSettingsV1 LowPowerSettings;

bool low_power_settings_reset(LowPowerSettings* settings);
bool low_power_settings_load(LowPowerSettings* settings);
bool low_power_settings_save(const LowPowerSettings* settings);
