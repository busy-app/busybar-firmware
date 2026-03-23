#include "device_name_settings.h"

#include <storage/storage.h>

#define DEVICE_NAME_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define DEVICE_NAME_SETTINGS_VERSION   1
#define DEVICE_NAME_SETTINGS_ROOT      device_name_settings_v1_root

bool device_name_settings_load(DeviceNameSettings* settings) {
    furi_check(settings);

    SettingProvider* provider = setting_provider_alloc(
        DEVICE_NAME_SETTINGS_FILE_PATH, DEVICE_NAME_SETTINGS_VERSION, NULL, 0);

    const bool success = setting_provider_load(provider, &DEVICE_NAME_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return success;
}

bool device_name_settings_save(const DeviceNameSettings* settings) {
    furi_check(settings);

    SettingProvider* provider = setting_provider_alloc(
        DEVICE_NAME_SETTINGS_FILE_PATH, DEVICE_NAME_SETTINGS_VERSION, NULL, 0);

    const bool success = setting_provider_save(provider, &DEVICE_NAME_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return success;
}
