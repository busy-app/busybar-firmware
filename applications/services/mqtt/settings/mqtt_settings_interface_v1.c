#include "mqtt_settings_interface_v1.h"

#include "../mqtt_profile_i.h"

static bool mqtt_settings_v1_profile_is_valid_callback(
    const SettingProviderSetting* setting,
    const void* value) {
    UNUSED(setting);

    furi_assert(value);
    const MqttProfile* profile = value;

    return mqtt_profile_is_valid(profile);
}

static bool mqtt_settings_v1_profile_serialize_callback(
    const SettingProviderSetting* setting,
    const void* value,
    cJSON* json_node) {
    UNUSED(setting);

    furi_assert(value);
    const MqttProfile* profile = value;

    return mqtt_profile_serialize_raw(profile, json_node);
}

static bool mqtt_settings_v1_profile_deserialize_callback(
    const SettingProviderSetting* setting,
    const cJSON* json_node,
    void* value) {
    UNUSED(setting);

    furi_assert(value);
    MqttProfile* profile = value;

    return mqtt_profile_deserialize_raw(profile, json_node);
}

static const SettingProviderRawInterface mqtt_settings_v1_profile_interface = {
    .is_valid_callback = mqtt_settings_v1_profile_is_valid_callback,
    .serialize_callback = mqtt_settings_v1_profile_serialize_callback,
    .deserialize_callback = mqtt_settings_v1_profile_deserialize_callback,
    .default_value_size = sizeof(MqttProfile),
    .default_value =
        &(const MqttProfile){
            .type = MqttProfileTypeDefault,
            .custom_config = {},
        },
};

const SettingProviderSetting mqtt_settings_v1[] = {
    [MqttSettingsV1IdxProfile] =
        {
            .name = "profile",
            .interface = &mqtt_settings_v1_profile_interface,
            .field_offset = offsetof(MqttSettingsV1, profile),
            .type = SettingProviderSettingTypeRaw,
        },
};

const SettingProviderSetting mqtt_settings_v1_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = mqtt_settings_v1,
            .inner_settings_count = COUNT_OF(mqtt_settings_v1),
        },
    .type = SettingProviderSettingTypeStruct,
};

static_assert(COUNT_OF(mqtt_settings_v1) == MqttSettingsV1IdxMax);
