#include "generic_access_i.h"

#include "device_name/device_name.h"

#define BLE_GENERIC_WATCH_APPEARANCE (0x00C0)

// TODO: Refactor to use device name value provided in callback
static void ble_on_name_change_callback(const void* message, void* context) {
    UNUSED(message);
    BleServiceObject* instance = context;
    ble_service_enqueue_run(instance);
}

static void ble_subscribe_on_name_change(BleServiceObject* instance) {
    DeviceName* name_record = furi_record_open(RECORD_DEVICE_NAME);
    FuriState* pubsub = device_name_get_state(name_record);
    furi_state_get_subscribe(pubsub, NULL, ble_on_name_change_callback, instance);
    furi_record_close(RECORD_DEVICE_NAME);
}

static void ble_get_name_from_record(FuriString* output) {
    DeviceName* name_record = furi_record_open(RECORD_DEVICE_NAME);
    device_name_get(name_record, output);
    furi_record_close(RECORD_DEVICE_NAME);
}

static void ble_service_generic_access_update_device_name_char(BleServiceObject* instance) {
    FuriString* name = furi_string_alloc();
    ble_get_name_from_record(name);

    char* buf = malloc(DEVICE_NAME_MAX_SIZE);
    memcpy(buf, furi_string_get_cstr(name), furi_string_size(name));

    BleCharacteristicObject* ch = instance->chars[BleGenericAccessCharacterDeviceName];
    ble_characteristic_set_data(ch, buf, DEVICE_NAME_MAX_SIZE);
    furi_string_free(name);
    free(buf);
}

bool ble_service_generic_access_init(void* object) {
    BleServiceObject* instance = object;

    ble_subscribe_on_name_change(instance);
    ble_service_generic_access_update_device_name_char(instance);

    uint16_t appearance = BLE_GENERIC_WATCH_APPEARANCE;
    BleCharacteristicObject* ch = instance->chars[BleGenericAccessCharacterAppearance];
    ble_characteristic_set_data(ch, &appearance, sizeof(appearance));

    return true;
}

bool ble_service_generic_access_run(void* object) {
    furi_assert(object);
    BleServiceObject* instance = object;

    ble_service_generic_access_update_device_name_char(instance);
    return true;
}
