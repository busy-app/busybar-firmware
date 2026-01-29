#include "account_model.h"
#include <mqtt_client/mqtt_client.h>

#define LINK_PIN_TIMEOUT (3000)

struct AccountModel {
    MqttClient* mqtt;
    FuriPubSubSubscription* mqtt_event_sub;
    MqttClientStatus status;

    FuriTimer* link_timeout_timer;

    AccountModelEventCallback callback;
    void* context;
};

static void account_model_event_callback(const void* message, void* context) {
    AccountModel* model = context;
    furi_assert(model);

    MqttClientEvent* mqtt_event = (MqttClientEvent*)message;
    furi_assert(mqtt_event);

    if(!model->callback) return;

    if(mqtt_event->type == MqttClientEventLinkPin) {
        furi_timer_stop(model->link_timeout_timer);
        // TODO: PIN expire time
        model->callback(AccountModelEventPinGot, mqtt_event->link.pin, model->context);
    } else if(mqtt_event->type == MqttClientEventLinkDone) {
        model->callback(AccountModelEventLinkDone, NULL, model->context);
    } else if(mqtt_event->type == MqttClientEventUnlinked) {
        model->callback(AccountModelEventUnlinked, NULL, model->context);
    } else {
        model->callback(AccountModelEventStateChange, NULL, model->context);
    }
}

static void account_model_link_timeout_callback(void* ctx) {
    AccountModel* model = ctx;
    if(model->callback) {
        model->callback(AccountModelEventPinTimeout, NULL, model->context);
    }
}

AccountModel* account_model_alloc(void) {
    AccountModel* model = malloc(sizeof(AccountModel));
    model->mqtt = furi_record_open(RECORD_MQTT);
    model->mqtt_event_sub = furi_pubsub_subscribe(
        mqtt_client_get_pubsub(model->mqtt), account_model_event_callback, model);

    model->link_timeout_timer =
        furi_timer_alloc(account_model_link_timeout_callback, FuriTimerTypeOnce, model);

    return model;
}

void account_model_free(AccountModel* model) {
    furi_assert(model);
    furi_timer_free(model->link_timeout_timer);
    model->callback = NULL;
    furi_pubsub_unsubscribe(mqtt_client_get_pubsub(model->mqtt), model->mqtt_event_sub);
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
    MqttClientStatus status = mqtt_client_get_status(model->mqtt);
    if(status == MqttClientStatusConnectedLinked) {
        return AccountModelStateConnectedLinked;
    } else if(status == MqttClientStatusConnectedNotLinked) {
        return AccountModelStateConnectedNotLinked;
    }
    return AccountModelStateNotConnected;
}

bool account_model_is_linked(AccountModel* model) {
    return mqtt_client_is_linked(model->mqtt);
}

void account_model_get_email(AccountModel* model, FuriString* email) {
    furi_assert(email);
    mqtt_client_get_session_info(model->mqtt, NULL, email, NULL);
}

void account_model_unlink(AccountModel* model) {
    mqtt_client_unlink(model->mqtt);
}

void account_model_request_link_pin(AccountModel* model) {
    furi_timer_start(model->link_timeout_timer, furi_ms_to_ticks(LINK_PIN_TIMEOUT));
    mqtt_client_request_link_pin(model->mqtt);
}
