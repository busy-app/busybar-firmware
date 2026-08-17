/**
 * @file light_sensor_common.h
 * @brief Common Light Sensor types and defines.
 */
#pragma once

#include <stdint.h>

/**
 * @brief Minimum possible light level value.
 */
#define LIGHT_SENSOR_LIGHT_LEVEL_MIN (0U)

/**
 * @brief Maximum possible light level value.
 */
#define LIGHT_SENSOR_LIGHT_LEVEL_MAX (15U)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Structure containing ambient brightness values in lux.
 */
typedef struct {
    float mean; /**< Mean (sliding average) brightness value. */
    float instant; /**< Last measured (instant) brightness value. */
} LightSensorLuxLevel;

/**
 * @brief Structure containing the quantised light level value.
 *
 * The light level value shall be between @ref LIGHT_SENSOR_LIGHT_LEVEL_MIN and
 * @ref LIGHT_SENSOR_LIGHT_LEVEL_MAX, inclusive.
 */
typedef struct LightSensorLevel {
    uint8_t val; /**< Quantised light level value. */
} LightSensorLevel;

/**
 * @brief Structure containing the light sensor state.
 */
typedef struct {
    LightSensorLuxLevel lux; /**< Ambient brightness values in lux */
    LightSensorLevel level; /**< Quantised light level */
} LightSensorState;

#ifdef __cplusplus
}
#endif
