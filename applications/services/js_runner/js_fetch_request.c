#include "js_fetch_request.h"

#include <core/check.h>

#include <ctype.h>
#include <string.h>

#include <toolbox/url.h>
#include <http/http_method.h>

#include "js_util.h"
#include "js_fetch_ext_header.h"

#define TAG "JsFetchRequest"

#define JS_FETCH_HEADER_COUNT_MAX (FETCH_HEADERS_COUNT_MAX - 1)

#define JS_FETCH_HEADER_KEY_TOKEN_CHARS "!#$%&'*+-.^_`|~"

typedef jerry_value_t (*ParseRequestStep)(jerry_value_t obj, FetchRequest* request);

static void
    fetch_request_headers_append(FetchRequestHeaders* headers, const char* key, const char* value) {
    const uint32_t headers_count = headers->count;
    furi_check(headers_count < FETCH_HEADERS_COUNT_MAX);

    const size_t buf_size = strlen(key) + strlen(value) + sizeof(": ");

    char* header = malloc(buf_size);
    sprintf(header, "%s: %s", key, value);

    headers->data[headers_count] = header;
    headers->count = headers_count + 1;
}

static void fetch_request_headers_append_extra(FetchRequestHeaders* headers) {
    fetch_request_headers_append(headers, JS_FETCH_EXTRA_HEADER, JS_FETCH_EXTRA_VALUE);
}

static jerry_value_t parse_request_throw_error(const char* message) {
    return jerry_throw_value(jerry_string_sz(message), true);
}

static inline bool parse_request_extra_header_present(const char* input) {
    return strcasestr(input, JS_FETCH_EXTRA_HEADER) != NULL;
}

static bool parse_request_header_key_is_valid(const char* key) {
    bool is_valid = true;

    do {
        const size_t key_len = strlen(key);
        if(key_len == 0) {
            is_valid = false;
            break;
        }

        for(size_t i = 0; i < key_len; ++i) {
            const unsigned char c = key[i];
            if((c > 0x7F) ||
               (!isalnum(c) && (strchr(JS_FETCH_HEADER_KEY_TOKEN_CHARS, c) == NULL))) {
                is_valid = false;
                break;
            }
        }
    } while(false);

    return is_valid;
}

static inline bool is_whitespace(unsigned char c) {
    return (c <= 0x7F) && isspace(c);
}

static bool parse_request_header_value_is_valid(const char* value) {
    bool is_valid = true;

    do {
        const size_t value_len = strlen(value);
        if(value_len == 0) {
            is_valid = false;
            break;
        }

        if(is_whitespace(value[0])) {
            is_valid = false;
            break;
        }

        if(is_whitespace(value[value_len - 1])) {
            is_valid = false;
            break;
        }

        for(size_t i = 0; i < value_len; ++i) {
            const unsigned char c = value[i];
            if((c > 0x7F) || !isprint(c)) {
                is_valid = false;
                break;
            }
        }
    } while(false);

    return is_valid;
}

static jerry_value_t parse_request_url(jerry_value_t obj, FetchRequest* request) {
    jerry_value_t result;
    Url* parser = url_alloc();

    do {
        const jerry_value_t url_val = jerry_object_get_sz(obj, "url");
        if(jerry_value_is_exception(url_val)) {
            result = url_val;
            break;
        }

        char* source_str = js_string_to_c_string(url_val);
        jerry_value_free(url_val);

        if(source_str == NULL) {
            result = parse_request_throw_error("URL is not a string");
            break;
        }

        const bool can_parse = url_parse(parser, source_str);
        free(source_str);

        if(!can_parse) {
            result = parse_request_throw_error("Invalid URL");
            break;
        }

        const StringSlice* part = url_get_part(parser, UrlPartIdRequest);
        request->url = string_slice_to_owned_cstr(part);

        result = jerry_undefined();
    } while(false);

    url_free(parser);
    return result;
}

static jerry_value_t parse_request_method(jerry_value_t obj, FetchRequest* request) {
    jerry_value_t result;

    do {
        if(!js_object_has_property(obj, "method")) {
            result = jerry_undefined();
            break;
        }

        const jerry_value_t method_val = jerry_object_get_sz(obj, "method");
        if(jerry_value_is_exception(method_val)) {
            result = method_val;
            break;
        }

        char* method_str = js_string_to_c_string(method_val);
        jerry_value_free(method_val);

        if(method_str == NULL) {
            result = parse_request_throw_error("Method is not a string");
            break;
        }

        if(!http_method_is_valid(method_str)) {
            result = parse_request_throw_error("Invalid method");
            free(method_str);
            break;
        }

        http_method_normalize(method_str);
        request->method = method_str;

        result = jerry_undefined();
    } while(false);

    return result;
}

static jerry_value_t parse_request_dispatcher(jerry_value_t obj, FetchRequest* request) {
    jerry_value_t result;

    do {
        static const char* const keys[] = {"dispatcher", "connect", "useDeviceKey"};
        const jerry_value_t prop_val = js_object_get_nested_property(obj, keys, COUNT_OF(keys));

        if(jerry_value_is_exception(prop_val)) {
            result = prop_val;
            break;
        }

        const bool is_device_cert = jerry_value_to_boolean(prop_val);
        jerry_value_free(prop_val);

        TlsClientCertInfo* cert_info = &request->tls_config.client_cert_info;
        cert_info->type = is_device_cert ? TlsClientCertTypeDevice : TlsClientCertTypeNone;

        result = jerry_undefined();
    } while(false);

    return result;
}

