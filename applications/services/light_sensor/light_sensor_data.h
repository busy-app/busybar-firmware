/**
 * @file light_sensor_data.h
 * @brief Data structure containing various light sensor values.
 */
#pragma once

#include "light_sensor_common.h"

/**
 * @brief Minimum light intensity value, in lux.
 */
#define LIGHT_SENSOR_DATA_LUX_MIN (1.0f)

/**
 * @brief Maximum light intensity value, in lux.
 */
#define LIGHT_SENSOR_DATA_LUX_MAX (10000.0f)

/**
 * @brief Number of data points for the sliding average.
 */
#define LIGHT_SENSOR_DATA_WINDOW_SIZE (5)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Light sensor data structure.
 */
typedef struct LightSensorData LightSensorData;

/**
 * @brief Allocate a new LightSensorData instance.
 *
 * @returns Pointer to the allocated LightSensorData instance.
 */
LightSensorData* light_sensor_data_alloc(void);

/**
 * @brief Add a new light measurement.
 *
 * @param[in,out] instance Pointer to the LightSensorData instance.
 * @param[in] lux Light level in lux.
 */
void light_sensor_data_add_measurement(LightSensorData* instance, float lux);

/**
 * @brief Get the current light sensor state.
 * @param[in] instance Pointer to the LightSensorData instance.
 * @param[out] state Pointer to the state object to be copied into (must be allocated).
 */
void light_sensor_data_get_state(const LightSensorData* instance, LightSensorState* state);

#ifdef __cplusplus
}
#endif
