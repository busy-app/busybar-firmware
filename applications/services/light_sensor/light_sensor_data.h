#pragma once

#include "light_sensor_common.h"

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
 * @return Pointer to the allocated LightSensorData instance.
 */
LightSensorData* light_sensor_data_alloc(void);

/**
 * @brief Free a LightSensorData instance.
 * 
 * @param instance Pointer to the LightSensorData instance to be freed.
 */
void light_sensor_data_free(LightSensorData* instance);

/**
 * @brief Add a new light measurement.
 * 
 * @param instance Pointer to the LightSensorData instance.
 * @param lux Light level in lux.
 */
void light_sensor_data_add_measurement(LightSensorData* instance, float lux);

void light_sensor_data_get_state(const LightSensorData* instance, LightSensorState* state);

#ifdef __cplusplus
}
#endif
