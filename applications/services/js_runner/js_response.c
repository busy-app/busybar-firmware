#include "js_response.h"

#define RESPONSE_CLASS_NAME "Response"

static jerry_value_t response_constructor(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    JS_CHECK_CONSTRUCTOR();

    return jerry_undefined();
}

static jerry_value_t js_response_construct(void) {
    jerry_value_t global_obj = jerry_current_realm();

    jerry_value_t constructor = jerry_object_get_sz(global_obj, RESPONSE_CLASS_NAME);
    furi_check(jerry_value_is_function(constructor));

    jerry_value_t result = jerry_construct(constructor, NULL, 0);

    jerry_value_free(constructor);
    jerry_value_free(global_obj);

    return result;
}

jerry_value_t js_response_alloc(uint32_t status, StringSlice status_text) {
    jerry_value_t response = js_response_construct();

    jerry_value_t status_val = jerry_number(status);
    js_set_property(response, "status", status_val);

    jerry_value_t status_text_val = jerry_string(
        (const jerry_char_t*)status_text.first_char, status_text.length, JERRY_ENCODING_CESU8);
    js_set_property(response, "statusText", status_text_val);

    jerry_value_t ok_val = jerry_boolean(status / 100 == 2);
    js_set_property(response, "ok", ok_val);

    return response;
}

void js_setup_response(void) {
    jerry_value_t global_obj = jerry_current_realm();

    jerry_value_t constructor = jerry_function_external(response_constructor);
    jerry_value_free(jerry_object_set_sz(global_obj, RESPONSE_CLASS_NAME, constructor));

    jerry_value_free(constructor);
    jerry_value_free(global_obj);
}
