
#include "generic_access_i.h"

#include "ble/worker/ble_worker.h"

static void
    ble_service_generic_access_device_name_update(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    BleCharacteristicObject* ch = context;

    const char* name = data;
    BleServiceObject* service = ble_characteristic_get_parent_service(ch);
    ble_worker_set_name(service->worker, name);
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
