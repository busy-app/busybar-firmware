#include "light_sensor_data.h"
#include "light_sensor_common.h"

#include <math.h>
#include <float_tools.h>

#include <core/check.h>
#include <core/log.h>

#define LIGHT_SENSOR_DATA_THRESHOLD ((1.0f / LIGHT_SENSOR_LIGHT_LEVEL_MAX) / 2.0f)

struct LightSensorData {
    float measurements[LIGHT_SENSOR_DATA_WINDOW_SIZE];
    size_t measurement_index;
    LightSensorState state;
};

static void light_sensor_data_update_light_level(LightSensorData* instance) {
    LightSensorState* state = &instance->state;
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

    const float light_level_real = a * logf(state->lux.mean) + b;
    const float light_level_delta = light_level_real - state->level.val;

    if(fabsf(light_level_delta) > LIGHT_SENSOR_DATA_THRESHOLD) {
        state->level.val = floorf(light_level_real);
    }
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

void light_sensor_data_add_measurement(LightSensorData* instance, float lux) {
    furi_check(instance);

    LightSensorLuxLevel* lux_level = &instance->state.lux;
    lux_level->instant = CLAMP(lux, LIGHT_SENSOR_DATA_LUX_MAX, LIGHT_SENSOR_DATA_LUX_MIN);

    instance->measurements[instance->measurement_index] = lux_level->instant;
    instance->measurement_index =
        (instance->measurement_index + 1) % LIGHT_SENSOR_DATA_WINDOW_SIZE;

    float lux_sum = 0.0f;
    for(size_t i = 0; i < LIGHT_SENSOR_DATA_WINDOW_SIZE; i++) {
        lux_sum += instance->measurements[i];
    }

    lux_level->mean = lux_sum / LIGHT_SENSOR_DATA_WINDOW_SIZE;
    light_sensor_data_update_light_level(instance);
}

void light_sensor_data_get_state(const LightSensorData* instance, LightSensorState* state) {
    furi_check(instance);
    furi_check(state);
    *state = instance->state;
}
