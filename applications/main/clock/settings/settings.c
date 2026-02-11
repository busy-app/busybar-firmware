#include "settings.h"

#include <storage/storage.h>

#define CLOCK_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define CLOCK_SETTINGS_VERSION   1
#define CLOCK_SETTINGS_ROOT      clock_v1_settings_root

void clock_settings_reset(ClockSettings* settings) {
    SettingProvider* provider =
        setting_provider_alloc(CLOCK_SETTINGS_FILE_PATH, CLOCK_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_reset(provider, &CLOCK_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);
}

void clock_settings_load(ClockSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(CLOCK_SETTINGS_FILE_PATH, CLOCK_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_load(provider, &CLOCK_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);
}

bool clock_settings_save(const ClockSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(CLOCK_SETTINGS_FILE_PATH, CLOCK_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    bool is_success = setting_provider_save(provider, &CLOCK_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);

    return is_success;
}
