#pragma once

#include "../mqtt_common.h"

#include <setting_provider.h>

#define MQTT_SETTINGS_CUSTOM_URL_MAX_SIZE (512 + 1)

typedef enum {
    MqttSettingsV1IdxProfileId,
    MqttSettingsV1IdxCustomUrl,
    MqttSettingsV1IdxCustomIgnoreServerCert,
    MqttSettingsV1IdxCustomUseMtls,
    MqttSettingsV1IdxCustomClientCert,
    MqttSettingsV1IdxMax,
} MqttSettingsV1Idx;

typedef struct {
    MqttProfileId profile_id;
    char custom_url[MQTT_SETTINGS_CUSTOM_URL_MAX_SIZE];
    bool custom_ignore_server_cert;
    bool custom_use_mtls;
    bool custom_client_cert;
} MqttSettingsV1;

extern const SettingProviderSetting mqtt_settings_v1[];
extern const SettingProviderSetting mqtt_settings_v1_root;

void mqtt_settings_v1_init(MqttSettingsV1* settings_v1);
