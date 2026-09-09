#include "js_stubs.h"

static jerry_value_t stubs_constructor_common(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    JS_CHECK_CONSTRUCTOR();

    return jerry_undefined();
}

void js_setup_stubs(void) {
    jerry_value_t global_obj = jerry_current_realm();

    jerry_value_t constructor = jerry_function_external(stubs_constructor_common);
    jerry_value_free(jerry_object_set_sz(global_obj, "AbortController", constructor));
    jerry_value_free(jerry_object_set_sz(global_obj, "DOMException", constructor));
    jerry_value_free(jerry_object_set_sz(global_obj, "FormData", constructor));

    jerry_value_free(constructor);
    jerry_value_free(global_obj);
}
