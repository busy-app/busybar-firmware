#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "furi_hal_i2c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FuriHalLightSensorLightWavelength600nm,
    FuriHalLightSensorLightWavelength840nm,
} FuriHalLightSensorLightWavelength;

/**
 * @brief Initialize light sensor
 * 
 * @param handle I2C bus handle
 * @return true on success, false otherwise
 */
bool furi_hal_light_sensor_init(FuriHalI2cBusHandle* handle);

/**
 * @brief Read raw data from light sensor
 * Read sensor ADC samples for 600nm wavelength
 *
 * @param handle I2C bus handle
 * @param wavelength Wavelength to read
 * @param value Pointer to store the read value
 * @return true on success, false otherwise
 */
bool furi_hal_light_sensor_read_raw(
    FuriHalI2cBusHandle* handle,
    FuriHalLightSensorLightWavelength wavelength,
    uint16_t* value);

/** 
 * @brief Read lux value from light sensor
 * Reads raw data and converts it to lux value
 *
 * @param handle I2C bus handle
 * @param value Pointer to store the read value
 * @return true on success, false otherwise
 */
bool furi_hal_light_sensor_read_lux(FuriHalI2cBusHandle* handle, float* value);

#ifdef __cplusplus
}
#endif
