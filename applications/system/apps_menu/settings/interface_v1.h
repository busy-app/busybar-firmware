#pragma once

#include <toolbox/setting_provider.h>

typedef enum {
    AppsMenuSettingV1IdxActiveApp,

    AppsMenuSettingV1IdxsCount,
} AppsMenuSettingV1Idx;

typedef struct {
    FuriString* active_application;
} AppsMenuSettingsV1;

extern const SettingProviderSetting apps_menu_v1_settings[];
extern const SettingProviderSetting apps_menu_v1_settings_root;
