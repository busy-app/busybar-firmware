#include "settings_i.h"

#include <storage/storage.h>

#define UPDATER_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define UPDATER_SETTINGS_VERSION   1
#define UPDATER_SETTINGS_ROOT      updater_v1_settings_root

void updater_settings_reset(UpdaterSettings* settings) {
    SettingProvider* provider =
        setting_provider_alloc(UPDATER_SETTINGS_FILE_PATH, UPDATER_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_reset(provider, &UPDATER_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);
}

void updater_settings_load(UpdaterSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(UPDATER_SETTINGS_FILE_PATH, UPDATER_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_load(provider, &UPDATER_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);
}

bool updater_settings_save(const UpdaterSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(UPDATER_SETTINGS_FILE_PATH, UPDATER_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    bool is_success = setting_provider_save(provider, &UPDATER_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);

    return is_success;
}

void updater_settings_copy(UpdaterSettings* target, const UpdaterSettings* source) {
    furi_check(source);
    furi_check(target);

    target->check_startup_interval = source->check_startup_interval;
    target->check_interval = source->check_interval;
    target->autoupdate_enabled = source->autoupdate_enabled;
    target->autoupdate_interval_start = source->autoupdate_interval_start;
    target->autoupdate_interval_end = source->autoupdate_interval_end;

    furi_string_set(target->check_url, source->check_url);
    furi_string_set(target->check_channel_id, source->check_channel_id);
}
