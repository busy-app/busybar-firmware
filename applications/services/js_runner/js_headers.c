#include "js_headers.h"
#include <toolbox/http_headers.h>

#include <m-array.h>

#include <ctype.h>

#define TAG "JsHeaders"

typedef struct Headers {
    size_t ref_count;

    HttpHeaders* headers;
} Headers;

typedef enum IterMode {
    IterModePairs,
    IterModeKeys,
    IterModeValues,
} IterMode;

typedef struct HeadersIter {
    Headers* parent;
    size_t index;
    jerry_value_t self;
    IterMode mode;
} HeadersIter;

static void headers_free_cb(void* native_p, jerry_object_native_info_t* info_p);
static void headers_iter_free_cb(void* native_p, jerry_object_native_info_t* info_p);

static const jerry_object_native_info_t headers_native_info = {.free_cb = headers_free_cb};
static const jerry_object_native_info_t headers_iter_native_info = {
    .free_cb = headers_iter_free_cb};

jerry_value_t headers_iter_next(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);

    HeadersIter* instance =
        jerry_object_get_native_ptr(call_info->this_value, &headers_iter_native_info);
    JS_CHECK_INSTANCE();
    if(instance->index == http_headers_get_header_count(instance->parent->headers)) {
        return js_iterator_result(true, jerry_undefined());
    } else {
        const HttpHeader* header =
            http_headers_get_header(instance->parent->headers, instance->index);
        instance->index += 1;
        jerry_value_t result = 0; // Suppress "may be uninitialized" warning
        switch(instance->mode) {
        case IterModePairs: {
            result = jerry_array(2);
            jerry_value_t key = jerry_string_sz(furi_string_get_cstr(header->key));
            jerry_value_t value = jerry_string_sz(furi_string_get_cstr(header->value));
            jerry_value_free(jerry_object_set_index(result, 0, key));
            jerry_value_free(jerry_object_set_index(result, 1, value));
            jerry_value_free(key);
            jerry_value_free(value);
            break;
        }
        case IterModeKeys: {
            result = jerry_string_sz(furi_string_get_cstr(header->key));
            break;
        }
        case IterModeValues: {
            result = jerry_string_sz(furi_string_get_cstr(header->value));
            break;
        }
        }
        return js_iterator_result(false, result);
    }
}

jerry_value_t headers_iter_iterator(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);

    HeadersIter* instance =
        jerry_object_get_native_ptr(call_info->this_value, &headers_iter_native_info);
    JS_CHECK_INSTANCE();

    return jerry_value_copy(instance->self);
}

jerry_value_t headers_iterator_method(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count,
    IterMode mode) {
    UNUSED(args);
    UNUSED(args_count);

    Headers* instance = jerry_object_get_native_ptr(call_info->this_value, &headers_native_info);
    JS_CHECK_INSTANCE();
    HeadersIter* headers_iter = malloc(sizeof(HeadersIter));
    headers_iter->parent = instance;
    headers_iter->index = 0;
    headers_iter->parent->ref_count += 1;
    headers_iter->mode = mode;
    jerry_value_t iter = jerry_object();
    jerry_object_set_native_ptr(iter, &headers_iter_native_info, headers_iter);

    js_set_method(iter, "next", headers_iter_next);
    js_set_method_sym(iter, JERRY_SYMBOL_ITERATOR, headers_iter_iterator);

    headers_iter->self = iter;

    return iter;
}

jerry_value_t headers_entries(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    return headers_iterator_method(call_info, args, args_count, IterModePairs);
}

jerry_value_t headers_keys(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    return headers_iterator_method(call_info, args, args_count, IterModeKeys);
}

jerry_value_t headers_values(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    return headers_iterator_method(call_info, args, args_count, IterModeValues);
}

