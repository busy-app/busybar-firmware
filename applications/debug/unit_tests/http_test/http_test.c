#include "../unit_tests.h"

#include <http/http_response.h>
#include <http/http_headers.h>

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

MU_TEST(http_headers_parse_test) {
    FuriString* response_str = furi_string_alloc_set("HTTP/1.1 200 OK\r\n");

    for(uint32_t i = 0; i < COUNT_OF(misc_test_http_headers); ++i) {
        const HttpHeaderTest* test = &misc_test_http_headers[i];
        furi_string_cat_printf(response_str, "%s: %s\r\n", test->key, test->value);
    }

    furi_string_cat(response_str, "\r\n");

    HttpResponse response;
    mu_check(http_response_parse(
        &response, furi_string_get_cstr(response_str), furi_string_size(response_str)));
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

MU_TEST(http_headers_min_parse_test) {
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

MU_TEST(http_headers_set_test) {
    HttpHeaders* headers = http_headers_alloc();

    for(uint32_t i = 0; i < COUNT_OF(misc_test_http_headers); ++i) {
        const HttpHeaderTest* test = &misc_test_http_headers[i];
        http_headers_set(headers, test->key, test->value);
    }

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
}

MU_TEST_SUITE(http_test_suite) {
    MU_RUN_TEST(http_headers_parse_test);
    MU_RUN_TEST(http_headers_min_parse_test);
    MU_RUN_TEST(http_headers_set_test);
}

int run_minunit_http_test(void) {
    MU_RUN_SUITE(http_test_suite);
    return MU_EXIT_CODE;
}
