#include "light_sensor.h"
#include "light_sensor_data.h"

#include <furi/furi.h>
#include <furi_hal_light_sensor.h>
#include <furi_hal_i2c_config.h>

#include <power/power_service/power.h>

#define TAG "LightSensor"

#define LIGHT_SENSOR_I2C (&furi_hal_i2c_handle_1)

#define LIGHT_SENSOR_SAMPLE_INTERVAL_MS (1000)

struct LightSensor {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    FuriState* state;
    LightSensorData* data;
    bool is_alive;
};

static bool light_sensor_read(LightSensor* instance) {
    bool success = false;

    do {
        float lux = 0.0f;

        if(!furi_hal_light_sensor_read_lux(LIGHT_SENSOR_I2C, &lux)) {
            FURI_LOG_E(TAG, "Failed to read light sensor");
            break;
        }

        light_sensor_data_add_measurement(instance->data, lux);

        success = true;
    } while(false);

    return success;
}

static void light_sensor_update_state(LightSensor* instance) {
    with_furi_state(instance->state, LightSensorState * state, {
        light_sensor_data_get_state(instance->data, state);
    });
}

static void light_sensor_timer_callback(void* context) {
    furi_assert(context);
    LightSensor* instance = context;

    if(light_sensor_read(instance)) {
        light_sensor_update_state(instance);
    }
}

static bool light_sensor_get_initial_readings(LightSensor* instance) {
    bool success = true;

    for(uint32_t i = 0; i < LIGHT_SENSOR_DATA_WINDOW_SIZE; ++i) {
        if(!light_sensor_read(instance)) {
            success = false;
            break;
        }
    }

    return success;
}

static bool light_sensor_init(LightSensor* instance) {
    bool success = false;

    do {
        if(!furi_hal_light_sensor_init(LIGHT_SENSOR_I2C)) {
            break;
        }

        if(!light_sensor_get_initial_readings(instance)) {
            break;
        }

        furi_event_loop_timer_start(instance->timer, LIGHT_SENSOR_SAMPLE_INTERVAL_MS);
        success = true;

    } while(false);

    return success;
}

LightSensor* light_sensor_alloc() {
    LightSensor* instance = malloc(sizeof(LightSensor));

    instance->data = light_sensor_data_alloc();
    instance->event_loop = furi_event_loop_alloc();
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        light_sensor_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);
    instance->state = furi_state_alloc(sizeof(LightSensorState));
    instance->is_alive = light_sensor_init(instance);

    light_sensor_update_state(instance);

    if(!instance->is_alive) {
        FURI_LOG_E(TAG, "Failed to initialize light sensor");
    }

    furi_record_create(RECORD_LIGHT_SENSOR, instance);
    return instance;
}

int32_t light_sensor_srv(void* p) {
    UNUSED(p);
    // Must be first to ensure that power subsystem is OK
    furi_record_open(RECORD_POWER);

    LightSensor* instance = light_sensor_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

FuriState* light_sensor_get_state(LightSensor* instance) {
    furi_check(instance);
    return instance->state;
}

bool light_sensor_get_raw_data(
    LightSensor* instance,
    LightSensorLightWavelength wavelength,
    uint16_t* raw) {
    furi_check(instance);

    bool result = false;

    if(instance->is_alive) {
        if(wavelength == LightSensorLightWavelength600nm) {
            result = furi_hal_light_sensor_read_raw(
                LIGHT_SENSOR_I2C, FuriHalLightSensorLightWavelength600nm, raw);
        } else if(wavelength == LightSensorLightWavelength840nm) {
            result = furi_hal_light_sensor_read_raw(
                LIGHT_SENSOR_I2C, FuriHalLightSensorLightWavelength840nm, raw);
        }
    }

    return result;
}

bool light_sensor_sleep(LightSensor* instance, bool sleep) {
    furi_check(instance);
    return furi_hal_light_sensor_sleep(LIGHT_SENSOR_I2C, sleep);
}
