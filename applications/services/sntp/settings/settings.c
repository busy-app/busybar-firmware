#include "settings.h"

#include <storage/storage.h>

extern const SettingProviderSetting sntp_v1_settings_root;

#define SNTP_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define SNTP_SETTINGS_VERSION   1
#define SNTP_SETTINGS_ROOT      sntp_v1_settings_root

void sntp_settings_reset(SntpSettings* settings) {
    SettingProvider* provider =
        setting_provider_alloc(SNTP_SETTINGS_FILE_PATH, SNTP_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_reset(provider, &SNTP_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);
}

void sntp_settings_load(SntpSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(SNTP_SETTINGS_FILE_PATH, SNTP_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_load(provider, &SNTP_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);
}

bool sntp_settings_save(const SntpSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(SNTP_SETTINGS_FILE_PATH, SNTP_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    bool is_success = setting_provider_save(provider, &SNTP_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);

    return is_success;
}
