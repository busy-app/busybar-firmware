#pragma once

#include <setting_provider.h>

#define APPS_MENU_ACTIVE_APPLICATION_MAX_LEN  (32)
#define APPS_MENU_ACTIVE_APPLICATION_MAX_SIZE (APPS_MENU_ACTIVE_APPLICATION_MAX_LEN + 1)

typedef struct {
    char active_application[APPS_MENU_ACTIVE_APPLICATION_MAX_SIZE];
} AppsMenuSettingsV1;

extern const SettingProviderSetting apps_menu_v1_settings_root;
