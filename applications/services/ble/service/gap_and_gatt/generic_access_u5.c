#include "generic_access_i.h"

#include "device_name/device_name.h"

#define BLE_GENERIC_WATCH_APPEARANCE (0x00C0)

static void ble_on_name_change_callback_state(const void* item, void* context) {
    BleServiceObject* instance = context;

    const DeviceNameInfo* device_name_info = item;
    ble_service_enqueue_run_with_data(instance, DEVICE_NAME_MAX_SIZE, device_name_info);
}

static void ble_subscribe_on_name_change(BleServiceObject* instance) {
    DeviceName* name_record = furi_record_open(RECORD_DEVICE_NAME);
    FuriState* state = device_name_get_state(name_record);

    DeviceNameInfo device_name_info;
    furi_state_get_subscribe(
        state, &device_name_info, ble_on_name_change_callback_state, instance);

    BleCharacteristicObject* ch = instance->chars[BleGenericAccessCharacterDeviceName];
    ble_characteristic_set_data(ch, device_name_info.name, DEVICE_NAME_MAX_SIZE);

    furi_record_close(RECORD_DEVICE_NAME);
}

bool ble_service_generic_access_init(void* object) {
    BleServiceObject* instance = object;
    ble_subscribe_on_name_change(instance);

    uint16_t appearance = BLE_GENERIC_WATCH_APPEARANCE;
    BleCharacteristicObject* ch = instance->chars[BleGenericAccessCharacterAppearance];
    ble_characteristic_set_data(ch, &appearance, sizeof(appearance));

    return true;
}

bool ble_service_generic_access_run(void* object, size_t data_size, const void* data) {
    furi_assert(object);
    UNUSED(data_size);
    UNUSED(data);
    BleServiceObject* instance = object;

    if(data_size == 0 || data == NULL) {
        BLE_LOG_W("%s - data or size missing", instance->config->name);
        return false;
    }

    const DeviceNameInfo* name_info = data;
    const size_t len_with_zero = strlen(name_info->name) + 1;
    BleCharacteristicObject* ch = instance->chars[BleGenericAccessCharacterDeviceName];

    ble_characteristic_set_data(ch, name_info->name, len_with_zero);

    return true;
}
