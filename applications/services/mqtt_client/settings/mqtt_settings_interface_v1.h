#pragma once

#include <toolbox/setting_provider.h>

#include "../mqtt_common.h"

typedef enum {
    MqttSettingsV1IdxProfileId,
    MqttSettingsV1IdxCustomUrl,
    MqttSettingsV1IdxMax,
} MqttSettingsV1Idx;

typedef struct {
    MqttProfileId profile_id;
    FuriString* custom_url;
} MqttSettingsV1;

extern const SettingProviderSetting mqtt_settings_v1[];
extern const SettingProviderSetting mqtt_settings_v1_root;

void mqtt_settings_v1_init(MqttSettingsV1* settings_v1);
