#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FuriHalI2cBusHandle FuriHalI2cBusHandle;

/**
 * @brief Initialize BH1730 sensor
 * 
 * @param handle I2C bus handle
 * @return true on success, false otherwise
 */
bool bh1730_init(FuriHalI2cBusHandle* handle);

/**
 * @brief Read raw data from BH1730 sensor
 * BH1730 data0 samples 600nm wavelength
 *
 * @param handle I2C bus handle
 * @param value Pointer to store the read value
 * @return true on success, false otherwise
 */
bool bh1730_read_raw_data0(FuriHalI2cBusHandle* handle, uint16_t* value);

/**
 * @brief Read raw data from BH1730 sensor
 * BH1730 data1 samples 840nm wavelength
 *
 * @param handle I2C bus handle
 * @param value Pointer to store the read value
 * @return true on success, false otherwise
 */
bool bh1730_read_raw_data1(FuriHalI2cBusHandle* handle, uint16_t* value);

/** 
 * @brief Read lux value from BH1730 sensor
 * Reads data0 and data1 measurements and calculates lux value
 *
 * @param handle I2C bus handle
 * @param value Pointer to store the read value
 * @return true on success, false otherwise
 */
bool bh1730_read_lux(FuriHalI2cBusHandle* handle, float* value);

#ifdef __cplusplus
}
#endif
