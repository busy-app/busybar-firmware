#include "js_readable_stream.h"
#include "js_runner_i.h"
#include <m-deque.h>

#define TAG "JsReadableStream"

M_DEQUE_DEF(PromiseQueue, jerry_value_t);

typedef struct JsReadableStream {
    JsFetch* parent;

    PromiseQueue_t promise_queue;
    bool data_expected;

    bool has_closed_promise;
    jerry_value_t closed_promise;

    ChildStatus readable_stream_status;
    ChildStatus async_iterator_status;
} JsReadableStream;

static void readable_stream_free_cb(void* native_p, jerry_object_native_info_t* info_p);
static void async_iterator_free_cb(void* native_p, jerry_object_native_info_t* info_p);
static const jerry_object_native_info_t readable_stream_native_info = {
    .free_cb = readable_stream_free_cb};
static const jerry_object_native_info_t async_iterator_native_info = {
    .free_cb = async_iterator_free_cb};

static jerry_value_t async_iterator(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count);

static jerry_value_t readable_stream_cancel(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count);
static jerry_value_t get_reader(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count);

static void detach_sink(JsReadableStream* instance);

jerry_value_t js_readable_stream_alloc(JsFetch* parent) {
    JsReadableStream* instance = malloc(sizeof(JsReadableStream));
    instance->parent = parent;
    instance->data_expected = true;
    instance->readable_stream_status = ChildStatusRunning;
    instance->async_iterator_status = ChildStatusNotYet;
    instance->has_closed_promise = false;
    PromiseQueue_init(instance->promise_queue);

    jerry_value_t rs = jerry_object();
    jerry_object_set_native_ptr(rs, &readable_stream_native_info, instance);

    js_set_method_sym(rs, JERRY_SYMBOL_ASYNC_ITERATOR, async_iterator);
    js_set_method(rs, "cancel", readable_stream_cancel);
    js_set_method(rs, "getReader", get_reader);

    return rs;
}

static jerry_value_t chunk_to_uint8array(SizedBuffer data) {
    jerry_value_t arraybuffer = js_arraybuffer_from_sized_buffer(data);

    jerry_value_t uint8array = jerry_typedarray_with_buffer(JERRY_TYPEDARRAY_UINT8, arraybuffer);
    jerry_value_free(arraybuffer);
    return uint8array;
}

static void resolve_everything_with_done(JsReadableStream* instance, jerry_value_t value) {
    while(!PromiseQueue_empty_p(instance->promise_queue)) {
        jerry_value_t promise;
        PromiseQueue_pop_front(&promise, instance->promise_queue);

        jerry_value_t result = js_iterator_result(true, jerry_value_copy(value));
        js_check_and_free(jerry_promise_resolve(promise, result));
        jerry_value_free(promise);
        jerry_value_free(result);
    }
    jerry_value_free(value);
}

static void resolve_closed_promise(JsReadableStream* instance, const char* error_msg) {
    if(instance->has_closed_promise) {
        if(error_msg) {
            // jerry_value_t error = jerry_throw_sz(JERRY_ERROR_TYPE, error_msg);
            jerry_value_t error = jerry_string_sz(error_msg);
            jerry_value_free(jerry_promise_reject(instance->closed_promise, error));
            jerry_value_free(error);
        } else {
            jerry_value_t result = jerry_undefined();
            jerry_value_free(jerry_promise_resolve(instance->closed_promise, result));
            jerry_value_free(result);
        }
        jerry_value_free(instance->closed_promise);
        instance->has_closed_promise = false;
        js_run_jobs();
    }
}

static void free_if_can(JsReadableStream* instance) {
    if(instance->async_iterator_status != ChildStatusRunning &&
       instance->readable_stream_status != ChildStatusRunning) {
        resolve_closed_promise(instance, NULL);
        furi_check(PromiseQueue_empty_p(instance->promise_queue));
        PromiseQueue_clear(instance->promise_queue);

        free(instance);
    }
}

static void readable_stream_free_cb(void* native_p, jerry_object_native_info_t* info_p) {
    UNUSED(info_p);
    JsReadableStream* instance = native_p;
    instance->readable_stream_status = ChildStatusDone;
    free_if_can(instance);
}

static jerry_value_t iterator_next(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);

    JsReadableStream* instance =
        jerry_object_get_native_ptr(call_info->this_value, &async_iterator_native_info);
    JS_CHECK_INSTANCE();

    jerry_value_t promise = jerry_promise();

    if(instance->data_expected && instance->parent) {
        PromiseQueue_push_back(instance->promise_queue, jerry_value_copy(promise));

        js_fetch_data_sink_ready(instance->parent);
    } else {
        jerry_value_t result = js_iterator_result(true, jerry_undefined());
        js_check_and_free(jerry_promise_resolve(promise, result));
        jerry_value_free(result);
        js_run_jobs();
    }

    return promise;
}

