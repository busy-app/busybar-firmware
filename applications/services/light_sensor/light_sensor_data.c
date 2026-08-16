#include "light_sensor_data.h"
#include "light_sensor_common.h"

#include <math.h>

#include <core/check.h>
#include <core/log.h>

#define TAG "LightSensorData"

#define LIGHT_SENSOR_DATA_LUX_MIN     (1.0f)
#define LIGHT_SENSOR_DATA_LUX_MAX     (10000.0f)
#define LIGHT_SENSOR_DATA_WINDOW_SIZE (5)

struct LightSensorData {
    float measurements[LIGHT_SENSOR_DATA_WINDOW_SIZE];
    size_t measurement_index;

    float lux_mean;
    float lux_instant;
    uint8_t light_level;
};

static void light_sensor_data_update_light_level(LightSensorData* instance) {
    const float lux =
        CLAMP(instance->lux_mean, LIGHT_SENSOR_DATA_LUX_MAX, LIGHT_SENSOR_DATA_LUX_MIN);
    /*
     * Using logarithmic mapping: level = a * log(lux) + b
     * Where a and b are constants chosen to map:
     * lux_min -> level 0
     * lux_max -> level max
     */
    const float log_min = logf(LIGHT_SENSOR_DATA_LUX_MIN);
    const float log_max = logf(LIGHT_SENSOR_DATA_LUX_MAX);
    const float a = LIGHT_SENSOR_LIGHT_LEVEL_MAX / (log_max - log_min);
    const float b = -a * log_min;

    const float light_level = a * logf(lux) + b;

    instance->light_level = CLAMP(
        (uint8_t)roundf(light_level), LIGHT_SENSOR_LIGHT_LEVEL_MAX, LIGHT_SENSOR_LIGHT_LEVEL_MIN);

    FURI_LOG_T(TAG, "Light mapping: lux=%.2f -> level=%d", lux, instance->light_level);
}

LightSensorData* light_sensor_data_alloc(void) {
    LightSensorData* instance = malloc(sizeof(LightSensorData));

    for(size_t i = 0; i < LIGHT_SENSOR_DATA_WINDOW_SIZE; i++) {
        instance->measurements[i] = LIGHT_SENSOR_DATA_LUX_MIN;
    }
    instance->measurement_index = 0;

    instance->lux_mean = LIGHT_SENSOR_DATA_LUX_MIN;
    instance->lux_instant = LIGHT_SENSOR_DATA_LUX_MIN;
    instance->light_level = LIGHT_SENSOR_LIGHT_LEVEL_MIN;

    return instance;
}

void light_sensor_data_free(LightSensorData* instance) {
    furi_check(instance);

    free(instance->measurements);
    free(instance);
}

void light_sensor_data_add_measurement(LightSensorData* instance, float lux) {
    furi_check(instance);

    instance->lux_instant = lux;

    instance->measurements[instance->measurement_index] = lux;
    instance->measurement_index =
        (instance->measurement_index + 1) % LIGHT_SENSOR_DATA_WINDOW_SIZE;

    float lux_sum = 0.0f;
    for(size_t i = 0; i < LIGHT_SENSOR_DATA_WINDOW_SIZE; i++) {
        lux_sum += instance->measurements[i];
    }
    instance->lux_mean = lux_sum / LIGHT_SENSOR_DATA_WINDOW_SIZE;

    light_sensor_data_update_light_level(instance);

    FURI_LOG_T(
        TAG,
        "new Lux: %.2f. New sum: %.2f. New mean: %.2f. level: %d",
        lux,
        lux_sum,
        instance->lux_mean,
        instance->light_level);
}

float light_sensor_data_get_lux(LightSensorData* instance) {
    furi_check(instance);

    return instance->lux_mean;
}

float light_sensor_data_get_lux_instant(LightSensorData* instance) {
    furi_check(instance);

    return instance->lux_instant;
}

uint8_t light_sensor_data_get_light_level(LightSensorData* instance) {
    furi_check(instance);

    return instance->light_level;
}
