#include "js_fetch_body_methods.h"
#include "js_runner_i.h"

#define TAG "JsFetchBodyMethods"

typedef struct BodyMethod BodyMethod;
typedef bool (*BodyCollectedCallback)(BodyMethod* instance);

typedef struct BodyMethod {
    JsFetch* parent;
    jerry_value_t promise;
    ByteArray_t* body;

    BodyCollectedCallback on_body_collected;
} BodyMethod;

static bool data_sink_callback(JsFetch* fetch, JsFetchDataEvent* event, void* callback_context);
static jerry_value_t run_js_method(JsFetch* parent, BodyCollectedCallback on_body_collected);

static bool array_buffer_body_collected(BodyMethod* instance);
static bool blob_body_collected(BodyMethod* instance);
static bool bytes_body_collected(BodyMethod* instance);
static bool json_body_collected(BodyMethod* instance);
static bool form_data_body_collected(BodyMethod* instance);
static bool text_body_collected(BodyMethod* instance);

jerry_value_t js_fetch_array_buffer(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    JsFetch* instance =
        jerry_object_get_native_ptr(call_info->this_value, &js_fetch_response_native_info);
    JS_CHECK_INSTANCE();
    return run_js_method(instance, array_buffer_body_collected);
}

jerry_value_t js_fetch_blob(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    JsFetch* instance =
        jerry_object_get_native_ptr(call_info->this_value, &js_fetch_response_native_info);
    JS_CHECK_INSTANCE();
    return run_js_method(instance, blob_body_collected);
}

jerry_value_t js_fetch_bytes(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    JsFetch* instance =
        jerry_object_get_native_ptr(call_info->this_value, &js_fetch_response_native_info);
    JS_CHECK_INSTANCE();
    return run_js_method(instance, bytes_body_collected);
}

jerry_value_t js_fetch_form_data(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    JsFetch* instance =
        jerry_object_get_native_ptr(call_info->this_value, &js_fetch_response_native_info);
    JS_CHECK_INSTANCE();
    return run_js_method(instance, form_data_body_collected);
}

jerry_value_t js_fetch_json(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    JsFetch* instance =
        jerry_object_get_native_ptr(call_info->this_value, &js_fetch_response_native_info);
    JS_CHECK_INSTANCE();
    return run_js_method(instance, json_body_collected);
}

jerry_value_t js_fetch_text(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    JsFetch* instance =
        jerry_object_get_native_ptr(call_info->this_value, &js_fetch_response_native_info);
    JS_CHECK_INSTANCE();
    return run_js_method(instance, text_body_collected);
}

jerry_object_native_info_t promise_native_info = {0};

static jerry_value_t run_js_method(JsFetch* parent, BodyCollectedCallback on_body_collected) {
    BodyMethod* instance = malloc(sizeof(BodyMethod));
    instance->parent = parent;
    instance->promise = jerry_promise();
    instance->body = malloc(sizeof(*instance->body));
    ByteArray_init(*instance->body);
    instance->on_body_collected = on_body_collected;
    if(!js_fetch_set_data_sink(parent, data_sink_callback, instance)) {
        jerry_value_free(instance->promise);
        ByteArray_clear(*instance->body);
        free(instance->body);
        free(instance);
        return js_rejected_promise("Body is already in use");
    } else {
        jerry_object_set_native_ptr(instance->promise, &promise_native_info, instance);
        js_fetch_data_sink_ready(parent);
        return jerry_value_copy(instance->promise);
    }
}

static void process_data(BodyMethod* instance, SizedBuffer data) {
    JS_TRACE("process_data (size=%zu)", data.size);
    size_t old_size = ByteArray_size(*instance->body);
    ByteArray_resize(*instance->body, old_size + data.size);
    memcpy(ByteArray_get(*instance->body, old_size), data.buffer, data.size);
    free(data.buffer);
}

static void process_done(BodyMethod* instance) {
    JS_TRACE("process_done");
    js_fetch_set_data_sink(instance->parent, NULL, NULL);
    bool success = instance->on_body_collected(instance);
    if(!success) {
        // otherwise buffer ownership is transferred
        ByteArray_clear(*instance->body);
        free(instance->body);
    }
    free(instance);
}

