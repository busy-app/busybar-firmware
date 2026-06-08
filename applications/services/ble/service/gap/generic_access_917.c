
#include "generic_access_i.h"

#include "ble/worker/ble_worker.h"

static void
    ble_service_generic_access_device_name_update(size_t data_size, void* data, void* context) {
    const char* name = data;
    ble_worker_set_name(name);

    ///TODO: Remove this block when chars will be maintained on our side, not nwp
    BleCharacteristicObject* ch = context;
    const uint16_t handle = ble_characteristic_get_handle(ch);
    const uint8_t cccd_value = ble_characteristic_get_cccd_value(ch);
    ble_worker_send(handle, data_size, data, cccd_value);
}

bool ble_service_generic_access_init(void* object) {
    UNUSED(object);
    BleServiceObject* service = object;
    BleCharacteristicObject* ch = service->chars[BleGenericAccessCharacterDeviceName];

    ble_characteristic_register_update_callback(
        ch, ble_service_generic_access_device_name_update, ch);
    return true;
}

bool ble_service_generic_access_run(void* object) {
    UNUSED(object);
    return true;
}
