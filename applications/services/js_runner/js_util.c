#include "js_runner_i.h"

#define TAG "JsUtil"

void js_log_exception(const char* tag, const char* msg, jerry_value_t exception) {
    FuriString* exception_string = js_get_exception_string(exception);
    FURI_LOG_E(tag, "%s: %s", msg, furi_string_get_cstr(exception_string));
    furi_string_free(exception_string);
}

void js_check_and_free(jerry_value_t val) {
    if(jerry_value_is_exception(val)) {
        js_log_exception(TAG, "check-and-free", val);
    }
    furi_check(!jerry_value_is_exception(val));
    jerry_value_free(val);
}

void js_set_property(jerry_value_t object, const char* name, jerry_value_t property) {
    js_check_and_free(jerry_object_set_sz(object, name, property));
    jerry_value_free(property);
}

void js_set_property_sym(
    jerry_value_t object,
    jerry_well_known_symbol_t key,
    jerry_value_t property) {
    jerry_value_t sym = jerry_symbol(key);
    js_check_and_free(jerry_object_set(object, sym, property));
    jerry_value_free(sym);
    jerry_value_free(property);
}

void js_set_method(jerry_value_t object, const char* name, jerry_external_handler_t handler) {
    jerry_value_t fn = jerry_function_external(handler);
    js_check_and_free(jerry_object_set_sz(object, name, fn));
    jerry_value_free(fn);
}

void js_set_method_sym(
    jerry_value_t object,
    jerry_well_known_symbol_t key,
    jerry_external_handler_t handler) {
    jerry_value_t fn = jerry_function_external(handler);
    jerry_value_t sym = jerry_symbol(key);
    js_check_and_free(jerry_object_set(object, sym, fn));
    jerry_value_free(fn);
    jerry_value_free(sym);
}

void js_set_property_getset(
    jerry_value_t object,
    const char* name,
    jerry_external_handler_t getter,
    jerry_external_handler_t setter) {
    furi_check(getter || setter);

    jerry_value_t name_val = jerry_string_sz(name);
    jerry_property_descriptor_t desc = jerry_property_descriptor();

    if(getter) {
        desc.flags |= JERRY_PROP_IS_GET_DEFINED;
        desc.getter = jerry_function_external(getter);
    }
    if(setter) {
        desc.flags |= JERRY_PROP_IS_SET_DEFINED;
        desc.setter = jerry_function_external(setter);
    }
    js_check_and_free(jerry_object_define_own_prop(object, name_val, &desc));
    jerry_value_free(name_val);
    jerry_property_descriptor_free(&desc);
}

jerry_value_t js_iterator_result(bool done, jerry_value_t value) {
    jerry_value_t obj = jerry_object();
    js_check_and_free(jerry_object_set_sz(obj, "value", value));
    jerry_value_t done_val = jerry_boolean(done);
    js_check_and_free(jerry_object_set_sz(obj, "done", done_val));

    jerry_value_free(value);
    jerry_value_free(done_val);
    return obj;
}

char* js_string_to_c_string(jerry_value_t value) {
    if(!jerry_value_is_string(value)) {
        return NULL;
    }

    size_t length = jerry_string_size(value, JERRY_ENCODING_UTF8);
    char* buffer = malloc(length + 1);
    jerry_string_to_buffer(value, JERRY_ENCODING_UTF8, (jerry_char_t*)buffer, length);
    buffer[length] = 0;
    return buffer;
}

FuriString* js_string_to_furi_string(jerry_value_t value) {
    char* buffer = js_string_to_c_string(value);
    if(!buffer) {
        return NULL;
    }
    FuriString* result = furi_string_alloc_set(buffer);
    free(buffer);
    return result;
}

FuriString* js_get_exception_string(jerry_value_t exception) {
    furi_check(jerry_value_is_exception(exception));
    jerry_value_t val = jerry_exception_value(exception, false);
    jerry_value_t str = jerry_value_to_string(val);
    FuriString* result = js_string_to_furi_string(str);
    furi_assert(result);
    jerry_value_free(str);
    jerry_value_free(val);
    return result;
}

