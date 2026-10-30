#include "../unit_tests.h"
#include <path.h>

static void check_path_normalize(const char* src, const char* expected, bool allow_escape_root) {
    FuriString* result = furi_string_alloc();
    path_normalize(src, result, allow_escape_root);
    mu_assert_string_eq(expected, furi_string_get_cstr(result));
    furi_string_free(result);
}

MU_TEST(path_tests_path_normalize) {
    check_path_normalize("a/bar/cat", "a/bar/cat", true);
    check_path_normalize("a/bar/cat", "a/bar/cat", false);
    check_path_normalize("/a/bar/cat", "/a/bar/cat", true);
    check_path_normalize("/a/bar/cat", "/a/bar/cat", false);

    check_path_normalize("a/././bar/./cat", "a/bar/cat", true);
    check_path_normalize("././a/bar/cat/.", "a/bar/cat", false);

    check_path_normalize("a/bar/../foo/cat/../dog", "a/foo/dog", true);

    check_path_normalize("/a/bar/cat/../../../../././././/.//.././", "/", false);
    check_path_normalize("/a/bar/cat/../../../../././././/.//.././", "/", true);

    check_path_normalize("a/../../../", "../..", true);
}

#define check_path_replace_prefix(src, old_prefix, new_prefix, expected)            \
    {                                                                               \
        FuriString* result = furi_string_alloc();                                   \
        bool is_success = path_replace_prefix(src, old_prefix, new_prefix, result); \
        if(expected == NULL) {                                                      \
            mu_check(!is_success);                                                  \
        } else {                                                                    \
            mu_check(is_success);                                                   \
            mu_assert_string_eq(expected, furi_string_get_cstr(result));            \
        }                                                                           \
        furi_string_free(result);                                                   \
    }

MU_TEST(path_tests_path_replace_prefix) {
    check_path_replace_prefix("/a/b/c/d", "/a/b", "/d", "/d/c/d");
    check_path_replace_prefix("/a/bar/c/d", "/a/b", "/d", NULL);
    check_path_replace_prefix("foo/bar/baz", "foo", "bar", "bar/bar/baz");
    check_path_replace_prefix("/foo/bar/baz", "/foo", "/bar/", "/bar/bar/baz");
    check_path_replace_prefix("/foo/bar/baz", "/foo/", "/bar/", "/bar/bar/baz");
    check_path_replace_prefix("/foo /bar/baz", "/foo", "/bar/", NULL);
    check_path_replace_prefix("/foo /bar/baz", "/foo ", "/bar/", "/bar/bar/baz");
    check_path_replace_prefix("/foo /bar/baz", "/foo /", "/bar/", "/bar/bar/baz");
    check_path_replace_prefix("/foo / bar/baz", "/foo ", "/bar/", "/bar/ bar/baz");
    check_path_replace_prefix("/foo / bar/baz", "/foo /", "/bar/", "/bar/ bar/baz");
    check_path_replace_prefix("/foo/ bar/baz", "/foo/", "/bar/", "/bar/ bar/baz");
    check_path_replace_prefix("/foo/ bar/baz", "/foo", "/bar/", "/bar/ bar/baz");
    check_path_replace_prefix("/foo/ bar/baz", "/foo", "/bar ", "/bar / bar/baz");
    check_path_replace_prefix("/foo/bar/baz", "/", "/bar", "/bar/foo/bar/baz");
    check_path_replace_prefix("/foo/bar/baz", "", "/bar", NULL);
    check_path_replace_prefix("/foo/bar/baz", "/", "/bar/", "/bar/foo/bar/baz");
    check_path_replace_prefix("/foo/bar/baz", "", "/bar/", NULL);
    check_path_replace_prefix("/foo/bar", "/foo/bar", "/baz", "/baz");
    check_path_replace_prefix("/foo/bar", "/foo/bar", "/baz/", "/baz");
    check_path_replace_prefix("/foo/bar/", "/foo/bar", "/baz", "/baz");
    check_path_replace_prefix("/foo/bar/", "/foo/bar", "/baz/", "/baz");
    check_path_replace_prefix("/", "/", "/baz/", "/baz");
    check_path_replace_prefix("/", "", "/baz/", NULL);
    check_path_replace_prefix("", "", "/baz/", "/baz");
    check_path_replace_prefix("", "/foo", "/baz/", NULL);
    check_path_replace_prefix("", "/", "/baz/", NULL);
    check_path_replace_prefix("foo/bar", "foo", "baz", "baz/bar");
    check_path_replace_prefix("foo/bar", "foo", "/baz", "/baz/bar");
    check_path_replace_prefix("foo/bar", "/foo", "/baz", NULL);
    check_path_replace_prefix("/", "/", "/", "/");
    check_path_replace_prefix("/foo", "/foo", "/", "/");
}

MU_TEST_SUITE(path_test_suite) {
    MU_RUN_TEST(path_tests_path_normalize);
    MU_RUN_TEST(path_tests_path_replace_prefix);
}

int run_minunit_path_test(void) {
    MU_RUN_SUITE(path_test_suite);
    return MU_EXIT_CODE;
}
