#include "settings.h"

#include <storage/storage.h>

#define WIFI_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define WIFI_SETTINGS_VERSION   1
#define WIFI_SETTINGS_ROOT      wifi_v1_settings_root

void wifi_settings_reset(WifiSettings* settings) {
    SettingProvider* provider =
        setting_provider_alloc(WIFI_SETTINGS_FILE_PATH, WIFI_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_reset(provider, &WIFI_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);
}

void wifi_settings_load(WifiSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(WIFI_SETTINGS_FILE_PATH, WIFI_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_load(provider, &WIFI_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);
}

bool wifi_settings_save(const WifiSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(WIFI_SETTINGS_FILE_PATH, WIFI_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    bool is_success = setting_provider_save(provider, &WIFI_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);

    return is_success;
}
