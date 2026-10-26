#include "../unit_tests.h"

#include <toolbox/url.h>

#define URL_TEST_PROTOCOL "http:"
#define URL_TEST_HOSTNAME "127.0.0.1"
#define URL_TEST_PORT     "8080"
#define URL_TEST_PATHNAME "/api/my_endpoint:v1"
#define URL_TEST_SEARCH   "?my_arg=awesome&hello=there&time=12:00&next=/a/b"

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
    URL_CHECK_PART(url, UrlPartIdHref, URL_TEST_REQUIRED "/");

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

MU_TEST(url_reset_on_failure_test) {
    Url* url = url_alloc();

    mu_check(url_parse(url, URL_TEST_FULL));
    mu_check(!url_parse(url, "abcd"));

    for(uint32_t i = 0; i < UrlPartIdMax; ++i) {
        const StringSlice* part = url_get_part(url, i);
        mu_assert_not_null(part);
        mu_assert_not_null(part->first_char);
        mu_assert_int_eq(0, part->length);
    }

    url_free(url);
}

MU_TEST(url_encode_protocol_test) {
    Url* url = url_alloc();

    mu_check(!url_parse(url, "http//" URL_TEST_HOSTNAME));
    mu_check(!url_parse(url, "ht1p://" URL_TEST_HOSTNAME));
    mu_check(!url_parse(url, "ht-p://" URL_TEST_HOSTNAME));
    mu_check(!url_parse(url, "ht%70://" URL_TEST_HOSTNAME));
    mu_check(!url_parse(url, "httп://" URL_TEST_HOSTNAME));

    mu_check(url_parse(url, "HtTpS://" URL_TEST_HOSTNAME));
    URL_CHECK_PART(url, UrlPartIdProtocol, "https:");
    URL_CHECK_PART(url, UrlPartIdHref, "https://" URL_TEST_HOSTNAME "/");

    url_free(url);
}

MU_TEST(url_encode_hostname_test) {
    Url* url = url_alloc();

    mu_check(!url_parse(url, "http://龙.example/"));
    mu_check(!url_parse(url, "http://bücher.example"));
    mu_check(!url_parse(url, "http://b%C3%BCcher.example/"));
    mu_check(!url_parse(url, "http://my host.example/"));
    mu_check(!url_parse(url, "http://my~host.example/"));
    mu_check(!url_parse(url, "http://user@host.example/"));

    mu_check(url_parse(url, "http://WWW.Example-Host_1.COM/Path"));
    URL_CHECK_PART(url, UrlPartIdHostname, "www.example-host_1.com");
    URL_CHECK_PART(url, UrlPartIdHost, "www.example-host_1.com");
    URL_CHECK_PART(url, UrlPartIdOrigin, "http://www.example-host_1.com");
    URL_CHECK_PART(url, UrlPartIdPathname, "/Path");
    URL_CHECK_PART(url, UrlPartIdHref, "http://www.example-host_1.com/Path");

    mu_check(url_parse(url, "http://xn--bcher-kva.example/"));
    URL_CHECK_PART(url, UrlPartIdHostname, "xn--bcher-kva.example");
    URL_CHECK_PART(url, UrlPartIdHref, "http://xn--bcher-kva.example/");

    url_free(url);
}

MU_TEST(url_encode_port_test) {
    Url* url = url_alloc();

    mu_check(!url_parse(url, URL_TEST_REQUIRED ":80a/"));
    mu_check(!url_parse(url, URL_TEST_REQUIRED ":-80/"));
    mu_check(!url_parse(url, URL_TEST_REQUIRED ":8%30/"));
    mu_check(!url_parse(url, URL_TEST_REQUIRED ":8\xd9\xa0/"));
    mu_check(!url_parse(url, URL_TEST_REQUIRED ":"));
    mu_check(!url_parse(url, URL_TEST_REQUIRED ":/"));
    mu_check(!url_parse(url, URL_TEST_REQUIRED ":" URL_TEST_SEARCH));

    mu_check(url_parse(url, URL_TEST_REQUIRED ":" URL_TEST_PORT "/"));
    URL_CHECK_PART(url, UrlPartIdPort, URL_TEST_PORT);

    mu_check(url_parse(url, URL_TEST_REQUIRED ":" URL_TEST_PORT URL_TEST_SEARCH));
    URL_CHECK_PART(url, UrlPartIdHostname, URL_TEST_HOSTNAME);
    URL_CHECK_PART(url, UrlPartIdPort, URL_TEST_PORT);
    URL_CHECK_PART(url, UrlPartIdPathname, "/");
    URL_CHECK_PART(url, UrlPartIdSearch, URL_TEST_SEARCH);

    url_free(url);
}

MU_TEST(url_encode_pathname_test) {
    Url* url = url_alloc();

    mu_check(url_parse(url, URL_TEST_REQUIRED "/my path/\"<>\\^`{|}"));
    URL_CHECK_PART(url, UrlPartIdPathname, "/my%20path/%22%3C%3E%5C%5E%60%7B%7C%7D");
    URL_CHECK_PART(url, UrlPartIdHref, URL_TEST_REQUIRED "/my%20path/%22%3C%3E%5C%5E%60%7B%7C%7D");

    mu_check(url_parse(
        url,
        URL_TEST_REQUIRED "/b\xc3\xbc"
                          "cher/\xe2\x82\xac"));
    URL_CHECK_PART(url, UrlPartIdPathname, "/b%C3%BCcher/%E2%82%AC");

    mu_check(url_parse(url, URL_TEST_REQUIRED "/a\tb\x7f"));
    URL_CHECK_PART(url, UrlPartIdPathname, "/a%09b%7F");

    mu_check(url_parse(url, URL_TEST_REQUIRED "/AZaz09-._~/#[]@!$&'()*+,;="));
    URL_CHECK_PART(url, UrlPartIdPathname, "/AZaz09-._~/#[]@!$&'()*+,;=");

    url_free(url);
}

