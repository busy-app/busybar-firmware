#pragma once

#include <setting_provider.h>

#define BUSY_SETTINGS_V1_THEME_NAME_LEN (64)

typedef enum {
    BusySettingsV1IdxThemeName,
    BusySettingsV1IdxSmartHome,
    BusySettingsV1IdxWorkOnly,
    BusySettingsV1IdxMax,
} BusySettingsV1Idx;

typedef struct {
    char theme_name[BUSY_SETTINGS_V1_THEME_NAME_LEN + 1];
    bool is_smart_home_enabled;
    bool is_show_work_only_enabled;
} BusySettingsV1;

extern const SettingProviderSetting busy_settings_v1_root;
