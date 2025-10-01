/**
 * @file state_test.c
 * @brief Tests FuriState
 */

#include "../unit_tests.h"

#include <furi.h>

typedef enum {
    MoonPhaseNewMoon,
    MoonPhaseWaxingCrescent,
    MoonPhaseFirstQuarter,
    MoonPhaseWaxingGibbous,
    MoonPhaseFullMoon,
    MoonPhaseWaningGibbous,
    MoonPhaseLastQuarter,
    MoonPhaseWaningCrescent,
    MoonPhaseMAX,
} MoonPhase;

static size_t notification_ctr = 0;

static void check_phase(const void* state, void* context) {
    MoonPhase* expected = context;
    const MoonPhase* actual = state;
    mu_assert_int_eq(*expected, *actual);
    notification_ctr++;
}

MU_TEST(state_test_moon) {
    FuriState* state = furi_state_alloc(sizeof(MoonPhase));

    const size_t sub_limit = 100;
    const size_t total_days = 1000;
    FuriStateSub* subs[sub_limit];

    MoonPhase phase;
    for(size_t day = 0; day < total_days; day++) {
        phase = (day * 8) / 30;

        size_t sub_count = CLAMP(day, sub_limit, 0ul);

        notification_ctr = 0;
        furi_state_set(state, &phase);
        mu_assert_int_eq(sub_count, notification_ctr);

        if(day < sub_limit) {
            MoonPhase stored_phase;
            subs[day] = furi_state_subscribe(state, &stored_phase, check_phase, &phase);
            mu_assert_int_eq(phase, stored_phase);
        }

        sub_count = CLAMP(day, sub_limit, 0ul);

        for(size_t i = 0; i < CLAMP(sub_count + 1, sub_limit, 0ul); i++) {
            MoonPhase stored_phase;
            furi_state_get(subs[i], &stored_phase);
            mu_assert_int_eq(phase, stored_phase);
        }
    }

    for(size_t i = 0; i < sub_limit; i++) {
        furi_state_unsubscribe(subs[i]);
    }

    furi_state_free(state);
}

MU_TEST_SUITE(state_test_suite) {
    MU_RUN_TEST(state_test_moon);
}

int run_minunit_state_test(void) {
    MU_RUN_SUITE(state_test_suite);
    return MU_EXIT_CODE;
}
