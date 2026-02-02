#include "furi_hal_light_sensor.h"

#include <drivers/bh1730/bh1730.h>

bool furi_hal_light_sensor_init(FuriHalI2cBusHandle* handle) {
    return bh1730_init(handle);
}

bool furi_hal_light_sensor_read_raw(
    FuriHalI2cBusHandle* handle,
    FuriHalLightSensorLightWavelength wavelength,
    uint16_t* value) {
    bool result = false;

    if(wavelength == FuriHalLightSensorLightWavelength600nm) {
        result = bh1730_read_raw_data0(handle, value);
    } else if(wavelength == FuriHalLightSensorLightWavelength840nm) {
        result = bh1730_read_raw_data1(handle, value);
    }

    return result;
}

bool furi_hal_light_sensor_read_lux(FuriHalI2cBusHandle* handle, float* value) {
    return bh1730_read_lux(handle, value);
}
