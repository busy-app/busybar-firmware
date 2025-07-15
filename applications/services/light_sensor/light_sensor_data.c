#include "light_sensor_data.h"

#include <furi/furi.h>

#define TAG "LightSensorData"

struct LightSensorData {
    LightSensorDataConfig config;

    float* measurements;
    size_t measurement_index;

    float lux_mean;
    float lux_instant;
    uint8_t light_level;
};

static void light_sensor_data_update_light_level(LightSensorData* instance) {
    // Clamp lux value to valid range
    float lux = instance->lux_mean;
    if(lux < instance->config.lux_min) lux = instance->config.lux_min;
    if(lux > instance->config.lux_max) lux = instance->config.lux_max;

    float value;

    if(instance->config.use_logarithmic_mapping) {
        // Using logarithmic mapping: level = a * log(lux) + b
        // Where a and b are constants chosen to map:
        // lux_min -> level 0
        // lux_max -> level max
        const float log_min = logf(instance->config.lux_min);
        const float log_max = logf(instance->config.lux_max);
        const float a = instance->config.light_level_max / (log_max - log_min);
        const float b = -a * log_min;

        value = a * logf(lux) + b;
    } else {
        // Linear mapping as fallback
        // Map lux from [lux_min, lux_max] to [0, light_level_max]
        value = (lux - instance->config.lux_min) /
                (instance->config.lux_max - instance->config.lux_min) *
                instance->config.light_level_max;
    }

    // Ensure the result is within bounds
    if(value < 0.0f) value = 0.0f;
    if(value > instance->config.light_level_max) value = instance->config.light_level_max;

    instance->light_level = (uint8_t)roundf(value);

    FURI_LOG_T(
        TAG,
        "Light mapping: lux=%.2f -> level=%d (mode=%s)",
        lux,
        instance->light_level,
        instance->config.use_logarithmic_mapping ? "logarithmic" : "linear");
}

LightSensorData* light_sensor_data_alloc(const LightSensorDataConfig* config) {
    furi_check(config);

    LightSensorData* instance = malloc(sizeof(LightSensorData));
    instance->config = *config;

    instance->measurements = malloc(config->window_size * sizeof(float));
    for(size_t i = 0; i < config->window_size; i++) {
        instance->measurements[i] = config->lux_min;
    }
    instance->measurement_index = 0;

    instance->lux_mean = config->lux_min;
    instance->lux_instant = config->lux_min;
    instance->light_level = 0;

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
