#include "../unit_tests.h"
#include <state_publisher/rate_limiter.h>

static uint32_t packets_sent = 0;
static bool send_always(void* context, bool heartbeat) {
    UNUSED(context);
    UNUSED(heartbeat);
    packets_sent += 1;
    return true;
}

static bool send_if_heartbeat(void* context, bool heartbeat) {
    UNUSED(context);
    if(heartbeat) packets_sent += 1;
    return heartbeat;
}

MU_TEST(rate_limiter_test_unlimited) {
    packets_sent = 0;
    RateLimiter limiter = rate_limiter_init(RATE_LIMITER_UNLIMITED);
    uint32_t sleep_time_ms =
        rate_limiter_send_if_allowed(&limiter, 1000, send_always, NULL, false);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(1, packets_sent);
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 2000, send_always, NULL, false);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(2, packets_sent);
}

MU_TEST(rate_limiter_test_limited) {
    packets_sent = 0;
    RateLimiter limiter =
        rate_limiter_init((RateLimiterLimit){.period_ms = 1000, .max_packet_count = 3});
    // Packet 1
    uint32_t sleep_time_ms =
        rate_limiter_send_if_allowed(&limiter, 2000, send_always, NULL, false);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(1, packets_sent);

    // Packet 2
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 2001, send_always, NULL, false);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(2, packets_sent);

    // Packet 3
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 2002, send_always, NULL, false);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(3, packets_sent);

    // Packet 4 - postponed
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 2003, send_always, NULL, false);
    mu_assert_int_eq(997, sleep_time_ms);
    mu_assert_int_eq(3, packets_sent);

    // Packet 5 - postponed
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 2010, send_always, NULL, false);
    mu_assert_int_eq(990, sleep_time_ms);
    mu_assert_int_eq(3, packets_sent);

    // Timer
    sleep_time_ms =
        rate_limiter_send_if_allowed(&limiter, 2010 + sleep_time_ms, send_always, NULL, false);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(4, packets_sent);

    // Packet 6
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 3010, send_always, NULL, false);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(5, packets_sent);

    // Packet 7
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 3020, send_always, NULL, false);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(6, packets_sent);

    // Packet 8 - postponed
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 3030, send_always, NULL, false);
    mu_assert_int_eq(970, sleep_time_ms);
    mu_assert_int_eq(6, packets_sent);

    sleep_time_ms =
        rate_limiter_send_if_allowed(&limiter, 3030 + sleep_time_ms, send_always, NULL, false);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(7, packets_sent);

    // Packet 9
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 4050, send_always, NULL, false);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(8, packets_sent);

    // Packet 10 - long delay
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 6500, send_always, NULL, false);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(9, packets_sent);

    // Packet 11
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 6510, send_always, NULL, false);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(10, packets_sent);

    // Packet 12
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 6520, send_always, NULL, false);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(11, packets_sent);

    // Packet 13 - postponed
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 6530, send_always, NULL, false);
    mu_assert_int_eq(970, sleep_time_ms);
    mu_assert_int_eq(11, packets_sent);
}

MU_TEST(rate_limiter_test_no_data) {
    packets_sent = 0;
    RateLimiter limiter =
        rate_limiter_init((RateLimiterLimit){.period_ms = 1000, .max_packet_count = 3});

    // Packet 1
    uint32_t sleep_time_ms =
        rate_limiter_send_if_allowed(&limiter, 2000, send_if_heartbeat, NULL, true);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(1, packets_sent);

    // No packet is sent
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 2010, send_if_heartbeat, NULL, false);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(1, packets_sent);

    // No packet is sent - again
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 2020, send_if_heartbeat, NULL, false);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(1, packets_sent);

    // Packet 2
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 2030, send_if_heartbeat, NULL, true);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(2, packets_sent);

    // Packet 3
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 2040, send_if_heartbeat, NULL, true);
    mu_assert_int_eq(UINT32_MAX, sleep_time_ms);
    mu_assert_int_eq(3, packets_sent);

    // Packet 4 - postpone
    sleep_time_ms = rate_limiter_send_if_allowed(&limiter, 2050, send_if_heartbeat, NULL, true);
    mu_assert_int_eq(950, sleep_time_ms);
    mu_assert_int_eq(3, packets_sent);
}

MU_TEST_SUITE(rate_limiter_test_suite) {
    MU_RUN_TEST(rate_limiter_test_unlimited);
    MU_RUN_TEST(rate_limiter_test_limited);
    MU_RUN_TEST(rate_limiter_test_no_data);
}

int run_minunit_rate_limiter_test(void) {
    MU_RUN_SUITE(rate_limiter_test_suite);
    return MU_EXIT_CODE;
}
