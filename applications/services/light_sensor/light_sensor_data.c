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
    LightSensorState state;
};

static void light_sensor_data_update_light_level(LightSensorData* instance) {
    LightSensorState* state = &instance->state;

    const float lux = CLAMP(state->lux.mean, LIGHT_SENSOR_DATA_LUX_MAX, LIGHT_SENSOR_DATA_LUX_MIN);
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

    state->level.val = CLAMP(
        (uint8_t)roundf(light_level), LIGHT_SENSOR_LIGHT_LEVEL_MAX, LIGHT_SENSOR_LIGHT_LEVEL_MIN);
}

LightSensorData* light_sensor_data_alloc(void) {
    LightSensorData* instance = malloc(sizeof(LightSensorData));

    for(size_t i = 0; i < LIGHT_SENSOR_DATA_WINDOW_SIZE; i++) {
        instance->measurements[i] = LIGHT_SENSOR_DATA_LUX_MIN;
    }
    instance->measurement_index = 0;

    instance->state = (const LightSensorState){
        .lux =
            {
                .mean = LIGHT_SENSOR_DATA_LUX_MIN,
                .instant = LIGHT_SENSOR_DATA_LUX_MIN,
            },
        .level = {LIGHT_SENSOR_LIGHT_LEVEL_MIN},
    };

    return instance;
}

void light_sensor_data_free(LightSensorData* instance) {
    furi_check(instance);

    free(instance->measurements);
    free(instance);
}

void light_sensor_data_add_measurement(LightSensorData* instance, float lux) {
    furi_check(instance);

    LightSensorLuxLevel* lux_level = &instance->state.lux;
    lux_level->instant = lux;

    instance->measurements[instance->measurement_index] = lux;
    instance->measurement_index =
        (instance->measurement_index + 1) % LIGHT_SENSOR_DATA_WINDOW_SIZE;

    float lux_sum = 0.0f;
    for(size_t i = 0; i < LIGHT_SENSOR_DATA_WINDOW_SIZE; i++) {
        lux_sum += instance->measurements[i];
    }
    lux_level->mean = lux_sum / LIGHT_SENSOR_DATA_WINDOW_SIZE;

    light_sensor_data_update_light_level(instance);
}

float light_sensor_data_get_lux(LightSensorData* instance) {
    furi_check(instance);

    return instance->state.lux.mean;
}

float light_sensor_data_get_lux_instant(LightSensorData* instance) {
    furi_check(instance);

    return instance->state.lux.instant;
}

uint8_t light_sensor_data_get_light_level(LightSensorData* instance) {
    furi_check(instance);

    return instance->state.level.val;
}

void light_sensor_data_get_state(const LightSensorData* instance, LightSensorState* state) {
    furi_check(instance);
    furi_check(state);

    *state = instance->state;
}
