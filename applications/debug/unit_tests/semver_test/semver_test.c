#include "../unit_tests.h"

#include <toolbox/semver.h>

#define SEMVER_TEST_MAJOR "1"
#define SEMVER_TEST_MINOR "23"
#define SEMVER_TEST_PATCH "456"

#define SEMVER_TEST_U32_MAX      "4294967295"
#define SEMVER_TEST_U32_OVERFLOW "4294967296"
#define SEMVER_TEST_U32_TOO_LONG "12345678901"

#define SEMVER(major, minor, patch) major "." minor "." patch

#define SEMVER_TEST_VALID SEMVER(SEMVER_TEST_MAJOR, SEMVER_TEST_MINOR, SEMVER_TEST_PATCH)

#define SEMVER_CHECK(semver, major_ref, minor_ref, patch_ref) \
    do {                                                      \
        mu_check((semver).major == (major_ref));              \
        mu_check((semver).minor == (minor_ref));              \
        mu_check((semver).patch == (patch_ref));              \
    } while(false)

#define SEMVER_COMPARE_CHECK_EQUAL(instance, other)                                   \
    do {                                                                              \
        mu_assert_int_eq(SemVerOrderingEqual, semver_compare(&(instance), &(other))); \
        mu_assert_int_eq(SemVerOrderingEqual, semver_compare(&(other), &(instance))); \
    } while(false)

#define SEMVER_COMPARE_CHECK_NEWER(newer, older)                                   \
    do {                                                                           \
        mu_assert_int_eq(SemVerOrderingNewer, semver_compare(&(newer), &(older))); \
        mu_assert_int_eq(SemVerOrderingOlder, semver_compare(&(older), &(newer))); \
    } while(false)

MU_TEST(semver_empty_test) {
    SemVer semver;

    mu_check(!semver_parse(&semver, ""));
    mu_check(!semver_parse(&semver, " "));
    mu_check(!semver_parse(&semver, "."));
    mu_check(!semver_parse(&semver, ".."));
    mu_check(!semver_parse(&semver, "..."));
}

MU_TEST(semver_valid_test) {
    SemVer semver;

    mu_check(semver_parse(&semver, SEMVER_TEST_VALID));
    SEMVER_CHECK(semver, 1, 23, 456);

    mu_check(semver_parse(&semver, SEMVER("0", "0", "0")));
    SEMVER_CHECK(semver, 0, 0, 0);

    mu_check(semver_parse(&semver, SEMVER("10", "0", "1")));
    SEMVER_CHECK(semver, 10, 0, 1);
}

MU_TEST(semver_limits_test) {
    SemVer semver;

    mu_check(semver_parse(
        &semver, SEMVER(SEMVER_TEST_U32_MAX, SEMVER_TEST_U32_MAX, SEMVER_TEST_U32_MAX)));
    SEMVER_CHECK(semver, UINT32_MAX, UINT32_MAX, UINT32_MAX);

    mu_check(!semver_parse(
        &semver, SEMVER(SEMVER_TEST_U32_OVERFLOW, SEMVER_TEST_MINOR, SEMVER_TEST_PATCH)));
    mu_check(!semver_parse(
        &semver, SEMVER(SEMVER_TEST_MAJOR, SEMVER_TEST_U32_OVERFLOW, SEMVER_TEST_PATCH)));
    mu_check(!semver_parse(
        &semver, SEMVER(SEMVER_TEST_MAJOR, SEMVER_TEST_MINOR, SEMVER_TEST_U32_OVERFLOW)));

    mu_check(!semver_parse(
        &semver, SEMVER(SEMVER_TEST_U32_TOO_LONG, SEMVER_TEST_MINOR, SEMVER_TEST_PATCH)));
    mu_check(!semver_parse(
        &semver, SEMVER(SEMVER_TEST_MAJOR, SEMVER_TEST_U32_TOO_LONG, SEMVER_TEST_PATCH)));
    mu_check(!semver_parse(
        &semver, SEMVER(SEMVER_TEST_MAJOR, SEMVER_TEST_MINOR, SEMVER_TEST_U32_TOO_LONG)));
}

MU_TEST(semver_missing_parts_test) {
    SemVer semver;

    mu_check(!semver_parse(&semver, SEMVER_TEST_MAJOR));
    mu_check(!semver_parse(&semver, SEMVER_TEST_MAJOR "."));
    mu_check(!semver_parse(&semver, "." SEMVER_TEST_MINOR));
    mu_check(!semver_parse(&semver, SEMVER_TEST_MAJOR "." SEMVER_TEST_MINOR));
    mu_check(!semver_parse(&semver, SEMVER_TEST_MAJOR "." SEMVER_TEST_MINOR "."));
    mu_check(!semver_parse(&semver, "." SEMVER_TEST_MINOR "." SEMVER_TEST_PATCH));
    mu_check(!semver_parse(&semver, SEMVER("", "", SEMVER_TEST_PATCH)));
    mu_check(!semver_parse(&semver, SEMVER(SEMVER_TEST_MAJOR, "", SEMVER_TEST_PATCH)));
    mu_check(!semver_parse(&semver, SEMVER(SEMVER_TEST_MAJOR, "", "")));
}

MU_TEST(semver_extra_parts_test) {
    SemVer semver;

    mu_check(!semver_parse(&semver, SEMVER_TEST_VALID "."));
    mu_check(!semver_parse(&semver, "." SEMVER_TEST_VALID));
    mu_check(!semver_parse(&semver, SEMVER_TEST_VALID ".4"));
    mu_check(!semver_parse(&semver, SEMVER_TEST_VALID ".4.5"));
    mu_check(
        !semver_parse(&semver, SEMVER_TEST_MAJOR ".." SEMVER_TEST_MINOR "." SEMVER_TEST_PATCH));
}

