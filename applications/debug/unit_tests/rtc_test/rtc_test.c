/**
 * @file rtc_test.c
 * @brief Unit tests for RTC HAL API
 */

#include "../unit_tests.h"

#include <furi_hal_rtc.h>

#define RTC_MILLIS_EPSILON 35

static inline bool are_millis_equal(int16_t expected, int16_t actual) {
    return abs(expected - actual) <= RTC_MILLIS_EPSILON;
}

MU_TEST(rtc_test_set_get_datetime) {
    furi_delay_ms(1500);
    DateTime test = {
        .year = 2025,
        .month = 1,
        .day = 15,
        .hour = 12,
        .minute = 30,
        .second = 45,
        .millis = 500,
        .weekday = 3,
    };

    furi_hal_rtc_set_datetime(&test);

    DateTime result;
    furi_hal_rtc_get_datetime(&result);

    mu_assert_int_eq(test.year, result.year);
    mu_assert_int_eq(test.month, result.month);
    mu_assert_int_eq(test.day, result.day);
    mu_assert_int_eq(test.hour, result.hour);
    mu_assert_int_eq(test.minute, result.minute);
    mu_assert_int_eq(test.second, result.second);
    mu_assert_int_eq(test.weekday, result.weekday);
    mu_check(are_millis_equal(test.millis, result.millis));
}

MU_TEST(rtc_test_time_progression) {
    uint32_t delay = 1500;
    DateTime test = {
        .year = 2025,
        .month = 1,
        .day = 15,
        .hour = 12,
        .minute = 30,
        .second = 45,
        .millis = 0,
        .weekday = 3,
    };

    furi_hal_rtc_set_datetime(&test);

    time_t initial = furi_hal_rtc_get_timestamp_ms();
    furi_delay_ms(delay);
    time_t final = furi_hal_rtc_get_timestamp_ms();

    time_t elapsed = final - initial;
    mu_assert_int_between(delay - RTC_MILLIS_EPSILON, delay + RTC_MILLIS_EPSILON, elapsed);
}

MU_TEST(rtc_test_millis_low) {
    DateTime test = {
        .year = 2025,
        .month = 1,
        .day = 15,
        .hour = 12,
        .minute = 30,
        .second = 45,
        .millis = 100,
        .weekday = 3,
    };

    furi_hal_rtc_set_datetime(&test);

    DateTime result;
    furi_hal_rtc_get_datetime(&result);

    mu_check(are_millis_equal(test.millis, result.millis));
}

MU_TEST(rtc_test_millis_middle) {
    DateTime test = {
        .year = 2025,
        .month = 1,
        .day = 15,
        .hour = 12,
        .minute = 30,
        .second = 45,
        .millis = 500,
        .weekday = 3,
    };

    furi_hal_rtc_set_datetime(&test);

    DateTime result;
    furi_hal_rtc_get_datetime(&result);

    mu_check(are_millis_equal(test.millis, result.millis));
}

MU_TEST(rtc_test_millis_high) {
    DateTime test = {
        .year = 2025,
        .month = 1,
        .day = 15,
        .hour = 12,
        .minute = 30,
        .second = 45,
        .millis = 900,
        .weekday = 3,
    };

    furi_hal_rtc_set_datetime(&test);

    DateTime result;
    furi_hal_rtc_get_datetime(&result);

    mu_check(are_millis_equal(test.millis, result.millis));
}

MU_TEST_SUITE(rtc_test_suite) {
    MU_RUN_TEST(rtc_test_set_get_datetime);
    MU_RUN_TEST(rtc_test_time_progression);
    MU_RUN_TEST(rtc_test_millis_low);
    MU_RUN_TEST(rtc_test_millis_middle);
    MU_RUN_TEST(rtc_test_millis_high);
}

int run_minunit_rtc_test(void) {
    MU_RUN_SUITE(rtc_test_suite);
    return MU_EXIT_CODE;
}
