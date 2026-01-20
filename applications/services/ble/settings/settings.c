#include "settings.h"

#include <storage/storage.h>

#define BLE_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define BLE_SETTINGS_VERSION   1
#define BLE_SETTINGS_ROOT      ble_v1_settings_root

void ble_settings_reset(BleSettings* settings) {
    SettingProvider* provider =
        setting_provider_alloc(BLE_SETTINGS_FILE_PATH, BLE_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_reset(provider, &BLE_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);
}

void ble_settings_load(BleSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(BLE_SETTINGS_FILE_PATH, BLE_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_load(provider, &BLE_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);
}

bool ble_settings_save(const BleSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(BLE_SETTINGS_FILE_PATH, BLE_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    bool is_success = setting_provider_save(provider, &BLE_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);

    return is_success;
}
