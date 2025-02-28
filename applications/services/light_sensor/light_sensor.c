#include "light_sensor.h"
#include "light_sensor_data.h"

#include <furi/furi.h>
#include <furi_hal_light_sensor.h>
#include <furi_hal_i2c_config.h>

#include <power_simple/power.h>

#define TAG "LightSensor"

#define LIGHT_SENSOR_I2C (&furi_hal_i2c_handle_1)

#define LIGHT_SENSOR_SAMPLE_INTERVAL_MS        (1000)
#define LIGHT_SENSOR_LIGHT_LEVEL_MAX_THRESHOLD (1000.0f)
#define LIGHT_SENSOE_WINDOW_SIZE               (10)

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    FuriPubSub* pubsub;

    LightSensorData* data;
    uint8_t light_level_previous;
    uint8_t light_level_current;
} LightSensor;

LightSensor* light_sensor = NULL;

static void light_sensor_timer_callback(void* context) {
    LightSensor* instance = context;

    float lux = 0.0f;
    bool read_success = furi_hal_light_sensor_read_lux(LIGHT_SENSOR_I2C, &lux);
    if(!read_success) {
        FURI_LOG_E(TAG, "Failed to read light sensor");
        return;
    }

    FURI_LOG_T(TAG, "Light sensor: %.2f lux", lux);
    light_sensor_data_add_measurement(instance->data, lux);

    instance->light_level_current = light_sensor_data_get_light_level(instance->data);
    if(instance->light_level_current != instance->light_level_previous) {
        FURI_LOG_D(
            TAG,
            "Light level changed: %u -> %u",
            instance->light_level_previous,
            instance->light_level_current);
        LightSensorEvent event = {
            .type = instance->light_level_current > instance->light_level_previous ?
                        LightSensorEventTypeLightLevelIncreased :
                        LightSensorEventTypeLightLevelDecreased,
            .light_level_previous = instance->light_level_previous,
            .light_level_current = instance->light_level_current,
        };

        furi_pubsub_publish(instance->pubsub, &event);
        instance->light_level_previous = instance->light_level_current;
    }
}

static LightSensor* light_sensor_alloc(void) {
    light_sensor = malloc(sizeof(LightSensor));

    LightSensorDataConfig config = {
        .window_size = LIGHT_SENSOE_WINDOW_SIZE,
        .light_level_max = LIGHT_SENSOR_LIGHT_LEVEL_MAX,
        .light_level_max_threshold = LIGHT_SENSOR_LIGHT_LEVEL_MAX_THRESHOLD,
    };
    light_sensor->data = light_sensor_data_alloc(&config);

    light_sensor->event_loop = furi_event_loop_alloc();
    light_sensor->timer = furi_event_loop_timer_alloc(
        light_sensor->event_loop,
        light_sensor_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        light_sensor);
    light_sensor->pubsub = furi_pubsub_alloc();

    furi_record_create(RECORD_LIGHT_SENSOR_EVENTS, light_sensor->pubsub);

    furi_event_loop_timer_start(light_sensor->timer, LIGHT_SENSOR_SAMPLE_INTERVAL_MS);

    return light_sensor;
}

int32_t light_sensor_srv(void* p) {
    UNUSED(p);

    // Must be first to ensure that power subsystem is OK
    furi_record_open(RECORD_POWER);

    bool initialized = furi_hal_light_sensor_init(LIGHT_SENSOR_I2C);
    if(!initialized) {
        FURI_LOG_E(TAG, "Failed to initialize light sensor");
        return -1;
    }

    LightSensor* instance = light_sensor_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

float light_sensor_get_lux(void) {
    furi_check(light_sensor);

    return light_sensor_data_get_lux(light_sensor->data);
}

float light_sensor_get_lux_instant(void) {
    furi_check(light_sensor);

    return light_sensor_data_get_lux_instant(light_sensor->data);
}
