#pragma once

#include <setting_provider.h>

#define APPS_MENU_ACTIVE_APPLICATION_MAX_SIZE (32 + 1)

typedef enum {
    AppsMenuSettingV1IdxActiveApp,

    AppsMenuSettingV1IdxsCount,
} AppsMenuSettingV1Idx;

typedef struct {
    char active_application[APPS_MENU_ACTIVE_APPLICATION_MAX_SIZE];
} AppsMenuSettingsV1;

extern const SettingProviderSetting apps_menu_v1_settings[];
extern const SettingProviderSetting apps_menu_v1_settings_root;
