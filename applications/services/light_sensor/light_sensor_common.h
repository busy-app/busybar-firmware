/**
 * @file light_sensor_common.h
 */
#pragma once

#include <stdint.h>

/** Light sensor light level min */
#define LIGHT_SENSOR_LIGHT_LEVEL_MIN (0U)
/** Light sensor light level max */
#define LIGHT_SENSOR_LIGHT_LEVEL_MAX (15U)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float mean;
    float instant;
} LightSensorLuxLevel;

typedef struct LightSensorLevel {
    uint8_t val;
} LightSensorLevel;

typedef struct {
    LightSensorLuxLevel lux;
    LightSensorLevel level;
} LightSensorState;

#ifdef __cplusplus
}
#endif
