#include "../unit_tests.h"

#include <datetime/datetime.h>

MU_TEST(datetime_test_parse_timestamp) {
    DateTime dt;
    mu_check(datetime_parse_timestamp("2026-01-30T15:47:50Z", &dt));
    mu_assert_int_eq(2026, dt.year);
    mu_assert_int_eq(1, dt.month);
    mu_assert_int_eq(30, dt.day);
    mu_assert_int_eq(5, dt.weekday);
    mu_assert_int_eq(15, dt.hour);
    mu_assert_int_eq(47, dt.minute);
    mu_assert_int_eq(50, dt.second);
    mu_assert_int_eq(0, dt.millis);

    mu_check(datetime_parse_timestamp("20260201T114230Z", &dt));
    mu_assert_int_eq(2026, dt.year);
    mu_assert_int_eq(2, dt.month);
    mu_assert_int_eq(1, dt.day);
    mu_assert_int_eq(7, dt.weekday);
    mu_assert_int_eq(11, dt.hour);
    mu_assert_int_eq(42, dt.minute);
    mu_assert_int_eq(30, dt.second);
    mu_assert_int_eq(0, dt.millis);

    mu_check(datetime_parse_timestamp("2026-01-30T15:47:50+03:00", &dt));
    mu_assert_int_eq(2026, dt.year);
    mu_assert_int_eq(1, dt.month);
    mu_assert_int_eq(30, dt.day);
    mu_assert_int_eq(5, dt.weekday);
    mu_assert_int_eq(12, dt.hour);
    mu_assert_int_eq(47, dt.minute);
    mu_assert_int_eq(50, dt.second);
    mu_assert_int_eq(0, dt.millis);

    mu_check(datetime_parse_timestamp("2026-01-30T15:47:50-01:30", &dt));
    mu_assert_int_eq(2026, dt.year);
    mu_assert_int_eq(1, dt.month);
    mu_assert_int_eq(30, dt.day);
    mu_assert_int_eq(5, dt.weekday);
    mu_assert_int_eq(17, dt.hour);
    mu_assert_int_eq(17, dt.minute);
    mu_assert_int_eq(50, dt.second);
    mu_assert_int_eq(0, dt.millis);

    // Unspecified timezone
    mu_check(!datetime_parse_timestamp("2026-01-30T15:47:50", &dt));

    // Junk at the end
    mu_check(!datetime_parse_timestamp("2026-01-30T15:47:50-01:30:31", &dt));

    // Inconsistent hyphens
    mu_check(!datetime_parse_timestamp("2026-0130T15:47:50-01:30", &dt));

    // Inconsistent hyphens again
    mu_check(!datetime_parse_timestamp("20260130T1547:50-01:30", &dt));
}

MU_TEST(datetime_test_format_timestamp) {
    LocalTime lt = {
        .dt = {
            .year = 2026,
            .month = 1,
            .day = 26,
            .weekday = 1,
            .hour = 14,
            .minute = 56,
            .second = 12,
            .millis = 0,
        },
        .offset = utz_offset_init(true, 1, 30),
    };

    char buf[DATETIME_TIMESTAMP_STR_LEN + 1];
    datetime_format_timestamp(&lt, buf);

    mu_assert_string_eq("2026-01-26T14:56:12-01:30", buf);
}


int run_minunit_datetime_test(void) {
    MU_RUN_TEST(datetime_test_parse_timestamp);
    MU_RUN_TEST(datetime_test_format_timestamp);
    return MU_EXIT_CODE;
}