MU_TEST(url_encode_search_test) {
    Url* url = url_alloc();

    mu_check(url_parse(url, URL_TEST_USECASE "?q=hello world&name=J\xc3\xbcrgen&x=\"y\""));
    URL_CHECK_PART(url, UrlPartIdPathname, URL_TEST_PATHNAME);
    URL_CHECK_PART(url, UrlPartIdSearch, "?q=hello%20world&name=J%C3%BCrgen&x=%22y%22");
    URL_CHECK_PART(
        url, UrlPartIdHref, URL_TEST_USECASE "?q=hello%20world&name=J%C3%BCrgen&x=%22y%22");

    mu_check(url_parse(url, URL_TEST_USECASE "?a=b&c=d/e?f@h#frag"));
    URL_CHECK_PART(url, UrlPartIdSearch, "?a=b&c=d/e?f@h#frag");

    mu_check(url_parse(url, URL_TEST_REQUIRED URL_TEST_SEARCH));
    URL_CHECK_PART(url, UrlPartIdHostname, URL_TEST_HOSTNAME);
    URL_CHECK_PART(url, UrlPartIdPort, "");
    URL_CHECK_PART(url, UrlPartIdPathname, "/");
    URL_CHECK_PART(url, UrlPartIdSearch, URL_TEST_SEARCH);

    url_free(url);
}

MU_TEST(url_encode_full_test) {
    Url* url = url_alloc();

    mu_check(
        url_parse(url, "HTTP://My-Host.Local:8080/some dir/f\xc3\xa4il.txt?k=v w&\xc3\xb1=1"));

    URL_CHECK_PART(url, UrlPartIdProtocol, "http:");
    URL_CHECK_PART(url, UrlPartIdHostname, "my-host.local");
    URL_CHECK_PART(url, UrlPartIdPort, "8080");
    URL_CHECK_PART(url, UrlPartIdHost, "my-host.local:8080");
    URL_CHECK_PART(url, UrlPartIdOrigin, "http://my-host.local:8080");
    URL_CHECK_PART(url, UrlPartIdPathname, "/some%20dir/f%C3%A4il.txt");
    URL_CHECK_PART(url, UrlPartIdSearch, "?k=v%20w&%C3%B1=1");
    URL_CHECK_PART(
        url, UrlPartIdHref, "http://my-host.local:8080/some%20dir/f%C3%A4il.txt?k=v%20w&%C3%B1=1");

    url_free(url);
}

MU_TEST(url_no_double_encoding_test) {
    Url* url = url_alloc();

    const char* encoded = "http://my-host.local:8080/some%20dir/f%C3%A4il.txt?k=v%20w&%C3%B1=1%25";

    mu_check(url_parse(url, encoded));
    URL_CHECK_PART(url, UrlPartIdPathname, "/some%20dir/f%C3%A4il.txt");
    URL_CHECK_PART(url, UrlPartIdSearch, "?k=v%20w&%C3%B1=1%25");
    URL_CHECK_PART(url, UrlPartIdHref, encoded);

    mu_check(url_parse(url, URL_TEST_REQUIRED "/a%20b c?x=%22y\""));
    URL_CHECK_PART(url, UrlPartIdPathname, "/a%20b%20c");
    URL_CHECK_PART(url, UrlPartIdSearch, "?x=%22y%22");

    mu_check(url_parse(url, "HTTP://Host.Local/d\xc3\xafr name?q=a b"));
    const StringSlice* href = url_get_part(url, UrlPartIdHref);

    FuriString* first_pass = furi_string_alloc();
    furi_string_set_strn(first_pass, href->first_char, href->length);
    const char* first_pass_str = furi_string_get_cstr(first_pass);

    mu_assert_string_eq("http://host.local/d%C3%AFr%20name?q=a%20b", first_pass_str);

    mu_check(url_parse(url, first_pass_str));
    URL_CHECK_PART(url, UrlPartIdHref, first_pass_str);

    furi_string_free(first_pass);

    url_free(url);
}

MU_TEST_SUITE(url_test_suite) {
    MU_RUN_TEST(url_empty_test);
    MU_RUN_TEST(url_required_test);
    MU_RUN_TEST(url_usecase_test);
    MU_RUN_TEST(url_usecase_args_test);
    MU_RUN_TEST(url_full_test);
    MU_RUN_TEST(url_empty_path_test);
    MU_RUN_TEST(url_reset_on_failure_test);
    MU_RUN_TEST(url_encode_protocol_test);
    MU_RUN_TEST(url_encode_hostname_test);
    MU_RUN_TEST(url_encode_port_test);
    MU_RUN_TEST(url_encode_pathname_test);
    MU_RUN_TEST(url_encode_search_test);
    MU_RUN_TEST(url_encode_full_test);
    MU_RUN_TEST(url_no_double_encoding_test);
}

int run_minunit_url_test(void) {
    MU_RUN_SUITE(url_test_suite);
    return MU_EXIT_CODE;
}
