#include "js_request.h"

#define TAG "JsRequest"

static jerry_value_t
    js_request_init(jerry_value_t this_value, jerry_value_t url, jerry_value_t init) {
    if(!jerry_value_is_undefined(init) && !jerry_value_is_object(init) &&
       !jerry_value_is_null(init)) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Invalid parameter");
    }

    jerry_value_t result;
    do {
        if(jerry_value_is_string(url)) {
            jerry_value_free(jerry_object_set_sz(this_value, "url", url));
        } else if(js_is_instance_of(url, "Request")) {
            js_copy_property(this_value, url, "url");
            js_copy_property(this_value, url, "method");
            js_copy_property(this_value, url, "headers");
            js_copy_property(this_value, url, "body");
        } else {
            result = jerry_throw_sz(JERRY_ERROR_TYPE, "Invalid URL");
            break;
        }

        if(jerry_value_is_object(init)) {
            js_copy_property(this_value, init, "url");
            js_copy_property(this_value, init, "method");
            js_copy_property(this_value, init, "headers");
            js_copy_property(this_value, init, "body");
        }

        result = jerry_undefined();
    } while(false);

    return result;
}

static jerry_value_t request_constructor(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    if(!jerry_value_is_object(call_info->this_value)) {
        return jerry_throw_sz(
            JERRY_ERROR_TYPE, "Class constructor Request cannot be invoked without 'new'");
    }
    if(args_count == 0) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Too few arguments");
    }

    jerry_value_t url = JS_ARG_OR_UNDEFINED(0);
    jerry_value_t init = JS_ARG_OR_UNDEFINED(1);

    return js_request_init(call_info->this_value, url, init);
}

jerry_value_t js_request_construct(jerry_value_t url, jerry_value_t init) {
    jerry_value_t global_obj = jerry_current_realm();
    jerry_value_t constructor = jerry_object_get_sz(global_obj, "Request");
    furi_check(jerry_value_is_function(constructor));
    jerry_value_t args[2] = {url, init};
    jerry_value_t result = jerry_construct(constructor, args, 2);
    jerry_value_free(constructor);
    jerry_value_free(global_obj);
    return result;
}

void js_setup_request(void) {
    jerry_value_t global_obj = jerry_current_realm();

    jerry_value_t constructor = jerry_function_external(request_constructor);
    jerry_value_free(jerry_object_set_sz(global_obj, "Request", constructor));

    jerry_value_t prototype = jerry_object();
    js_check_and_free(jerry_object_set_proto(constructor, prototype));
    jerry_value_free(prototype);
    jerry_value_free(constructor);
    jerry_value_free(global_obj);
}
