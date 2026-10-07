#include "settings.h"

#include <storage/storage.h>

#define APPS_MENU_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define APPS_MENU_SETTINGS_VERSION   1
#define APPS_MENU_SETTINGS_ROOT      apps_menu_v1_settings_root

bool apps_menu_settings_reset(AppsMenuSettings* settings) {
    SettingProvider* provider =
        setting_provider_alloc(APPS_MENU_SETTINGS_FILE_PATH, APPS_MENU_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_reset(provider, &APPS_MENU_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool apps_menu_settings_load(AppsMenuSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(APPS_MENU_SETTINGS_FILE_PATH, APPS_MENU_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_load(provider, &APPS_MENU_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool apps_menu_settings_save(const AppsMenuSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(APPS_MENU_SETTINGS_FILE_PATH, APPS_MENU_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_save(provider, &APPS_MENU_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}
