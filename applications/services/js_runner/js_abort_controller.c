#include "js_abort_controller.h"

#define TAG "JsAbortController"

static jerry_value_t abort_controller_constructor(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(call_info);
    UNUSED(args);
    FURI_LOG_D(TAG, "Args count: %lu", args_count);

    return jerry_undefined();
}

void js_setup_abort_controller(void) {
    jerry_value_t global_obj = jerry_current_realm();

    jerry_value_t constructor = jerry_function_external(abort_controller_constructor);
    jerry_value_free(jerry_object_set_sz(global_obj, "AbortController", constructor));

    jerry_value_t prototype = jerry_object();
    js_check_and_free(jerry_object_set_proto(constructor, prototype));

    jerry_value_free(prototype);
    jerry_value_free(constructor);
    jerry_value_free(global_obj);
}
