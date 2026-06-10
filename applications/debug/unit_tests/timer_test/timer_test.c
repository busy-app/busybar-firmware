#include "../unit_tests.h"

#include <toolbox/timers.h>

#define COARSE_TIMER_TIMEOUT_MS (10)
#define COARSE_TIMER_ABSTOL_MS  (2)

#define PRECISE_TIMER_TIMEOUT_US (1000)
#define PRECISE_TIMER_ABSTOL_US  (5)

#define MAGIC_DELAY_US    (PRECISE_TIMER_TIMEOUT_US / 2)
#define COUNTER_THRESHOLD (10)

static bool always_false(void* context) {
    UNUSED(context);
    return false;
}

static bool eventually_true(void* context) {
    furi_assert(context);
    uint32_t* counter = context;
    return ++(*counter) == COUNTER_THRESHOLD;
}

static bool true_after_expire(void* context) {
    furi_delay_us(PRECISE_TIMER_TIMEOUT_US / COUNTER_THRESHOLD);
    return eventually_true(context);
}

MU_TEST(coarse_timer_test) {
    FURI_CRITICAL_ENTER();
    const CoarseTimer timer = coarse_timer_create(COARSE_TIMER_TIMEOUT_MS);

    const uint32_t elapsed_start_ms = coarse_timer_get_elapsed(timer);
    const bool is_expired_start = coarse_timer_is_expired(timer);
    FURI_CRITICAL_EXIT();

    furi_delay_ms(COARSE_TIMER_TIMEOUT_MS);

    const uint32_t elapsed_end_ms = coarse_timer_get_elapsed(timer);
    const bool is_expired_end = coarse_timer_is_expired(timer);

    mu_check(!is_expired_start);
    mu_check(is_expired_end);

    mu_assert_int_eq(0, elapsed_start_ms);
    mu_assert_int_between(
        COARSE_TIMER_TIMEOUT_MS - COARSE_TIMER_ABSTOL_MS,
        COARSE_TIMER_TIMEOUT_MS + COARSE_TIMER_ABSTOL_MS,
        (int32_t)elapsed_end_ms);
}

MU_TEST(precise_timer_test) {
    FURI_CRITICAL_ENTER();
    const PreciseTimer timer = precise_timer_create(PRECISE_TIMER_TIMEOUT_US);

    const uint32_t elapsed_start_us = precise_timer_get_elapsed(timer);
    const bool is_expired_start = precise_timer_is_expired(timer);

    precise_timer_wait(timer);

    const uint32_t elapsed_end_us = precise_timer_get_elapsed(timer);
    const bool is_expired_end = precise_timer_is_expired(timer);
    FURI_CRITICAL_EXIT();

    mu_check(!is_expired_start);
    mu_check(is_expired_end);

    mu_assert_int_eq(0, elapsed_start_us);
    mu_assert_int_between(
        PRECISE_TIMER_TIMEOUT_US,
        PRECISE_TIMER_TIMEOUT_US + PRECISE_TIMER_ABSTOL_US,
        elapsed_end_us);
}

MU_TEST(precise_timer_test_condition) {
    FURI_CRITICAL_ENTER();
    const PreciseTimer timer1 = precise_timer_create(PRECISE_TIMER_TIMEOUT_US);
    const bool timer1_condition_reached = precise_timer_wait_for(timer1, always_false, NULL);
    const bool timer1_is_expired = precise_timer_is_expired(timer1);

    const PreciseTimer timer2 = precise_timer_create(PRECISE_TIMER_TIMEOUT_US);
    uint32_t counter2 = 0;
    const bool timer2_condition_reached =
        precise_timer_wait_for(timer2, eventually_true, &counter2);
    const bool timer2_is_expired = precise_timer_is_expired(timer2);

    const PreciseTimer timer3 = precise_timer_create(PRECISE_TIMER_TIMEOUT_US);
    uint32_t counter3 = 0;
    const bool timer3_condition_reached =
        precise_timer_wait_for(timer3, true_after_expire, &counter3);
    const bool timer3_is_expired = precise_timer_is_expired(timer3);
    FURI_CRITICAL_EXIT();

    mu_check(!timer1_condition_reached);
    mu_check(timer2_condition_reached);
    mu_check(!timer3_condition_reached);

    mu_check(timer1_is_expired);
    mu_check(!timer2_is_expired);
    mu_check(timer3_is_expired);

    mu_assert_int_eq(COUNTER_THRESHOLD, counter2);
    mu_assert_int_eq(COUNTER_THRESHOLD, counter3);
}

MU_TEST_SUITE(timer_test_suite) {
    MU_RUN_TEST(coarse_timer_test);
    MU_RUN_TEST(precise_timer_test);
    MU_RUN_TEST(precise_timer_test_condition);
}

int run_minunit_timer_test(void) {
    MU_RUN_SUITE(timer_test_suite);
    return MU_EXIT_CODE;
}
