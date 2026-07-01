#include "../unit_tests.h"

#include <toolbox/getopt.h>

#define OPT_A "a"
#define OPT_B "b"
#define OPT_C "c"
#define OPT_D "D"
#define OPT_E "E"
#define OPT_F "F"

#define OPTS_ALL OPT_A ":" OPT_B ":" OPT_C ":" OPT_D OPT_E OPT_F

#define OPTVAL_A "hello"
#define OPTVAL_B "42"
#define OPTVAL_C "0xdeadbeef"
#define OPTVAL_D // Empty
#define OPTVAL_E // Empty
#define OPTVAL_F // Empty

#define POSARG_1 "lorem"
#define POSARG_2 "ipsum"

#define ARG(opt, optval)        "-" opt " " optval
#define ARG_TAB(opt, optval)    "-" opt "\t" optval
#define ARG_CONCAT(opt, optval) "-" opt optval

#define ARGS_ALL                                                          \
    ARG(OPT_A, OPTVAL_A)                                                  \
    " " ARG_TAB(OPT_B, OPTVAL_B) " " ARG_CONCAT(OPT_C, OPTVAL_C) " " ARG( \
        OPT_D, OPTVAL_D) " " ARG(OPT_E, OPTVAL_E) " " ARG(OPT_F, OPTVAL_F)

#define ARGS_WITH_POSARGS                                                                        \
    ARG(OPT_A, OPTVAL_A)                                                                         \
    " " POSARG_1 "\t" ARG(OPT_B, OPTVAL_B) " \t " POSARG_2 " " ARG(OPT_C, OPTVAL_C) " " POSARG_1 \
                                                                                    " \t "

#define FIRST_CHAR(str) (str[0])

static void* getopt_test_context = (void*)(0xfeedcafe);

static void getopt_test_option_callback(char opt, const char* optval, void* context) {
    mu_assert_pointers_eq(context, getopt_test_context);

    if(opt == FIRST_CHAR(OPT_A)) {
        mu_assert_string_eq(OPTVAL_A, optval);
    } else if(opt == FIRST_CHAR(OPT_B)) {
        mu_assert_string_eq(OPTVAL_B, optval);
    } else if(opt == FIRST_CHAR(OPT_C)) {
        mu_assert_string_eq(OPTVAL_C, optval);
    } else if(opt == FIRST_CHAR(OPT_D)) {
        mu_assert_null(optval);
    } else if(opt == FIRST_CHAR(OPT_E)) {
        mu_assert_null(optval);
    } else if(opt == FIRST_CHAR(OPT_F)) {
        mu_assert_null(optval);
    } else {
        mu_assert(false, "Invalid option");
    }
}

static void getopt_test_posarg_option_callback(char opt, const char* optval, void* context) {
    mu_assert_pointers_eq(context, getopt_test_context);

    if(opt == FIRST_CHAR(OPT_A)) {
        mu_assert_string_eq(OPTVAL_A, optval);
    } else if(opt == FIRST_CHAR(OPT_B)) {
        mu_assert_string_eq(OPTVAL_B, optval);
    } else if(opt == FIRST_CHAR(OPT_C)) {
        mu_assert_string_eq(OPTVAL_C, optval);
    } else if(opt == 0) {
        mu_check((strcmp(optval, POSARG_1) == 0) || (strcmp(optval, POSARG_2) == 0));
    } else {
        mu_assert(false, "Invalid option");
    }
}

MU_TEST(getopt_empty_test) {
    FuriString* args = furi_string_alloc();

    mu_check(getopts(args, "", getopt_test_option_callback, NULL));
    mu_check(getopts(args, OPTS_ALL, getopt_test_option_callback, NULL));

    furi_string_set(args, ARGS_ALL);
    mu_check(!getopts(args, "", getopt_test_option_callback, NULL));

    furi_string_free(args);
}

MU_TEST(getopt_basic_test) {
    FuriString* args = furi_string_alloc_set(ARGS_ALL);

    mu_check(getopts(args, OPTS_ALL, getopt_test_option_callback, getopt_test_context));

    furi_string_free(args);
}

MU_TEST(getopt_posarg_test) {
    FuriString* args = furi_string_alloc_set(ARGS_WITH_POSARGS);

    mu_check(getopts(args, OPTS_ALL, getopt_test_posarg_option_callback, getopt_test_context));

    furi_string_free(args);
}

MU_TEST_SUITE(getopt_test_suite) {
    MU_RUN_TEST(getopt_empty_test);
    MU_RUN_TEST(getopt_basic_test);
    MU_RUN_TEST(getopt_posarg_test);
}

int run_minunit_getopt_test(void) {
    MU_RUN_SUITE(getopt_test_suite);
    return MU_EXIT_CODE;
}
