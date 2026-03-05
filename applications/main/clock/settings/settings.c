#include "settings.h"

#include <storage/storage.h>

#define CLOCK_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define CLOCK_SETTINGS_VERSION   1
#define CLOCK_SETTINGS_ROOT      clock_v1_settings_root

bool clock_settings_reset(ClockSettings* settings) {
    SettingProvider* provider =
        setting_provider_alloc(CLOCK_SETTINGS_FILE_PATH, CLOCK_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_reset(provider, &CLOCK_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool clock_settings_load(ClockSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(CLOCK_SETTINGS_FILE_PATH, CLOCK_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_load(provider, &CLOCK_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool clock_settings_save(const ClockSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(CLOCK_SETTINGS_FILE_PATH, CLOCK_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_save(provider, &CLOCK_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}
