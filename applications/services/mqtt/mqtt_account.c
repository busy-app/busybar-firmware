#include "mqtt_i.h"

static void mqtt_account_link_otp_message_callback(const MqttMessage* message, void* context) {
    furi_assert(message);
    furi_assert(context);
    Mqtt* instance = context;

    const struct mg_str json_str = TO_RAW_MESSAGE(message)->data;

    char* pin = mg_json_get_str(json_str, "$.code");
    double pin_expires_at = 0.;
    mg_json_get_num(json_str, "$.expires_at", &pin_expires_at);

    if(pin) {
        FURI_LOG_I(TAG, "Link PIN: %s", pin);
        MqttEvent pub_event = {
            .type = MqttEventTypeLinkPinReceived,
            .link_pin_received =
                {
                    .pin = pin,
                    .expires_at = (time_t)pin_expires_at,
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
        strlcpy(saved_state->session_id, session_id, sizeof(saved_state->session_id));
        strlcpy(saved_state->user_id, user_id, sizeof(saved_state->user_id));
        strlcpy(saved_state->email, email, sizeof(saved_state->email));
        strlcpy(saved_state->token, token, sizeof(saved_state->token));

        mqtt_saved_state_save(saved_state);

        MqttEvent pub_event = {
            .type = MqttEventTypeLinkDone,
        };

        furi_pubsub_publish(instance->event_pubsub, &pub_event);

        mqtt_connection_close(instance, true);
    }

    if(session_id) free(session_id);
    if(user_id) free(user_id);
    if(token) free(token);
    if(email) free(email);
}

static void mqtt_account_unlink_message_callback(const MqttMessage* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    Mqtt* instance = context;

    FURI_LOG_I(TAG, "Received unlink message from cloud");
    mqtt_account_unlink(instance);
}

void mqtt_account_init(Mqtt* instance) {
    mqtt_subscribe_internal(
        instance,
        MqttScopeDevice,
        MqttQosExactlyOnce,
        MQTT_TOPIC_LINK_PIN,
        mqtt_account_link_otp_message_callback,
        instance);

    mqtt_subscribe_internal(
        instance,
        MqttScopeDevice,
        MqttQosExactlyOnce,
        MQTT_TOPIC_LINK_DONE,
        mqtt_account_link_token_message_callback,
        instance);

    mqtt_subscribe_internal(
        instance,
        MqttScopeSession,
        MqttQosAtLeastOnce,
        MQTT_TOPIC_UNLINK_FROM_CLOUD,
        mqtt_account_unlink_message_callback,
        instance);
}

void mqtt_account_unlink(Mqtt* instance) {
    furi_assert(instance);

    mqtt_connection_close(instance, true);
    mqtt_reset_saved_state(instance);

    MqttEvent pub_event = {
        .type = MqttEventTypeUnlinked,
    };

    furi_pubsub_publish(instance->event_pubsub, &pub_event);
}
