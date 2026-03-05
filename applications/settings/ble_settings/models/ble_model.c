#include "ble_model.h"
#include <device_name/device_name.h>
#include <busy_timer/time_macros.h>

#define TAG "BLE Model"

#define BLE_PAIRING_TIMEOUT_MIN (5)

struct BleModel {
    DeviceName* device_name;
    Ble* ble;
    FuriMutex* lock;
    BleState state;
    FuriPubSubSubscription* ble_subscription;
    FuriPubSubSubscription* device_name_subscription;
    FuriTimer* pairing_timer;

    BleModelStateCallback callback;
    void* context;
};

static void ble_model_on_state_change_callback(const void* message, void* context) {
    BleModel* model = context;
    furi_assert(model);

    const BleState* state = message;

    furi_mutex_acquire(model->lock, FuriWaitForever);
    memcpy(&model->state, state, sizeof(BleState));
    furi_timer_stop(model->pairing_timer);
    furi_mutex_release(model->lock);

    if(model->callback) {
        model->callback(BleModelStateEventBleChanged, model->context);
    }
}

static void ble_model_on_device_name_change_callback(const void* message, void* context) {
    BleModel* model = context;
    furi_assert(model);
    UNUSED(message);

    if(model->callback) {
        model->callback(BleModelStateEventNameChanged, model->context);
    }
}

static void ble_pairing_timeout_callback(void* ctx) {
    BleModel* model = ctx;

    if(model->callback) {
        model->callback(BleModelStateEventPairingTimeout, model->context);
    }
}

BleModel* ble_model_alloc(void) {
    BleModel* model = malloc(sizeof(BleModel));

    model->device_name = furi_record_open(RECORD_DEVICE_NAME);
    model->device_name_subscription = furi_pubsub_subscribe(
        device_name_get_pubsub(model->device_name),
        ble_model_on_device_name_change_callback,
        model);

    model->ble = furi_record_open(RECORD_BLE);
    model->lock = furi_mutex_alloc(FuriMutexTypeNormal);

    bool result = ble_get_state(model->ble, &model->state);
    furi_assert(result);

    FuriPubSub* pubsub = ble_get_pubsub(model->ble);
    model->ble_subscription =
        furi_pubsub_subscribe(pubsub, ble_model_on_state_change_callback, model);

    model->pairing_timer =
        furi_timer_alloc(ble_pairing_timeout_callback, FuriTimerTypeOnce, model);

    return model;
}

void ble_model_free(BleModel* model) {
    furi_assert(model);
    model->callback = NULL;

    furi_timer_free(model->pairing_timer);
    furi_pubsub_unsubscribe(
        device_name_get_pubsub(model->device_name), model->device_name_subscription);
    furi_pubsub_unsubscribe(ble_get_pubsub(model->ble), model->ble_subscription);
    furi_mutex_free(model->lock);
    furi_record_close(RECORD_BLE);
    furi_record_close(RECORD_DEVICE_NAME);
    free(model);
}

void ble_model_get_state(BleModel* model, BleState* output) {
    furi_assert(model);
    furi_assert(output);
    furi_mutex_acquire(model->lock, FuriWaitForever);
    memcpy(output, &model->state, sizeof(BleState));
    furi_mutex_release(model->lock);
}

bool ble_model_is_device_paired(BleModel* model) {
    furi_assert(model);
    furi_mutex_acquire(model->lock, FuriWaitForever);
    bool result =
        (model->state.status == BleServiceStatusConnected ||
         model->state.status == BleServiceStatusConnectable);
    furi_mutex_release(model->lock);
    return result;
}

void ble_model_get_name(BleModel* model, FuriString* name) {
    furi_assert(model);
    furi_assert(name);
    device_name_get(model->device_name, name);
}

void ble_model_set_state_callback(BleModel* model, BleModelStateCallback callback, void* context) {
    furi_assert(model);
    model->context = context;
    model->callback = callback;
}

void ble_model_start(BleModel* model) {
    furi_assert(model);
    bool result = ble_start(model->ble);
    furi_assert(result);
    furi_timer_start(model->pairing_timer, M_TO_MS(BLE_PAIRING_TIMEOUT_MIN));
}

void ble_model_stop(BleModel* model) {
    furi_assert(model);
    bool result = ble_stop(model->ble);
    furi_assert(result);
    furi_timer_stop(model->pairing_timer);
}

void ble_model_forget(BleModel* model) {
    furi_assert(model);
    bool result = ble_forget(model->ble);
    furi_assert(result);
}
