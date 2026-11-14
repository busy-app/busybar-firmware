#include "../unit_tests.h"

#include <http/http_response.h>
#include <http/http_headers.h>
#include <http/http_method.h>

#include <fetch/fetch_chunked.h>

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

#define HTTP_METHOD_BUF_SIZE (16U)

typedef struct {
    const char* key;
    const char* value;
} HttpHeaderTest;

typedef struct {
    const char* input;
    const char* expected;
} HttpMethodNormalizeTest;

static const HttpHeaderTest misc_test_http_headers[] = {
    [0] = {"Server", "nginx/1.18.0"},
    [1] = {"Date", "Tue, 28 Jul 2026 13:17:26 GMT"},
    [2] = {"Content-Type", "text/html"},
    [3] = {"Content-Length", "4592"},
    [4] = {"Last-Modified", "Tue, 09 Apr 2024 06:23:39 GMT"},
    [5] = {"Connection", "close"},
    [6] = {"Accept-Ranges", "bytes"},
};

static const char* const http_test_valid_methods[] = {
    "GET",
    "post",
    "Put",
    "PATCH",
    "M",
    "Method2",
    "0",
};

static const char* const http_test_invalid_methods[] = {
    "",
    " GET",
    "GET ",
    "GE T",
    "GET\r\n",
    "\tPOST",
    "G\xC3\x89T",
    "\x80",
};

