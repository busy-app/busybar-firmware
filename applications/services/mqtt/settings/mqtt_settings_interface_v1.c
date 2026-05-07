#include "mqtt_settings_interface_v1.h"

#include "../mqtt_config_i.h"

static bool mqtt_settings_v1_config_is_valid_callback(
    const SettingProviderSetting* setting,
    const void* value) {
    UNUSED(setting);

    furi_assert(value);
    const MqttConfig* config = value;

    return mqtt_config_is_valid(config);
}

static bool mqtt_settings_v1_config_serialize_callback(
    const SettingProviderSetting* setting,
    const void* value,
    cJSON* json_node) {
    UNUSED(setting);

    furi_assert(value);
    const MqttConfig* config = value;

    return mqtt_config_serialize_raw(config, json_node);
}

static bool mqtt_settings_v1_config_deserialize_callback(
    const SettingProviderSetting* setting,
    const cJSON* json_node,
    void* value) {
    UNUSED(setting);

    furi_assert(value);
    MqttConfig* config = value;

    return mqtt_config_deserialize_raw(config, json_node);
}

static const SettingProviderRawInterface mqtt_settings_v1_config_interface = {
    .is_valid_callback = mqtt_settings_v1_config_is_valid_callback,
    .serialize_callback = mqtt_settings_v1_config_serialize_callback,
    .deserialize_callback = mqtt_settings_v1_config_deserialize_callback,
    .default_value_size = sizeof(MqttConfig),
    .default_value =
        &(const MqttConfig){
            .server_url = MQTT_CONFIG_SERVER_URL_DEFAULT,
            .client_cert_type = MqttClientCertTypeDefault,
            .ignore_server_cert = false,
        },
};

const SettingProviderSetting mqtt_settings_v1[] = {
    [MqttSettingsV1IdxConfig] =
        {
            .name = "config",
            .interface = &mqtt_settings_v1_config_interface,
            .field_offset = offsetof(MqttSettingsV1, config),
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
