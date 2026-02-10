#include "busy_settings.h"

#include <storage/storage.h>

#define BUSY_SETTINGS_VERSION    1
#define BUSY_SETTINGS_TEMPLATE   busy_settings_v1
#define BUSY_SETTINGS_INTERFACES busy_settings_v1_interfaces
#define BUSY_SETTINGS_COUNT      BusySettingsV1IdxMax

static const char* busy_settings_file_paths[BusySettingsProfileIdMax] = {
    [BusySettingsProfileIdBusy] = APP_DATA_PATH("settings_busy.json"),
    [BusySettingsProfileIdCustom] = APP_DATA_PATH("settings_custom.json"),
};

static void busy_settings_init_inner_settings(
    SettingProviderSetting* inner_settings,
    BusySettingsProfileId profile_id) {
    memcpy(inner_settings, BUSY_SETTINGS_TEMPLATE, sizeof(BUSY_SETTINGS_TEMPLATE));

    const void** inner_settings_interfaces = BUSY_SETTINGS_INTERFACES[profile_id];

    for(uint32_t i = 0; i < BUSY_SETTINGS_COUNT; ++i) {
        inner_settings[i].interface = inner_settings_interfaces[i];
    }
}

void busy_settings_load(BusySettings* settings, BusySettingsProfileId profile_id) {
    furi_check(settings);
    furi_check(profile_id < BusySettingsProfileIdMax);

    SettingProvider* provider = setting_provider_alloc(
        busy_settings_file_paths[profile_id], BUSY_SETTINGS_VERSION, NULL, 0);

    SettingProviderSetting inner_settings[BUSY_SETTINGS_COUNT];
    busy_settings_init_inner_settings(inner_settings, profile_id);

    const SettingProviderSetting settings_root = {
        .interface =
            &(const SettingProviderStructureInterface){
                .inner_settings = inner_settings,
                .inner_settings_count = BUSY_SETTINGS_COUNT,
            },
        .type = SettingProviderSettingTypeStructure,
    };

    setting_provider_open(provider);
    setting_provider_load(provider, &settings_root, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);
}

bool busy_settings_save(const BusySettings* settings, BusySettingsProfileId profile_id) {
    furi_check(settings);
    furi_check(profile_id < BusySettingsProfileIdMax);

    SettingProvider* provider = setting_provider_alloc(
        busy_settings_file_paths[profile_id], BUSY_SETTINGS_VERSION, NULL, 0);

    SettingProviderSetting inner_settings[BUSY_SETTINGS_COUNT];
    busy_settings_init_inner_settings(inner_settings, profile_id);

    const SettingProviderSetting settings_root = {
        .interface =
            &(const SettingProviderStructureInterface){
                .inner_settings = inner_settings,
                .inner_settings_count = BUSY_SETTINGS_COUNT,
            },
        .type = SettingProviderSettingTypeStructure,
    };

    setting_provider_open(provider);
    const bool success = setting_provider_save(provider, &settings_root, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);

    return success;
}
