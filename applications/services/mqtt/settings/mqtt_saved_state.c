#include "mqtt_saved_state.h"

#include <storage/storage.h>

#define MQTT_SAVED_STATE_FILE_PATH APP_DATA_PATH("state.json")
#define MQTT_SAVED_STATE_VERSION   1
#define MQTT_SAVED_STATE_ROOT      mqtt_saved_state_v1_root

void mqtt_saved_state_init(MqttSavedState* saved_state) {
    furi_check(saved_state);
    mqtt_saved_state_v1_init(saved_state);
}

void mqtt_saved_state_reset(MqttSavedState* saved_state) {
    furi_check(saved_state);

    SettingProvider* provider =
        setting_provider_alloc(MQTT_SAVED_STATE_FILE_PATH, MQTT_SAVED_STATE_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_reset(provider, &MQTT_SAVED_STATE_ROOT, saved_state);
    setting_provider_close(provider);
    setting_provider_free(provider);
}

void mqtt_saved_state_load(MqttSavedState* saved_state) {
    furi_check(saved_state);

    SettingProvider* provider =
        setting_provider_alloc(MQTT_SAVED_STATE_FILE_PATH, MQTT_SAVED_STATE_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_load(provider, &MQTT_SAVED_STATE_ROOT, saved_state);
    setting_provider_close(provider);
    setting_provider_free(provider);
}

bool mqtt_saved_state_save(const MqttSavedState* saved_state) {
    furi_check(saved_state);

    SettingProvider* provider =
        setting_provider_alloc(MQTT_SAVED_STATE_FILE_PATH, MQTT_SAVED_STATE_VERSION, NULL, 0);

    setting_provider_open(provider);
    const bool success = setting_provider_save(provider, &MQTT_SAVED_STATE_ROOT, saved_state);
    setting_provider_close(provider);
    setting_provider_free(provider);

    return success;
}

bool mqtt_saved_state_is_valid(const MqttSavedState* saved_state) {
    furi_check(saved_state);
    return mqtt_saved_state_v1_is_valid(saved_state);
}
