#pragma once

#include "interface_v1.h"

typedef ClockSettingsV1 ClockSettings;

void clock_settings_reset(ClockSettings* settings);
void clock_settings_load(ClockSettings* settings);
bool clock_settings_save(const ClockSettings* settings);
