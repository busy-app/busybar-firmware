#include "furi_hal_light_sensor.h"

#include <drivers/bh1730/bh1730.h>

bool furi_hal_light_sensor_init(FuriHalI2cBusHandle* handle) {
    return bh1730_init(handle);
}

bool furi_hal_light_sensor_read_raw_600nm(FuriHalI2cBusHandle* handle, uint16_t* value) {
    return bh1730_read_raw_data0(handle, value);
}

bool furi_hal_light_sensor_read_raw_840nm(FuriHalI2cBusHandle* handle, uint16_t* value) {
    return bh1730_read_raw_data1(handle, value);
}

bool furi_hal_light_sensor_read_lux(FuriHalI2cBusHandle* handle, float* value) {
    return bh1730_read_lux(handle, value);
}
