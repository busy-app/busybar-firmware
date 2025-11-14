#include "ble_service_device_events_i.h"
#include "device_name/device_name.h"

#define TAG "BleDeviceEvents"

#define WAIT_TX_DONE_TIMEOUT (500)

static BleServiceObject* event_context;

typedef struct {
    bool waiting_tx_done;
    bool run_enqueued;
    FuriMutex* lock;
    FuriTimer* wait_tx_timer;
    BleServiceDeviceEvents shadow;
    BleServiceDeviceEvents sent_shadow;
} BleServiceDeviceEventContext;

typedef FuriPubSub* (*BleServiceDeviceEventFlagGetPubSubCallback)(void);

typedef enum {
    BleServiceDeviceEventFlagBitNameChange = 0, /* Device name changed flag */
    /* Add more flags here */
    BleServiceDeviceEventFlagBitCount, /* Total flags count */
    BleServiceDeviceEventFlagBitMax = 32, /* Max flags possible */
} BleServiceDeviceEventFlagBit;

static_assert(BleServiceDeviceEventFlagBitCount <= BleServiceDeviceEventFlagBitMax);

static FuriPubSub* ble_service_device_name_get_pubsub() {
    DeviceName* device_name = furi_record_open(RECORD_DEVICE_NAME);
    FuriPubSub* pubsub = device_name_get_pubsub(device_name);
    furi_record_close(RECORD_DEVICE_NAME);
    return pubsub;
}

//Add proper get_pubsub callback for your flag here
static const BleServiceDeviceEventFlagGetPubSubCallback
    pubsub_callbacks[BleServiceDeviceEventFlagBitCount] = {
        [BleServiceDeviceEventFlagBitNameChange] = ble_service_device_name_get_pubsub,
};

static void ble_device_events_enqueue_run(BleServiceObject* instance) {
    BleServiceDeviceEventContext* ctx = instance->context;
    if(!ctx->run_enqueued) {
        ble_service_enqueue_run(instance);
        ctx->run_enqueued = true;
    }
}

static void
    ble_device_events_set_flag(BleServiceObject* instance, BleServiceDeviceEventFlagBit flag_bit) {
    furi_assert(instance);
    furi_assert(flag_bit < BleServiceDeviceEventFlagBitCount);
    BLE_LOG_D("device_events_set_flag");
    if(ble_service_lock(instance)) {
        BleServiceDeviceEventContext* ctx = instance->context;
        furi_mutex_acquire(ctx->lock, FuriWaitForever);
        ctx->shadow |= (1 << flag_bit);

        if(!ctx->waiting_tx_done) {
            ble_device_events_enqueue_run(instance);
        }
        furi_mutex_release(ctx->lock);

        ble_service_unlock(instance);
    }
}

static void ble_device_events_callback(const void* message, void* context) {
    UNUSED(message);
    const uint32_t bit = (uint32_t)context;
    ble_device_events_set_flag(event_context, bit);
}

static void ble_service_device_events_tx_done(void* context) {
    BLE_LOG_D("device_events_tx_done");
    BleServiceObject* instance = context;
    BleServiceDeviceEventContext* ctx = instance->context;

    furi_mutex_acquire(ctx->lock, FuriWaitForever);
    furi_timer_stop(ctx->wait_tx_timer);

    ctx->waiting_tx_done = false;

    ble_device_events_enqueue_run(instance);

    furi_mutex_release(ctx->lock);
}

static void ble_service_device_events_subscribe(BleServiceObject* instance) {
    event_context = instance;
    for(uint8_t i = 0; i < BleServiceDeviceEventFlagBitCount; i++) {
        BleServiceDeviceEventFlagGetPubSubCallback get_pubsub = pubsub_callbacks[i];
        furi_assert(get_pubsub);
        FuriPubSub* pubsub = get_pubsub();
        uint32_t bit = i;
        furi_pubsub_subscribe(pubsub, ble_device_events_callback, (void*)bit);
    }
}

static void ble_service_device_events_wait_tx_done_timeout(void* context) {
    BleServiceObject* instance = context;
    BleServiceDeviceEventContext* ctx = instance->context;

    furi_mutex_acquire(ctx->lock, FuriWaitForever);
    ctx->waiting_tx_done = false;
    ctx->shadow |= ctx->sent_shadow;
    furi_mutex_release(ctx->lock);
    BLE_LOG_W("Events restored");
}

bool ble_service_device_events_init(void* object) {
    BleServiceObject* instance = object;

    BleServiceDeviceEventContext* ctx = malloc(sizeof(BleServiceDeviceEventContext));
    ctx->lock = furi_mutex_alloc(FuriMutexTypeNormal);
    ctx->wait_tx_timer = furi_timer_alloc(
        ble_service_device_events_wait_tx_done_timeout, FuriTimerTypeOnce, instance);
    ctx->shadow = 0;
    instance->context = ctx;

    ble_service_device_events_subscribe(instance);

    BleCharacteristicObject* ch = instance->chars[BleSrvDeviceEventsCharacterIndexFlags];
    ble_characteristic_register_tx_done_callback(ch, ble_service_device_events_tx_done, instance);

    return true;
}

bool ble_service_device_events_run(void* object) {
    BLE_LOG_D("ble_service_device_events_run");
    BleServiceObject* instance = object;
    BleServiceDeviceEventContext* ctx = instance->context;
    BleCharacteristicObject* ch = instance->chars[BleSrvDeviceEventsCharacterIndexFlags];

    furi_mutex_acquire(ctx->lock, FuriWaitForever);
    if(ctx->shadow != 0) {
        ble_characteristic_set_data(ch, &ctx->shadow, sizeof(BleServiceDeviceEvents));
        ctx->sent_shadow = ctx->shadow;
        ctx->shadow = 0;
        ctx->waiting_tx_done = true;
        furi_timer_start(ctx->wait_tx_timer, furi_ms_to_ticks(WAIT_TX_DONE_TIMEOUT));
    }
    ctx->run_enqueued = false;

    furi_mutex_release(ctx->lock);

    return true;
}
