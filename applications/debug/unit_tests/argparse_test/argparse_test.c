#include "../unit_tests.h"

#include <toolbox/argparse.h>

#define OPT_A "a"
#define OPT_B "b"
#define OPT_C "c"
#define OPT_D "D"
#define OPT_E "E"
#define OPT_F "F"
#define OPT_G "g"

#define OPTS_ALL OPT_A ":" OPT_B ":" OPT_C ":" OPT_D OPT_E OPT_F OPT_G ":"

#define OPTVAL_A "hello"
#define OPTVAL_B "42"
#define OPTVAL_C "0xdeadbeef"
#define OPTVAL_D // Empty
#define OPTVAL_E // Empty
#define OPTVAL_F // Empty
#define OPTVAL_G "This is a\tstring \twith\t spaces"

#define POSARG_1 "lorem"
#define POSARG_2 "ipsum"
#define POSARG_3 "This \t is another \tstring with\t spaces"
#define POSARG_4 "https://my.awesome.site.link/endpoint?param=hello:999"

#define ARG(opt, optval)        "-" opt " " optval
#define ARG_TAB(opt, optval)    "-" opt "\t" optval
#define ARG_CONCAT(opt, optval) "-" opt optval

#define ARGS_SPACE " \n  \t  \r   \n  \t\t\t "

#define ARGS_SIMPLE                                                       \
    ARG(OPT_A, OPTVAL_A)                                                  \
    " " ARG_TAB(OPT_B, OPTVAL_B) " " ARG_CONCAT(OPT_C, OPTVAL_C) " " ARG( \
        OPT_D, OPTVAL_D) " " ARG(OPT_E, OPTVAL_E) " " ARG(OPT_F, OPTVAL_F)

#define ARGS_WITH_POSARGS                                                                   \
    ARG(OPT_A, OPTVAL_A)                                                                    \
    " " POSARG_1 "\t" ARG_CONCAT(OPT_B, OPTVAL_B) " \t " POSARG_2                           \
                                                  " " ARG_TAB(OPT_C, OPTVAL_C) " " POSARG_1 \
                                                                               " \t "
#define ARGS_WITH_QUOTES                                                \
    ARG_TAB(OPT_A, "\"" OPTVAL_A "\"")                                  \
    " " ARG(OPT_G, "\"" OPTVAL_G "\"") " \"" POSARG_3 "\" " ARG_CONCAT( \
        OPT_B, "'" OPTVAL_B "'") " " ARG(OPT_G, "'" OPTVAL_G "'") " '" POSARG_3 "' "

#define ARGS_POSARG_ONLY "\t " POSARG_1 " " POSARG_4 " \"" POSARG_4 "\""

#define ARGS_MISSING_OPTARG ARG_CONCAT(OPT_A, "") " " ARG_CONCAT(OPT_B, "")

#define ARGS_UNKNOWN_OPTS ARG_CONCAT("X", "") " " ARG_CONCAT("Y", "") ARG_CONCAT("Z", "")

#define FIRST_CHAR(str) (str[0])

typedef struct {
    uint32_t run_count;
} ArgparseTestContext;

static void argparse_test_option_callback(char opt, const char* optval, void* context) {
    mu_assert_not_null(context);

    ArgparseTestContext* ctx = context;
    ++ctx->run_count;

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
        mu_fail("Invalid option");
    }
}

static void argparse_test_mixed_option_callback(char opt, const char* optval, void* context) {
    mu_assert_not_null(context);

    ArgparseTestContext* ctx = context;
    ++ctx->run_count;

    if(opt == FIRST_CHAR(OPT_A)) {
        mu_assert_string_eq(OPTVAL_A, optval);
    } else if(opt == FIRST_CHAR(OPT_B)) {
        mu_assert_string_eq(OPTVAL_B, optval);
    } else if(opt == FIRST_CHAR(OPT_C)) {
        mu_assert_string_eq(OPTVAL_C, optval);
    } else if(opt == 0) {
        mu_check((strcmp(optval, POSARG_1) == 0) || (strcmp(optval, POSARG_2) == 0));
    } else {
        mu_fail("Invalid option");
    }
}

