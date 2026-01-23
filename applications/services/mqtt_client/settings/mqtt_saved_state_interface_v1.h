#pragma once

#include <toolbox/setting_provider.h>

typedef enum {
    MqttSavedStateV1IdxClientId,
    MqttSavedStateV1IdxSessionId,
    MqttSavedStateV1IdxUserId,
    MqttSavedStateV1IdxEmail,
    MqttSavedStateV1IdxToken,
    MqttSavedStateV1IdxMax,
} MqttSavedStateV1Idx;

typedef struct {
    FuriString* client_id;
    FuriString* session_id;
    FuriString* user_id;
    FuriString* email;
    FuriString* token;
} MqttSavedStateV1;

extern const SettingProviderSetting mqtt_saved_state_v1[];
extern const SettingProviderSetting mqtt_saved_state_v1_root;

void mqtt_saved_state_v1_init(MqttSavedStateV1* saved_state_v1);

bool mqtt_saved_state_v1_is_valid(const MqttSavedStateV1* saved_state_v1);
