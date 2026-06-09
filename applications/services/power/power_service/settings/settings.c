#include "settings.h"

#include <storage/storage.h>

extern const SettingProviderSetting power_v1_settings_root;

#define POWER_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define POWER_SETTINGS_VERSION   1
#define POWER_SETTINGS_ROOT      power_v1_settings_root

bool power_settings_reset(PowerSettings* settings) {
    SettingProvider* provider =
        setting_provider_alloc(POWER_SETTINGS_FILE_PATH, POWER_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_reset(provider, &POWER_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool power_settings_load(PowerSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(POWER_SETTINGS_FILE_PATH, POWER_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_load(provider, &POWER_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool power_settings_save(const PowerSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(POWER_SETTINGS_FILE_PATH, POWER_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_save(provider, &POWER_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}
