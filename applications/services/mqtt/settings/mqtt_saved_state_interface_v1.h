#pragma once

#include <setting_provider.h>

#define MQTT_SAVED_STATE_CLIENT_ID_MAX_SIZE  (64 + 1)
#define MQTT_SAVED_STATE_SESSION_ID_MAX_SIZE (64 + 1)
#define MQTT_SAVED_STATE_USER_ID_MAX_SIZE    (64 + 1)
#define MQTT_SAVED_STATE_EMAIL_MAX_SIZE      (64 + 1)
#define MQTT_SAVED_STATE_TOKEN_MAX_SIZE      (512 + 1)

typedef enum {
    MqttSavedStateV1IdxClientId,
    MqttSavedStateV1IdxSessionId,
    MqttSavedStateV1IdxUserId,
    MqttSavedStateV1IdxEmail,
    MqttSavedStateV1IdxToken,
    MqttSavedStateV1IdxMax,
} MqttSavedStateV1Idx;

typedef struct {
    char client_id[MQTT_SAVED_STATE_CLIENT_ID_MAX_SIZE];
    char session_id[MQTT_SAVED_STATE_SESSION_ID_MAX_SIZE];
    char user_id[MQTT_SAVED_STATE_USER_ID_MAX_SIZE];
    char email[MQTT_SAVED_STATE_EMAIL_MAX_SIZE];
    char token[MQTT_SAVED_STATE_TOKEN_MAX_SIZE];
} MqttSavedStateV1;

extern const SettingProviderSetting mqtt_saved_state_v1[];
extern const SettingProviderSetting mqtt_saved_state_v1_root;

void mqtt_saved_state_v1_init(MqttSavedStateV1* saved_state_v1);

bool mqtt_saved_state_v1_is_valid(const MqttSavedStateV1* saved_state_v1);
