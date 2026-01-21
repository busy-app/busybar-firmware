#include "ble_model.h"
#include <device_name/device_name.h>

#define TAG "BLE Model"

struct BleModel {
    DeviceName* device_name;
    Ble* ble;
    FuriMutex* lock;
    BleStatus status;
    FuriPubSubSubscription* ble_subscription;
    FuriPubSubSubscription* device_name_subscription;

    BleModelStateCallback callback;
    void* context;
};

static void ble_model_on_state_change_callback(const void* message, void* context) {
    BleModel* model = context;
    furi_assert(model);

    const BleStatus* status = message;

    furi_mutex_acquire(model->lock, FuriWaitForever);
    memcpy(&model->status, status, sizeof(BleStatus));
    furi_mutex_release(model->lock);

    if(model->callback) {
        model->callback(BleModelStateEventBleChanged, model->context);
    }
}

static void ble_model_on_device_name_change_callback(const void* message, void* context) {
    BleModel* model = context;
    furi_assert(model);
    UNUSED(message);
    // FURI_LOG_W(TAG, "Here1");
    // device_name_get(model->device_name, model->name);

    if(model->callback) {
        model->callback(BleModelStateEventNameChanged, model->context);
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

    bool result = ble_get_status(model->ble, &model->status);
    furi_assert(result);

    FuriPubSub* pubsub = ble_get_pubsub(model->ble);
    model->ble_subscription =
        furi_pubsub_subscribe(pubsub, ble_model_on_state_change_callback, model);

    return model;
}

void ble_model_free(BleModel* model) {
    furi_assert(model);
    model->callback = NULL;

    furi_pubsub_unsubscribe(
        device_name_get_pubsub(model->device_name), model->device_name_subscription);
    furi_pubsub_unsubscribe(ble_get_pubsub(model->ble), model->ble_subscription);
    furi_mutex_free(model->lock);
    furi_record_close(RECORD_BLE);
    furi_record_close(RECORD_DEVICE_NAME);
    free(model);
}

void ble_model_get_status(BleModel* model, BleStatus* output) {
    furi_assert(model);
    furi_assert(output);
    furi_mutex_acquire(model->lock, FuriWaitForever);
    memcpy(output, &model->status, sizeof(BleStatus));
    furi_mutex_release(model->lock);
}

bool ble_model_is_device_paired(BleModel* model) {
    furi_assert(model);
    furi_mutex_acquire(model->lock, FuriWaitForever);
    bool result = model->status.pairing == BlePairingStatePaired;
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
}

void ble_model_stop(BleModel* model) {
    furi_assert(model);
    bool result = ble_stop(model->ble);
    furi_assert(result);
}

void ble_model_forget(BleModel* model) {
    furi_assert(model);
    bool result = ble_forget(model->ble);
    furi_assert(result);
}
