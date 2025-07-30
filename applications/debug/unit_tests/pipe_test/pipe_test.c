/**
 * @file pipe_test.c
 */

#include "../unit_tests.h"
#include <containers/pipe.h>
#include <containers/pipe_util.h>

typedef struct {
    const char* input;
    const char* terminator;
    const char* expected_output;
} PipeCopyTestCase;

static const PipeCopyTestCase test_cases[] = {
    // real-world use cases
    {"property  : value\r\n>: ", "\r\n>: ", "property  : value"},
    // synthetic use cases
    {"abcabcabc", "cab", "ab"},
    {"aaaabc", "abc", "aaa"},
    {"aababcabcd", "abcd", "aababc"},
};

MU_TEST(pipe_copy_until_test) {
    for(size_t i = 0; i < COUNT_OF(test_cases); i++) {
        const PipeCopyTestCase* test_case = &test_cases[i];

        PipeSideBundle bundle_a = pipe_alloc(strlen(test_case->input), 1);
        PipeSideBundle bundle_b = pipe_alloc(strlen(test_case->input), 1);

        PipeSide* a = bundle_a.alices_side;
        PipeSide* b = bundle_a.bobs_side;
        PipeSide* c = bundle_b.alices_side;
        PipeSide* d = bundle_b.bobs_side;

        pipe_send(a, test_case->input, strlen(test_case->input));

        mu_assert_int_eq(true, pipe_copy_until(b, c, test_case->terminator));
        pipe_free(a);
        pipe_free(c);

        char output_buf[strlen(test_case->input) + 1];
        size_t output_cnt = pipe_receive(d, output_buf, sizeof(output_buf));
        output_buf[output_cnt] = '\0';
        pipe_free(d);

        char leftover_buf[strlen(test_case->input) + 1];
        size_t leftover_cnt = pipe_receive(b, leftover_buf, sizeof(leftover_buf));
        leftover_buf[leftover_cnt] = '\0';
        pipe_free(b);

        const char* expected_leftover =
            test_case->input + strlen(test_case->terminator) + strlen(test_case->expected_output);
        mu_assert_string_eq(test_case->expected_output, output_buf);
        mu_assert_string_eq(expected_leftover, leftover_buf);
    }
}

MU_TEST(pipe_copy_to_null_until_test) {
    for(size_t i = 0; i < COUNT_OF(test_cases); i++) {
        const PipeCopyTestCase* test_case = &test_cases[i];

        PipeSideBundle bundle_a = pipe_alloc(strlen(test_case->input), 1);

        PipeSide* a = bundle_a.alices_side;
        PipeSide* b = bundle_a.bobs_side;

        pipe_send(a, test_case->input, strlen(test_case->input));

        mu_assert_int_eq(true, pipe_copy_until(b, NULL, test_case->terminator));
        pipe_free(a);

        char leftover_buf[strlen(test_case->input) + 1];
        size_t leftover_cnt = pipe_receive(b, leftover_buf, sizeof(leftover_buf));
        leftover_buf[leftover_cnt] = '\0';
        pipe_free(b);

        const char* expected_leftover =
            test_case->input + strlen(test_case->terminator) + strlen(test_case->expected_output);
        mu_assert_string_eq(expected_leftover, leftover_buf);
    }
}

MU_TEST_SUITE(pipe_test_suite) {
    MU_RUN_TEST(pipe_copy_until_test);
    MU_RUN_TEST(pipe_copy_to_null_until_test);
}

int run_minunit_pipe_test(void) {
    MU_RUN_SUITE(pipe_test_suite);
    return MU_EXIT_CODE;
}
