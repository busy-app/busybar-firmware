/**
 * @file light_sensor.h
 * @brief Light service API.
*/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Record name for light sensor */
#define RECORD_LIGHT_SENSOR        "light_sensor"
/** Record name for light sensor events */
#define RECORD_LIGHT_SENSOR_EVENTS "light_sensor_events"

/** Light sensor light level min */
#define LIGHT_SENSOR_LIGHT_LEVEL_MIN (0U)
/** Light sensor light level max */
#define LIGHT_SENSOR_LIGHT_LEVEL_MAX (15U)

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
 * @brief Light sensor event types.
*/
typedef enum {
    LightSensorEventTypeLightLevelChanged, /**< Light level changed event */
} LightSensorEventType;

typedef struct LightSensorLevel {
    uint8_t val;
} LightSensorLevel;

/**
 * @brief Light sensor event structure.
*/
typedef struct {
    LightSensorEventType type; /**< Type of the event */
    LightSensorLevel light_level; /**< Current light level */
} LightSensorEvent;

/**
 * @brief Get the mean across measurement window lux value.
 * @note Must be called after RECORD_LIGHT_SENSOR_EVENTS is created.
 *
 * @param[in] instance pointer to the LightSensor instance.
 * @return Current light level in lux.
 */
float light_sensor_get_lux(LightSensor* instance);

/**
 * @brief Get the instant light level in lux.
 * @note Must be called after RECORD_LIGHT_SENSOR_EVENTS is created.
 *
 * @param[in] instance pointer to the LightSensor instance.
 * @return Instant light level in lux.
 */
float light_sensor_get_lux_instant(LightSensor* instance);

/**
 * @brief Get the current light level.
 * @note Must be called after RECORD_LIGHT_SENSOR_EVENTS is created.
 *
 * @param[in] instance pointer to the LightSensor instance.
 * @return Current light level.
 */
LightSensorLevel light_sensor_get_light_level(LightSensor* instance);

/**
 * @brief Get the raw light sensor value.
 * @note Must be called after RECORD_LIGHT_SENSOR_EVENTS is created.
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
