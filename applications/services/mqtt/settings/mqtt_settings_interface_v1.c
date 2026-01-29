#include "mqtt_settings_interface_v1.h"

#define MQTT_PROFILE_DEFAULT             MqttProfileIdDevelopment
#define MQTT_SETTINGS_CUSTOM_URL_DEFAULT ""

static const char* mqtt_settings_v1_profile_map[] = {
    [MqttProfileIdDevelopment] = "development",
    [MqttProfileIdProduction] = "production",
    [MqttProfileIdLocal] = "local",
    [MqttProfileIdCustom] = "custom",
};

static bool mqtt_settings_v1_profile_serialize_cb(
    const SettingProviderSetting* setting,
    FuriString* string,
    const void* value) {
    UNUSED(setting);
    furi_assert(value);

    const MqttProfileId profile_id = *(const MqttProfileId*)value;
    const bool is_valid = profile_id < COUNT_OF(mqtt_settings_v1_profile_map);

    if(is_valid) {
        furi_string_set(string, mqtt_settings_v1_profile_map[profile_id]);
    }

    return is_valid;
}

static bool mqtt_settings_v1_profile_deserialize_cb(
    const SettingProviderSetting* setting,
    void* value,
    const FuriString* string) {
    UNUSED(setting);
    furi_assert(value);

    MqttProfileId profile_id;
    for(profile_id = 0; profile_id < MqttProfileIdMax; ++profile_id) {
        if(furi_string_equal(string, mqtt_settings_v1_profile_map[profile_id])) {
            *((MqttProfileId*)value) = profile_id;
            break;
        }
    }

    return profile_id < MqttProfileIdMax;
}

static bool mqtt_settings_v1_custom_url_is_valid_cb(
    const SettingProviderSetting* setting,
    const FuriString* value) {
    UNUSED(setting);
    return furi_string_empty(value) || furi_string_start_with(value, MQTT_URL_TLS_PREFIX) ||
           furi_string_start_with(value, MQTT_URL_PREFIX);
}

static const SettingProviderCustomInterface mqtt_settings_v1_profile_interface = {
    .default_value = &(const MqttProfileId){MQTT_PROFILE_DEFAULT},
    .serialize_callback = mqtt_settings_v1_profile_serialize_cb,
    .deserialize_callback = mqtt_settings_v1_profile_deserialize_cb,
    .default_value_size = SIZEOF_MEMBER(MqttSettingsV1, profile_id),
};

static const SettingProviderFuriStringInterface mqtt_settings_v1_custom_url_interface = {
    .default_value = MQTT_SETTINGS_CUSTOM_URL_DEFAULT,
    .is_valid_callback = mqtt_settings_v1_custom_url_is_valid_cb,
};

void mqtt_settings_v1_init(MqttSettingsV1* settings_v1) {
    furi_check(settings_v1->custom_url == NULL);

    settings_v1->profile_id = MqttProfileIdDevelopment;
    settings_v1->custom_url = furi_string_alloc();
}

const SettingProviderSetting mqtt_settings_v1[] = {
    [MqttSettingsV1IdxProfileId] =
        {
            .name = "profile",
            .interface = &mqtt_settings_v1_profile_interface,
            .field_offset = offsetof(MqttSettingsV1, profile_id),
            .type = SettingProviderSettingTypeCustom,
        },
    [MqttSettingsV1IdxCustomUrl] =
        {
            .name = "custom_url",
            .interface = &mqtt_settings_v1_custom_url_interface,
            .field_offset = offsetof(MqttSettingsV1, custom_url),
            .type = SettingProviderSettingTypeFuriString,
        },
};

const SettingProviderSetting mqtt_settings_v1_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructureInterface){
            .inner_settings = mqtt_settings_v1,
            .inner_settings_count = COUNT_OF(mqtt_settings_v1),
        },
    .type = SettingProviderSettingTypeStructure,
};

static_assert(COUNT_OF(mqtt_settings_v1) == MqttSettingsV1IdxMax);
