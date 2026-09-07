#include "../unit_tests.h"

#include <wifi/wifi_util.h>
#include <path.h>

#include <http/http_response.h>
#include <http/http_headers.h>

MU_TEST(misc_tests_ipv6_format) {
    char buf[40];

    // unspecified
    WifiIpv6 addr = {0};
    wifi_format_ipv6(&addr, buf, sizeof(buf));
    mu_assert_string_eq("::", buf);

    // localhost
    addr = (WifiIpv6){.bytes = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}};
    wifi_format_ipv6(&addr, buf, sizeof(buf));
    mu_assert_string_eq("::1", buf);

    // zeros at the end
    addr = (WifiIpv6){.bytes = {0xca, 0xfe, 0x13, 0x37, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    wifi_format_ipv6(&addr, buf, sizeof(buf));
    mu_assert_string_eq("cafe:1337::", buf);

    // zeros in the middle
    addr = (WifiIpv6){.bytes = {0xca, 0xfe, 0x13, 0x37, 0, 0, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4}};
    wifi_format_ipv6(&addr, buf, sizeof(buf));
    mu_assert_string_eq("cafe:1337::1:2:3:4", buf);

    // one zero
    addr = (WifiIpv6){.bytes = {0xca, 0xfe, 0x13, 0x37, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5}};
    wifi_format_ipv6(&addr, buf, sizeof(buf));
    mu_assert_string_eq("cafe:1337:0:1:2:3:4:5", buf);

    // two equal zero runs
    addr = (WifiIpv6){.bytes = {0xca, 0xfe, 0x13, 0x37, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2}};
    wifi_format_ipv6(&addr, buf, sizeof(buf));
    mu_assert_string_eq("cafe:1337::1:0:0:2", buf);
}

MU_TEST(misc_tests_ipv4_format) {
    char buf[16];
    WifiIpv4 addr = {.bytes = {192, 168, 0, 1}};

    wifi_format_ipv4(&addr, buf, sizeof(buf));
    mu_assert_string_eq("192.168.0.1", buf);
}

static void check_path_normalize(const char* src, const char* expected, bool allow_escape_root) {
    FuriString* result = furi_string_alloc();
    path_normalize(src, result, allow_escape_root);
    mu_assert_string_eq(expected, furi_string_get_cstr(result));
    furi_string_free(result);
}

MU_TEST(misc_tests_path_normalize) {
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

#define CHECK_HEADER_BY_INDEX(index, k, v)                                      \
    do {                                                                        \
        const HttpHeader* __header = http_headers_get_by_index(headers, index); \
        mu_assert_string_eq(k, furi_string_get_cstr(__header->key));            \
        mu_assert_string_eq(v, furi_string_get_cstr(__header->value));          \
    } while(false)

#define CHECK_HEADER(k, v)                                             \
    do {                                                               \
        const HttpHeader* __header = http_headers_get(headers, k);     \
        mu_assert_not_null(__header);                                  \
        mu_assert_string_eq(k, furi_string_get_cstr(__header->key));   \
        mu_assert_string_eq(v, furi_string_get_cstr(__header->value)); \
    } while(false)

typedef struct {
    const char* key;
    const char* value;
} HttpHeaderTest;

static const HttpHeaderTest misc_test_http_headers[] = {
    [0] = {"Server", "nginx/1.18.0"},
    [1] = {"Date", "Tue, 28 Jul 2026 13:17:26 GMT"},
    [2] = {"Content-Type", "text/html"},
    [3] = {"Content-Length", "4592"},
    [4] = {"Last-Modified", "Tue, 09 Apr 2024 06:23:39 GMT"},
    [5] = {"Connection", "close"},
    [6] = {"Accept-Ranges", "bytes"},
};

MU_TEST(misc_tests_http_headers) {
    FuriString* response_str = furi_string_alloc_set("HTTP/1.1 200 OK\r\n");

    for(uint32_t i = 0; i < COUNT_OF(misc_test_http_headers); ++i) {
        const HttpHeaderTest* test = &misc_test_http_headers[i];
        furi_string_cat_printf(response_str, "%s: %s\r\n", test->key, test->value);
    }

    furi_string_cat(response_str,"\r\n");

    HttpResponse response;
    mu_check(http_response_parse(&response, furi_string_get_cstr(response_str), furi_string_size(response_str)));
    mu_assert_int_eq(200, response.status);
    mu_assert_mem_eq("OK", response.status_text.first_char, response.status_text.length);

    HttpHeaders* headers = http_headers_alloc();
    mu_check(headers);
    mu_check(http_headers_parse(headers, response.headers.first_char, response.headers.length));
    mu_assert_int_eq(COUNT_OF(misc_test_http_headers), http_headers_get_count(headers));

    for(uint32_t i = 0; i < COUNT_OF(misc_test_http_headers); ++i) {
        const HttpHeaderTest* test = &misc_test_http_headers[i];
        CHECK_HEADER(test->key, test->value);
    }

    for(uint32_t i = 0; i < COUNT_OF(misc_test_http_headers); ++i) {
        const HttpHeaderTest* test = &misc_test_http_headers[i];
        CHECK_HEADER_BY_INDEX(i, test->key, test->value);
    }

    http_headers_free(headers);
    furi_string_free(response_str);
}

MU_TEST(misc_tests_http_headers_min) {
    const char* request = "HTTP/7.2 239 \r\n"
                          "\r\n";

    HttpResponse response;
    mu_check(http_response_parse(&response, request, strlen(request)));
    mu_assert_int_eq(239, response.status);
    mu_assert_int_eq(0, response.status_text.length);

    HttpHeaders* headers = http_headers_alloc();
    mu_check(headers);
    mu_check(http_headers_parse(headers, response.headers.first_char, response.headers.length));
    mu_assert_int_eq(0, http_headers_get_count(headers));

    http_headers_free(headers);
}

MU_TEST_SUITE(misc_test_suite) {
    MU_RUN_TEST(misc_tests_ipv6_format);
    MU_RUN_TEST(misc_tests_ipv4_format);
    MU_RUN_TEST(misc_tests_path_normalize);
    MU_RUN_TEST(misc_tests_http_headers);
    MU_RUN_TEST(misc_tests_http_headers_min);
}

int run_minunit_misc_test(void) {
    MU_RUN_SUITE(misc_test_suite);
    return MU_EXIT_CODE;
}
