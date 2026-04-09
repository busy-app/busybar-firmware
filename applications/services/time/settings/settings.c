#include "settings.h"

#include <storage/storage.h>

extern const SettingProviderSetting time_v1_settings_root;

#define TIME_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define TIME_SETTINGS_VERSION   1
#define TIME_SETTINGS_ROOT      time_v1_settings_root

bool time_settings_reset(TimeSettings* settings) {
    SettingProvider* provider =
        setting_provider_alloc(TIME_SETTINGS_FILE_PATH, TIME_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_reset(provider, &TIME_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool time_settings_load(TimeSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(TIME_SETTINGS_FILE_PATH, TIME_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_load(provider, &TIME_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool time_settings_save(const TimeSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(TIME_SETTINGS_FILE_PATH, TIME_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_save(provider, &TIME_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}
