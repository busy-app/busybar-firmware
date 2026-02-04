#include "busy_settings.h"

#include <storage/storage.h>

#define BUSY_SETTINGS_ROOT         busy_settings_v1_root
#define BUSY_SETTINGS_FILE_PATH(p) busy_settings_file_paths[(p)]
#define BUSY_SETTINGS_VERSION      1

static const char* busy_settings_file_paths[BusySettingsProfileIdMax] = {
    [BusySettingsProfileIdBusy] = APP_DATA_PATH("settings_busy.json"),
    [BusySettingsProfileIdCustom] = APP_DATA_PATH("settings_custom.json"),
};

static const BusySettings busy_settings_defaults[BusySettingsProfileIdMax] = {
    [BusySettingsProfileIdBusy] =
        {
            .theme_name = "default",
            .is_smart_home_enabled = true,
            .is_show_work_only_enabled = false,
        },
    [BusySettingsProfileIdCustom] =
        {
            .theme_name = "keep_out",
            .is_smart_home_enabled = true,
            .is_show_work_only_enabled = true,
        },
};

void busy_settings_load(BusySettings* settings, BusySettingsProfileId profile_id) {
    furi_check(settings);
    furi_check(profile_id < BusySettingsProfileIdMax);

    SettingProvider* provider = setting_provider_alloc(
        BUSY_SETTINGS_FILE_PATH(profile_id), BUSY_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_load(provider, &BUSY_SETTINGS_ROOT, settings);

    if(strlen(settings->theme_name) == 0) {
        const BusySettings* default_settings = &busy_settings_defaults[profile_id];

        strcpy(settings->theme_name, default_settings->theme_name);
        settings->is_smart_home_enabled = default_settings->is_smart_home_enabled;
        settings->is_show_work_only_enabled = default_settings->is_show_work_only_enabled;

        setting_provider_save(provider, &BUSY_SETTINGS_ROOT, settings);
    }

    setting_provider_close(provider);
    setting_provider_free(provider);
}

bool busy_settings_save(const BusySettings* settings, BusySettingsProfileId profile_id) {
    furi_check(settings);
    furi_check(profile_id < BusySettingsProfileIdMax);

    SettingProvider* provider = setting_provider_alloc(
        BUSY_SETTINGS_FILE_PATH(profile_id), BUSY_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    const bool success = setting_provider_save(provider, &BUSY_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);

    return success;
}
