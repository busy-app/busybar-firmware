#include "mqtt_i.h"

static void mqtt_account_link_otp_message_callback(const MqttMessage* message, void* context) {
    furi_assert(message);
    furi_assert(context);
    Mqtt* instance = context;

    const struct mg_str json_str = TO_RAW_MESSAGE(message)->data;

    char* pin = mg_json_get_str(json_str, "$.code");
    int32_t pin_expires_at = mg_json_get_long(json_str, "$.expires_at", -1);

    if(pin) {
        FURI_LOG_I(TAG, "Link PIN: %s", pin);
        MqttEvent pub_event = {
            .type = MqttEventTypeLinkPinReceived,
            .link_pin_received =
                {
                    .pin = pin,
                    .expires_at = pin_expires_at,
                },
        };

        furi_pubsub_publish(instance->event_pubsub, &pub_event);
        free(pin);
    }
}

static void mqtt_account_link_token_message_callback(const MqttMessage* message, void* context) {
    furi_assert(message);
    furi_assert(context);
    Mqtt* instance = context;

    const struct mg_str json_str = TO_RAW_MESSAGE(message)->data;

    char* session_id = mg_json_get_str(json_str, "$.session_id");
    char* token = mg_json_get_str(json_str, "$.token");
    char* email = mg_json_get_str(json_str, "$.email");
    char* user_id = mg_json_get_str(json_str, "$.user_id");

    if(session_id && token && email && user_id) {
        FURI_LOG_I(TAG, "Link done!");

        MqttSavedState* saved_state = &instance->saved_state;

        furi_string_set(saved_state->session_id, session_id);
        furi_string_set(saved_state->user_id, user_id);
        furi_string_set(saved_state->email, email);
        furi_string_set(saved_state->token, token);

        mqtt_saved_state_save(saved_state);

        MqttEvent pub_event = {
            .type = MqttEventTypeLinkDone,
        };

        furi_pubsub_publish(instance->event_pubsub, &pub_event);

        // Close MQTT connection to reconnect with new token
        instance->conn->is_draining = 1;
        instance->fast_reconnect = true;
    }

    if(session_id) free(session_id);
    if(user_id) free(user_id);
    if(token) free(token);
    if(email) free(email);
}

void mqtt_account_init(Mqtt* instance) {
    mqtt_subscribe_internal(
        instance,
        MqttScopeDevice,
        MqttQosExactlyOnce,
        "link/otp",
        mqtt_account_link_otp_message_callback,
        instance);

    mqtt_subscribe_internal(
        instance,
        MqttScopeDevice,
        MqttQosExactlyOnce,
        "link/token",
        mqtt_account_link_token_message_callback,
        instance);
}