static void argparse_test_quoted_option_callback(char opt, const char* optval, void* context) {
    mu_assert_not_null(context);

    ArgparseTestContext* ctx = context;
    ++ctx->run_count;

    if(opt == FIRST_CHAR(OPT_A)) {
        mu_assert_string_eq(OPTVAL_A, optval);
    } else if(opt == FIRST_CHAR(OPT_B)) {
        mu_assert_string_eq(OPTVAL_B, optval);
    } else if(opt == FIRST_CHAR(OPT_G)) {
        mu_assert_string_eq(OPTVAL_G, optval);
    } else if(opt == 0) {
        mu_assert_string_eq(POSARG_3, optval);
    } else {
        mu_fail("Invalid option");
    }
}

static void argparse_test_posarg_option_callback(char opt, const char* optval, void* context) {
    mu_assert_not_null(context);

    ArgparseTestContext* ctx = context;
    ++ctx->run_count;

    if(opt == 0) {
        mu_check((strcmp(optval, POSARG_1) == 0) || (strcmp(optval, POSARG_4) == 0));
    } else {
        mu_fail("Invalid option");
    }
}

MU_TEST(argparse_empty_test) {
    FuriString* args = furi_string_alloc();

    mu_check(parse_args(args, NULL, argparse_test_option_callback, NULL));
    mu_check(parse_args(args, OPTS_ALL, argparse_test_option_callback, NULL));

    furi_string_set(args, ARGS_SPACE);

    mu_check(parse_args(args, NULL, argparse_test_option_callback, NULL));
    mu_check(parse_args(args, OPTS_ALL, argparse_test_option_callback, NULL));

    furi_string_set(args, ARGS_SIMPLE);
    mu_check(!parse_args(args, NULL, argparse_test_option_callback, NULL));

    furi_string_free(args);
}

MU_TEST(argparse_simple_test) {
    ArgparseTestContext context = {0};
    FuriString* args = furi_string_alloc_set(ARGS_SIMPLE);

    mu_check(parse_args(args, OPTS_ALL, argparse_test_option_callback, &context));
    mu_assert_int_eq(6, context.run_count);

    furi_string_free(args);
}

MU_TEST(argparse_mixed_test) {
    ArgparseTestContext context = {0};
    FuriString* args = furi_string_alloc_set(ARGS_WITH_POSARGS);

    mu_check(parse_args(args, OPTS_ALL, argparse_test_mixed_option_callback, &context));
    mu_assert_int_eq(6, context.run_count);

    furi_string_free(args);
}

MU_TEST(argparse_quoted_test) {
    ArgparseTestContext context = {0};
    FuriString* args = furi_string_alloc_set(ARGS_WITH_QUOTES);

    mu_check(parse_args(args, OPTS_ALL, argparse_test_quoted_option_callback, &context));
    mu_assert_int_eq(6, context.run_count);

    furi_string_free(args);
}

MU_TEST(argparse_posarg_test) {
    ArgparseTestContext context = {0};
    FuriString* args = furi_string_alloc_set(ARGS_POSARG_ONLY);

    mu_check(parse_args(args, NULL, argparse_test_posarg_option_callback, &context));
    mu_assert_int_eq(3, context.run_count);

    furi_string_free(args);
}

MU_TEST(argparse_error_test) {
    FuriString* args = furi_string_alloc_set(ARGS_MISSING_OPTARG);
    mu_check(!parse_args(args, OPTS_ALL, argparse_test_option_callback, NULL));

    furi_string_set(args, ARGS_UNKNOWN_OPTS);
    mu_check(!parse_args(args, OPTS_ALL, argparse_test_option_callback, NULL));

    furi_string_free(args);
}

MU_TEST_SUITE(argparse_test_suite) {
    MU_RUN_TEST(argparse_empty_test);
    MU_RUN_TEST(argparse_simple_test);
    MU_RUN_TEST(argparse_mixed_test);
    MU_RUN_TEST(argparse_quoted_test);
    MU_RUN_TEST(argparse_posarg_test);
    MU_RUN_TEST(argparse_error_test);
}

int run_minunit_argparse_test(void) {
    MU_RUN_SUITE(argparse_test_suite);
    return MU_EXIT_CODE;
}
