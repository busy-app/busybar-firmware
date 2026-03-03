#pragma once

#include "interface_v1.h"
#include <storage/storage.h>

typedef BrightnessSettingsV1 BrightnessSettings;

extern const SettingProviderSetting brightness_v1_settings_root;

#define BRIGHTNESS_SETTINGS_FILE_PATH APP_DATA_PATH("config.json")
#define BRIGHTNESS_SETTINGS_VERSION   1
#define BRIGHTNESS_SETTINGS_ROOT      brightness_v1_settings_root