static jerry_value_t iterator_return(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);

    JsReadableStream* instance =
        jerry_object_get_native_ptr(call_info->this_value, &async_iterator_native_info);
    JS_CHECK_INSTANCE();

    jerry_value_t promise = jerry_promise();

    jerry_value_t value = JS_ARG_OR_UNDEFINED(0);

    detach_sink(instance);

    jerry_value_t result = js_iterator_result(true, value);
    js_check_and_free(jerry_promise_resolve(promise, result));
    jerry_value_free(result);
    js_run_jobs();

    return promise;
}

static jerry_value_t readable_stream_cancel(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);

    JsReadableStream* instance =
        jerry_object_get_native_ptr(call_info->this_value, &readable_stream_native_info);
    JS_CHECK_INSTANCE();

    if(!instance->parent || js_fetch_cancel(instance->parent)) {
        jerry_value_t promise = jerry_promise();
        jerry_value_t result = jerry_undefined();
        js_check_and_free(jerry_promise_resolve(promise, result));
        jerry_value_free(result);

        return promise;
    } else {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Invalid state: body is locked");
    }
}

static void handle_data(JsReadableStream* instance, SizedBuffer data) {
    jerry_value_t promise;
    PromiseQueue_pop_front(&promise, instance->promise_queue);

    jerry_value_t result = js_iterator_result(false, chunk_to_uint8array(data));
    js_check_and_free(jerry_promise_resolve(promise, result));
    jerry_value_free(promise);
    jerry_value_free(result);
    js_run_jobs();
}

static void handle_done(JsReadableStream* instance) {
    instance->data_expected = false;
    resolve_closed_promise(instance, NULL);
    detach_sink(instance);

    resolve_everything_with_done(instance, jerry_undefined());
    js_run_jobs();
}

static void handle_error(JsReadableStream* instance, FuriString* error) {
    instance->data_expected = false;

    resolve_closed_promise(instance, "cancelled");
    detach_sink(instance);

    // resolve_everything_with_done(instance, jerry_throw_sz(JERRY_ERROR_TYPE, furi_string_get_cstr(event->error)));
    resolve_everything_with_done(instance, jerry_string_sz(furi_string_get_cstr(error)));

    furi_string_free(error);
    js_run_jobs();
}

static bool data_sink_callback(JsFetch* fetch, JsFetchDataEvent* event, void* callback_context) {
    UNUSED(fetch);
    JsReadableStream* instance = callback_context;
    if(!PromiseQueue_empty_p(instance->promise_queue)) {
        switch(event->type) {
        case JsFetchDataEventTypeData:
            handle_data(instance, event->data);
            break;
        case JsFetchDataEventTypeDone:
            handle_done(instance);
            break;
        case JsFetchDataEventTypeError:
            handle_error(instance, event->error);
            break;
        case JsFetchDataEventTypeInvalid:
            furi_check(false);
            break;
        }
        return true;
    } else {
        return false;
    }
}

static void async_iterator_free_cb(void* native_p, jerry_object_native_info_t* info_p) {
    UNUSED(native_p);
    UNUSED(info_p);
    JsReadableStream* instance = native_p;

    detach_sink(instance);
    resolve_everything_with_done(instance, jerry_undefined());
    js_run_jobs();
    instance->async_iterator_status = ChildStatusDone;
    free_if_can(instance);
}

static jerry_value_t async_iterator(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);

    JsReadableStream* instance =
        jerry_object_get_native_ptr(call_info->this_value, &readable_stream_native_info);
    JS_CHECK_INSTANCE();

    bool set_data_sink_ok = instance->parent &&
                            js_fetch_set_data_sink(instance->parent, data_sink_callback, instance);

    if(!set_data_sink_ok) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Data is already in use");
    }

    jerry_value_t iter = jerry_object();
    jerry_object_set_native_ptr(iter, &async_iterator_native_info, instance);

    js_set_method(iter, "next", iterator_next);
    js_set_method(iter, "return", iterator_return);
    js_set_method(iter, "throw", iterator_return);

    instance->async_iterator_status = ChildStatusRunning;

    return iter;
}

static jerry_value_t get_reader(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);

    JsReadableStream* instance =
        jerry_object_get_native_ptr(call_info->this_value, &readable_stream_native_info);
    JS_CHECK_INSTANCE();

    bool set_data_sink_ok = instance->parent &&
                            js_fetch_set_data_sink(instance->parent, data_sink_callback, instance);

    if(!set_data_sink_ok) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Data is already in use");
    }

    jerry_value_t reader = jerry_object();
    jerry_object_set_native_ptr(reader, &async_iterator_native_info, instance);

    // .read() is the same as iterator's .next()
    js_set_method(reader, "read", iterator_next);

    // .cancel() is the same as iterator's .return()
    js_set_method(reader, "cancel", iterator_return);

    {
        // .closed promise
        instance->has_closed_promise = true;
        instance->closed_promise = jerry_promise();
        js_check_and_free(jerry_object_set_sz(reader, "closed", instance->closed_promise));
    }

    instance->async_iterator_status = ChildStatusRunning;

    return reader;
}

static void detach_sink(JsReadableStream* instance) {
    if(instance->parent) {
        js_fetch_set_data_sink(instance->parent, NULL, NULL);
    }
    instance->parent = NULL;
}
