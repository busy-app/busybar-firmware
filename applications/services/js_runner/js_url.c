#include "js_url.h"

#define TAG "JsUrl"

static jerry_value_t js_url_method_origin_get(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(call_info);
    UNUSED(args);
    UNUSED(args_count);

    return jerry_string_external((const jerry_char_t*)"http://127.0.0.1", 16, NULL);
}

static jerry_value_t js_url_init(jerry_value_t this_value, jerry_value_t url_str) {
    UNUSED(url_str);

    js_set_property_getset(this_value, "origin", js_url_method_origin_get, NULL);

    return jerry_undefined();
}

static jerry_value_t url_constructor(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    if(!jerry_value_is_object(call_info->this_value)) {
        return jerry_throw_sz(
            JERRY_ERROR_TYPE, "Class constructor URL cannot be invoked without 'new'");
    }

    if(args_count == 0) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Too few arguments");
    }

    jerry_value_t url_str = JS_ARG(0);

    return js_url_init(call_info->this_value, url_str);
}

void js_setup_url(void) {
    jerry_value_t global_obj = jerry_current_realm();

    jerry_value_t constructor = jerry_function_external(url_constructor);
    jerry_value_free(jerry_object_set_sz(global_obj, "URL", constructor));

    jerry_value_t prototype = jerry_object();
    js_check_and_free(jerry_object_set_proto(constructor, prototype));

    jerry_value_free(prototype);
    jerry_value_free(constructor);
    jerry_value_free(global_obj);
}
