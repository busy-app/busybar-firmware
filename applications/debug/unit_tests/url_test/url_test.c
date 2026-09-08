#include "../unit_tests.h"

#include <toolbox/url.h>

#define URL_TEST_PROTOCOL "http:"
#define URL_TEST_HOSTNAME "127.0.0.1"
#define URL_TEST_PORT     "8080"
#define URL_TEST_PATHNAME "/api/my_endpoint"
#define URL_TEST_SEARCH   "?my_arg=awesome&hello=there"

#define URL_TEST_REQUIRED     URL_TEST_PROTOCOL "//" URL_TEST_HOSTNAME
#define URL_TEST_USECASE      URL_TEST_REQUIRED URL_TEST_PATHNAME
#define URL_TEST_USECASE_ARGS URL_TEST_USECASE URL_TEST_SEARCH
#define URL_TEST_FULL         URL_TEST_REQUIRED ":" URL_TEST_PORT URL_TEST_PATHNAME URL_TEST_SEARCH
#define URL_TEST_EMPTY_PATH   URL_TEST_REQUIRED "/"

#define URL_CHECK_PART(url, part_id, ref)                            \
    do {                                                             \
        const StringSlice* __part = url_get_part((url), (part_id));  \
        mu_assert_not_null(__part);                                  \
        mu_assert_int_eq(strlen(ref), __part->length);               \
        mu_assert_mem_eq((ref), __part->first_char, __part->length); \
    } while(false)

MU_TEST(url_empty_test) {
    Url* url = url_alloc();

    for(uint32_t i = 0; i < UrlPartIdMax; ++i) {
        const StringSlice* part = url_get_part(url, i);
        mu_assert_not_null(part);
        mu_assert_not_null(part->first_char);
        mu_assert_int_eq(0, part->length);
    }

    url_free(url);
}

MU_TEST(url_required_test) {
    Url* url = url_alloc();

    mu_check(!url_parse(url, ""));
    mu_check(!url_parse(url, "abcd"));
    mu_check(!url_parse(url, URL_TEST_HOSTNAME));
    mu_check(!url_parse(url, URL_TEST_PROTOCOL "//"));

    mu_check(url_parse(url, URL_TEST_REQUIRED));

    URL_CHECK_PART(url, UrlPartIdProtocol, URL_TEST_PROTOCOL);
    URL_CHECK_PART(url, UrlPartIdHost, URL_TEST_HOSTNAME);
    URL_CHECK_PART(url, UrlPartIdHostname, URL_TEST_HOSTNAME);
    URL_CHECK_PART(url, UrlPartIdOrigin, URL_TEST_REQUIRED);
    URL_CHECK_PART(url, UrlPartIdHref, URL_TEST_REQUIRED);

    url_free(url);
}

MU_TEST(url_usecase_test) {
    Url* url = url_alloc();

    mu_check(url_parse(url, URL_TEST_USECASE));

    URL_CHECK_PART(url, UrlPartIdProtocol, URL_TEST_PROTOCOL);
    URL_CHECK_PART(url, UrlPartIdHost, URL_TEST_HOSTNAME);
    URL_CHECK_PART(url, UrlPartIdHostname, URL_TEST_HOSTNAME);
    URL_CHECK_PART(url, UrlPartIdOrigin, URL_TEST_REQUIRED);
    URL_CHECK_PART(url, UrlPartIdHref, URL_TEST_USECASE);
    URL_CHECK_PART(url, UrlPartIdPathname, URL_TEST_PATHNAME);

    url_free(url);
}

MU_TEST(url_usecase_args_test) {
    Url* url = url_alloc();

    mu_check(url_parse(url, URL_TEST_USECASE_ARGS));

    URL_CHECK_PART(url, UrlPartIdProtocol, URL_TEST_PROTOCOL);
    URL_CHECK_PART(url, UrlPartIdHost, URL_TEST_HOSTNAME);
    URL_CHECK_PART(url, UrlPartIdHostname, URL_TEST_HOSTNAME);
    URL_CHECK_PART(url, UrlPartIdOrigin, URL_TEST_REQUIRED);
    URL_CHECK_PART(url, UrlPartIdHref, URL_TEST_USECASE_ARGS);
    URL_CHECK_PART(url, UrlPartIdPathname, URL_TEST_PATHNAME);
    URL_CHECK_PART(url, UrlPartIdSearch, URL_TEST_SEARCH);

    url_free(url);
}

MU_TEST(url_full_test) {
    Url* url = url_alloc();

    mu_check(url_parse(url, URL_TEST_FULL));

    URL_CHECK_PART(url, UrlPartIdProtocol, URL_TEST_PROTOCOL);
    URL_CHECK_PART(url, UrlPartIdHost, URL_TEST_HOSTNAME ":" URL_TEST_PORT);
    URL_CHECK_PART(url, UrlPartIdHostname, URL_TEST_HOSTNAME);
    URL_CHECK_PART(url, UrlPartIdPort, URL_TEST_PORT);
    URL_CHECK_PART(url, UrlPartIdOrigin, URL_TEST_REQUIRED ":" URL_TEST_PORT);
    URL_CHECK_PART(url, UrlPartIdHref, URL_TEST_FULL);
    URL_CHECK_PART(url, UrlPartIdPathname, URL_TEST_PATHNAME);
    URL_CHECK_PART(url, UrlPartIdSearch, URL_TEST_SEARCH);

    url_free(url);
}

MU_TEST(url_empty_path_test) {
    Url* url = url_alloc();

    mu_check(url_parse(url, URL_TEST_EMPTY_PATH));

    URL_CHECK_PART(url, UrlPartIdProtocol, URL_TEST_PROTOCOL);
    URL_CHECK_PART(url, UrlPartIdHost, URL_TEST_HOSTNAME);
    URL_CHECK_PART(url, UrlPartIdHostname, URL_TEST_HOSTNAME);
    URL_CHECK_PART(url, UrlPartIdOrigin, URL_TEST_REQUIRED);
    URL_CHECK_PART(url, UrlPartIdHref, URL_TEST_EMPTY_PATH);
    URL_CHECK_PART(url, UrlPartIdPathname, "/");

    url_free(url);
}

MU_TEST_SUITE(url_test_suite) {
    MU_RUN_TEST(url_empty_test);
    MU_RUN_TEST(url_required_test);
    MU_RUN_TEST(url_usecase_test);
    MU_RUN_TEST(url_usecase_args_test);
    MU_RUN_TEST(url_full_test);
    MU_RUN_TEST(url_empty_path_test);
}

int run_minunit_url_test(void) {
    MU_RUN_SUITE(url_test_suite);
    return MU_EXIT_CODE;
}
