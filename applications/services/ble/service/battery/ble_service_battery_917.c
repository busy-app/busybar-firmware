#include "ble_service_battery_i.h"

#define TAG "BleBattery917"

bool ble_service_battery_init(void* object) {
    UNUSED(object);
    return true;
}

bool ble_service_battery_run(void* object, size_t data_size, const void* data) {
    UNUSED(object);
    UNUSED(data_size);
    UNUSED(data);
    return true;
}
