#include "settings_i.h"

#include <storage/storage.h>

#define UPDATER_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define UPDATER_SETTINGS_VERSION   2
#define UPDATER_SETTINGS_ROOT      updater_v2_settings_root

bool updater_settings_reset(UpdaterSettings* settings) {
    SettingProvider* provider =
        setting_provider_alloc(UPDATER_SETTINGS_FILE_PATH, UPDATER_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_reset(provider, &UPDATER_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool updater_settings_load(UpdaterSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(UPDATER_SETTINGS_FILE_PATH, UPDATER_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_load(provider, &UPDATER_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool updater_settings_save(const UpdaterSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(UPDATER_SETTINGS_FILE_PATH, UPDATER_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_save(provider, &UPDATER_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

const char* updater_settings_get_check_url_value(UpdaterSettings* settings) {
    int default_key_comparison_result = strncmp(
        settings->check_url,
        UPDATER_SETTINGS_CHECK_URL_DEFAULT_ALIAS,
        sizeof(settings->check_url));

    return (default_key_comparison_result == 0) ? UPDATER_SETTINGS_CHECK_URL_DEFAULT_VALUE :
                                                  settings->check_url;
}
