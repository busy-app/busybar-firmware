#include "light_sensor_data.h"

#include <furi/furi.h>

struct LightSensorData {
    LightSensorDataConfig config;

    float* measurements;
    size_t measurement_index;

    float lux_mean;
    float lux_instant;
    uint8_t light_level;
};

static void light_sensor_data_update_light_level(LightSensorData* instance) {
    // TODO think about light_level = f(lux_mean, lux_instant, light_level) function
    // Now it is just a linear function

    uint8_t new_light_level =
        (uint8_t)(instance->lux_mean / instance->config.light_level_max_threshold *
                  instance->config.light_level_max);
    instance->light_level = new_light_level > instance->config.light_level_max ?
                                instance->config.light_level_max :
                                new_light_level;
}

LightSensorData* light_sensor_data_alloc(const LightSensorDataConfig* config) {
    furi_check(config);

    LightSensorData* instance = malloc(sizeof(LightSensorData));
    instance->config = *config;

    instance->measurements = malloc(config->window_size * sizeof(float));
    for(size_t i = 0; i < config->window_size; i++) {
        instance->measurements[i] = config->light_level_max_threshold;
    }
    instance->measurement_index = 0;

    instance->lux_mean = config->light_level_max_threshold;
    instance->lux_instant = config->light_level_max_threshold;
    instance->light_level = config->light_level_max;

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
    instance->measurement_index = (instance->measurement_index + 1) % instance->config.window_size;

    float lux_sum = 0.0f;
    for(size_t i = 0; i < instance->config.window_size; i++) {
        lux_sum += instance->measurements[i];
    }
    instance->lux_mean = lux_sum / instance->config.window_size;

    light_sensor_data_update_light_level(instance);

    FURI_LOG_T(
        "LightSensorData",
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