static const HttpMethodNormalizeTest http_test_normalize_methods[] = {
    {"GET", "GET"},
    {"get", "GET"},
    {"pOsT", "POST"},
    {"put", "PUT"},
    {"Delete", "DELETE"},
    {"head", "HEAD"},
    {"options", "OPTIONS"},
    {"patch", "patch"},
    {"Connect", "Connect"},
    {"getx", "getx"},
    {"ge", "ge"},
    {"", ""},
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

MU_TEST(http_headers_empty_value_parse_test) {
    // Home Assistant (aiohttp) sends "Server: " with an empty value
    const char* request = "HTTP/1.1 401 Unauthorized\r\n"
                          "Content-Type: text/plain; charset=utf-8\r\n"
                          "Server: \r\n"
                          "X-Empty:\r\n"
                          "Content-Length: 17\r\n"
                          "\r\n";
    HttpResponse response;
    mu_check(http_response_parse(&response, request, strlen(request)));
    mu_assert_int_eq(401, response.status);

    HttpHeaders* headers = http_headers_alloc();
    mu_check(headers);
    mu_check(http_headers_parse(headers, response.headers.first_char, response.headers.length));
    mu_assert_int_eq(4, http_headers_get_count(headers));

    CHECK_HEADER_BY_INDEX(0, "Content-Type", "text/plain; charset=utf-8");
    CHECK_HEADER_BY_INDEX(1, "Server", "");
    CHECK_HEADER_BY_INDEX(2, "X-Empty", "");
    CHECK_HEADER_BY_INDEX(3, "Content-Length", "17");

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

MU_TEST(http_method_valid_test) {
    for(uint32_t i = 0; i < COUNT_OF(http_test_valid_methods); ++i) {
        mu_check(http_method_is_valid(http_test_valid_methods[i]));
    }
}

MU_TEST(http_method_invalid_test) {
    for(uint32_t i = 0; i < COUNT_OF(http_test_invalid_methods); ++i) {
        mu_check(!http_method_is_valid(http_test_invalid_methods[i]));
    }
}

MU_TEST(http_method_normalize_test) {
    char method[HTTP_METHOD_BUF_SIZE];

    for(uint32_t i = 0; i < COUNT_OF(http_test_normalize_methods); ++i) {
        const HttpMethodNormalizeTest* test = &http_test_normalize_methods[i];
        strlcpy(method, test->input, sizeof(method));
        http_method_normalize(method);
        mu_assert_string_eq(test->expected, method);
    }
}

typedef struct ChunkedReceiver {
    char data[64];
    size_t data_length;
    bool is_error;
} ChunkedReceiver;

static void chunked_on_data(SizedBufferConst data, void* context) {
    ChunkedReceiver* instance = context;

    if(instance->data_length + data.size > sizeof(instance->data)) {
        printf("Data overflow\n");
        instance->is_error = true;
    }

    memcpy(instance->data + instance->data_length, data.buffer, data.size);
    instance->data_length += data.size;
}

static void chunked_on_error(void* context) {
    ChunkedReceiver* instance = context;
    instance->is_error = true;
}

typedef struct ChunkedPacket {
    const char* data;
    bool is_done;
    bool is_error;
} ChunkedPacket;

static void test_chunked(const ChunkedPacket* packets, size_t packets_count, const char* result) {
    ChunkedReceiver receiver = {0};
    FetchChunked* chunked = fetch_chunked_alloc(chunked_on_data, chunked_on_error, &receiver);

    for(size_t i = 0; i != packets_count; ++i) {
        bool done = fetch_chunked_process(
            chunked,
            (SizedBufferConst){.buffer = packets[i].data, .size = strlen(packets[i].data)});

        mu_assert_int_eq(packets[i].is_done, done);
        mu_assert_int_eq(packets[i].is_error, receiver.is_error);
    }

    if(result) {
        size_t result_len = strlen(result);
        mu_assert_int_eq(result_len, receiver.data_length);
        mu_assert_mem_eq(result, receiver.data, result_len);
        mu_check(fetch_chunked_is_complete(chunked));
    }
    fetch_chunked_free(chunked);
}

MU_TEST(http_chunked_test_simple) {
    ChunkedPacket packets[] = {{"5\r\nhello\r\n0\r\n\r\n", true, false}};
    test_chunked(packets, COUNT_OF(packets), "hello");
}

MU_TEST(http_chunked_test_broken_data) {
    ChunkedPacket packets[] = {{"5\r\nhel", false, false}, {"lo\r\n0\r\n\r\n", true, false}};
    test_chunked(packets, COUNT_OF(packets), "hello");
}

MU_TEST(http_chunked_test_two_chunks) {
    ChunkedPacket packets[] = {
        {"5\r\nhel", false, false},
        {"lo\r\n6\r\n ", false, false},
        {"world\r\n0\r", false, false},
        {"\n\r\n", true, false},
    };
    test_chunked(packets, COUNT_OF(packets), "hello world");
}

MU_TEST(http_chunked_test_overflow) {
    ChunkedPacket packets[] = {
        {"5\r\nhello\r\n", false, false},
        {"123456789abcdef012345\r\nhello\r\n", true, true},
        {"0\r\n\r\n", true, true}};
    test_chunked(packets, COUNT_OF(packets), NULL);
}

MU_TEST(http_chunked_test_no_size) {
    ChunkedPacket packets[] = {
        {"hello\r\n", true, true},
    };
    test_chunked(packets, COUNT_OF(packets), NULL);
}

MU_TEST(http_chunked_test_double_digit) {
    ChunkedPacket packets[] = {
        {"1b\r\nhello from chunked transfer\r\n0\r\n\r\n", true, false},
    };
    test_chunked(packets, COUNT_OF(packets), "hello from chunked transfer");
}

MU_TEST(http_chunked_test_ext) {
    ChunkedPacket packets[] = {
        {"5; flipper=cool\r\nhello\r\n", false, false},
        {"0\r\n\r\n", true, false},
    };
    test_chunked(packets, COUNT_OF(packets), "hello");
}

MU_TEST(http_chunked_test_incomplete) {
    ChunkedReceiver receiver = {0};
    FetchChunked* chunked = fetch_chunked_alloc(chunked_on_data, chunked_on_error, &receiver);

    bool done = fetch_chunked_process(chunked, (SizedBufferConst){"5\r\nhel", 6});
    mu_check(!done);
    mu_check(!fetch_chunked_is_complete(chunked));
    fetch_chunked_free(chunked);
}

MU_TEST_SUITE(http_test_suite) {
    MU_RUN_TEST(http_headers_parse_test);
    MU_RUN_TEST(http_headers_min_parse_test);
    MU_RUN_TEST(http_headers_empty_value_parse_test);
    MU_RUN_TEST(http_headers_set_test);
    MU_RUN_TEST(http_method_valid_test);
    MU_RUN_TEST(http_method_invalid_test);
    MU_RUN_TEST(http_method_normalize_test);

    MU_RUN_TEST(http_chunked_test_simple);
    MU_RUN_TEST(http_chunked_test_broken_data);
    MU_RUN_TEST(http_chunked_test_two_chunks);
    MU_RUN_TEST(http_chunked_test_overflow);
    MU_RUN_TEST(http_chunked_test_no_size);
    MU_RUN_TEST(http_chunked_test_double_digit);
    MU_RUN_TEST(http_chunked_test_ext);
    MU_RUN_TEST(http_chunked_test_incomplete);
}

int run_minunit_http_test(void) {
    MU_RUN_SUITE(http_test_suite);
    return MU_EXIT_CODE;
}
