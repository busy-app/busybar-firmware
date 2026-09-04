#include "js_headers.h"
#include <http/http_headers.h>

#define TAG "JsHeaders"

typedef struct HeadersNative {
    HttpHeaders* headers;
    size_t ref_count;
} HeadersNative;

typedef enum IterMode {
    IterModePairs,
    IterModeKeys,
    IterModeValues,
} IterMode;

typedef struct HeadersIter {
    HeadersNative* parent;
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

    HeadersNative* instance =
        jerry_object_get_native_ptr(call_info->this_value, &headers_native_info);
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
    HeadersNative* instance =
        jerry_object_get_native_ptr(call_info->this_value, &headers_native_info);
    JS_CHECK_INSTANCE();

    JS_CHECK_ARGS_COUNT(1);
    JS_CHECK_ARG_IS_STRING(JS_ARG(0));

    char* key = js_string_to_c_string(JS_ARG(0));
    furi_assert(key);

    const bool has_key = (http_headers_get(instance->headers, key) != NULL);

    free(key);
    return jerry_boolean(has_key);
}

jerry_value_t headers_foreach(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    HeadersNative* instance =
        jerry_object_get_native_ptr(call_info->this_value, &headers_native_info);
    JS_CHECK_INSTANCE();

    JS_CHECK_ARGS_COUNT(1);

    jerry_value_t callback = JS_ARG(0);
    JS_CHECK_ARG_IS_FUNCTION(callback);

    jerry_value_t this_value = args_count == 1 ? jerry_undefined() : jerry_value_copy(JS_ARG(1));
    jerry_value_t result = jerry_undefined();

    for(size_t i = 0; i != http_headers_get_header_count(instance->headers); ++i) {
        const HttpHeader* header = http_headers_get_header(instance->headers, i);

        jerry_value_t call_args[3] = {
            [0] = jerry_string_sz(furi_string_get_cstr(header->value)),
            [1] = jerry_string_sz(furi_string_get_cstr(header->key)),
            [2] = call_info->this_value,
        };

        jerry_value_t call_result =
            jerry_call(callback, this_value, call_args, COUNT_OF(call_args));

        jerry_value_free(call_args[0]);
        jerry_value_free(call_args[1]);

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

static jerry_value_t headers_get(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    HeadersNative* instance =
        jerry_object_get_native_ptr(call_info->this_value, &headers_native_info);
    JS_CHECK_INSTANCE();

    JS_CHECK_ARGS_COUNT(1);
    JS_CHECK_ARG_IS_STRING(JS_ARG(0));

    char* key = js_string_to_c_string(JS_ARG(0));
    furi_assert(key);

    jerry_value_t result;

    const HttpHeader* item = http_headers_get(instance->headers, key);
    if(item != NULL) {
        result = jerry_string_sz(furi_string_get_cstr(item->value));
    } else {
        result = jerry_null();
    }

    free(key);
    return result;
}

static jerry_value_t headers_set(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    HeadersNative* instance =
        jerry_object_get_native_ptr(call_info->this_value, &headers_native_info);
    JS_CHECK_INSTANCE();

    JS_CHECK_ARGS_COUNT(2);
    JS_CHECK_ARG_IS_STRING(JS_ARG(0));
    JS_CHECK_ARG_IS_STRING(JS_ARG(1));

    char* key = js_string_to_c_string(JS_ARG(0));
    furi_assert(key);
    char* value = js_string_to_c_string(JS_ARG(1));
    furi_assert(value);

    http_headers_set(instance->headers, key, value);

    free(key);
    free(value);

    return jerry_undefined();
}

static jerry_value_t headers_constructor(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);

    jerry_value_t obj = call_info->this_value;

    HeadersNative* headers_native = malloc(sizeof(HeadersNative));
    headers_native->headers = http_headers_alloc();
    headers_native->ref_count = 1;

    jerry_object_set_native_ptr(obj, &headers_native_info, headers_native);

    js_set_method(obj, "entries", headers_entries);
    js_set_method(obj, "keys", headers_keys);
    js_set_method(obj, "values", headers_values);
    js_set_method(obj, "forEach", headers_foreach);
    js_set_method(obj, "has", headers_has);
    js_set_method(obj, "get", headers_get);
    js_set_method(obj, "set", headers_set);

    return jerry_undefined();
}

static jerry_value_t js_headers_construct(void) {
    jerry_value_t global_obj = jerry_current_realm();

    jerry_value_t constructor = jerry_object_get_sz(global_obj, "Headers");
    furi_check(jerry_value_is_function(constructor));

    jerry_value_t this_value = jerry_construct(constructor, NULL, 0);

    jerry_value_free(constructor);
    jerry_value_free(global_obj);

    return this_value;
}

void js_setup_headers(void) {
    jerry_value_t global_obj = jerry_current_realm();

    jerry_value_t constructor = jerry_function_external(headers_constructor);
    jerry_value_free(jerry_object_set_sz(global_obj, "Headers", constructor));

    jerry_value_t prototype = jerry_object();
    js_check_and_free(jerry_object_set_proto(constructor, prototype));
    jerry_value_free(prototype);
    jerry_value_free(constructor);
    jerry_value_free(global_obj);
}

jerry_value_t js_headers_alloc(StringSlice headers_text) {
    jerry_value_t obj = js_headers_construct();

    HeadersNative* instance = jerry_object_get_native_ptr(obj, &headers_native_info);
    furi_assert(instance);

    if(!http_headers_parse(instance->headers, headers_text.first_char, headers_text.length)) {
        jerry_value_free(obj);
        obj = jerry_throw_sz(JERRY_ERROR_COMMON, "Error parsing headers");
    }

    return obj;
}

static void headers_free_cb(void* native_p, jerry_object_native_info_t* info_p) {
    UNUSED(info_p);
    HeadersNative* instance = native_p;
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
