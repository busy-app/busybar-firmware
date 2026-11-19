#include "../minunit.h"
#include <light_sensor/light_sensor_data.h>
#include <stdlib.h>

static LightSensorData* light_sensor_test_data;

static void light_sensor_test_setup(void) {
    light_sensor_test_data = light_sensor_data_alloc();
}

static void light_sensor_test_teardown(void) {
    free(light_sensor_test_data);
}

static void light_sensor_test_fill_lux(float lux) {
    for(size_t i = 0; i < LIGHT_SENSOR_DATA_WINDOW_SIZE; i++) {
        light_sensor_data_add_measurement(light_sensor_test_data, lux);
    }
}

static void light_sensor_test_fill_level(float level) {
    const float lux = powf(LIGHT_SENSOR_DATA_LUX_MAX, level / LIGHT_SENSOR_LIGHT_LEVEL_MAX);
    light_sensor_test_fill_lux(lux);
}

static uint8_t light_sensor_test_level(void) {
    LightSensorState state;
    light_sensor_data_get_state(light_sensor_test_data, &state);
    return state.level.val;
}

MU_TEST(light_sensor_test_small_fall) {
    light_sensor_test_fill_level(8.1f);
    mu_assert_int_eq(8, light_sensor_test_level());
    light_sensor_test_fill_level(7.9f);
    mu_assert_int_eq(8, light_sensor_test_level());
}

MU_TEST(light_sensor_test_boundaries) {
    light_sensor_test_fill_level(8.1f);
    light_sensor_test_fill_level(7.51f);
    mu_assert_int_eq(8, light_sensor_test_level());
    light_sensor_test_fill_level(7.49f);
    mu_assert_int_eq(7, light_sensor_test_level());
    light_sensor_test_fill_level(7.99f);
    mu_assert_int_eq(7, light_sensor_test_level());
    light_sensor_test_fill_level(8.01f);
    mu_assert_int_eq(8, light_sensor_test_level());
}

MU_TEST(light_sensor_test_noise) {
    light_sensor_test_fill_level(8.1f);
    for(size_t i = 0; i < 100; i++) {
        const float level = i % 2 ? 8.1f : 7.7f;
        const float lux = powf(LIGHT_SENSOR_DATA_LUX_MAX, level / LIGHT_SENSOR_LIGHT_LEVEL_MAX);
        light_sensor_data_add_measurement(light_sensor_test_data, lux);
        mu_assert_int_eq(8, light_sensor_test_level());
    }
}

MU_TEST(light_sensor_test_jumps_and_limits) {
    light_sensor_test_fill_level(3.1f);
    mu_assert_int_eq(3, light_sensor_test_level());
    light_sensor_test_fill_level(12.1f);
    mu_assert_int_eq(12, light_sensor_test_level());
    light_sensor_test_fill_level(2.1f);
    mu_assert_int_eq(2, light_sensor_test_level());
    light_sensor_test_fill_lux(LIGHT_SENSOR_DATA_LUX_MAX * 2.0f);
    mu_assert_int_eq(LIGHT_SENSOR_LIGHT_LEVEL_MAX, light_sensor_test_level());
    LightSensorState state;
    light_sensor_data_get_state(light_sensor_test_data, &state);
    mu_check(state.lux.instant == LIGHT_SENSOR_DATA_LUX_MAX);
    mu_check(state.lux.mean == LIGHT_SENSOR_DATA_LUX_MAX);
    light_sensor_test_fill_lux(-1.0f);
    mu_assert_int_eq(LIGHT_SENSOR_LIGHT_LEVEL_MIN, light_sensor_test_level());
    light_sensor_data_get_state(light_sensor_test_data, &state);
    mu_check(state.lux.instant == LIGHT_SENSOR_DATA_LUX_MIN);
    mu_check(state.lux.mean == LIGHT_SENSOR_DATA_LUX_MIN);
}

MU_TEST(light_sensor_test_startup_and_mean) {
    LightSensorState state;
    light_sensor_data_get_state(light_sensor_test_data, &state);
    mu_assert_int_eq(LIGHT_SENSOR_LIGHT_LEVEL_MIN, state.level.val);
    mu_check(state.lux.instant == LIGHT_SENSOR_DATA_LUX_MIN);
    mu_check(state.lux.mean == LIGHT_SENSOR_DATA_LUX_MIN);
    for(size_t i = 1; i <= LIGHT_SENSOR_DATA_WINDOW_SIZE; i++) {
        light_sensor_data_add_measurement(light_sensor_test_data, 11.0f);
        light_sensor_data_get_state(light_sensor_test_data, &state);
        mu_check(state.lux.instant == 11.0f);
        mu_check(state.lux.mean == 1.0f + 2.0f * i);
    }
    light_sensor_data_add_measurement(light_sensor_test_data, 21.0f);
    light_sensor_data_get_state(light_sensor_test_data, &state);
    mu_check(state.lux.instant == 21.0f);
    mu_check(state.lux.mean == 13.0f);
}

MU_TEST_SUITE(light_sensor_test_suite) {
    MU_SUITE_CONFIGURE(light_sensor_test_setup, light_sensor_test_teardown);
    MU_RUN_TEST(light_sensor_test_small_fall);
    MU_RUN_TEST(light_sensor_test_boundaries);
    MU_RUN_TEST(light_sensor_test_noise);
    MU_RUN_TEST(light_sensor_test_jumps_and_limits);
    MU_RUN_TEST(light_sensor_test_startup_and_mean);
}

int run_minunit_light_sensor_test(void) {
    MU_RUN_SUITE(light_sensor_test_suite);
    return MU_EXIT_CODE;
}
