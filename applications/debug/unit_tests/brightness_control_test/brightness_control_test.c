#include "../unit_tests.h"
#include <brightness_control/brightness_ramp_i.h>
#include <math.h>
#include <stdlib.h>

#define BRIGHTNESS_TEST_FULL_STEPS (128U)
#define BRIGHTNESS_TEST_STEP       (1.f / 128.f)

MU_TEST(brightness_ramp_convergence_test) {
    BrightnessRamp ramp = {0};
    brightness_ramp_set(&ramp, (InternalBrightness){0.f});
    mu_check(brightness_ramp_target(&ramp, (InternalBrightness){1.f}));
    brightness_ramp_step(&ramp);
    mu_check(ramp.current.val == BRIGHTNESS_TEST_STEP);
    for(unsigned step = 1; step < BRIGHTNESS_TEST_FULL_STEPS; ++step) {
        brightness_ramp_step(&ramp);
    }
    mu_check(ramp.current.val == 1.f && !ramp.is_active);
    mu_check(brightness_ramp_target(&ramp, (InternalBrightness){0.f}));
    for(unsigned step = 0; step < BRIGHTNESS_TEST_FULL_STEPS; ++step) {
        brightness_ramp_step(&ramp);
    }
    mu_check(ramp.current.val == 0.f && !ramp.is_active);
}

MU_TEST(brightness_ramp_retarget_cancel_test) {
    BrightnessRamp ramp = {0};
    brightness_ramp_set(&ramp, (InternalBrightness){0.f});
    mu_check(!brightness_ramp_target(&ramp, (InternalBrightness){0.f}));
    brightness_ramp_target(&ramp, (InternalBrightness){1.f});
    brightness_ramp_step(&ramp);
    mu_check(!brightness_ramp_target(&ramp, (InternalBrightness){1.f}));
    mu_check(ramp.current.val == BRIGHTNESS_TEST_STEP);
    brightness_ramp_target(&ramp, (InternalBrightness){0.f});
    mu_check(ramp.current.val == BRIGHTNESS_TEST_STEP);
    brightness_ramp_step(&ramp);
    mu_check(ramp.current.val == 0.f && !ramp.is_active);
    brightness_ramp_target(&ramp, (InternalBrightness){1.f});
    brightness_ramp_set(&ramp, brightness_conv_user_to_internal((UserBrightness){37}));
    brightness_ramp_step(&ramp);
    mu_assert_int_eq(37, brightness_conv_internal_to_user(ramp.current).val);
    mu_check(!ramp.is_active);
}

MU_TEST(brightness_ramp_adjacent_level_test) {
    BrightnessRamp ramp = {0};
    brightness_ramp_set(&ramp, brightness_conv_light_sensor_to_internal((LightSensorLevel){7}));
    const InternalBrightness target =
        brightness_conv_light_sensor_to_internal((LightSensorLevel){8});
    brightness_ramp_target(&ramp, target);
    unsigned steps = 0;
    while(ramp.is_active) {
        brightness_ramp_step(&ramp);
        mu_check(++steps <= BRIGHTNESS_TEST_FULL_STEPS);
    }
    mu_assert_int_eq(9, steps);
    mu_check(ramp.current.val == target.val);
}

MU_TEST(brightness_ramp_curve_bounds_test) {
    static const float starts[] = {0.f, .001f, .05f, .123f, .49f, .5f, .777f, .999f, 1.f};
    for(unsigned start = 0; start < COUNT_OF(starts); ++start) {
        for(unsigned level = 0; level <= LIGHT_SENSOR_LIGHT_LEVEL_MAX; ++level) {
            BrightnessRamp ramp = {0};
            brightness_ramp_set(&ramp, (InternalBrightness){starts[start]});
            const InternalBrightness target =
                brightness_conv_light_sensor_to_internal((LightSensorLevel){level});
            brightness_ramp_target(&ramp, target);
            unsigned steps = 0;
            while(ramp.is_active) {
                const InternalBrightness previous = ramp.current;
                const int front = brightness_conv_internal_to_front(previous).val;
                const int back = brightness_conv_internal_to_back(previous).val;
#if defined(SRV_STATUS_LIGHTS)
                const int status = brightness_conv_internal_to_status(previous).val;
#endif
                brightness_ramp_step(&ramp);
                mu_check(
                    fabsf(ramp.current.val - previous.val) <= BRIGHTNESS_TEST_STEP + .000001f);
                mu_check(
                    abs((int)brightness_conv_internal_to_front(ramp.current).val - front) <= 1);
                mu_check(abs((int)brightness_conv_internal_to_back(ramp.current).val - back) <= 1);
#if defined(SRV_STATUS_LIGHTS)
                mu_check(
                    abs((int)brightness_conv_internal_to_status(ramp.current).val - status) <= 1);
#endif
                mu_check(++steps <= BRIGHTNESS_TEST_FULL_STEPS);
            }
            mu_check(ramp.current.val == target.val);
        }
    }
}

MU_TEST_SUITE(brightness_control_test_suite) {
    MU_RUN_TEST(brightness_ramp_convergence_test);
    MU_RUN_TEST(brightness_ramp_retarget_cancel_test);
    MU_RUN_TEST(brightness_ramp_adjacent_level_test);
    MU_RUN_TEST(brightness_ramp_curve_bounds_test);
}

int run_minunit_brightness_control_test(void) {
    MU_RUN_SUITE(brightness_control_test_suite);
    return MU_EXIT_CODE;
}