static bool parse_request_header(
    FetchRequestHeaders* headers,
    jerry_value_t values,
    jerry_value_t keys,
    size_t index,
    bool* has_extra_header) {
    bool success = false;

    const jerry_value_t key = jerry_object_get_index(keys, index);
    const jerry_value_t value = jerry_object_get(values, key);
    const jerry_value_t value_conv = jerry_value_to_string(value);

    char* key_string = NULL;
    char* value_string = NULL;

    do {
        key_string = js_string_to_c_string(key);
        value_string = js_string_to_c_string(value_conv);

        if((key_string == NULL) || (value_string == NULL)) {
            break;
        }

        if(!parse_request_header_key_is_valid(key_string)) {
            break;
        }

        if(!parse_request_header_value_is_valid(value_string)) {
            break;
        }

        if(parse_request_extra_header_present(key_string)) {
            if(!(*has_extra_header)) {
                FURI_LOG_W(TAG, "%s header override", JS_FETCH_EXTRA_HEADER);
                fetch_request_headers_append_extra(headers);
                *has_extra_header = true;
            } else {
                FURI_LOG_W(TAG, "%s header duplication", JS_FETCH_EXTRA_HEADER);
            }

        } else {
            fetch_request_headers_append(headers, key_string, value_string);
        }

        success = true;
    } while(false);

    if(key_string != NULL) {
        free(key_string);
    }
    if(value_string != NULL) {
        free(value_string);
    }

    jerry_value_free(key);
    jerry_value_free(value);
    jerry_value_free(value_conv);

    return success;
}

static jerry_value_t parse_request_headers(jerry_value_t obj, FetchRequest* request) {
    jerry_value_t result;

    do {
        FetchRequestHeaders* headers = &request->headers;

        if(!js_object_has_property(obj, "headers")) {
            fetch_request_headers_append_extra(headers);
            result = jerry_undefined();
            break;
        }

        const jerry_value_t headers_val = jerry_object_get_sz(obj, "headers");
        if(jerry_value_is_exception(headers_val)) {
            result = headers_val;
            break;
        }

        // TODO: Instance of Headers
        if(!jerry_value_is_object(headers_val)) {
            jerry_value_free(headers_val);
            result = parse_request_throw_error("Headers is not an object");
            break;
        }

        bool is_error = false;
        bool has_extra_header = false;

        const jerry_value_t keys = jerry_object_keys(headers_val);
        const size_t num_keys = jerry_array_length(keys);

        for(size_t i = 0; (i < num_keys) && (headers->count < JS_FETCH_HEADER_COUNT_MAX); ++i) {
            if(!parse_request_header(headers, headers_val, keys, i, &has_extra_header)) {
                is_error = true;
                break;
            }
        }

        jerry_value_free(headers_val);
        jerry_value_free(keys);

        if(is_error) {
            result = parse_request_throw_error("Invalid header");
            break;
        }

        if(!has_extra_header) {
            fetch_request_headers_append_extra(headers);
        }

        result = jerry_undefined();
    } while(false);

    return result;
}

static jerry_value_t parse_request_body(jerry_value_t obj, FetchRequest* request) {
    jerry_value_t result;

    do {
        if(!js_object_has_property(obj, "body")) {
            result = jerry_undefined();
            break;
        }

        const jerry_value_t body_val = jerry_object_get_sz(obj, "body");
        const jerry_value_t body_val_str = jerry_value_to_string(body_val);
        jerry_value_free(body_val);

        if(jerry_value_is_exception(body_val_str)) {
            result = body_val_str;
            break;
        }

        const size_t body_len = jerry_string_size(body_val_str, JERRY_ENCODING_UTF8);
        unsigned char* body_data = malloc(body_len + 1);

        jerry_string_to_buffer(body_val_str, JERRY_ENCODING_UTF8, body_data, body_len);
        jerry_value_free(body_val_str);

        body_data[body_len] = '\0';

        FetchRequestBody* request_body = &request->body;
        request_body->data = body_data;
        request_body->length = body_len;

        result = jerry_undefined();
    } while(false);

    return result;
}

static const ParseRequestStep parse_request_steps[] = {
    parse_request_url,
    parse_request_method,
    parse_request_dispatcher,
    parse_request_headers,
    parse_request_body,
};

jerry_value_t js_fetch_request_parse(jerry_value_t obj, FetchRequest* request) {
    furi_assert(js_is_instance_of(obj, "Request"));
    jerry_value_t result = jerry_undefined();

    for(size_t i = 0; i < COUNT_OF(parse_request_steps); ++i) {
        result = parse_request_steps[i](obj, request);
        if(jerry_value_is_exception(result)) {
            break;
        }

        furi_check(jerry_value_is_undefined(result));
    }

    if(jerry_value_is_exception(result)) {
        js_fetch_request_clear(request);
    }

    return result;
}

void js_fetch_request_clear(FetchRequest* request) {
    if(request->url) {
        free((void*)request->url);
    }
    if(request->method) {
        free((void*)request->method);
    }
    for(size_t i = 0; i != request->headers.count; ++i) {
        free((void*)request->headers.data[i]);
    }
    if(request->body.data) {
        free((void*)request->body.data);
    }
}
