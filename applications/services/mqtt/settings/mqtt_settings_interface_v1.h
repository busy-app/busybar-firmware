#pragma once

#include "../mqtt_config.h"

#include <setting_provider.h>

typedef enum {
    MqttSettingsV1IdxConfig,
    MqttSettingsV1IdxMax,
} MqttSettingsV1Idx;

typedef struct {
    MqttConfig config;
} MqttSettingsV1;

extern const SettingProviderSetting mqtt_settings_v1[];
extern const SettingProviderSetting mqtt_settings_v1_root;
