#include "mqtt_saved_state_interface_v1.h"

bool mqtt_saved_state_v1_is_valid(const MqttSavedStateV1* saved_state_v1) {
    return *saved_state_v1->client_id != '\0' && *saved_state_v1->session_id != '\0' &&
           *saved_state_v1->user_id != '\0' && *saved_state_v1->email != '\0' &&
           *saved_state_v1->token != '\0';
}

const SettingProviderSetting mqtt_saved_state_v1[] = {
    [MqttSavedStateV1IdxClientId] =
        {
            .name = "client_id",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = "",
                    .max_size = SIZEOF_MEMBER(MqttSavedStateV1, client_id),
                },
            .field_offset = offsetof(MqttSavedStateV1, client_id),
            .type = SettingProviderSettingTypeString,
        },
    [MqttSavedStateV1IdxSessionId] =
        {
            .name = "session_id",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = "",
                    .max_size = SIZEOF_MEMBER(MqttSavedStateV1, session_id),
                },
            .field_offset = offsetof(MqttSavedStateV1, session_id),
            .type = SettingProviderSettingTypeString,
        },
    [MqttSavedStateV1IdxUserId] =
        {
            .name = "user_id",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = "",
                    .max_size = SIZEOF_MEMBER(MqttSavedStateV1, user_id),
                },
            .field_offset = offsetof(MqttSavedStateV1, user_id),
            .type = SettingProviderSettingTypeString,
        },
    [MqttSavedStateV1IdxEmail] =
        {
            .name = "email",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = "",
                    .max_size = SIZEOF_MEMBER(MqttSavedStateV1, email),
                },
            .field_offset = offsetof(MqttSavedStateV1, email),
            .type = SettingProviderSettingTypeString,
        },
    [MqttSavedStateV1IdxToken] =
        {
            .name = "token",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = "",
                    .max_size = SIZEOF_MEMBER(MqttSavedStateV1, token),
                },
            .field_offset = offsetof(MqttSavedStateV1, token),
            .type = SettingProviderSettingTypeString,
        },
};

const SettingProviderSetting mqtt_saved_state_v1_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = mqtt_saved_state_v1,
            .inner_settings_count = COUNT_OF(mqtt_saved_state_v1),
        },
    .type = SettingProviderSettingTypeStruct,
};

static_assert(COUNT_OF(mqtt_saved_state_v1) == MqttSavedStateV1IdxMax);
