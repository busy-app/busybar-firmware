#pragma once

#include "settings.h"

bool time_settings_reset(TimeSettings* settings);
bool time_settings_load(TimeSettings* settings);
bool time_settings_save(const TimeSettings* settings);
