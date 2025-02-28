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

/** Record name for light sensor events */
#define RECORD_LIGHT_SENSOR_EVENTS "light_sensor_events"

/** Light sensor light level max */
#define LIGHT_SENSOR_LIGHT_LEVEL_MAX (10U)

/**
 * @brief Light sensor event types.
*/
typedef enum {
    LightSensorEventTypeLightLevelIncreased, /**< Light level increased event */
    LightSensorEventTypeLightLevelDecreased, /**< Light level decreased event */
} LightSensorEventType;

/**
 * @brief Light sensor event structure.
*/
typedef struct {
    LightSensorEventType type; /**< Type of the event */
    uint8_t light_level_previous; /**< Previous light level */
    uint8_t light_level_current; /**< Current light level */
} LightSensorEvent;

/**
 * @brief Get the current light level in lux.
 * @note Must be called after RECORD_LIGHT_SENSOR_EVENTS is created.
 *
 * @return Current light level in lux.
 */
float light_sensor_get_lux(void);

/**
 * @brief Get the instant light level in lux.
 * @note Must be called after RECORD_LIGHT_SENSOR_EVENTS is created.
 *
 * @return Instant light level in lux.
 */
float light_sensor_get_lux_instant(void);

/**
 * @brief Get the current light level.
 * @note Must be called after RECORD_LIGHT_SENSOR_EVENTS is created.
 *
 * @return Current light level.
 */
uint8_t light_sensor_get_light_level(void);

/**
 * @brief Get the raw 600nm light sensor value.
 * @note Must be called after RECORD_LIGHT_SENSOR_EVENTS is created.
 *
 * @param[out] raw Raw 600nm light sensor value.
 * @return True if successful, false otherwise.
 */
bool light_sensor_get_raw_600nm(uint16_t* raw);

/**
 * @brief Get the raw 840nm light sensor value.
 * @note Must be called after RECORD_LIGHT_SENSOR_EVENTS is created.
 *
 * @param[out] raw Raw 840nm light sensor value.
 * @return True if successful, false otherwise.
 */
bool light_sensor_get_raw_840nm(uint16_t* raw);

#ifdef __cplusplus
}
#endif
