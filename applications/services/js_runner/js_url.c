#include "js_url.h"

#include <toolbox/url.h>

#define TAG "JsUrl"

static const char* const js_url_part_keys[UrlPartIdMax] = {
    [UrlPartIdHref] = "href",
    [UrlPartIdOrigin] = "origin",
    [UrlPartIdProtocol] = "protocol",
    [UrlPartIdHost] = "host",
    [UrlPartIdHostname] = "hostname",
    [UrlPartIdPort] = "port",
    [UrlPartIdPathname] = "pathname",
    [UrlPartIdSearch] = "search",
};

static void url_free_cb(void* native_p, jerry_object_native_info_t* info_p) {
    UNUSED(info_p);

    Url* url_native = native_p;
    url_free(url_native);
}

static const jerry_object_native_info_t url_native_info = {.free_cb = url_free_cb};

static jerry_value_t js_url_init(jerry_value_t this_value, const char* url_str) {
    Url* url_native = url_alloc();

    if(!url_parse(url_native, url_str)) {
        url_free(url_native);
        return jerry_throw_sz(JERRY_ERROR_COMMON, "Invalid URL");
    }

    jerry_object_set_native_ptr(this_value, &url_native_info, url_native);

    for(uint32_t i = 0; i < COUNT_OF(js_url_part_keys); ++i) {
        const StringSlice* part = url_get_part(url_native, i);

        jerry_value_t prop =
            jerry_string_external((const jerry_char_t*)part->first_char, part->length, NULL);

        js_set_property(this_value, js_url_part_keys[i], prop);
    }

    return jerry_undefined();
}

static jerry_value_t url_constructor(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    jerry_value_t this_value = call_info->this_value;

    if(!jerry_value_is_object(this_value)) {
        return jerry_throw_sz(
            JERRY_ERROR_TYPE, "Class constructor URL cannot be invoked without 'new'");
    }

    if(args_count == 0) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Too few arguments");
    }

    jerry_value_t arg = JS_ARG(0);

    if(!jerry_value_is_string(arg)) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "String argument expected");
    }

    char* url_str = js_string_to_c_string(arg);
    furi_check(url_str != NULL);

    const jerry_value_t result = js_url_init(this_value, url_str);
    free(url_str);

    return result;
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
