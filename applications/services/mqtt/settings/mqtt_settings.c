#include "mqtt_settings.h"

#include <storage/storage.h>

#define MQTT_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define MQTT_SETTINGS_VERSION   1
#define MQTT_SETTINGS_ROOT      mqtt_settings_v1_root

bool mqtt_settings_reset(MqttSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(MQTT_SETTINGS_FILE_PATH, MQTT_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_reset(provider, &MQTT_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool mqtt_settings_load(MqttSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(MQTT_SETTINGS_FILE_PATH, MQTT_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_load(provider, &MQTT_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool mqtt_settings_save(const MqttSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(MQTT_SETTINGS_FILE_PATH, MQTT_SETTINGS_VERSION, NULL, 0);
    bool is_successful = setting_provider_save(provider, &MQTT_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}
