#pragma once

#include "../settings.h"

#define SETTINGS_VOLUME_RANGE_MIN 0
#define SETTINGS_VOLUME_RANGE_MAX 100
#define SETTINGS_VOLUME_STEP      5

void settings_volume_set(SettingsApp* instance, uint8_t volume);
uint8_t settings_volume_get(SettingsApp* instance);
