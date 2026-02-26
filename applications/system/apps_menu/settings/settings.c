#include "settings.h"

#include <storage/storage.h>

#define APPS_MENU_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define APPS_MENU_SETTINGS_VERSION   1

void apps_menu_settings_reset(AppsMenuSettings* settings) {
    SettingProvider* provider =
        setting_provider_alloc(APPS_MENU_SETTINGS_FILE_PATH, APPS_MENU_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_reset(provider, &apps_menu_v1_settings_root, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);
}

void apps_menu_settings_load(AppsMenuSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(APPS_MENU_SETTINGS_FILE_PATH, APPS_MENU_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_load(provider, &apps_menu_v1_settings_root, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);
}

bool apps_menu_settings_save(const AppsMenuSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(APPS_MENU_SETTINGS_FILE_PATH, APPS_MENU_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    bool is_success = setting_provider_save(provider, &apps_menu_v1_settings_root, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);

    return is_success;
}
