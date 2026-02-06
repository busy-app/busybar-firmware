#include "account_model.h"
#include <mqtt/mqtt.h>

#define LINK_PIN_TIMEOUT (3000)

struct AccountModel {
    Mqtt* mqtt;
    FuriPubSubSubscription* mqtt_event_sub;
    MqttStatus status;

    FuriTimer* link_timeout_timer;

    AccountModelEventCallback callback;
    void* context;
};

static void account_model_event_callback(const void* message, void* context) {
    AccountModel* model = context;
    furi_assert(model);

    MqttEvent* mqtt_event = (MqttEvent*)message;
    furi_assert(mqtt_event);

    if(!model->callback) return;

    if(mqtt_event->type == MqttEventTypeLinkPinReceived) {
        furi_timer_stop(model->link_timeout_timer);
        model->callback(
            AccountModelEventPinGot,
            mqtt_event->link_pin_received.pin,
            mqtt_event->link_pin_received.expires_at,
            model->context);
    } else if(mqtt_event->type == MqttEventTypeLinkDone) {
        model->callback(AccountModelEventLinkDone, NULL, 0, model->context);
    } else if(mqtt_event->type == MqttEventTypeUnlinked) {
        model->callback(AccountModelEventUnlinked, NULL, 0, model->context);
    } else {
        model->callback(AccountModelEventStateChange, NULL, 0, model->context);
    }
}

static void account_model_link_timeout_callback(void* ctx) {
    AccountModel* model = ctx;
    if(model->callback) {
        model->callback(AccountModelEventPinTimeout, NULL, 0, model->context);
    }
}

AccountModel* account_model_alloc(void) {
    AccountModel* model = malloc(sizeof(AccountModel));
    model->mqtt = furi_record_open(RECORD_MQTT);
    model->mqtt_event_sub =
        furi_pubsub_subscribe(mqtt_get_pubsub(model->mqtt), account_model_event_callback, model);

    model->link_timeout_timer =
        furi_timer_alloc(account_model_link_timeout_callback, FuriTimerTypeOnce, model);

    return model;
}

void account_model_free(AccountModel* model) {
    furi_assert(model);
    furi_timer_free(model->link_timeout_timer);
    model->callback = NULL;
    furi_pubsub_unsubscribe(mqtt_get_pubsub(model->mqtt), model->mqtt_event_sub);
    furi_record_close(RECORD_MQTT);
    free(model);
}

void account_model_set_event_callback(
    AccountModel* model,
    AccountModelEventCallback callback,
    void* context) {
    furi_assert(model);
    model->context = context;
    model->callback = callback;
}

AccountModelState account_model_get_state(AccountModel* model) {
    MqttStatus status = mqtt_get_status(model->mqtt);
    if(status == MqttStatusConnectedLinked) {
        return AccountModelStateConnectedLinked;
    } else if(status == MqttStatusConnectedNotLinked) {
        return AccountModelStateConnectedNotLinked;
    }
    return AccountModelStateNotConnected;
}

bool account_model_is_linked(AccountModel* model) {
    MqttSessionInfo info = {.session_id = NULL, .email = NULL, .user_id = NULL};
    mqtt_get_session_info(model->mqtt, &info);
    return info.is_valid;
}

void account_model_get_email(AccountModel* model, FuriString* email) {
    furi_assert(email);
    MqttSessionInfo info = {.session_id = NULL, .email = email, .user_id = NULL};
    mqtt_get_session_info(model->mqtt, &info);
}

void account_model_unlink(AccountModel* model) {
    mqtt_unlink(model->mqtt);
}

void account_model_request_link_pin(AccountModel* model) {
    furi_timer_start(model->link_timeout_timer, furi_ms_to_ticks(LINK_PIN_TIMEOUT));
    mqtt_request_link_pin(model->mqtt);
}