jerry_value_t headers_has(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    Headers* instance = jerry_object_get_native_ptr(call_info->this_value, &headers_native_info);
    JS_CHECK_INSTANCE();

    if(args_count == 0) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Too few arguments for has");
    }

    jerry_value_t name = args[0];
    if(!jerry_value_is_string(name)) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Not a string");
    }
    char* name_buf = js_string_to_c_string(name);

    bool found = false;
    for(size_t i = 0; i != http_headers_get_header_count(instance->headers); ++i) {
        const HttpHeader* header = http_headers_get_header(instance->headers, i);
        if(furi_string_cmpi(header->key, name_buf) == 0) {
            found = true;
            break;
        }
    }

    free(name_buf);
    return jerry_boolean(found);
}

jerry_value_t headers_foreach(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    Headers* instance = jerry_object_get_native_ptr(call_info->this_value, &headers_native_info);
    JS_CHECK_INSTANCE();

    if(args_count == 0) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Too few arguments for forEach");
    }

    jerry_value_t callback = args[0];
    if(!jerry_value_is_function(callback)) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Argument is not a function");
    }
    jerry_value_t this_value = args_count == 1 ? jerry_undefined() : jerry_value_copy(args[1]);

    jerry_value_t result = jerry_undefined();

    for(size_t i = 0; i != http_headers_get_header_count(instance->headers); ++i) {
        const HttpHeader* header = http_headers_get_header(instance->headers, i);

        jerry_value_t args[3] = {
            [0] = jerry_string_sz(furi_string_get_cstr(header->value)),
            [1] = jerry_string_sz(furi_string_get_cstr(header->key)),
            [2] = call_info->this_value,
        };
        jerry_value_t call_result = jerry_call(callback, this_value, args, 3);
        jerry_value_free(args[0]);
        jerry_value_free(args[1]);
        if(jerry_value_is_exception(call_result)) {
            jerry_value_free(result);
            result = call_result;
            break;
        } else {
            jerry_value_free(call_result);
        }
    }

    jerry_value_free(this_value);

    return result;
}

jerry_value_t js_headers_alloc(jerry_value_t response, const char* data, size_t data_size) {
    Headers* instance = malloc(sizeof(Headers));
    instance->ref_count = 1;
    instance->headers = http_headers_alloc();
    if(!http_headers_parse(instance->headers, data, data_size)) {
        http_headers_free(instance->headers);
        free(instance);
        return jerry_throw_sz(JERRY_ERROR_COMMON, "Error parsing headers");
    }

    uint32_t status = http_headers_get_status(instance->headers);
    js_set_property(response, "status", jerry_number((double)status));
    js_set_property(
        response, "statusText", jerry_string_sz(http_headers_get_status_text(instance->headers)));
    js_set_property(response, "ok", jerry_boolean(status / 100 == 2));

    jerry_value_t obj = jerry_object();
    jerry_object_set_native_ptr(obj, &headers_native_info, instance);

    for(size_t i = 0; i != http_headers_get_header_count(instance->headers); ++i) {
        const HttpHeader* header = http_headers_get_header(instance->headers, i);
        jerry_value_t key = jerry_string_sz(furi_string_get_cstr(header->key));
        jerry_value_t value = jerry_string_sz(furi_string_get_cstr(header->value));
        jerry_value_free(jerry_object_set(obj, key, value));
        jerry_value_free(key);
        jerry_value_free(value);
    }

    js_set_method(obj, "entries", headers_entries);
    js_set_method(obj, "keys", headers_keys);
    js_set_method(obj, "values", headers_values);
    js_set_method(obj, "forEach", headers_foreach);
    js_set_method(obj, "has", headers_has);

    return obj;
}

static void headers_free_cb(void* native_p, jerry_object_native_info_t* info_p) {
    UNUSED(info_p);
    Headers* instance = native_p;
    instance->ref_count -= 1;
    if(instance->ref_count == 0) {
        http_headers_free(instance->headers);
        free(instance);
    }
}

static void headers_iter_free_cb(void* native_p, jerry_object_native_info_t* info_p) {
    UNUSED(info_p);
    HeadersIter* instance = native_p;
    headers_free_cb(instance->parent, NULL);
    free(instance);
}