MU_TEST(semver_invalid_chars_test) {
    SemVer semver;

    mu_check(!semver_parse(&semver, " " SEMVER_TEST_VALID));
    mu_check(!semver_parse(&semver, SEMVER_TEST_VALID " "));
    mu_check(!semver_parse(
        &semver, SEMVER(SEMVER_TEST_MAJOR, " " SEMVER_TEST_MINOR, SEMVER_TEST_PATCH)));
    mu_check(!semver_parse(&semver, "v" SEMVER_TEST_VALID));
    mu_check(!semver_parse(&semver, "+" SEMVER_TEST_VALID));
    mu_check(!semver_parse(&semver, "-" SEMVER_TEST_VALID));
    mu_check(!semver_parse(&semver, SEMVER_TEST_VALID "-rc1"));
    mu_check(!semver_parse(&semver, SEMVER_TEST_VALID "+build"));
    mu_check(!semver_parse(&semver, SEMVER("a", "b", "c")));
    mu_check(!semver_parse(&semver, SEMVER("0x1", SEMVER_TEST_MINOR, SEMVER_TEST_PATCH)));
    mu_check(!semver_parse(&semver, SEMVER(SEMVER_TEST_MAJOR, SEMVER_TEST_MINOR, "1e3")));
    mu_check(
        !semver_parse(&semver, SEMVER_TEST_MAJOR "," SEMVER_TEST_MINOR "," SEMVER_TEST_PATCH));
    mu_check(!semver_parse(&semver, SEMVER(SEMVER_TEST_MAJOR, SEMVER_TEST_MINOR, "\xd9\xa3")));
}

MU_TEST(semver_compare_equal_test) {
    const SemVer semver = {1, 23, 456};
    const SemVer semver_copy = semver;
    const SemVer semver_zero = {0, 0, 0};
    const SemVer semver_max = {UINT32_MAX, UINT32_MAX, UINT32_MAX};

    mu_assert_int_eq(SemVerOrderingEqual, semver_compare(&semver, &semver));
    SEMVER_COMPARE_CHECK_EQUAL(semver, semver_copy);
    SEMVER_COMPARE_CHECK_EQUAL(semver_zero, semver_zero);
    SEMVER_COMPARE_CHECK_EQUAL(semver_max, semver_max);
}

MU_TEST(semver_compare_major_test) {
    const SemVer semver = {1, 23, 456};
    const SemVer semver_major_newer = {2, 0, 0};
    const SemVer semver_major_older = {0, 99, 999};

    SEMVER_COMPARE_CHECK_NEWER(semver_major_newer, semver);
    SEMVER_COMPARE_CHECK_NEWER(semver, semver_major_older);
    SEMVER_COMPARE_CHECK_NEWER(semver_major_newer, semver_major_older);
}

MU_TEST(semver_compare_minor_test) {
    const SemVer semver = {1, 23, 456};
    const SemVer semver_minor_newer = {1, 24, 0};
    const SemVer semver_minor_older = {1, 22, 999};

    SEMVER_COMPARE_CHECK_NEWER(semver_minor_newer, semver);
    SEMVER_COMPARE_CHECK_NEWER(semver, semver_minor_older);
    SEMVER_COMPARE_CHECK_NEWER(semver_minor_newer, semver_minor_older);
}

MU_TEST(semver_compare_patch_test) {
    const SemVer semver = {1, 23, 456};
    const SemVer semver_patch_newer = {1, 23, 457};
    const SemVer semver_patch_older = {1, 23, 455};

    SEMVER_COMPARE_CHECK_NEWER(semver_patch_newer, semver);
    SEMVER_COMPARE_CHECK_NEWER(semver, semver_patch_older);
    SEMVER_COMPARE_CHECK_NEWER(semver_patch_newer, semver_patch_older);
}

MU_TEST(semver_compare_limits_test) {
    const SemVer semver_zero = {0, 0, 0};
    const SemVer semver_max = {UINT32_MAX, UINT32_MAX, UINT32_MAX};
    const SemVer semver_max_major = {UINT32_MAX, 0, 0};
    const SemVer semver_max_minor = {0, UINT32_MAX, 0};
    const SemVer semver_max_patch = {0, 0, UINT32_MAX};
    const SemVer semver_almost_max = {UINT32_MAX, UINT32_MAX, UINT32_MAX - 1};

    SEMVER_COMPARE_CHECK_NEWER(semver_max, semver_zero);
    SEMVER_COMPARE_CHECK_NEWER(semver_max, semver_almost_max);
    SEMVER_COMPARE_CHECK_NEWER(semver_max_major, semver_max_minor);
    SEMVER_COMPARE_CHECK_NEWER(semver_max_minor, semver_max_patch);
    SEMVER_COMPARE_CHECK_NEWER(semver_max_patch, semver_zero);
}

MU_TEST_SUITE(semver_test_suite) {
    MU_RUN_TEST(semver_empty_test);
    MU_RUN_TEST(semver_valid_test);
    MU_RUN_TEST(semver_limits_test);
    MU_RUN_TEST(semver_missing_parts_test);
    MU_RUN_TEST(semver_extra_parts_test);
    MU_RUN_TEST(semver_invalid_chars_test);
    MU_RUN_TEST(semver_compare_equal_test);
    MU_RUN_TEST(semver_compare_major_test);
    MU_RUN_TEST(semver_compare_minor_test);
    MU_RUN_TEST(semver_compare_patch_test);
    MU_RUN_TEST(semver_compare_limits_test);
}

int run_minunit_semver_test(void) {
    MU_RUN_SUITE(semver_test_suite);
    return MU_EXIT_CODE;
}
