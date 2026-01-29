#include "mqtt_saved_state_interface_v1.h"

void mqtt_saved_state_v1_init(MqttSavedStateV1* saved_state_v1) {
    furi_assert(saved_state_v1->client_id == NULL);
    furi_assert(saved_state_v1->session_id == NULL);
    furi_assert(saved_state_v1->user_id == NULL);
    furi_assert(saved_state_v1->email == NULL);
    furi_assert(saved_state_v1->token == NULL);

    saved_state_v1->client_id = furi_string_alloc();
    saved_state_v1->session_id = furi_string_alloc();
    saved_state_v1->user_id = furi_string_alloc();
    saved_state_v1->email = furi_string_alloc();
    saved_state_v1->token = furi_string_alloc();
}

bool mqtt_saved_state_v1_is_valid(const MqttSavedStateV1* saved_state_v1) {
    return !furi_string_empty(saved_state_v1->client_id) &&
           !furi_string_empty(saved_state_v1->session_id) &&
           !furi_string_empty(saved_state_v1->user_id) &&
           !furi_string_empty(saved_state_v1->email) && !furi_string_empty(saved_state_v1->token);
}

static const SettingProviderFuriStringInterface mqtt_saved_state_v1_default_string_interface = {
    .default_value = "",
    .is_valid_callback = NULL,
};

const SettingProviderSetting mqtt_saved_state_v1[] = {
    [MqttSavedStateV1IdxClientId] =
        {
            .name = "client_id",
            .interface = &mqtt_saved_state_v1_default_string_interface,
            .field_offset = offsetof(MqttSavedStateV1, client_id),
            .type = SettingProviderSettingTypeFuriString,
        },
    [MqttSavedStateV1IdxSessionId] =
        {
            .name = "session_id",
            .interface = &mqtt_saved_state_v1_default_string_interface,
            .field_offset = offsetof(MqttSavedStateV1, session_id),
            .type = SettingProviderSettingTypeFuriString,
        },
    [MqttSavedStateV1IdxUserId] =
        {
            .name = "user_id",
            .interface = &mqtt_saved_state_v1_default_string_interface,
            .field_offset = offsetof(MqttSavedStateV1, user_id),
            .type = SettingProviderSettingTypeFuriString,
        },
    [MqttSavedStateV1IdxEmail] =
        {
            .name = "email",
            .interface = &mqtt_saved_state_v1_default_string_interface,
            .field_offset = offsetof(MqttSavedStateV1, email),
            .type = SettingProviderSettingTypeFuriString,
        },
    [MqttSavedStateV1IdxToken] =
        {
            .name = "token",
            .interface = &mqtt_saved_state_v1_default_string_interface,
            .field_offset = offsetof(MqttSavedStateV1, token),
            .type = SettingProviderSettingTypeFuriString,
        },
};

const SettingProviderSetting mqtt_saved_state_v1_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructureInterface){
            .inner_settings = mqtt_saved_state_v1,
            .inner_settings_count = COUNT_OF(mqtt_saved_state_v1),
        },
    .type = SettingProviderSettingTypeStructure,
};

static_assert(COUNT_OF(mqtt_saved_state_v1) == MqttSavedStateV1IdxMax);
