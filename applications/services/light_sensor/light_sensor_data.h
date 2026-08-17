/**
 * @file light_sensor_data.h
 * @brief Data structure containing various light sensor values.
 */
#pragma once

#include "light_sensor_common.h"

#include <stdbool.h>

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
 * @brief Free a LightSensorData instance.
 *
 * @param[in,out] instance Pointer to the LightSensorData instance to be freed.
 */
void light_sensor_data_free(LightSensorData* instance);

/**
 * @brief Add a new light measurement.
 *
 * @param[in,out] instance Pointer to the LightSensorData instance.
 * @param[in] lux Light level in lux.
 *
 * @returns @c true if the measurement contributed to the mean value change, @c false otherwise
 */
bool light_sensor_data_add_measurement(LightSensorData* instance, float lux);

/**
 * @brief Get the current light sensor state.
 * @param[in] instance Pointer to the LightSensorData instance.
 * @param[out] state Pointer to the state object to be copied into (must be allocated).
 */
void light_sensor_data_get_state(const LightSensorData* instance, LightSensorState* state);

#ifdef __cplusplus
}
#endif
