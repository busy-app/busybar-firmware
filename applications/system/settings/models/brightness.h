#pragma once

#include "../settings.h"

#define SETTINGS_BRIGHTNESS_RANGE_MIN 5
#define SETTINGS_BRIGHTNESS_RANGE_MAX 100
#define SETTINGS_BRIGHTNESS_STEP      5

typedef enum {
    SettingsBrightnessModeManual,
    SettingsBrightnessModeAuto,

    SettingsBrightnessModeCount,
} SettingsBrightnessMode;

void settings_brightness_set_auto_mode(SettingsApp* instance);
SettingsBrightnessMode settings_brightness_get_mode(SettingsApp* instance);

void settings_brightness_set(SettingsApp* instance, uint8_t brightness);
uint8_t settings_brightness_get(SettingsApp* instance);