static void process_error(BodyMethod* instance, FuriString* msg) {
    js_fetch_set_data_sink(instance->parent, NULL, NULL);
    ByteArray_clear(*instance->body);
    free(instance->body);
    // jerry_value_t exception = jerry_throw_sz(JERRY_ERROR_TYPE, furi_string_get_cstr(msg));
    jerry_value_t exception = jerry_string_sz(furi_string_get_cstr(msg));
    jerry_value_free(jerry_promise_reject(instance->promise, exception));
    jerry_value_free(instance->promise);
    jerry_value_free(exception);
    furi_string_free(msg);
    free(instance);
    js_run_jobs();
}

static bool data_sink_callback(JsFetch* fetch, JsFetchDataEvent* event, void* callback_context) {
    UNUSED(fetch);
    BodyMethod* instance = callback_context;
    switch(event->type) {
    case JsFetchDataEventTypeData:
        process_data(instance, event->data);
        break;
    case JsFetchDataEventTypeDone:
        process_done(instance);
        break;
    case JsFetchDataEventTypeError:
        process_error(instance, event->error);
        break;
    case JsFetchDataEventTypeInvalid:
        furi_check(false);
        break;
    }
    return true;
}

static bool array_buffer_body_collected(BodyMethod* instance) {
    size_t body_size = ByteArray_size(*instance->body);
    jerry_value_t array_buffer;
    if(body_size > 0) {
        array_buffer = jerry_arraybuffer_external(
            ByteArray_get(*instance->body, 0), body_size, instance->body);
    } else {
        array_buffer = jerry_arraybuffer(0);
    }
    js_check_and_free(jerry_promise_resolve(instance->promise, array_buffer));
    jerry_value_free(instance->promise);
    jerry_value_free(array_buffer);
    js_run_jobs();
    return true;
}

static bool blob_body_collected(BodyMethod* instance) {
    jerry_value_t exception = jerry_string_sz("unimplemented");
    jerry_value_free(jerry_promise_reject(instance->promise, exception));
    jerry_value_free(instance->promise);
    js_run_jobs();
    return false;
}

static bool bytes_body_collected(BodyMethod* instance) {
    size_t body_size = ByteArray_size(*instance->body);
    jerry_value_t bytes;
    if(body_size > 0) {
        jerry_value_t array_buffer = jerry_arraybuffer_external(
            ByteArray_get(*instance->body, 0), body_size, instance->body);
        bytes = jerry_typedarray_with_buffer(JERRY_TYPEDARRAY_UINT8, array_buffer);
        jerry_value_free(array_buffer);
    } else {
        bytes = jerry_typedarray(JERRY_TYPEDARRAY_UINT8, 0);
    }

    js_check_and_free(jerry_promise_resolve(instance->promise, bytes));
    jerry_value_free(instance->promise);
    jerry_value_free(bytes);
    js_run_jobs();
    return true;
}

static bool json_body_collected(BodyMethod* instance) {
    size_t body_size = ByteArray_size(*instance->body);
    jerry_value_t json;
    if(body_size > 0) {
        json = jerry_json_parse(ByteArray_cget(*instance->body, 0), body_size);
    } else {
        json = jerry_undefined();
    }
    if(jerry_value_is_exception(json)) {
        js_reject_promise_with_exception(instance->promise, json);
    } else {
        jerry_value_free(jerry_promise_resolve(instance->promise, json));
        jerry_value_free(instance->promise);
        jerry_value_free(json);
    }
    js_run_jobs();
    return false;
}

static bool form_data_body_collected(BodyMethod* instance) {
    jerry_value_t exception = jerry_string_sz("unimplemented");
    jerry_value_free(jerry_promise_reject(instance->promise, exception));
    jerry_value_free(instance->promise);
    js_run_jobs();
    return false;
}

static bool text_body_collected(BodyMethod* instance) {
    size_t body_size = ByteArray_size(*instance->body);
    jerry_value_t string;
    if(body_size > 0) {
        string = jerry_string(ByteArray_get(*instance->body, 0), body_size, JERRY_ENCODING_UTF8);
    } else {
        string = jerry_string_sz("");
    }
    js_check_and_free(jerry_promise_resolve(instance->promise, string));
    jerry_value_free(instance->promise);
    jerry_value_free(string);
    js_run_jobs();
    return false;
}
