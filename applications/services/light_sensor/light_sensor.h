/**
 * @file light_sensor.h
 * @brief Light service API.
*/

#pragma once

#include <core/state.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Record name for light sensor */
#define RECORD_LIGHT_SENSOR "light_sensor"

/**
 * @brief LightSensor opaque pointer type.
 */
typedef struct LightSensor LightSensor;

/**
 * @brief Light sensor wavelengths.
 */
typedef enum {
    LightSensorLightWavelength600nm, /**< 600nm wavelength */
    LightSensorLightWavelength840nm, /**< 840nm wavelength */
} LightSensorLightWavelength;

/**
 * @brief Get the state object containing the current light levels.
 *
 * The underlying data will be of @ref LightSensorState type.
 *
 * @param[in] instance pointer to the LightSensor instance.
 * @return pointer to the state object
 */
FuriState* light_sensor_get_state(LightSensor* instance);

/**
 * @brief Get the raw light sensor value.
 *
 * @param[in] instance pointer to the LightSensor instance.
 * @param[in] wavelength Wavelength to read.
 * @param[out] raw Raw light sensor value.
 * @return True if successful, false otherwise.
 */
bool light_sensor_get_raw_data(
    LightSensor* instance,
    LightSensorLightWavelength wavelength,
    uint16_t* raw);

/**
 * @brief Enter low-power sleep mode.
 *
 * @param[in] instance pointer to the LightSensor instance.
 * @param[in] sleep true - sleep mode, false - normal operation mode.
 * @return True if successful, false otherwise.
 */
bool light_sensor_sleep(LightSensor* instance, bool sleep);

#ifdef __cplusplus
}
#endif
