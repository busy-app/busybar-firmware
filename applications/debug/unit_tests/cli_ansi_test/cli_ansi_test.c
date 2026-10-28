/**
 * @file cli_ansi_test.c
 * @brief cli_ansi module tests
 */

#include "../unit_tests.h"

#include <cli/cli_ansi.h>

typedef struct {
    const char* string;
    size_t expected_result;
} PrintableStrlenTest;
MU_TEST(cli_ansi_test_printable_strlen) {
    static const PrintableStrlenTest cases[] = {
        // no shenanigans
        {"hello", 5},
        {"hello world", 11},
        // UTF-8
        {"привет", 6},
        {"привет мир", 10},
        {"🙂", 1},
        {"🇺🇳", 2}, // 2 codepoints
        // escape sequences
        {ANSI_FG_RED, 0},
        {ANSI_FG_RED "hi", 2},
        {ANSI_FG_RED "hi" ANSI_RESET, 2},
        // ASCII non-printable
        {"\aDing", 4}, // "Alert" code (invokes bell sound)
        {"\a\b\e\f\n\r\t\v", 0}, // All non-printable ASCII chars that have C escape sequences
        // mixed
        {"\r\n🔔🔔🔔 " ANSI_FG_YELLOW "Ding!" ANSI_RESET, 9},
    };
    for(size_t i = 0; i < COUNT_OF(cases); i++) {
        mu_assert_int_eq(cases[i].expected_result, cli_ansi_printable_strlen(cases[i].string));
    }
}

MU_TEST_SUITE(cli_ansi_test_suite) {
    MU_RUN_TEST(cli_ansi_test_printable_strlen);
}

int run_minunit_cli_ansi_test(void) {
    MU_RUN_SUITE(cli_ansi_test_suite);
    return MU_EXIT_CODE;
}
