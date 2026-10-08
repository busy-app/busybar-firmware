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

MU_TEST(semver_missing_places_test) {
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

MU_TEST(semver_extra_places_test) {
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

MU_TEST_SUITE(semver_test_suite) {
    MU_RUN_TEST(semver_empty_test);
    MU_RUN_TEST(semver_valid_test);
    MU_RUN_TEST(semver_limits_test);
    MU_RUN_TEST(semver_missing_places_test);
    MU_RUN_TEST(semver_extra_places_test);
    MU_RUN_TEST(semver_invalid_chars_test);
}

int run_minunit_semver_test(void) {
    MU_RUN_SUITE(semver_test_suite);
    return MU_EXIT_CODE;
}
