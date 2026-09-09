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

    static const char* stub_class_names[] = {
        "AbortController",
        "DOMException",
        "FormData",
    };

    for(uint32_t i = 0; i < COUNT_OF(stub_class_names); ++i) {
        jerry_value_t constructor = jerry_function_external(stubs_constructor_common);
        js_check_and_free(jerry_object_set_sz(global_obj, stub_class_names[i], constructor));
        jerry_value_free(constructor);
    }

    jerry_value_free(global_obj);
}
