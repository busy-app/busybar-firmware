/**
 * @file light_sensor_common.h
 */
#pragma once

/** Light sensor light level min */
#define LIGHT_SENSOR_LIGHT_LEVEL_MIN (0.0f)
/** Light sensor light level max */
#define LIGHT_SENSOR_LIGHT_LEVEL_MAX (1.0f)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float mean;
    float instant;
} LightSensorLuxLevel;

typedef struct LightSensorLevel {
    float val;
} LightSensorLevel;

typedef struct {
    LightSensorLuxLevel lux;
    LightSensorLevel level;
} LightSensorState;

#ifdef __cplusplus
}
#endif
