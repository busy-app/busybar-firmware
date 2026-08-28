#include "js_response.h"

#define TAG "JsResponse"

static jerry_value_t response_constructor(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(call_info);
    UNUSED(args);
    UNUSED(args_count);

    // TODO: Implementation

    return jerry_undefined();
}

jerry_value_t js_response_construct(void) {
    jerry_value_t global_obj = jerry_current_realm();

    jerry_value_t constructor = jerry_object_get_sz(global_obj, "Response");
    furi_check(jerry_value_is_function(constructor));

    jerry_value_t result = jerry_construct(constructor, NULL, 0);

    jerry_value_free(constructor);
    jerry_value_free(global_obj);

    return result;
}

void js_setup_response(void) {
    jerry_value_t global_obj = jerry_current_realm();

    jerry_value_t constructor = jerry_function_external(response_constructor);
    jerry_value_free(jerry_object_set_sz(global_obj, "Response", constructor));

    jerry_value_t prototype = jerry_object();
    js_check_and_free(jerry_object_set_proto(constructor, prototype));

    jerry_value_free(prototype);
    jerry_value_free(constructor);
    jerry_value_free(global_obj);
}