void js_copy_property(jerry_value_t dst, jerry_value_t src, const char* key) {
    if(js_object_has_property(src, key)) {
        jerry_value_t val = jerry_object_get_sz(src, key);
        jerry_value_free(jerry_object_set_sz(dst, key, val));
        jerry_value_free(val);
    }
}

bool js_is_instance_of(jerry_value_t obj, const char* constructor_name) {
    jerry_value_t global_obj = jerry_current_realm();
    jerry_value_t constructor = jerry_object_get_sz(global_obj, constructor_name);
    bool result = false;
    if(!jerry_value_is_exception(constructor)) {
        result = jerry_binary_op(JERRY_BIN_OP_INSTANCEOF, obj, constructor);
    }
    jerry_value_free(constructor);
    jerry_value_free(global_obj);
    return result;
}

bool js_object_has_property(jerry_value_t object, const char* key) {
    jerry_value_t has = jerry_object_has_sz(object, key);
    bool result = jerry_value_is_true(has);
    jerry_value_free(has);
    return result;
}

jerry_value_t js_rejected_promise(const char* msg) {
    jerry_value_t ret = jerry_promise();
    // The following code will print "true" on nodejs and "false" on our implementation
    //
    // fetch().catch((e) => {
    //     console.error(e instanceof TypeError);
    // })
    //
    // Rejecting a promise with an exception is a special case in jerryscript (exception will propagate, the promise won't be rejected)
    //
    // TODO: solve this
    //
    // jerry_value_t msg_val = jerry_throw_sz(JERRY_ERROR_TYPE, msg);
    jerry_value_t msg_val = jerry_string_sz(msg);
    jerry_value_t is_ok = jerry_promise_reject(ret, msg_val);
    jerry_value_free(is_ok);
    jerry_value_free(msg_val);
    return ret;
}

void js_reject_promise_with_exception(jerry_value_t promise, jerry_value_t exception) {
    // TODO investigate and make things right
    // See comment in js_rejected_promise
    jerry_value_t val = jerry_exception_value(exception, false);
    jerry_value_free(jerry_promise_reject(promise, val));
    jerry_value_free(promise);
    jerry_value_free(exception);
    jerry_value_free(val);
}

jerry_value_t js_rejected_promise_from_exception(jerry_value_t exception) {
    // TODO investigate and make things right
    // See comment in js_rejected_promise
    jerry_value_t val = jerry_exception_value(exception, false);
    jerry_value_t ret = jerry_promise();
    jerry_value_free(jerry_promise_reject(ret, val));
    jerry_value_free(val);
    jerry_value_free(exception);
    return ret;
}

jerry_value_t js_utf8_string(const FuriString* s) {
    return jerry_string(
        (const jerry_char_t*)furi_string_get_cstr(s), furi_string_size(s), JERRY_ENCODING_UTF8);
}

bool js_value_to_integer(jerry_value_t value, int* result) {
    jerry_value_t num = jerry_value_to_number(value);
    bool ok = false;
    if(jerry_value_is_number(num)) {
        double d = jerry_value_as_number(num);
        if(isfinite(d) && d < (double)INT_MAX && d > (double)INT_MIN) {
            *result = (int)d;
            ok = true;
        }
    }
    jerry_value_free(num);
    return ok;
}

jerry_value_t js_arraybuffer_from_byte_array(ByteArray_t* array) {
    size_t size = ByteArray_size(*array);
    JsRunnerByteArrayDestructor* destructor = malloc(sizeof(JsRunnerByteArrayDestructor));
    destructor->destructor = js_runner_byte_array_destructor;
    destructor->byte_array = array;
    return jerry_arraybuffer_external(ByteArray_get(*array, 0), size, destructor);
}

jerry_value_t js_arraybuffer_from_sized_buffer(SizedBuffer buffer) {
    return jerry_arraybuffer_external(buffer.buffer, buffer.size, js_runner_heap_destructor);
}
