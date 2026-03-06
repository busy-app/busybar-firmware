#pragma once

#include "../mqtt_common.h"

#include <setting_provider.h>

#define MQTT_SETTINGS_CUSTOM_URL_MAX_SIZE (512 + 1)

typedef enum {
    MqttSettingsV1IdxProfileId,
    MqttSettingsV1IdxCustomUrl,
    MqttSettingsV1IdxMax,
} MqttSettingsV1Idx;

typedef struct {
    MqttProfileId profile_id;
    char custom_url[MQTT_SETTINGS_CUSTOM_URL_MAX_SIZE];
} MqttSettingsV1;

extern const SettingProviderSetting mqtt_settings_v1[];
extern const SettingProviderSetting mqtt_settings_v1_root;

void mqtt_settings_v1_init(MqttSettingsV1* settings_v1);
