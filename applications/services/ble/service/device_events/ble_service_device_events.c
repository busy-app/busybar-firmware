#include "ble_service_device_events.h"

#include "../ble_service_i.h"
#include "device_name/device_name.h"

#define TAG "BleDevEvents"

typedef uint32_t BleServiceDeviceEvents;

typedef enum {
    BleServiceDeviceEventFlagBitNameChange = 0,
    BleServiceDeviceEventFlagBitCount
} BleServiceDeviceEventFlagBit;

typedef struct {
    BleServiceDeviceEventFlagBit flag_bit;
    BleServiceObject* instance;
} BleDeviceEventSubscriptionContext;

typedef struct {
    FuriMutex* lock;
    FuriTimer* timer; ///TODO: remove after test
    uint8_t cout; ///TODO: remove after test
    BleServiceDeviceEvents shadow;
} BleServiceDeviceEventContext;

typedef enum {
    BleSrvDeviceEventsCharacterIndexFlags,
} BleSrvDeviceEventsCharacterIndex;

#define UUID_VALUE(i) \
    {0xAF, 0x56, 0x9d, i, 0x71, 0x6a, 0x45, 0x2d, 0xbe, 0x64, 0x66, 0xe4, 0x65, 0x76, 0x6c, 0x29}

#define EVENT_SERVICE_UUID UUID_VALUE(0)

#define EVENT_SERVICE_FLAGS_CHAR_UUID UUID_VALUE(1)

//==========================================================

#if defined(SI917)
static bool ble_service_device_events_init_917(void* object) {
    UNUSED(object);
    BLE_LOG_D("device_events_init_917");
    return true;
}
#else

static void
    ble_device_events_set_flag(BleServiceObject* instance, BleServiceDeviceEventFlagBit flag_bit) {
    furi_assert(flag_bit < BleServiceDeviceEventFlagBitCount);

    if(ble_service_lock(instance)) {
        BleServiceDeviceEventContext* ctx = instance->context;
        furi_mutex_acquire(ctx->lock, FuriWaitForever);
        ctx->shadow |= (1 << flag_bit);
        furi_mutex_release(ctx->lock);

        ble_service_enqueue_run(instance);
        ble_service_unlock(instance);
    }
}

static void ble_device_events_callback(const void* message, void* context) {
    UNUSED(message);
    BleDeviceEventSubscriptionContext* ctx = context;
    ble_device_events_set_flag(ctx->instance, ctx->flag_bit);
}

static void ble_service_device_events_tx_done(void* context) {
    UNUSED(context);
    BLE_LOG_I("device_events_tx_done");
    BleServiceObject* instance = context;
    BleServiceDeviceEventContext* ctx = instance->context;
    furi_timer_start(ctx->timer, furi_ms_to_ticks(500));

    ble_service_enqueue_run(context);
}

void timer_cb(void* context) {
    BLE_LOG_I("timer_cb");
    BleServiceObject* instance = context;
    if(ble_service_lock(instance)) {
        BleServiceDeviceEventContext* ctx = instance->context;
        furi_mutex_acquire(ctx->lock, FuriWaitForever);

        if(ctx->cout == 0)
            ctx->shadow |= (1 << 1);
        else if(ctx->cout == 1)
            ctx->shadow |= (1 << 2);
        else if(ctx->cout == 2) {
            ctx->shadow |= ((1 << 3) | (1 << 2) | (1 << 1));
        }
        furi_mutex_release(ctx->lock);

        if(ctx->cout < 3) ble_service_enqueue_run(instance);
        ctx->cout++;
        ble_service_unlock(instance);
    }
}

typedef FuriPubSub* (*BleServiceDeviceEventFlagGetPubSubCallback)(void);

static FuriPubSub* ble_service_device_event_flag_get_pubsub() {
    DeviceName* device_name = furi_record_open(RECORD_DEVICE_NAME);
    FuriPubSub* pubsub = device_name_get_pubsub(device_name);
    furi_record_close(RECORD_DEVICE_NAME);
    return pubsub;
}

static const BleServiceDeviceEventFlagGetPubSubCallback
    pubsub_callbacks[BleServiceDeviceEventFlagBitCount] = {
        [BleServiceDeviceEventFlagBitNameChange] = ble_service_device_event_flag_get_pubsub,
};

static bool ble_service_device_events_init_u5(void* object) {
    BLE_LOG_I("device_events_init_u5");
    BleServiceObject* instance = object;

    BleServiceDeviceEventContext* ctx = malloc(sizeof(BleServiceDeviceEventContext));
    ctx->lock = furi_mutex_alloc(FuriMutexTypeNormal);
    ctx->timer = furi_timer_alloc(timer_cb, FuriTimerTypeOnce, instance);
    ctx->shadow = 0;
    instance->context = ctx;

    for(uint8_t i = 0; i < BleServiceDeviceEventFlagBitCount; i++) {
        BleServiceDeviceEventFlagGetPubSubCallback get_pubsub = pubsub_callbacks[i];
        FuriPubSub* pubsub = get_pubsub();
        BleDeviceEventSubscriptionContext* sub_context =
            malloc(sizeof(BleDeviceEventSubscriptionContext));
        sub_context->flag_bit = i;
        sub_context->instance = instance;
        furi_pubsub_subscribe(pubsub, ble_device_events_callback, sub_context);
    }

    // FuriPubSub* pubsub = ble_service_device_event_flag_get_pubsub();
    // furi_pubsub_subscribe(pubsub, ble_device_event_on_name_changed, instance);

    BleCharacteristicObject* ch = instance->chars[BleSrvDeviceEventsCharacterIndexFlags];

    ble_characteristic_register_tx_done_callback(ch, ble_service_device_events_tx_done, instance);

    return true;
}

static bool ble_service_device_events_run(void* object) {
    BLE_LOG_I("ble_service_device_events_run");
    BleServiceObject* instance = object;
    BleServiceDeviceEventContext* ctx = instance->context;
    BleCharacteristicObject* ch = instance->chars[BleSrvDeviceEventsCharacterIndexFlags];

    furi_mutex_acquire(ctx->lock, FuriWaitForever);
    if(ctx->shadow != 0) {
        ble_characteristic_set_data(ch, &ctx->shadow, sizeof(BleServiceDeviceEvents));
        ctx->shadow = 0;
    }
    furi_mutex_release(ctx->lock);

    return true;
}
#endif

static const BleCharacteristicDescriptor device_events_service_characteristics[] = {
    {
        .intercom_index = BleSrvDeviceEventsCharacterIndexFlags,
        .name = "Event Flags",
        .initial_data_size = sizeof(BleServiceDeviceEvents),
#if defined(SI917)
        .uuid = {.Char_UUID_128 = EVENT_SERVICE_FLAGS_CHAR_UUID},
        .uuid_size = 16,
        ///TODO: maybe BLE_ATT_PROPERTY_READ is optional in current context, and can be removed
        .char_properties = BLE_ATT_PROPERTY_READ | BLE_ATT_PROPERTY_INDICATE,
#endif
    },
};

const BleServiceDescriptor ble_service_config_device_events = {
    .name = "Device Events",
#if defined(SI917)
    .uuid = {.Char_UUID_128 = EVENT_SERVICE_UUID},
    .uuid_size = 16,
    .init = ble_service_device_events_init_917,
#else
    .init = ble_service_device_events_init_u5,
    .run = ble_service_device_events_run,
#endif
    .index = BleServiceIndexDeviceEvents,
    .init_method = BleServiceInitMethodRemote,
    .char_count = COUNT_OF(device_events_service_characteristics),
    .char_descriptors = device_events_service_characteristics,
};
