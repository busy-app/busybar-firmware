#pragma once

#include "../mqtt_profile.h"

#include <setting_provider.h>

typedef enum {
    MqttSettingsV1IdxProfile,
    MqttSettingsV1IdxMax,
} MqttSettingsV1Idx;

typedef struct {
    MqttProfile profile;
} MqttSettingsV1;

extern const SettingProviderSetting mqtt_settings_v1[];
extern const SettingProviderSetting mqtt_settings_v1_root;
