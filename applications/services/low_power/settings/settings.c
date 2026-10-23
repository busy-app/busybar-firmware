#include "settings.h"

#include <storage/storage.h>

#define LOW_POWER_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define LOW_POWER_SETTINGS_VERSION   1
#define LOW_POWER_SETTINGS_ROOT      low_power_v1_settings_root

bool low_power_settings_reset(LowPowerSettings* settings) {
    SettingProvider* provider =
        setting_provider_alloc(LOW_POWER_SETTINGS_FILE_PATH, LOW_POWER_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_reset(provider, &LOW_POWER_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool low_power_settings_load(LowPowerSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(LOW_POWER_SETTINGS_FILE_PATH, LOW_POWER_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_load(provider, &LOW_POWER_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool low_power_settings_save(const LowPowerSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(LOW_POWER_SETTINGS_FILE_PATH, LOW_POWER_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_save(provider, &LOW_POWER_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}
