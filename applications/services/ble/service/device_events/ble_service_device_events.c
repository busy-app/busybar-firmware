#include "ble_service_device_events.h"

#include "../ble_service_i.h"
#include "device_name/device_name.h"

#define TAG "BleDevEvents"

typedef union {
    uint32_t raw;
    struct {
        bool name_changed : 1;
        bool test_flag_1  : 1;
        bool test_flag_2  : 1;
    } flags;
} BleServiceDeviceEvents;

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
static const BleCharacteristicDescriptor device_events_service_characteristics[] = {
    {
        .intercom_index = BleSrvDeviceEventsCharacterIndexFlags,
        .name = "Event Flags",
#if defined(SI917)
        .uuid = {.Char_UUID_128 = EVENT_SERVICE_FLAGS_CHAR_UUID},
        .uuid_size = 16,
        ///TODO: maybe BLE_ATT_PROPERTY_READ is optional in current context, and can be removed
        .char_properties = BLE_ATT_PROPERTY_READ | BLE_ATT_PROPERTY_INDICATE,
#endif
    },
};

#if defined(SI917)
static bool ble_service_device_events_init_917(void* object) {
    UNUSED(object);
    BLE_LOG_D("device_events_init_917");
    return true;
}
#else

static void ble_device_event_on_name_changed(const void* message, void* context) {
    UNUSED(message);
    BleServiceObject* instance = context;
    if(ble_service_lock(instance)) {
        BleServiceDeviceEventContext* ctx = instance->context;
        furi_mutex_acquire(ctx->lock, FuriWaitForever);
        ctx->shadow.flags.name_changed = true;
        furi_mutex_release(ctx->lock);

        ble_service_enqueue_run(instance);
        ble_service_unlock(instance);
    }
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
            ctx->shadow.flags.test_flag_1 = true;
        else if(ctx->cout == 1)
            ctx->shadow.flags.test_flag_2 = true;
        else if(ctx->cout == 2) {
            ctx->shadow.flags.test_flag_1 = true;
            ctx->shadow.flags.test_flag_2 = true;
        }
        furi_mutex_release(ctx->lock);

        if(ctx->cout < 3) ble_service_enqueue_run(instance);
        ctx->cout++;
        ble_service_unlock(instance);
    }
}

static bool ble_service_device_events_init_u5(void* object) {
    BLE_LOG_I("device_events_init_u5");
    BleServiceObject* instance = object;

    BleServiceDeviceEventContext* ctx = malloc(sizeof(BleServiceDeviceEventContext));
    ctx->lock = furi_mutex_alloc(FuriMutexTypeNormal);
    ctx->timer = furi_timer_alloc(timer_cb, FuriTimerTypeOnce, instance);
    ctx->shadow.raw = 0;
    instance->context = ctx;

    DeviceName* device_name = furi_record_open(RECORD_DEVICE_NAME);
    FuriPubSub* pub_sub = device_name_get_on_change_pub_sub(device_name);
    furi_pubsub_subscribe(pub_sub, ble_device_event_on_name_changed, instance);
    furi_record_close(RECORD_DEVICE_NAME);

    BleCharacteristicObject* ch = instance->chars[BleSrvDeviceEventsCharacterIndexFlags];

    furi_mutex_acquire(ctx->lock, FuriWaitForever);
    ble_characteristic_set_data(ch, &ctx->shadow, sizeof(BleServiceDeviceEvents));
    ctx->shadow.raw = 0;
    furi_mutex_release(ctx->lock);

    ble_characteristic_register_tx_done_callback(ch, ble_service_device_events_tx_done, instance);

    return true;
}

static bool ble_service_device_events_run(void* object) {
    BLE_LOG_I("ble_service_device_events_run");
    BleServiceObject* instance = object;
    BleServiceDeviceEventContext* ctx = instance->context;
    BleCharacteristicObject* ch = instance->chars[BleSrvDeviceEventsCharacterIndexFlags];

    furi_mutex_acquire(ctx->lock, FuriWaitForever);
    if(ctx->shadow.raw != 0) {
        ble_characteristic_set_data(ch, &ctx->shadow, sizeof(BleServiceDeviceEvents));
        ctx->shadow.raw = 0;
    }
    furi_mutex_release(ctx->lock);

    return true;
}

#endif

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
