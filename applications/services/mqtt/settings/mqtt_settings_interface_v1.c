#include "mqtt_settings_interface_v1.h"

#define MQTT_PROFILE_DEFAULT             MqttProfileIdProduction
#define MQTT_SETTINGS_CUSTOM_URL_DEFAULT ""

static const char* mqtt_settings_v1_profile_string_map[] = {
    [MqttProfileIdProduction] = "production",
    [MqttProfileIdCustom] = "custom",
};

static_assert(COUNT_OF(mqtt_settings_v1_profile_string_map) == MqttProfileIdMax);

static bool mqtt_settings_v1_custom_url_is_valid_cb(
    const SettingProviderSetting* setting,
    const char* value) {
    UNUSED(setting);

    return strlen(value) == 0 ||
           strncmp(value, MQTT_URL_TLS_PREFIX, strlen(MQTT_URL_TLS_PREFIX)) == 0 ||
           strncmp(value, MQTT_URL_TLS_PREFIX, strlen(MQTT_URL_PREFIX)) == 0;
}

static const SettingProviderEnumInterface mqtt_settings_v1_profile_interface = {
    .string_map = mqtt_settings_v1_profile_string_map,
    .string_map_length = COUNT_OF(mqtt_settings_v1_profile_string_map),
    .type_size = SIZEOF_MEMBER(MqttSettingsV1, profile_id),
    .default_value = &(const MqttProfileId){MQTT_PROFILE_DEFAULT},
};

static const SettingProviderStringInterface mqtt_settings_v1_custom_url_interface = {
    .is_valid_callback = mqtt_settings_v1_custom_url_is_valid_cb,
    .max_size = SIZEOF_MEMBER(MqttSettingsV1, custom_url),
    .default_value = MQTT_SETTINGS_CUSTOM_URL_DEFAULT,
};

void mqtt_settings_v1_init(MqttSettingsV1* settings_v1) {
    furi_check(settings_v1->custom_url == NULL);
    settings_v1->profile_id = MQTT_PROFILE_DEFAULT;
}

const SettingProviderSetting mqtt_settings_v1[] = {
    [MqttSettingsV1IdxProfileId] =
        {
            .name = "profile",
            .interface = &mqtt_settings_v1_profile_interface,
            .field_offset = offsetof(MqttSettingsV1, profile_id),
            .type = SettingProviderSettingTypeEnum,
        },
    [MqttSettingsV1IdxCustomUrl] =
        {
            .name = "custom_url",
            .interface = &mqtt_settings_v1_custom_url_interface,
            .field_offset = offsetof(MqttSettingsV1, custom_url),
            .type = SettingProviderSettingTypeString,
        },
    [MqttSettingsV1IdxCustomIgnoreServerCert] =
        {
            .name = "custom_ignore_server_cert",
            .interface = &(const SettingProviderBoolInterface){.default_value = false},
            .field_offset = offsetof(MqttSettingsV1, custom_ignore_server_cert),
            .type = SettingProviderSettingTypeBool,
        },
    [MqttSettingsV1IdxCustomUseMtls] =
        {
            .name = "custom_use_mtls",
            .interface = &(const SettingProviderBoolInterface){.default_value = false},
            .field_offset = offsetof(MqttSettingsV1, custom_use_mtls),
            .type = SettingProviderSettingTypeBool,
        },
    [MqttSettingsV1IdxCustomClientCert] =
        {
            .name = "custom_client_cert",
            .interface = &(const SettingProviderBoolInterface){.default_value = false},
            .field_offset = offsetof(MqttSettingsV1, custom_client_cert),
            .type = SettingProviderSettingTypeBool,
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
