#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Light sensor data structure.
 */
typedef struct LightSensorData LightSensorData;

/**
 * @brief Configuration structure for LightSensorData.
 */
typedef struct {
    size_t window_size; /**< Size of the measurement window. The less value, the faster response */

    // Light level range
    uint8_t light_level_max; /**< Maximum light level index */

    // Light mapping parameters
    float lux_min; /**< Minimum lux value to map (default: 5.0) */
    float lux_max; /**< Maximum lux value to map (default: 15000.0) */
    bool use_logarithmic_mapping; /**< Whether to use logarithmic mapping (true) or linear (false) */
} LightSensorDataConfig;

/**
 * @brief Allocate a new LightSensorData instance.
 * 
 * @param config Configuration for the LightSensorData instance.
 * @return Pointer to the allocated LightSensorData instance.
 */
LightSensorData* light_sensor_data_alloc(const LightSensorDataConfig* config);

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

/**
 * @brief Get the mean light level in lux.
 * 
 * @param instance Pointer to the LightSensorData instance.
 * @return Mean light level in lux.
 */
float light_sensor_data_get_lux(LightSensorData* instance);

/**
 * @brief Get the instant light level in lux.
 * 
 * @param instance Pointer to the LightSensorData instance.
 * @return Instant light level in lux.
 */
float light_sensor_data_get_lux_instant(LightSensorData* instance);

/**
 * @brief Get the current light level.
 * 
 * @param instance Pointer to the LightSensorData instance.
 * @return Current light level.
 */
uint8_t light_sensor_data_get_light_level(LightSensorData* instance);

#ifdef __cplusplus
}
#endif
