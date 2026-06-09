#pragma once

#include "settings.h"

bool power_settings_reset(PowerSettings* settings);
bool power_settings_load(PowerSettings* settings);
bool power_settings_save(const PowerSettings* settings);
